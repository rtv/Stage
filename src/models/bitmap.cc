///////////////////////////////////////////////////////////////////////////
//
// File: fixedobstacle.cc
// Author: Andrew Howard
// Date: 29 Dec 2001
// Desc: Simulates fixed obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/bitmap.cc,v $
//  $Author: rtv $
//  $Revision: 1.3.4.3 $
//
///////////////////////////////////////////////////////////////////////////

#include <float.h>
#include <math.h>
#include "bitmap.hh"
#include "matrix.hh"

//#define DEBUG

///////////////////////////////////////////////////////////////////////////
// Default constructor
CBitmap::CBitmap(LibraryItem* libit, int id, CEntity *parent)
    : CEntity(libit, id, parent)
{
  vision_return = true;
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
  puck_return = false; // we trade velocities with pucks
  idar_return = IDARReflect;
  

  this->size_x = 1.0;
  this->size_y = 1.0;
  
#ifdef INCLUDE_RTK2
  //grid_enable = false;

  // We cant move fixed obstacles (yet!)
  //this->movemask = 0;
  // TODO - add Update() method so we can move bitmaps around
  //this->movemask = RTK_MOVE_TRANS | RTK_MOVE_ROT;
#endif
}

int CBitmap::SetProperty( int con, stage_prop_id_t property, 
		     void* value, size_t len )
{
  PRINT_DEBUG3( "setting prop %d (%d bytes) for ent %p",
		property, len, this );
  
  assert( value );
  assert( len > 0 );
  assert( (int)len < MAX_PROPERTY_DATA_LEN );
  
  
  switch( property )
    {

    case STG_PROP_ENTITY_POSE:
    case STG_PROP_ENTITY_SIZE:
      RenderRects( false ); // remove rects from the matrix
      
      // let the entity change my pose or size
      CEntity::SetProperty( con, property, value, len );
      
      RenderRects( true ); // draw the rects back into the matrix
      
      // this will render my shape, if I have one
      //double x, y, th;
      //GetGlobalPose( x,y,th );    
      //ReMap(x, y, th);
      break;
      
    default: // we didn't handle it. let the entity try
      // let the entity handle things
      CEntity::SetProperty( con, property, value, len );
      break;
    }
  
  return 0;
}


#ifdef INCLUDE_RTK2


#endif




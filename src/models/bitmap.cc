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
//  $Revision: 1.3.4.2 $
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
  
  this->rects = NULL;
  this->rect_count = 0;

  // the upper bounds of our rectangles
  this->rects_max_x = 0;
  this->rects_max_y = 0;

  this->shape = ShapeNone;
  this->size_x = 1.0;
  this->size_y = 1.0;
  
#ifdef INCLUDE_RTK2
  grid_enable = false;

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
    case STG_PROP_BITMAP_RECTS:
      {
	// delete any previous rectangles we might have
	if( this->rects ) delete[] rects;
	
	// we just got this many rectangles
	this->rect_count = len / sizeof(stage_rotrect_t);
	
	// make space for the new rects
	this->rects = new stage_rotrect_t[ this->rect_count ];
	
	// copy the rects into our local storage
	memcpy( this->rects, value, len );
	
	// find the bounds
	rects_max_x = rects_max_y = 0.0;
	
	int r;
	for( r=0; r<this->rect_count; r++ )
	  {
	    if( (rects[r].x+rects[r].w)  > rects_max_x ) 
	      rects_max_x = (rects[r].x+rects[r].w);

	    if( (rects[r].y+rects[r].h)  > rects_max_y ) 
	      rects_max_y = (rects[r].y+rects[r].h);
	  }

	PRINT_WARN2( "bounds %.2f %.2f", rects_max_x, rects_max_y );

	RenderRects();
	// change my size to cover the rectangles
	//stage_size_t sz;
	//sz.x = maxx;
	//sz.y = maxy;
	//this->SetProperty( -1, STG_PROP_ENTITY_SIZE, &sz, sizeof(sz) );
  
      }
      break;
      
    default: // we didn't handle it. let the entity try
      break;
    }

  // let the entity do some more work
  CEntity::SetProperty( con, property, value, len );
  
  return 0;
}

void CBitmap::RenderRects( void )
{
  // TODO: find the scaling
  double scalex = size_x / rects_max_x;
  double scaley = size_y / rects_max_y;
    
  double x,y,a,w,h;

  int r;
  for( r=0; r<this->rect_count; r++ )
    {
      x = ((this->rects[r].x + this->rects[r].w/2.0) * scalex) - size_x/2.0;
      y = ((this->rects[r].y + this->rects[r].h/2.0)* scaley) - size_y/2.0;
      a = this->rects[r].a;
      w = this->rects[r].w * scalex;
      h = this->rects[r].h * scaley;
      
      CEntity::matrix->SetRectangle( x, y, a, w, h, 
				     this, CEntity::ppm, true);
    }
}


#ifdef INCLUDE_RTK2

void CBitmap::RtkStartup()
{
  CEntity::RtkStartup();
  PRINT_DEBUG1( "rendering %d rectangles", this->rect_count );

  // bitmaps don't need labels
  //rtk_fig_clear( this->label );
  
  double scalex = size_x / rects_max_x;
  double scaley = size_y / rects_max_y;
  
  rtk_fig_origin( this->fig, local_px, local_py, local_pth );
  rtk_fig_color_rgb32(this->fig, this->color);
  
  // now create GUI rects
  int r;
  for( r=0; r<this->rect_count; r++ )
    {
      double x, y, a, w, h;
      
      x = ((this->rects[r].x + this->rects[r].w/2.0) * scalex) - size_x/2.0;
      y = ((this->rects[r].y + this->rects[r].h/2.0)* scaley) - size_y/2.0;
      a = this->rects[r].a;
      w = this->rects[r].w * scalex;
      h = this->rects[r].h * scaley;
      
      rtk_fig_rectangle(this->fig, x, y, a, w, h, 0 ); 
    }
}

#endif




///////////////////////////////////////////////////////////////////////////
//
// File: truthdevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 6 July 2001
// Desc: Ground truth device, useful e.g. for GUIs (xpsi) and data loggers
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/truthdevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

#include "world.hh"
#include "truthdevice.hh"

//#define DEBUG
//#define VERBOSE
#undef DEBUG
#undef VERBOSE

// the maximum number of records we can send and receive from player per update
// this limits the memory and bandwidth usage of this device
#define MAX_TRUTH_EXPORT 1000 
#define MAX_TRUTH_IMPORT 1 // must be 1 for now

///////////////////////////////////////////////////////////////////////////
// Constructor
//
CTruthDevice::CTruthDevice(CWorld *world, CEntity *parent )
  : CEntity( world, parent )
{
  if( parent ) puts( "Warning: TruthDevice has a parent." ); 
  
  // set the Player IO sizes correctly for this type of Entity
  m_data_len = sizeof( player_generic_truth_t ) * MAX_TRUTH_EXPORT;
  m_command_len = sizeof( player_generic_truth_t ) * MAX_TRUTH_IMPORT;
  m_config_len = 0;// not configurable (yet. Update rate should be, though)

  m_player_type = PLAYER_TRUTH_CODE; // from player's messages.h
  m_player_port = GLOBALPORT; // visible to all players

  //m_interval = 0.05; // update at 20Hz - twice as fast as most things
  m_interval = 0.1; // update at 20Hz - twice as fast as most things
}


///////////////////////////////////////////////////////////////////////////
// Gather and publish truth data
//
void CTruthDevice::Update( double sim_time )
{
#ifdef debug
  CEntity::Update( sim_time );
#endif

  if( Subscribed() < 1 ) return; // noone is listening :(

  player_generic_truth_t truth;  
  int updatedAnEntity = false;
  int num_objs = m_world->GetObjectCount();
  
  // import commands
  if( GetCommand( &truth,sizeof(truth)) == sizeof(truth) ) 
    {
      //printf( "Truth received (%d,%d,%d)\n", 
      //      truth.id.port, truth.id.type, truth.id.index );
    
      // this isn't very safe but it'll do for now
      m_info_io->command_avail = 0; // consume this command
    
      CEntity* ent = 0;

      // find the object with this id
      for( int i=0; i<num_objs; i++ )
	{
	  ent = m_world->GetObject( i );
	  
	  if( ent->m_player_port == truth.id.port &&
	      ent->m_player_type == truth.id.type &&
	      ent->m_player_index == truth.id.index )
	    {
	      ent->SetGlobalPose( truth.x/1000.0, truth.y/1000.0, 
				  DTOR(truth.th) );
	      updatedAnEntity = true;

	      //ent->m_publish_truth = true;
	      break;
	    }
	}
      if( !updatedAnEntity ) 
	printf( "Stage Warning: "
		"truth received for unknown object (%d,%d,%d)\n",
		truth.id.port, truth.id.type, truth.id.index );
    }
  
  // Check to see if it is time to update
  //  - if not, return right away unless we received a command
  if( !updatedAnEntity && (sim_time - m_last_update < m_interval )) 
    return;
  
  m_last_update = sim_time;
	
  // if nothing has changed, quit immediately
  //if( m_world->m_truth_is_current ) return;

  // OK its time to update - now gather the truths from everywhere
  
  player_generic_truth_t truths[ MAX_TRUTH_EXPORT ];

  static int i = 0;

  int t = 0; 
  // we've done 'em all, or the buffer is full
  while( t<num_objs && t<MAX_TRUTH_EXPORT  ) 
    {
      memset( &(truths[t]), 0 , sizeof( player_generic_truth_t ) );

      // get a pointer to the next entity
      CEntity* ent = m_world->GetObject( i );

      // if the thing hasn't changed, we don't need to export it's truth
      //if( !ent->modified ) break;
      
      truths[t].id.port = ent->m_player_port;
      truths[t].id.type = ent->m_player_type;
      truths[t].id.index = ent->m_player_index;
      
      if( ent->m_parent_object )
	{
	  truths[t].parent.port = ent->m_parent_object->m_player_port;
	  truths[t].parent.type = ent->m_parent_object->m_player_type;
	  truths[t].parent.index = ent->m_parent_object->m_player_index;
	}
      else
	memset( &(truths[t].parent), 0, sizeof( truths[t].parent ) );
      
      double x, y, th;
      ent->GetGlobalPose( x, y, th );
  
      truths[t].x = (uint32_t)( x * 1000 );
      truths[t].y = (uint32_t)( y * 1000 );
      truths[t].th = (uint16_t)RTOD( th );  
      truths[t].w = (uint16_t)(ent->m_size_x * 1000);
      truths[t].h = (uint16_t)(ent->m_size_y * 1000);
      
      i++; // go to the next object
      t++; // count the number of truths we've exported this time

      //cycle through the indexes - mod with the total so we don't get too big
      i %= num_objs;
    }
  // publish the truth data
  PutData( &truths, t * sizeof(player_generic_truth_t) );    

  // the published truth now matches the actual truth
  m_world->m_truth_is_current = true; 
}






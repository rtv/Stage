/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: A root device model - replaces the CWorld class
 * Author: Richard Vaughan
 * Date: 31 Jan 2003
 * CVS info: $Id: root.cc,v 1.1.2.8 2003-02-27 02:10:14 rtv Exp $
 */


#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "sio.h"
#include "root.hh"
#include "matrix.hh"
#include "library.hh"

#define DEBUG

extern Library model_library;
extern stage_gui_library_item_t gui_library[];

// constructor
CRootEntity::CRootEntity( )
  : CEntity( 0, "root", "red", NULL )    
{
  PRINT_DEBUG( "Creating root model" );
  
  CEntity::root = this; // phear me!
  
  size_x = 10.0; // a 10m world by default
  size_y = 10.0;
  ppm = 10; // default 10cm resolution passed into matrix (DEBUG)
  //this->ppm = 100; // default 10cm resolution passed into matrix

  // the global origin is the bottom left corner of the root object
  origin_x = size_x/2.0;
  origin_y = size_y/2.0;

  vision_return = false; 
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
  idar_return = IDARReflect;
  
  
  /// default 5cm resolution passed into matrix
  double ppm = 20.0;
  PRINT_DEBUG3( "Creating a matrix [%.2fx%.2f]m at %.2f ppm",
		size_x, size_y, ppm );
  
  assert( matrix = new CMatrix( size_x, size_y, ppm, 1) );
  
  model_library.InsertRoot( this );
  
#ifdef INCLUDE_RTK2
  grid_enable = true;
#endif
}

int CRootEntity::Property( int con, stage_prop_id_t property, 
			      void* value, size_t len, stage_buffer_t* reply )
{
  PRINT_DEBUG3( "setting prop %d (%d bytes) for ROOT ent %p",
		property, len, this );
    
  double x, y, th;
  
  switch( property )
    {      
      // we'll tackle this one, 'cos it means creating a new matrix
    case STG_PROP_ENTITY_SIZE:
      PRINT_WARN( "setting size of ROOT" );
      assert( len == sizeof( stage_size_t ) );
      matrix->Resize( size_x, size_y, ppm );      
      this->MapFamily();
      break;
      
      /*    case STG_PROP_ROOT_PPM:
      PRINT_WARN( "setting PPM of ROOT" );
      assert(len == sizeof(ppm) );
      memcpy( &(ppm), value, sizeof(ppm) );
      
      matrix->Resize( size_x, size_y, ppm );      
      this->MapFamily();
      break;
      */

    case STG_PROP_ROOT_CREATE:
      if( value && reply ) // both must be good
	{
	  int num_models =  len / sizeof(stage_model_t);
	  
	  PRINT_WARN1( "root received CREATE request for %d models", num_models );	  
	  for( int m=0; m<num_models; m++ )
	    {
	      // make a new model from the correct offset in the value buffer.
	      // this call fills in their ID field correctly
	      stage_model_t* mod = (stage_model_t*)
		((char*)value + m*sizeof(stage_model_t));
	      
	      if( model_library.CreateEntity( mod ) == -1 )
		PRINT_ERR( "failed to create a new model (invalid ID)" );	    
	    }
	  
	  // reply with the same array of models, with the IDs set
	  SIOBufferProperty( reply, this->stage_id, 
			     STG_PROP_ROOT_CREATE,
			     value, len,
			     STG_ISREPLY );
	}
      else
	PRINT_ERR2( "root received CREATE request but didn't"
		    "get data and/or reply buffers (data: %p) (reply: %p)",		    value, reply );
      break;
      
    case STG_PROP_ROOT_DESTROY:
      if( value )
	{
	  int num_models = len / sizeof(stage_model_t);
	  
	  PRINT_WARN1( "root received DESTROY request for %d models", 
		       num_models );
	  
	  for( int m=0; m<num_models; m++ )
	    {	    
	      stage_model_t* mod = (stage_model_t*)
		((char*)value + m*sizeof(stage_model_t));
	      
	      // todo - remove from my list of children
	      
	      if( model_library.DestroyEntity( mod ) == -1 )
		PRINT_ERR1( "failed to destroy model %d", mod->id );
	    }
	}
      
      if( reply ) //acknowledge by returning the same models
	SIOBufferProperty( reply, this->stage_id, 
			   STG_PROP_ROOT_DESTROY,
			   value, len, STG_ISREPLY );
      break;
	
    case STG_PROP_ROOT_GUI:
      if( value )
	{
	  assert( len == sizeof(stage_gui_config_t) );
	  stage_gui_config_t* cfg  = (stage_gui_config_t*)value;
	  
	  PRINT_DEBUG1( "received GUI configuraton for library %s", 
			cfg->token );
	  
	  // find the library index of the GUI library with this token
	  for( stage_gui_library_item_t* gui = gui_library; 
	       strlen(gui->token) > 0;
	       gui++ )
	    {
	      if( strcmp( gui->token, cfg->token ) == 0 )
		{
		  // call the config function from the correct library and return
		  if( (gui->load_func)( cfg ) == -1 )
		    PRINT_ERR1( "gui load failed for gui %s",
				gui->token );
		  break;
		}
	    }
	  
	  // re-create my GUI parts 
	  GuiEntityShutdown( this );
	  GuiEntityStartup( this );

	  PRINT_WARN1( "Received config for unknown GUI library %s", 
		       cfg->token );

	  if( reply ) //acknowledge (no data)
	    SIOBufferProperty( reply, this->stage_id, 
			       STG_PROP_ROOT_GUI,
			       NULL, 0, STG_ISREPLY );
	}
    default: 
      break;
    }
  
  // get the inherited behavior 
  return CEntity::Property( con, property, value, len, reply  );
}


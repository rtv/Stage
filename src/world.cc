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
 * Desc: A world device model - replaces the CWorld class
 * Author: Richard Vaughan
 * Date: 31 Jan 2003
 * CVS info: $Id: world.cc,v 1.153 2003-10-22 19:51:02 rtv Exp $
 */


#include <math.h>
#include <stdio.h>
#include <assert.h>

#define DEBUG

#include "stage.h"
#include "world.hh"
#include "matrix.hh"

// declare a function from main.cc for use here
// write a stg_property_t on the channel returns TRUE on success, else FALSE
gboolean StgPropertyWrite( GIOChannel* channel, stg_property_t* prop );


ss_world_t* ss_world_create( stg_id_t id, stg_world_create_t* rc ) 
{
  ss_world_t* world = (ss_world_t*)calloc( sizeof(ss_world_t), 1 );
  assert( world );

  
  // must set the id, name and token before using the BASE_DEBUG macro
  world->id = id; // a unique id for this object
  world->name = g_string_new( rc->name );
  WORLD_DEBUG( world, "construction" );
  
  
  //world->client = client; // this client created this world
  world->width = rc->width;
  world->height = rc->height;
  world->ppm = 1.0/rc->resolution;
  world->running = FALSE;  
  world->win = NULL; // gui data is attached here
  world->models = NULL; // no models initially
  
  WORLD_DEBUG( world, "world construction complete" );

  WORLD_DEBUG( world,"world startup");
  
  g_assert( ss_world_create_matrix( world ) );

  if( world->win ) PRINT_WARN( "creating window, but window pointer not NULL" );  
  world->win = gui_window_create( world, 0,0 ); // use default dimensions

  world->running = TRUE;
  
  WORLD_DEBUG( world, "world startup complete" );

  // console output
  PRINT_MSG2( "Created world \"%s\" (%d).", world->name->str, world->id ); 

  world->interval = 0.1; 
  //world->clock_tag = g_timeout_add( (int)(world->interval * 1000.0), 
  //			    stg_world_clock_tick, world );
  
  return world;
}


void ss_model_update( gpointer data, gpointer userdata )
{
  CEntity* ent = (CEntity*)data;
  // recursively update the model's children
  g_list_foreach( ent->children, ss_model_update, NULL );
  
  // todo?
  //if( (world->time - ent->last_update) > (double)ent->interval / 1000.0 )

  // then the model itself
  ent->Update();
}

void ss_world_save( ss_world_t* world )
{
  PRINT_DEBUG1( "sending SAVE message to client %p", world->client );
  
  // send a save message with the world's id as payload
  int fd = g_io_channel_unix_get_fd(world->client->channel);
  stg_fd_msg_write( fd, STG_MSG_SAVE, &world->id, sizeof(world->id) );
}

void ss_world_update( ss_world_t* world  )
{  
  world->time += world->interval;
  
  PRINT_DEBUG2( "world \"%s\" time: %.3f", 
		world->name->str, world->time );

  g_list_foreach( world->models, ss_model_update, NULL );
}

int ss_world_destroy( ss_world_t* world )
{
  WORLD_DEBUG( world, "world destruction" );

  // recursively shutdown all the entities in the world
  if( world->models ) for( GList* l = world->models; l; l=l->next )
    delete (CEntity*)l->data;
  
  g_list_free( world->models );
  world->models = NULL;
  
  // kill this gui window
  if( world->win ) gui_window_destroy( world->win );
  world->win = NULL;
  
  world->running = FALSE;
  WORLD_DEBUG( world,"world shutdown complete");
  
  // remove the world from the client that created it
  //world->client->worlds = g_list_remove( world->client->worlds, world ); 

  if( world->matrix ) ss_world_destroy_matrix( world );


  WORLD_DEBUG( world, "world destruction complete" );

  PRINT_MSG1( "Destroyed world \"%s\".", world->name->str );

  free(world);
  return 0; //ok
}


CMatrix* ss_world_create_matrix( ss_world_t* world )
{
  g_assert( world );
  if( world->matrix ) ss_world_destroy_matrix( world );
  
  WORLD_DEBUG3( world, "Creating a matrix [%.2fx%.2f]m at %.2f ppm",
		world->width, world->height, world->ppm );
  g_assert( (world->matrix = 
	     new CMatrix( world->width, world->height, world->ppm, 1) ) );
  
  return world->matrix;
}

void ss_world_destroy_matrix( ss_world_t* world )
{
  g_assert( world );
  if( world->matrix == NULL )
    PRINT_WARN( "destroy matrix called for NULL matrix pointer!" );
  else
    delete world->matrix;
  
  world->matrix = NULL;
}
  
stg_property_t* ss_world_property_get( ss_world_t* world, stg_prop_id_t id )
{
  stg_property_t* prop = NULL;
  
  switch( id )
    {
    case STG_MOD_TIME:
      //PRINT_DEBUG2( "getting world %d time %.3f", world->id, world->time );

      prop = stg_property_create(); 
      prop->id = world->id;
      prop->type = STG_MOD_TIME;
      
      prop = stg_property_attach_data(prop, &world->time, sizeof(world->time));
      break;
      
    default:
      PRINT_WARN1( "failed to get world property %s",
		   stg_property_string( id ) );
      break;
    }

  return prop;
}

void ss_world_property_set( ss_world_t* world, stg_property_t* prop )
{
  // todo
  PRINT_WARN1( "setting world property %s not implemented",
	      stg_property_string(prop->type) );
}

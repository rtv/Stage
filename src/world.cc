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
 * CVS info: $Id: world.cc,v 1.138.4.3 2003-08-19 00:57:19 rtv Exp $
 */


#include <math.h>
#include <stdio.h>
#include <assert.h>

#define DEBUG

#include "stage.h"
#include "world.hh"
#include "matrix.hh"

extern GHashTable* global_hash_table;
extern int global_next_available_id;



stg_world_t* stg_world_create( GIOChannel* channel, stg_world_create_t* rc )
{
  stg_world_t* world = (stg_world_t*)calloc( sizeof(stg_world_t), 1 );
  assert( world );

  // must set the id, name and token before using the BASE_DEBUG macro
  world->id = global_next_available_id++; // a unique id for this object
  world->name = g_string_new( rc->name );
  world->token = g_string_new( rc->token );
  WORLD_DEBUG( world, "construction" );

  
  world->width = rc->width;
  world->height = rc->height;
  world->ppm = 1.0/rc->resolution;

  world->channel = channel;

  world->node = NULL; // we attach a model tree here
  world->running = FALSE;
  
  world->win = NULL;
  
  // create a root node that will contain my model tree
  world->node = g_node_new( world );

  // add my node to the hash table with my id as the key (this ID
  // should not already exist)
  g_assert( g_hash_table_lookup( global_hash_table, &world->id ) == NULL ); 
  g_hash_table_insert( global_hash_table, &world->id, world->node );  
  
  WORLD_DEBUG( world, "world construction complete" );

  return world;
}

int stg_world_destroy( stg_world_t* world )
{
  WORLD_DEBUG( world, "world destruction" );

  // must shut everything down before destroying the matrix
  // as children will try to undrender themselves
  if( world->running ) stg_world_shutdown( world );
  //  if( world->matrix ) delete matrix;

  WORLD_DEBUG( world, "world destruction complete" );

  return 0; //ok
}

int stg_world_startup( stg_world_t* world )
{
  WORLD_DEBUG( world,"world startup");
  
  g_assert( stg_world_create_matrix( world ) );

  if( world->win ) PRINT_WARN( "creating window, but window pointer not NULL" );  
  world->win = stg_gui_window_create( world, 0,0 ); // use default dimensions

  // startup all the entities in the world (possibly none at this point)
  for( CEntity* child = stg_world_first_child(world); child; 
       child = stg_ent_next_sibling( child ) )
    if( child ) child->Startup();
  
  world->running = TRUE;
  
  WORLD_DEBUG( world, "world startup complete" );
  return 0; //ok
}

int stg_world_shutdown( stg_world_t* world )
{
  WORLD_DEBUG( world, "world shutdown");
  
  // shutdown all the entities in the world
  for( CEntity* child = stg_world_first_child( world ); child; 
       child = stg_ent_next_sibling( child ) )
    if( child ) child->Shutdown();
  
  // kill this gui window
  if( world->win ) stg_gui_window_destroy( world->win );
  world->win = NULL;

  if( world->matrix ) stg_world_destroy_matrix( world );
  
  world->running = FALSE;
  WORLD_DEBUG( world,"world shutdown complete");
  return 0;
}

CMatrix* stg_world_create_matrix( stg_world_t* world )
{
  g_assert( world );
  if( world->matrix ) stg_world_destroy_matrix( world );
  
  WORLD_DEBUG3( world, "Creating a matrix [%.2fx%.2f]m at %.2f ppm",
		world->width, world->height, world->ppm );
  g_assert( (world->matrix = 
	     new CMatrix( world->width, world->height, world->ppm, 1) ) );
  
  return world->matrix;
}

void stg_world_destroy_matrix( stg_world_t* world )
{
  g_assert( world );
  if( world->matrix == NULL )
    PRINT_WARN( "destroy matrix called for NULL matrix pointer!" );
  else
    delete world->matrix;
  
  world->matrix = NULL;
}
  


// TODO - dynamic resizing and resolution changes to world
/*
  int CWorld::SetProperty( stg_prop_id_t ptype, void* data, size_t len )
  {
  //PRINT_MSG4( "Setting World Entity %d:%s prop %d bytes %d", 
  //      this->id, this->token->str, ptype, len );
  
  switch( ptype )
  {
      // we'll tackle this one, 'cos it means creating a new matrix
    case STG_PROP_WORLD_SIZE:
      BASE_DEBUG(n "setting size of world" );
      assert( len == sizeof( stg_size_t ) );
      //matrix->Resize( size_x, size_y, ppm );      
      //this->MapFamily();
      break;
      
      //    case STG_PROP_WORLD_PPM:
      //PRINT_WARN( "setting PPM of WORLD" );
      //assert(len == sizeof(ppm) );
      //memcpy( &(ppm), value, sizeof(ppm) );
      
      //matrix->Resize( size_x, size_y, ppm );      
      //this->MapFamily();
      //break;
      
      
      default: 
      break;
      }

  if( this->win ) stg_gui_window_update( this, ptype );

      return CBase::SetProperty( ptype, data, len );

      }
*/

/*

CWorld::CWorld( GIOChannel* chan, stg_world_create_t* rc )
{
  g_assert(rc);
  // must set the id, name and token before using the BASE_DEBUG macro
  this->id = global_next_available_id++; // a unique id for this object
  this->name = g_string_new( rc->name );
  this->token = g_string_new( rc->token );
  BASE_DEBUG( "world construction" );

  
  this->width = rc->width;
  this->height = rc->height;
  this->ppm = 1.0/rc->resolution;

  this->channel = chan;

  this->node = NULL; // we attach a model tree here
  this->running = FALSE;
  
  this->win = NULL;
  
  BASE_DEBUG3( "Creating a matrix [%.2fx%.2f]m at %.2f ppm",
	       width, height, ppm );
  g_assert( (this->matrix = new CMatrix( width, height, ppm, 1)) );
  
  // create a root node that will contain my model tree
  this->node = g_node_new( this );

  // add my node to the hash table with my id as the key (this ID
  // should not already exist)
  g_assert( g_hash_table_lookup( global_hash_table, &this->id ) == NULL ); 
  g_hash_table_insert( global_hash_table, &this->id, this->node );  
  
  BASE_DEBUG( "world construction complete" );
}

CWorld::~CWorld( void )
{
  BASE_DEBUG( "world destruction" );

  // must shut everything down before destroying the matrix
  // as children will try to undrender themselves
  if( running ) this->Shutdown();
  if( matrix ) delete matrix;

  BASE_DEBUG( "world destruction complete" );
}
 
// Initialise entity  
int CWorld::Startup( void )
{
  BASE_DEBUG("world startup");
  
  if( this->win ) PRINT_WARN( "creating window, but window pointer not NULL" ); 
 
  this->win = stg_gui_window_create( this, 0,0 ); // use default dimensions

  // startup all the entities in the world (possibly none at this point)
  for( CEntity* child = stg_world_first_child(this); child; 
       child = stg_ent_next_sibling( child ) )
    if( child ) child->Startup();
  
  this->running = TRUE;
  
  BASE_DEBUG( "world startup complete" );
  return 0;
}

// Finalize entity
int CWorld::Shutdown()
{
  BASE_DEBUG("world shutdown");
  
  // shutdown all the entities in the world
  for( CEntity* child = stg_world_first_child( this ); child; 
       child = stg_ent_next_sibling( child ) )
    if( child ) child->Shutdown();
  
  // kill this gui window
  if( this->win ) stg_gui_window_destroy( this->win );
  this->win = NULL;
  
  this->running = FALSE;
  BASE_DEBUG("world shutdown complete");
  return 0;
}
 
*/

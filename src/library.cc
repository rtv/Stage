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
 * Desc: The library knows how to create devices. All device models
 * must register a worldfile token and a creator function
 * (usually a static wrapper for a constructor) with the library. Just
 * add your device to the static table below.
 *
 * Author: Richard Vaughan Date: 27 Oct 2002 (this header added) 
 * CVS info: $Id: library.cc,v 1.13.4.9 2003-02-10 01:08:32 gerkey Exp $
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "library.hh"
#include "entity.hh"

//#define DEBUG

// Library //////////////////////////////////////////////////////////////

// constructor from null-terminated array
Library::Library( const stage_libitem_t item_array[] )
{
  entPtrs = NULL;
  model_count = 0;
  
  items = item_array;
  
  // count all the items in the array
  item_count = 0;
  for( stage_libitem_t* item = (stage_libitem_t*)items; item->token; item++ )
    {
      PRINT_DEBUG4( "counting library item %d : %s %s %p", 
		    item_count, item->token, item->colorstr, item->fp );
      
      item_count++;
    }
}


void Library::StoreEntPtr( int id, CEntity* ent )
{
  // TODO - extend this with dynamic storage
  // store the pointer in our array
  entPtrs[id] = ent;
}   

// look in the array for an item with matching token and return a pointer to it
stage_libitem_t* Library::FindItemWithToken( const stage_libitem_t* items, 
					     int count, char* token )
{
  for( int c=0; c<count; c++ )
    if( strcmp( items[c].token, token ) == 0 )
      return (stage_libitem_t*)&(items[c]);
  
  return NULL; // didn't find the token
}

// create an instance of an entity given a worldfile token
CEntity* Library::CreateEntity( stage_model_t* model )
{
  assert( model );
  
  // look up this token in the library
  stage_libitem_t* libit = FindItemWithToken( model->token );
  
  CEntity* ent;
  
  if( libit == NULL )
    {
      PRINT_ERR1( "requested model '%s' is not in the library", 
		  model->token );
      return NULL;
    }
  
  if( libit->fp == NULL )
  {
    PRINT_ERR1( "requested model '%s' doesn't have a creator function", 
		model->token );
    return NULL;
  }
  
  
  PRINT_DEBUG4( "creating model %p  - %d %s %d",
		model, model->id, model->token, model->parent_id );
  
  // if it has a valid parent, look up the parent's address
  CEntity* parentPtr = NULL;
  if( model->parent_id != -1 ) 
    {
      if( (parentPtr = entPtrs[model->parent_id] ) == NULL ) 
	{
	  PRINT_ERR1( "parent specified (%d) but pointer not found",
		      model->parent_id );
	  return NULL;
	}
    }
  
  // set the id of this model to the next available value
  model->id = model_count;
  
  // create an entity through the creator callback fucntion  
  // which calls the constructor
  ent = (*libit->fp)( model->id, 
		      (char*)model->token, 
		      (char*)libit->colorstr, 
		      parentPtr ); 
  
  assert( ent );
  
  // need to do some startup outside the constructor to allow for
  // full polymorphism
  if( ent->Startup() == -1 ) // if startup fails
    {
      PRINT_WARN3( "Startup failed for model %d (%s) at %p",
		   model->id, model->token, ent );
      delete ent;
      return NULL;
    }
  
  // now start the GUI repn for this entity
  if( GuiEntityStartup( ent ) == -1 )
    {
      PRINT_WARN3( "Gui startup failed for model %d (%s) at %p",
		   model->id, model->token, ent );
      delete ent; // destructor calls CEntity::Shutdown()
      return NULL;
    }      
  
  // add space for this model
  entPtrs = (CEntity**)realloc( entPtrs, (model_count+1) * sizeof(entPtrs[0]) );
  entPtrs[model_count] = ent;
  model_count++;
 
  for( int t=0; t<model_count; t++ )
    printf( "entPtrs[%d] = %p\n", t, entPtrs[t] );

  if( ent ) PRINT_DEBUG3(  "Startup successful for model %d (%s) at %p",
			   model->id, model->token, ent );
  return ent; // NULL if failed
} 

void Library::Print( void )
{
  PRINT_MSG("[Library contents:");
  
  int i;
  for( i=0; i<item_count; i++ )
    printf( "\n\t%s:%s:%p", 
	    items[i].token, items[i].colorstr, items[i].fp );
  
  puts( "]" );
}


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
 * CVS info: $Id: library.cc,v 1.13.4.2 2003-02-01 23:19:51 rtv Exp $
 */

#include "library.hh"
#include "entity.hh"
#include "cwrapper.h"

#include "root.hh"
/*
#include "models/bitmap.hh"
#include "models/box.hh"
#include "models/bumperdevice.hh"
#include "models/broadcastdevice.hh"
#include "models/gpsdevice.hh"
#include "models/gripperdevice.hh"
#include "models/idardevice.hh"
#include "models/idarturretdevice.hh"
#include "models/fiducialfinderdevice.hh"
#include "models/laserdevice.hh"
#include "models/motedevice.hh"
#include "models/positiondevice.hh"
#include "models/powerdevice.hh"
#include "models/ptzdevice.hh"
#include "models/puck.hh"
#include "models/sonardevice.hh"
#include "models/truthdevice.hh"
#include "models/visiondevice.hh"
#include "models/regularmcldevice.hh"
//#include "models/bpsdevice.hh"
*/

typedef CreatorFunctionPtr CFP;

// this array defines the models that are available to Stage. New
// devices must be added here.

libitem_t library_items[] = { 
  { "root", "black", (CFP)CRootDevice::Creator},
  { "box", "blue", (CFP)CEntity::Creator},
  /*
  { "bitmap", "black", (CFP)CBitmap::Creator},
  { "laser", "blue", (CFP)CLaserDevice::Creator},
  { "position", "red", (CFP)CPositionDevice::Creator},
  { "sonar", "green", (CFP)CSonarDevice::Creator},
  { "box", "yellow", (CFP)CBox::Creator},
  { "gps", "gray", (CFP)CGpsDevice::Creator},
  { "gripper", "blue", (CFP)CGripperDevice::Creator},
  { "idar", "DarkRed", (CFP)CIdarDevice::Creator},
  { "idarturret", "DarkRed", (CFP)CIdarTurretDevice::Creator},
  { "lbd", "gray", (CFP)CFiducialFinder::Creator},
  { "fiducialfinder", "gray", (CFP)CFiducialFinder::Creator},
  { "mote", "orange", (CFP)CMoteDevice::Creator},
  { "power", "wheat", (CFP)CPowerDevice::Creator},
  { "ptz", "magenta", (CFP)CPtzDevice::Creator},
  { "puck", "green", (CFP)CPuck::Creator},
  { "truth", "purple", (CFP)CTruthDevice::Creator},
  { "vision", "gray", (CFP)CVisionDevice::Creator},
  { "blobfinder", "gray", (CFP)CVisionDevice::Creator},
  { "broadcast", "brown", (CFP)CBroadcastDevice::Creator},
  { "bumper", "LightBlue", (CFP)CBumperDevice::Creator},
  { "regularmcl", "blue", (CFP)CRegularMCLDevice::Creator},
  // { "bps", BpsType, (CFP)CBpsDevice::Creator},
  */

  {NULL, NULL, NULL } // marks the end of the array
};  


// statically allocate a libray filled with the entries above
Library model_library( library_items );

//#define DEBUG

// LibraryItem //////////////////////////////////////////////////////////////

int LibraryItem::type_count = 0;

LibraryItem::LibraryItem( const char* token, 
			  const char* colorname, 
			  CreatorFunctionPtr creator_func )
{
  assert( token );
  assert( strlen(token) > 0 );
  assert( colorname ); 
  assert( creator_func );
  
  this->token = token;
  this->color = ::LookupColor( colorname ); // convert to StageColor (RGB)
  this->creator_func = creator_func;
  this->type_num = LibraryItem::type_count++;
  this->prev = this->next = NULL;
}


LibraryItem* LibraryItem::FindLibraryItemFromToken( char* token )
{
  if( token == this->token ) return this; // same pointer!
  if( strcmp( token, this->token) == 0 ) return this; // same string!
  
  // try the next item in the list
  if( next ) return next->FindLibraryItemFromToken( token );
  
  // fail! token not found here or later in the list
  PRINT_WARN1( "Failed to find token %s in library\n", token );
  return NULL;
}

CreatorFunctionPtr LibraryItem::FindCreatorFromToken( char* token )
{
  if( strcmp( token, this->token) == 0 ) return this->creator_func;
  
  // try the next item in the list
  if( next ) return next->FindCreatorFromToken( token );
  
  // fail! token not found here or later in the list
  PRINT_WARN1( "failed to find creator of token %s in library\n", token );
  return NULL;
}

int LibraryItem::FindTypeNumFromToken( char* token )
{
  if( strcmp( token, this->token) == 0 ) return this->type_num;
  
  // try the next item in the list
  if( next ) return next->FindTypeNumFromToken( token );
  
  // fail! token not found here or later in the list
  PRINT_WARN1( "failed to find type_num of token %s in library\n", token );
  return -1;
}

const char* LibraryItem::FindTokenFromCreator( CreatorFunctionPtr cfp )
{
  if( cfp == this->creator_func ) return this->token;
  
  // try the next item in the list
  if( next ) return next->FindTokenFromCreator( cfp );
  
  // fail! token not found here or later in the list
  PRINT_WARN1( "failed to find token of creator %p in library\n", cfp );
  return NULL;
}

// Library //////////////////////////////////////////////////////////////

// constructor
Library::Library( void )
{
  // TODO - fix this with dynamic allocation
  memset( entPtrs, 0, 10000 * sizeof(CEntity*) );
}

// constructor from null-terminated array
Library::Library( const libitem_t item_array[] )
{
  //PRINT_DEBUG( "Building libray from array\n" );

  int type=1;
  for( libitem_t* item = (libitem_t*)&(item_array[0]);
       item->token; 
       item++ )
    {
      //PRINT_DEBUG3( "%s %d %p\n", item->token, item->type, item->fp );
      this->AddDevice( (char*)item->token, item->colorstr, item->fp );
      type++;
    }
}


// add a device type to the library
void Library::AddDevice( const char* token, const char* colorstr, CreatorFunctionPtr creator )
{
  LibraryItem* item = new LibraryItem( token, colorstr, creator );

  // check to make sure that this item isn't already there
  //if( liblist && liblist->FindCreatorFromType( type ) != NULL )
  //PRINT_ERR2( "attempting to add a duplicate type %d:%s to library",
  //	type, token );
  
  //printf(  "Registering %d:%s %p in library\n",
  //   type, token, creator );

  // store the new item in our list
  STAGE_LIST_APPEND( liblist, item );
}

void Library::StoreEntPtr( int id, CEntity* ent )
{
  // TODO - extend this with dynamic storage
  // store the pointer in our array
  entPtrs[id] = ent;
}   

// create an instance of an entity given a worldfile token
CEntity* Library::CreateEntity( stage_model_t* model )
{
  assert( model );

  // if an entity exists with this id, we zap the old one.
  if( CEntity* extant = entPtrs[model->id] )
    {  
      PRINT_DEBUG3(  "Removing extant model with ID %d (%s) at %p",
		     extant->stage_id, extant->lib_entry->token, extant );
      extant->Shutdown();
      delete extant;
      entPtrs[model->id] = NULL;
    }
  
  // now we create the replacement.

  // look up this token in the library
  LibraryItem* libit = liblist->FindLibraryItemFromToken( model->token );

  CEntity* ent;
  
  if( libit )
    {
      assert( libit->creator_func );
      
      // if it has a valid parent, look up the parent's address
      CEntity* parentPtr = NULL;
      if( model->parent_id != -1 ) parentPtr = entPtrs[model->parent_id]; 
      
      // create an entity through the creator callback fucntion  
      // which calls the constructor
      ent = (*libit->creator_func)( libit, model->id, parentPtr ); 
     
      // need to do some startup outside the constructor to allow for
      // full polymorphism
      if( !ent->Startup() ) // if startup fails
	{
	  PRINT_WARN3( "Startup failed for model %d (%s) at %p",
		      model->id, model->token, ent );
	  delete ent;
	  ent = NULL;
	}

      PRINT_DEBUG3(  "Startup successful for model %d (%s) at %p",
		     model->id, model->token, ent );
    }
  else
    {
      PRINT_WARN1( "Client requested model '%s' is not in the library",
		   model->token );
      ent = NULL;
    }
  
  StoreEntPtr( model->id, ent );
  return ent; // NULL if failed
} 
  
  
const char* Library::TokenFromCreator( CreatorFunctionPtr cfp )
{ 
  //printf( "token from creator %p\n", cfp );

  assert( liblist );
  assert( cfp );
  return( liblist->FindTokenFromCreator( cfp ) ); 
}

int Library::TypeNumFromToken( char* token )
{ 
  printf( "type_num from token %s\n", token );

  assert( liblist );
  return( liblist->FindTypeNumFromToken( token ) ); 
}

LibraryItem* Library::LibraryItemFromToken( char* token )
{ 
  //printf( "library item from token %s\n", token );

  assert( liblist );
  return( liblist->FindLibraryItemFromToken( token ) ); 
}



void Library::Print( void )
{
  //printf("\n[Library contents:");
  
  for( LibraryItem* it = liblist; it; it = it->next )
    printf( "\n\t%s:%p:%d", it->token, it->creator_func, it->type_num );
  
  puts( "]" );
}

// C wrapper 
int CreateEntityFromLibrary( stage_model_t* model )
{
  // create an entity. return success on getting a valid pointer
  return( model_library.CreateEntity( model ) ? 0 : -1);
}

int Update( double simtime )
{
  CEntity::root->Update( simtime );
  return 0;
}



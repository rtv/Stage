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
 * must register a worldfile token, type number and a creator function
 * (usually a static wrapper for a constructor) with the library. Just
 * add your device to the static table below.
 *
 * Author: Richard Vaughan Date: 27 Oct 2002 (this header added) 
 * CVS info: $Id: library.cc,v 1.4 2002-10-31 07:48:35 gerkey Exp $
 */

#include "library.hh"

#include "models/bitmap.hh"
#include "models/box.hh"
#include "models/broadcastdevice.hh"
#include "models/descartesdevice.hh"
#include "models/gpsdevice.hh"
#include "models/gripperdevice.hh"
#include "models/idardevice.hh"
#include "models/idarturretdevice.hh"
#include "models/laserbeacondevice.hh"
#include "models/laserbeacon.hh"
#include "models/laserdevice.hh"
#include "models/motedevice.hh"
#include "models/omnipositiondevice.hh"
#include "models/positiondevice.hh"
#include "models/powerdevice.hh"
#include "models/ptzdevice.hh"
#include "models/puck.hh"
#include "models/sonardevice.hh"
#include "models/truthdevice.hh"
#include "models/visionbeacon.hh"
#include "models/visiondevice.hh"

typedef CreatorFunctionPtr CFP;

// this array defines the models that are available to Stage. New
// devices must be added here.

libitem_t library_items[] = { 
  { "bitmap", BitmapType, (CFP)CBitmap::Creator},
  { "box", BoxType, (CFP)CBox::Creator},
  { "descartes", DescartesType, (CFP)CDescartesDevice::Creator},
  { "gps", GpsType, (CFP)CGpsDevice::Creator},
  { "gripper", GripperType, (CFP)CGripperDevice::Creator},
  { "idar", IdarType, (CFP)CIdarDevice::Creator},
  { "idarturret", IdarTurretType, (CFP)CIdarTurretDevice::Creator},
  { "laser", LaserTurretType,(CFP)CLaserDevice::Creator},
  { "laserbeacon",LaserBeaconType, (CFP)CLaserBeacon::Creator},
  { "lbd", LbdType, (CFP)CLBDDevice::Creator},
  { "mote", MoteType, (CFP)CMoteDevice::Creator},
  { "omniposition", OmniPositionType, (CFP)COmniPositionDevice::Creator},
  { "position", PositionType, (CFP)CPositionDevice::Creator},
  { "power", PowerType, (CFP)CPowerDevice::Creator},
  { "ptz", PtzType, (CFP)CPtzDevice::Creator},
  { "puck", PuckType, (CFP)CPuck::Creator},
  { "sonar", SonarType, (CFP)CSonarDevice::Creator},
  { "truth", TruthType, (CFP)CTruthDevice::Creator},
  { "vision", VisionType, (CFP)CVisionDevice::Creator},
  { "broadcast", BroadcastType, (CFP)CBroadcastDevice::Creator},
  // { "visionbeacon", VisionBeaconType, (CFP)CVisionBeacon::Creator},
  // { "bps", BpsType, (CFP)CBpsDevice::Creator},
  {NULL, NUMBER_OF_STAGE_TYPES, NULL } // marks the end of the array
};  


// statically allocate a libray filled with the entries above
Library model_library( library_items );

#define DEBUG

// LibraryItem //////////////////////////////////////////////////////////////

LibraryItem::LibraryItem( char* token, StageType type, 
			  CreatorFunctionPtr creator_func )
{
  assert( token );
  assert( creator_func );
  assert( type > 0 );
  assert( type < NUMBER_OF_STAGE_TYPES );
  assert( strlen(token) > 0 );
  
  strncpy( this->token, token, STAGE_MAX_TOKEN_LEN );
  this->creator_func = creator_func;
  this->type  = type;
  this->prev = this->next = NULL;
}

CreatorFunctionPtr LibraryItem::FindCreatorFromType( StageType type )
{
  if( type == this->type ) return this->creator_func;
  
  // try the next item in the list
  if( next ) return next->FindCreatorFromType( type );
  
  // fail! type not found here or later in the list
  printf( "failed to find creator of type %d in library\n", type );
  return NULL;
}


CreatorFunctionPtr LibraryItem::FindCreatorFromToken( char* token )
{
  if( strcmp( token, this->token) == 0 ) return this->creator_func;
  
  // try the next item in the list
  if( next ) return next->FindCreatorFromToken( token );
  
  // fail! token not found here or later in the list
  printf( "failed to find creator of token %s in library\n", token );
  return NULL;
}

StageType LibraryItem::FindTypeFromToken( char* token )
{
  if(  strcmp( token, this->token) == 0 ) return this->type;
  
  // try the next item in the list
  if( next ) return next->FindTypeFromToken( token );
  
  // fail! token not found here or later in the list
  printf( "failed to find type of token %s in library\n", token );
  return NullType;
}

char* LibraryItem::FindTokenFromType( StageType type )
{
  if( type == this->type ) return this->token;
  
  // try the next item in the list
  if( next ) return next->FindTokenFromType( type );
  
  // fail! type not found here or later in the list
  printf( "failed to find token of type %d in library\n", type );
  return NULL;
}

// Library //////////////////////////////////////////////////////////////

// constructor
Library::Library( void )
{
}

// constructor from null-terminated array
Library::Library( const libitem_t item_array[] )
{
  printf( "Building libray from array\n" );
  //PRINT_DEBUG( "Building libray from array\n" );

  for( libitem_t* item = (libitem_t*)&(item_array[0]);
       item->token; 
       item++ )
    {
      printf( "%s %d %p\n", item->token, item->type, item->fp );
      //PRINT_DEBUG3( "%s %d %p\n", item->token, item->type, item->fp );
      this->AddDeviceType( (char*)item->token, item->type, (void*)item->fp );
    }
}


// add a device type to the library
void Library::AddDeviceType( char* token, StageType type, void* creator )
{
  LibraryItem* item = new LibraryItem( token, type, 
				       (CreatorFunctionPtr)creator );

  // check to make sure that this item isn't already there
  //if( liblist && liblist->FindCreatorFromType( type ) != NULL )
  //PRINT_ERR2( "attempting to add a duplicate type %d:%s to library",
  //	type, token );
  
  //printf(  "Registering %d:%s %p in library\n",
  //   type, token, creator );

  // store the new item in our list
  STAGE_LIST_APPEND( liblist, item );
}


// create an instance of an entity given a worldfile token
CEntity* Library::CreateEntity( char* token, CWorld* world_ptr, CEntity* parent_ptr )
{
  assert( token );
  assert( world_ptr );
  // parent may be a null ptr
  
  // look up the creator fucntion
  CreatorFunctionPtr cfp = liblist->FindCreatorFromToken( token );
  assert( cfp );
  
  return( CreateEntity( cfp,
			world_ptr, 
			parent_ptr ) );
} 
  
// create an instance of an entity given a StageType number
CEntity* Library::CreateEntity( StageType type, CWorld* world_ptr, CEntity* parent_ptr )
{
  assert( type > 0 );
  assert( type < NUMBER_OF_STAGE_TYPES );
  assert( world_ptr );
  // parent may be a null ptr
  
  // look up the creator fucntion
  CreatorFunctionPtr cfp = liblist->FindCreatorFromType( type );
  assert( cfp );
  
  return( CreateEntity( cfp,
			world_ptr, 
			parent_ptr ) );
}
  
// create an instance of an entity given a pointer to a named constructor
CEntity* Library::CreateEntity( CreatorFunctionPtr cfp, CWorld* world_ptr, CEntity* parent_ptr )
{
  if( cfp == NULL )
    return NULL; // failed to create an entity
  else
    return (*cfp)( world_ptr, parent_ptr ); // create an entity through this callback  
} 
  
const char* Library::TokenFromType( StageType t )
{ 
  printf( "token from type %d\n", t );

  assert( liblist );
  assert( t > 0 );
  assert( t < NUMBER_OF_STAGE_TYPES );
  return( liblist->FindTokenFromType( t ) ); 
}

const StageType Library::TypeFromToken( char* token )
{ 
  assert( liblist );
  assert( token );
  return( liblist->FindTypeFromToken( token ) ); 
}

void Library::Print( void )
{
  printf("\n[Library contents:");
  
  for( LibraryItem* it = liblist; it; it = it->next )
    printf( "\n\t%d:%s", it->type, it->token );
  
  puts( "]" );
}



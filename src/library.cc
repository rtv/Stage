#include "library.hh"

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



#include "library.hh"

// constructor
Library::Library( void )
{
}

// add a device type to the library
void Library::AddDeviceType( string token, StageType type, void* creator )
{
  // store the device info mappings
  tokencreator_map[ token ] = creator;
  typecreator_map[ type ] = creator;
  tokentype_map[ token ] = type;
  typetoken_map[ type ] = token;
}

// create an instance of an entity given a worldfile token
CEntity* Library::CreateEntity( string token, CWorld* world_ptr, CEntity* parent_ptr )
{
  return( CreateEntity( (CreatorFunctionPtr)(this->tokencreator_map[ token ] ), 
			world_ptr, 
			parent_ptr ) );
} 
  
// create an instance of an entity given a StageType number
CEntity* Library::CreateEntity( StageType type, CWorld* world_ptr, CEntity* parent_ptr )
{
  return( CreateEntity( (CreatorFunctionPtr)(this->typecreator_map[ type ] ), 
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
  
const char* Library::StringFromType( StageType t )
{ 
  return( typetoken_map[ t ].c_str() ); 
}
  
const StageType Library::TypeFromString( char* token )
{ 
  string str( token );
  return( tokentype_map[ str ] ); 
}

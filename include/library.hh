
#ifndef _LIBRARY_HH
#define _LIBRARY_HH

class CWorld;
class CEntity;

#include "stage_types.hh"

// pointer to a function that returns a new  entity
// given a world and a parent
typedef CEntity*(*CreatorFunctionPtr)( CWorld* world, CEntity* parent );

#include <string>
#include <map>

using namespace std;

// store the info for an entity type
//typedef struct
//{
//StageType type;
//void* creator;
//string color;
//string token;
//} LibraryEntry_t;

typedef map< string, void* > TokenCreatorMap;  
typedef map< StageType, void* > TypeCreatorMap;  
typedef map< string, StageType > TokenTypeMap;  
typedef map< StageType, string > TypeTokenMap;  

// and a function that creates a new entity of that type. 

// Each entity type should declare a static function that returns a
// pointer to a new instance of that entity (given a world and a
// parent).
//
// here is an example function definition from positiondevice.hh:
//
///  public: static CPositionDevice* Creator( CWorld *world, CEntity *parent )
//   {
//      return( new CPositionDevice( world, parent ) );
//   }
//

class Library
{
private:
  // store allmappings between worldfile tokens, stage type number and
  // creator functions in associative arrays.  By storing them
  // redundantly, we get fast lookup later
  TokenCreatorMap tokencreator_map;
  TypeCreatorMap typecreator_map;
  TokenTypeMap tokentype_map;
  TypeTokenMap typetoken_map;
  
public:
  // constructor
  Library( void );
  
  void AddDeviceType( string token, StageType type, void* creator );

  // create an instance of an entity given a worldfile token
  CEntity* CreateEntity( string token, CWorld* world_ptr, CEntity* parent_ptr );
  
  // create an instance of an entity given a type number
  CEntity* CreateEntity( StageType type, CWorld* world_ptr, CEntity* parent_ptr );
  
  // create an instance of an entity given a pointer to a named constructor
  CEntity* CreateEntity( CreatorFunctionPtr cfp, CWorld* world_ptr, CEntity* parent_ptr );
  
  // get a worldfile token from a type and vice versa
  const char* StringFromType( StageType t );
  const StageType TypeFromString( char* token );
  
};


#endif

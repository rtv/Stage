
#ifndef _LIBRARY_HH
#define _LIBRARY_HH

#include <map>
//#include <string>

#include "stage_types.hh"
#include "stage_types.hh"

class CWorld;
class CEntity;


// ADDING A DEVICE TO THE LIBRARY
//
// Each entity class should declare a static function that returns a
// pointer to a new instance of that entity, given a world and a
// parent.
//
// here is an example function definition from positiondevice.hh:
//
///  public: static CPositionDevice* Creator( CWorld *world, CEntity *parent )
//   {
//      return( new CPositionDevice( world, parent ) );
//   }
//

class LibraryItem
{
public:
  LibraryItem( char* token, StageType type, CreatorFunctionPtr creator_func );
  
  CreatorFunctionPtr FindCreatorFromType( StageType type );
  CreatorFunctionPtr FindCreatorFromToken( char* token );
  StageType FindTypeFromToken( char* token );
  char* FindTokenFromType( StageType type );
  
  char token[STAGE_MAX_TOKEN_LEN];
  CreatorFunctionPtr creator_func;
  StageType type;
  
  // list-enabling pointers for use with STAGE_LIST_X macros
  LibraryItem *prev, *next;
};


class Library
{
private:
  // a list of [type,token,creator] mappings
  LibraryItem* liblist;
  
public:
  // constructor
  Library( void );
  Library( const libitem_t items[] );
  
  void AddDeviceType( char* token, StageType type, void* creator );

  // create an instance of an entity given a worldfile token
  CEntity* CreateEntity( char* token, CWorld* world_ptr, CEntity* parent_ptr );
  
  // create an instance of an entity given a type number
  CEntity* CreateEntity( StageType type, CWorld* world_ptr, CEntity* parent_ptr );
  
  // create an instance of an entity given a pointer to a named constructor
  CEntity* CreateEntity( CreatorFunctionPtr cfp, CWorld* world_ptr, CEntity* parent_ptr );
  
  // get a worldfile token from a type and vice versa
  const char* TokenFromType( StageType t );
  const StageType TypeFromToken( char* token );
  

  void Print();
};


#endif

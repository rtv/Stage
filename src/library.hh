
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
  LibraryItem( const char* token, const char* colorstr, CreatorFunctionPtr creator_func );
  
  CreatorFunctionPtr FindCreatorFromToken( char* token );
  int  FindTypeNumFromToken( char* token );
  const char* FindTokenFromCreator( CreatorFunctionPtr cfp );
  LibraryItem* FindLibraryItemFromToken( char* token );

  const char* token;//[STAGE_TOKEN_MAX];
  CreatorFunctionPtr creator_func;
  int type_num; // each library entry has a unique type number
  
  StageColor color;

  //char* SetToken( char* token )
  //{ return( strncpy( token, this->token, STAGE_TOKEN_MAX ) ); }
  
  // this is used to set the type number in the constructor, where it
  // is incremented in each use.
  static int type_count;

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
  
  void AddDevice( const char* token, const char* colorstr, CreatorFunctionPtr creator );

  // create an instance of an entity given a worldfile token
  CEntity* CreateEntity( char* token, CWorld* world_ptr, CEntity* parent_ptr );
  
  // create an instance of an entity given a pointer to a named constructor
  //CEntity* CreateEntity( CreatorFunctionPtr cfp, CWorld* world_ptr, CEntity* parent_ptr );
  
  const char* TokenFromCreator( CreatorFunctionPtr cfp );

  int TypeNumFromToken( char* token );
  LibraryItem* LibraryItemFromToken( char* token );
  void Print();
};


#endif


#ifndef _LIBRARY_HH
#define _LIBRARY_HH

#include <map>
//#include <string>

#include "stage.h"
#include "colors.hh"

class CEntity;
class LibraryItem;

// pointer to a function that returns a new  entity
typedef CEntity*(*CreatorFunctionPtr)( LibraryItem *libit, 
				       int id, 
				       CEntity *parent );

typedef struct libitem
{
  const char* token;
  //StageType type;
  const char* colorstr;
  CreatorFunctionPtr fp;
} libitem_t;


// ADDING A DEVICE TO THE LIBRARY
//
// Each entity class should declare a static function that returns a
// pointer to a new instance of that entity, given a unique int id and and a
// pointer to a parent.
//
// here is an example function definition from positiondevice.hh:
//
///  public: static CPositionDevice* Creator( int id, CEntity *parent )
//   {
//      return( new CPositionDevice( id, parent ) );
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
  
  // TODO - make this length adaptive
  CEntity* entPtrs[ 10000 ];
  void StoreEntPtr( int id, CEntity* ent );

  void AddDevice( const char* token, const char* colorstr, CreatorFunctionPtr creator );
  
  // create an instance of an entity given a request
  // returns a unique id for the entity, which happens to also be valid pointer to the
  // entity object
  CEntity* CreateEntity( stage_model_t* model );
  
  const char* TokenFromCreator( CreatorFunctionPtr cfp );

  int TypeNumFromToken( char* token );
  LibraryItem* LibraryItemFromToken( char* token );
  void Print();
};

 
#endif


#ifndef _LIBRARY_HH
#define _LIBRARY_HH

#include "stage.h"

// TODO - make this dynamic
//#define STG_MAX_MODELS 1000

class CEntity;

// pointer to a function that returns a new  entity
typedef CEntity*(*CreatorFunctionPtr)( int id, char* token, char* color,
				       CEntity *parent );

typedef CreatorFunctionPtr CFP;

typedef struct libitem
{
  const char* token;
  const char* colorstr;
  CreatorFunctionPtr fp;
} stage_libitem_t;

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

class Library
{
private:
  // an array of [token,color,creator_function] structures
  // the position in the array is used as a type number
  const stage_libitem_t* items;
  int item_count;

  // an array of pointers to entities
  // used to map id (array index) to an entity
  CEntity** entPtrs;
  
public:
  // constructor
  Library( const stage_libitem_t items[] );
  
  int model_count; //keep track of how many models we have created
  
  // print the items on stdout
  void Print(void);

  // returns a pointer the matching item, or NULL if none is found
  stage_libitem_t* FindItemWithToken( const stage_libitem_t* items, 
				      int count, char* token );
  
  // as above, but uses data members as arguments
  stage_libitem_t* Library::FindItemWithToken( char* token  )
  { return FindItemWithToken( this->items, this->item_count, token ); };
  
  void StoreEntPtr( int id, CEntity* ent );
  //CEntity* GetEntPtr( int id ){ return( entPtrs[id] ); };
  
  void AddDevice( const char* token, const char* colorstr, CreatorFunctionPtr creator );
  
  // create an instance of an entity given a request returns a unique
  // id for the entity, which happens to be it's index in the array
  stage_id_t CreateEntity( stage_model_t* model );
  
  // remove an entity and its children from the sim
  int DestroyEntity( stage_model_t* model );
  
  // put the root object at the start of the array -
  // warning -  this destroys the rest of the array, so should
  // only be called from root's contructor
  void InsertRoot( CEntity* root );

  CEntity* GetEntFromId( int id )
  {
    return entPtrs[id];
  };
};

 
#endif

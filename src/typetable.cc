#include "model.hh"

// declare some factory functions that create model objects. We use
// pointers to these functions to map constructors to strings in the
// type table.

StgModel* new_model( stg_world_t* world, StgModel* parent, 
		     stg_id_t id, CWorldFile* wf )
{ return (StgModel*)new StgModel( world, parent, id, wf ); }

StgModel* new_laser( stg_world_t* world, StgModel* parent, 
		     stg_id_t id, CWorldFile* wf )
{ return (StgModel*)new StgModelLaser( world, parent, id, wf ); }

StgModel* new_position( stg_world_t* world, StgModel* parent, 
		     stg_id_t id, CWorldFile* wf )
{ return (StgModel*)new StgModelPosition( world, parent, id, wf ); }


typedef struct 
{
  const char* token;
  StgModel* (*creator_fn)( stg_world_t*, StgModel*, stg_id_t, CWorldFile* );
} stg_typetable_entry_t;


// ******************************
// Register new model types here
// Each entry maps a worldfile keywords onto a model initialization function

stg_typetable_entry_t typetable[] = {
  { "model",     new_model },
  { "laser",     new_laser },
  { "position",  new_position },
  { NULL, NULL } // this must be the last entry
};

// ******************************

// generate a hash table from the typetable array
GHashTable* stg_create_typetable( void )
{
  GHashTable* table = g_hash_table_new( g_str_hash, g_str_equal );
  
  for( stg_typetable_entry_t* ent = typetable;
       ent->token;
       ent++ )  
    g_hash_table_insert( table, (void*)(ent->token), (void*)(ent->creator_fn) );
  
  return table;
}



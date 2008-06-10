#include "stage_internal.hh"

// ******************************
// Register new model types here
// Each entry maps a worldfile keyword onto a model constructor wrapper

static stg_typetable_entry_t typearray[] = {
	{ "model",      StgModel::Create },
	{ "laser",      StgModelLaser::Create },
	{ "position",   StgModelPosition::Create },
	{ "ranger",     StgModelRanger::Create },
	{ "fiducial",   StgModelFiducial::Create },
	{ "blobfinder", StgModelBlobfinder::Create },
	{ "blinkenlight", StgModelBlinkenlight::Create },
	{ "camera", StgModelCamera::Create },
	{ NULL, NULL } // this must be the last entry
};

// ******************************
// generate a hash table from the typetable array
GHashTable* Stg::stg_create_typetable( void )
{
	GHashTable* table = g_hash_table_new( g_str_hash, g_str_equal );

	for( stg_typetable_entry_t* ent = typearray;
			ent->token;
			ent++ )  
		g_hash_table_insert( table, (void*)(ent->token), (void*)(ent->creator_fn) );

	return table;
}

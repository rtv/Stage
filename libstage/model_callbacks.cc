#include "stage_internal.hh"

int key_gen( Model* mod, void* address )
{
	return ((int*)address) - ((int*)mod);
}

void Model::AddCallback( void* address, 
		stg_model_callback_t cb, 
		void* user )
{
	int* key = (int*)g_new( int, 1 );
	*key = key_gen( this, address );

	GList* cb_list = (GList*)g_hash_table_lookup( callbacks, key );

	//printf( "installing callback in model %s with key %d\n",
	//  mod->token, *key );

	// add the callback & argument to the list
	cb_list = g_list_prepend( cb_list, cb_create( cb, user ) );

	// and replace the list in the hash table
	g_hash_table_insert( callbacks, key, cb_list );

	// if the callback was an update function, add this model to the
	// world's list of models that need updating (if it's not there
	// already)
	//if( address == (void*)&update )
	//stg_world_start_updating_model( world, this );
}


int Model::RemoveCallback( void* member,
		stg_model_callback_t callback )
{
	int key = key_gen( this, member );

	GList* cb_list = (GList*)g_hash_table_lookup( callbacks, &key );

	// find our callback in the list of stg_cbarg_t
	GList* el = NULL;

	// scan the list for the first matching callback
	for( el = g_list_first( cb_list );
			el;
			el = el->next )
	{
		if( ((stg_cb_t*)el->data)->callback == callback )
			break;
	}

	if( el ) // if we found the matching callback, remove it
	{
		//puts( "removed callback" );

		cb_list = g_list_remove( cb_list, el->data);

		// store the new, shorter, list of callbacks
		g_hash_table_insert( callbacks, &key, cb_list );

		// we're done with that
		//free( el->data );
		// TODO - fix leak of cb_t

		// if we just removed a model's last update callback,
		// remove this model from the world's update list
		//if( (member == (void*)&update) && (cb_list == NULL) )
		//stg_world_stop_updating_model( world, this );
	}
	else
	{
		//puts( "callback was not installed" );
	}

	return 0; //ok
}


void Model::CallCallbacks(  void* address )
{
	assert( address );

	int key = key_gen( this, address );

	//printf( "Model %s has %d callbacks. Checking key %d\n", 
	//  mod->token, g_hash_table_size( mod->callbacks ), key );

	GList* cbs = (GList*)g_hash_table_lookup( callbacks, &key );

	//printf( "key %d has %d callbacks registered\n",
	//  key, g_list_length( cbs ) );

	// maintain a list of callbacks that should be cancelled
	GList* doomed = NULL;

	// for each callback in the list
	while( cbs )
	{  
		stg_cb_t* cba = (stg_cb_t*)cbs->data;

		//puts( "calling..." );

		if( (cba->callback)( this, cba->arg ) )
		{
			//printf( "callback returned TRUE - schedule removal from list\n" );
			doomed = g_list_prepend( doomed, (void*)cba->callback );
		}
		else
		{
			//printf( "callback returned FALSE - keep in list\n" );
		}

		cbs = cbs->next;
	}      

	if( doomed ) 	// delete all the callbacks that returned TRUE    
	{
		//printf( "removing %d doomed callbacks\n", g_list_length( doomed ) );

		for( ; doomed ; doomed = doomed->next )
			this->RemoveCallback( address, (stg_model_callback_t)doomed->data );

		g_list_free( doomed );
	}

}



#include "stage_internal.h"

void stg_model_add_callback( stg_model_t* mod, 
			     void* address, 
			     stg_model_callback_t cb, 
			     void* user )
{
  int* key = malloc(sizeof(int));
  *key = address - (void*)mod;

  GList* cb_list = g_hash_table_lookup( mod->callbacks, key );

  //printf( "installing callback in model %s with key %d\n",
  //  mod->token, *key );
  
  // add the callback & argument to the list
  cb_list = g_list_prepend( cb_list, cb_create( cb, user ) );
  
  // and replace the list in the hash table
  g_hash_table_insert( mod->callbacks, key, cb_list );

  // if the callback was an update function, add this model to the
  // world's list of models that need updating (if it's not there
  // already)
  if( address == (void*)&mod->update )
    stg_world_start_updating_model( mod->world, mod );
}


int stg_model_remove_callback( stg_model_t* mod,
			       void* member,
			       stg_model_callback_t callback )
{
  int key = member - (void*)mod;

  GList* cb_list = g_hash_table_lookup( mod->callbacks, &key );
 
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
      g_hash_table_insert( mod->callbacks, &key, cb_list );

      // we're done with that
      //free( el->data );
      // TODO - fix leak of cb_t

      // if we just removed a model's last update callback,
      // remove this model from the world's update list
      if( (member == (void*)&mod->update) && (cb_list == NULL) )
	stg_world_stop_updating_model( mod->world, mod );
    }
  else
    {
      //puts( "callback was not installed" );
    }
 
  return 0; //ok
}


void model_call_callbacks( stg_model_t* mod, void* address )
{
  assert( mod );
  assert( address );
  assert( address > (void*)mod );
  assert( address < (void*)( mod + sizeof(stg_model_t)));
  
  int key = address - (void*)mod;
  
  //printf( "Model %s has %d callbacks. Checking key %d\n", 
  //  mod->token, g_hash_table_size( mod->callbacks ), key );
  
  GList* cbs = g_hash_table_lookup( mod->callbacks, &key ); 
  
  //printf( "key %d has %d callbacks registered\n",
  //  key, g_list_length( cbs ) );
  
  // maintain a list of callbacks that should be cancelled
  GList* doomed = NULL;

  // for each callback in the list
  while( cbs )
    {  
      stg_cb_t* cba = (stg_cb_t*)cbs->data;
      
      //puts( "calling..." );
      
      if( (cba->callback)( mod, cba->arg ) )
	{
	  //printf( "callback returned TRUE - schedule removal from list\n" );
	  doomed = g_list_prepend( doomed, cba->callback );
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
	stg_model_remove_callback( mod, address, (stg_model_callback_t)doomed->data );
      
      g_list_free( doomed );      
    }

}


/* wrappers for the generic callback add & remove functions that hide
   some implementation detail */

void stg_model_add_update_callback( stg_model_t* mod, 
				    stg_model_callback_t cb, 
				    void* user )
{
  stg_model_add_callback( mod, &mod->update, cb, user );
}

void stg_model_remove_update_callback( stg_model_t* mod, 
				       stg_model_callback_t cb )
{
  stg_model_remove_callback( mod, &mod->update, cb );
}

void stg_model_add_load_callback( stg_model_t* mod, 
				    stg_model_callback_t cb, 
				    void* user )
{
  stg_model_add_callback( mod, &mod->load, cb, user );
}

void stg_model_remove_load_callback( stg_model_t* mod, 
				       stg_model_callback_t cb )
{
  stg_model_remove_callback( mod, &mod->load, cb );
}

void stg_model_add_save_callback( stg_model_t* mod, 
				    stg_model_callback_t cb, 
				    void* user )
{
  stg_model_add_callback( mod, &mod->save, cb, user );
}

void stg_model_remove_save_callback( stg_model_t* mod, 
				       stg_model_callback_t cb )
{
  stg_model_remove_callback( mod, &mod->save, cb );
}

void stg_model_add_startup_callback( stg_model_t* mod, 
				    stg_model_callback_t cb, 
				    void* user )
{
  stg_model_add_callback( mod, &mod->startup, cb, user );
}

void stg_model_remove_startup_callback( stg_model_t* mod, 
				       stg_model_callback_t cb )
{
  stg_model_remove_callback( mod, &mod->startup, cb );
}

void stg_model_add_shutdown_callback( stg_model_t* mod, 
				    stg_model_callback_t cb, 
				    void* user )
{
  stg_model_add_callback( mod, &mod->shutdown, cb, user );
}

void stg_model_remove_shutdown_callback( stg_model_t* mod, 
				       stg_model_callback_t cb )
{
  stg_model_remove_callback( mod, &mod->shutdown, cb );
}


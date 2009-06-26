#include "stage.hh"
using namespace Stg;


void Model::AddCallback( void* address, 
						 stg_model_callback_t cb, 
						 void* user )
{
//   // treat update callbacks specially as they are so frequently called
//   if( address == &hooks.update )
// 	{
// 	  // a dedicated vector avoids a hash table lookup 
// 	  update_callbacks.push_back( std::pair<stg_model_callback_t,void*>( cb, user ) );	  
// 	}
//   else
	{
	  GList* cb_list = (GList*)g_hash_table_lookup( callbacks, address );
	  
	  //printf( "installing callback in model %s with key %p\n",
	  //		  token, address );
	  
	  // add the callback & argument to the list
	  cb_list = g_list_prepend( cb_list, new stg_cb_t( cb, user ) );
	  
	  // and replace the list in the hash table
	  g_hash_table_insert( callbacks, address, cb_list );
	}
}


int Model::RemoveCallback( void* member,
		stg_model_callback_t callback )
{

  // XX TODO
  //   if( address == &hooks.update )
  // 	{
  // 	  std::erase( std::remove( update_callbacks.begin(); update_callbacks.end(), 

	GList* cb_list = (GList*)g_hash_table_lookup( callbacks, member );

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
		g_hash_table_insert( callbacks, member, cb_list );

		// we're done with the stored data
		delete (stg_cb_t*)(el->data);
	}
	else
	{
		//puts( "callback was not installed" );
	}
	
	// return the number of callbacks now in the list. Useful for
	// detecting when the list is empty.
	return g_list_length( cb_list );
}


void Model::CallCallbacks(  void* address )
{

	assert( address );
	
// 	// avoid a hash table lookup for 
// 	if( address == &hooks.update )
// 	  {
// 		FOR_EACH( it, update_callbacks )
// 		  (it->first)( this, it->second );	  

// 		return;
// 	  }


	//int key = key_gen( this, address );
	
	//printf( "CallCallbacks for model %s %p key %p\n", this->Token(), this, address );

	//printf( "Model %s has %d callbacks. Checking key %d\n", 
	//		  this->token, g_hash_table_size( this->callbacks ), key );
	
	GList* cbs = (GList*)g_hash_table_lookup( callbacks, address );
	
	//printf( "key %p has %d callbacks registered\n",
	//	  address, g_list_length( cbs ) );
	
	// maintain a list of callbacks that should be cancelled
	GList* doomed = NULL;

	// for each callback in the list
	while( cbs )
	{  
	  //printf( "cbs %p data %p cvs->next %p\n", cbs, cbs->data, cbs->next );

		stg_cb_t* cba = (stg_cb_t*)cbs->data;
		assert( cba );

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

	//puts( "Callbacks done" );
}



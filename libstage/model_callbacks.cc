#include "stage.hh"
using namespace Stg;
using namespace std;

void Model::AddCallback( callback_type_t type, 
												 stg_model_callback_t cb, 
												 void* user )
{
  //callbacks[address].insert( stg_cb_t( cb, user ));	
	callbacks[type].insert( stg_cb_t( cb, user ));
}


int Model::RemoveCallback( callback_type_t type,
													 stg_model_callback_t callback )
{
	set<stg_cb_t>& callset = callbacks[type];	
	callset.erase( stg_cb_t( callback, NULL) );
	
	// return the number of callbacks remaining for this address. Useful
	// for detecting when there are none.
	return callset.size();
}


int Model::CallCallbacks( callback_type_t type )
{
	// maintain a list of callbacks that should be cancelled
	vector<stg_cb_t> doomed;
	
	set<stg_cb_t>& callset = callbacks[type];
	
	FOR_EACH( it, callset )
	  {  
			const stg_cb_t& cba = *it;  
			if( (cba.callback)( this, cba.arg ) )
				doomed.push_back( cba );
	  }      
	
	FOR_EACH( it, doomed )
		callset.erase( *it );

	// return the number of callbacks remaining for this address. Useful
	// for detecting when there are none.
	return callset.size();
}



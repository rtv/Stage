#include "stage.hh"
using namespace Stg;
using namespace std;

void Model::AddCallback( callback_type_t type, 
												 model_callback_t cb, 
												 void* user )
{
  //callbacks[address].insert( cb_t( cb, user ));	
	callbacks[type].insert( cb_t( cb, user ));
}


int Model::RemoveCallback( callback_type_t type,
													 model_callback_t callback )
{
	set<cb_t>& callset = callbacks[type];	
	callset.erase( cb_t( callback, NULL) );
	
	// return the number of callbacks remaining for this address. Useful
	// for detecting when there are none.
	return callset.size();
}


int Model::CallCallbacks( callback_type_t type )
{
	// maintain a list of callbacks that should be cancelled
	vector<cb_t> doomed;
	
	set<cb_t>& callset = callbacks[type];
	
	FOR_EACH( it, callset )
	  {  
			const cb_t& cba = *it;  
			if( (cba.callback)( this, cba.arg ) )
				doomed.push_back( cba );
	  }      
	
	FOR_EACH( it, doomed )
		callset.erase( *it );

	// return the number of callbacks remaining for this address. Useful
	// for detecting when there are none.
	return callset.size();
}



#include "stage.hh"
using namespace Stg;
using namespace std;

void Model::AddCallback( void* address, 
						 stg_model_callback_t cb, 
						 void* user )
{
  callbacks[address].insert( stg_cb_t( cb, user ));
	
	// count the number of callbacks attached to pose and
	// velocity. These are changed very often, so we avoid looking up
	// the callbacks if these values are zero. */
	if( address == &this->pose )
		++hooks.attached_pose;
	else if( address == &this->velocity )
		++hooks.attached_velocity;	
	else if( address == &this->hooks.update )
		++hooks.attached_update;	
}


int Model::RemoveCallback( void* address,
													 stg_model_callback_t callback )
{
	set<stg_cb_t>& callset = callbacks[address];
	
	callset.erase( stg_cb_t( callback, NULL) );
	
	// count the number of callbacks attached to pose and velocity. Can
	// not go below zero.
	if( address == &pose )
		hooks.attached_pose = max( --hooks.attached_pose, 0 );
	else if( address == &velocity )
		hooks.attached_velocity = max( --hooks.attached_velocity, 0 );
	else if( address == &hooks.update )
		hooks.attached_update = max( --hooks.attached_update, 0 );

	// return the number of callbacks remaining for this address. Useful
	// for detecting when there are none.
	return callset.size();
}


int Model::CallCallbacks( void* address )
{
	assert( address );
	
	// maintain a list of callbacks that should be cancelled
	vector<stg_cb_t> doomed;
	
	set<stg_cb_t>& callset = callbacks[address];
	
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



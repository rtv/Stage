/* 
   File rangernoise.cc: ranger noise plugin demo for Stage
   Author: Richard Vaughan
   Date: 3 March 2008
   CVS: $Id: rangernoise.cc,v 1.1 2008-03-04 02:09:56 rtv Exp $
*/

#include "stage.hh"
using namespace Stg;

const double DEVIATION = 0.05;

double simple_normal_deviate( double mean, double stddev )
{
  double x = 0.0;
  
  for( int i=0; i<12; i++ )
    x += rand()/(RAND_MAX+1.0);
  
  return ( stddev * (x - 6.0) + mean );  
}

// process the ranger data
int RangerUpdate( ModelRanger* mod, void* dummy )
{
  // get the data
	std::vector<meters_t>& scan = mod->GetSensorsMutable()[0].ranges;
	
  if( scan.size()>0 )
    FOR_EACH( it, scan )
      *it *= simple_normal_deviate( 1.0, DEVIATION );
  
  return 0; // run again
}

// Stage calls this when the model starts up. we just add a callback to
// the model that gets called just after the sensor update is done.
extern "C" int Init( Model* mod )
{
	mod->AddCallback( Model::CB_UPDATE, (model_callback_t)RangerUpdate, NULL );
	
  // add this so we can see the effects immediately, without needing
  // anyone else to subscribe to the ranger
  mod->Subscribe();

  return 0; // ok
}


#include "stage.hh"
using namespace Stg;
  
extern "C" void Init( StgModel* mod );
extern "C" void Update( StgModel* mod );

StgModelPosition* pos = NULL;
StgModelLaser* laser = NULL;


// Stage calls this once when the model starts up
void Init( StgModel* mod )
{
  puts( "Init controller" );
  
  pos = (StgModelPosition*)mod;
 
  laser = (StgModelLaser*)mod->GetUnsubscribedModelOfType( "laser" );
  assert( laser );
  laser->Subscribe();
}

// Stage calls this once per simulation update
void Update( StgModel* mod )
{
 
  pos->SetSpeed( 0.1, 0, 0.1 );  
}


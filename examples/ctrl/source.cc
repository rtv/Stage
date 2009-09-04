#include "stage.hh"
using namespace Stg;

const int INTERVAL = 200;
const double FLAGSZ = 0.4;

int Update( Model* mod, void* dummy );


// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{
  mod->AddUpdateCallback( (stg_model_callback_t)Update, NULL );  
  mod->Subscribe();
  return 0; //ok
}

// inspect the laser data and decide what to do
int Update( Model* mod, void* dummy )
{
  if( mod->GetWorld()->GetUpdateCount() % INTERVAL  == 0 )
	 mod->PushFlag( new Model::Flag( Color( 1,1,0 ), FLAGSZ ) );

  return 0; // run again
}


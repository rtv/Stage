#include "stage.hh"
using namespace Stg;

const int INTERVAL = 6000;

int Update( Model* mod, void* dummy );

// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{  
  mod->AddCallback( Model::CB_UPDATE, (stg_model_callback_t)Update, NULL );
  mod->Subscribe();
  return 0; //ok
}

// inspect the laser data and decide what to do
int Update( Model* mod, void* dummy )
{
  if( mod->GetWorld()->GetUpdateCount() % INTERVAL  == 0 )
	 // get rid of all the flags
	 while( mod->GetFlagCount() )
		mod->PopFlag();

  return 0; // run again
}


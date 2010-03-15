#include "stage.hh"
using namespace Stg;

const unsigned int INTERVAL = 100;
const double FLAGSZ = 0.25;
const unsigned int CAPACITY = 1;


int Update( Model* mod, void* dummy )
{
  if((  mod->GetWorld()->GetUpdateCount() % INTERVAL == 0 )
	  && ( mod->GetFlagCount() < CAPACITY) )
	 mod->PushFlag( new Model::Flag( Color( 1,1,0 ), FLAGSZ ) );

  return 0; // run again
}

// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{
  mod->AddCallback( Model::CB_UPDATE, (stg_model_callback_t)Update, NULL );  
  mod->Subscribe();
  return 0; //ok
}


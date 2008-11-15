#include "stage.hh"
using namespace Stg;

const int INTERVAL = 50;

int Update( StgModel* mod, void* dummy );

const double flagsz = 0.4;

// Stage calls this when the model starts up
extern "C" int Init( StgModel* mod )
{
  for( int i=0; i<5; i++ )
    mod->PushFlag( new StgFlag( stg_color_pack( 1,1,0,0 ), flagsz ) );
  
  mod->AddUpdateCallback( (stg_model_callback_t)Update, NULL );
  
  return 0; //ok
}

// inspect the laser data and decide what to do
int Update( StgModel* mod, void* dummy )
{
  // protect access to this model from other controllers
  mod->Lock();

  if( mod->GetWorld()->GetUpdateCount() % INTERVAL  == 0 )
    mod->PushFlag( new StgFlag( stg_color_pack( 1,1,0,0), flagsz ) );

  mod->Unlock();
  
  return 0; // run again
}


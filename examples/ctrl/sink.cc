#include "stage.hh"
using namespace Stg;

const int INTERVAL = 50;

int Update( StgModel* mod, void* dummy );

// Stage calls this when the model starts up
extern "C" int Init( StgModel* mod )
{
  
  for( int i=0; i<5; i++ )
    mod->PushFlag( new StgFlag( stg_color_pack( 1,1,0,0), 0.5 ) );
  
  mod->AddUpdateCallback( (stg_model_callback_t)Update, NULL );

  return 0; //ok
}

// inspect the laser data and decide what to do
int Update( StgModel* mod, void* dummy )
{
  
  if( mod->GetWorld()->GetUpdateCount() % INTERVAL  == 0 )
    mod->PopFlag();
  
  return 0; // run again
}


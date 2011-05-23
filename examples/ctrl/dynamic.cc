#include "stage.hh"
using namespace Stg;


// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{
  printf( "Creating 100 models" );
  
  for( int i=0; i<100; i++ )
	 {
		Model* child = mod->GetWorld()->CreateModel( mod, "model" );		
		assert(child);

		child->SetPose( Pose::Random( -1, 1, -1, 1 ) );
		child->SetGeom( Geom( Pose(), Stg::Size( 0.1, 0.1, 0.1 ) ));
		child->SetColor( Color(0,0,1,1) );
	 }
  return 0; //ok
}


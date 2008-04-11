/**
   \defgroup stage The Stage standalone robot simulator.

   Here is where I describe the Stage standalone program.
*/


//#include "config.h"
#include "stage_internal.hh"

const char* PACKAGE = "Stage";
const char* VERSION = "3.dev";

int main( int argc, char* argv[] )
{
  printf( "%s %s\n", PACKAGE, VERSION );
  
  if( argc < 2 )
    {
      printf( "Usage: %s <worldfile>", PACKAGE );
      exit(0);
    }
  
  // initialize libstage
  Stg::Init( &argc, &argv );
  
  StgWorldGui world(800, 700, argv[0]);
  
  world.Load( argv[1] );
  
  while( ! world.TestQuit() )
    world.RealTimeUpdate();
}


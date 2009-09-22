/**
   \defgroup stage The Stage standalone robot simulator.

   Here is where I describe the Stage standalone program.
*/

#include <getopt.h>

#include "stage_internal.hh"

const char* PACKAGE = "Stage";
const char* VERSION = "3.dev";

/* options descriptor */
static struct option longopts[] = {
  { "gui",  no_argument,   NULL,  'g' },
  //  { "fast",  no_argument,   NULL,  'f' },
  { NULL, 0, NULL, 0 }
};


int main( int argc, char* argv[] )
{
  printf( "%s %s ", PACKAGE, VERSION );

  int ch=0, optindex=0;
  bool usegui = true;

  while ((ch = getopt_long(argc, argv, "gf", longopts, &optindex)) != -1)
    {
      switch( ch )
	{
	case 0: // long option given
	  printf( "option %s given", longopts[optindex].name );
	  break;
	case 'g': 
	  usegui = false; 
	  printf( "[GUI disabled]" ); 
	  break;
	case '?':  
	  break;
	default:
	  printf("unhandled option %c\n", ch );
	}
    }

  puts("");// end the first start-up line
  
  // initialize libstage
  Stg::Init( &argc, &argv );
  
  StgWorld* world = usegui ? new StgWorldGui(800, 700, argv[0]) : new StgWorld();
  
  world->Update();
  world->Load( argv[argc-1] );
  
  while( ! world->TestQuit() )
    world->RealTimeUpdate();
}


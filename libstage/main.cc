/**
  \defgroup stage The Stage standalone robot simulator.

  Here is where I describe the Stage standalone program.
 */

#include <getopt.h>

#include "stage.hh"
#include "config.h"
using namespace Stg;

/* options descriptor */
static struct option longopts[] = {
	{ "gui",  optional_argument,   NULL,  'g' },
	{ "port",  required_argument,   NULL,  'p' },
	{ "host",  required_argument,   NULL,  'h' },
	{ "federation",  required_argument,   NULL,  'f' },
	{ NULL, 0, NULL, 0 }
};

int main( int argc, char* argv[] )
{
  // initialize libstage - call this first
  Stg::Init( &argc, &argv );

  printf( "%s %s ", PROJECT, VERSION );
  
  int ch=0, optindex=0;
  bool usegui = true;
  
  while ((ch = getopt_long(argc, argv, "gfp:h:f:", longopts, &optindex)) != -1)
	 {
		switch( ch )
		  {
		  case 0: // long option given
			 printf( "option %s given\n", longopts[optindex].name );
			 break;
		  case 'g': 
			 usegui = false;
			 printf( "[GUI disabled]" );
			 break;
		  case 'p':
			 printf( "PORT %d\n", atoi(optarg) );
		  case '?':  
			 break;
		  default:
			 printf("unhandled option %c\n", ch );
		  }
	 }
  
  puts("");// end the first start-up line
  
  exit(0);

  // arguments at index [optindex] and later are not options, so they
  // must be world file names
  
  bool loaded_world_file = false;
  optindex = optind; //points to first non-option
  while( optindex < argc )
	 {
		if( optindex > 0 )
		  {      
			 const char* worldfilename = argv[optindex];
			 World* world = ( usegui ? 
										new WorldGui( 400, 300, worldfilename ) : 
										new World( worldfilename ) );
			 world->Load( worldfilename );
			 loaded_world_file = true;
		  }
		optindex++;
	 }
  
  if( loaded_world_file == false ) 
	 {
		// TODO: special window/loading dialog for this case
		new WorldGui( 400, 300 );
	 }
  
  if( usegui == true ) 
	 {
		//don't close the window once time has finished
		while( true )
		  World::UpdateAll();
	 } 
  else 
	 {
		//close program once time has completed
		bool quit = false;
		while( quit == false )
		  quit = World::UpdateAll();
	 }
}

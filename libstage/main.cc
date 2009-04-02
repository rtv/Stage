/**
  \defgroup stage The Stage standalone robot simulator.

  Here is where I describe the Stage standalone program.
 */

#include <getopt.h>

#include "stage.hh"
#include "config.h"
using namespace Stg;

const char* USAGE = 
  "USAGE:  stage [options] [<worldfile>]\n"
  "Available [options] are:\n"
  "  --gui          : run without a GUI (same as -g)\n"
  "  -g             : run without a GUI (same as --gui)\n"
  "  --help         : print this message.\n"
  "  -h             : print this message.\n"
  "  -?             : print this message.\n"
  " If <worldfile> is not specified, Stage starts with a file selector dialog";

/* options descriptor */
static struct option longopts[] = {
	{ "gui",  optional_argument,   NULL,  'g' },
	{ "help",  optional_argument,   NULL,  'h' },
	{ NULL, 0, NULL, 0 }
};

int main( int argc, char* argv[] )
{
  // initialize libstage - call this first
  Stg::Init( &argc, &argv );

  printf( "%s %s ", PROJECT, VERSION );
  
  int ch=0, optindex=0;
  bool usegui = true;
  
  while ((ch = getopt_long(argc, argv, "gh?", longopts, &optindex)) != -1)
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
		  case 'h':  
		  case '?':  
			 puts( USAGE );
			 exit(0);
			 break;
		  default:
			 printf("unhandled option %c\n", ch );
			 puts( USAGE );
			 exit(0);
		  }
	 }
  
  puts("");// end the first start-up line

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
  
  if( usegui )
	 while( true ) World::UpdateAll();
  else
	 while( ! World::UpdateAll() );

  exit(0);
}

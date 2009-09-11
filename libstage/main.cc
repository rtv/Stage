/**
  \defgroup stage The Stage standalone robot simulator.

  Here is where I describe the Stage standalone program.
 */

#include <getopt.h>

#include "stage.hh"
#include "config.h"
using namespace Stg;

const char* USAGE = 
  "USAGE:  stage [options] <worldfile1> [worldfile2 ... worldfileN]\n"
  "Available [options] are:\n"
  "  --clock        : print simulation time peridically on standard output\n"
  "  -c             : short for --clock\n"
  "  --gui          : run without a GUI\n"
  "  -g             : short for --gui\n"
  "  --help         : print this message\n"
  "  --args \"str\"   : define an argument string to be passed to all controllers\n"
  "  -a \"str\"       : short for --args\n"
  "  -h             : short for --help\n"
  "  -?             : short for --help";

/* options descriptor */
static struct option longopts[] = {
	{ "gui",  optional_argument,   NULL,  'g' },
	{ "clock",  optional_argument,   NULL,  'c' },
	{ "help",  optional_argument,   NULL,  'h' },
	{ "args",  required_argument,   NULL,  'a' },
	{ NULL, 0, NULL, 0 }
};

int main( int argc, char* argv[] )
{
  // initialize libstage - call this first
  Stg::Init( &argc, &argv );

  printf( "%s %s ", PROJECT, VERSION );
  
  int ch=0, optindex=0;
  bool usegui = true;
  bool showclock = false;
  
  while ((ch = getopt_long(argc, argv, "cgh?", longopts, &optindex)) != -1)
	 {
		switch( ch )
		  {
		  case 0: // long option given
			 printf( "option %s given\n", longopts[optindex].name );
			 break;
		  case 'a':
			 World::ctrlargs = std::string(optarg);
			 break;
		  case 'c': 
			 showclock = true;
			 printf( "[Clock enabled]" );
			 break;
		  case 'g': 
			 usegui = false;
			 printf( "[GUI disabled]" );
			 break;
		  case 'h':  
		  case '?':  
			 puts( USAGE );
			 //			 exit(0);
			 break;
		  default:
			 printf("unhandled option %c\n", ch );
			 puts( USAGE );
			 //exit(0);
		  }
	 }
  
  puts("");// end the first start-up line

  // arguments at index [optindex] and later are not options, so they
  // must be world file names
  
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
			 world->ShowClock( showclock );

			 if( ! world->paused ) 
				world->Start();
		  }
		optindex++;
	 }
  
  if( usegui )
	 Fl::run();	 
  else
	 while( ! World::UpdateAll() );

  puts( "\n[Stage: done]" );
  exit(0);
}

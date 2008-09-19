/**
  \defgroup stage The Stage standalone robot simulator.

  Here is where I describe the Stage standalone program.
 */

#include <getopt.h>

#include "stage_internal.hh"
#include "config.h"

/* options descriptor */
static struct option longopts[] = {
	{ "gui",  no_argument,   NULL,  'g' },
	//  { "fast",  no_argument,   NULL,  'f' },
	{ NULL, 0, NULL, 0 }
};

int main( int argc, char* argv[] )
{
	printf( "%s %s ", PROJECT, VERSION );

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

	// arguments at index optindex and later are not options, so they
	// must be world file names


	bool loaded_world_file = false;
	optindex = optind; //points to first non-option
	while( optindex < argc )
	{
		if( optindex > 0 )
		{      
			const char* worldfilename = argv[optindex];
			StgWorld* world = ( usegui ? 
									  new StgWorldGui( 400, 300, worldfilename ) : 
									  new StgWorld( worldfilename ) );
			world->Load( worldfilename );
			loaded_world_file = true;
		}
		optindex++;
	}

 	if( loaded_world_file == false ) {
 		// TODO: special window/loading dialog for this case
 		new StgWorldGui( 400, 300 );
 	}

	if( usegui == true ) {
		//don't close the window once time has finished
		while( true )
			StgWorld::UpdateAll();
	} else {
		//close program once time has completed
		bool quit = false;
		while( quit == false )
			quit = StgWorld::UpdateAll();
	}
}


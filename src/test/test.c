
/*
  $Id: test.c,v 1.1.2.2 2003-02-03 07:12:48 rtv Exp $
*/

#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif


#define DEBUG
#define VERBOSE
//#undef DEBUG

#include <stdlib.h>
#include <stdio.h>
#include "stageio.h"

#define TEST(msg) (1 ? printf(  "TEST: " msg " ... "), fflush(stdout) : 0)
#define TEST1(msg, a) (1 ? printf( "TEST: " msg " ... ", a), fflush(stdout) : 0)
#define TESTPASS() (1 ? printf("pass\n"), fflush(stdout) : 0)
#define TESTFAIL() (1 ? printf("\033[41mfail\033[0m\n"), fflush(stdout) : 0)

int main( int argc, char** argv )
{
  printf("\n** Test program for Stage  v%s **\n", (char*) VERSION);

  TEST( "Connecting to Stage" );

  int stage_fd = SIOInitClient( argc, argv);

  if( stage_fd == -1 )
    {
      TESTFAIL();
      exit( -1 );
    }
  TESTPASS();

  //while( 1 )
    {
      TEST( "Creating some models..." );
      stage_model_t ent[5];

      ent[0].id = 0;
      ent[0].parent_id = -1; // only the root can have no parent
      strcpy( ent[0].token, "root" );

      ent[1].id = 1;
      ent[1].parent_id = 0;
      strcpy( ent[1].token, "box" );

      ent[2].id = 2;
      ent[2].parent_id = 0;
      strcpy( ent[2].token, "box" );

      int result = SIOWriteModels( stage_fd, 0.0, ent, 3 );

      result == -1 ? TESTFAIL() : TESTPASS(); 

      int c;
      //while( 1 )
      for( c=0; c<10; c++ )
	{
	  printf( "cycle %d\n", c );
	  
	  SIOContinue( stage_fd, 0.0 );
	  
	  SIOServiceConnections( NULL, NULL, NULL );
	  
	}
    }
    
    return 0;
}

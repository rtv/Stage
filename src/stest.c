
/*
  $Id: stest.c,v 1.1.2.2 2003-02-04 03:35:38 rtv Exp $
*/

#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

#include <string.h> // for strcpy


#define DEBUG
#define VERBOSE
//#undef DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "stageio.h"

#define TEST(msg) (1 ? printf(  "TEST: " msg " ... "), fflush(stdout) : 0)
#define TEST1(msg, a) (1 ? printf( "TEST: " msg " ... ", a), fflush(stdout) : 0)
#define TESTPASS() (1 ? printf("pass\n"), fflush(stdout) : 0)
#define TESTFAIL() (1 ? printf("\033[41mfail\033[0m\n"), fflush(stdout) : 0)


int HandleModel( int connection, stage_model_t* model )
{
  printf( "Received model packet on connection %d", connection );
  return 0; //success
}

int HandleProperty( int connection, stage_property_t* prop )
{
  printf( "Received property (%d,%d,%d) on connection %d\n",
	  prop->id, prop->property, prop->len, connection );
  return 0; //success
}

int HandleCommand( int connection, stage_cmd_t* cmd )
{
  printf( "Received command (%d) on connection %d\n", 
	  *cmd, connection );
  return 0; //success
}


int main( int argc, char** argv )
{
  printf("\n** Test program for Stage  v%s **\n", (char*) VERSION);

  TEST( "Connecting to Stage" );

  int connection = SIOInitClient( argc, argv);

  if( connection == -1 )
    {
      TESTFAIL();
      exit( -1 );
    }
  TESTPASS();

  //while( 1 )
    {
      TEST( "Creating some models..." );

      // set some properties
      stage_buffer_t* models = SIOCreateBuffer();
      assert( models );

      stage_model_t model;
      model.id = 0;
      model.parent_id = -1; // only the root can have no parent
      strncpy( model.token, "root", STG_TOKEN_MAX );
      
      SIOBufferPacket( models, (char*)&model, sizeof(model) );

      (SIOWriteModels( connection, 0.0, (stage_model_t*)models->data, 1 ) == -1 ) 
	? TESTFAIL():TESTPASS(); 
      
      SIOFreeBuffer( models );

      // set some properties
      stage_buffer_t* props = SIOCreateBuffer();
      assert( props );
      
      stage_pose_t pose;
      pose.x = 2.0;
      pose.y = 3.0;
      pose.a = 4.0;
      
      SIOBufferProperty( props, 0,
			 STG_PROP_ENTITY_POSE, (char*)&pose, sizeof(pose) );

      
      SIOWriteProperties( connection, 0.0, // timestamp 
			  props->data, props->len );
      
      SIOFreeBuffer( props );

      int c = 0;
      while( c<5 )
	{
	  printf( "cycle %d\n", c++ );
	  
	  SIOContinue( connection, 0.0 );
	  
	  SIOServiceConnections(&HandleCommand,
				&HandleModel, 
				&HandleProperty );
	}
    }
    
    return 0;
}









/*
  $Id: stest.c,v 1.1.2.3 2003-02-05 03:59:49 rtv Exp $
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
#define EVAL(result) (1 ? (result == -1) ? TESTFAIL() : TESTPASS() : 0 )
		      
int HandleModel( int connection, char* data, size_t len )
{
  assert( len == sizeof(stage_model_t) );
  stage_model_t* model = (stage_model_t*)data;
  
  printf( "Received %d bytes model packet on connection %d", len, connection );
  return 0; //success
}

int HandleProperty( int connection, char* data, size_t len )
{ 
  assert( len >= sizeof(stage_property_t) );
  stage_property_t* prop = (stage_property_t*)data;
  
  printf( "Received %d bytes  property (%d,%d,%d) on connection %d\n",
	  len, prop->id, prop->property, prop->len, connection );
  return 0; //success
}

int HandleCommand( int connection, char* data, size_t len )
{  
  assert( len == sizeof(stage_cmd_t) );
  stage_cmd_t* cmd = (stage_cmd_t*)data;
  
  printf( "Received %d bytes command (%d) on connection %d\n", 
	  len, *cmd, connection );
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
  
  int result = 0;
  //while( 1 )
    {
      double timestamp = 0.0;

      TEST( "Creating a GUI..." );

      stage_gui_config_t gui;
      strcpy( gui.token, "rtk" );
      gui.width = 200;
      gui.height = 100;
      gui.ppm = 20;
      gui.originx = 0;
      gui.originy = 0;
      gui.showsubscribedonly = 0;
      gui.showgrid = 1;
      gui.showdata = 1;
            
      result = SIOWriteMessage( connection, timestamp, 
				STG_HDR_GUI, (char*)&gui, sizeof(gui) ) ;
      
      EVAL(result);
      
      TEST( "Creating some models..." );
      
      // set some properties
      stage_buffer_t* models = SIOCreateBuffer();
      assert( models );
    
      
      stage_model_t model;
      model.id = 0;
      model.parent_id = -1; // only the root can have no parent
      strncpy( model.token, "root", STG_TOKEN_MAX );
      
      SIOBufferPacket( models, (char*)&model, sizeof(model) );
     
      model.id = 1;
      model.parent_id = 0; // root
      strncpy( model.token, "box", STG_TOKEN_MAX );

      SIOBufferPacket( models, (char*)&model, sizeof(model) );

      model.id = 2;
      model.parent_id = 0; // root
      strncpy( model.token, "box", STG_TOKEN_MAX );

      SIOBufferPacket( models, (char*)&model, sizeof(model) );

      model.id = 3;
      model.parent_id = 0; // root
      strncpy( model.token, "box", STG_TOKEN_MAX );

      SIOBufferPacket( models, (char*)&model, sizeof(model) );


      result = SIOWriteMessage( connection, timestamp, 
				STG_HDR_MODELS, models->data, models->len ) ;
      
      EVAL(result);
      
      SIOFreeBuffer( models );


      // set some properties
      
      stage_buffer_t* props = SIOCreateBuffer();
      assert( props );
      
      stage_pose_t pose;
      SIOPackPose( &pose, 1.0, 1.0, 0.0 );      
      SIOBufferProperty( props, 1,
			 STG_PROP_ENTITY_POSE, (char*)&pose, sizeof(pose) );
      
      SIOPackPose( &pose, 2.0, 1.0, 45.0 );
      SIOBufferProperty( props, 2,
			 STG_PROP_ENTITY_POSE, (char*)&pose, sizeof(pose) );

      SIOPackPose( &pose, 3.0, 2.0, 80.0 );
      SIOBufferProperty( props, 3,
			 STG_PROP_ENTITY_POSE, (char*)&pose, sizeof(pose) );

      result = SIOWriteMessage( connection, timestamp,
				STG_HDR_PROPS, props->data, props->len );
      EVAL(result);

      SIOFreeBuffer( props );
      

      int c = 0;
      while( c<5 )
	{
	  printf( "cycle %d\n", c++ );
	  
	  SIOWriteMessage( connection, timestamp, STG_HDR_CONTINUE, NULL, 0 );
	  
	  SIOServiceConnections(&HandleCommand,
				&HandleModel, 
				&HandleProperty,
				NULL );
	}
    }
    
    return 0;
}









/*
  $Id: stest.c,v 1.1.2.6 2003-02-06 03:55:54 rtv Exp $
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
#include <math.h>
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

      TEST( "Creating a GUI" );

      stage_gui_config_t gui;
      strcpy( gui.token, "rtk" );
      gui.width = 600;
      gui.height = 600;
      gui.ppm = 10;
      gui.originx = 0;
      gui.originy = 0;
      gui.showsubscribedonly = 0;
      gui.showgrid = 1;
      gui.showdata = 1;
            
      result = SIOWriteMessage( connection, timestamp, 
				STG_HDR_GUI, (char*)&gui, sizeof(gui) ) ;
      
      EVAL(result);
      
      const int ROOT = 0, BOX = 1, BITMAP = 2;
      
      // define some models we want to create
      stage_model_t models[3];
      models[0].id = ROOT;
      models[0].parent_id = -1; // only the root can have no parent
      strncpy( models[0].token, "root", STG_TOKEN_MAX );
      
      models[1].id = BOX;
      models[1].parent_id = ROOT; // root
      strncpy( models[1].token, "box", STG_TOKEN_MAX );
      
      models[2].id = BITMAP;
      models[2].parent_id = ROOT; // root
      strncpy( models[2].token, "bitmap", STG_TOKEN_MAX );
      
      TEST( "Sending models" );
      result = SIOWriteMessage( connection, timestamp, STG_HDR_MODELS, 
				(char*)&models, 3*sizeof(stage_model_t) );
      EVAL(result);
      
      // define some properties
      stage_buffer_t* props = SIOCreateBuffer();
      assert(props);
      
      // set the size of the root (and hence the matrix)
      stage_size_t sz;
      sz.x = 10.0;
      sz.y = 5.0;
      SIOBufferProperty( props, ROOT, STG_PROP_ENTITY_SIZE, 
			 (char*)&sz, sizeof(sz) );

      // set the spatial resolution of the simulator in pixels-per-meter
      double ppm = 200;
      //SIOBufferProperty( props, ROOT, STG_PROP_ENTITY_PPM, 
      //		 (char*)&ppm, sizeof(ppm) );
      
      
      // pose the bitmap
      stage_pose_t pose;
      SIOPackPose( &pose, 4.0, 6.0, 0.0 );      
      SIOBufferProperty( props, BITMAP, STG_PROP_ENTITY_POSE, 
      		 (char*)&pose, sizeof(pose) );
      
      
      // push some rectangles into the bitmap
      stage_rotrect_t rects[10];
      int r;
      for( r=0; r<10; r++ )
	{
	  rects[r].x = r/10.0;
	  rects[r].y = r/20.0;
	  rects[r].a = 0;
	  rects[r].w = 0.1;
	  rects[r].h = 0.1;
	}
      
      SIOBufferProperty( props, BITMAP, STG_PROP_BITMAP_RECTS, 
			 (char*)&rects, 10*sizeof(rects[0]) );
      
      TEST( "Sending properties" );
      result = SIOWriteMessage( connection, timestamp,
				STG_HDR_PROPS, props->data, props->len );
      EVAL(result);
      
      SIOFreeBuffer( props );

      stage_size_t bitmapsize;

      int c = 0;
      double x = 0;
      while( 1 )
	{
	  printf( "cycle %d\n", c++ );
	  
	  SIOWriteMessage( connection, timestamp, STG_HDR_CONTINUE, NULL, 0 );

	  //if( x == 0 ) sleep(3);

	  // define some properties
	  stage_buffer_t* bp = SIOCreateBuffer();
	  assert(bp);
	  
	  // change the size of the bitmap 
	  bitmapsize.x = 0.5 + 3.0 * fabs(sin(x));
	  bitmapsize.y = 0.5 + 3.0 * fabs(cos(x+=0.05));
	  
	  SIOBufferProperty( bp, BITMAP, STG_PROP_ENTITY_SIZE, 
			     (char*)&bitmapsize, sizeof(bitmapsize) );
	  
	  //TEST( "Sending properties" );
	  result = SIOWriteMessage( connection, timestamp,
	  			    STG_HDR_PROPS, bp->data, bp->len );
	  //EVAL(result);
	   SIOFreeBuffer( bp );
	   
	   if( c == 75 )
	     {
	       stage_gui_config_t gui;
	       strcpy( gui.token, "rtk" );
	       gui.width = 600;
	       gui.height = 600;
	       gui.ppm = 50;
	       gui.originx = 0;
	       gui.originy = 0;
	       gui.showsubscribedonly = 0;
	       gui.showgrid = 1;
	       gui.showdata = 1;
	       
	       result = SIOWriteMessage( connection, timestamp, 
					 STG_HDR_GUI, (char*)&gui, sizeof(gui) ) ;
	     }
	   
	  SIOServiceConnections(&HandleCommand,
				&HandleModel, 
				&HandleProperty,
				NULL );
	}
    }
    
    return 0;
}








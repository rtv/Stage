
/*
  $Id: stest.c,v 1.1.2.23 2003-02-26 01:57:15 rtv Exp $
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
#include <pnm.h> // libpnm functions

#include "sio.h"

#define TEST(msg) (1 ? printf(  "TEST: " msg " ... "), fflush(stdout) : 0)
#define TEST1(msg, a) (1 ? printf( "TEST: " msg " ... ", a), fflush(stdout) : 0)
#define TESTPASS() (1 ? printf("pass\n"), fflush(stdout) : 0)
#define TESTFAIL() (1 ? printf("\033[41mfail\033[0m\n"), fflush(stdout) : 0)
#define EVAL(result) (1 ? (result == -1) ? TESTFAIL() : TESTPASS() : 0 )
		      
const double timestamp = 0.0;

int pending_key = 0;
int pending_id = -1;

// load the pnm file and compress it into rectangles
// the caller must free the memory
stage_rotrect_t*  RectsFromPnm( int *num_rects, char* filename )
{
  printf( "attempting to open %s\n", filename );
  FILE* pnmfile = fopen( filename, "r" );
  
  if( pnmfile == NULL )
    perror( "failed to open pnm file" );
  
  //int num_rects;
  stage_rotrect_t* rects;
  assert( rects = malloc( 10000 * sizeof(stage_rotrect_t) ) );
  *num_rects = 0;

  int cols, rows, format;
  xelval maxval;
  
  xel** pixels = 
    pnm_readpnm( pnmfile, &cols, &rows, &maxval, &format );
  
  printf( "read pnm %s cols %d rows %d maxval %d format %d\n",
	  filename, cols, rows, maxval, format );
  
  // RTV - this box-drawing algorithm compresses hospital.world from
  // 104,000+ pixels to 5,757 rectangles. it's not perfect but pretty
  // darn good with bitmaps featuring lots of horizontal and vertical
  // lines - such as most worlds. Also combined matrix & gui
  // rendering loops.  hospital.pnm.gz now loads more than twice as
  // fast and redraws waaaaaay faster. yay!
  int x,y;
  for (y = 0; y < rows; y++)
  {
    for (x = 0; x < cols; x++)
      {	
	if (PNM_GET1( pixels[y][x]) == 0)
	  continue;
	
	// a rectangle starts from this point
	int startx = x;
	int starty = rows - y;
	int height = rows; // assume full height for starters
	
	// grow the width - scan along the line until we hit an empty pixel
	for( ;  x < cols && PNM_GET1( pixels[y][x] ) > 0; x++ )
	  {
	    // handle horizontal cropping
	    //double ppx = x * sx; 
	    //if (ppx < this->crop_ax || ppx > this->crop_bx)
	    //continue;
	    
	    // look down to see how large a rectangle below we can make
	    int yy  = y;
	    while( yy < rows && PNM_GET1( pixels[yy][x] ) > 0   )
	      { 
		// handle vertical cropping
		//double ppy = (this->image->height - yy) * sy;
		//if (ppy < this->crop_ay || ppy > this->crop_by)
		//continue;
		
		yy++; 
	      } 	      
	    // now yy is the depth of a line of non-zero pixels
	    // downward we store the smallest depth - that'll be the
	    // height of the rectangle
	    if( yy-y < height ) height = yy-y; // shrink the height to fit
	  } 
	
	// we've found a rectangle of (width by height) non-zero pixels 
	int width = x - startx;
	
	// delete the pixels we have used in this rect
	int rmy;
	for( rmy = y; rmy < y + height; rmy++ )
	  memset( &pixels[rmy][startx], 0, width * sizeof(xel) );
	
	rects[*num_rects].x = startx;
	rects[*num_rects].y = starty;
	rects[*num_rects].a = 0;
	rects[*num_rects].w = width;
	rects[*num_rects].h = -height;
	(*num_rects)++;
      }
  }  
  return rects;
}


int HandleLostConnection( int connection )
{
  PRINT_ERR1( "lost connection on %d", connection );
  return 0;
}

int HandleModel( int connection, void* data, size_t len, 
		 stage_buffer_t* replies )
{
  assert( len == sizeof(stage_model_t) );
  stage_model_t* model = (stage_model_t*)data;
  
  //printf( "Received %d bytes model packet on connection %d", (int)len, connection );

  // if we see the key we've been waiting for, the model request is confirmed
  if( model->key == pending_key )
    pending_id = model->id; // find out what id we were given
  
  return 0; //success
}

int HandleProperty( int connection, void* data, size_t len,
		    stage_buffer_t* replies )
{ 
  assert( len >= sizeof(stage_property_t) );
  stage_property_t* prop = (stage_property_t*)data;
  
  printf( "Received %d bytes property (%d,%s,%d) on connection %d\n",
    (int)len, prop->id, SIOPropString(prop->property), (int)prop->len, connection );
  return 0; //success
}

int HandleCommand( int connection, void* data, size_t len,
		 stage_buffer_t* replies )
{  
  assert( len == sizeof(stage_cmd_t) );
  stage_cmd_t* cmd = (stage_cmd_t*)data;
  
  printf( "Received %d bytes command (%d) on connection %d\n", 
	  (int)len, *cmd, connection );
  return 0; //success
}


void Resize( int con, int id, double x, double y )
{
  // define some properties
  stage_buffer_t* bp = SIOCreateBuffer();
  assert(bp);
  
  // set the size of the root (and hence the matrix)
  stage_size_t sz;
  sz.x = x;
  sz.y = y;
  SIOBufferProperty( bp, id, STG_PROP_ENTITY_SIZE, 
		     (char*)&sz, sizeof(sz), STG_NOREPLY );
    
  SIOWriteMessage( con, timestamp,
		   STG_HDR_PROPS, bp->data, bp->len );
  
  SIOFreeBuffer( bp );
}

void SetResolution( int con, int id, double ppm )
{
  // define some properties
  stage_buffer_t* bp = SIOCreateBuffer();
  assert(bp);
  
  // set the spatial resolution of the simulator in pixels-per-meter
  SIOBufferProperty( bp, id, STG_PROP_ENTITY_PPM, 
		     (char*)&ppm, sizeof(ppm), STG_NOREPLY);
  
  SIOWriteMessage( con, timestamp,
		   STG_HDR_PROPS, bp->data, bp->len );
  
  SIOFreeBuffer( bp );
}

void SetVelocity( int con, int id, double vx, double vy, double va )
{
  stage_velocity_t vel;
  vel.x = vx;
  vel.y = vy;
  vel.a = va;
  
  stage_buffer_t* bp = SIOCreateBuffer();
  assert(bp);

  SIOBufferProperty( bp, id, STG_PROP_ENTITY_VELOCITY, 
		     (char*)&vel, sizeof(vel),  STG_NOREPLY);

  SIOWriteMessage( con, timestamp,
		   STG_HDR_PROPS, bp->data, bp->len );
  
  SIOFreeBuffer( bp );
}



  int main( int argc, char** argv )
{
  printf("\n** Test program for Stage  v%s **\n", (char*) VERSION);


  
  // init libpnm
  pnm_init( &argc, argv );


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

      stage_buffer_t* guireq = SIOCreateBuffer();

      // request a GUI so we can see what we're doing
      stage_gui_config_t gui;
      strcpy( gui.token, "rtk" );
      gui.width = 600;
      gui.height = 600;
      gui.ppm = 40;
      gui.originx = 0;//300;
      gui.originy = 0;//300;
      gui.showsubscribedonly = 0;
      gui.showgrid = 1;
      gui.showdata = 1;
      
      SIOBufferProperty( guireq, 0, STG_PROP_ROOT_GUI,
			 &gui, sizeof(gui), STG_NOREPLY );
      
      // no replies expected 
      SIOPropertyUpdate( connection, timestamp, guireq, NULL );
      
      SIOFreeBuffer( guireq );
  
      //    sleep(4);

      int b;
     
      stage_model_t bases[2];
      for( b=0; b<2; b++ )
	{
	  bases[b].parent_id = 0; // root!
	  strncpy( bases[b].token, "position", STG_TOKEN_MAX );
	}
      
      SIOCreateModels( connection, timestamp, bases, 2 );
 

      stage_model_t boxes[4];
      for( b=0; b<4; b++ )
	{
	  boxes[b].parent_id = 0; // root!
	  strncpy( boxes[b].token, "box", STG_TOKEN_MAX );
	}
      
      SIOCreateModels( connection, timestamp, boxes, 4 );
      
      for( b=0; b<4; b++ )
	{
	  PRINT_DEBUG3( "created %s id %d parent %d", 
			boxes[b].token, boxes[b].id, boxes[b].parent_id );
	}

      stage_model_t idars[2];
      for( b=0; b<2; b++ )
	{
	  idars[b].parent_id = bases[b].id; // root 
	  strncpy( idars[b].token, "idar", STG_TOKEN_MAX );
	}
      
      SIOCreateModels( connection, timestamp, idars, 2 );


      stage_buffer_t* props = SIOCreateBuffer();
      assert(props);

      int num_rects;
      stage_rotrect_t* rects = RectsFromPnm( &num_rects, "worlds/cave.pnm" );
      
      // send the rectangles to root
      SIOBufferProperty( props, 0, STG_PROP_ENTITY_RECTS, 
			 rects, num_rects * sizeof(rects[0]), STG_NOREPLY );
      
      
      // size a box
      stage_size_t size;

      size.x = 0.5;
      size.y = 0.5;
      SIOBufferProperty( props, boxes[0].id, STG_PROP_ENTITY_SIZE,
			 &size, sizeof(size), STG_NOREPLY );
      
      size.x = 1.0;
      size.y = 1.0;
      SIOBufferProperty( props, boxes[1].id, STG_PROP_ENTITY_SIZE,
			 &size, sizeof(size), STG_NOREPLY );

      size.x = 1.5;
      size.y = 1.5;
      SIOBufferProperty( props, boxes[2].id, STG_PROP_ENTITY_SIZE,
			 &size, sizeof(size), STG_NOREPLY );

      size.x = 2.0;
      size.y = 2.0;
      SIOBufferProperty( props, boxes[3].id, STG_PROP_ENTITY_SIZE,
			 &size, sizeof(size), STG_NOREPLY );

      // pose the box
      stage_pose_t pose;
      SIOPackPose( &pose, 2.0, 1.0, 0.0 );      
      SIOBufferProperty( props, boxes[0].id, STG_PROP_ENTITY_POSE, 
			 &pose, sizeof(pose), STG_NOREPLY );

      SIOPackPose( &pose, 4.0, 1.0, 0.5 );      
      SIOBufferProperty( props, boxes[1].id, STG_PROP_ENTITY_POSE, 
			 &pose, sizeof(pose), STG_NOREPLY );

      SIOPackPose( &pose, 6.0, 1.0, 1.0 );      
      SIOBufferProperty( props, boxes[2].id, STG_PROP_ENTITY_POSE, 
			 &pose, sizeof(pose), STG_NOREPLY );

      SIOPackPose( &pose, 8.0, 1.0, 1.5 );      
      SIOBufferProperty( props, boxes[3].id, STG_PROP_ENTITY_POSE, 
			 &pose, sizeof(pose), STG_NOREPLY );


      SIOPackPose( &pose, 2.0, 7.0, 1.5 );      
      SIOBufferProperty( props, bases[0].id, STG_PROP_ENTITY_POSE, 
			 &pose, sizeof(pose), STG_NOREPLY );


      SIOPackPose( &pose, 2.0, 7.5, 1.9 );      
      SIOBufferProperty( props, bases[1].id, STG_PROP_ENTITY_POSE, 
			 &pose, sizeof(pose), STG_NOREPLY );

      
      stage_position_cmd_t cmd;
      cmd.xdot = 0.3;
      cmd.ydot = 0.1;
      cmd.adot = 0.3;
      cmd.x = 0.0;
      cmd.y = 0.0;
      cmd.a = 0.0;
      
      SIOBufferProperty( props, bases[1].id, STG_PROP_ENTITY_COMMAND, 
			 &cmd, sizeof(cmd), STG_NOREPLY );
      

      // subscribe to each box's pose and size
      int subs[2];
      subs[0] = STG_PROP_ENTITY_POSE;
      subs[1] = STG_PROP_ENTITY_SIZE;
      
      //for( b=0; b<4; b++ ) 
      //SIOBufferProperty( props, boxes[b].id, STG_PROP_ENTITY_SUBSCRIBE,
      //		   subs, 2*sizeof(subs[0]), STG_NOREPLY );
      
      
      //rects = RectsFromPnm( &num_rects, "worlds/smiley.ppm" );
      //SIOBufferProperty( props, bitmap.id, STG_PROP_ENTITY_RECTS, 
      //	 (char*)rects, num_rects * sizeof(rects[0]),0 );
      
      
      // no replies expected for these properties
      result = SIOPropertyUpdate( connection, timestamp, props, NULL );
      
      SIOFreeBuffer( props );
      
      props = SIOCreateBuffer();
      
      // we sized the boxes - now make some sonars
      stage_model_t sonars[4];
      for( b=0; b<4; b++ )
	{
	  sonars[b].parent_id = boxes[b].id; 
	  strncpy( sonars[b].token, "sonar", STG_TOKEN_MAX );
	}
      
      SIOCreateModels( connection, timestamp, sonars, 4 );
     
      // configure the sonars
      // set the geometry of the sonars
      double sgeom[3][3];
      
      sgeom[0][0] = 0.2;
      sgeom[0][1] = 0.0;
      sgeom[0][2] = 0.0;

      sgeom[1][0] = 0.0;
      sgeom[1][1] = 0.2;
      sgeom[1][2] = 1.0;

      sgeom[2][0] = 0.0;
      sgeom[2][1] = -0.2;
      sgeom[2][2] = -1.0;
      
      //SIOBufferProperty( props, sonars[0].id, STG_PROP_ENTITY_GEOM,
      //		 sgeom, 3*3*sizeof(double), STG_NOREPLY );
      
      // subscribe to each sonar's data
      int sub = STG_PROP_ENTITY_DATA;
      
      for( b=0; b<1; b++ ) 
	SIOBufferProperty( props, sonars[b].id, STG_PROP_ENTITY_SUBSCRIBE,
			   &sub, sizeof(sub), STG_NOREPLY );
      
      // no replies expected for these properties
      result = SIOPropertyUpdate( connection, timestamp, props, NULL );
      
      SIOFreeBuffer( props );
      
     
      //TEST( "Sending properties" );

      //getprops = SIOCreateBuffer();
      //result = SIOPropertyUpdate( connection, props, getprops );
      
      //EVAL(result);

      //SIODebugBuffer( getprops );
      


      //puts( "looping" );
      
      int c = 0;
      double x = 0;
      double  y = 0;
      double z = 0;
      while( c < 100000 )
	{
	  printf( " cycle %d\r", c++ ); fflush( stdout );
	  
	  
	  //Resize( connection, bitmap.id,  
	  // 0.5 + 3.0 * fabs(sin(x)),  0.5 + 3.0 * fabs(cos(x+=0.05)) );
	 
	  //SetVelocity( connection, box.id, 3.0 * sin(x), 2.0 * cos(x+=0.1), 2.0 );
	  
	  if(  0 )//(c > 400 ) && ((c % 20 ) == 0) ) // 5Hz
	    {
	      stage_buffer_t* buf = SIOCreateBuffer();

	      stage_idar_tx_t tx;
	      strncpy( tx.mesg, "Foo", IDARBUFLEN ); 
	      tx.len = strlen( "Foo" );
	      tx.intensity = 64;
	      
	      SIOBufferProperty( buf, idars[0].id, STG_PROP_IDAR_TXRX, 
				 &tx, sizeof(tx), STG_WANTREPLY );
	      
	      stage_buffer_t* res = SIOCreateBuffer();
		
	      result = SIOPropertyUpdate( connection, timestamp, buf, res );

	      SIODebugBuffer( res );

	      SIOFreeBuffer( buf );
	      SIOFreeBuffer( res );
	    }
	  /*
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
	      
	      //result = SIOWriteMessage( connection, timestamp, 
	      //STG_HDR_GUI, (char*)&gui, sizeof(gui) ) ;
	    }
	  */

	  SIOWriteMessage( connection, timestamp, STG_HDR_CONTINUE, NULL, 0 );
	  // don't do anything in here!
	  SIOServiceConnections( &HandleLostConnection,
				 &HandleCommand,
				 &HandleProperty );
	}
    }
    
    return 0;
}









/*
  $Id: stest.c,v 1.1.2.19 2003-02-15 21:15:01 rtv Exp $
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

int HandleModel( int connection, char* data, size_t len )
{
  assert( len == sizeof(stage_model_t) );
  stage_model_t* model = (stage_model_t*)data;
  
  printf( "Received %d bytes model packet on connection %d", (int)len, connection );

  // if we see the key we've been waiting for, the model request is confirmed
  if( model->key == pending_key )
    pending_id = model->id; // find out what id we were given
  
  return 0; //success
}

int HandleProperty( int connection, char* data, size_t len )
{ 
  assert( len >= sizeof(stage_property_t) );
  stage_property_t* prop = (stage_property_t*)data;
  
  printf( "Received %d bytes property (%d,%s,%d) on connection %d\n",
	  (int)len, prop->id, SIOPropString(prop->property), (int)prop->len, connection );
  return 0; //success
}

int HandleCommand( int connection, char* data, size_t len )
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
		     (char*)&sz, sizeof(sz) );
    
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
		     (char*)&ppm, sizeof(ppm) );
  
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
		     (char*)&vel, sizeof(vel) );

  SIOWriteMessage( con, timestamp,
		   STG_HDR_PROPS, bp->data, bp->len );
  
  SIOFreeBuffer( bp );
}

// requests creation of a model. fills in the model.id record with the correct
// id returned from stage. returns -1 on failure, 0 on success
int CreateModel( int connection, stage_model_t* model )
{
  // set up the request packet and local state variables for getting the reply
  model->id = pending_id = -1; // CREATE a model
  model->key = pending_key = rand(); //(some random integer number)
  
  SIOWriteMessage( connection, timestamp, STG_HDR_MODEL, 
		   (char*)model, sizeof(stage_model_t) );
  
  // loop on read until we hear that our model was created
  while( pending_id == -1 )
    SIOServiceConnections( &HandleLostConnection,
			   &HandleCommand,
			   &HandleModel, 
			   NULL, NULL );
  
  // now we know the id Stage gave this model
  model->id = pending_id;
  
  return 0; // success
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

      TEST( "Creating a GUI" );

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
            
      result = SIOWriteMessage( connection, timestamp, 
				STG_HDR_GUI, (char*)&gui, sizeof(gui) ) ;
      
      EVAL(result);
      
      // define some models we want to create
      stage_model_t root;
      root.parent_id = -1; // only the root can have no parent
      strncpy( root.token, "box", STG_TOKEN_MAX );
      assert( CreateModel( connection,  &root ) == 0 );
      
      stage_model_t box;
      box.parent_id = root.id;
      strncpy( box.token, "box", STG_TOKEN_MAX );
      assert( CreateModel( connection,  &box ) == 0 );
      
      stage_model_t bitmap;
      bitmap.parent_id = root.id;
      strncpy( bitmap.token, "box", STG_TOKEN_MAX );
      assert( CreateModel( connection,  &bitmap ) == 0 );

      stage_model_t sonar;
      sonar.parent_id = box.id;
      strncpy( sonar.token, "sonar", STG_TOKEN_MAX );
      assert( CreateModel( connection,  &sonar ) == 0 );
      
      // define some properties
      stage_buffer_t* props = SIOCreateBuffer();
      assert(props);
      
      stage_pose_t pose;
      
      // pose the bitmap
      SIOPackPose( &pose, 4.0, 6.0, 0.0 );      
      SIOBufferProperty( props, bitmap.id, STG_PROP_ENTITY_POSE, 
			 (char*)&pose, sizeof(pose) );
      
      // size the bitmap
      stage_size_t size;
      size.x = 2.0;
      size.y = 2.0;
      SIOBufferProperty( props, bitmap.id, STG_PROP_ENTITY_SIZE,
			 (char*)&size, sizeof(size) );
      

      // pose the box
      SIOPackPose( &pose, 2.0, 1.0, 1.0 );      
      SIOBufferProperty( props, box.id, STG_PROP_ENTITY_POSE, 
      	 (char*)&pose, sizeof(pose) );

      // subscribe to the sonar's data, pose, size and rects.
      int subs[4];
      subs[0] = STG_PROP_ENTITY_DATA;
      subs[1] = STG_PROP_ENTITY_POSE;
      subs[2] = STG_PROP_ENTITY_SIZE;
      subs[3] = STG_PROP_ENTITY_RECTS;
      
      SIOBufferProperty( props, sonar.id, STG_PROP_ENTITY_SUBSCRIBE,
			 (char*)subs, 4*sizeof(subs[0]) );
     
      
      int num_rects;
      stage_rotrect_t* rects = RectsFromPnm( &num_rects, "worlds/cave.pnm" );
      printf( "attempting to buffer %d rects\n", num_rects );

      // send the rectangles to root
      SIOBufferProperty( props, root.id, STG_PROP_ENTITY_RECTS, 
			 (char*)rects, num_rects * sizeof(rects[0]) );

      //rects = RectsFromPnm( &num_rects, "worlds/smiley.ppm" );
      //SIOBufferProperty( props, bitmap.id, STG_PROP_ENTITY_RECTS, 
      //	 (char*)rects, num_rects * sizeof(rects[0]) );
      
      TEST( "Sending properties" );
      result = SIOWriteMessage( connection, timestamp,
				STG_HDR_PROPS, props->data, props->len );
      EVAL(result);
      
      SIOFreeBuffer( props );


      puts( "looping" );
      
      int c = 0;
      double x = 0;
      double  y = 0;
      double z = 0;
      while( c < 1000 )
	{
	  printf( " cycle %d\r", c++ );
	  
	  SIOWriteMessage( connection, timestamp, STG_HDR_CONTINUE, NULL, 0 );
	  
	  //Resize( connection, bitmap.id,  
	  // 0.5 + 3.0 * fabs(sin(x)),  0.5 + 3.0 * fabs(cos(x+=0.05)) );
	 
	  //SetVelocity( connection, box.id, 3.0 * sin(x), 2.0 * cos(x+=0.1), 2.0 );

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
	  
	  SIOServiceConnections( &HandleLostConnection,
				 &HandleCommand,
				 &HandleModel, 
				 &HandleProperty,
				 NULL );
	}
    }
    
    return 0;
}








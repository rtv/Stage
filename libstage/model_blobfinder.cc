 ///////////////////////////////////////////////////////////////////////////
 //
 // File: model_blobfinder.c
 // Author: Richard Vaughan
 // Date: 10 June 2004
 //
 // CVS info:
 //  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_blobfinder.cc,v $
 //  $Author: rtv $
 //  $Revision: 1.1.2.1 $
 //
 ///////////////////////////////////////////////////////////////////////////

 //#define DEBUG 

#include <sys/time.h>
#include "stage_internal.hh"

static const stg_watts_t DEFAULT_BLOBFINDERWATTS = 2.0;
static const stg_meters_t DEFAULT_BLOBFINDERRANGE = 12.0;
static const stg_radians_t DEFAULT_BLOBFINDERFOV = M_PI/3.0;
static const stg_radians_t DEFAULT_BLOBFINDERPAN = 0.0;
static const unsigned int DEFAULT_BLOBFINDERINTERVAL_MS = 100;
static const unsigned int DEFAULT_BLOBFINDERRESOLUTION = 1;
static const unsigned int DEFAULT_BLOBFINDERSCANWIDTH = 80;
static const unsigned int DEFAULT_BLOBFINDERSCANHEIGHT = 60;

 /**
@ingroup model
@defgroup model_blobfinder Blobfinder model

The blobfinder model simulates a color-blob-finding vision device,
like a CMUCAM2, or the ACTS image processing software. It can track
areas of color in a simulated 2D image, giving the location and size
of the color 'blobs'. Multiple colors can be tracked at once; they are
separated into channels, so that e.g. all red objects are tracked as
channel one, blue objects in channel two, etc. The color associated
with each channel is configurable.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
ptz(
  # blobfinder MUST be a child of a PTZ model
  blobfinder
  (
    # blobfinder properties
    colors_count 6
    colors ["red" "green" "blue" "cyan" "yellow" "magenta" ]
    range 8.0
    ptz[0 0 60.0]
    image[80 60]

    # model properties
    size3 [0 0 0]
  )
)
@endverbatim

@par Details
- colors_count int
  - number of colors being tracked
- colors [ string ... ]
  - define the colors detected in each channel, using color names from the X11-style color database 
   The number of strings should match colors_count.
- image [int int]
  - [width height]
  - dimensions of the image in pixels. This determines the blobfinder's 
    resolution
- ptz [float float float]
   - [pan_angle tilt_angle zoom_angle] 
   - control the panning, tilt and fov angle (zoom) of the blobfinder. Tilt angle currently has no effect.
- range float
   - maximum range of the sensor in meters.
 */


StgModelBlobfinder::StgModelBlobfinder( StgWorld* world, 
			      StgModel* parent,
			      stg_id_t id,
			      char* typestr )
  : StgModel( world, parent, id, typestr )
{
  PRINT_DEBUG2( "Constructing StgModelBlobfinder %d (%s)\n", 
		id, typestr );

  scan_width = DEFAULT_BLOBFINDERSCANWIDTH;
  scan_height = DEFAULT_BLOBFINDERSCANHEIGHT;
  fov = DEFAULT_BLOBFINDERFOV;
  range = DEFAULT_BLOBFINDERRANGE;

  ClearBlocks();
  
  blobs = g_array_new( false, true, sizeof(stg_blobfinder_blob_t));

  // leave the color filter array empty initally - this tracks all colors
  colors = g_array_new( false, true, sizeof(stg_color_t) );  
}


StgModelBlobfinder::~StgModelBlobfinder( void )
{
  if( blobs )
    g_array_free( blobs, true );
  
  if( colors )
    g_array_free( colors, true );
}

static bool blob_match( StgBlock* testblock, 
			StgModel* finder,
			const void* dummy )
{ 
  return( ! finder->IsRelated( testblock->Model() ));
}	


static bool ColorMatchIgnoreAlpha( stg_color_t a, stg_color_t b )
{
  return( (a & 0x00FFFFFF) == (b & 0x00FFFFFF ) );
}

void StgModelBlobfinder::StgModelBlobfinder::AddColor( stg_color_t col )
{
  g_array_append_val( colors, col );
}

/** Stop tracking blobs with this color */
void StgModelBlobfinder::RemoveColor( stg_color_t col )
{
  for( unsigned int i=0; i<colors->len; i++ )
    if( col ==  g_array_index( colors, stg_color_t, i ) )      
      g_array_remove_index_fast( colors, i );
}

/** Stop tracking all colors. Call this to clear the defaults, then
    add colors individually with AddColor(); */
void StgModelBlobfinder::RemoveAllColors()
{
  g_array_set_size( colors, 0 );
}

void StgModelBlobfinder::Load( void )
{  
  StgModel::Load(); 
  
  Worldfile* wf = world->GetWorldFile();
  
  scan_width = (int)wf->ReadTupleFloat( id, "image", 0, scan_width );
  scan_height= (int)wf->ReadTupleFloat( id, "image", 1, scan_height );
  range = wf->ReadFloat( id, "range", range );
  fov = wf->ReadAngle( id, "fov", fov );
  pan = wf->ReadAngle( id, "pan", pan );
  
  if( wf->PropertyExists( id, "colors" ) )
    {
      RemoveAllColors(); // empty the color list to start from scratch
      
      unsigned int count = wf->ReadFloat( id, "colors_count", 0 );
      
      for( unsigned int c=0; c<count; c++ )
	{
	  const char* colorstr = 
	    wf->ReadTupleString( id, "colors", c, NULL );
	  
	  if( ! colorstr )
	    break;
	  else
	    AddColor( stg_lookup_color( colorstr ));
	}
    }    
}


void StgModelBlobfinder::Update( void )
{     
  StgModel::Update();
  
  // generate a scan for post-processing into a blob image
  
  stg_raytrace_sample_t* samples = new stg_raytrace_sample_t[scan_width];

  Raytrace( pan, range, fov, blob_match, NULL, samples, scan_width );

  
  // now the colors and ranges are filled in - time to do blob detection
  double yRadsPerPixel = fov / scan_height; 

  g_array_set_size( blobs, 0 );

  // scan through the samples looking for color blobs
  for(unsigned int s=0; s < scan_width; s++ )
    {
      if( samples[s].block == NULL  )
	continue; // we saw nothing
      
      //if( samples[s].block->Color() != 0xFFFF0000 )
      //continue; // we saw nothing
      
      unsigned int blobleft = s;
      stg_color_t blobcol = samples[s].block->Color();
      
      //printf( "blob start %d color %X\n", blobleft, blobcol );
      
      // loop until we hit the end of the blob
      // there has to be a gap of >1 pixel to end a blob
      // this avoids getting lots of crappy little blobs
      while( s < scan_width && samples[s].block && 
	     ColorMatchIgnoreAlpha( samples[s].block->Color(), blobcol) )
	{
	  //printf( "%u blobcol %X block %p %s color %X\n", s, blobcol, samples[s].block, samples[s].block->Model()->Token(), samples[s].block->Color() );
	  s++;
	}
            
      unsigned int blobright = s-1;
      
      //if we have color filters in place, check to see if we're looking for this color
      if( colors->len )
	{
	  bool found = false;
	  
	  for( unsigned int c=0; c<colors->len; c++ )
	    if( ColorMatchIgnoreAlpha( blobcol, g_array_index( colors, stg_color_t, c )))
	      {
		found = true;
		break;
	      }
	  if( ! found )
	    continue; // continue scanning array for next blob
	}

      //printf( "blob end %d %X\n", blobright, blobcol );
      
      double robotHeight = 0.6; // meters

      // find the average range to the blob;
      stg_meters_t range = 0;
      for( unsigned int t=blobleft; t<=blobright; t++ )
	range += samples[t].range;
      range /= blobright-blobleft + 1;

      double startyangle = atan2( robotHeight/2.0, range );
      double endyangle = -startyangle;
      int blobtop = scan_height/2 - (int)(startyangle/yRadsPerPixel);
      int blobbottom = scan_height/2 -(int)(endyangle/yRadsPerPixel);
      
      blobtop = MIN( blobtop, (int)scan_height );
      blobbottom = MIN( blobbottom, (int)scan_height );

      // fill in an array entry for this blob
      stg_blobfinder_blob_t blob;
      memset( &blob, 0, sizeof(blob) );      
      blob.color = blobcol;
      blob.left = blobleft;
      blob.top = blobtop;
      blob.right = blobright;
      blob.bottom = blobbottom;
      blob.range = range;
      
      //printf( "Robot %p sees %d xpos %d ypos %d\n",
      //  mod, blob.color, blob.xpos, blob.ypos );
      
      // add the blob to our stash
      g_array_append_val( blobs, blob );
    }
  
  delete [] samples;

  data_dirty = true;
}


void StgModelBlobfinder::Startup(  void )
{ 
  StgModel::Startup();
  
  PRINT_DEBUG( "blobfinder startup" );
  
  // start consuming power
  SetWatts( DEFAULT_BLOBFINDERWATTS );
}

void StgModelBlobfinder::Shutdown( void )
{ 
  
  PRINT_DEBUG( "blobfinder shutdown" );
  
  // stop consuming power
  SetWatts( 0 );
  
  // clear the data - this will unrender it too
  if( blobs )
    g_array_set_size( blobs, 0 );
  
  StgModel::Shutdown();
}

void StgModelBlobfinder::DataVisualize( void )
{
 if( debug )
    {
      // draw the FOV
      GLUquadric* quadric = gluNewQuadric();
      
      PushColor( 0,0,0,0.2  );
      
      gluQuadricDrawStyle( quadric, GLU_SILHOUETTE );
      //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      gluPartialDisk( quadric,
		      0, 
		      range,
		      20, // slices	
		      1, // loops
		      rtod( M_PI/2.0 + fov/2.0 - pan), // start angle
		      rtod(-fov) ); // sweep angle
      
      gluDeleteQuadric( quadric );
      PopColor();
    }

 if( subs < 1 )
   return;

  glPushMatrix();

  // return to global rotation frame
  stg_pose_t gpose = GetGlobalPose();
  glRotatef( 180 + rtod(-gpose.a),0,0,1 );
  
  // place the "screen" a little away from the robot
  glTranslatef( 0.5, 0.5, 1.0 );

  // convert blob pixels to meters scale - arbitrary
  glScalef( 0.025, 0.025, 1 );

  // draw a white screen with a black border
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  PushColor( 0xFFFFFFFF );
  glRectf( 0,0, scan_width, scan_height );
  PopColor();

  glTranslatef(0,0,0.01 );

  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  PushColor( 0xFF000000 );
  glRectf( 0,0, scan_width, scan_height );
  PopColor();

  // draw the blobs on the screen
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  for( unsigned int s=0; s<blobs->len; s++ )
    {
      stg_blobfinder_blob_t* b = 
	&g_array_index( blobs, stg_blobfinder_blob_t, s);
      
      PushColor( b->color );
      glRectf( b->left, b->top, b->right, b->bottom );

      //printf( "%u l %u t%u r %u b %u\n", s, b->left, b->top, b->right, b->bottom );
      PopColor();
    }

  glPopMatrix();

}



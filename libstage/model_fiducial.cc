///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_fiducial.cc,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG 1

#include <assert.h>
#include <math.h>
#include "stage_internal.hh"

const stg_meters_t DEFAULT_FIDUCIAL_RANGEMIN = 0.0;
const stg_meters_t DEFAULT_FIDUCIAL_RANGEMAXID = 5.0;
const stg_meters_t DEFAULT_FIDUCIAL_RANGEMAXANON = 8.0;
const stg_radians_t DEFAULT_FIDUCIAL_FOV = M_PI;
const stg_watts_t DEFAULT_FIDUCIAL_WATTS = 10.0;

/** 
@ingroup model
@defgroup model_fiducial Fiducial detector model

The fiducial model simulates a fiducial-detecting device.

API: Stg::StgModelFiducial

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
fiducialfinder
(
  # fiducialfinder properties
  range_min 0.0
  range_max 8.0
  range_max_id 5.0
  fov 180.0

  # model properties
  size [0 0]
)
@endverbatim

@par Details
- range_min float
  - the minimum range reported by the sensor, in meters. The sensor will detect objects closer than this, but report their range as the minimum.
- range_max float
  - the maximum range at which the sensor can detect a fiducial, in meters. The sensor may not be able to uinquely identify the fiducial, depending on the value of range_max_id.
- range_max_id float
- the maximum range at which the sensor can detect the ID of a fiducial, in meters.
- fov float
- the angular field of view of the scanner, in degrees. 

*/

StgModelFiducial::StgModelFiducial( StgWorld* world, 
				    StgModel* parent,
				    stg_id_t id,
				    char* typestr )
: StgModel( world, parent, id, typestr )
{
  PRINT_DEBUG2( "Constructing StgModelFiducial %d (%s)\n", 
		id, typestr );
  
  // sensible fiducial defaults 
  //  interval = 200; // common for a SICK LMS200

  this->ClearBlocks();

  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom));
  SetGeom( geom );

  // default parameters
  min_range = DEFAULT_FIDUCIAL_RANGEMIN;
  max_range_anon = DEFAULT_FIDUCIAL_RANGEMAXANON;
  max_range_id = DEFAULT_FIDUCIAL_RANGEMAXID;
  fov = DEFAULT_FIDUCIAL_FOV;
  key = 0;

  data = g_array_new( false, true, sizeof(stg_fiducial_t) );
}

StgModelFiducial::~StgModelFiducial( void )
{
  if( data )
    g_array_free( data, true );
}

static bool fiducial_raytrace_match( StgBlock* testblock, StgModel* finder, const void* dummy )
{
  return( ! finder->IsRelated( testblock->Model() ) );
}	


void StgModelFiducial::AddModelIfVisible( StgModel* him )  
{
  //PRINT_DEBUG2( "Fiducial %s is testing model %s", token, him->Token() );

  // don't consider models with invalid returns  
  if( him->FiducialReturn() == 0 )
    {
      //PRINT_DEBUG1( "  but model %s has a zero fiducial ID", him->Token());
      return;
    }

  // check to see if this neighbor has the right fiducial key
  if( fiducial_key != him->FiducialKey() )
    {
      //PRINT_DEBUG1( "  but model %s doesn't match the fiducial key", him->Token());
      return;
    }

  stg_pose_t mypose = this->GetGlobalPose();

  // are we within range?
  stg_pose_t hispose = him->GetGlobalPose();
  double dx = hispose.x - mypose.x;
  double dy = hispose.y - mypose.y;
  double range = hypot( dy, dx );

  //  printf( "range to target %.2f m (

  if( range > max_range_anon )
    {
      //PRINT_DEBUG3( "  but model %s is %.2f m away, outside my range of %.2f m", 
      //	    him->Token(),
      //	    range,
      //	    max_range_anon );
      return;
    }
  
  // is he in my field of view?
  double bearing = atan2( dy, dx );
  double dtheta = normalize(bearing - mypose.a);
  
  if( fabs(dtheta) > fov/2.0 )
    {
      //PRINT_DEBUG1( "  but model %s is outside my FOV", him->Token());
      return;
    }
  
  if( IsRelated( him ) )
    return;

   PRINT_DEBUG1( "  %s is a candidate. doing ray trace", him->Token());
  

  //printf( "bearing %.2f\n", RTOD(bearing) );

  stg_raytrace_sample_t ray;

  Raytrace( dtheta,
	    max_range_anon,
	    fiducial_raytrace_match,
	    NULL,
	    &ray,
	    false );
  
  range = ray.range;
  StgModel* hitmod = ray.block->Model();

  //  printf( "ray hit %s and was seeking LOS to %s\n",
  //hitmod ? hitmod->Token() : "null",
  //him->Token() );
  
  // if it was him, we can see him
  if( hitmod == him )
    {
      stg_geom_t hisgeom = him->GetGeom();
      
      // record where we saw him and what he looked like
      stg_fiducial_t fid;
      fid.range = range;
      fid.bearing = dtheta;
      fid.geom.x = hisgeom.size.x;
      fid.geom.y = hisgeom.size.y;
      fid.geom.a = normalize( hispose.a - mypose.a);
      
      // store the global pose of the fiducial (mainly for the GUI)
      memcpy( &fid.pose, &hispose, sizeof(fid.pose));
      
      // if he's within ID range, get his fiducial.return value, else
      // we see value 0
      fid.id = range < max_range_id ? hitmod->FiducialReturn() : 0;
      
      PRINT_DEBUG2( "adding %s's value %d to my list of fiducials",
		    him->Token(), him->FiducialReturn() );
      
      g_array_append_val( data, fid );
    }
  
  fiducials = (stg_fiducial_t*)data->data;
  fiducial_count = data->len;
}
  
  
///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void StgModelFiducial::Update( void )
{
  StgModel::Update();
  
  PRINT_DEBUG( "fiducial update" );
  
  if( subs < 1 )
    return;
  
  // reset the array of detected fiducials
  data = g_array_set_size( data, 0 );
  
  // for all the objects in the world, test if we can see them as
  // fiducials
  
  // TODO - add a fiducial-only hash table to the world to speed this
  // up a lot for large populations
  world->ForEachModel( (GHFunc)(StgModelFiducial::AddModelIfVisibleStatic), 
		       this );
  
  PRINT_DEBUG2( "model %s saw %d fiducials", token, data->len );
}
 
 void StgModelFiducial::Load( void )
 {  
   PRINT_DEBUG( "fiducial load" );
   
   StgModel::Load();
   
   Worldfile* wf = world->GetWorldFile();
   
   // load fiducial-specific properties
   min_range      = wf->ReadLength( id, "range_min",    min_range );
   max_range_anon = wf->ReadLength( id, "range_max",    max_range_anon );
   max_range_id   = wf->ReadLength( id, "range_max_id", max_range_id );
   fov            = wf->ReadAngle(  id, "fov",          fov );
 }  
 

 void StgModelFiducial::DataVisualize()
 {
   // draw the FOV
//    GLUquadric* quadric = gluNewQuadric();

//    PushColor( 0,0,0,0.2  );

//    gluQuadricDrawStyle( quadric, GLU_SILHOUETTE );
//    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
//    gluPartialDisk( quadric,
// 		   0, 
// 		   max_range_anon,
// 		   20, // slices	
// 		   1, // loops
// 		   rtod( M_PI/2.0 + fov/2.0), // start angle
// 		   rtod(-fov) ); // sweep angle

//    gluDeleteQuadric( quadric );
//    PopColor();

   if( data->len == 0 )
     return;
  
   // draw the fiducials
   
   for( unsigned int f=0; f<data->len; f++ )
     {
       stg_fiducial_t fid = g_array_index( data, stg_fiducial_t, f );
       
       PushColor( 0,0,0,0.7  );

       double dx = fid.range * cos( fid.bearing);
       double dy = fid.range * sin( fid.bearing);

       glBegin( GL_LINES );
       glVertex2f( 0,0 );
       glVertex2f( dx, dy );
       glEnd();

       glPushMatrix();
       gl_coord_shift( dx,dy,0,fid.geom.a );
       
       glPolygonMode( GL_FRONT, GL_LINE );
       glRectf( -fid.geom.x/2.0, -fid.geom.y/2.0,
		fid.geom.x/2.0, fid.geom.y/2.0 );

       // show the fiducial ID
       char idstr[32];
       snprintf(idstr, 31, "%d", fid.id );

       PushColor( 0,0,0,1 ); // black
       gl_draw_string( 0,0,0, idstr );
       PopColor();

       glPopMatrix();
       
       PopColor();
       

     }
 }

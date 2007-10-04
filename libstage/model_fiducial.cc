///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_fiducial.cc,v $
//  $Author: rtv $
//  $Revision: 1.1.2.1 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG 

#include <assert.h>
#include <math.h>
#include "stage_internal.h"
#include "gui.h"

#define STG_FIDUCIALS_MAX 64
#define STG_DEFAULT_FIDUCIAL_RANGEMIN 0
#define STG_DEFAULT_FIDUCIAL_RANGEMAXID 5
#define STG_DEFAULT_FIDUCIAL_RANGEMAXANON 8
#define STG_DEFAULT_FIDUCIAL_FOV DTOR(180)

const double STG_FIDUCIAL_WATTS = 10.0;

/** 
@ingroup model
@defgroup model_fiducial Fiducial detector model

The fiducial model simulates a fiducial-detecting device.

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

StgModelFiducial::StgModelFiducial( stg_world_t* world, 
			      StgModel* parent,
			      stg_id_t id,
			      CWorldFile* wf )
  : StgModel( world, parent, id, wf )
{
  PRINT_DEBUG2( "Constructing StgModelFiducial %d (%s)\n", 
		id, wf->GetEntityType( id ) );
  
  // sensible fiducial defaults 
  this->update_interval_ms = 200; // common for a SICK LMS200

  stg_geom_t geom; 
  memset( &geom, 0, sizeof(geom));
  geom.size.x = STG_DEFAULT_FIDUCIAL_SIZEX;
  geom.size.y = STG_DEFAULT_FIDUCIAL_SIZEY;
  geom.size.z = STG_DEFAULT_FIDUCIAL_SIZEZ;
  this->SetGeom( &geom );

  // set default color
  this->SetColor( stg_lookup_color(STG_FIDUCIAL_GEOM_COLOR));

  // set up a fiducial-specific config structure
  stg_fiducial_config_t lconf;
  memset(&lconf,0,sizeof(lconf));  
  lconf.range_min   = STG_DEFAULT_FIDUCIAL_MINRANGE;
  lconf.range_max   = STG_DEFAULT_FIDUCIAL_MAXRANGE;
  lconf.fov         = STG_DEFAULT_FIDUCIAL_FOV;
  lconf.samples     = STG_DEFAULT_FIDUCIAL_SAMPLES;  
  lconf.resolution = 1;
  this->SetCfg( &lconf, sizeof(lconf) );

  this->SetData( NULL, 0 );

  this->ClearBlocks();

  // default parameters
  min_range = STG_DEFAULT_FIDUCIAL_RANGEMIN;
  max_range_anon = STG_DEFAULT_FIDUCIAL_RANGEMAXANON;
  max_range_id = STG_DEFAULT_FIDUCIAL_RANGEMAXID;
  fov = STG_DEFAULT_FIDUCIAL_FOV;  
}

StgModelFiducial::~StgModelFiducial( void )
{
  // nothing to do. C++ is not pretty.
}

typedef struct 
{
  stg_model_t* mod;
  stg_pose_t pose;
  stg_fiducial_config_t cfg;
  GArray* fiducials;
} model_fiducial_buffer_t;


int fiducial_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  return( !stg_model_is_related(mod,hitmod) );
}	


void StgModelFiducial::AddModelIfVisible( StgModel* him )  
{
  PRINT_DEBUG2( "Fiducial %s is testing model %s", token, him->Token() );

  // don't consider models with invalid returns  
  if( him->fiducial_return == 0 )
    {
      PRINT_DEBUG1( "  but model %s has a zero fiducial ID", him->token);
      return;
    }

  // check to see if this neighbor has the right fiducial key
  if( fiducial_key != him->fiducial_key )
    {
      PRINT_DEBUG1( "  but model %s doesn't match the fiducial key", him->token);
      return;
    }

  stg_pose_t mypose;
  this->GetGlobalPose( &mypose );

  // are we within range?
  stg_pose_t hispose;
  him->GetGlobalPose( &hispose );  
  double dx = hispose.x - mfb->pose.x;
  double dy = hispose.y - mfb->pose.y;  
  double range = hypot( dy, dx );
  if( range > max_range_anon )
    {
      PRINT_DEBUG1( "  but model %s is outside my range", him->token);
      return;
    }
  
  // is he in my field of view?
  double hisbearing = atan2( dy, dx );
  double dif = mypose.a - hisbearing;

  if( fabs(NORMALIZE(dif)) > fov/2.0 )
    {
      PRINT_DEBUG1( "  but model %s is outside my FOV", him->token);
      return;
    }
   
  // now check if we have line-of-sight
  itl_t *itl = itl_create( mypose.x, mypose.y,
			   hispose.x, hispose.y, 
			   world->matrix, PointToPoint );
  
  stg_model_t* hitmod = 
    itl_first_matching( itl, fiducial_raytrace_match, mfb->mod );
  
  itl_destroy( itl )

  if( hitmod )
    PRINT_DEBUG2( "I saw %s with fid %d",
		  hitmod->token, hitmod->fiducial_return );
  
       if( (hitmod = stg_first_model_on_ray( mypose.x, mypose.y, mypose.z,
					     hispose.x, hispose.y, 
					     range_max,
					     world->matrix,
					     PointToPoint,
					     laser_raytrace_match,
					     this,
					     &hitrange, &hitx, &hity )))
	 {


  // if it was him, we can see him
  if( hitmod == him )
    {
      stg_geom_t hisgeom;
      stg_model_get_geom(him,&hisgeom);

      // record where we saw him and what he looked like
      stg_fiducial_t fid;      
      fid.range = range;
      fid.bearing = NORMALIZE( hisbearing - mfb->pose.a);
      fid.geom.x = hisgeom.size.x;
      fid.geom.y = hisgeom.size.y;
      fid.geom.a = NORMALIZE( hispose.a - mfb->pose.a);

      // store the global pose of the fiducial (mainly for the GUI)
      memcpy( &fid.pose, &hispose, sizeof(fid.pose));
      
      // if he's within ID range, get his fiducial.return value, else
      // we see value 0
      fid.id = range < mfb->cfg.max_range_id ? hitmod->fiducial_return : 0;

      PRINT_DEBUG2( "adding %s's value %d to my list of fiducials",
      	    him->token, him->fiducial_return );

      g_array_append_val( mfb->fiducials, fid );
    }
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void StgModelFiducial::Update( void )
{
  PRINT_DEBUG( "fiducial update" );
  
  if( subs < 1 )
    return 0;
  
  model_fiducial_buffer_t mfb;
  memset( &mfb, 0, sizeof(mfb) );
  
  mfb.mod = mod;
  mfb.cfg...;
  
  mfb.fiducials = g_array_new( FALSE, TRUE, sizeof(stg_fiducial_t) );

  this->GetGlobalPose( &mfb.pose );
  
  // for all the objects in the world
  g_hash_table_foreach( world->models, 
			(GHFunc)(StgModelFiducial::AddModelIfVisibleStatic, this );

  PRINT_DEBUG2( "model %s saw %d fiducials", token, mfb.fiducials->len );
  
  stg_model_set_data( mod, mfb.fiducials->data, 
		      mfb.fiducials->len * sizeof(stg_fiducial_t) );
  
  g_array_free( mfb.fiducials, TRUE );

  return 0;
}

void StgModelFiducial::Load( void )
{  
  StgModel::Load();
  
  // load fiducial-specific properties
  min_range      = wf->ReadLength( id, "range_min",    min_range );
  max_range_anon = wf->ReadLength( id, "range_max",    max_range_anon );
  max_range_id   = wf->ReadLength( id, "range_max_id", max_range_id );
  fov            = wf->ReadAngle(  id, "fov",          fov );
}  

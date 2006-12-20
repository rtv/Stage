///////////////////////////////////////////////////////////////////////////
//
// File: model_bumper.c
// Author: Richard Vaughan
// Date: 28 March 2006
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_bumper.c,v $
//  $Author: rtv $
//  $Revision: 1.1.4.2 $
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_bumper Bumper/Whisker  model 
The bumper model simulates an array of binary touch sensors.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
bumper 
(
  # bumper properties
  bcount 1
  bpose[0] [ 0 0 0 ]
  blength 0.1
)
@endverbatim

@par Notes

The bumper model allows configuration of the pose and length parameters of each transducer seperately using bpose[index] and blength[index]. For convenience, the length of all bumpers in the array can be set at once with the blength property. If a blength with no index is specified, this global setting is applied first, then specific blengh[index] properties are applied afterwards. Note that the order in the worldfile is ignored.

@par Details
- bcount int 
  - the number of bumper transducers
- bpose[\<transducer index\>] [float float float]
  - [x y theta] 
  - pose of the center of the transducer relative to its parent.
- blength float
  - sets the length in meters of all transducers in the array
- blength[\<transducer index\>] float
  - length in meters of a specific transducer. This is applied after the global setting above.

*/

#include <math.h>
#include "stage_internal.h"
#include "gui.h"

static const stg_watts_t STG_BUMPER_WATTS = 0.1; // bumper power consumption

static int bumper_update( stg_model_t* mod, void* unused );
static int bumper_startup( stg_model_t* mod, void* unused );
static int bumper_shutdown( stg_model_t* mod, void* unused );
static int bumper_load( stg_model_t* mod, void* unused );

// implented by the gui in some other file
void gui_speech_init( stg_model_t* mod );


int bumper_init( stg_model_t* mod )
{  
  stg_model_add_callback( mod, &mod->startup, bumper_startup, NULL );
  stg_model_add_callback( mod, &mod->shutdown, bumper_shutdown, NULL );
  stg_model_add_callback( mod, &mod->load, bumper_load, NULL );

  // update is added/removed in startup/shutdown
  //stg_model_add_callback( mod, &mod->update, bumper_update, NULL );


  stg_model_set_data( mod, NULL, 0 );
  
  // Set up sensible defaults
  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom)); // no size
  stg_model_set_geom( mod, &geom );
    
  // remove the polygon: bumper has no body
  stg_model_set_polygons( mod, NULL, 0 );

  stg_bumper_config_t cfg;
  memset( &cfg, 0, sizeof(cfg));
  cfg.length = 0.1;
  stg_model_set_cfg( mod, &cfg, sizeof(cfg) );
  
  gui_bumper_init(mod);

  return 0;
}

int bumper_startup( stg_model_t* mod, void* unused )
{
  PRINT_DEBUG( "bumper startup" ); 
  stg_model_add_callback( mod, &mod->update, bumper_update, NULL );
  stg_model_set_watts( mod, STG_BUMPER_WATTS );
  return 0;
}


int bumper_shutdown( stg_model_t* mod, void* unused )
{
  PRINT_DEBUG( "bumper shutdown" );
  stg_model_remove_callback( mod, &mod->update, bumper_update );
  stg_model_set_watts( mod, 0 );
  stg_model_set_data( mod, NULL, 0 );
  return 0;
}

int bumper_load( stg_model_t* mod, void* unused )
{
  // Load the geometry of a bumper array
  int bcount = wf_read_int( mod->id, "bcount", 0);
  if (bcount > 0)
    {
      char key[256];
      
      size_t cfglen = bcount * sizeof(stg_bumper_config_t); 
      stg_bumper_config_t* cfg = (stg_bumper_config_t*)
	calloc( sizeof(stg_bumper_config_t), bcount ); 
      
      double common_length = wf_read_length(mod->id, "blength", cfg[0].length );

      // set all transducers with the common setting
      int i;
      for(i = 0; i < bcount; i++)
	{
	  cfg[i].length = common_length;
	}

      // allow individual configuration of transducers
      for(i = 0; i < bcount; i++)
	{
	  snprintf(key, sizeof(key), "bpose[%d]", i);
	  cfg[i].pose.x = wf_read_tuple_length(mod->id, key, 0, 0);
	  cfg[i].pose.y = wf_read_tuple_length(mod->id, key, 1, 0);
	  cfg[i].pose.a = wf_read_tuple_angle(mod->id, key, 2, 0);	 

	  snprintf(key, sizeof(key), "blength[%d]", i);
	  cfg[i].length = wf_read_length(mod->id, key, common_length );	 
	}
      
      stg_model_set_cfg( mod, cfg, cfglen );      
      free( cfg );
    }
  return 0;
}

int bumper_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  return( hitmod->obstacle_return && !stg_model_is_related(mod,hitmod) );
}	


int bumper_update( stg_model_t* mod, void* unused )
{     
  //if( mod->subs < 1 )
  //return 0;

  //PRINT_DEBUG1( "[%d] updating bumpers", mod->world->sim_time );
  size_t len = mod->cfg_len;
  stg_bumper_config_t *cfg = (stg_bumper_config_t*)mod->cfg;

  if( len < sizeof(stg_bumper_config_t) )
    return 0; // nothing to see here
  
  int rcount = len / sizeof(stg_bumper_config_t);
  
  static stg_bumper_sample_t* data = NULL;
  static size_t last_datalen = 0;

  size_t datalen = sizeof(stg_bumper_sample_t) *  rcount; 

  if( datalen != last_datalen )
    {
      data = (stg_bumper_sample_t*)realloc( data, datalen );
      last_datalen = datalen;
    }

  memset( data, 0, datalen );

  int t;
  for( t=0; t<rcount; t++ )
    {
      // get the sensor's pose in global coords
      stg_pose_t pz;
      memcpy( &pz, &cfg[t].pose, sizeof(pz) );

      pz.a = pz.a + M_PI/2.0;
      pz.x -= cfg[t].length/2.0 * cos(pz.a);
      //pz.y -= cfg[t].length/2.0 * sin(cfg[t].pose.a );
      
      stg_model_local_to_global( mod, &pz );
      
      itl_t* itl = itl_create( pz.x, pz.y, pz.a,
			       cfg[t].length,
			       mod->world->matrix,
			       PointToBearingRange );
      
      stg_model_t * hitmod = 
	itl_first_matching( itl, bumper_raytrace_match, mod );
      
      if( hitmod )
	{
	  data[t].hit = hitmod;
	  data[t].hit_point.x = itl->x;
	  data[t].hit_point.y = itl->y;
	}

      itl_destroy( itl );
    }
  
  stg_model_set_data( mod, data, datalen );

  return 0;
}


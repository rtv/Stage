///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_fiducial.c,v $
//  $Author: rtv $
//  $Revision: 1.3 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <math.h>
#include "model.h"
#include "raytrace.h"

#include "gui.h"
extern rtk_fig_t* fig_debug;

void model_fiducial_init( model_t* mod )
{
  stg_fiducial_config_t* fid = calloc( sizeof(stg_fiducial_config_t), 1 );
  memset( fid, 0, sizeof(stg_fiducial_config_t) );
  fid->max_range_anon = 8.0;
  fid->max_range_id = 4.0;
  
  // a sensible default fiducial return value is the model's id
  mod->fiducial_return = mod->id;
  
  // add this property to the model
  model_set_prop_generic( mod, STG_PROP_FIDUCIALCONFIG, 
			  fid, sizeof(stg_fiducial_config_t) );
  
  /*
    mod->max_range_anon = worldfile->ReadLength(0, "lbd_range_anon",
    mod->max_range_anon);
    mod->max_range_anon = worldfile->ReadLength(section, "range_anon",
    mod->max_range_anon);
    mod->max_range_id = worldfile->ReadLength(0, "lbd_range_id",
    mod->max_range_id);
    mod->max_range_id = worldfile->ReadLength(section, "range_id",
    mod->max_range_id);
  */
}


typedef struct 
{
  model_t* mod;
  stg_pose_t pose;
  stg_fiducial_config_t* cfg;
  GArray* fiducials;
} model_fiducial_buffer_t;

void model_fiducial_check_neighbor( gpointer key, gpointer value, gpointer user )
{
  model_fiducial_buffer_t* mfb = (model_fiducial_buffer_t*)user;
  model_t* him = (model_t*)value;
  
  // dont' compare a model with itself, and don't consider models with
  // fiducial returns less than 1
  if( him == mfb->mod || him->fiducial_return < 1 ) return; 

  stg_pose_t hispose;
  model_global_pose( him, &hispose );
  
  double dx = mfb->pose.x - hispose.x;
  double dy = mfb->pose.y - hispose.y;
  
  // are we within range?
  double range = hypot( dy, dx );
  
  //printf( "range = %fm\n", range );
  
  if( range < mfb->cfg->max_range_anon )
    {
      //printf( "in range!\n" );

      // now check if we have line-of-sight
      itl_t *itl = itl_create( mfb->pose.x, mfb->pose.y,
			       hispose.x, hispose.y, 
			       him->world->matrix, PointToPoint );
      
      // iterate until we hit something solid
      model_t* hitmod;
      while( (hitmod = itl_next( itl )) ) 
	{
	  if( hitmod != mfb->mod && hitmod->obstacle_return ) 
	    break;
	}
      
      // if it was him, we can see him
      if( hitmod == him )
	{
	  // record where we saw him and what he looked like
	  stg_fiducial_t fid;      
	  fid.range = range;
	  fid.bearing = NORMALIZE(M_PI + atan2( dy, dx ) - mfb->pose.a);
	  fid.id = range < mfb->cfg->max_range_id ? him->id : 0;
	  fid.geom.x = him->size.x;
	  fid.geom.y = him->size.y;
	  fid.geom.a = NORMALIZE(him->pose.a - mfb->pose.a);
	  	  
	  g_array_append_val( mfb->fiducials, fid );
	}
    }
  //else
  //printf( "out of range\n" );
}

///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void model_fiducial_update( model_t* mod )
{
  // first clear ny old data
  model_set_prop_generic( mod, STG_PROP_FIDUCIALDATA, 
			  NULL, 0 );
  
  if( fig_debug ) rtk_fig_clear( fig_debug );
  
  model_fiducial_buffer_t mfb;
  memset( &mfb, 0, sizeof(mfb) );
  
  mfb.mod = mod;
  mfb.cfg = model_get_prop_data_generic( mod,STG_PROP_FIDUCIALCONFIG ); 
  mfb.fiducials = g_array_new( FALSE, TRUE, sizeof(stg_fiducial_t) );
  model_global_pose( mod, &mfb.pose );
    
  // for all the objects in the world
  g_hash_table_foreach( mod->world->models, model_fiducial_check_neighbor, &mfb );
  
  if( mfb.fiducials->len > 0 )
    {
      //printf( "storing %d fiducials\n", mfb.fiducials->len );
      model_set_prop_generic( mod, STG_PROP_FIDUCIALDATA, 
			      mfb.fiducials->data, 
			      mfb.fiducials->len * sizeof(stg_fiducial_t) );
      
      g_array_free( mfb.fiducials, TRUE );
    }
}

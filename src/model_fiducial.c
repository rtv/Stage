///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_fiducial.c,v $
//  $Author: rtv $
//  $Revision: 1.9 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <math.h>
#include "model.h"
#include "raytrace.h"
#include "gui.h"
extern rtk_fig_t* fig_debug;


void model_fiducial_config_init( model_t* mod );
int model_fiducial_config_set( model_t* mod, void* config, size_t len );
int model_fiducial_data_shutdown( model_t* mod );
int model_fiducial_data_service( model_t* mod );
int model_fiducial_data_set( model_t* mod, void* data, size_t len );
void model_fiducial_return_init( model_t* mod );

void model_fiducial_register(void)
{ 
  PRINT_DEBUG( "FIDUCIAL INIT" );  

  model_register_init( STG_PROP_FIDUCIALCONFIG, model_fiducial_config_init );
  model_register_set( STG_PROP_FIDUCIALCONFIG, model_fiducial_config_set );

  model_register_set( STG_PROP_FIDUCIALDATA, model_fiducial_data_set );
  model_register_shutdown( STG_PROP_FIDUCIALDATA, model_fiducial_data_shutdown );
  model_register_service( STG_PROP_FIDUCIALDATA, model_fiducial_data_service );

  model_register_init( STG_PROP_FIDUCIALRETURN, model_fiducial_return_init );

}

void model_fiducial_return_init( model_t* mod )
{
  int val = mod->id;
  model_set_prop_generic( mod, STG_PROP_FIDUCIALRETURN, &val, sizeof(val) );
}


int model_fiducial_return( model_t* mod )
{
  int* val = model_get_prop_data_generic( mod, STG_PROP_FIDUCIALRETURN );
  assert(val);
  return *val;
}


void model_fiducial_config_init( model_t* mod )
{
  stg_fiducial_config_t newfc; // init a basic config
  newfc.min_range = 0.0;
  newfc.max_range_anon = 5.0;
  newfc.max_range_id = 5.0;
  newfc.fov = M_PI;
  
  model_set_prop_generic( mod, STG_PROP_FIDUCIALCONFIG, &newfc, sizeof(newfc) );
}

void model_fiducial_data_init( model_t* mod )
{
  model_set_prop_generic( mod, STG_PROP_RANGERDATA, NULL, 0 );
}

// convenience 
stg_fiducial_config_t* model_fiducial_config_get( model_t* mod )
{
  stg_fiducial_config_t* cf = model_get_prop_data_generic( mod, STG_PROP_FIDUCIALCONFIG );
  assert(cf);
  return cf;
}

// convenience 
stg_fiducial_t* model_fiducial_data_get( model_t* mod, size_t* count  )
{
  stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_FIDUCIALDATA );
  assert(prop);
  
  stg_fiducial_t* fds = prop->data;
  *count = prop->len / sizeof(stg_fiducial_t);
  return fds;
}

int model_fiducial_data_set( model_t* mod, void* data, size_t len )
{  
  // store the data
  model_set_prop_generic( mod, STG_PROP_FIDUCIALDATA, data, len );
    
  // and redraw it
  model_fiducial_data_render( mod );

  return 0; //OK
}
 
int model_fiducial_config_set( model_t* mod, void* config, size_t len )
{  
  // store the config
  model_set_prop_generic( mod, STG_PROP_FIDUCIALCONFIG, config, len );
  
  // and redraw it
  model_fiducial_config_render( mod);

  return 0; //OK
}
 

int model_fiducial_data_shutdown( model_t* mod )
{
  //PRINT_WARN( "fiducial shutdown" );
  
  model_remove_prop_generic( mod, STG_PROP_FIDUCIALDATA );
 model_fiducial_data_render( mod);
  return 0;
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
  if( him == mfb->mod || model_fiducial_return(him) < 1 ) return; 

  stg_pose_t hispose;
  model_global_pose( him, &hispose );
  
  double dx = hispose.x - mfb->pose.x;
  double dy = hispose.y - mfb->pose.y;
  
  // are we within range?
  double range = hypot( dy, dx );
  if( range > mfb->cfg->max_range_anon )
    return;
  
  // is he in my field of view?
  double hisbearing = atan2( dy, dx );
  double dif = mfb->pose.a - hisbearing;

  if( fabs(NORMALIZE(dif)) > mfb->cfg->fov/2.0 )
    return;
   
  // now check if we have line-of-sight
  itl_t *itl = itl_create( mfb->pose.x, mfb->pose.y,
			   hispose.x, hispose.y, 
			   him->world->matrix, PointToPoint );
  
  // iterate until we hit something solid
  model_t* hitmod;
  while( (hitmod = itl_next( itl )) ) 
    {
      if( hitmod != mfb->mod && model_obstacle_get(hitmod) ) 
	break;
    }
  
  itl_destroy( itl );

  // if it was him, we can see him
  if( hitmod == him )
    {
      stg_geom_t* hisgeom = model_geom_get(him);

      // record where we saw him and what he looked like
      stg_fiducial_t fid;      
      fid.range = range;
      fid.bearing = NORMALIZE( hisbearing - mfb->pose.a);
      fid.id = range < mfb->cfg->max_range_id ? him->id : 0;
      fid.geom.x = hisgeom->size.x;
      fid.geom.y = hisgeom->size.y;
      fid.geom.a = NORMALIZE( hispose.a - mfb->pose.a);
      
      g_array_append_val( mfb->fiducials, fid );
    }
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
int model_fiducial_data_service( model_t* mod )
{
  //PRINT_WARN( "fiducial update" );

  if( fig_debug ) rtk_fig_clear( fig_debug );
  
  model_fiducial_buffer_t mfb;
  memset( &mfb, 0, sizeof(mfb) );
  
  mfb.mod = mod;
  mfb.cfg = model_fiducial_config_get(mod);
  mfb.fiducials = g_array_new( FALSE, TRUE, sizeof(stg_fiducial_t) );
  model_global_pose( mod, &mfb.pose );
  
  // for all the objects in the world
  g_hash_table_foreach( mod->world->models, model_fiducial_check_neighbor, &mfb );
  
  model_set_prop( mod, STG_PROP_FIDUCIALDATA, 
		  mfb.fiducials->data, 
		  mfb.fiducials->len * sizeof(stg_fiducial_t) );
  
  g_array_free( mfb.fiducials, TRUE );

  return 0;
}



void model_fiducial_data_render( model_t* mod )
{ 
  //PRINT_WARN( "fiducial render" );

  char text[32];

  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_FIDUCIALDATA];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_FIDUCIALDATA,
				 mod->gui.top, STG_LAYER_NEIGHBORDATA );
  
  if( mod->subs[STG_PROP_FIDUCIALDATA] )
    {      
      rtk_fig_color_rgb32( fig, stg_lookup_color( STG_FIDUCIAL_COLOR ) );
      
      stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_FIDUCIALDATA );
      
      if( prop )
	{
	  int bcount = prop->len / sizeof(stg_fiducial_t);
	  
	  stg_fiducial_t* fids = (stg_fiducial_t*)prop->data;
	  
	  int b;
	  for( b=0; b < bcount; b++ )
	    {
	      // the location of the target
	      double pa = fids[b].bearing;
	      double px = fids[b].range * cos(pa); 
	      double py = fids[b].range * sin(pa);
	      
	      rtk_fig_line( fig, 0, 0, px, py );	

	      // the size and heading of the target
	      double wx = fids[b].geom.x;
	      double wy = fids[b].geom.y;
	      double wa = fids[b].geom.a;
	      
	      rtk_fig_rectangle(fig, px, py, wa, wx, wy, 0);
	      rtk_fig_arrow(fig, px, py, wa, wy, 0.10);
	      
	      if( fids[b].id > 0 )
		{
		  snprintf(text, sizeof(text), "  %d", fids[b].id);
		  rtk_fig_text(fig, px, py, pa, text);
		}
	    }  
	}
    }
}

void model_fiducial_config_render( model_t* mod )
{ 
  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_FIDUCIALCONFIG];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_FIDUCIALCONFIG,
				 mod->gui.top, STG_LAYER_NEIGHBORCONFIG );
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_FIDUCIAL_CFG_COLOR ));
  
  stg_fiducial_config_t* cfg = (stg_fiducial_config_t*) 
    model_get_prop_data_generic( mod, STG_PROP_FIDUCIALCONFIG );
  
  if( cfg )
    {  
      double mina = -cfg->fov / 2.0;
      double maxa = +cfg->fov / 2.0;
      
      double dx =  cfg->max_range_anon * cos(mina);
      double dy =  cfg->max_range_anon * sin(mina);
      double ddx = cfg->max_range_anon * cos(maxa);
      double ddy = cfg->max_range_anon * sin(maxa);
      
      rtk_fig_line( fig, 0,0, dx, dy );
      rtk_fig_line( fig, 0,0, ddx, ddy );
      
      // max range
      rtk_fig_ellipse_arc( fig, 0,0,0,
			   2.0*cfg->max_range_anon,
			   2.0*cfg->max_range_anon, 
			   mina, maxa );      
      
      // max range that IDs can be, er... identified	  
      rtk_fig_ellipse_arc( fig, 0,0,0,
			   2.0*cfg->max_range_id,
			   2.0*cfg->max_range_id, 
			   mina, maxa );      
    }
}



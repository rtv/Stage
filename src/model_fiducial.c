///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_fiducial.c,v $
//  $Author: rtv $
//  $Revision: 1.5 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <math.h>
#include "model.h"
#include "raytrace.h"

#include "gui.h"
extern rtk_fig_t* fig_debug;

/* void model_fiducial_init( model_t* mod ) */
/* { */
/*   PRINT_WARN( "fiducial init" ); */

/*   stg_fiducial_config_t* fid = calloc( sizeof(stg_fiducial_config_t), 1 ); */
/*   memset( fid, 0, sizeof(stg_fiducial_config_t) ); */
/*   fid->max_range_anon = 6.0;  */
/*   fid->max_range_id = 3.0; */
/*   fid->fov = 2.0 * M_PI;  */

/*   // add this property to the model */
/*   model_set_prop_generic( mod, STG_PROP_FIDUCIALCONFIG,  */
/* 			  fid, sizeof(stg_fiducial_config_t) );   */
/* } */


void model_fiducial_shutdown( model_t* mod )
{
  PRINT_WARN( "fiducial shutdown" );
  
  model_remove_prop_generic( mod, STG_PROP_FIDUCIALDATA );
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
  
  double dx = hispose.x - mfb->pose.x;
  double dy = hispose.y - mfb->pose.y;
  
  // are we within range?
  double range = hypot( dy, dx );
  if( range > mfb->cfg->max_range_anon )
    return;
  
  // is he in my field of view?
  double hisbearing = atan2( dy, dx );
  double dif = mfb->pose.a - hisbearing;
  printf( "my heading %f   his bearing %f  dif %f\n", 
	  mfb->pose.a, hisbearing, dif );
  
  if( fabs(dif) > mfb->cfg->fov/2.0 )
    return;
  
 
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
      fid.bearing = NORMALIZE(atan2( dy, dx ) - mfb->pose.a);
      fid.id = range < mfb->cfg->max_range_id ? him->id : 0;
      fid.geom.x = him->geom.size.x;
      fid.geom.y = him->geom.size.y;
      fid.geom.a = NORMALIZE(him->pose.a - mfb->pose.a);
      
      g_array_append_val( mfb->fiducials, fid );
    }
  //else
  //printf( "out of range\n" );
}

///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void model_fiducial_update( model_t* mod )
{
  PRINT_WARN( "fiducial update" );

  if( fig_debug ) rtk_fig_clear( fig_debug );
  
  model_fiducial_buffer_t mfb;
  memset( &mfb, 0, sizeof(mfb) );
  
  mfb.mod = mod;
  mfb.cfg = model_get_prop_data_generic( mod,STG_PROP_FIDUCIALCONFIG ); 
  assert( mfb.cfg );
  mfb.fiducials = g_array_new( FALSE, TRUE, sizeof(stg_fiducial_t) );
  model_global_pose( mod, &mfb.pose );
  
  // for all the objects in the world
  g_hash_table_foreach( mod->world->models, model_fiducial_check_neighbor, &mfb );
  
  model_set_prop_generic( mod, STG_PROP_FIDUCIALDATA, 
			  mfb.fiducials->data, 
			  mfb.fiducials->len * sizeof(stg_fiducial_t) );
  
  g_array_free( mfb.fiducials, TRUE );
}



void model_fiducial_render( model_t* mod )
{ 
  PRINT_WARN( "fiducial render" );

  char text[32];

  gui_window_t* win = mod->world->win;  
  
  rtk_fig_t* fig = gui_model_figs(mod)->fiducial_data;  
  if( fig ) rtk_fig_clear( fig ); 
  
  if( win->show_fiducialdata && mod->subs[STG_PROP_FIDUCIALDATA] )
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
  gui_window_t* win = mod->world->win;

  rtk_fig_t* fig = gui_model_figs(mod)->fiducial_cfg;  
  rtk_fig_clear(fig);
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



///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_fiducial.c,v $
//  $Author: rtv $
//  $Revision: 1.23 $
//
///////////////////////////////////////////////////////////////////////////


/** 
@defgroup model_fiducial Fiducialfinder model
 
The fiducialfinder model simulates a fiducial-detecting device.
*/

//#define DEBUG

#include <math.h>
#include "stage.h"
#include "gui.h"

extern rtk_fig_t* fig_debug;

void fiducial_render_config( stg_model_t* mod );
void fiducial_render_data( stg_model_t* mod );

void fiducial_init( stg_model_t* mod )
{
  PRINT_DEBUG( "fiducial init" );

  // fiducialfinders don't have a body in the sim
  //stg_model_set_lines( mod, NULL, 0 );
  
  // a fraction smaller than a laser by default
  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom));
  geom.size.x = STG_DEFAULT_LASER_SIZEX * 0.9;
  geom.size.y = STG_DEFAULT_LASER_SIZEY * 0.9;
  stg_model_set_geom( mod, &geom );  
  
  stg_color_t color = stg_lookup_color( "magenta" );
  stg_model_set_color( mod, &color );

  mod->obstacle_return = 0;
  mod->laser_return = LaserTransparent;
  
  // start with no data
  stg_model_set_data( mod, NULL, 0 );

  // default parameters
  stg_fiducial_config_t cfg; 
  cfg.min_range = STG_DEFAULT_FIDUCIAL_RANGEMIN;
  cfg.max_range_anon = STG_DEFAULT_FIDUCIAL_RANGEMAXANON;
  cfg.max_range_id = STG_DEFAULT_FIDUCIAL_RANGEMAXID;
  cfg.fov = STG_DEFAULT_FIDUCIAL_FOV;  
  stg_model_set_config( mod, &cfg, sizeof(cfg) );
}

int fiducial_set_data( stg_model_t* mod, void* data, size_t len )
{  
  PRINT_DEBUG( "fiducial set data" );

  // store the data normally
  _set_data( mod, data, len );
  
  // and redraw it
  fiducial_render_data( mod );
  
  //PRINT_DEBUG3( "model %d(%s) now has %d bytes of data",
  //	mod->id, mod->token, (int)mod->data_len );

  return 0; //OK
}

int fiducial_set_config( stg_model_t* mod, void* config, size_t len )
{  
  PRINT_DEBUG( "fiducial set config" );
  
  // store the data normally
  _set_cfg( mod, config, len );
  
  // and redraw it
  fiducial_render_config( mod);

  return 0; //OK
}
 
int fiducial_shutdown( stg_model_t* mod )
{
  // this will undrender the data
  stg_model_set_data( mod, NULL, 0 );
}


typedef struct 
{
  stg_model_t* mod;
  stg_pose_t pose;
  stg_fiducial_config_t* cfg;
  GArray* fiducials;
} model_fiducial_buffer_t;

void model_fiducial_check_neighbor( gpointer key, gpointer value, gpointer user )
{
  model_fiducial_buffer_t* mfb = (model_fiducial_buffer_t*)user;
  stg_model_t* him = (stg_model_t*)value;
  
  //PRINT_DEBUG2( "checking model %s - he has fiducial value %d",
  //	him->token, him->fiducial_return );
		
  // dont' compare a model with itself, and don't consider models with
  // invalid returns
  if( him == mfb->mod || him->fiducial_return == FiducialNone ) 
    return; 

  stg_pose_t hispose;
  stg_model_global_pose( him, &hispose );
  
  double dx = hispose.x - mfb->pose.x;
  double dy = hispose.y - mfb->pose.y;
  
  // are we within range?
  double range = hypot( dy, dx );
  if( range > mfb->cfg->max_range_anon )
    {
      //PRINT_DEBUG1( "  but model %s is outside my range", him->token);
      return;
    }

  
  // is he in my field of view?
  double hisbearing = atan2( dy, dx );
  double dif = mfb->pose.a - hisbearing;

  if( fabs(NORMALIZE(dif)) > mfb->cfg->fov/2.0 )
    {
      //PRINT_DEBUG1( "  but model %s is outside my FOV", him->token);
      return;
    }
   
  // now check if we have line-of-sight
  itl_t *itl = itl_create( mfb->pose.x, mfb->pose.y,
			   hispose.x, hispose.y, 
			   him->world->matrix, PointToPoint );
  
  // iterate until we hit something
  stg_model_t* hitmod;
  while( (hitmod = itl_next( itl )) ) 
    {
      if( hitmod != mfb->mod &&  //&& model_obstacle_get(hitmod) ) 
	!stg_model_is_related(mfb->mod,hitmod) )
	break;
    }
  
  itl_destroy( itl );


  //PRINT_DEBUG( "finished raytracing" );
  
  //if( hitmod )
  //PRINT_DEBUG2( "I saw %s with fid %d",
  //	 hitmod->token, hitmod->fiducial_return );

  // if it was him, we can see him
  if( hitmod == him )
    {
      stg_geom_t* hisgeom = stg_model_get_geom(him);

      // record where we saw him and what he looked like
      stg_fiducial_t fid;      
      fid.range = range;
      fid.bearing = NORMALIZE( hisbearing - mfb->pose.a);
      fid.geom.x = hisgeom->size.x;
      fid.geom.y = hisgeom->size.y;
      fid.geom.a = NORMALIZE( hispose.a - mfb->pose.a);
      
      // if he's within ID range, get his fiducial.return value, else
      // we see value 0
      fid.id = range < mfb->cfg->max_range_id ? 
	him->fiducial_return : 0;

      //PRINT_DEBUG2( "adding %s's value %d to my list of fiducials",
      //	    him->token, him->fiducial_return );

      g_array_append_val( mfb->fiducials, fid );
    }
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
int fiducial_update( stg_model_t* mod )
{
  PRINT_DEBUG( "fiducial update" );

  if( mod->subs < 1 )
    return 0;

  if( fig_debug ) rtk_fig_clear( fig_debug );
  
  model_fiducial_buffer_t mfb;
  memset( &mfb, 0, sizeof(mfb) );
  
  mfb.mod = mod;
  
  size_t len;
  mfb.cfg = stg_model_get_config( mod, &len );
  assert(len==sizeof(stg_fiducial_config_t));

  mfb.fiducials = g_array_new( FALSE, TRUE, sizeof(stg_fiducial_t) );
  stg_model_global_pose( mod, &mfb.pose );
  
  // for all the objects in the world
  g_hash_table_foreach( mod->world->models, model_fiducial_check_neighbor, &mfb );

  //PRINT_DEBUG2( "model %s saw %d fiducials", mod->token, mfb.fiducials->len );

  stg_model_set_data( mod, 
		  mfb.fiducials->data, 
		  mfb.fiducials->len * sizeof(stg_fiducial_t) );
  
  g_array_free( mfb.fiducials, TRUE );

  return 0;
}



void fiducial_render_data( stg_model_t* mod )
{ 
  //PRINT_WARN( "fiducial render" );

  char text[32];

  
  if(  mod->gui.data  )
    rtk_fig_clear( mod->gui.data);
  else // create the figure, store it in the model and keep a local pointer
    mod->gui.data = rtk_fig_create( mod->world->win->canvas,
				    mod->gui.top, STG_LAYER_NEIGHBORDATA );
  
  rtk_fig_t* fig = mod->gui.data;
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_FIDUCIAL_COLOR ) );
  
  size_t len;
  stg_fiducial_t* fids = stg_model_get_data(mod, &len);
  
  int bcount = len / sizeof(stg_fiducial_t);
  
  
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

void fiducial_render_config( stg_model_t* mod )
{ 
  
  if( mod->gui.cfg  )
    rtk_fig_clear(mod->gui.cfg);
  else // create the figure, store it in the model and keep a local pointer
    mod->gui.cfg = rtk_fig_create( mod->world->win->canvas,
				 mod->gui.top, STG_LAYER_NEIGHBORCONFIG );

  rtk_fig_t* fig = mod->gui.cfg;  
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_FIDUCIAL_CFG_COLOR ));
  
  size_t len;
  stg_fiducial_config_t* cfg = stg_model_get_config(mod,&len);
  
  assert( len == sizeof(stg_fiducial_config_t) );
  assert( cfg );

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

/* int register_fiducial( lib_entry_t* lib ) */
/* {  */
/*   assert(lib); */
  
/*   lib[STG_MODEL_FIDUCIAL].init = fiducial_init; */
/*   lib[STG_MODEL_FIDUCIAL].update = fiducial_update; */
/*   lib[STG_MODEL_FIDUCIAL].shutdown = fiducial_shutdown; */
/*   lib[STG_MODEL_FIDUCIAL].set_config = fiducial_set_config; */
/*   lib[STG_MODEL_FIDUCIAL].set_data = fiducial_set_data; */

/*   return 0; //ok */
/* }  */

stg_lib_entry_t fiducial_entry = { 
  fiducial_init,     // init
  NULL,              // startup
  fiducial_shutdown, // shutdown
  fiducial_update,   // update
  fiducial_set_data, // set data
  NULL,              // get data
  NULL,              // set command
  NULL,              // get command
  fiducial_set_config, // set config
  NULL               // get config
};

///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_fiducial.c,v $
//  $Author: rtv $
//  $Revision: 1.13 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <math.h>
#include "model.h"
#include "raytrace.h"
#include "gui.h"
extern rtk_fig_t* fig_debug;

void fiducial_render_config( model_t* mod );
void fiducial_render_data( model_t* mod );

void fiducial_init( model_t* mod )
{
  PRINT_DEBUG( "fiducial init" );

  model_set_data( mod, NULL, 0 );

  stg_fiducial_config_t newfc; // init a basic config
  newfc.min_range = STG_DEFAULT_FIDUCIAL_RANGEMIN;
  newfc.max_range_anon = STG_DEFAULT_FIDUCIAL_RANGEMAXANON;
  newfc.max_range_id = STG_DEFAULT_FIDUCIAL_RANGEMAXID;
  newfc.fov = STG_DEFAULT_FIDUCIAL_FOV;
  
  model_set_config( mod, &newfc, sizeof(newfc) );
}

int fiducial_set_data( model_t* mod, void* data, size_t len )
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

int fiducial_set_config( model_t* mod, void* config, size_t len )
{  
  PRINT_DEBUG( "fiducial set config" );
  
  // store the data normally
  _set_cfg( mod, config, len );
  
  // and redraw it
  fiducial_render_config( mod);

  return 0; //OK
}
 
int fiducial_shutdown( model_t* mod )
{
  // this will undrender the data
  model_set_data( mod, NULL, 0 );
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
  
  //PRINT_DEBUG2( "checking model %s - he has fiducial value %d",
  //	him->token, him->fiducial_return );
		
  // dont' compare a model with itself, and don't consider models with
  // invalid returns
  if( him == mfb->mod || him->fiducial_return == FiducialNone ) 
    return; 

  stg_pose_t hispose;
  model_global_pose( him, &hispose );
  
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
  model_t* hitmod;
  while( (hitmod = itl_next( itl )) ) 
    {
      if( hitmod != mfb->mod &&  //&& model_obstacle_get(hitmod) ) 
	!model_is_descendent(mfb->mod,hitmod) && 
	!model_is_antecedent(mfb->mod,hitmod) )
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
      stg_geom_t* hisgeom = model_get_geom(him);

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
int fiducial_update( model_t* mod )
{
  PRINT_DEBUG( "fiducial update" );

  if( fig_debug ) rtk_fig_clear( fig_debug );
  
  model_fiducial_buffer_t mfb;
  memset( &mfb, 0, sizeof(mfb) );
  
  mfb.mod = mod;
  
  size_t len;
  mfb.cfg = model_get_config( mod, &len );
  assert(len==sizeof(stg_fiducial_config_t));

  mfb.fiducials = g_array_new( FALSE, TRUE, sizeof(stg_fiducial_t) );
  model_global_pose( mod, &mfb.pose );
  
  // for all the objects in the world
  g_hash_table_foreach( mod->world->models, model_fiducial_check_neighbor, &mfb );

  //PRINT_DEBUG2( "model %s saw %d fiducials", mod->token, mfb.fiducials->len );

  model_set_data( mod, 
		  mfb.fiducials->data, 
		  mfb.fiducials->len * sizeof(stg_fiducial_t) );
  
  g_array_free( mfb.fiducials, TRUE );

  return 0;
}



void fiducial_render_data( model_t* mod )
{ 
  //PRINT_WARN( "fiducial render" );

  char text[32];

  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_DATA];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_DATA,
				 mod->gui.top, STG_LAYER_NEIGHBORDATA );
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_FIDUCIAL_COLOR ) );
  
  size_t len;
  stg_fiducial_t* fids = model_get_data(mod, &len);
  
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

void fiducial_render_config( model_t* mod )
{ 
  rtk_fig_t* fig = mod->gui.propdata[STG_PROP_CONFIG];  
  
  if( fig  )
    rtk_fig_clear(fig);
  else // create the figure, store it in the model and keep a local pointer
    fig = model_prop_fig_create( mod, mod->gui.propdata, STG_PROP_CONFIG,
				 mod->gui.top, STG_LAYER_NEIGHBORCONFIG );
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_FIDUCIAL_CFG_COLOR ));
  
  size_t len;
  stg_fiducial_config_t* cfg = model_get_config(mod,&len);
  
  if( cfg )
    { 
      assert( len == sizeof(stg_fiducial_config_t));
 
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



int register_fiducial( void )
{
  register_init( STG_MODEL_FIDUCIAL, fiducial_init );
  register_set_data( STG_MODEL_FIDUCIAL, fiducial_set_data );
  register_set_config( STG_MODEL_FIDUCIAL, fiducial_set_config );
  register_update( STG_MODEL_FIDUCIAL, fiducial_update );
  register_shutdown( STG_MODEL_FIDUCIAL, fiducial_shutdown );

  return 0; //ok
} 

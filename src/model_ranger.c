#include "stage.h"
#include "raytrace.h"


#include "gui.h"
extern rtk_fig_t* fig_debug;


void model_ranger_init( model_t* mod )
{
  mod->ranger_return = LaserVisible;
  
  // start with no rangers 
  model_set_prop_generic( mod, STG_PROP_RANGERCONFIG,
			  NULL, 0 );
  
  //mod->ranger_data = g_array_new( FALSE, TRUE, sizeof(stg_ranger_sample_t));
  //mod->ranger_config = g_array_new( FALSE, TRUE, sizeof(stg_ranger_config_t) );
}


void model_ranger_update( model_t* mod )
{   
  //PRINT_DEBUG1( "[%.3f] updating rangers", mod->world->sim_time );
  
  //assert( mod->ranger_config );
  //assert( mod->ranger_data );
  
  stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
  
  if( prop == NULL ) // no rangers to update!
    return;
  //else
  
  stg_ranger_config_t* cfg = (stg_ranger_config_t*)prop->data;
  assert( cfg );

  int rcount = prop->len / sizeof( stg_ranger_config_t );
  
  if( rcount < 1 )
    return;
  
  //PRINT_WARN1( "wierd! %d rangers", rcount);
  
  stg_ranger_sample_t* ranges = (stg_ranger_sample_t*)
    calloc( sizeof(stg_ranger_sample_t), rcount );
  
  if( fig_debug ) rtk_fig_clear( fig_debug );

  int t;
  for( t=0; t<rcount; t++ )
    {
      //stg_ranger_config_t* cfg = &g_array_index( mod->ranger_config, 
      //				   stg_ranger_config_t, t );
      
      //g_assert( cfg );
      
      // get the sensor's pose in global coords
      stg_pose_t pz;
      memcpy( &pz, &cfg[t].pose, sizeof(pz) ); 
      model_local_to_global( mod, &pz );
      
      // todo - use bounds_range.min

      itl_t* itl = itl_create( pz.x, pz.y, pz.a, 
			       cfg[t].bounds_range.max, 
			       mod->world->matrix, 
			       PointToBearingRange );
      
      model_t * hitmod;
      double range = cfg[t].bounds_range.max;
      
      while( (hitmod = itl_next( itl )) ) 
	{
	  //printf( "model %d %p   hit model %d %p\n",
	  //  mod->id, mod, hitmod->id, hitmod );
	  
	  // Ignore myself, things which are attached to me, and
	  // things that we are attached to (except root) The latter
	  // is useful if you want to stack beacons on the laser or
	  // the laser on somethine else.
	  if (hitmod == mod )//|| modthis->IsDescendent(ent) )//|| 
	    continue;
	  
	  // Stop looking when we see something
	  if( hitmod->ranger_return != LaserTransparent) 
	    {
	      range = itl->range;
	      break;
	    }	
	}
      
      if( range < cfg[t].bounds_range.min )
	range = cfg[t].bounds_range.min;
      
      ranges[t].range = range;
      //ranges[t].error = TODO;
    }
  
  model_set_prop_generic( mod, STG_PROP_RANGERDATA,
			  ranges, sizeof(stg_ranger_sample_t) * rcount );
  
}

void gui_model_rangers( model_t* mod )
{
  //PRINT_DEBUG( "drawing rangers" );

  rtk_fig_t* fig = gui_model_figs(mod)->rangers;
  
  rtk_fig_color_rgb32(fig, stg_lookup_color("orange") );
  //rtk_fig_color_rgb32( fig, stg_lookup_color(STG_RANGER_COLOR) );
  
  rtk_fig_origin( fig, mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );  
  rtk_fig_clear( fig ); 
  

  stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
  
  if( prop == NULL ) // no rangers to update!
    return;
  //else
  
  stg_ranger_config_t* cfg = (stg_ranger_config_t*)prop->data;
  assert( cfg );

  int rcount = prop->len / sizeof( stg_ranger_config_t );
  
  if( rcount < 1 )
    return;


  // add rects showing ranger positions
  int s;
  for( s=0; s<rcount; s++ )
    {
      stg_ranger_config_t* rngr = &cfg[s];
      //printf( "drawing a ranger rect (%.2f,%.2f,%.2f)[%.2f %.2f]\n",
      //  rngr->pose.x, rngr->pose.y, rngr->pose.a,
      //  rngr->size.x, rngr->size.y );
      
      rtk_fig_rectangle( fig, 
			 rngr->pose.x, rngr->pose.y, rngr->pose.a,
			 rngr->size.x, rngr->size.y, 0 ); 
      
      // show the FOV too
      double sidelen = rngr->size.x/2.0;
      
      double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - rngr->fov/2.0 );
      double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - rngr->fov/2.0 );
      double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + rngr->fov/2.0 );
      double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + rngr->fov/2.0 );
      
      rtk_fig_line( fig, rngr->pose.x, rngr->pose.y, x1, y1 );
      rtk_fig_line( fig, rngr->pose.x, rngr->pose.y, x2, y2 );	
    }
}

void model_ranger_render( model_t* mod )
{ 
  gui_window_t* win = mod->world->win;
  
  rtk_fig_t* fig = gui_model_figs(mod)->ranger_data;  
  if( fig ) rtk_fig_clear( fig );   
  
  if( win->show_rangerdata && mod->subs[STG_PROP_RANGERDATA] )
    {
      
      stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_RANGERCONFIG );
      
      if( prop == NULL ) // no rangers to update!
	return;
      
      stg_ranger_config_t* cfg = (stg_ranger_config_t*)prop->data;  
      int rcount = prop->len / sizeof( stg_ranger_config_t );
      
      stg_ranger_sample_t* samples = (stg_ranger_sample_t*)
	model_get_prop_data_generic( mod, STG_PROP_RANGERDATA );
      
      if( rcount > 0 && cfg && samples )
	{
	  rtk_fig_color_rgb32(fig, stg_lookup_color(STG_RANGER_COLOR) );
	  rtk_fig_origin( fig, 
			  mod->local_pose.x, mod->local_pose.y, mod->local_pose.a );	  
	  // draw the range  beams
	  int s;
	  for( s=0; s<rcount; s++ )
	    {
	      if( samples[s].range > 0.0 )
		{
		  stg_ranger_config_t* rngr = &cfg[s];
		  
		  rtk_fig_arrow( fig, rngr->pose.x, rngr->pose.y, rngr->pose.a, 			       samples[s].range, 0.02 );
		}
	    }
	}
    }
}


///////////////////////////////////////////////////////////////////////////
//
// File: model_gripper.c
// Authors: Richard Vaughan <vaughan@sfu.ca>
//          Doug Blank
// Date: 21 April 2005
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_gripper.c,v $
//  $Author: rtv $
//  $Revision: 1.25 $
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_gripper Gripper model 

The ranger model simulates a simple two-fingered gripper with two
internal break-beams, similar to the the Pioneer gripper.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
gripper
(
  # gripper properties
  <none>

  # model properties
  size [0.12 0.28]
)
@endverbatim

@par Notes

@par Details

*/


#include <sys/time.h>
#include <math.h>
#include "gui.h"

//#define DEBUG

#define STG_DEFAULT_GRIPPER_SIZEX 0.12
#define STG_DEFAULT_GRIPPER_SIZEY 0.28
#define STG_GRIPPER_WATTS 1.0

// TODO - simulate energy use when moving grippers

#include "stage_internal.h"

// standard callbacks
int gripper_update( stg_model_t* mod );
int gripper_startup( stg_model_t* mod );
int gripper_shutdown( stg_model_t* mod );
void gripper_load( stg_model_t* mod );

int gripper_render_data( stg_model_t* mod, void* userp );
int gripper_render_cfg( stg_model_t* mod, void* userp );
int gripper_unrender_data( stg_model_t* mod, void* userp );
int gripper_unrender_cfg( stg_model_t* mod, void* userp );


static stg_color_t gripper_col = 0;

void gripper_generate_paddles( stg_model_t* mod, stg_gripper_config_t* cfg );

int gripper_paddle_contact( stg_model_t* mod, 
			    stg_gripper_config_t* cfg,
			    double contacts[2] );
int gripper_break_beam(stg_model_t* mod, stg_gripper_config_t* cfg, int beam);

void gripper_load( stg_model_t* mod )
{
  //stg_gripper_config_t cfg;

  // TODO: read gripper params from the world file
  
  //stg_model_set_cfg( mod, gconf, sizeof(stg_gripper_config_t));
}

int gripper_init( stg_model_t* mod )
{
  // we don't consume any power until subscribed
  //mod->watts = 0.0; 
  
  // override the default methods
  mod->f_startup = gripper_startup;
  mod->f_shutdown = gripper_shutdown;
  mod->f_update =  gripper_update;
  mod->f_load = gripper_load;

  // sensible gripper defaults
  stg_geom_t geom;
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = STG_DEFAULT_GRIPPER_SIZEX;
  geom.size.y = STG_DEFAULT_GRIPPER_SIZEY;
  stg_model_set_geom( mod, &geom );
  
  stg_model_set_gripper_return( mod, 0 );

  // set up a gripper-specific config structure
  stg_gripper_config_t gconf;
  memset(&gconf,0,sizeof(gconf));  
  gconf.paddle_size.x = 0.66; // proportion of body that is paddles
  gconf.paddle_size.y = 0.15; // the thickness of the paddles as a proportion of total size

  gconf.paddles = STG_GRIPPER_PADDLE_OPEN;
  gconf.lift = STG_GRIPPER_LIFT_DOWN;
  gconf.paddles_stalled = FALSE;

  // place the break beam sensors at 1/4 and 3/4 the length of the paddle 
  gconf.inner_break_beam_inset = 3.0/4.0 * gconf.paddle_size.x;
  gconf.outer_break_beam_inset = 1.0/4.0 * gconf.paddle_size.x;
  
  gconf.close_limit = 1.0;

  stg_model_set_cfg( mod, &gconf, sizeof(gconf) );
  
  // sensible starting command
  stg_gripper_cmd_t cmd; 
  cmd.cmd = STG_GRIPPER_CMD_NOP;
  cmd.arg = 0;  
  stg_model_set_cmd( mod, &cmd, sizeof(cmd) ) ;
  
  gripper_col = stg_lookup_color(STG_GRIPPER_COLOR);

  // install three polygons for the gripper body;
  stg_polygon_t* polys = stg_polygons_create( 3 );
  stg_model_set_polygons( mod, polys, 3 );

  // figure out where the paddles should be
  gripper_generate_paddles( mod, &gconf );

  stg_gripper_data_t data;
  memset(&data,0,sizeof(data));
  stg_model_set_data( mod, &data, sizeof(data));
  

  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, &mod->data,
				  gripper_render_data, NULL,
				  gripper_unrender_data, NULL,
				  "gripper_data",
				  "gripper data",
				  TRUE );

  // this doesn't show anything very useful
  /*  stg_model_add_property_toggles( mod, &mod->cfg,
				  gripper_render_cfg, NULL,
				  gripper_unrender_cfg, NULL,
				  "gripper_cfg",
				  "gripper config",
				  TRUE );
  */
  
  return 0;
}

// create three rectangles that are the gripper's body
void gripper_generate_paddles( stg_model_t* mod, stg_gripper_config_t* cfg )
{
  stg_point_t body[4];
  body[0].x = 0;
  body[0].y = 0;
  body[1].x = 1.0 - cfg->paddle_size.x;
  body[1].y = 0;
  body[2].x = body[1].x;
  body[2].y = 1.0;
  body[3].x = 0;
  body[3].y = 1.0;

  stg_point_t l_pad[4];
  l_pad[0].x = 1.0 - cfg->paddle_size.x;
  l_pad[0].y = cfg->paddle_position * (0.5 - cfg->paddle_size.y);
  l_pad[1].x = 1.0;
  l_pad[1].y = l_pad[0].y;
  l_pad[2].x = 1.0;
  l_pad[2].y = l_pad[0].y + cfg->paddle_size.y;
  l_pad[3].x = l_pad[0].x;
  l_pad[3].y = l_pad[2].y;

  stg_point_t r_pad[4];
  r_pad[0].x = 1.0 - cfg->paddle_size.x;
  r_pad[0].y = 1.0 - cfg->paddle_position * (0.5 - cfg->paddle_size.y);
  r_pad[1].x = 1.0;
  r_pad[1].y = r_pad[0].y;
  r_pad[2].x = 1.0;
  r_pad[2].y = r_pad[0].y - cfg->paddle_size.y;
  r_pad[3].x = r_pad[0].x;
  r_pad[3].y = r_pad[2].y;
  
  // move the paddle arm polygons 
  
  // TODO - this could be much more
  // efficient if we didn't do the scaling every time
  assert( mod->polygons_count == 3 );
  assert( mod->polygons );
  stg_polygon_t* polys = mod->polygons;
  stg_polygon_set_points( &polys[0], body, 4 );
  stg_polygon_set_points( &polys[1], l_pad, 4 );
  stg_polygon_set_points( &polys[2], r_pad, 4 );	  
  
  stg_polygons_normalize( mod->polygons, mod->polygons_count,
			  mod->geom.size.x, mod->geom.size.y );
  
  model_change( mod, &mod->polygons );
}

int gripper_update( stg_model_t* mod )
{   
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;
    
  stg_gripper_config_t cfg;
  memcpy(&cfg, mod->cfg, sizeof(cfg));

  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  stg_gripper_cmd_t* cmd = (stg_gripper_cmd_t*)mod->cmd;
  
  switch( cmd->cmd )
    {
    case 0:
      //puts( "gripper NOP" );
      break;
      
    case STG_GRIPPER_CMD_CLOSE:     
      if( cfg.paddles != STG_GRIPPER_PADDLE_CLOSED )
	{
	  //puts( "closing gripper paddles" );
	  cfg.paddles = STG_GRIPPER_PADDLE_CLOSING;
	}
      break;
      
    case STG_GRIPPER_CMD_OPEN:
      if( cfg.paddles != STG_GRIPPER_PADDLE_OPEN )
	{
	  //puts( "opening gripper paddles" );      
	  cfg.paddles = STG_GRIPPER_PADDLE_OPENING;
	}
      break;
      
    case STG_GRIPPER_CMD_UP:
      if( cfg.lift != STG_GRIPPER_LIFT_UP )
	{
	  //puts( "raising gripper lift" );      
	  cfg.lift = STG_GRIPPER_LIFT_UPPING;
	}
      break;
      
    case STG_GRIPPER_CMD_DOWN:
      if( cfg.lift != STG_GRIPPER_LIFT_DOWN )
	{
	  //puts( "lowering gripper lift" );      
	  cfg.lift = STG_GRIPPER_LIFT_DOWNING;
	}      
      break;
      
    default:
      printf( "unknown gripper command %d\n",cmd->cmd );
    }
	      
  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &geom.pose, sizeof(pz) ); 
  stg_model_local_to_global( mod, &pz );

  // cast the rays of the beams
  // compute beam and arm states
  
  
  // move the paddles 
  if( cfg.paddles == STG_GRIPPER_PADDLE_OPENING && !cfg.paddles_stalled  )
    {
      cfg.paddle_position -= 0.05;
      
      if( cfg.paddle_position < 0.0 ) // if we're fully open
	{
	  cfg.paddle_position = 0.0;
	  cfg.paddles = STG_GRIPPER_PADDLE_OPEN; // change state
	}

      // drop the thing at the head of the stack
      if( cfg.grip_stack &&  
	  (cfg.paddle_position == 0.0 || cfg.paddle_position < cfg.close_limit ))
	{
	  stg_model_t* head = cfg.grip_stack->data;
	  cfg.grip_stack = g_slist_remove( cfg.grip_stack, head );
	  
	  // move it to the new location
	  stg_pose_t drop_pose;
	  stg_model_get_global_pose( head, &drop_pose );
	  stg_model_set_parent( head, NULL );	      
	  stg_model_set_global_pose( head, &drop_pose );
	  
	  cfg.close_limit = 1.0;
	}
    }
  else if( cfg.paddles == STG_GRIPPER_PADDLE_CLOSING && !cfg.paddles_stalled  )
    {
      cfg.paddle_position += 0.05;
      
      if( cfg.paddle_position > cfg.close_limit ) // if we're fully closed
	{
	  cfg.paddle_position = cfg.close_limit;
	  cfg.paddles = STG_GRIPPER_PADDLE_CLOSED; // change state
	}
    }
  
  // move the lift 
  if( cfg.lift == STG_GRIPPER_LIFT_DOWNING )
    {
      cfg.lift_position -= 0.05;
      
      if( cfg.lift_position < 0.0 ) // if we're fully down
	{
	  cfg.lift_position = 0.0;
	  cfg.lift = STG_GRIPPER_LIFT_DOWN; // change state
	}
    }
  else if( cfg.lift == STG_GRIPPER_LIFT_UPPING )
    {
      cfg.lift_position += 0.05;
      
      if( cfg.lift_position > 1.0 ) // if we're fully open
	{
	  cfg.lift_position = 1.0;
	  cfg.lift = STG_GRIPPER_LIFT_UP; // change state
	}
    }
  
  // figure out where the paddles should be
  gripper_generate_paddles( mod, &cfg );
  
  // change the gripper's configuration in response to the commands
  stg_model_set_cfg( mod, &cfg, sizeof(stg_gripper_config_t));
  
  // also publish the data
  stg_gripper_data_t data;

  // several items come from the config 
  data.paddles = cfg.paddles;
  data.paddle_position = cfg.paddle_position;
  data.lift = cfg.lift;
  data.lift_position = cfg.lift_position;

  if( cfg.grip_stack )
    data.stack_count = g_slist_length( cfg.grip_stack );
  else
    data.stack_count = 0;

  // calculate the dynamic items
  data.inner_break_beam = gripper_break_beam( mod, &cfg, 1 );
  data.outer_break_beam = gripper_break_beam( mod, &cfg, 0 );

  gripper_paddle_contact( mod, &cfg, &data.paddle_contacts );

  data.paddles_stalled = cfg.paddles_stalled;

  // publish the new stuff
  stg_model_set_data( mod, &data, sizeof(data));
  stg_model_set_cfg( mod,  &cfg, sizeof(cfg));

  //stg_print_gripper_config( &cfg );

  // inherit standard update behaviour
  _model_update( mod );

  return 0; //ok
}

int gripper_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  return( (hitmod != mod) && hitmod->gripper_return );

  // can't use the normal relation check, because we may pick things
  // up and we must still see them.
}	

int gripper_break_beam(stg_model_t* mod, stg_gripper_config_t* cfg, int beam) 
{
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  stg_pose_t pz;

  //double ibbx =  (geom.size.x - cfg->inner_break_beam_inset * geom.size.x) - geom.size.x/2.0;
  //double obbx =  (geom.size.x - cfg->outer_break_beam_inset * geom.size.x) - geom.size.x/2.0;
 

  // x location of break beam origin
  double inset = beam ? cfg->inner_break_beam_inset : cfg->outer_break_beam_inset;
  pz.x = (geom.size.x - inset * geom.size.x) - geom.size.x/2.0;
  
  // y location of break beam origin
  pz.y = (1.0 - cfg->paddle_position) * ((geom.size.y/2.0)-(geom.size.y*cfg->paddle_size.y));
  
  // break beam local heading
  pz.a = -M_PI/2.0;

  // break beam max range
  double bbr = 
    (1.0 - cfg->paddle_position) * (geom.size.y - (geom.size.y * cfg->paddle_size.y * 2.0 ));
  
  
  stg_model_local_to_global( mod, &pz );
  
  //printf( "ray tracing from (%.2f %.2f %.2f) for %.2f meters\n",
  //  pz.x, pz.y, pz.a, bbr );

  itl_t* itl = itl_create( pz.x, pz.y, pz.a, 
			   bbr,
			   mod->world->matrix, 
			   PointToBearingRange );
  
  // if the breakbeam strikes anything
  if( itl_first_matching( itl, gripper_raytrace_match, mod ) )
    return 1;
  
  //else
  return 0;
}

int gripper_paddle_contact( stg_model_t* mod, 
			    stg_gripper_config_t* cfg,
			    double contacts[2] )
{ 
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  stg_pose_t lpz, rpz;
  
  // x location of contact sensor origin

  lpz.x = ((1.0 - cfg->paddle_size.x) * geom.size.x) - geom.size.x/2.0 ;
  rpz.x = ((1.0 - cfg->paddle_size.x) * geom.size.x) - geom.size.x/2.0 ;

  //double inset = beam ? cfg->inner_break_beam_inset : cfg->outer_break_beam_inset;
  //pz.x = (geom.size.x - inset * geom.size.x) - geom.size.x/2.0;

  // y location of paddle beam origin

    lpz.y = (1.0 - cfg->paddle_position) * 
      ((geom.size.y/2.0) - (geom.size.y*cfg->paddle_size.y));

    rpz.y = (1.0 - cfg->paddle_position) * 
      -((geom.size.y/2.0) - (geom.size.y*cfg->paddle_size.y));
  
/*   // if we're opening, we check the outside of the paddle instead */
/*   if( cfg->paddles == STG_GRIPPER_PADDLE_OPENING ) */
/*     { */
/*       if( paddle == 0 ) */
/* 	pz.y += geom.size.y * cfg->paddle_size.y; */
/*       else */
/* 	pz.y -= geom.size.y * cfg->paddle_size.y; */
/*     } */
  
  // paddle beam local heading
  lpz.a = 0.0;
  rpz.a = 0.0;
  
  // paddle beam max range
  double bbr = cfg->paddle_size.x * geom.size.x; 
  
  stg_model_local_to_global( mod, &lpz );
  stg_model_local_to_global( mod, &rpz );
  
  //printf( "ray tracing from (%.2f %.2f %.2f) for %.2f meters\n",
  //  pz.x, pz.y, pz.a, bbr );

  itl_t* litl = itl_create( lpz.x, lpz.y, lpz.a, 
			   bbr,
			   mod->world->matrix, 
			   PointToBearingRange );

  itl_t* ritl = itl_create( rpz.x, rpz.y, rpz.a, 
			   bbr,
			   mod->world->matrix, 
			   PointToBearingRange );
  
  stg_model_t* lhit = NULL;
  stg_model_t* rhit = NULL;

  if( cfg->paddles_stalled )
      cfg->paddles_stalled = FALSE;

  // contact switches are not pressed
  contacts[0] = 0;
  contacts[1] = 0;


  // if the breakbeam strikes anything
  if( (lhit = itl_first_matching( litl, gripper_raytrace_match, mod )) )
    {
      contacts[0] = 1; // touching something
      
      //if( cfg->paddles == STG_GRIPPER_PADDLE_CLOSING || 
      //  cfg->paddles == STG_GRIPPER_PADDLE_OPENING )
	//cfg->paddles_stalled = TRUE;
      
      //puts( "left hit" );
    }

      
  if( (rhit = itl_first_matching( ritl, gripper_raytrace_match, mod )))
    {
      contacts[1] = 1;      
      //puts( "right hit" );
    }
  
  if( lhit && (lhit == rhit) )
    {
      //puts( "left and right hit same thing" );
      
      if(  cfg->paddles == STG_GRIPPER_PADDLE_CLOSING )
	{
	  // grab the model we hit - very simple grip model for now
	  stg_model_set_parent( lhit, mod );
	  
	  // and move it to be right between the paddles
	  stg_geom_t hitgeom;
	  stg_model_get_geom( lhit, &hitgeom );
	  
	  stg_pose_t pose;
	  pose.x = hitgeom.size.x/2.0;
	  pose.y = 0;
	  pose.a = 0;
	  
	  stg_model_set_pose( lhit, &pose );	      
	  
	  // add this item to the stack
	  cfg->grip_stack = g_slist_prepend( cfg->grip_stack, lhit );
	  
	  // calculate how far closed we can get the paddles now
	  double puckw = hitgeom.size.y;
	  double gripperw = geom.size.y;	      
	  
	  cfg->close_limit = 
	    MAX( 0.0, 1.0 - puckw/(gripperw - cfg->paddle_size.y/2.0 )); 
	}
    }

  itl_destroy( litl );
  itl_destroy( ritl );
  
  return 0;
}

int gripper_render_data(  stg_model_t* mod, void* userp )
{
  //puts( "gripper render data" );

  // only draw if someone is using the gripper
  if( mod->subs < 1 )
    return 0;

  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "gripper_data_fig" );  
  
  if( ! fig )
    {
      fig = stg_model_fig_create( mod, "gripper_data_fig", "top", STG_LAYER_GRIPPERDATA );
      //stg_rtk_fig_color_rgb32( fig, gripper_col );
      stg_rtk_fig_color_rgb32( fig, 0xFF0000 ); // red
  
    }
  else
    stg_rtk_fig_clear( fig );

  //printf( "SUBS %d\n", mod->subs );
  
  stg_gripper_data_t* data = (stg_gripper_data_t*)mod->data;
  assert(data);
  
  stg_gripper_config_t *cfg = (stg_gripper_config_t*)mod->cfg;
  assert(cfg);

  stg_geom_t *geom = &mod->geom;  
  
  //stg_rtk_fig_rectangle( gui->data, 0,0,0, geom.size.x, geom.size.y, 0 );
  
  // different x location for each beam
  double ibbx =  (geom->size.x - cfg->inner_break_beam_inset * geom->size.x) - geom->size.x/2.0;
  double obbx =  (geom->size.x - cfg->outer_break_beam_inset * geom->size.x) - geom->size.x/2.0;
  
  // common y position
  double bby = 
    (1.0-data->paddle_position) * ((geom->size.y/2.0)-(geom->size.y*cfg->paddle_size.y));
  
  // size of the paddle indicator lights
  double led_dx = cfg->paddle_size.y * 0.5 * geom->size.y;
  
  
  if( data->inner_break_beam )
	{
	  stg_rtk_fig_rectangle( fig, ibbx, bby+led_dx, 0, led_dx, led_dx, 1 );
	  stg_rtk_fig_rectangle( fig, ibbx, -bby-led_dx, 0, led_dx, led_dx, 1 );
	}
      else
	{
	  stg_rtk_fig_line( fig, ibbx, bby, ibbx, -bby );
	  stg_rtk_fig_rectangle( fig, ibbx, bby+led_dx, 0, led_dx, led_dx, 0 );
	  stg_rtk_fig_rectangle( fig, ibbx, -bby-led_dx, 0, led_dx, led_dx, 0 );
	}
      
      if( data->outer_break_beam )
	{
	  stg_rtk_fig_rectangle( fig, obbx, bby+led_dx, 0, led_dx, led_dx, 1 );
	  stg_rtk_fig_rectangle( fig, obbx, -bby-led_dx, 0, led_dx, led_dx, 1 );
	}
      else
	{
	  stg_rtk_fig_line( fig, obbx, bby, obbx, -bby );
	  stg_rtk_fig_rectangle( fig, obbx, bby+led_dx, 0, led_dx, led_dx, 0 );
	  stg_rtk_fig_rectangle( fig, obbx, -bby-led_dx, 0, led_dx, led_dx, 0 );
	}
      
      // draw the contact indicators
      stg_rtk_fig_rectangle( fig, 
			     ((1.0 - cfg->paddle_size.x/2.0) * geom->size.x) - geom->size.x/2.0,
			     (1.0 - cfg->paddle_position) * ((geom->size.y/2.0)-(geom->size.y*cfg->paddle_size.y)),
			     0.0,
			     cfg->paddle_size.x * geom->size.x,
			     cfg->paddle_size.y/6.0 * geom->size.y, 
			     data->paddle_contacts[0] );
      
      stg_rtk_fig_rectangle( fig, 
			     ((1.0 - cfg->paddle_size.x/2.0) * geom->size.x) - geom->size.x/2.0,
			     (1.0 - cfg->paddle_position) * -((geom->size.y/2.0)-(geom->size.y*cfg->paddle_size.y)),
			     0.0,
			     cfg->paddle_size.x * geom->size.x,
			     cfg->paddle_size.y/6.0 * geom->size.y, 
			     data->paddle_contacts[1] );
      
      //stg_rtk_fig_color_rgb32( fig,  gripper_col );      

      return 0; 
}


int gripper_render_cfg( stg_model_t* mod, void* user )
{ 
  puts( "gripper render cfg" );
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "gripper_cfg_fig" );  
  
  if( ! fig )
    {
      fig = stg_model_fig_create( mod, "gripper_cfg_fig", "top", 
				  STG_LAYER_GRIPPERCONFIG );
      
      stg_rtk_fig_color_rgb32( fig, stg_lookup_color( STG_GRIPPER_CFG_COLOR ));
    }
  else
    stg_rtk_fig_clear( fig );
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  // get the config and make sure it's the right size
  stg_gripper_config_t* cfg = (stg_gripper_config_t*)mod->cfg;
  assert( mod->cfg_len == sizeof(stg_gripper_config_t) );
  
  // different x location for each beam
  double ibbx = (cfg->inner_break_beam_inset) * geom.size.x - geom.size.x/2.0;
  double obbx = (cfg->outer_break_beam_inset) * geom.size.x - geom.size.x/2.0;
  
  // common y position
  double bby = 
    (1.0-cfg->paddle_position) * ((geom.size.y/2.0)-(geom.size.y*cfg->paddle_size.y));
  
  // draw the position of the break beam sensors
  stg_rtk_fig_rectangle( fig, ibbx, bby, 0, 0.01, 0.01, 0 );
  stg_rtk_fig_rectangle( fig, obbx, bby, 0, 0.01, 0.01, 0 );

  return 0; //ok
}


int gripper_startup( stg_model_t* mod )
{ 
  PRINT_DEBUG( "gripper startup" );
  stg_model_set_watts( mod, STG_GRIPPER_WATTS );
  return 0; // ok
}

int gripper_shutdown( stg_model_t* mod )
{ 
  PRINT_DEBUG( "gripper shutdown" );
  stg_model_set_watts( mod, 0 );
  
  // unrender the break beams & lights
  stg_model_fig_clear( mod, "gripper_data_fig" );
  return 0; // ok
}

void stg_print_gripper_config( stg_gripper_config_t* cfg ) 
{
  char* pstate;
  switch( cfg->paddles )
    {
    case STG_GRIPPER_PADDLE_OPEN: pstate = "OPEN"; break;
    case STG_GRIPPER_PADDLE_CLOSED: pstate = "CLOSED"; break;
    case STG_GRIPPER_PADDLE_OPENING: pstate = "OPENING"; break;
    case STG_GRIPPER_PADDLE_CLOSING: pstate = "CLOSING"; break;
    default: pstate = "*unknown*";
    }
  
  char* lstate;
  switch( cfg->lift )
    {
    case STG_GRIPPER_LIFT_UP: lstate = "UP"; break;
    case STG_GRIPPER_LIFT_DOWN: lstate = "DOWN"; break;
    case STG_GRIPPER_LIFT_DOWNING: lstate = "DOWNING"; break;
    case STG_GRIPPER_LIFT_UPPING: lstate = "UPPING"; break;
    default: lstate = "*unknown*";
    }
  
  printf("gripper state: paddles(%s)[%.2f] lift(%s)[%.2f] stall(%s)\n", 
	 pstate, cfg->paddle_position, lstate, cfg->lift_position,
	 cfg->paddles_stalled ? "true" : "false" );
}


int gripper_unrender_data( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "gripper_data_fig" );
  return 1; // callback just runs one time
}

int gripper_unrender_cfg( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "gripper_cfg_fig" );
  return 1; // callback just runs one time
}


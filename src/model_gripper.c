///////////////////////////////////////////////////////////////////////////
//
// File: model_gripper.c
// Author: Doug Blank, Richard Vaughan
// Date: 21 April 2005
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_gripper.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>
#include "gui.h"

//#define DEBUG

#define STG_DEFAULT_GRIPPER_SIZEX 0.14
#define STG_DEFAULT_GRIPPER_SIZEY 0.30

#include "stage_internal.h"

// temporary static definitions of paddle rectangles
// this will be replaced by nice animated paddles soon enough.
static stg_point_t body[4] = {{0,0},{0.4,0},{0.4,1},{0,1}};
static stg_point_t openleft[4] = {{0.4,0.0},{1.0,0.0},{1.0,0.15},{0.4,0.15}};
static stg_point_t closedleft[4] = {{0.4,0.34},{1.0,0.34},{1.0,0.49},{0.4,0.49}};
static stg_point_t openright[4] = {{0.4,1.0},{1.0,1.0},{1.0,0.85},{0.4,0.85}};
static stg_point_t closedright[4] = {{0.4,0.66},{1.0,0.66},{1.0,0.51},{0.4,0.51}};
void gripper_load( stg_model_t* mod )
{
  stg_gripper_config_t gconf;
  stg_model_get_config( mod, &gconf, sizeof(gconf));
  // 
  stg_model_set_config( mod, &gconf, sizeof(gconf));
}

int gripper_update( stg_model_t* mod );
int gripper_startup( stg_model_t* mod );
int gripper_shutdown( stg_model_t* mod );
void gripper_render_data(  stg_model_t* mod );
void gripper_render_cfg( stg_model_t* mod );

stg_model_t* stg_gripper_create( stg_world_t* world, 
			       stg_model_t* parent, 
			       stg_id_t id, 
			       char* token )
{
  stg_model_t* mod = 
    stg_model_create( world, parent, id, STG_MODEL_GRIPPER, token );
  
  // we don't consume any power until subscribed
  //mod->watts = 0.0; 
  
  // override the default methods
  mod->f_startup = gripper_startup;
  mod->f_shutdown = gripper_shutdown;
  mod->f_update =  gripper_update;
  mod->f_render_data = gripper_render_data;
  mod->f_render_cfg = gripper_render_cfg;
  mod->f_load = gripper_load;

  // sensible gripper defaults
  stg_geom_t geom;
  geom.pose.x = 0;//-STG_DEFAULT_GRIPPER_SIZEX/2.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = STG_DEFAULT_GRIPPER_SIZEX;
  geom.size.y = STG_DEFAULT_GRIPPER_SIZEY;
  stg_model_set_geom( mod, &geom );

  // set up a gripper-specific config structure
  stg_gripper_config_t gconf;
  memset(&gconf,0,sizeof(gconf));  
  gconf.paddles = STG_GRIPPER_PADDLE_OPEN;
  gconf.lift = STG_GRIPPER_LIFT_DOWN;

  stg_model_set_config( mod, &gconf, sizeof(gconf) );
  
  // sensible starting command
  stg_gripper_cmd_t cmd; 
  cmd.cmd = STG_GRIPPER_CMD_NOP;
  cmd.arg = 0;  
  stg_model_set_command( mod, &cmd, sizeof(cmd) ) ;

  // set default color
  stg_color_t col = stg_lookup_color( STG_GRIPPER_GEOM_COLOR ); 
  stg_model_set_color( mod, &col );

  stg_polygon_t* polys = stg_polygons_create(3);

  stg_polygon_set_points( &polys[0], body, 4 );
  stg_polygon_set_points( &polys[1], openleft, 4 );
  stg_polygon_set_points( &polys[2], openright, 4 );

  stg_model_set_polygons( mod, polys, 3 );
  free(polys);
  
  return mod;
}

int gripper_update( stg_model_t* mod )
{   
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;
    
  stg_gripper_config_t cfg;
  stg_model_get_config( mod, &cfg, sizeof(cfg));
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  stg_gripper_cmd_t cmd;
  stg_model_get_command( mod, &cmd, sizeof(cmd));
  
  switch( cmd.cmd )
    {
    case 0:
      //puts( "gripper NOP" );
      break;
      
    case STG_GRIPPER_CMD_CLOSE:     
      if( cfg.paddles == STG_GRIPPER_PADDLE_OPEN )
	{
	  puts( "closing gripper paddles" );
	  cfg.paddles = STG_GRIPPER_PADDLE_CLOSED;
	  
	  size_t count=0;
	  stg_polygon_t* polys = stg_model_get_polygons(mod, &count);
	  stg_polygon_set_points( &polys[0], body, 4 );
	  stg_polygon_set_points( &polys[1], closedleft, 4 );
	  stg_polygon_set_points( &polys[2], closedright, 4 );	  
	  stg_model_set_polygons( mod, polys, 3 );
	}
      break;
      
    case STG_GRIPPER_CMD_OPEN:
      if( cfg.paddles == STG_GRIPPER_PADDLE_CLOSED )
	{
	  puts( "opening gripper paddles" );      
	  cfg.paddles = STG_GRIPPER_PADDLE_OPEN;
	  
	  size_t count=0;
	  stg_polygon_t* polys = stg_model_get_polygons(mod, &count);
	  stg_polygon_set_points( &polys[0], body, 4 );
	  stg_polygon_set_points( &polys[1], openleft, 4 );
	  stg_polygon_set_points( &polys[2], openright, 4 );	  
	  stg_model_set_polygons( mod, polys, 3 );
	}
      break;
      
    case STG_GRIPPER_CMD_UP:
      if( cfg.lift == STG_GRIPPER_LIFT_DOWN )
	{
	  puts( "raising gripper lift" );      
	  cfg.lift = STG_GRIPPER_LIFT_UP;
	}
      break;
      
    case STG_GRIPPER_CMD_DOWN:
      if( cfg.lift == STG_GRIPPER_LIFT_UP )
	{
	  puts( "lowering gripper lift" );      
	  cfg.lift = STG_GRIPPER_LIFT_DOWN;
	}      
      break;
      
    default:
      printf( "unknown gripper command %d\n",cmd.cmd );
    }
	      
  // get the sensor's pose in global coords
  stg_pose_t pz;
  memcpy( &pz, &geom.pose, sizeof(pz) ); 
  stg_model_local_to_global( mod, &pz );

  // cast the rays of the beams
  // compute beam and arm states

  // change the gripper's configuration in response to the commands
  stg_model_set_config( mod, &cfg, sizeof(cfg));
  stg_print_gripper_config( &cfg );

  _model_update( mod );

  return 0; //ok
}

int break_beam(stg_model_t* mod, int beam) {
  // computing beam test
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  stg_pose_t pz;
  memcpy( &pz, &geom.pose, sizeof(pz) ); 
  stg_model_local_to_global( mod, &pz );
  double bearing = pz.a - 3.14/2.0;
  itl_t* itl = itl_create( pz.x, pz.y, bearing, 
			   1.0, //cfg.range_max, DISTANCE BETWEEN PADDLES
			   mod->world->matrix, 
			   PointToBearingRange );
  return 0;
}

void gripper_render_data(  stg_model_t* mod )
{
  puts( "gripper render data" );
  if( mod->gui.data  )
    stg_rtk_fig_clear(mod->gui.data);
  else 
    {
      mod->gui.data = stg_rtk_fig_create( mod->world->win->canvas,
				      NULL, STG_LAYER_GRIPPERDATA );
      
      stg_rtk_fig_color_rgb32( mod->gui.data, stg_lookup_color(STG_GRIPPER_COLOR) );
    }
  
  if( mod->gui.data_bg )
    stg_rtk_fig_clear( mod->gui.data_bg );
  else // create the data background
    {
      mod->gui.data_bg = stg_rtk_fig_create( mod->world->win->canvas,
					 mod->gui.data, STG_LAYER_BACKGROUND );      
      stg_rtk_fig_color_rgb32( mod->gui.data_bg, 
			   stg_lookup_color( STG_GRIPPER_FILL_COLOR ));
    }
  
  stg_gripper_config_t cfg;
  assert( stg_model_get_config( mod, &cfg, sizeof(cfg))
	  == sizeof(stg_gripper_config_t ));
  stg_pose_t pose;
  stg_model_get_global_pose( mod, &pose );
  
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
  
  stg_rtk_fig_origin( mod->gui.data, pose.x, pose.y, pose.a );  
  stg_rtk_fig_color_rgb32( mod->gui.data, stg_lookup_color(STG_GRIPPER_BRIGHT_COLOR) );  
}

void gripper_render_cfg( stg_model_t* mod )
{ 
  if( mod->gui.cfg  )
    stg_rtk_fig_clear(mod->gui.cfg);
  else // create the figure, store it in the model and keep a local pointer
    mod->gui.cfg = stg_rtk_fig_create( mod->world->win->canvas, 
				   mod->gui.top, STG_LAYER_GRIPPERCONFIG );
  stg_rtk_fig_color_rgb32( mod->gui.cfg, stg_lookup_color( STG_GRIPPER_CFG_COLOR ));
  // get the config and make sure it's the right size
  stg_gripper_config_t cfg;
  size_t len = stg_model_get_config( mod, &cfg, sizeof(cfg));
  assert( len == sizeof(cfg) );
}

int gripper_startup( stg_model_t* mod )
{ 
  PRINT_DEBUG( "gripper startup" );
  return 0; // ok
}

int gripper_shutdown( stg_model_t* mod )
{ 
  PRINT_DEBUG( "gripper shutdown" );
    stg_model_set_data( mod, NULL, 0 );
  return 0; // ok
}

void stg_print_gripper_config( stg_gripper_config_t* cfg ) 
{
  char* pstate;
  switch( cfg->paddles )
    {
    case STG_GRIPPER_PADDLE_OPEN: pstate = "OPEN"; break;
    case STG_GRIPPER_PADDLE_CLOSED: pstate = "CLOSED"; break;
    case STG_GRIPPER_PADDLE_MOVING: pstate = "MOVING"; break;
    default: pstate = "*broken*";
    }
  
  char* lstate;
  switch( cfg->lift )
    {
    case STG_GRIPPER_LIFT_UP: lstate = "UP"; break;
    case STG_GRIPPER_LIFT_DOWN: lstate = "DOWN"; break;
    case STG_GRIPPER_LIFT_MOVING: lstate = "MOVING"; break;
    default: lstate = "*broken*";
    }
  
  printf("gripper state: paddles(%s) lift(%s)\n", pstate, lstate );
}

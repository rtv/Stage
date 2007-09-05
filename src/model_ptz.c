/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Device to simulate the ACTS vision system.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 28 Nov 2000
 * CVS info: $Id: model_ptz.c,v 1.4 2007-09-05 19:01:43 gerkey Exp $
 */

#include <math.h>

//#define DEBUG

#include "stage_internal.h"
#include "gui.h"

extern stg_rtk_fig_t* fig_debug_rays;

const double STG_DEFAULT_PTZ_PAN = 0.0;
const double STG_DEFAULT_PTZ_TILT = 0.0;
const double STG_DEFAULT_PTZ_ZOOM = DTOR(60);
const stg_color_t STG_PTZ_COLOR = 0x000080;
const double STG_PTZ_WATTS = 1.0; 
const double STG_PTZ_SIZE_X = 0.1;
const double STG_PTZ_SIZE_Y = 0.1;
const double STG_PTZ_SPEED_PAN = 1.0;
const double STG_PTZ_SPEED_TILT = 0.0;
const double STG_PTZ_SPEED_ZOOM = 0.3;

/** 
@ingroup model
@defgroup model_ptz Ptz model

The ptz model simulates a pan-tilt-zoom camera head.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
ptz
(
  # ptz properties
  ptz[0 0 60.0]
  ptz_speed[ 1.0 0.0 0.3]

  # model properties
  size [0.1 0.1]
  color "navy blue"
  watts 1.0
)
@endverbatim

@par Details
- ptz [float float float]
   - [pan_angle tilt_angle zoom_angle] 
   - control the pan, tilt and zoom angles and thus the field of view (fov) of the device. Tilt angle has no effect.
- ptz_speed [float float float]
   - [pan_speed tilt_speed zoom_speed]
   - Controls the speed at which the device servos to the desired angles, in radians per second.
*/

int ptz_init( stg_model_t* mod );
int ptz_startup( stg_model_t* mod );
int ptz_shutdown( stg_model_t* mod );
int ptz_update( stg_model_t* mod );
void ptz_load( stg_model_t* mod );

int ptz_render_data( stg_model_t* mod, void* userp );
int ptz_unrender_data( stg_model_t* mod, void* userp );
int ptz_render_cfg( stg_model_t* mod, void* userp );
int ptz_unrender_cfg( stg_model_t* mod, void* userp );

int ptz_init( stg_model_t* mod )
{
  // override the default methods
  mod->f_startup = ptz_startup;
  mod->f_shutdown = ptz_shutdown;
  mod->f_update = NULL;// installed at startup/shutdown
  mod->f_load = ptz_load;
  
  stg_geom_t geom;
  memcpy( &geom, &mod->geom, sizeof(geom));
  geom.size.x = STG_PTZ_SIZE_X;
  geom.size.y = STG_PTZ_SIZE_Y;
  stg_model_set_geom( mod, &geom );

  stg_model_set_color( mod, STG_PTZ_COLOR );

  stg_ptz_config_t cfg;
  memset(&cfg,0,sizeof(cfg));
  
  stg_ptz_data_t data;
  memset(&cfg,0,sizeof(cfg));

  data.pan = cfg.goal.pan = STG_DEFAULT_PTZ_PAN;
  data.tilt = cfg.goal.tilt = STG_DEFAULT_PTZ_TILT;
  data.zoom = cfg.goal.zoom = STG_DEFAULT_PTZ_ZOOM;
  
  cfg.speed.zoom = STG_PTZ_SPEED_ZOOM;
  cfg.speed.pan = STG_PTZ_SPEED_PAN;
  cfg.speed.tilt = STG_PTZ_SPEED_TILT;
   
  
  cfg.position_mode = TRUE;

  stg_model_set_cfg( mod, &cfg, sizeof(cfg) );
  stg_model_set_data( mod, &data, sizeof(data) );
  
  stg_model_add_callback( mod, &mod->data, ptz_render_data, NULL );

  return 0; //ok
}

void ptz_load( stg_model_t* mod )
{
  stg_ptz_data_t* data = (stg_ptz_config_t*)mod->data;
  
  data->pan = 
    wf_read_tuple_angle(mod->id, "ptz", 0, data->pan );
  data->tilt = 
    wf_read_tuple_angle(mod->id, "ptz", 1, data->tilt );
  data->zoom =  
    wf_read_tuple_angle(mod->id, "ptz", 2, data->zoom );
  
  model_change( mod, &mod->data );
  
  stg_ptz_config_t* cfg = (stg_ptz_config_t*)mod->cfg;
  
  cfg->speed.pan = 
    wf_read_tuple_angle(mod->id, "ptz_speed", 0, cfg->speed.pan );
  cfg->speed.tilt = 
    wf_read_tuple_angle(mod->id, "ptz_speed", 1, cfg->speed.tilt );
  cfg->speed.zoom =  
    wf_read_tuple_angle(mod->id, "ptz_speed", 2, cfg->speed.zoom );
  
  cfg->min.pan = 
    wf_read_tuple_angle(mod->id, "ptz_min", 0, cfg->min.pan );
  cfg->min.tilt = 
    wf_read_tuple_angle(mod->id, "ptz_min", 1, cfg->min.tilt );
  cfg->min.zoom =  
    wf_read_tuple_angle(mod->id, "ptz_min", 2, cfg->min.zoom );
  
  cfg->max.pan = 
    wf_read_tuple_angle(mod->id, "ptz_max", 0, cfg->max.pan );
  cfg->max.tilt = 
    wf_read_tuple_angle(mod->id, "ptz_max", 1, cfg->max.tilt );
  cfg->max.zoom =  
    wf_read_tuple_angle(mod->id, "ptz_max", 2, cfg->max.zoom );
  
  model_change( mod, &mod->cfg );
}

int ptz_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "ptz startup" );  
  
  mod->f_update = ptz_update;
  mod->watts = STG_PTZ_WATTS;
  
  return 0;
}

int ptz_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "ptz shutdown" );  
  
  mod->f_update = NULL;
  mod->watts = 0.0;
  
  return 0;
}


 
// servo to the PTZ goal angles
int ptz_update( stg_model_t* mod )
{
  PRINT_DEBUG( "ptz update" );  
  
  stg_ptz_data_t *data = (stg_ptz_data_t*)mod->data; 
  stg_ptz_config_t *cfg = (stg_ptz_config_t*)mod->cfg; 
    
  double pandist = cfg->speed.pan * mod->world->sim_interval/1e3;
  double panerror = cfg->goal.pan - data->pan;
  if( panerror < pandist && cfg->position_mode==TRUE)  //ignore goal commands in velocity mode
	pandist = panerror;		
  data->pan += pandist; 
  double zoomdist = cfg->speed.zoom * mod->world->sim_interval/1e3;
  double zoomerror = cfg->goal.zoom - data->zoom;
  if( zoomerror < zoomdist ) zoomdist = zoomerror;    
  if(cfg->position_mode == TRUE)                       //again, ignore goal command in velocity mode
	data->zoom += zoomdist; 

  model_change( mod, &mod->data );
   
  
  //printf( "PAN: %.2f\n", cfg->pan );
  //printf( "ZOOM: %.2f\n", cfg->zoom );

  return 0;
}


int ptz_unrender_data( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "blob_data_fig" );
  return 1;
}

int ptz_render_data( stg_model_t* mod, void* userp )
{ 
  PRINT_DEBUG( "ptz render" );  
  
  stg_ptz_data_t *data = (stg_ptz_data_t*)mod->data;
  stg_ptz_config_t *cfg = (stg_ptz_config_t*)mod->cfg;
    
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "ptz_data_fig" );
  
  if( fig == NULL )
    {
      fig = stg_model_fig_create( mod, "ptz_data_fig", "top", STG_LAYER_PTZDATA );
      stg_rtk_fig_color_rgb32( fig, 0x909090); // grey
    }

  stg_rtk_fig_clear( fig );

  double mina = data->pan + data->zoom / 2.0;
  double maxa = data->pan - data->zoom / 2.0;
  double delta = mod->geom.size.x/2.0;
  double dx = delta * cos(mina);
  double dy = delta * sin(mina);
  double ddx = delta * cos(maxa);
  double ddy = delta * sin(maxa);
    
  if( fig == NULL )
    fig = stg_model_fig_create( mod, "ptz_cfg_fig", "top", STG_LAYER_PTZDATA );
  
  stg_rtk_fig_line( fig, 0,0, dx, dy );
  stg_rtk_fig_line( fig, 0,0, ddx, ddy );
  stg_rtk_fig_line( fig, dx,dy, ddx, ddy );

 /*  double pts[3][2]; */
/*   pts[0][0] = 0.0; */
/*   pts[0][1] = 0.0; */
/*   pts[1][0] = dx; */
/*   pts[1][1] = dy; */
/*   pts[2][0] = ddx; */
/*   pts[2][1] = ddy; */
/*   stg_rtk_fig_polygon( fig, 0,0,0, 3, pts, 1 ); */

  /* stg_rtk_fig_ellipse_arc( fig, 0,0,0, */
/* 			   2.0*delta, */
/* 			   2.0*delta, */
/* 			   mina, maxa ); */

  return 0;
}

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
 * CVS info: $Id: model_blobfinder.c,v 1.18 2004-09-25 02:14:59 rtv Exp $
 */

#include <math.h>

//#define DEBUG

#include "stage.h"

extern rtk_fig_t* fig_debug;

void blobfinder_init( stg_model_t* mod );
int blobfinder_set_config( stg_model_t* mod, void* config, size_t len );
int blobfinder_set_data( stg_model_t* mod, void* data, size_t len );
int blobfinder_startup( stg_model_t* mod );
int blobfinder_shutdown( stg_model_t* mod );
void blobfinder_render_data( stg_model_t* mod );
void blobfinder_render_config( stg_model_t* mod );
int blobfinder_update( stg_model_t* mod );

// utility - we get the config quite a lot, so use this
stg_blobfinder_config_t* blobfinder_get_config( stg_model_t* mod )
{
  size_t len = 0;
  stg_blobfinder_config_t* cfg = 
    (stg_blobfinder_config_t*)stg_model_get_config(mod,&len);
  assert( cfg );
  assert( len == sizeof( stg_blobfinder_config_t ));
  return cfg;
}

void blobfinder_init( stg_model_t* mod )
{
  // sensible blobfinder defaults
  stg_geom_t geom;
  geom.pose.x = 0; //STG_DEFAULT_LASER_POSEX;
  geom.pose.y = 0; // STG_DEFAULT_LASER_POSEY;
  geom.pose.a = 0; //STG_DEFAULT_LASER_POSEA;
  geom.size.x = 0.01; //STG_DEFAULT_LASER_SIZEX;
  geom.size.y = 0.01; //STG_DEFAULT_LASER_SIZEY;
  stg_model_set_geom( mod, &geom );

  // a blobfinder has no body
  stg_model_set_lines( mod, NULL, 0 );

  // nothing can see a blobfinder
  //mod->obstacle_return = 0;
  //mod->laser_return = LaserTransparent;
  //mod->fiducial_return = FiducialNone;
  //stg_color_t col = stg_lookup_color("magenta");
  //stg_model_set_color( mod, &col );

  stg_blobfinder_config_t cfg;
  memset(&cfg,0,sizeof(cfg));
  
  cfg.scan_width = STG_DEFAULT_BLOB_SCANWIDTH;
  cfg.scan_height = STG_DEFAULT_BLOB_SCANHEIGHT;  
  cfg.range_max = STG_DEFAULT_BLOB_RANGEMAX;  
  cfg.pan = STG_DEFAULT_BLOB_PAN;
  cfg.tilt =STG_DEFAULT_BLOB_TILT;
  cfg.zoom = STG_DEFAULT_BLOB_ZOOM;
 
  cfg.channel_count = 6; //STG_DEFAULT_BLOB_CHANNELCOUNT;
  cfg.channels[0] = stg_lookup_color( "red" );
  cfg.channels[1] = stg_lookup_color( "green" );
  cfg.channels[2] = stg_lookup_color( "blue" );
  cfg.channels[3] = stg_lookup_color( "cyan" );
  cfg.channels[4] = stg_lookup_color( "yellow" );
  cfg.channels[5] = stg_lookup_color( "magenta" );
  
  //int c;
  //for( c=0; c<6; c++ )
  //PRINT_WARN2( "init channel %d has val %X", c, cfg.channels[c] );

  blobfinder_set_config( mod, &cfg,sizeof(cfg) );

  blobfinder_set_data( mod, NULL, 0 );
}

int blobfinder_set_data( stg_model_t* mod, void* data, size_t len )
{  
  // store the data
  _set_data( mod, data, len );
  
  // and redraw it
  blobfinder_render_data( mod );

  return 0; //OK
}
 
int blobfinder_set_config( stg_model_t* mod, void* config, size_t len )
{  
  // store the config
  _set_cfg( mod, config, len );
  
  // and redraw it
  blobfinder_render_config( mod);

  return 0; //OK
}


int blobfinder_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "blobfinder startup" );  
  
  return 0;
}

int blobfinder_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "blobfinder shutdown" );  
  
  // clear the data - this will unrender it too
  blobfinder_set_data( mod, NULL, 0 );
  return 0;
}


int blobfinder_update( stg_model_t* mod )
{
  PRINT_DEBUG( "blobfinder update" );  
  
  // only need to work if we're subscribed
  if( mod->subs < 1 )
    return 0; 

  size_t len = 0;
  stg_blobfinder_config_t* cfg = blobfinder_get_config(mod); 
  
  // Generate the scan-line image
  
  // Get the camera's global pose
  stg_pose_t pose;  
  stg_model_global_pose( mod, &pose );
  
  double ox = pose.x;
  double oy = pose.y;
  double oth = pose.a;

  // Compute starting angle
  oth = oth + cfg->pan + cfg->zoom / 2.0;
  
  // Compute fov, range, etc
  double dth = cfg->zoom / cfg->scan_width;

  // Make sure the data buffer is big enough
  //ASSERT((size_t)m_scan_width<=sizeof(m_scan_channel)/sizeof(m_scan_channel[0]));
  
  //printf( "scan width %d\n", cfg->scan_width );

  // record the color of every ray we project
  stg_color_t* colarray = 
    (stg_color_t*)calloc( sizeof(stg_color_t),cfg->scan_width); 

  // record the range of every ray we project
  double* rngarray = 
    (double*)calloc( sizeof(double),cfg->scan_width); 
  
  // Do each scan
  // Note that the scan is taken *clockwise*
  // i'm scanning this as half-resolution for a significant speed-up

  //int num_blobs = 0;
  stg_color_t col;

  if( fig_debug ) rtk_fig_clear( fig_debug );

  int s;
  for( s = 0; s < cfg->scan_width; s++)
    {
      //printf( "scan %d of %d\n", s, m_scan_width );

      // indicate no valid color found (MSB normally unused in 32bit RGB value)
      col = 0xFF000000; 
      
      // Compute parameters of scan line
      double px = ox;
      double py = oy;
      double pth = oth - s * dth;
    
      itl_t*  itl = itl_create( px, py, pth, 
				cfg->range_max, 
				mod->world->matrix, 
				PointToBearingRange );
      
      stg_model_t* ent;
      double range = cfg->range_max;
      
      while( (ent = itl_next( itl ) ))
	{
	  // Ignore itself, its ancestors and its descendents
	  if( ent == mod )
	    continue;
	  
	  // Ignore transparent things
	  if( !ent->blob_return )
	    continue;
	  
	  if(  stg_model_is_related( mod, ent ) )
	    {
	      PRINT_DEBUG2( "blob \"%s\" ignoring \"%s\" as a relative",
			    mod->token, ent->token );
	      continue;
	    }
	  
	  range = itl->range; // it's this far away
	  
	  // get the color of the entity
	  stg_color_t hiscol = stg_model_get_color(ent);
	  memcpy( &col, &hiscol, sizeof( stg_color_t ) );
	  
	  break;
	}
    
      itl_destroy( itl );

      // if we found a color on this ray
      if( !(col & 0xFF000000) ) 
	{
	  PRINT_DEBUG3( "ray %d sees color %0X at range %.2f", s, col, range );
	  
	  //colarray[s] = col;
	  //rngarray[s] = range;
	  
	  // look up this color in the color/channel mapping array
	  int c;
	  for( c=0; c < cfg->channel_count; c++ )
	    {
	      //PRINT_DEBUG3( "looking up %X - channel %d is %X",
	      //	    col, c, cfg->channels[c] );

	      if( cfg->channels[c] == col)
		{
		  //printf("color %0X is channel %d\n", col, c);
		  
		  colarray[s] = c+1;
		  rngarray[s] = range;
		
		  break;
		}
	    }
	}
    }
  
  //printf( "model %p colors: ", mod );
  //int g;
  //for( g=0; g<cfg->scan_width; g++ )
  //  printf( "%0X ", colarray[g] );
  // puts("");

  // now the colors and ranges are filled in - time to do blob detection
  float yRadsPerPixel = cfg->zoom / cfg->scan_height;

  int blobleft = 0, blobright = 0;
  unsigned char blobcol = 0;
  int blobtop = 0, blobbottom = 0;
  
  GArray* blobs = g_array_new( FALSE, TRUE, sizeof(stg_blobfinder_blob_t) );
  
  // scan through the samples looking for color blobs
  for( s=0; s < cfg->scan_width; s++ )
    {
      if( colarray[s] > 0 &&
	  colarray[s] < STG_BLOB_CHANNELS_MAX )
	{
	  blobleft = s;
	  blobcol = colarray[s];
	  
	  //printf( "blob start %d\n", blobleft );

	  // loop until we hit the end of the blob
	  // there has to be a gap of >1 pixel to end a blob
	  // this avoids getting lots of crappy little blobs
	  while( colarray[s] == blobcol || colarray[s+1] == blobcol ) 
	    s++;
	  
	  blobright = s-1;

	  //printf( "blob end %d\n", blobright );

	  double robotHeight = 0.6; // meters
	  int xCenterOfBlob = blobleft + ((blobright - blobleft )/2);
	  double rangeToBlobCenter = rngarray[ xCenterOfBlob ];
	  if(!rangeToBlobCenter)
	    {
	      // if the range to the "center" is zero, then use the range
	      // to the start.  this can happen, for example, when two 1-pixel
	      // blobs that are 1 pixel apart are grouped into a single blob in 
	      // whose "center" there is really no blob at all
	      rangeToBlobCenter = rngarray[ blobleft ];
	    }
	  double startyangle = atan2( robotHeight/2.0, rangeToBlobCenter );
	  double endyangle = -startyangle;
	  blobtop = cfg->scan_height/2 - (int)(startyangle/yRadsPerPixel);
	  blobbottom = cfg->scan_height/2 -(int)(endyangle/yRadsPerPixel);
	  int yCenterOfBlob = blobtop +  ((blobbottom - blobtop )/2);
	  
	  if (blobtop < 0)
	    blobtop = 0;
	  if (blobbottom > cfg->scan_height - 1)
	    blobbottom = cfg->scan_height - 1;
	  
	  
	  // fill in an array entry for this blob
	  //
	  stg_blobfinder_blob_t blob;
	  memset( &blob, 0, sizeof(blob) );
	  
	  blob.channel = blobcol-1;
	  blob.color = cfg->channels[ blob.channel];
	  blob.xpos = xCenterOfBlob;
	  blob.ypos = yCenterOfBlob;
	  blob.left = blobleft;
	  blob.top = blobtop;
	  blob.right = blobright;
	  blob.bottom = blobbottom;
	  blob.area = (blobtop - blobbottom) * (blobleft-blobright);
	  blob.range = (int)(rangeToBlobCenter*1000.0); 
	  
	  //printf( "Robot %p sees %d xpos %d ypos %d\n",
	  //  mod, blob.color, blob.xpos, blob.ypos );
	  
	  // add the blob to our stash
	  g_array_append_val( blobs, blob );
	}
    }

  blobfinder_set_data( mod, 
		       blobs->data, 
		       blobs->len * sizeof(stg_blobfinder_blob_t) );
  
  g_array_free( blobs, TRUE );

  PRINT_DEBUG( "blobfinder data service done" );  

  return 0; //OK
}


void blobfinder_render_data( stg_model_t* mod )
{ 
  PRINT_DEBUG( "blobfinder render" );  

 
  
  if( mod->gui.data  )
    rtk_fig_clear(mod->gui.data);
  else // create the figure, store it in the model and keep a local pointer
    mod->gui.data = rtk_fig_create(  mod->world->win->canvas, 
				     NULL, 
				     STG_LAYER_BLOBDATA );
  
  rtk_fig_t* fig = mod->gui.data;  
  
  // place the visualization a little away from the device
  stg_pose_t pose;
  stg_model_global_pose( mod, &pose );
  
  pose.x -= 1.0;
  pose.y += 1.0;
  pose.a = 0.0;
  rtk_fig_origin( fig, pose.x, pose.y, pose.a );
  
  double scale = 0.01; // shrink from pixels to meters for display
  
  stg_blobfinder_config_t* cfg = blobfinder_get_config( mod );
  
  size_t len = 0;
  stg_blobfinder_blob_t* blobs =  stg_model_get_data(mod,&len);
  
  if( len < sizeof(stg_blobfinder_blob_t) )
    return; // no data to render
  
  int num_blobs = len / sizeof(stg_blobfinder_blob_t);
  
  short width = cfg->scan_width;
  short height = cfg->scan_height;
  double mwidth = width * scale;
  double mheight = height * scale;
  
  // the view outline rectangle
  rtk_fig_color_rgb32(fig, 0xFFFFFF);
  rtk_fig_rectangle(fig, 0.0, 0.0, 0.0, mwidth,  mheight, 1 ); 
  rtk_fig_color_rgb32(fig, 0x000000);
  rtk_fig_rectangle(fig, 0.0, 0.0, 0.0, mwidth,  mheight, 0); 
  
  int c;
  for( c=0; c<num_blobs; c++)
    {
      stg_blobfinder_blob_t* blob = &blobs[c];
      
      // set the color from the blob data
      rtk_fig_color_rgb32( fig, blob->color ); 
      
      short top =   blob->top;
      short bot =   blob->bottom;
      short left =   blob->left;
      short right =   blob->right;
      
      double mtop = top * scale;
      double mbot = bot * scale;
      double mleft = left * scale;
      double mright = right * scale;
      
      // get the range in meters
      //double range = (double)ntohs(data.blobs[index+b].range) / 1000.0; 
      
      rtk_fig_rectangle(fig, 
			-mwidth/2.0 + (mleft+mright)/2.0, 
			-mheight/2.0 +  (mtop+mbot)/2.0,
			0.0, 
			mright-mleft, 
			mbot-mtop, 
			1 );
    }
}

void blobfinder_render_config( stg_model_t* mod )
{ 
  PRINT_DEBUG( "blobfinder render config" );  
  
  
  if( mod->gui.cfg  )
    rtk_fig_clear(mod->gui.cfg);
  else // create the figure, store it in the model and keep a local pointer
    mod->gui.cfg = rtk_fig_create( mod->world->win->canvas,
				   mod->gui.top, STG_LAYER_BLOBCONFIG );
  
  rtk_fig_t* fig = mod->gui.cfg;  
  
  
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_BLOB_CFG_COLOR ));
  
  stg_blobfinder_config_t* cfg = blobfinder_get_config(mod);
  
  // Get the camera's global pose
  stg_pose_t* pose = stg_model_get_pose( mod );
  
  double ox = pose->x;
  double oy = pose->y;
  double mina = pose->a + (cfg->pan + cfg->zoom / 2.0);
  double maxa = pose->a - (cfg->pan + cfg->zoom / 2.0);
  
  double dx = cfg->range_max * cos(mina);
  double dy = cfg->range_max * sin(mina);
  double ddx = cfg->range_max * cos(maxa);
  double ddy = cfg->range_max * sin(maxa);
  
  rtk_fig_line( fig, ox,oy, dx, dy );
  rtk_fig_line( fig, ox,oy, ddx, ddy );
  rtk_fig_ellipse_arc( fig, 0,0,0,
		       2.0*cfg->range_max,
		       2.0*cfg->range_max, 
		       mina, maxa );      
}


int register_blobfinder( lib_entry_t* lib )
{ 
  assert(lib);
  
  lib[STG_MODEL_BLOB].init = blobfinder_init;
  lib[STG_MODEL_BLOB].update = blobfinder_update;
  lib[STG_MODEL_BLOB].startup = blobfinder_startup;
  lib[STG_MODEL_BLOB].shutdown = blobfinder_shutdown;
  lib[STG_MODEL_BLOB].set_config = blobfinder_set_config;
  lib[STG_MODEL_BLOB].set_data = blobfinder_set_data;

  return 0; //ok
} 

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
 * CVS info: $Id: model_blobfinder.c,v 1.4 2004-06-13 07:27:55 rtv Exp $
 */

#include <math.h>

#include "model.h"
#include "raytrace.h"

#define DEBUG

#include "gui.h"
extern rtk_fig_t* fig_debug;


void model_blobfinder_init( model_t* mod )
{
  stg_blobfinder_config_t cfg;
  memset( &cfg, 0, sizeof(stg_blobfinder_config_t) );
  
  cfg.scan_width  = 160; // pixels
  cfg.scan_height = 120;
  cfg.range_max = 8.0; // meters

  cfg.pan = 0; // radians
  cfg.tilt = 0;
  cfg.zoom = DTOR( 60 ); // field of view
  
  // reasonable channel defaults
  cfg.channel_count = 6;
  cfg.channels[0] = stg_lookup_color( "red" );
  cfg.channels[1] = stg_lookup_color( "green" );
  cfg.channels[2] = stg_lookup_color( "blue" );
  cfg.channels[3] = stg_lookup_color( "yellow" );
  cfg.channels[4] = stg_lookup_color( "cyan" );
  cfg.channels[5] = stg_lookup_color( "magenta" );
  
  model_set_prop_generic( mod, STG_PROP_BLOBCONFIG, 
			  &cfg, sizeof(cfg) );
}

void model_blobfinder_startup( model_t* mod )
{
  PRINT_DEBUG( "blobfinder startup" );

}

void model_blobfinder_shutdown( model_t* mod )
{
  PRINT_DEBUG( "blobfinder shutdown" );  
}


void model_blobfinder_update( model_t* mod )
{  
  stg_blobfinder_config_t* cfg = (stg_blobfinder_config_t*)
    model_get_prop_data_generic( mod, STG_PROP_BLOBCONFIG );
  
  assert( cfg );

  // Generate the scan-line image

  // Get the camera's global pose
  // TODO: have the pose configurable
  stg_pose_t pose;
  pose.x = pose.y = pose.a = 0.0;
  model_local_to_global( mod, &pose );
  
  double ox = pose.x;
  double oy = pose.y;
  double oth = pose.a;

  // Compute starting angle
  oth = oth + cfg->pan + cfg->zoom / 2.0;
  
  // Compute fov, range, etc
  double dth = cfg->zoom / cfg->scan_width;

  // Make sure the data buffer is big enough
  //ASSERT((size_t)m_scan_width<=sizeof(m_scan_channel)/sizeof(m_scan_channel[0]));
  
  // record the color of every ray we project
  stg_color_t* colarray = 
    (stg_color_t*)calloc( sizeof(stg_color_t),cfg->scan_width); 

  // record the range of every ray we project
  double* rngarray = 
    (double*)calloc( sizeof(double),cfg->scan_width); 
  
  // Do each scan
  // Note that the scan is taken *clockwise*
  // i'm scanning this as half-resolution for a significant speed-up

  int num_blobs = 0;
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
      
      model_t* ent;
      double range = cfg->range_max;
      
      while( (ent = itl_next( itl ) ))
	{
	  //printf( "ent %p (%s), vision_return %d\n",
	  //  ent, ent->token, ent->color );
	  
	  // Ignore itself, its ancestors and its descendents
	  if( ent == mod )//|| this->IsDescendent(ent) || ent->IsDescendent(this))
	    continue;
	  
	  // Ignore transparent things
	  //if( !ent->vision_return )
	  //  continue;

	  range = itl->range; // it's this far away

	  // get the color of the entity
	  memcpy( &col, &(ent->color), sizeof( stg_color_t ) );
	  
	  break;
	}
    
      // if we found a color on this ray
      if( !(col & 0xFF000000) ) 
	{
	  //colarray[s] = col;
	  //rngarray[s] = range;
	  
	  // look up this color in the color/channel mapping array
	  int c;
	  for( c=0; c < cfg->channel_count; c++ )
	    {
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
  
  //printf( "scanning for blobs\n" );
  
  // scan through the samples looking for color blobs
  for( s=0; s < cfg->scan_width; s++ )
    {
      if( colarray[s] > 0 &&
	  colarray[s] < STG_BLOBFINDER_CHANNELS_MAX )
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

  model_set_prop_generic( mod, STG_PROP_BLOBDATA,
			  blobs->data, blobs->len * sizeof(stg_blobfinder_blob_t) );
  
  g_array_free( blobs, TRUE );
}

void model_blobfinder_render( model_t* mod )
{ 
  gui_window_t* win = mod->world->win;
  rtk_fig_t* fig = gui_model_figs(mod)->blob_data;  
  rtk_fig_clear(fig);

  if( win->show_blobdata && mod->subs[STG_PROP_BLOBDATA] )
    {
      // place the visualization a little away from the device
      stg_pose_t pose;
      pose.x = pose.y = pose.a = 0.0;
  
      model_local_to_global( mod, &pose );
  
      pose.x += 1.0;
      pose.y += 1.0;
      pose.a = 0.0;
      rtk_fig_origin( fig, pose.x, pose.y, pose.a );

      double scale = 0.007; // shrink from pixels to meters for display
  
      stg_blobfinder_config_t* cfg = (stg_blobfinder_config_t*) 
	model_get_prop_data_generic( mod, STG_PROP_BLOBCONFIG );
      assert(cfg);
  
      stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_BLOBDATA );
  
      if( prop == NULL )
	return; // no data to render yet
  
      stg_blobfinder_blob_t* blobs = (stg_blobfinder_blob_t*)prop->data;
      if( blobs == NULL )
	return; // no data to render yet
  
      int num_blobs = prop->len / sizeof(stg_blobfinder_blob_t);
      if( num_blobs < 1 )
	return; // no data to render yet
  
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
}

void model_blobfinder_config_render( model_t* mod )
{ 
  gui_window_t* win = mod->world->win;

  rtk_fig_t* fig = gui_model_figs(mod)->blob_cfg;  
  rtk_fig_clear(fig);
  rtk_fig_color_rgb32( fig, stg_lookup_color( STG_BLOB_CFG_COLOR ));
  
  if( win->show_blobcfg )
    {
      stg_blobfinder_config_t* cfg = (stg_blobfinder_config_t*) 
	model_get_prop_data_generic( mod, STG_PROP_BLOBCONFIG );
      
      if( cfg )
	{  
	  // Get the camera's global pose
	  stg_pose_t pose;
	  pose.x = pose.y = pose.a = 0.0;
	  model_local_to_global( mod, &pose );
	  
	  double ox = pose.x;
	  double oy = pose.y;
	  double mina = pose.a + (cfg->pan + cfg->zoom / 2.0);
	  double maxa = pose.a - (cfg->pan + cfg->zoom / 2.0);
	  
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
    }
}



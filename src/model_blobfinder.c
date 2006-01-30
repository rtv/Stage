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
 * CVS info: $Id: model_blobfinder.c,v 1.58 2006-01-30 06:58:15 rtv Exp $
 */

#include <math.h>

//#define DEBUG

#include "stage_internal.h"
#include "gui.h"

extern stg_rtk_fig_t* fig_debug_rays;

#define STG_DEFAULT_BLOB_CHANNELCOUNT 6
#define STG_DEFAULT_BLOB_SCANWIDTH 80
#define STG_DEFAULT_BLOB_SCANHEIGHT 60 
#define STG_DEFAULT_BLOB_RANGEMAX 8.0 

const int STG_BLOBFINDER_BLOBS_MAX = 32;
const double STG_BLOB_WATTS = 10.0; // power consumption


/** 
@ingroup model
@defgroup model_blobfinder Blobfinder model

The blobfinder model simulates a color-blob-finding vision device,
like a CMUCAM2, or the ACTS image processing software. It can track
areas of color in a simulated 2D image, giving the location and size
of the color 'blobs'. Multiple colors can be tracked at once; they are
separated into channels, so that e.g. all red objects are tracked as
channel one, blue objects in channel two, etc. The color associated
with each channel is configurable.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
ptz(
  # blobfinder MUST be a child of a PTZ model
  blobfinder
  (
    # blobfinder properties
    channel_count 6
    channels ["red" "green" "blue" "cyan" "yellow" "magenta" ]
    range_max 8.0
    ptz[0 0 60.0]
    image[80 60]

    # model properties
    size [0.01 0.01]
  )
)
@endverbatim

@par Details
- channel_count int
  - number of channels; i.e. the number of discrete colors detected
- channels [ string ... ]
  - define the colors detected in each channel, using color names from the X11 database 
   (usually /usr/X11R6/lib/X11/rgb.txt). The number of strings should match channel_count.
- image [int int]
  - [width height]
  - dimensions of the image in pixels. This determines the blobfinder's 
    resolution
- ptz [float float float]
   - [pan_angle tilt_angle zoom_angle] 
   - control the panning, tilt and zoom angle (fov) of the blobfinder. Tilt angle currently has no effect.
- range_max float
   - maximum range of the sensor in meters.

*/

int blobfinder_init( stg_model_t* mod );
int blobfinder_startup( stg_model_t* mod );
int blobfinder_shutdown( stg_model_t* mod );
int blobfinder_update( stg_model_t* mod );
void blobfinder_load( stg_model_t* mod );

int blobfinder_render_data( stg_model_t* mod, void* userp );
int blobfinder_unrender_data( stg_model_t* mod, void* userp );
int blobfinder_render_cfg( stg_model_t* mod, void* userp );
int blobfinder_unrender_cfg( stg_model_t* mod, void* userp );

int blobfinder_init( stg_model_t* mod )
{
  // override the default methods
  mod->f_startup = blobfinder_startup;
  mod->f_shutdown = blobfinder_shutdown;
  mod->f_update = NULL;// installed at startup/shutdown
  mod->f_load = blobfinder_load;
  
  
  stg_geom_t geom;
  memset( &geom, 0, sizeof(geom));
  stg_model_set_geom( mod, &geom );
  
  stg_blobfinder_config_t cfg;
  memset(&cfg,0,sizeof(cfg));
  
  cfg.scan_width = STG_DEFAULT_BLOB_SCANWIDTH;
  cfg.scan_height = STG_DEFAULT_BLOB_SCANHEIGHT;  
  cfg.range_max = STG_DEFAULT_BLOB_RANGEMAX;  
  
  cfg.channel_count = 6; 
  cfg.channels[0] = stg_lookup_color( "red" );
  cfg.channels[1] = stg_lookup_color( "green" );
  cfg.channels[2] = stg_lookup_color( "blue" );
  cfg.channels[3] = stg_lookup_color( "cyan" );
  cfg.channels[4] = stg_lookup_color( "yellow" );
  cfg.channels[5] = stg_lookup_color( "magenta" );
  
  stg_model_set_cfg( mod, &cfg, sizeof(cfg) );
  
  //int c;
  //for( c=0; c<6; c++ )
  //PRINT_WARN2( "init channel %d has val %X", c, cfg.channels[c] );
  
  stg_model_set_data( mod, NULL, 0 );
  stg_model_set_polygons( mod, NULL, 0 );
  
  stg_model_add_callback( mod, &mod->data, blobfinder_render_data, NULL );

  stg_model_add_property_toggles( mod, &mod->data,
				  blobfinder_render_data, // called when toggled on
				  NULL,
				  blobfinder_unrender_data, // called when toggled off
				  NULL,
				  "blob_data",
				  "blob data",
				  TRUE );

  stg_model_add_property_toggles( mod, &mod->cfg,
				  blobfinder_render_cfg, // called when toggled on
				  NULL,
				  blobfinder_unrender_cfg, // called when toggled off
				  NULL,
				  "blob_cfg",
				  "blob config",
				  FALSE );

  return 0; //ok
}

void blobfinder_load( stg_model_t* mod )
{
  stg_blobfinder_config_t* now = (stg_blobfinder_config_t*)mod->cfg; 
  assert(now);
  
  stg_blobfinder_config_t bcfg;
  memcpy( &bcfg, now, sizeof(bcfg));

  bcfg.channel_count = 
    wf_read_int(mod->id, "channel_count", now->channel_count );
  
  bcfg.scan_width = (int)
    wf_read_tuple_float(mod->id, "image", 0, now->scan_width );
  bcfg.scan_height = (int)
    wf_read_tuple_float(mod->id, "image", 1, now->scan_height );	    
  bcfg.range_max = 
    wf_read_length(mod->id, "range_max", now->range_max );

  if( bcfg.channel_count > STG_BLOB_CHANNELS_MAX )
    bcfg.channel_count = STG_BLOB_CHANNELS_MAX;
  
  // TODO - load the channel specification properly
  int ch;
  for( ch = 0; ch<bcfg.channel_count; ch++ )
    { 
      const char* colorstr = wf_read_tuple_string( mod->id, "channels", ch, NULL );
      
      if( ! colorstr )
	break;
      
      bcfg.channels[ch] = stg_lookup_color( colorstr );        
    }    
  
  stg_model_set_cfg( mod, &bcfg, sizeof(bcfg));
}

int blobfinder_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "blobfinder startup" );  
  
  mod->f_update = blobfinder_update;
  //mod->watts = STG_BLOB_WATTS;
  
  return 0;
}

int blobfinder_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "blobfinder shutdown" );  
  
  mod->f_update = NULL;
  //mod->watts = 0.0;
  
  // clear the data - this will unrender it too
  stg_model_set_data( mod, NULL, 0 );
  return 0;
}


int blobfinder_raytrace_filter( stg_model_t* finder, stg_model_t* found )
{
  if( found->blob_return && !stg_model_is_related( finder, found ))
    return 1;  
  return 0;
}


// we check that our parent is a PTZ model by making sure it's startup
// method is this one
int ptz_startup( stg_model_t* mod );

int blobfinder_update( stg_model_t* mod )
{
  PRINT_DEBUG( "blobfinder update" );  

  // we MUST have a PTZ parent model
  assert( mod->parent );
  assert( mod->parent->f_startup == ptz_startup );
  stg_ptz_data_t* ptz = (stg_ptz_data_t*)mod->parent->data;

  stg_blobfinder_config_t *cfg = (stg_blobfinder_config_t*)mod->cfg; 
  
  // Generate the scan-line image
  // Get the camera's global pose
  stg_pose_t pose;  
  stg_model_get_global_pose( mod, &pose );
  
  double ox = pose.x;
  double oy = pose.y;
  double oth = pose.a;

  // Compute starting angle
  oth = oth + ptz->pan + ptz->zoom / 2.0;
  
  // Compute fov, range, etc
  double dth = ptz->zoom / cfg->scan_width;

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

  if( fig_debug_rays ) stg_rtk_fig_clear( fig_debug_rays );

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

      ent = itl_first_matching( itl, blobfinder_raytrace_filter, mod );

      if( ent )
	{
	  range = itl->range; // it's this far away	
	  col = ent->color; // it's this color
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
  float yRadsPerPixel = ptz->zoom / cfg->scan_height;

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

  //PRINT_WARN1( "blobfinder setting data %d bytes of data", 
  //       blobs->len * sizeof(stg_blobfinder_blob_t) );
  
  
  stg_model_set_data( mod, 
		      blobs->data, 
		      blobs->len * sizeof(stg_blobfinder_blob_t) );
  
  g_array_free( blobs, TRUE );

  free( colarray );
  free( rngarray );

  PRINT_DEBUG( "blobfinder data service done" );  

  return 0; //OK
}

int blobfinder_unrender_data( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "blob_data_fig" );
  return 1;
}

int blobfinder_render_data( stg_model_t* mod, void* userp )
{ 
  PRINT_DEBUG( "blobfinder render" );  
  
  stg_blobfinder_blob_t *blobs = (stg_blobfinder_blob_t*)mod->data;
  int num_blobs = mod->data_len / sizeof(stg_blobfinder_blob_t);

    
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "blob_data_fig" );
  
  if( fig == NULL )
    fig = stg_model_fig_create( mod, "blob_data_fig", NULL, STG_LAYER_BLOBDATA );
  
  stg_rtk_fig_clear( fig );

  if( num_blobs < 1 )
    return 0;

  // place the visualization a little away from the device
  stg_pose_t pose;
  //memset(&pose, 0, sizeof(pose));
  stg_model_get_global_pose( mod, &pose );
  
  pose.x -= 1.0;
  pose.y += 1.0;
  pose.a = 0.0;
  stg_rtk_fig_origin( fig, pose.x, pose.y, pose.a );
  
  double scale = 0.01; // shrink from pixels to meters for display
  
  stg_blobfinder_config_t *cfg = (stg_blobfinder_config_t*)mod->cfg;  

  
  short width = cfg->scan_width;
  short height = cfg->scan_height;
  double mwidth = width * scale;
  double mheight = height * scale;
  
  // the view outline rectangle
  stg_rtk_fig_color_rgb32(fig, 0xFFFFFF);
  stg_rtk_fig_rectangle(fig, 0.0, 0.0, 0.0, mwidth,  mheight, 1 ); 
  stg_rtk_fig_color_rgb32(fig, 0x000000);
  stg_rtk_fig_rectangle(fig, 0.0, 0.0, 0.0, mwidth,  mheight, 0); 
  
  int c;
  for( c=0; c<num_blobs; c++)
    {
      stg_blobfinder_blob_t* blob = &blobs[c];
      
      // set the color from the blob data
      stg_rtk_fig_color_rgb32( fig, blob->color ); 
      
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
      
      stg_rtk_fig_rectangle(fig, 
			    -mwidth/2.0 + (mleft+mright)/2.0, 
			    -mheight/2.0 +  (mtop+mbot)/2.0,
			    0.0, 
			    mright-mleft, 
			    mbot-mtop, 
			    1 );
    }

  return 0;
}

int blobfinder_unrender_cfg( stg_model_t* mod, void* userp )
{
  stg_rtk_fig_clear( stg_model_get_fig( mod, "blob_cfg_fig" ));
  return 1;
}

int blobfinder_render_cfg( stg_model_t* mod, void* userp )
{
  PRINT_DEBUG( "blobfinder render config" );

  // we MUST have a PTZ parent model
  assert( mod->parent );
  assert( mod->parent->f_startup == ptz_startup );
  stg_ptz_data_t* ptz = (stg_ptz_data_t*)mod->parent->data;

  stg_blobfinder_config_t *cfg = (stg_blobfinder_config_t*)mod->cfg;
  assert(cfg);
  
  stg_pose_t* pose = &mod->pose;
  
  double ox = pose->x;
  double oy = pose->y;
  double mina = pose->a + (ptz->pan + ptz->zoom / 2.0);
  double maxa = pose->a + (ptz->pan - ptz->zoom / 2.0);
  
  double dx = cfg->range_max * cos(mina);
  double dy = cfg->range_max * sin(mina);
  double ddx = cfg->range_max * cos(maxa);
  double ddy = cfg->range_max * sin(maxa);
  
  stg_rtk_fig_t* fig = stg_model_get_fig( mod, "blob_cfg_fig" );
  
  if( fig == NULL )
    fig = stg_model_fig_create( mod, "blob_cfg_fig", "top", STG_LAYER_BLOBDATA );
  
  stg_rtk_fig_clear( fig );

  stg_rtk_fig_color_rgb32( fig, stg_lookup_color( STG_BLOB_CFG_COLOR ));
  stg_rtk_fig_line( fig, ox,oy, dx, dy );
  stg_rtk_fig_line( fig, ox,oy, ddx, ddy );
  stg_rtk_fig_ellipse_arc( fig, 0,0,0,
			   2.0*cfg->range_max,
			   2.0*cfg->range_max,
			   mina, maxa );

  return 0;
}

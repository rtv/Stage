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
 * CVS info: $Id: visiondevice.cc,v 1.3.8.2 2004-10-08 15:58:05 gerkey Exp $
 */

#include <math.h>

#include <iostream>

#include "world.hh"
#include "visiondevice.hh"
#include "ptzdevice.hh"
#include "raytrace.hh"

#define DEBUG

///////////////////////////////////////////////////////////////////////////
// Default constructor
CVisionDevice::CVisionDevice(LibraryItem* libit,CWorld *world, CPtzDevice *parent)
        : CPlayerEntity( libit, world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_blobfinder_data_t ); 
  m_command_len = 0;
  m_config_len  = 0;
  m_reply_len  = 0;
 
  m_player.code = PLAYER_BLOBFINDER_CODE;

  m_interval = 0.1; // 10Hz - the real cam is around this

  // If the parent is a ptz device we will use it, otherwise
  // we will operate as a naked vision device.
  if (parent->m_player.code == PLAYER_PTZ_CODE )
    m_ptz_device = parent;
  else
    m_ptz_device = NULL;
  
  cameraImageWidth = 160;
  cameraImageHeight = 120;

  m_scan_width = 160;

  m_pan = 0;
  m_tilt = 0;
  m_zoom = DTOR(60);

  m_max_range = 8.0;

  numBlobs = 0;
  memset( blobs, 0, PLAYER_BLOBFINDER_MAX_BLOBS * sizeof( player_blobfinder_blob_t ) );

#ifdef INCLUDE_RTK2
  vision_fig = NULL;
#endif
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CVisionDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the worldfile
bool CVisionDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CVisionDevice::Update( double sim_time )
{
  ASSERT(m_world != NULL);

  CPlayerEntity::Update( sim_time );

  // Dont update anything if we are not subscribed
  if( Subscribed() < 1 )
    return;
  
  // See if its time to recalculate vision
  if( sim_time - m_last_update < m_interval )
    return;

  m_last_update = sim_time;
  
  //RTK_TRACE0("generating new data");
  
  // Generate the scan-line image
  UpdateScan();
  
  // Generate ACTS data  
  player_blobfinder_data_t data;
  memset( &data, 0, sizeof(data) );
  UpdateACTS( &data );
  
  // Copy data to the output buffer
  PutData( &data, sizeof(data) );
}


///////////////////////////////////////////////////////////////////////////
// Generate the scan-line image
void CVisionDevice::UpdateScan()
{
  // Get the camera's global pose
  double ox, oy, oth;
  GetGlobalPose(ox, oy, oth);

  // Get the ptz settings
  if (m_ptz_device != NULL)
    m_ptz_device->GetPTZ(m_pan, m_tilt, m_zoom);

  // Compute starting angle
  oth = oth + m_pan + m_zoom / 2;

  // Compute fov, range, etc
  double dth = m_zoom / m_scan_width;

  // Make sure the data buffer is big enough
  ASSERT((size_t)m_scan_width<=sizeof(m_scan_color)/sizeof(m_scan_color[0]));

  // TODO
  //int skip = (int) (m_world->m_vision_res / m_scan_res - 0.5);

  // Do each scan
  // Note that the scan is taken *clockwise*
  // i'm scanning this as half-resolution for a significant speed-up

  StageColor col;
  
  for (int s = 0; s < m_scan_width; s++)
  {
    // initialize the reading 
    m_scan_color[s] = 0; 
    m_scan_range[s] = -1; // range < 0 is no-blob

    // indicate no valid color found (MSB normally unused in 32bit RGB value)
    col = 0xFF000000; 
      
    // Compute parameters of scan line
    double px = ox;
    double py = oy;
    double pth = oth - s * dth;
   
    CLineIterator lit( px, py, pth, m_max_range, 
                       m_world->ppm, m_world->matrix, PointToBearingRange );
      
    CEntity* ent;
    double range = m_max_range;
      
    while( (ent = lit.GetNextEntity()) ) 
    {
      // Ignore itself, its ancestors and its descendents
      if( ent == this || this->IsDescendent(ent) || ent->IsDescendent(this))
	  continue;
     
      // Ignore transparent things
      if( !ent->vision_return )
	  continue;

      // Also ignore walls
      if(!strcmp(ent->lib_entry->token,"bitmap"))
        continue;

      range = lit.GetRange(); // it's this far away

      // get the color of the entity
      memcpy( &col, &(ent->color), sizeof( StageColor ) );
	  
      break;
    }
	
    // if we found a color on this ray
    if( !(col & 0xFF000000) ) 
    {
      m_scan_color[s] = col;
      m_scan_range[s] = range;
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Generate ACTS data from scan-line image
size_t CVisionDevice::UpdateACTS( player_blobfinder_data_t* data )
{
  PRINT_DEBUG( "entered" );

  assert( data );

  // now the colors and ranges are filled in - time to do blob detection
  float yRadsPerPixel = m_zoom / cameraImageHeight;

  int blobleft = 0, blobright = 0;
  StageColor blobcol = 0;
  int blobtop = 0, blobbottom = 0;

  numBlobs = 0;
  // scan through the samples looking for color blobs
  for( int s=0; s < m_scan_width; s++ )
  {
    if( m_scan_range[s] >= 0 )
    {
      blobleft = s;
      blobcol = m_scan_color[s];

      // loop until we hit the end of the blob
      // there has to be a gap of >1 pixel to end a blob
      // this avoids getting lots of crappy little blobs
      while( m_scan_color[s] == blobcol || m_scan_color[s+1] == blobcol ) 
        s++;

      blobright = s-1;
      double robotHeight = 0.6; // meters
      int xCenterOfBlob = blobleft + ((blobright - blobleft )/2);
      double rangeToBlobCenter = m_scan_range[ xCenterOfBlob ];
      if(rangeToBlobCenter < 0)
      {
        // if the range to the "center" is < 0, then use the range
        // to the start.  this can happen, for example, when two 1-pixel
        // blobs that are 1 pixel apart are grouped into a single blob in 
        // whose "center" there is really no blob at all
        rangeToBlobCenter = m_scan_range[ blobleft ];
      }
      double startyangle = atan2( robotHeight/2.0, rangeToBlobCenter );
      double endyangle = -startyangle;
      blobtop = cameraImageHeight/2 - (int)(startyangle/yRadsPerPixel);
      blobbottom = cameraImageHeight/2 -(int)(endyangle/yRadsPerPixel);
      int yCenterOfBlob = blobtop +  ((blobbottom - blobtop )/2);

      if (blobtop < 0)
        blobtop = 0;
      if (blobbottom > cameraImageHeight - 1)
        blobbottom = cameraImageHeight - 1;

      // useful debug - keep
      /*
      printf("Robot %p sees %u blobcol l: %u r: %u t: %u b: %u range: %f\n",
             this, blobcol, blobleft, blobright, blobtop, blobbottom,rangeToBlobCenter);
             */

      // fill in an arrau entry for this blob
      //
      blobs[numBlobs].id = numBlobs;
      blobs[numBlobs].color = (uint32_t)blobcol;
      blobs[numBlobs].x = xCenterOfBlob;
      blobs[numBlobs].y = yCenterOfBlob;
      blobs[numBlobs].left = blobleft;
      blobs[numBlobs].top = blobtop;
      blobs[numBlobs].right = blobright;
      blobs[numBlobs].bottom = blobbottom;
      blobs[numBlobs].area = (blobtop - blobbottom) * (blobleft-blobright);
      blobs[numBlobs].range = (int)(rangeToBlobCenter*1000.0); 

      numBlobs++;
    }
  }

  // now pack them into the Player packet
  data->width = htons((uint16_t)this->cameraImageWidth);
  data->height = htons((uint16_t)this->cameraImageHeight);
  data->blob_count = htons(numBlobs);
  for( int b=0; b<numBlobs; b++ )
  {
    // useful debug - leave in
    /*
      cout << "blob "
      << " channel: " <<  (int)blobs[b].channel
      << " area: " <<  blobs[b].area
      << " left: " <<  blobs[b].left
      << " right: " <<  blobs[b].right
      << " top: " <<  blobs[b].top
      << " bottom: " <<  blobs[b].bottom
      << endl;
    */

    data->blobs[b].area = htonl(blobs[b].area);
    data->blobs[b].color = htonl(blobs[b].color);
    data->blobs[b].x = htons(blobs[b].x);
    data->blobs[b].y = htons(blobs[b].y);
    data->blobs[b].left = htons(blobs[b].left);
    data->blobs[b].right = htons(blobs[b].right);
    data->blobs[b].top = htons(blobs[b].top);
    data->blobs[b].bottom = htons(blobs[b].bottom);
    data->blobs[b].range = htons(blobs[b].range);
  }

  return sizeof(player_blobfinder_data_t);
}


#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CVisionDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // Create a figure representing this object
  this->vision_fig = rtk_fig_create(m_world->canvas, this->fig, 99);
  rtk_fig_origin(this->vision_fig, -0.75, +0.75, 0.0);
  rtk_fig_movemask(this->vision_fig, RTK_MOVE_TRANS);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CVisionDevice::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->vision_fig);

  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CVisionDevice::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();

  if (Subscribed() < 1)
  {
    rtk_fig_show(this->vision_fig, 0);
    return;
  }
  
  rtk_fig_clear(this->vision_fig);
  rtk_fig_show(this->vision_fig, 1);
    
  // The vision figure is attached to the entity, but we dont want
  // it to rotate.  Fix the rotation here.
  double gx, gy, gth;
  double nx, ny, nth;
  GetGlobalPose(gx, gy, gth);
  rtk_fig_get_origin(this->vision_fig, &nx, &ny, &nth);
  rtk_fig_origin(this->vision_fig, nx, ny, -gth);
  
  player_blobfinder_data_t data;

  // if a client is subscribed to this device
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->lib_entry->type_num) )
  {
    // attempt to get the right size chunk of data from the mmapped buffer
    if( GetData( &data, sizeof(data) ) == sizeof(data) )
    {  
      double scale = 0.007; // shrink from pixels to meters for display
	
      short width = ntohs(data.width);
      short height = ntohs(data.height);
      double mwidth = width * scale;
      double mheight = height * scale;
	
      // the view outline rectangle
      rtk_fig_color_rgb32(this->vision_fig, 0xFFFFFF);
      rtk_fig_rectangle(this->vision_fig, 0.0, 0.0, 0.0, mwidth,  mheight, 1 ); 
      rtk_fig_color_rgb32(this->vision_fig, 0x000000);
      rtk_fig_rectangle(this->vision_fig, 0.0, 0.0, 0.0, mwidth,  mheight, 0); 

      short numblobs = ntohs(data.blob_count);
      for(int b=0; b<numblobs; b++ )
      {
        // set the color from the blob data
        rtk_fig_color_rgb32( this->vision_fig, 
                             ntohl(data.blobs[b].color) ); 

        //short x =  ntohs(data.blobs[index+b].x);
        //short y =  ntohs(data.blobs[index+b].y);
        short top =  ntohs(data.blobs[b].top);
        short bot =  ntohs(data.blobs[b].bottom);
        short left =  ntohs(data.blobs[b].left);
        short right =  ntohs(data.blobs[b].right);

        //double mx = x * scale;
        //double my = y * scale;
        double mtop = top * scale;
        double mbot = bot * scale;
        double mleft = left * scale;
        double mright = right * scale;

        // get the range in meters
        //double range = (double)ntohs(data.blobs[index+b].range) / 1000.0; 

        rtk_fig_rectangle(this->vision_fig, 
                          -mwidth/2.0 + (mleft+mright)/2.0, 
                          -mheight/2.0 +  (mtop+mbot)/2.0,
                          0.0, 
                          mright-mleft, 
                          mbot-mtop, 
                          1 );

        //rtk_fig_line( this->vision_fig,
        //	0,0,0, 

      }
    }
    //else
    //PRINT_WARN( "no vision data available" );
  }
}

#endif


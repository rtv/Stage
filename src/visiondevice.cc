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
 * CVS info: $Id: visiondevice.cc,v 1.34 2002-07-04 01:06:02 rtv Exp $
 */

#include <math.h>

#include <iostream>

#include "world.hh"
#include "visiondevice.hh"
#include "ptzdevice.hh"
#include "raytrace.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
CVisionDevice::CVisionDevice(CWorld *world, CPtzDevice *parent)
        : CEntity( world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_vision_data_t ); 
  m_command_len = 0;
  m_config_len  = 0;
  m_reply_len  = 0;
 
  m_player.code = PLAYER_VISION_CODE;
  this->stage_type = VisionType;
  this->color = ::LookupColor(VISION_COLOR);
  
  m_interval = 0.1; // 10Hz - the real cam is around this

  // If the parent is a ptz device we will use it, otherwise
  // we will operate as a naked vision device.
  if (parent->stage_type == PtzType)
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

  // Set the default channel-color mapping for ACTS.  Let's start with
  // a reasonable set of color channels these get overwritten when
  // specifiedby the worldfile.
  this->channels[0] = ::LookupColor( "red" );
  this->channels[1] = ::LookupColor( "green" );
  this->channels[2] = ::LookupColor( "blue" );
  this->channels[3] = ::LookupColor( "yellow" );
  this->channels[4] = ::LookupColor( "cyan" );
  this->channels[5] = ::LookupColor( "magenta" );
  this->channel_count = 6;

  numBlobs = 0;
  memset( blobs, 0, MAXBLOBS * sizeof( ColorBlob ) );

#ifdef INCLUDE_RTK2
  vision_fig = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the worldfile
bool CVisionDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CEntity::Load(worldfile, section))
    return false;

  // Read the vision channel/color mapping
  for (int i = 0; true; i++)
  {
    const char *color = worldfile->ReadTupleString(section, "channels", i, NULL);
    if (!color)
      break;
    this->channels[i] = ::LookupColor(color);
    this->channel_count = i + 1;
  }
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CVisionDevice::Update( double sim_time )
{
  ASSERT(m_world != NULL);

  CEntity::Update( sim_time );

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
  player_vision_data_t data;
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
  ASSERT((size_t)m_scan_width<=sizeof(m_scan_channel)/sizeof(m_scan_channel[0]));

  // TODO
  //int skip = (int) (m_world->m_vision_res / m_scan_res - 0.5);

  // Do each scan
  // Note that the scan is taken *clockwise*
  // i'm scanning this as half-resolution for a significant speed-up

#ifdef DEBUG
  printf("1?\n");
#endif
  
  StageColor col;
  
  for (int s = 0; s < m_scan_width; s++)
  {
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
      // Ignore ourself, our ancestors and our descendents
      if( ent == this || this->IsDescendent(ent) || ent->IsDescendent(this))
        continue;
	  
      // Ignore transparent things
      if (!ent->vision_return)
        continue;
	  
      range = lit.GetRange(); // it's this far away
      //channel = ent->channel_return; // it's this color
	  
      // get the color of the entity
      memcpy( &col, &(ent->color), sizeof( StageColor ) );
	  
      break;
    }
	
    // initialize the reading 
    m_scan_channel[s] = 0; // channel 0 is no-blob
    m_scan_range[s] = 0;

    // if we found a color on this ray
    if( !(col & 0xFF000000) ) 
    {
      // look up this color in the color/channel mapping array
      for( int c=0; c < this->channel_count; c++ )
      {
        if( this->channels[c] == col)
        {
          //printf("color %d is channel %d\n", col, c);
          //printf("m_scan_channel[%d] = %d\n", s, c+1);

          m_scan_channel[s] = c + 1; // channel 0 is no-blob
          m_scan_range[s] = range;
          break;
        }
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// Generate ACTS data from scan-line image
size_t CVisionDevice::UpdateACTS( player_vision_data_t* data )
{
  assert( data );

  // now the colors and ranges are filled in - time to do blob detection
  float yRadsPerPixel = m_zoom / cameraImageHeight;

  int blobleft = 0, blobright = 0;
  unsigned char blobcol = 0;
  int blobtop = 0, blobbottom = 0;

  numBlobs = 0;
  // scan through the samples looking for color blobs
  for( int s=0; s < m_scan_width; s++ )
  {
    if( m_scan_channel[s] != 0 && m_scan_channel[s] < VISION_NUM_CHANNELS)
    {
      blobleft = s;
      blobcol = m_scan_channel[s];

      // loop until we hit the end of the blob
      // there has to be a gap of >1 pixel to end a blob
      // this avoids getting lots of crappy little blobs
      while( m_scan_channel[s] == blobcol || m_scan_channel[s+1] == blobcol ) 
        s++;

      blobright = s-1;
      double robotHeight = 0.6; // meters
      int xCenterOfBlob = blobleft + ((blobright - blobleft )/2);
      double rangeToBlobCenter = m_scan_range[ xCenterOfBlob ];
      if(!rangeToBlobCenter)
      {
        // if the range to the "center" is zero, then use the range
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
        cout << "Robot " << this
        << " sees " << (int)blobcol-1
        << " start: " << blobleft
        << " end: " << blobright
        << " top: " << blobtop
        << " bottom: " << blobbottom
        << endl << endl;
        */

      // fill in an arrau entry for this blob
      //
      blobs[numBlobs].channel = blobcol-1;
      blobs[numBlobs].x = xCenterOfBlob;
      blobs[numBlobs].y = yCenterOfBlob;
      blobs[numBlobs].left = blobleft;
      blobs[numBlobs].top = blobtop;
      blobs[numBlobs].right = blobright;
      blobs[numBlobs].bottom = blobbottom;
      blobs[numBlobs].area = (blobtop - blobbottom) * (blobleft-blobright);

      numBlobs++;
    }
  }

  // now we have the blobs we have to pack them into ACTS format (yawn...)

  // ACTS has blobs sorted by channel (color), and by area within channel, 
  // so we'll bubble sort the
  // blobs - this could be more efficient, so might fix later.
  if( numBlobs > 1 )
  {
    //cout << "Sorting " << numBlobs << " blobs." << endl;
    int change = true;
    ColorBlob tmp;

    while( change )
    {
      change = false;

      for( int b=0; b<numBlobs-1; b++ )
      {
        // if the channels are in the wrong order
        if( (blobs[b].channel > blobs[b+1].channel)
            // or they are the same channel but the areas are 
            // in the wrong order
            ||( (blobs[b].channel == blobs[b+1].channel) 
                && blobs[b].area < blobs[b+1].area ) )
        {		      
          //switch the blobs
          memcpy( &tmp, &(blobs[b]), sizeof( ColorBlob ) );
          memcpy( &(blobs[b]), &(blobs[b+1]), sizeof( ColorBlob ) );
          memcpy( &(blobs[b+1]), &tmp, sizeof( ColorBlob ) );

          change = true;
        }
      }
    }
  }

  // now run through the blobs, packing them into the ACTS buffer
  // counting the number of blobs in each channel and making entries
  // in the acts header

  for( int b=0; b<numBlobs; b++ )
  {
    // I'm not sure the ACTS-area is really just the area of the
    // bounding box, or if it is in fact the pixel count of the
    // actual blob. Here it's just the rectangular area.

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

    // RTV - blobs[b].area is already set above - just byteswap
    data->blobs[b].area = htonl(blobs[b].area);

    // look up the color for this channel
    data->blobs[b].color = htonl( this->channels[ blobs[b].channel ] );

    data->blobs[b].x = htons(blobs[b].x);
    data->blobs[b].y = htons(blobs[b].y);
    data->blobs[b].left = htons(blobs[b].left);
    data->blobs[b].right = htons(blobs[b].right);
    data->blobs[b].top = htons(blobs[b].top);
    data->blobs[b].bottom = htons(blobs[b].bottom);

    // increment the count for this channel
    data->header[blobs[b].channel].num++;
  }


  // now we finish the header by setting the blob indexes and byte
  // swapping the counts.
  int pos = 0;
  for( int ch=0; ch<VISION_NUM_CHANNELS; ch++ )
  {
    data->header[ch].index = htons(pos);
    pos += data->header[ch].num;

    // byte swap the blob count for each channel
    PRINT_DEBUG2( "channel: %d blobs: %d\n", ch,  data->header[ch].num ); 
    data->header[ch].num = htons(data->header[ch].num);
  }

  // and set the image width * height
  data->width = htons((uint16_t)cameraImageWidth);
  data->height = htons((uint16_t)cameraImageHeight);

  return sizeof(player_vision_data_t);
}


#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CVisionDevice::RtkStartup()
{
  CEntity::RtkStartup();
  
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

  CEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CVisionDevice::RtkUpdate()
{
  CEntity::RtkUpdate();

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
  
  player_vision_data_t data;

  // if a client is subscribed to this device
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->stage_type) )
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

      for( int c=0; c<VISION_NUM_CHANNELS;c++)
      {
        short numblobs = ntohs(data.header[c].num);
        short index = ntohs(data.header[c].index);	    
	    
        for( int b=0; b<numblobs; b++ )
	      {
          // set the color from the blob data
          rtk_fig_color_rgb32( this->vision_fig, 
                               ntohl(data.blobs[index+b].color) ); 

          //short x =  ntohs(data.blobs[index+b].x);
          //short y =  ntohs(data.blobs[index+b].y);
          short top =  ntohs(data.blobs[index+b].top);
          short bot =  ntohs(data.blobs[index+b].bottom);
          short left =  ntohs(data.blobs[index+b].left);
          short right =  ntohs(data.blobs[index+b].right);
		
          //double mx = x * scale;
          //double my = y * scale;
          double mtop = top * scale;
          double mbot = bot * scale;
          double mleft = left * scale;
          double mright = right * scale;
	    
          rtk_fig_rectangle(this->vision_fig, 
                            -mwidth/2.0 + (mleft+mright)/2.0, 
                            -mheight/2.0 +  (mtop+mbot)/2.0,
                            0.0, 
                            mright-mleft, 
                            mbot-mtop, 
                            1 );
	      }
      }
    }
    else
      PRINT_WARN( "no vision data available" );
  }
}

#endif


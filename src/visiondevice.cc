///////////////////////////////////////////////////////////////////////////
//
// File: visiondevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the ACTS vision system
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/visiondevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.21.2.1 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#include <math.h>
//#include <iostream.h>
#include "world.hh"
#include "visiondevice.hh"
#include "ptzdevice.hh"
#include "raytrace.hh"

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CVisionDevice::CVisionDevice(CWorld *world, CPtzDevice *parent)
        : CEntity( world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_vision_data_t ); 
  m_command_len = 0;
  m_config_len  = 0;
  m_reply_len  = 0;
 
  m_player_type = PLAYER_VISION_CODE;
  m_stage_type = VisionType;

  m_interval = 0.1; // 10Hz - the real cam is around this

  // ACTS must be associated with a physical camera
  // so parent must be a PTZ device
  // this check isn;t bulletproof, but it's better than nothin'.
  // But why not allow a naked vision device? AH
  //ASSERT( parent != NULL);
  //ASSERT( parent->m_player_type == PLAYER_PTZ_CODE );

  if (parent->m_stage_type == PtzType)
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

  // Set the default channel-color mapping for ACTS
  memset( channel, 0, sizeof(StageColor) * ACTS_NUM_CHANNELS );
  channel[0].red = 255;
  channel[1].green = 255;
  channel[2].blue = 255;

  numBlobs = 0;
  memset( blobs, 0, MAXBLOBS * sizeof( ColorBlob ) );

#ifdef INCLUDE_RTK
  m_hit_count = 0;
#endif

}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the worldfile
bool CVisionDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CEntity::Load(worldfile, section))
    return false;

  // Read the vision channel/color mapping from global settings
  for (int i = 0; true; i++)
  {
    const char *color = worldfile->ReadTupleString(0, "vision_channels", i, NULL);
    if (!color)
      break;
    m_world->ColorFromString(this->channel + i, color);
  }

  // Read the vision channel/color mapping from local settings
  for (int i = 0; true; i++)
  {
    const char *color = worldfile->ReadTupleString(section, "channels", i, NULL);
    if (!color)
      break;
    m_world->ColorFromString(this->channel + i, color);
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
  //
  if( Subscribed() < 1 )
    return;
  
  // See if its time to recalculate vision
  //
  if( sim_time - m_last_update < m_interval )
    return;
  
  m_last_update = sim_time;
  
  //RTK_TRACE0("generating new data");
  
  // Generate the scan-line image
  //
  UpdateScan();
  
  // Generate ACTS data
  //
  size_t len = UpdateACTS();

  // Copy data to the output buffer
  // no need to byteswap - this is single-byte data
  //
  if (len > 0)
    PutData((void*)(&actsBuf), len);
}


///////////////////////////////////////////////////////////////////////////
// Generate the scan-line image
//
void CVisionDevice::UpdateScan()
{
  // Get the camera's global pose
  //
  double ox, oy, oth;
  GetGlobalPose(ox, oy, oth);

  // Get the ptz settings
  //
  if (m_ptz_device != NULL)
    m_ptz_device->GetPTZ(m_pan, m_tilt, m_zoom);

  // Compute starting angle
  //
  oth = oth + m_pan + m_zoom / 2;

  // Compute fov, range, etc
  //
  double dth = m_zoom / m_scan_width;

  // Initialise gui data
  //
#ifdef INCLUDE_RTK
  m_hit_count = 0;
#endif

  // Make sure the data buffer is big enough
  //
  ASSERT((size_t)m_scan_width<=sizeof(m_scan_channel)/sizeof(m_scan_channel[0]));

  // TODO
  //int skip = (int) (m_world->m_vision_res / m_scan_res - 0.5);

  // Do each scan
  // Note that the scan is taken *clockwise*
  //

  // i'm scanning this as half-resolution for a significant speed-up

  StageColor col;

  for (int s = 0; s < m_scan_width; s++)
  {
    bzero(&col,sizeof(col));
      
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
	
    //printf( "ray: %d channel: %d\n", s, channel );
    //fflush( stdout );

    // initialize the reading 
    m_scan_channel[s] = 0; // channel 0 is no-blob
    m_scan_range[s] = 0;
	
    // look up this color in the color/channel mapping array
    for( int c=0; c<ACTS_NUM_CHANNELS; c++ )
    {
      if( channel[c].red == col.red &&
          channel[c].green == col.green &&
          channel[c].blue == col.blue  )
      {
        //printf("m_scan_channel[%d] = %d\n", s, c+1);
        m_scan_channel[s] = c + 1; // channel 0 is no-blob
        m_scan_range[s] = range;
        break;
      }
    }
	
    // Update the gui data
    //
#ifdef INCLUDE_RTK
    if (m_scan_channel[s] > 0)
    {
      assert((unsigned) m_hit_count < sizeof(m_hit) / sizeof(m_hit[0]));
      m_hit[m_hit_count][0] = px + range * cos(pth);
      m_hit[m_hit_count][1] = py + range * sin(pth);
      m_hit[m_hit_count][2] = m_scan_channel[s];            
      m_hit[m_hit_count][3] = col.red;
      m_hit[m_hit_count][4] = col.green;
      m_hit[m_hit_count][5] = col.blue;
      m_hit_count++;
    }
#endif
  }   
}


///////////////////////////////////////////////////////////////////////////
// Generate ACTS data from scan-line image
//
size_t CVisionDevice::UpdateACTS()
{
  // now the colors and ranges are filled in - time to do blob detection
  float yRadsPerPixel = m_zoom / cameraImageHeight;

  int blobleft = 0, blobright = 0;
  unsigned char blobcol = 0;
  int blobtop = 0, blobbottom = 0;

  numBlobs = 0;
  // scan through the samples looking for color blobs
  for( int s=0; s < m_scan_width; s++ )
  {
    if( m_scan_channel[s] != 0 && m_scan_channel[s] < ACTS_NUM_CHANNELS)
    {
      blobleft = s;
      blobcol = m_scan_channel[s];

      // loop until we hit the end of the blob
      // there has to be a gap of >1 pixel to end a blob
      // this avoids getting lots of crappy little blobs
      while( m_scan_channel[s] == blobcol || m_scan_channel[s+1] == blobcol ) s++;
      //while( m_scan[s] == blobcol ) s++;

      blobright = s-1;
      double robotHeight = 0.6; // meters
      int xCenterOfBlob = blobleft + ((blobright - blobleft )/2);
      double rangeToBlobCenter = m_scan_range[ xCenterOfBlob ];
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
      //cout << "Robot " << this
      //<< " sees " << (int)blobcol-1
      //<< " start: " << blobleft
      //<< " end: " << blobright
      //<< endl << endl;

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

  int buflen = VISION_HEADER_SIZE + numBlobs * VISION_BLOB_SIZE;
  //bzero(&actsBuf, buflen );
  bzero(&actsBuf, sizeof(actsBuf));

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

    // RTV - blobs[b].area is already set above
    actsBuf.blobs[b].area = htonl(blobs[b].area);

    // useful debug
    /*
    cout << "blob "
    << " area: " <<  blobs[b].area
    << " left: " <<  blobs[b].left
    << " right: " <<  blobs[b].right
    << " top: " <<  blobs[b].top
    << " bottom: " <<  blobs[b].bottom
    << endl;
    */

    actsBuf.blobs[b].x = htons(blobs[b].x);
    actsBuf.blobs[b].y = htons(blobs[b].y);
    actsBuf.blobs[b].left = htons(blobs[b].left);
    actsBuf.blobs[b].right = htons(blobs[b].right);
    actsBuf.blobs[b].top = htons(blobs[b].top);
    actsBuf.blobs[b].bottom = htons(blobs[b].bottom);


    // increment the count for this channel (and byte-swap)
    actsBuf.header[blobs[b].channel].num = 
            htons(ntohs(actsBuf.header[blobs[b].channel].num)+1);
  }

  // now we finish the header by setting the blob indexes.
  int pos = 0;
  for( int ch=0; ch<VISION_NUM_CHANNELS; ch++ )
  {
    actsBuf.header[ch].index = htons(pos);
    pos += actsBuf.header[ch].num;
  }

  // finally, we're done.
  //cout << endl;

  //RTK_TRACE1("Found %d blobs", (int) numBlobs);

  return buflen;
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CVisionDevice::OnUiUpdate(RtkUiDrawData *data)
{
    // Draw our children
    //
    CEntity::OnUiUpdate(data);
    
    // Draw ourself
    //
    data->begin_section("global", "vision");

    int sub = Subscribed();
    if (data->draw_layer("fov", true))
    {
      if(sub)
        DrawFOV(data);
    }
    
    if (data->draw_layer("scan", true))
    {
      if(sub)
        DrawScan(data);
    }
    if(sub)
      Update(m_world->GetTime());
    
    data->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CVisionDevice::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// Draw the field of view
//
void CVisionDevice::DrawFOV(RtkUiDrawData *data)
{
    #define COLOR_FOV RGB(0, 192, 0)
    
    // Get global pose
    //
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);
    
    double ax = gx + m_max_range * cos(gth + m_pan - m_zoom / 2);
    double ay = gy + m_max_range * sin(gth + m_pan - m_zoom / 2);

    double bx = gx + m_max_range * cos(gth + m_pan + m_zoom / 2);
    double by = gy + m_max_range * sin(gth + m_pan + m_zoom / 2);

    data->set_color(COLOR_FOV);
    data->line(gx, gy, ax, ay);
    data->line(ax, ay, bx, by);
    data->line(bx, by, gx, gy);
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser scan
//
void CVisionDevice::DrawScan(RtkUiDrawData *data)
{    
    // Get global pose
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);

    // Draw colored rays to show objects
    double ch = -1000;
    for (int i = 0; i < m_hit_count; i++)
    {
        if (m_hit[i][2] != ch)
            data->set_color(RTK_RGB(m_hit[i][3], m_hit[i][4], m_hit[i][5]));
        else
            data->line(gx, gy, m_hit[i][0], m_hit[i][1]);
        ch = m_hit[i][2];
    }
}

#endif




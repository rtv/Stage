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
//  $Revision: 1.12 $
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
#include "world.hh"
#include "visiondevice.hh"
#include "ptzdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CVisionDevice::CVisionDevice(CWorld *world, CPtzDevice *parent)
        : CEntity( world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_vision_data_t ); 
  m_command_len = 0;//sizeof( player_vision_cmd_t );
  m_config_len  = 0;//sizeof( player_vision_config_t );
 
  m_player_type = PLAYER_VISION_CODE;
  
  m_stage_type = VisionType;

  m_size_x = 0.9 * parent->m_size_x;
  m_size_y = 0.9 * parent->m_size_y;

  m_interval = 0.1; // 10Hz

  // ACTS must be associated with a physical camera
  // so parent must be a PTZ device
  // this check isn;t bulletproof, but it's better than nothin'.
  ASSERT( parent != NULL);
  ASSERT( parent->m_player_type == PLAYER_PTZ_CODE );
  
  m_ptz_device = parent;
  
  cameraImageWidth = 160;
  cameraImageHeight = 120;
  
  m_scan_width = 160;
  
  m_pan = 0;
  m_tilt = 0;
  m_zoom = DTOR(60);
  
  m_max_range = 8.0;
  
  numBlobs = 0;
  memset( blobs, 0, MAXBLOBS * sizeof( ColorBlob ) );
  

    cameraImageWidth = 160;
    cameraImageHeight = 120;
    
    m_scan_width = 160;
    
    m_pan = 0;
    m_tilt = 0;
    m_zoom = DTOR(60);

    m_max_range = 8.0;

    numBlobs = 0;
    memset( blobs, 0, MAXBLOBS * sizeof( ColorBlob ) );

#ifdef INCLUDE_RTK
  m_hit_count = 0;
#endif

}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CVisionDevice::Update( double sim_time )
{
  CEntity::Update( sim_time ); // inherit debug output

    //RTK_TRACE0("updating vision data");
    
    // Dont update anything if we are not subscribed
    //
  if(!Subscribed())
    return;
  
  ASSERT(m_world != NULL);

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
        PutData(actsBuf, len);
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
    double dr = 1.0 / m_world->ppm;

    // Ignore obstacles closer than this range
    // (so we dont see ourself as a obstacle)
    //
    double min_range = 0.20;

    // Initialise gui data
    //
#ifdef INCLUDE_RTK
    m_hit_count = 0;
#endif

    // Make sure the data buffer is big enough
    //
    ASSERT((size_t) m_scan_width <= sizeof(m_scan_channel) / sizeof(m_scan_channel[0]));

    // Do each scan
    // Note that the scan is taken *clockwise*
    //
    for (int s = 0; s < m_scan_width; s++)
    {
        // Compute parameters of scan line
        //
        double px = ox;
        double py = oy;
        double pth = oth - s * dth;

        // Compute the step for simple ray-tracing
        //
        double dx = dr * cos(pth);
        double dy = dr * sin(pth);

        double range;
        int channel = 0;
        
        // Look along scan line for beacons
        // Could make this an int again for a slight speed-up.
        //
        for (range = 0; range < m_max_range; range += dr)
        {            
            // Look in the vision layer for beacons.
            // Also look at the two cells to the right and above
            // so we dont sneak through gaps.
            //
            uint8_t cell = 0;
            cell = m_world->GetCell(px, py, layer_vision);
            if (cell == 0)
                cell = m_world->GetCell(px + dr, py, layer_vision);
            if (cell == 0)
                cell = m_world->GetCell(px, py + dr, layer_vision);

            if (cell != 0)
            {
                channel = cell;
                break;
            }
            
            // Look in the laser layer for obstacles.
            // Also look at the two cells to the right and above
            // so we dont sneak through gaps.
            //
            if (range > min_range)
            {
                if (m_world->GetCell(px, py, layer_laser) > 0)
                    break;
                if (m_world->GetCell(px + dr, py, layer_laser) > 0)
                    break;
                if (m_world->GetCell(px, py + dr, layer_laser) > 0)
                    break;
            }
            
            px += dx;
            py += dy;
        }

        // Set the channel
        //
        m_scan_channel[s] = channel;
        m_scan_range[s] = range;
        
        // Update the gui data
        //
#ifdef INCLUDE_RTK
        if (channel > 0)
        {
            m_hit[m_hit_count][0] = px;
            m_hit[m_hit_count][1] = py;
            m_hit[m_hit_count][2] = channel;
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
	    // << " sees " << (int)blobcol-1
	    // << " start: " << blobleft
	    // << " end: " << blobright
	    // << endl << endl;
	         
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
     
    int buflen = ACTS_HEADER_SIZE + numBlobs * ACTS_BLOB_SIZE;
    memset( actsBuf, 0, buflen );
            
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
      
    int index = ACTS_HEADER_SIZE;
      
    for( int b=0; b<numBlobs; b++ )
	{
        // I'm not sure the ACTS-area is really just the area of the
        // bounding box, or if it is in fact the pixel count of the
        // actual blob. Here it's just the rectangular area.
	  
        // RTV - blobs[b].area is already set above
        int blob_area = blobs[b].area;
	  
	// encode the area in ACTS's 3 bytes, 6-bits-per-byte
        for (int tt=3; tt>=0; --tt) 
	    {  
            actsBuf[ index + tt] = (blob_area & 63);
            blob_area = blob_area >> 6;
            //cout << "byte:" << (int)(actsBuf[ index + tt]) << endl;
	    }
	  
        // useful debug
        //  cout << "blob "
        //    << " area: " <<  blobs[b].area
        //    << " left: " <<  blobs[b].left
        //    << " right: " <<  blobs[b].right
        //    << " top: " <<  blobs[b].top
        //    << " bottom: " <<  blobs[b].bottom
        //    << endl;
	  
        actsBuf[ index + 4 ] = blobs[b].x;
        actsBuf[ index + 5 ] = blobs[b].y;
        actsBuf[ index + 6 ] = blobs[b].left;
        actsBuf[ index + 7 ] = blobs[b].right;
        actsBuf[ index + 8 ] = blobs[b].top;
        actsBuf[ index + 9 ] = blobs[b].bottom;
	  
        index += ACTS_BLOB_SIZE;
	  
        // increment the count for this channel
        actsBuf[ blobs[b].channel * 2 +1 ]++;
	}
      
    // now we finish the header by setting the blob indexes.
    int pos = 0;
    for( int ch=0; ch<ACTS_NUM_CHANNELS; ch++ )
	{
        actsBuf[ ch*2 ] = pos;
        pos += actsBuf[ ch*2 +1 ];
	}
      
    // add 1 to every byte in the acts buffer.
    // no overflow should be possible, so don't check
    //cout << "ActsBuf: ";
    for( int c=0; c<buflen; c++ )
	{
	  //cout << (int)actsBuf[c] << ' ';  
	  actsBuf[c]++;
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
    //
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);

    // *** Can we get colors from channels?
    //
    int color = RTK_RGB(0, 192, 0);
    data->set_color(color);

    double ox, oy;
    
    for (int i = 0; i < m_hit_count; i++)
    {
        int channel = (int) m_hit[i][2];
        if (channel == 0)
            continue;

        if (i > 0)
            data->line(ox, oy, m_hit[i][0], m_hit[i][1]);
        ox = m_hit[i][0];
        oy = m_hit[i][1];
    }
}

#endif



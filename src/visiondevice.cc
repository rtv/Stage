///////////////////////////////////////////////////////////////////////////
//
// File: visiondevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the ACTS vision system
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/visiondevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.3 $
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

#define ENABLE_TRACE 0

#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations
#include "world.h"
#include "robot.h"
#include "visiondevice.hh"
#include "ptzdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CVisionDevice::CVisionDevice(CRobot *robot, CPtzDevice *ptz_device,
                             void *buffer, size_t data_len,
                             size_t command_len, size_t config_len)
        : CPlayerDevice(robot, buffer, data_len, command_len, config_len)
{
    m_ptz_device = ptz_device;
        
    m_update_interval = 0.1;
    m_last_update = 0;

    cameraImageWidth = 160 / 2;
    cameraImageHeight = 120 / 2;

    numBlobs = 0;
    memset( blobs, 0, MAXBLOBS * sizeof( ColorBlob ) );
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
bool CVisionDevice::Update()
{
    //TRACE0("updating vision data");
    
    // Dont update anything if we are not subscribed
    //
    if (!IsSubscribed())
        return false;
    //TRACE0("device has been subscribed");
    
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);
     
    // Get pointers to the various bitmaps
    //
    Nimage *img = m_world->img;
    Nimage *laser_img = m_world->m_laser_img;

    ASSERT(img != NULL);
    ASSERT(laser_img != NULL);

    // See if its time to recalculate vision
    //
    if( m_world->timeNow - m_last_update < m_update_interval )
        return false;
    m_last_update = m_world->timeNow;
    TRACE0("generating new data");
    
    unsigned char* colors = new unsigned char[ cameraImageWidth ];
    float* ranges = new float[ cameraImageWidth ];

    // Get the ptz settings
    //
    double pan, tilt, zoom;
    ASSERT(m_ptz_device != NULL);
    m_ptz_device->GetPTZ(pan, tilt, zoom);
    
    // ray trace the 1d color / range image
    float startAngle = (m_robot->a + pan) - (zoom / 2.0);
    float xRadsPerPixel = zoom / cameraImageWidth;
    float yRadsPerPixel = zoom / cameraImageHeight;
    unsigned char pixel = 0;

    float xx, yy;

    int maxRange = (int)(8.0 * m_world->ppm); // 8m times pixels-per-meter
    int rng = 0;

    // do the ray trace for each pixel across the 1D image
    for( int s=0; s < cameraImageWidth; s++)
	{
        float dist, dx, dy, angle;
	  
        angle = startAngle + ( s * xRadsPerPixel );
	  
        dx = cos( angle );
        dy = sin( angle );
	  
        xx = m_robot->x; // assume for now that the camera is at the middle
        yy = m_robot->y; // of the robot - easy to change this
        rng = 0;
	  
        pixel = img->get_pixel( (int)xx, (int)yy );
	  
        // no need for bounds checking - get_pixel() does that internally
        // i'll put a bound of 1000 ion the ray length for sanity
        while( rng < 1000 && ( pixel == 0 || pixel == m_robot->color ) )
	    {
            xx+=dx;
            yy+=dy;
            rng++;
	      
            pixel = img->get_pixel( (int)xx, (int)yy );
	    }
	  
        // set color value
        colors[s] = pixel;
        // set range value, scaled to current ppm
        ranges[s] = rng / m_world->ppm;
	}
      
    // now the colors and ranges are filled in - time to do blob detection
      
    int blobleft = 0, blobright = 0;
    unsigned char blobcol = 0;
    int blobtop = 0, blobbottom = 0;
      
    numBlobs = 0;
      
    // scan through the samples looking for color blobs
    for( int s=0; s < cameraImageWidth; s++ )
	{
        if( colors[s] != 0 && colors[s] < ACTS_NUM_CHANNELS && colors[s] != m_robot->color )
	    {
            blobleft = s;
            blobcol = colors[s];
	      
            // loop until we hit the end of the blob
            // there has to be a gap of >1 pixel to end a blob
            //while( colors[s] == blobcol || colors[s+1] == blobcol ) s++;
            while( colors[s] == blobcol ) s++;
	      
            blobright = s-1;
            double robotHeight = 0.6; // meters
            int xCenterOfBlob = blobleft + ((blobright - blobleft )/2);
            double rangeToBlobCenter = ranges[ xCenterOfBlob ];
            double startyangle = atan2( robotHeight/2.0, rangeToBlobCenter );
            double endyangle = -startyangle;
            blobtop = cameraImageHeight/2 - (int)(startyangle/yRadsPerPixel);
            blobbottom = cameraImageHeight/2 -(int)(endyangle/yRadsPerPixel);
            int yCenterOfBlob = blobtop +  ((blobbottom - blobtop )/2);
	      
            //cout << "Robot " << (int)color-1
            //   << " sees " << (int)blobcol-1
            //   << " start: " << blobleft
            //   << " end: " << blobright
            //   << endl << endl;
	      
	      
            // fill in an arrau entry for this blob
            //
            blobs[numBlobs].channel = blobcol-1;
            blobs[numBlobs].x = xCenterOfBlob * 2;
            blobs[numBlobs].y = yCenterOfBlob * 2;
            blobs[numBlobs].left = blobleft * 2;
            blobs[numBlobs].top = blobtop * 2;
            blobs[numBlobs].right = blobright * 2;
            blobs[numBlobs].bottom = blobbottom * 2;
            blobs[numBlobs].area = (blobtop - blobbottom) * (blobleft-blobright) * 4;
	      
            // set the blob color - look up the robot with this
            // bitmap color and translate that to a visible channel no.
	      
            for( CRobot* r = m_world->bots; r; r = r->next )
            {
                if( r->color == blobcol )
                {
                    //cout << (int)(r->color) << " "
                    // << (int)(r->channel) << endl;
                    blobs[numBlobs].channel = r->channel;
                    break;
                }
            }
	      
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
		      
                    /*    cout << "switching " << b
                          << "(channel " << (int)(blobs[b].channel)
                          << ") and " << b+1 << "(channel "
                          << (int)(blobs[b+1].channel) << endl;
                    */
		      
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
        // actual blob. Here it's not!
	  
        // RTV - blobs[b].area is already set above

        //blobs[b].area = ((int)blobs[b].right - blobs[b].left) *
        //(blobs[b].bottom - blobs[b].top);
	  
        int blob_area = blobs[b].area;
	  
        for (int tt=3; tt>=0; --tt) 
	    {  
            actsBuf[ index + tt] = (blob_area & 63);
            blob_area = blob_area >> 6;
            //cout << "byte:" << (int)(actsBuf[ index + tt]) << endl;
	    }
	  
        // useful debug
        /*  cout << "blob "
            << " area: " <<  blobs[b].area
            << " left: " <<  blobs[b].left
            << " right: " <<  blobs[b].right
            << " top: " <<  blobs[b].top
            << " bottom: " <<  blobs[b].bottom
            << endl;
        */
	  
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

    TRACE1("Found %d blobs", (int) numBlobs);
    
    // Copy data to the output buffer
    // no need to byteswap - this is single-byte data
    //
    PutData(actsBuf, buflen);
    
    return true;
}









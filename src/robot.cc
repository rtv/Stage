/*************************************************************************
 * robot.cc - most of the action is here
 * RTV
 * $Id: robot.cc,v 1.7 2000-12-01 00:20:52 vaughan Exp $
 ************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <iomanip.h>
#include <iostream.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strstream.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip.h>
#include <sys/mman.h>

// BPG
#include <signal.h>
#include <sys/wait.h>
// GPB

#include <offsets.h> // from Player's include directory

#include "world.h"
#include "win.h"
#include "ports.h"

// Devices
//
#include "laserdevice.hh"
#include "pioneermobiledevice.h"

extern int errno;

//extern CWorld* world;
extern CWorldWin* win;

unsigned char f = 0xFF;

const int numPts = SONARSAMPLES;


CRobot::CRobot( CWorld* ww, int col, 
		float w, float l,
		float startx, float starty, float starta )
{
  world = ww;

  //id = iid;
  color = col;
  next  = NULL;

  xorigin = oldx = x = startx * world->ppm;
  yorigin = oldy = y = starty * world->ppm;
  aorigin = olda = a = starta;

  // this stuff will get packed into devices soon
  // from here...
  channel = 0; // vision system color channel - default 0

  redrawSonar = false;
  redrawLaser = false;
  leaveTrail = false;

  lastSonar = lastVision = 0.0;

  cameraAngleMax = 2.0 * M_PI / 3.0; // 120 degrees
  cameraAngleMin = 0.2 * M_PI / 3.0; // 12 degrees

  cameraFOV = cameraAngleMax;

  cameraImageWidth = 160 / 2;
  cameraImageHeight = 120 / 2;

  cameraPan = 0.0;
  cameraPanMax = 100.0; // degrees
  cameraPanMin = -cameraPanMax;

  numBlobs = 0;
  memset( blobs, 0, MAXBLOBS * sizeof( ColorBlob ) );

  // ...to here

  // -- create the memory map for IPC with Player --------------------------

// amount of memory to reserve per robot for Player IO
  size_t areaSize = TOTAL_SHARED_MEMORY_BUFFER_SIZE;

  // make a unique temporary file for shared mem
  strcpy( tmpName, "playerIO.XXXXXX" );
  int tfd = mkstemp( tmpName );

  // make the file the right size
  if( ftruncate( tfd, areaSize ) < 0 )
    {
      perror( "Failed to set file size" );
      exit( -1 );
    }

  if( (playerIO = (caddr_t)mmap( NULL, areaSize,
    PROT_READ | PROT_WRITE,
    MAP_SHARED, tfd, (off_t)0 ))
      == MAP_FAILED )
    {
      perror( "Failed to map memory" );
      exit( -1 );
    }

  close( tfd ); // can close fd once mapped

  //cout << "Mapped area: " << (unsigned int)playerIO << endl;

  // test the memory space
  for( int t=0; t<64; t++ ) playerIO[t] = (unsigned char)t;

  // ----------------------------------------------------------------------
  // fork off a player process to handle robot I/O
  if( (player_pid = fork()) < 0 )
    {
      cerr << "fork error creating robots" << flush;
      exit( -1 );
    }
  else
  {
    // BPG
    //if( pid == 0 ) // new child process
    // GPB
    if( player_pid == 0 ) // new child process
      {
 // create player port number for command line
 char portBuf[32];
 sprintf( portBuf, "%d", PLAYER_BASE_PORT + col -1 );

        // BPG
        // release controlling tty so Player doesn't get signals
        setpgrp();
        // GPB

 // we assume Player is in the current path
 if( execlp( "player", "player",
      "-gp", portBuf, "-stage", tmpName, (char*) 0) < 0 )
   {
     cerr << "execlp failed: make sure Player can be found"
       " in the current path."
   << endl;
     exit( -1 );
   }
      }
    //else
    //printf("forked player with pid: %d\n", player_pid);
  }

  // Initialise the device list
  //
  m_device_count = 0;
  
  // *** TESTING -- should be moved
  //
  Startup();
}

CRobot::~CRobot( void )
{
  // *** TESTING -- should be moved
  //
  Shutdown();

  // BPG
  if(kill(player_pid,SIGINT))
    perror("CRobot::~CRobot(): kill() failed sending SIGINT to Player");
  if(waitpid(player_pid,NULL,0) == -1)
    perror("CRobot::~CRobot(): waitpid() returned an error");
  // GPB

  // delete the playerIO.xxxxxx file
  remove( tmpName );
}

// Start all the devices
//
bool CRobot::Startup()
{
  //TRACE0("starting devices");
  
  m_device_count = 0;
  
  // Create laser device
  //
  m_device[m_device_count++] 
    = new CPioneerMobileDevice( this, 
				world->pioneerWidth, 
				world->pioneerLength,
				playerIO + P2OS_DATA_START,
				P2OS_DATA_BUFFER_SIZE,
				P2OS_COMMAND_BUFFER_SIZE,
				P2OS_CONFIG_BUFFER_SIZE);
  
  
  m_device[m_device_count++] = new CLaserDevice( this,
						 playerIO + LASER_DATA_START,
                                                  LASER_DATA_BUFFER_SIZE,
						 LASER_COMMAND_BUFFER_SIZE,
						 LASER_CONFIG_BUFFER_SIZE);
  

  // Start all the devices
  //
  for (int i = 0; i < m_device_count; i++)
    {
      if (!m_device[i]->Startup() )
	{
	  perror("CRobot::Startup: failed to open device; device unavailable");
	  m_device[i] = NULL;
	}
    }
  
  return true;
}


// Shutdown the devices
//
bool CRobot::Shutdown()
{
  //TRACE0("shutting down devices");
    
    for (int i = 0; i < m_device_count; i++)
    {
        if (!m_device[i])
            continue;
        if (!m_device[i]->Shutdown())
            perror("CRobot::Shutdown: failed to close device");
        delete m_device[i];
    }
    return true;
}


void CRobot::PublishSonar( void )
{
  world->LockShmem();

  unsigned short ss;

  for( int c=0; c<SONARSAMPLES; c++ )
    {
      ss =  (unsigned short)( sonar[c] * 1000.0 );
      //playerIO[ c*2 + SONAR_DATA_START ]
      //= htons((unsigned short) (sonar[c] * 1000.0) );

      *((unsigned short*)(playerIO + c*2 + SONAR_DATA_START)) = htons(ss);
    }

  world->UnlockShmem();
}


void CRobot::PublishVision( void )
{
  unsigned short blobDataSize = ACTS_HEADER_SIZE + numBlobs *
ACTS_BLOB_SIZE;

  world->LockShmem(); // lock buffer

  // no need to byteswap - this is single-byte data
  memcpy( 2 + playerIO + ACTS_DATA_START, actsBuf, blobDataSize );

  // set the first 2 bytes to indicate the buffer size
  *((unsigned short*)(playerIO + ACTS_DATA_START)) = blobDataSize;

  world->UnlockShmem();
}


int CRobot::UpdateSonar( Nimage* img  )
{
  if( world->timeNow - lastSonar > world->sonarInterval )
    {
      lastSonar = world->timeNow;

      float ppm = world->ppm;

  float tangle; // is the angle from the robot's nose to the transducer itself
  // and angle is the direction in which the transducer points

  // trace the new sonar beams to discover their ranges
  for( int s=0; s < SONARSAMPLES; s++ )
    {
      float dist, dx, dy, angle;
      float xx = x;
      float yy = y;

      switch( s )
 {
#ifdef PIONEER1
 case 0: angle = a - 1.57; break; //-90 deg
 case 1: angle = a - 0.52; break; // -30 deg
 case 2: angle = a - 0.26; break; // -15 deg
 case 3: angle = a       ; break;
 case 4: angle = a + 0.26; break; // 15 deg
 case 5: angle = a + 0.52; break; // 30 deg
 case 6: angle = a + 1.57; break; // 90 deg
#else
 case 0:
   angle = a - 1.57;
   tangle = a - 0.900;
   xx += 0.172 * cos( tangle ) * ppm;
   yy += 0.172 * sin( tangle ) * ppm;
   break; //-90 deg
 case 1: angle = a - 0.87;
   tangle = a - 0.652;
   xx += 0.196 * cos( tangle ) * ppm;
   yy += 0.196 * sin( tangle ) * ppm;
   break; // -50 deg
 case 2: angle = a - 0.52;
   tangle = a - 0.385;
   xx += 0.208 * cos( tangle ) * ppm;
   yy += 0.208 * sin( tangle ) * ppm;
   break; // -30 deg
 case 3: angle = a - 0.17;
   tangle = a - 0.137;
   xx += 0.214 * cos( tangle ) * ppm;
   yy += 0.214 * sin( tangle ) * ppm;
   break; // -10 deg
 case 4: angle = a + 0.17;
   tangle = a + 0.137;
   xx += 0.214 * cos( tangle ) * ppm;
   yy += 0.214 * sin( tangle ) * ppm;
   break; // 10 deg
 case 5: angle = a + 0.52;
   tangle = a + 0.385;
   xx += 0.208 * cos( tangle ) * ppm;
   yy += 0.208 * sin( tangle ) * ppm;
   break; // 30 deg
 case 6: angle = a + 0.87;
   tangle = a + 0.652;
   xx += 0.196 * cos( tangle ) * ppm;
   yy += 0.196 * sin( tangle ) * ppm;
   break; // 50 deg
 case 7: angle = a + 1.57;
   tangle = a + 0.900;
   xx += 0.172 * cos( tangle ) * ppm;
   yy += 0.172 * sin( tangle ) * ppm;
   break; // 90 deg
 case 8: angle = a + 1.57;
   tangle = a + 2.240;
   xx += 0.172 * cos( tangle ) * ppm;
   yy += 0.172 * sin( tangle ) * ppm;
   break; // 90 deg
 case 9: angle = a + 2.27;
   tangle = a + 2.488;
   xx += 0.196 * cos( tangle ) * ppm;
   yy += 0.196 * sin( tangle ) * ppm;
   break; // 130 deg
 case 10: angle = a + 2.62;
   tangle = a + 2.755;
   xx += 0.208 * cos( tangle ) * ppm;
   yy += 0.208 * sin( tangle ) * ppm;
   break; // 150 deg
 case 11: angle = a + 2.97;
   tangle = a + 3.005;
   xx += 0.214 * cos( tangle ) * ppm;
   yy += 0.214 * sin( tangle ) * ppm;
   break; // 170 deg
 case 12: angle = a - 2.97;
   tangle = a - 3.005;
   xx += 0.214 * cos( tangle ) * ppm;
   yy += 0.214 * sin( tangle ) * ppm;
   break; // -170 deg
 case 13: angle = a - 2.62;
   tangle = a - 2.755;
   xx += 0.208 * cos( tangle ) * ppm;
   yy += 0.208 * sin( tangle ) * ppm;
   break; // -150 deg
 case 14: angle = a - 2.27;
   tangle = a - 2.488;
   xx += 0.196 * cos( tangle ) * ppm;
   yy += 0.196 * sin( tangle ) * ppm;
   break; // -130 deg
 case 15: angle = a - 1.57;
   tangle = a - 2.240;
   xx += 0.172 * cos( tangle ) * ppm;
   yy += 0.172 * sin( tangle ) * ppm;
   break; // -90 deg
#endif
 }
      
      dx = cos( angle );
      dy = sin(  angle);
      
      int maxRange = (int)(5.0 * ppm); // 5m times pixels-per-meter
      int rng = 0;
      
      while( rng < maxRange &&
	     (img->get_pixel( (int)xx, (int)yy ) == 0 ||
	      img->get_pixel( (int)xx, (int)yy ) == color ) &&
	     (xx > 0) &&  (xx < img->width)
	     && (yy > 0) && (yy < img->height)  )
	{
	  xx+=dx;
	  yy+=dy;
	  rng++;
	}
      
      // set sonar value, scaled to current ppm
      sonar[s] = rng / ppm;
      
      if( world->win && world->win->showSensors )
	{
	  hitPts[s].x = (short)( (xx - world->win->panx)  );
	  hitPts[s].y = (short)( (yy - world->win->pany)  );
	  //hitPts[s].x = (short)( (xx - win->panx) * win->xscale );
	  //hitPts[s].y = (short)( (yy - win->pany) * win->yscale );
	}
    }
  
  redrawSonar = true;
  return true;
    }
  return false;
}

int CRobot::UpdateVision( Nimage* img )
{
  // if its time to recalculate vision
  if( world->timeNow - lastVision > world->visionInterval )
    {
      lastVision = world->timeNow;
      
      unsigned char* colors = new unsigned char[ cameraImageWidth ];
      float* ranges = new float[ cameraImageWidth ];
      
      
      // ray trace the 1d color / range image
      float startAngle = (a + cameraPan) - (cameraFOV / 2.0);
      float xRadsPerPixel = cameraFOV / cameraImageWidth;
      float yRadsPerPixel = cameraFOV / cameraImageHeight;
      unsigned char pixel = 0;

      float xx, yy;

      int maxRange = (int)(8.0 * world->ppm); // 8m times pixels-per-meter
      int rng = 0;

      // do the ray trace for each pixel across the 1D image
      for( int s=0; s < cameraImageWidth; s++)
	{
	  float dist, dx, dy, angle;
	  
	  angle = startAngle + ( s * xRadsPerPixel );
	  
	  dx = cos( angle );
	  dy = sin( angle );
	  
	  xx = x; // assume for now that the camera is at the middle
	  yy = y; // of the robot - easy to change this
	  rng = 0;
	  
	  pixel = img->get_pixel( (int)xx, (int)yy );
	  
	  // no need for bounds checking - get_pixel() does that internally
	  // i'll put a bound of 1000 ion the ray length for sanity
	  while( rng < 1000 && ( pixel == 0 || pixel == color ) )
	    {
	      xx+=dx;
	      yy+=dy;
	      rng++;
	      
	      pixel = img->get_pixel( (int)xx, (int)yy );
	    }
	  
	  // set color value
	  colors[s] = pixel;
	  // set range value, scaled to current ppm
	  ranges[s] = rng / world->ppm;
	}
      
      // now the colors and ranges are filled in - time to do blob detection
      
      int blobleft = 0, blobright = 0;
      unsigned char blobcol = 0;
      int blobtop = 0, blobbottom = 0;
      
      numBlobs = 0;
      
      // scan through the samples looking for color blobs
      for( int s=0; s < cameraImageWidth; s++ )
	{
	  if( colors[s] != 0 && colors[s] < ACTS_NUM_CHANNELS
	      && colors[s] != color )
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
	      
	      blobs[numBlobs].channel = blobcol-1;
	      blobs[numBlobs].x = xCenterOfBlob * 2;
	      blobs[numBlobs].y = yCenterOfBlob * 2;
	      blobs[numBlobs].left = blobleft * 2;
	      blobs[numBlobs].top = blobtop * 2;
	      blobs[numBlobs].right = blobright * 2;
	      blobs[numBlobs].bottom = blobbottom * 2;
	      blobs[numBlobs].area =
		(blobtop - blobbottom) * (blobleft-blobright) * 4;
	      
	      // set the blob color - look up the robot with this
	      // bitmap color and translate that to a visible channel no.
	      
	      for( CRobot* r = world->bots; r; r = r->next )
		if( r->color == blobcol )
		  {
		    
		    //cout << (int)(r->color) << " "
		    // << (int)(r->channel) << endl;
		    blobs[numBlobs].channel = r->channel;
		    break;
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
      
      return true;
    }
  return false;
}


int CRobot::UpdateAndPublishPtz( void )
{
  // if its time to recalculate vision
  if( world->timeNow - lastPtz > world->ptzInterval )
    {
      lastPtz = world->timeNow;

      // LOCK THE MEMORY
      world->LockShmem();

      short* cmd = (short*)( playerIO +  PTZ_COMMAND_START);

      short pan = ntohs(cmd[0]);
      short tilt = ntohs(cmd[1]);
      unsigned short zoom = (unsigned short) ntohs(cmd[2]);

      world->UnlockShmem();

      int p = pan;
      //int t = tilt;
      int t = 0;
      int z = zoom;

      // threshold panning
      if( p < cameraPanMin ) p = (short)cameraPanMin;
      if( p > cameraPanMax ) p = (short)cameraPanMax;

      cameraPan = -p * M_PI/180.0;

      //cameraTilt = t * M_PI/180.0; // leave tilt alone for now

      // threshold scaling
      if( z < 0 ) z = 0;
      if( z > 1024 ) z = 1024;

      double cameraAngleRange = cameraAngleMax - cameraAngleMin;
      double cameraScaling = cameraAngleRange / 1024.0;
      cameraFOV = cameraAngleMax - z * cameraScaling;

      short* ptzdata = (short*)( playerIO +  PTZ_DATA_START);

      // LOCK THE MEMORY
      world->LockShmem();

      ptzdata[0] = htons((short)p);
      ptzdata[1] = htons((short)t);
      (unsigned short)(ptzdata[2]) = htons((unsigned short)z);

      world->UnlockShmem();

      //cout << "p: " << p << " t: " << t << " z: " << z << endl;
      return true;
    }
  else
    return false;
}


void CRobot::Update()
{

  // legacy devices
  if( playerIO[ SUB_SONAR  ] && UpdateSonar( world->img ) ) PublishSonar();  
  if( playerIO[ SUB_VISION ] && UpdateVision( world->img ) ) PublishVision();
  if( playerIO[ SUB_PTZ ] ) UpdateAndPublishPtz();

  // Update all devices
  //
  for (int i = 0; i < m_device_count; i++)
    {
      // update subscribed devices, 
      if (m_device[i]->IsSubscribed() 
	  || ( world->win && world->win->dragging == this) )
	
	  m_device[i]->Update();
    }

}

bool CRobot::HasMoved( void )
{
  if( x != oldx || y != oldy || a != olda )
    return true;
  else
    return false;
}

void CRobot::MapUnDraw()
{
  for (int i = 0; i < m_device_count; i++) m_device[i]->MapUnDraw();
}  


void CRobot::MapDraw()
{
  for (int i = 0; i < m_device_count; i++) m_device[i]->MapDraw();
}  

void CRobot::GUIDraw()
{
  for (int i = 0; i < m_device_count; i++) m_device[i]->GUIDraw();
}  

void CRobot::GUIUnDraw()
{
  for (int i = 0; i < m_device_count; i++) m_device[i]->GUIUnDraw();
}  





/*************************************************************************
 * robot.cc - most of the action is here
 * RTV
 * $Id: robot.cc,v 1.5 2000-11-29 22:48:56 ahoward Exp $
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

extern int errno;

const double TWOPI = 6.283185307;

extern CWorld* world;
extern CWorldWin* win;

unsigned char f = 0xFF;

const int numPts = SONARSAMPLES;
const int laserSamples = LASERSAMPLES;


CRobot::CRobot( int iid, float w, float l,
  float startx, float starty, float starta )
{
  id = iid;

  color = id+1;
  next  = NULL;

  width = w;
  length = l;

  halfWidth = width / 2.0;   // the halves are used often in calculations
  halfLength = length / 2.0;

  xorigin = oldx = x = startx;
  yorigin = oldy = y = starty;
  aorigin = olda = a = starta;

  xodom = yodom = aodom = 0;


  channel = 0; // vision system color channel - default 0

  speed = 0.0;
  turnRate = 0.0;

  stall = 0;

  redrawSonar = false;
  redrawLaser = false;
  leaveTrail = false;


  memset( &rect, 0, sizeof( rect ) );
  memset( &oldRect, 0, sizeof( rect ) );

  lastLaser = lastSonar = lastVision = 0.0;


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
 sprintf( portBuf, "%d", PLAYER_BASE_PORT + id );

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

  StoreRect();

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
    TRACE0("starting devices");

    m_device_count = 0;
    
    // Create laser device
    //
    m_device[m_device_count++] = new CLaserDevice(playerIO + LASER_DATA_START,
                                                  LASER_DATA_BUFFER_SIZE,
                                                  LASER_COMMAND_BUFFER_SIZE,
                                                  LASER_CONFIG_BUFFER_SIZE);
    // Start all the devices
    //
    for (int i = 0; i < m_device_count; i++)
    {
        if (!m_device[i]->Startup(this, world))
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
    TRACE0("shutting down devices");
    
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


void CRobot::Stop( void )
{
  speed = turnRate = 0;
}

void CRobot::StoreRect( void )
{
  memcpy( &oldRect, &rect, sizeof( struct Rect ) );
  oldCenterx = centerx;
  oldCentery = centery;
}

void CRobot::CalculateRect( float x, float y, float a )
{
  // fill an array of Points with the corners of the robots new position
  float cosa = cos( a );
  float sina = sin( a );
  float cxcosa = halfLength * cosa;
  float cycosa = halfWidth  * cosa;
  float cxsina = halfLength * sina;
  float cysina = halfWidth  * sina;

  rect.toplx = (int)(x + (-cxcosa + cysina));
  rect.toply = (int)(y + (-cxsina - cycosa));
  rect.toprx = (int)(x + (+cxcosa + cysina));
  rect.topry = (int)(y + (+cxsina - cycosa));
  rect.botlx = (int)(x + (+cxcosa - cysina));
  rect.botly = (int)(y + (+cxsina + cycosa));
  rect.botrx = (int)(x + (-cxcosa - cysina));
  rect.botry = (int)(y + (-cxsina + cycosa));

  centerx = (int)x;
  centery = (int)y;
}


void CRobot::GetPositionCommands( void )
{
  //lock
  world->LockShmem();

  // read the command buffer and set the robot's speeds
  short v = *(short*)(playerIO + P2OS_COMMAND_START );
  short w = *(short*)(playerIO + P2OS_COMMAND_START + 2 );

  // unlock
  world->UnlockShmem();

  float fv = (float)(short)ntohs(v);//1000.0 * (float)ntohs(v);
  float fw = (float)(short)ntohs(w);//M_PI/180) * (float)ntohs(w);

  // set speeds unless we're being dragged
  if( win->dragging != this )
    {
      speed = 0.001 * fv;
      turnRate = -(M_PI/180.0) * fw;
    }

}


void CRobot::PublishPosition( void )
{
  // copy position data into the memory buffer shared with player

  // LOCK THE MEMORY
  world->LockShmem();

  double rtod = 180.0/M_PI;

  unsigned char* deviceData = (unsigned char*)(playerIO +
P2OS_DATA_START);

  //printf( "arena: playerIO: %d   devicedata: %d offset: %d\n", playerIO,
  // deviceData, (int)deviceData - (int)playerIO );

  // COPY DATA
  // position device
  *((int*)(deviceData + 0)) =
    htonl((int)((world->timeNow - world->timeBegan) * 1000.0));
  *((int*)(deviceData + 4)) = htonl((int)((x - xorigin)/ world->ppm *
1000.0));
  *((int*)(deviceData + 8)) = htonl((int)((y - yorigin)/ world->ppm *
1000.0));

  // odometry heading
  float odoHeading = fmod( aorigin -a + TWOPI, TWOPI );  // normalize angle
  
  *((unsigned short*)(deviceData + 12)) =
    htons((unsigned short)(odoHeading * rtod ));

  // speeds
  *((unsigned short*)(deviceData + 14))
    = htons((unsigned short)speed);

  *((unsigned short*)(deviceData + 16))
    = htons((unsigned short)(turnRate * rtod ));

  // compass heading
  float comHeading = fmod( a + M_PI/2.0 + TWOPI, TWOPI );  // normalize angle
  
  *((unsigned short*)(deviceData + 18))
    = htons((unsigned short)( comHeading * rtod ));

  // stall - currently shows if the robot is being dragged by the user
  *(unsigned char*)(deviceData + 20) = stall;

  // UNLOCK MEMORY
  world->UnlockShmem();
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

/*void CRobot::PublishLaser( void )
{
  // laser device
  //  int laserDataOffset = P2OS_DATA_BUFFER_SIZE +
P2OS_COMMAND_BUFFER_SIZE;

  world->LockShmem(); // lock buffer

  unsigned short ss;

  for( int c=0; c<LASERSAMPLES; c++ )
    {
      ss = (unsigned short)( laser[c] * 1000.0 );
      
      //(unsigned short)(playerIO[ c*2 + LASER_DATA_START ]) = htons(ss);  
      *((unsigned short*)(playerIO + c*2 + LASER_DATA_START)) = htons(ss);
      
    }

  world->UnlockShmem();
}

*/

//void CRobot::OrientRectangle( Rect& r1 )
//{
//Rect r2;

  




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


void CRobot::UnDraw( Nimage* img )
{
  //undraw the robot's old rectangle
  img->draw_line( oldRect.toplx, oldRect.toply,
    oldRect.toprx, oldRect.topry, 0 );
  img->draw_line( oldRect.toprx, oldRect.topry,
    oldRect.botlx, oldRect.botly, 0 );
  img->draw_line( oldRect.botlx, oldRect.botly,
    oldRect.botrx, oldRect.botry, 0 );
  img->draw_line( oldRect.botrx, oldRect.botry,
    oldRect.toplx, oldRect.toply, 0 );

  // solid robot
  //img->bgfill( centerx, centery, 0 );

  // 11/29/00 RTV -  took out the direction indicator 
  //img->draw_line( oldRect.botlx, oldRect.botly,
  //oldCenterx, oldCentery,0);
  //img->draw_line( oldRect.toprx, oldRect.topry,
  //oldCenterx, oldCentery,0);
}


void CRobot::Draw( Nimage* img )
{
  //draw the robot's rectangle
  img->draw_line( rect.toplx, rect.toply,
    rect.toprx, rect.topry, color );
  img->draw_line( rect.toprx, rect.topry,
    rect.botlx, rect.botly, color );
  img->draw_line( rect.botlx, rect.botly,
    rect.botrx, rect.botry, color );
  img->draw_line( rect.botrx, rect.botry,
    rect.toplx, rect.toply, color );

  img->draw_line( rect.toplx + 3, rect.toply + 3,
    rect.toprx -3, rect.topry + 3, color );
  /* img->draw_line( rect.toprx, rect.topry,
    rect.botlx, rect.botly, color );
  img->draw_line( rect.botlx, rect.botly,
    rect.botrx, rect.botry, color );
  img->draw_line( rect.botrx, rect.botry,
    rect.toplx, rect.toply, color );
  */

  // solid robot
  //img->bgfill( centerx, centery, color );

  // 11/29/00 RTV -  took out the direction indicator 
  //img->draw_line( rect.botlx, rect.botly,
  //centerx, centery, color );
  //img->draw_line( rect.toprx, rect.topry,
  //centerx, centery, color );
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

      if( win && win->showSensors )
 {
   hitPts[s].x = (short)( (xx - win->panx)  );
   hitPts[s].y = (short)( (yy - win->pany)  );
   //hitPts[s].x = (short)( (xx - win->panx) * win->xscale );
   //hitPts[s].y = (short)( (yy - win->pany) * win->yscale );
 }
    }

  redrawSonar = true;
  return true;
    }
  return false;
}

/*int CRobot::UpdateLaser( Nimage* img )
{
  if( world->timeNow - lastLaser > world->laserInterval )
    {
      lastLaser = world->timeNow;

      // draw the new laser beams and discover their ranges
      double startAngle = a + (M_PI_2);
      double increment = -M_PI/laserSamples;
      unsigned char pixel = 0;

      double xx, yy = y;

      int maxRange = (int)(8.0 * world->ppm); // 8m times pixels-per-meter
      int rng = 0;

      // get the laser ranges, ray-tracing every 2nd sample for speed.
      // then duplicating that range in the next sample
      for( int s=0; s < laserSamples; s++)
 {
   double dist, dx, dy, angle;

   angle = startAngle + ( s * increment );

   dx = cos( angle );
   dy = sin( angle);

   xx = x;
   yy = y;
   rng = 0;

   pixel = img->get_pixel( (int)xx, (int)yy );

   // no need for bounds checking - get_pixel() does that internally
   while( rng < maxRange && ( pixel == 0 || pixel == color ) )
     {
       xx+=dx;
       yy+=dy;
       rng++;

       pixel = img->get_pixel( (int)xx, (int)yy );
     }

   // set laser value, scaled to current ppm
   double v = rng / world->ppm;

   // copy value into laser buffer twice to save a bit of time
   //laser[ s+1 ] = laser[s] = v;
   laser[s] = v;
   // detect the type of surface that the laser has hit
   //colors[ s+1 ] = colors[s] = pixel;
   //colors[s] = pixel;
 
   if( win && win->showSensors ) //fill in the laser hit points for display
     { //just plot the ranges we actually calculated
       lhitPts[s].x = (short)( (xx - win->panx));
       lhitPts[s].y = (short)( (yy - win->pany));
       //lhitPts[s/2].x = (short)( (xx - win->panx) * win->xscale );
       //lhitPts[s/2].y = (short)( (yy - win->pany) * win->yscale );
     }
 }

      redrawLaser = true;
      return true;
    }
  return false;
}
*/

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

int CRobot::Move( Nimage* img )
{
  int moved = false;

  if( ( win->dragging != this ) &&  (speed != 0.0 || turnRate != 0.0) )
    {
      // record speeds now - they can be altered in another thread
      float nowSpeed = speed;
      float nowTurn = turnRate;
      float nowTimeStep = world->timeStep;

      // find the new position for the robot
      float tx = x + nowSpeed * world->ppm * cos( a ) * nowTimeStep;
      float ty = y + nowSpeed * world->ppm * sin( a ) * nowTimeStep;
      float ta = a + (nowTurn * nowTimeStep);
      ta = fmod( ta + TWOPI, TWOPI );  // normalize angle

      // calculate the rectangle for the robot's tentative new position
      CalculateRect( tx, ty, ta );

      // trace the robot's outline to see if it will hit anything
      char hit = 0;
      if( hit = img->rect_detect( rect, color ) > 0 )
	// hit! so don't do the move - just redraw the robot where it was
	{
	  //cout << "HIT! " << endl;
	  memcpy( &rect, &oldRect, sizeof( struct Rect ) );
	  Draw( img );
	  
	  stall = 1; // motors stalled due to collision
	}
      else // do the move
	{
	  moved = true;
	  
	  stall = 0; // motors not stalled
	  
	  x = tx; // move to the new position
	  y = ty;
	  a = ta;
	  
	  //update the robot's odometry estimate
	  xodom +=  nowSpeed * world->ppm * cos( aodom ) * nowTimeStep;
	  yodom +=  nowSpeed * world->ppm * sin( aodom ) * nowTimeStep;
	  
	  if( world->maxAngularError != 0.0 ) //then introduce some error
	    {
	      float error = 1.0 + ( drand48()* world->maxAngularError );
	      aodom += nowTurn * nowTimeStep * error;
	    }
	  else
	    aodom += nowTurn * nowTimeStep;
	  
	  aodom = fmod( aodom + TWOPI, TWOPI );  // normalize angle
	  
	  // erase and redraw the robot in the world bitmap
	  UnDraw( img );
	  Draw( img );
	  
	  // erase and redraw the robot on the X display
	  if( win ) win->DrawRobotIfMoved( this );
	  
	  // update the `old' stuff
	  memcpy( &oldRect, &rect, sizeof( struct Rect ) );
	  oldCenterx = centerx;
	  oldCentery = centery;
	}
    }
  
  oldx = x;
  oldy = y;
  olda = a;
  
  return moved;
}

int CRobot::HasMoved( void )
{
  //if the robot has moved on the world bitmap
  if( oldRect.toplx != rect.toplx ||
      oldRect.toply != rect.toply ||
      oldRect.toprx != rect.toprx ||
      oldRect.topry != rect.topry ||
      oldRect.botlx != rect.botlx ||
      oldRect.botly != rect.botly ||
      oldRect.botrx != rect.botrx ||
      oldRect.botry != rect.botry  ||
      centerx != oldCenterx ||
      centery != oldCentery )

    return true;
  else
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


void CRobot::Update( Nimage* img )
{
  // if someone has subscribed to the position device
  if( playerIO[ SUB_MOTORS ] )
    {
      // Get the desired speed and turn rate from Player
      GetPositionCommands();

      Move( img ); // try to move the robot

      // always publish the P2OS-like data, even if the robot didn't move
      // it contains the current time
      PublishPosition();
    }

  // only publish the sonar and laser if they were changed
  // UpdateFoo() methods return true if it was time to update this sensor
  if( playerIO[ SUB_SONAR  ] && UpdateSonar( img ) ) PublishSonar();
  //*** remove -- use new driver if( playerIO[ SUB_LASER  ] && UpdateLaser( img ) )  PublishLaser();
  if( playerIO[ SUB_VISION ] && UpdateVision( img ) ) PublishVision();
  if( playerIO[ SUB_PTZ ] ) UpdateAndPublishPtz();

  // Update all devices
  //
  for (int i = 0; i < m_device_count; i++) 
    // this subscription test is a hack for now!
       if( playerIO[ SUB_LASER ] ) m_device[i]->Update();
}







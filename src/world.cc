/*************************************************************************
 * world.cc - top level class that contains and updates robots
 * RTV
 * $Id: world.cc,v 1.4.2.3 2000-12-06 05:13:42 ahoward Exp $
 ************************************************************************/

#include <X11/Xlib.h>
#include <assert.h>
#include <fstream.h>
#include <iomanip.h> 
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define ENABLE_TRACE 1

#include "world.hh"
#include "win.h"
#include "offsets.h"

//#undef DEBUG
//#define DEBUG
//#define VERBOSE

#define SEMKEY 2000;

const double MILLION = 1000000.0;
const double TWOPI = 6.283185307;

int runDown = false;
double runStart;

//*** not used? ahoward unsigned int RGB( int r, int g, int b );

extern double quitTime;

CWorld::CWorld( char* initFile)
        : CObject(this, NULL)
{
  bots = NULL;

  //#ifdef VERBOSE
  cout << "[" << initFile << "]" << flush;
  //#endif

  // initialize the filenames
  bgFile[0] = 0;
  posFile[0] = 0;

  //zc = 0; //zonecount
  
  paused = false;
  
  // load the parameters from the initfile, including the position
  // and environment bitmap files.
  LoadVars( initFile); 

  // report the files we're using
  cout << "[" << bgFile << "][" << posFile << "]" << endl;

  if( population > 254 )
    {
      puts( "more than 254 robots requested. no can do. exiting." );
      exit( -1 ); // *** Exiting from a constructor is a BAD THING.  ahoward
    }

  //scale pioneer size to fit world - user specifies it in metres
  pioneerWidth *= ppm;
  pioneerLength *= ppm;

  localizationNoise = 0.0;
  sonarNoise = 0.0;
  maxAngularError =  0.0; // percent error on turning odometry

  // seed the random number generator
  srand48( time(NULL) );

  // create a new background image from the pnm file
  bimg = new Nimage( bgFile );
  //cout << "ok." << endl;

  width = bimg->width;
  height = bimg->height;

  // draw an outline around the background image
  bimg->draw_box( 0,0,width-1,height-1, 0xFF );
  
  // create a new forground image copied from the background
  img = new Nimage( bimg );

  // Extra data layers
  //
  m_laser_img = NULL;

  refreshBackground = false;


#ifdef DEBUG
  cout << "ok." << endl;
#endif

  int currentPopulation = 0;
  bots = NULL;

  // create the robots
#ifdef DEBUG
  cout << "Creating " << population << " robots... " << flush;
#endif

  CRobot* lastRobot = NULL;

#ifdef VERBOSE
  cout << "Making the robots... " << flush;
#endif

#ifdef VERBOSE
  cout << "Loading " << posFile << "... " << flush;
#endif

  ifstream in( posFile );

  /* *** REMOVE ahoward
  while( currentPopulation < population )
    {
      double xpos, ypos, theta;
      
      in >> xpos >> ypos >> theta;
      
#ifdef DEBUG
      cout << "read: " <<  xpos << ' ' << ypos << ' ' << theta << endl;
#endif



      CRobot* baby = (CRobot*)new CRobot( this, 
					  currentPopulation + 1,
					  pioneerWidth, 
					  pioneerLength,
					  xpos, ypos, theta );

      baby->channel = channels[currentPopulation];

      if( lastRobot ) lastRobot->next = baby;
      else bots = baby;      
      lastRobot = baby;

      // we're the parent and have spawned a Player...
      currentPopulation++;
   }
  */

  refreshBackground = true;
  
#ifdef VERBOSE
  cout << "done." << endl;
#endif

  // --------------------------------------------------------------------
  // create a single semaphore to sync access to the shared memory segments

#ifdef DEBUG  
  cout << "Obtaining semaphore... " << flush;
#endif

  semKey = SEMKEY;

  union semun
    {
      int val;
      struct semid_ds *buf;
      ushort *array;
  } argument;

  argument.val = 0; // initial semaphore value

  semid = semget( semKey, 1, 0666 | IPC_CREAT );

  if( semid < 0 ) // semget failed
    {
      perror( "Unable to create semaphore" );
      exit( -1 );
    }

  if( semctl( semid, 0, SETVAL, argument ) < 0 )
    {
      perror( "Failed to set semaphore value" );
    }
#ifdef DEBUG
  else
    cout << "ok" << endl;
#endif

  // ----------------------------------------------------------------------

  #ifndef INCLUDE_RTK
  refreshBackground = true;
  Draw(); // this will draw everything properly before we start up
  #endif

  // start the internal clock
  struct timeval tv;
  gettimeofday( &tv, NULL );
  timeNow = timeBegan = tv.tv_sec + (tv.tv_usec/MILLION);
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CWorld::~CWorld()
{
    // *** WARNING There are lots of things that should be delete here!!
    //
    // ahoward

    delete m_laser_img;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine -- creates objects in the world
//
bool CWorld::Startup(RtkCfgFile *cfg)
{
    TRACE0("Creating objects");

    // *** WARNING -- no overflow check
    // Set our id
    //
    strcpy(m_id, "world");
    
    // Initialise the world grids
    //
    StartupGrids();

    // Call the objects startup function
    //
    if (!CObject::Startup(cfg))
        return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine -- deletes objects in the world
//
void CWorld::Shutdown()
{
    TRACE0("Closing objects");
        
    CObject::Shutdown();
}


///////////////////////////////////////////////////////////////////////////
// Update the world
//
void CWorld::Update()
{
  // update is called every approx. 25ms from main using a timer
    #ifndef INCLUDE_RTK
        if( win ) win->HandleEvent();
    #endif

  if( !paused )
    {
      struct timeval tv;
      
      gettimeofday( &tv, NULL );
      timeNow = tv.tv_sec + (tv.tv_usec/MILLION);
      
      timeStep = (timeNow - timeThen); // real time
      //timeStep = 0.01; // step time;   
      //cout << timeStep << endl;
      timeThen = timeNow;
      
      // use a simple cludge to fix stutters caused by machine load or I/O
      if( timeStep > 0.1 ) 
	{
#ifdef DEBUG 
	  cout << "MAX TIMESTEP EXCEEDED" << endl;
#endif
	  timeStep = 0.1;
	}

      // Update child objects
      //
      CObject::Update();

      #ifndef INCLUDE_RTK
      if( refreshBackground ) Draw();
      #endif
      
      if( !runDown ) runStart = timeNow;
      else if( (quitTime > 0) && (timeNow > (runStart + quitTime) ) )
	exit( 0 );
    }
}


///////////////////////////////////////////////////////////////////////////
// Initialise the world grids
//
bool CWorld::StartupGrids()
{
    TRACE0("initialising grids");
    
    // Extra data layers
    // Ultimately, these should go into a startup routine,
    // or else get created by the devices themselves when they
    // are subscribed.  This would save some memory.
    // ahoward
    //
    m_laser_img = new Nimage(width, height);
    m_laser_img->clear(0);
    
    // Copy fixed obstacles into laser rep
    //
    for (int y = 0; y < img->height; y++)
    {
        for (int x = 0; x < img->width; x++)
        {
            if (img->get_pixel(x, y) != 0)
                m_laser_img->set_pixel(x, y, 1);
        }
    }
                
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Get a cell from the world grid
//
BYTE CWorld::GetCell(double px, double py, EWorldLayer layer)
{
    // Convert from world to image coords
    //
    int ix = (int) (px * ppm);
    int iy = height - (int) (py * ppm);

    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            return img->get_pixel(ix, iy);
        case layer_laser:
            return m_laser_img->get_pixel(ix, iy);
    }
    return 0;
}


///////////////////////////////////////////////////////////////////////////
// Set a cell in the world grid
//
void CWorld::SetCell(double px, double py, EWorldLayer layer, BYTE value)
{
    // Convert from world to image coords
    //
    int ix = (int) (px * ppm);
    int iy = height - (int) (py * ppm);

    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            img->set_pixel(ix, iy, value);
            break;
        case layer_laser:
            m_laser_img->set_pixel(ix, iy, value);
            break;
    }
}


///////////////////////////////////////////////////////////////////////////
// Set a rectangle in the world grid
//
void CWorld::SetRectangle(double px, double py, double pth,
                          double dx, double dy, EWorldLayer layer, BYTE value)
{
    Rect rect;
    double tx, ty;

    dx /= 2;
    dy /= 2;

    double cx = dx * cos(pth);
    double cy = dy * cos(pth);
    double sx = dx * sin(pth);
    double sy = dy * sin(pth);
    
    // This could be faster
    //
    tx = px + cx - sy;
    ty = py + sx + cy;
    rect.toplx = (int) (tx * ppm);
    rect.toply = height - (int) (ty * ppm);

    tx = px - cx - sy;
    ty = py - sx + cy;
    rect.toprx = (int) (tx * ppm);
    rect.topry = height - (int) (ty * ppm);

    tx = px - cx + sy;
    ty = py - sx - cy;
    rect.botlx = (int) (tx * ppm);
    rect.botly = height - (int) (ty * ppm);

    tx = px + cx + sy;
    ty = py + sx - cy;
    rect.botrx = (int) (tx * ppm);
    rect.botry = height - (int) (ty * ppm);
    
    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            img->draw_rect(rect, value);
            break;
        case layer_laser:
            m_laser_img->draw_rect(rect, value);
            break;
    }
}
  

int CWorld::LockShmem( void )
{
  struct sembuf ops[1];

  ops[0].sem_num = 0;
  ops[0].sem_op = 1;
  ops[0].sem_flg = 0;

  int retval = semop( semid, ops, 1 );

#ifdef DEBUG
  if( retval == 0 )
    puts( "successful lock" );
  else
    puts( "failed lock" );
#endif

  return true;
}


int CWorld::UnlockShmem( void )
{
  struct sembuf ops[1];

  ops[0].sem_num = 0;
  ops[0].sem_op = -1;
  ops[0].sem_flg = 0;

  int retval = semop( semid, ops, 1 );


#ifdef DEBUG
  if( retval == 0 )
    puts( "successful unlock" );
  else
    puts( "failed unlock" );
#endif

  return true;
}

void CWorld::SavePos( void )
{
  ofstream out( posFile );

  for( CRobot* r = bots; r; r = r->next )
    out <<  r->x/ppm <<  '\t' << r->y/ppm << '\t' << r->a << endl;
}

float diff( float a, float b )
{
  if( a > b ) return a-b;
  return b-a;
}

int CWorld::LoadVars( char* filename )
{
#ifdef DEBUG
  cout << "\nLoading world parameters from " << filename << "... " << flush;
#endif
  
  ifstream init( filename );
  char buffer[255];
  char c;

  if( !init ) return -1;

  char token[50];
  char value[200];
  
  while( !init.eof() )
    {
      init.get( buffer, 255, '\n' );
      init.get( c );
    
      sscanf( buffer, "%s = %s", token, value );
      
#ifdef DEBUG      
      printf( "token: %s value: %s.\n", token, value );
      fflush( stdout );
#endif
      
      if( token[0] == '#' ) // its a comment; ignore it
	{}
      else if ( strcmp( token, "robots" ) == 0 )
	population = strtol( value, NULL, 10 );
      else if ( strcmp( token, "robot_width" ) == 0 )
	pioneerWidth = strtod( value, NULL );  
      else if ( strcmp( token, "robot_length" ) == 0 )
	pioneerLength = strtod( value, NULL );  
      else if ( strcmp( token, "pixels_per_meter" ) == 0 )
	ppm = strtod( value, NULL );  
      else if ( strcmp( token, "environment_file" ) == 0 )
	sscanf( value, "%s", bgFile );  
      else if ( strcmp( token, "position_file" ) == 0 )
	sscanf( value, "%s", posFile );  
      //else if ( strcmp( token, "sonar_noise_variance" ) == 0 )
      //sonarNoise = strtod( value, NULL );  
      //else if ( strcmp( token, "localization_noise_variance" ) == 0 )
      //localizationNoise = strtod( value, NULL );  
      else if ( strcmp( token, "robot_colors" ) == 0 )
	{
	  //printf( "Colors: %s\n", value );
	  
	  // point into th line buffer after the equals sign
	  char* tokstr = strstr( buffer, "=" ) + 1;
	  
	  char* tok = strtok( tokstr, " \t" );
	  
	  int n = -1;
	  for( int c=0; tok; c++ )
	    {
	      n = (int)strtol( tok, NULL, 0 );
	      
	      if( c > 254 ) 
		{
		  puts( "bounds error loading color table" );
		  exit( -1 );
		}

	      channels[c] = n;
	      //printf( "color: %d   tok: %s\n", n, tok );
	      
	      tok =  strtok( NULL, " \t" );
	    }
	}
      else if ( strcmp( token, "quit_time" ) == 0 )
	quitTime = strtod( value, NULL );  
    }

#ifdef DEBUG
  cout << "ok." << endl;
#endif

  return 1;
}


#ifndef INCLUDE_RTK


CRobot* CWorld::NearestRobot( float x, float y )
{
  CRobot* nearest;
  float dist, far = 999999.9;
  
  for( CRobot* r = bots; r; r = r->next )
  {
    dist = hypot( r->x -  x, r->y -  y );
    
    if( dist < far ) 
      {
	nearest = r;
	far = dist;
      }
  }
  
  return nearest;
}

#else

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CWorld::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw the background
    //
    if (pData->DrawBack("global"))
        DrawBackground(pData);

    // Draw grid layers (for debugging)
    //
    pData->BeginSection("global", "debugging");

    if (pData->DrawLayer("obstacle", false))
        DrawLayer(pData, layer_obstacle);
        
    if (pData->DrawLayer("laser", false))
        DrawLayer(pData, layer_laser);
    
    pData->EndSection();

    // Get children to process message
    //
    CObject::OnUiUpdate(pData);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CWorld::OnUiMouse(RtkUiMouseData *pData)
{
    // Get children to process message
    //
    CObject::OnUiMouse(pData);
}


///////////////////////////////////////////////////////////////////////////
// Draw the background; i.e. things that dont move
//
void CWorld::DrawBackground(RtkUiDrawData *pData)
{
    TRACE0("drawing background");

    pData->SetColor(RGB(0, 0, 0));
    
    // Loop through the image and draw points individually.
    // Yeah, it's slow, but only happens once.
    //
    for (int y = 0; y < img->height; y++)
    {
        for (int x = 0; x < img->width; x++)
        {
            if (img->get_pixel(x, y) != 0)
            {
                double px = (double) x / ppm;
                double py = (double) (height - y) / ppm;
                double s = 1.0 / ppm;
                pData->Rectangle(px, py, px + s, py + s);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser layer
//
void CWorld::DrawLayer(RtkUiDrawData *pData, EWorldLayer layer)
{
    Nimage *img = NULL;

    // This could be cleaned up by having an array of images
    //
    switch (layer)
    {
        case layer_obstacle:
            img = this->img;
            break;
        case layer_laser:
            img = m_laser_img;
            break;
        default:
            return;
    }
    
    // Loop through the image and draw points individually.
    // Yeah, it's slow, but only happens for debugging
    //
    for (int y = 0; y < img->height; y++)
    {
        for (int x = 0; x < img->width; x++)
        {
            if (img->get_pixel(x, y) != 0)
            {
                double px = (double) x / ppm;
                double py = (double) (height - y) / ppm;
                pData->SetPixel(px, py, RGB(0, 0, 255));
            }
        }
    }
}


#endif





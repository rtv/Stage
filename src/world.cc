/*************************************************************************
 * world.cc - top level class that contains and updates robots
 * RTV
 * $Id: world.cc,v 1.7 2000-12-08 09:08:11 vaughan Exp $
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

#include "world.h"
#include "win.h"
#include "offsets.h"

//#undef DEBUG
//#define DEBUG
//#define VERBOSE
//#define DISPLAYFREQ

#define SEMKEY 2000;

const double MILLION = 1000000.0;
const double TWOPI = 6.283185307;

int runDown = false;
double runStart;

unsigned int RGB( int r, int g, int b );

extern double quitTime;

CWorld::CWorld( char* initFile ) 
{
  bots = NULL;
  win = NULL;
  
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

  sonarInterval = 0.1;// seconds
  laserInterval = 0.2; // seconds
  visionInterval = 1.0; // seconds
  ptzInterval = 0.5; //seconds

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
  // Ultimately, these should go into a startup routine,
  // or else get created by the devices themselves when they
  // are subscribed.  This would save some memory.
  // ahoward
  //
  m_laser_img = new Nimage(width, height);
  m_laser_img->clear(0);

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
  
  in.close(); // finished with the position file

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

  refreshBackground = true;
  Draw(); // this will draw everything properly before we start up

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

  // this should be everything! - RTV
  
  // kill all the robots
  CRobot* doomed = bots;
  
  while( doomed )
    {
      CRobot* next = doomed->next;
      delete doomed;
      doomed = next;
    }
  
  delete m_laser_img;
  delete bimg;
  delete img;
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

void CWorld::Update( void )
{
  // update is called every approx. 25ms from main using a timer

  static double smoothedTimestep = 0;

  if( win ) win->HandleEvent();

  if( !paused )
    {
      struct timeval tv;
      
      gettimeofday( &tv, NULL );
      timeNow = tv.tv_sec + (tv.tv_usec/MILLION);
      
      timeStep = (timeNow - timeThen); // real time
      //timeStep = 0.01; // step time;   
      //cout << timeStep << endl;
      timeThen = timeNow;
      
#ifdef DISPLAYFREQ
      smoothedTimestep = (smoothedTimestep * 0.9) + ( 0.1 * timeStep );
      
      printf( "\r%.4f   ", smoothedTimestep );
      fflush( stdout );
#endif

      // use a simple cludge to fix stutters caused by machine load or I/O
      if( timeStep > 0.1 ) 
	{
#ifdef DEBUG 
	  cout << "MAX TIMESTEP EXCEEDED" << endl;
#endif
	  timeStep = 0.1;
	}
      
      // move the robots
      for( CRobot* b = bots; b; b = b->next ) b->Update();

      if( refreshBackground ) Draw();
      
      if( !runDown ) runStart = timeNow;
      else if( (quitTime > 0) && (timeNow > (runStart + quitTime) ) )
	exit( 0 );
    }
}
  
    
CRobot* CWorld::NearestRobot( double x, double y )
{
  CRobot* nearest;
  double dist, far = 999999.9;
  
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


void CWorld::SavePos( void )
{
  ofstream out( posFile );

  for( CRobot* r = bots; r; r = r->next )
    out <<  r->x/ppm <<  '\t' << r->y/ppm << '\t' << r->a << endl;
}

void CWorld::Draw( void )
{
  memcpy( img->data, bimg->data, width*height*sizeof(char) );

  for( CRobot* r = bots; r; r = r->next ) r->MapDraw();

  refreshBackground = 0;
}

double diff( double a, double b )
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







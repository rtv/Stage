/*************************************************************************
 * xgui.cc - all the graphics and X management
 * RTV
 * $Id: xs.cc,v 1.11.2.3 2001-08-31 02:00:50 vaughan Exp $
 ************************************************************************/

#include <X11/keysym.h> 
#include <X11/keysymdef.h> 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>	/* basic system data types */
#include <sys/socket.h>	/* basic socket definitions */
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <values.h>
#include <signal.h>
#include <fstream.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>  // for atoi(3)
#include <string.h> /* for strcpy() */

#include <queue> // STL containers
#include <map>

#include "stage_types.hh"
#include "truthserver.hh"
#include "xs.hh"

#define MIN_WINDOW_WIDTH 0
#define MIN_WINDOW_HEIGHT 0

//#define DEBUG
//#define VERBOSE

Display* display = 0; 
int screen = 0;

const char* versionStr = "0.1";
const char* titleStr = "X Player/Stage Interface";

//#define LABELS
//typedef void (*callback_t)(truth_t*);

#define USAGE  "\nUSAGE: xs [-h <host>] [-tp <port>] [-ep <port>]\n\t[-geometry <geometry string>] [-zoom <factor>]\n\t[-pan <X\%xY\%>] [-channels <\"color0 .... colorN\">]\nDESCRIPTIONS:\n-h <host>: connect to Stage on this host (default `localhost')\n-tp <port>: connect to Stage's Truth server on this TCP port (default `6001')\n-ep <port>: connect to Stage's Environment server on this TCP port (default `6001')\n-geometry <string>*: standard X geometry specification\n-zoom <factor>*: floating point zoom multiplier\n-pan <X\%xY\%>*: pan initial view X percent of maximum by Y percent of maximum\n-channels <string>*: specify the colors drawn for each ACTS channel; e.g. \"red green blue\"\n(* this option can be set in your ~/.Xdefaults file - see README.xs)\n"

char stage_host[256] = "localhost";
int truth_port = 6001;
int env_port = 6002;

struct hostent* entp = 0;
struct sockaddr_in servaddr;

char* geometry = 0;
char* channels = 0;
char* zoom = 0;
char* pan = 0;

// provide some sensible defaults for the parameters/resources
char* default_channels = "red green blue magenta yellow cyan";
char* default_geometry = "400x400";
char* default_zoom = "1.0";
char* default_pan = "0x0";

std::queue<stage_truth_t> incoming_queue;
std::queue<stage_truth_t> outgoing_queue;

//pthread_mutex_t incoming_mutex;
//pthread_mutex_t outgoing_mutex;

// an associative array, indexed by player ID (neat!)
//typedef map< CPlayerID, truth_t > TruthMap;
typedef std::map< int, truth_t > TruthMap;
TruthMap truth_map; 

int truthfd = 0;

//////////////////////////////////////////////////////////////////////////

void CatchSigPipe( int signo )
{
#ifdef VERBOSE
  cout << "** SIGPIPE! **" << endl;
#endif

  exit( 0 );
}

void ExitFunc( void )
{
  //  pthread_mutex_destroy( &incoming_mutex );
  // pthread_mutex_destroy( &outgoing_mutex );
}

/* supply the desription string for a given device code */

char* CXGui::PlayerNameOf( const player_id_t& ent ) 
  { 
    switch( ent.type ) 
      { 
      case 0: return "None"; 
      case PLAYER_PLAYER_CODE: return "Player"; 
      case PLAYER_MISC_CODE: return "Misc"; 
      case PLAYER_GRIPPER_CODE: return "Gripper"; 
      case PLAYER_POSITION_CODE: return "Position"; 
      case PLAYER_SONAR_CODE: return "Sonar"; 
      case PLAYER_LASER_CODE: return "Laser"; 
      case PLAYER_VISION_CODE: return "Vision"; 
      case PLAYER_PTZ_CODE: return "PTZ"; 
      case PLAYER_AUDIO_CODE: return "Audio"; 
      case PLAYER_LASERBEACON_CODE: return "LaserBcn"; 
      case PLAYER_BROADCAST_CODE: return "Broadcast"; 
      case PLAYER_SPEECH_CODE: return "Speech"; 
      case PLAYER_TRUTH_CODE: return "Truth"; 
      case PLAYER_GPS_CODE: return "GPS"; 
      }	 
    return "Unknown"; 
  } 

char* CXGui::StageNameOf( const truth_t& truth ) 
  { 
    switch( truth.stage_type ) 
      { 
      case 0: return "None"; 
      case PlayerType: return "Player"; 
      case MiscType: return "Misc"; 
      case RectRobotType: return "RectRobot"; 
      case RoundRobotType: return "RoundRobot"; 
      case SonarType: return "Sonar"; 
      case LaserTurretType: return "Laser"; 
      case VisionType: return "Vision"; 
      case PtzType: return "PTZ"; 
      case BoxType: return "Box"; 
      case LaserBeaconType: return "LaserBcn"; 
      case LBDType: return "LBD"; 
      case VisionBeaconType: return "VisionBcn"; 
      case GripperType: return "Gripper"; 
      case AudioType: return "Audio"; 
      case BroadcastType: return "Bcast"; 
      case SpeechType: return "Speech"; 
      case TruthType: return "Truth"; 
      case GpsType: return "GPS"; 
      case PuckType: return "Puck"; 
      case OccupancyType: return "Occupancy"; 
      }	 
    return "Unknown"; 
  } 


void PrintStageTruth( stage_truth_t &truth )
{
  printf( "ID: %d (%4d,%d,%d)\tPID:(%4d,%d,%d)\tpose: [%d,%d,%d]\tsize: [%d,%d]\n", 
	  truth.stage_id,
	  truth.id.port, 
	  truth.id.type, 
	  truth.id.index,
	  truth.parent.port, 
	  truth.parent.type, 
	  truth.parent.index,
	  truth.x, truth.y, truth.th,
	  truth.w, truth.h );
  
  fflush( stdout );
}

void CXGui::PrintMetricTruth( int stage_id, truth_t &truth )
{
  printf( "%p:%s\t(%4d,%d,%d)\t(%4d,%d,%d)\t[%.2f,%.2f,%.2f]\t[%.2f,%.2f]\tACTS: %d\n",
	  (int*)stage_id,
	  StageNameOf( truth ),
	  truth.id.port, 
	  truth.id.type, 
	  truth.id.index,
	  truth.parent.port, 
	  truth.parent.type, 
	  truth.parent.index,
	  truth.x, truth.y, truth.th,
	  truth.w, truth.h,
	  truth.channel );
  
  fflush( stdout );
}

void CXGui::PrintMetricTruthVerbose( int stage_id, truth_t &truth )
{
  printf( "stage: %p:%s\tplayer: (%4d,%s:%d)\tparent(%4d,%s:%d)\tpose: [%.2f,%.2f,%.2f]\tsize: [%.2f,%.2f]\tACTS: %d\n", 
	  (int*)stage_id,
	  StageNameOf( truth ),
	  truth.id.port, 
	  PlayerNameOf( truth.id ), 
	  truth.id.index,
	  truth.parent.port, 
	  PlayerNameOf( truth.parent ), 
	  truth.parent.index,
	  truth.x, truth.y, truth.th,
	  truth.w, truth.h,
	  truth.channel );
  
  fflush( stdout );
}




static void* TruthReader( void*)
{
  int ffd = truthfd;

  //printf( "READING on %d\n", ffd );
  //fflush( stdout );

  /* create default mutex type (NULL) which is fast */
  //pthread_mutex_init( &incoming_mutex, NULL );

  pthread_detach(pthread_self());

  stage_truth_t truth;

  int r = 0;
  while( 1 )
    {	      
      int packetlen = sizeof(truth);
      int recv = 0;
     
      while( recv < packetlen )
     {
       //printf( "Reading on %d\n", ffd ); fflush( stdout );

       /* read will block until it has some bytes to return */
       r = read( ffd, &truth,  packetlen - recv );
       
       if( r < 0 )
	 perror( "TruthReader(): read error" );
       else
	 recv += r;
     }
     
      //printf( "RECV: " );
      //PrintStageTruth( truth );
     
      //pthread_mutex_lock( &incoming_mutex );
      incoming_queue.push( truth );
      //pthread_mutex_unlock( &incoming_mutex );
    }

    /* shouldn't ever get here, but just to be tidy */

  //     delete[]  buffer;
      
    exit(0);
}


static void* TruthWriter( void* )
{
  int ffd = truthfd;

  //printf( "WRITING on %d\n", ffd );
  //fflush( stdout );
  
  /* create default mutex type (NULL) which is fast */
  //pthread_mutex_init( &outgoing_mutex, NULL );

  pthread_detach(pthread_self());

  //puts( "TRUTH WRITER" );
  //fflush( stdout );

  // write out anything we find on the output queue
  
  while( 1 )
    {
      // very cautiously mutex the queue
      //pthread_mutex_lock( &outgoing_mutex );
      
      while( !outgoing_queue.empty() )
	{
	  
	  stage_truth_t struth = outgoing_queue.front();
	  outgoing_queue.pop();
	  
	  //printf( "WRITING %d bytes on %d", sizeof(struth), ffd );
	  //fflush( stdout );
	  
	  size_t b = write( ffd, &struth, sizeof( struth ) );
	  
	  if( b < 0 )
	    perror( "TruthWriter: write error" );
	  else if( b < sizeof( struth ) )
	    printf( "TruthWriter: short write (%d/%d bytes written)\n",
		    b, sizeof( struth ) );
	  
	}
      
      //pthread_mutex_unlock( &outgoing_mutex );	  
      
      usleep( 10000 );
    }
}


bool ConnectToTruth( void )
{
  /* open socket for network I/O */
  truthfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if( truthfd < 0 )
    {
      printf( "Error opening network socket. Exiting\n" );
      fflush( stdout );
      exit( -1 );
    }


  /* setup our server address (type, IP address and port) */
  bzero(&servaddr, sizeof(servaddr)); /* initialize */
  servaddr.sin_family = AF_INET;   /* internet address space */
  servaddr.sin_port = htons( truth_port ); /*our command port */ 
  memcpy(&(servaddr.sin_addr), entp->h_addr_list[0], entp->h_length);

  //printf( "[Truth on %s]", entp->h_name ); fflush( stdout );

  int v = connect( truthfd, (sockaddr*)&servaddr, sizeof( servaddr) );
    
  if( v < 0 )
    {
      printf( "Can't find a Stage truth server on %s. Quitting.\n", 
	      stage_host ); 
      perror( "" );
      fflush( stdout );
      exit( -1 );
    }
  //#ifdef VERBOSE
  //else
  //puts( " OK." );
  //#endif

/*-----------------------------------------------------------*/
      
  signal( SIGPIPE, CatchSigPipe );

  //printf( "Starting threads on %d\n", truthfd );

  // kick off the read and write threads
  pthread_t tid_dummy;
  memset( &tid_dummy, 0, sizeof( tid_dummy ) );

  if( pthread_create(&tid_dummy, NULL, &TruthReader, NULL ) != 0 )
    puts( "error creating truth reader thread" );
  
  if( pthread_create(&tid_dummy, NULL, &TruthWriter, NULL ) != 0 )  
    puts( "error creating truth writer thread" );

  return 1;
} 


bool DownloadEnvironment( environment_t* env )
{
  /* open socket for network I/O */
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if( sockfd < 0 )
    {
      printf( "Error opening network socket. Exiting\n" );
      fflush( stdout );
      exit( -1 );
    }

  /* setup our server address (type, IP address and port) */
  bzero(&servaddr, sizeof(servaddr)); /* initialize */
  servaddr.sin_family = AF_INET;   /* internet address space */
  servaddr.sin_port = htons( env_port ); /*our command port */ 
  //inet_pton(AF_INET, STAGE_IP, &servaddr.sin_addr); /* the arena IP */
  memcpy(&(servaddr.sin_addr), entp->h_addr_list[0], entp->h_length);
  
  if( connect( sockfd, (sockaddr*)&servaddr, sizeof( servaddr) )  != 0 )
    {     
      perror( "\nFailed to connect to environment server" );
      return false; // connect failed - boo.
    }

  player_occupancy_data_t header;
  int len = sizeof( header );
  int recv = 0;
  int r = 0;
  
  while( recv < len )
    {
      r = read( sockfd, &header, len-recv );
      if( r < 0 )
	{
	  perror( "\nError reading environment header" );
	  return false;
	}
      
      //printf( "read %d bytes\n", 
      recv += r;
    }
  
    // copy in the data
  env->width = header.width;
  env->height= header.height;
  env->ppm = header.ppm;
  env->num_pixels = header.num_pixels;

  if( env->num_pixels > 0 )
    {
      // allocate the pixel storage
      env->pixels = new XPoint[ env->num_pixels ]; 
      len = env->num_pixels * sizeof( XPoint );

      // download the pixels
      int recv = 0;
      int res = 0;
      
      while( recv < len )
	{
	  res = read( sockfd, (char*)(env->pixels) + recv, len-recv );
	  if( res < 0 )
	    {
	      perror( "EnvReader read error" );
	      return false;
	    }
	    
	  recv += res;

	  //#ifdef DEBUG
	  //printf( "read %d/%d bytes\r", recv, len );
	  //fflush( stdout );
	  //#endif
	}		
      
      //#ifdef DEBUG
      //printf( "\nreceived %d pixels\nDONE", env->num_pixels );
      //fflush( stdout );
      //#endif
    }

  // invert the y axis!
  for( int c=0; c<env->num_pixels; c++ )
    env->pixels[c].y = env->height - env->pixels[c].y;

  // make some space for the scaled pixels
  // they get filled in when call ScaleBackground()
  env->pixels_scaled = new XPoint[ env->num_pixels ];
    
  // all seems well...
  return true;
}

///////////////////////////////////////////////////////////////////

void  CXGui::SetupZoom( char* zm )
{
  assert( zm );
  double factor = strtod( zm, 0 );

  iwidth = (int)(factor * env->width); // STARTWIDTH
  iheight = (int)(factor * env->height); // STARTHEIGHT
  CalcPPM();
  
#ifdef VERBOSE
  printf( "Zoom factor: %.2f (ppm = %.2f)\n", factor, ppm );
#endif

}

void CXGui::SetupPan( char* pn )
{
  int xpc, ypc;

  // format is integer percentage
  sscanf( pn, "%dx%d", &xpc, &ypc );

  panx = ((iwidth - width) * xpc) / 100;
  pany = ((iheight - height) * ypc ) / 100;

#ifdef VERBOSE
  printf( "Pan: %d x %d\n", panx, pany );
#endif
}

void  CXGui::SetupGeometry( char* geom )
{    
  unsigned int uw, uh;
  XParseGeometry( geom, &x, &y, &uw, &uh ); 
  
  width = uw;
  height = uh;

#ifdef VERBOSE
  printf( "Geometry: %d,%d %dx%d\n", x, y, width, height );
#endif
}

void  CXGui::SetupChannels( char* channels )
{
  assert( channels );
  assert( strlen(channels) > 0 );
    
  XColor a, b;
  int c = 0;
  char* token = strtok( channels, " \t" );

  while( token )
    {
      if( !XAllocNamedColor( display, default_cmap, token, &a, &b ) )
	{
	  printf( "Warning: color name %s not recognized for channel %d\n",
		  token, c );
	}
      
#ifdef VERBOSE
      printf( "ACTS channel %d: \"%s\" (%d,%d,%d)\n",
	      c, token, a.red, a.green, a.blue );
#endif
      channel_colors[c++] = a.pixel;
      
      token = strtok( NULL, " \t" );
    }
}

/* easy little command line argument parser */
void parse_args(int argc, char** argv)
{
  int i;

  i=1;
  while(i<argc)
  {
    if(!strcmp(argv[i],"-geometry"))
      {
	if(++i<argc)
	  { 
	    geometry = new char[ strlen( argv[i] )+1 ];
	    memset( geometry, 0, strlen(argv[i]) +1 );
	    strncpy( geometry,argv[i], strlen( argv[i] ) );
	    printf( "[geometry: %s]", geometry );
	  }
	else
	  {
	    printf( "\n%s\n", USAGE );
	    exit(1);
	  }
      }
    else if(!strcmp(argv[i],"-zoom"))
      {
	if(++i<argc)
	  { 
	    zoom = new char[ strlen( argv[i] )+1 ];
	    memset( zoom, 0, strlen(argv[i]) +1 );
	    strncpy( zoom,argv[i], strlen( argv[i] ) );
	    printf( "[zoom: %s]", zoom );
	  }
	else
	  {
	    printf( "\n%s\n", USAGE );
	    exit(1);
	  }
      }
    else if(!strcmp(argv[i],"-pan"))
      {
	if(++i<argc)
	  { 
	    pan = new char[ strlen( argv[i] ) +1 ];
	    memset( pan, 0, strlen(argv[i]) +1 );
	    strncpy( pan,argv[i], strlen( argv[i] ) );
	    printf( "[pan: %s]", pan );
	  }
	else
	  {
	    printf( "\n%s\n", USAGE );
	    exit(1);
	  }
      }
    else if(!strcmp(argv[i],"-channels"))
      {
	if(++i<argc)
	  { 
	    channels = new char[ strlen( argv[i] ) + 1 ];
	    memset( channels, 0, strlen(argv[i]) +1 );
	    strncpy( channels,argv[i], strlen( argv[i] ) );
	    printf( "[channels: %s]", channels );
	  }
	else
	  {
	    printf( "\n%s\n", USAGE );
	    exit(1);
	  }
      }
    else if(!strcmp(argv[i],"-h"))
    {
      if(++i<argc)
	{ 
	  strcpy(stage_host,argv[i]);
	  printf( "[%s]", stage_host );
	}
      else
      {
	printf( "\n%s\n", USAGE );
        exit(1);
      }
    }
    else if(!strcmp(argv[i],"-tp"))
      {
        if(++i<argc)
	  {
	    truth_port = atoi(argv[i]); 
	    printf( "[Truth port %d]", truth_port );
	  }
	else
	  {
	    puts(USAGE);
	    exit(1);
	  }
      }
    else if(!strcmp(argv[i],"-ep"))
      {
        if(++i<argc)
	  {
	    env_port = atoi(argv[i]); 
	    printf( "[Environment port %d]", env_port );
	  }
	else
        {
          puts(USAGE);
          exit(1);
       }
    }
    else
    {
      puts(USAGE);
      exit(1);
    }
    i++;
  }

  // end the startup output line
  puts( "" );
  fflush( stdout ); 
}


int main(int argc, char **argv)
{
  printf( "** XS v%s ** ", versionStr );
  fflush( stdout );

  // register the exit function - deletes the mutexs
  atexit( ExitFunc );

  // parse out the required port and host 
  parse_args(argc,argv); 

  if((entp = gethostbyname( stage_host )) == NULL)
    {
      fprintf(stderr, "\nCan't find host \"%s\"; quitting\n", 
	      stage_host );
	      exit( -1 );
    }	  
  
  environment_t env;
 
  if( !DownloadEnvironment( &env ) )
     {
      puts( "\nFailed to download Stage's environment. Exiting" );
      exit( -1 );
    }

  if( !ConnectToTruth() )
    {
      puts( "\nFailed to connect to Stage's truth server. Exiting" );
      exit( -1 );
    }

  // now we have the environment and we have access to truths,
  // we can make a window
  CXGui* win = new CXGui( argc, argv, &env ); 

  assert( win ); // make sure the window was created

  // we're set up, so go into the read/handle loop
  while( true )
  {
    win->HandleXEvent();
    win->HandleIncomingQueue();
    // snooze to avoid hogging the processor
    usleep( 100000 ); // 0.05 seconds 
  }
}


//#define LABELS

  #define ULI unsigned long int
  #define USI unsigned short int

void CXGui::HandleIncomingQueue( void )
{
  //pthread_mutex_lock( &incoming_mutex );

  while( !incoming_queue.empty() )
    {
      stage_truth_t struth = incoming_queue.front();
      incoming_queue.pop();

      truth_t truth;

      int stage_id = truth.stage_id = struth.stage_id; // this is the map key

      truth.stage_type = struth.stage_type;

      truth.channel = struth.channel;

      truth.id.port = struth.id.port;
      truth.id.type = struth.id.type;
      truth.id.index = struth.id.index;
      
      truth.parent.port = struth.parent.port;
      truth.parent.type = struth.parent.type;
      truth.parent.index = struth.parent.index;

      truth.x = (double)struth.x / 1000.0;
      truth.y = (double)struth.y / 1000.0;
      truth.w = (double)struth.w / 1000.0;
      truth.h = (double)struth.h / 1000.0;
      //truth.th = NORMALIZE(DTOR( struth.th ));
      truth.th = DTOR( struth.th );

      truth.rotdx = (double)(struth.rotdx / 1000.0);
      truth.rotdy = (double)(struth.rotdy / 1000.0);

      //int presize = truth_map.size();
      truth_t old = truth_map[ stage_id ]; // get any old data
      //int postsize = truth_map.size();

      //if( presize != postsize )
      //{
      //  printf( "FOUND: " );
      //  PrintMetricTruth( old );
	  
      //  printf( "DB size prev: %d post %d\n", presize, postsize );
	  //}
	  
      RenderObject( old ); // undraw it
      RenderObject( truth ); // redraw it

      XFlush( display );// reduces flicker

      truth_map[ stage_id ] = truth; // update the database with it

      //puts( "RESULT: " );
      //for( TruthMap::iterator it = truth_map.begin();
      //	   it != truth_map.end(); it++ )
      //{
      //  cout << it->first << " -- ";
      //  PrintMetricTruth( it->second );
      //}
  
      //puts( "" );
    }

  //pthread_mutex_unlock( &incoming_mutex );
}

CXGui::CXGui( int argc, char** argv, environment_t* anenv )
{
#ifdef DEBUG
  cout << "Creating a window..." << flush;
#endif

  dragging = NULL;
  
  env = anenv;

  window_title = new char[256];

  draw_all_devices = false;

  sprintf( window_title,  "%s v.%s", titleStr, versionStr );


//  LoadVars( initFile ); // read the window parameters from the initfile

//    // init X globals 
    display = XOpenDisplay( (char*)NULL );
    screen = DefaultScreen( display );

    assert( display );

    default_cmap = DefaultColormap( display, screen ); 

    XColor col, rcol;
    XAllocNamedColor( display, default_cmap, "red", &col, &rcol ); 
    red = col.pixel;
    XAllocNamedColor( display, default_cmap, "green", &col, &rcol ); 
    green= col.pixel;
    XAllocNamedColor( display, default_cmap, "blue", &col, &rcol ); 
    blue = col.pixel;
    XAllocNamedColor( display, default_cmap, "cyan", &col, &rcol ); 
    cyan = col.pixel;
    XAllocNamedColor( display, default_cmap, "magenta", &col, &rcol ); 
    magenta = col.pixel;
    XAllocNamedColor( display, default_cmap, "yellow", &col, &rcol ); 
    yellow = col.pixel;
  
    black = BlackPixel( display, screen );
    white = WhitePixel( display, screen );
   
    XTextProperty windowName;
        
    
    // load the X resources here
    
    if( !geometry ) geometry = XGetDefault( display, argv[0], "geometry" ); 
    if( !geometry ) geometry = default_geometry;
    SetupGeometry( geometry );
    
    if( !channels ) channels = XGetDefault( display, argv[0], "channels" );
    if( !channels )
      {
	channels = new char[ strlen( default_channels ) ];
	strcpy( channels, default_channels );
      }
    SetupChannels( channels );
    
    if( !zoom ) zoom = XGetDefault( display, argv[0], "zoom" );
    if( !zoom ) zoom= default_zoom;
    SetupZoom( zoom );
    
    if( !pan ) pan = XGetDefault( display, argv[0], "pan" );
    if( !pan ) pan= default_pan;
    SetupPan( pan ); // must setupzoom first!
    


    win = XCreateSimpleWindow( display, 
  			     RootWindow( display, screen ), 
  			     x, y, width, height, 4, 
  			     black, white );
  
    XSelectInput(display, win, ExposureMask | StructureNotifyMask | 
  	       ButtonPressMask | ButtonReleaseMask | KeyPressMask );
  
    XStringListToTextProperty( &window_title, 1, &windowName);

    XSizeHints sz;

    sz.flags = PMinSize;
    sz.min_width = MIN_WINDOW_WIDTH; 
    sz.min_height = MIN_WINDOW_HEIGHT;
      
    XSetWMProperties(display, win, &windowName, (XTextProperty*)NULL, 
  		   (char**)NULL, 0, 
  		   &sz, (XWMHints*)NULL, (XClassHint*)NULL );
  
    gc = XCreateGC(display, win, (ULI)0, NULL );
  

    BoundsCheck();

    XMapWindow(display, win);
  
    // where did the WM put us?
    XWindowAttributes attrib;
    XGetWindowAttributes( display, win, &attrib );
    
    x = attrib.x;
    y = attrib.y;
    width = attrib.width;
    height = attrib.height;

    switch( attrib.depth )
      {
      case 32: // no problem
	break;

      case 24: // no problem
	break;
	
      case 16:
	//puts( "\nXS: Warning: color depth is 16 bits."
	//    " Using restricted color palette.\n" );
	break;
	
      default:	
	printf( "\nXS: Warning: color depth is %d bits. Colors _may_ not be "
		"drawn correctly\n", attrib.depth );
      }

#ifdef DEBUG
    cout << " Window coords: " << x << '+' << y << ':' 
	 << width << 'x' << height << " ok." << endl;
#endif
  }


CXGui::~CXGui( void )
{
  // empty destructor
}


//  int CXGui::LoadVars( char* filename )
//  {
  
//  #ifdef DEBUG
//    cout << "Loading window parameters from " << filename << "... " << flush;
//  #endif

//    ifstream init( filename );
//    char buffer[255];
//    char c;

//    if( !init )
//      {
//        cout << "FAILED" << endl;
//        return -1;
//      }
  
//    char token[50];
//    char value[200];
  
//    while( !init.eof() )
//      {
//        init.get( buffer, 255, '\n' );
//        init.get( c );
    
//        sscanf( buffer, "%s = %s", token, value );
      
//  #ifdef DEBUG      
//        printf( "token: %s value: %s.\n", token, value );
//        fflush( stdout );
//  #endif
      
//        if ( strcmp( token, "window_x_pos" ) == 0 )
//  	x = strtol( value, NULL, 10 );
//        else if ( strcmp( token, "window_y_pos" ) == 0 )
//  	y = strtol( value, NULL, 10 );
//        else if ( strcmp( token, "window_width" ) == 0 )
//  	width = strtol( value, NULL, 10 );
//        else if ( strcmp( token, "window_height" ) == 0 )
//  	height = strtol( value, NULL, 10 );
//        else if ( (strcmp( token, "window_xscroll" ) == 0) 
//  		|| (strcmp( token, "window_x_scroll" ) == 0) )
//  	panx = strtol( value, NULL, 10 );
//        else if ( (strcmp( token, "window_yscroll" ) == 0 )
//  		|| (strcmp( token, "window_y_scroll" ) == 0) )
//  	pany = strtol( value, NULL, 10 );
//      }

//  #ifdef DEBUG
//    cout << "ok." << endl;
//  #endif
//    return 1;
//  }


void CXGui::MoveObject( truth_t* exp, double x, double y, double th )
{    
  // compose the structure
  stage_truth_t output;

  output.stage_id = exp->stage_id;
  output.stage_type = exp->stage_type;

  output.channel = exp->channel;

  output.id.port = exp->id.port;
  output.id.type = exp->id.type;
  output.id.index = exp->id.index;

  output.parent.port = exp->parent.port;
  output.parent.type = exp->parent.type;
  output.parent.index = exp->parent.index;

  output.x = (uint32_t)(x * 1000.0); // the new pose
  output.y = (uint32_t)(y * 1000.0); // "
  output.th = (uint16_t)RTOD( th );  // "
  output.w = (uint32_t)(exp->w * 1000.0);
  output.h = (uint32_t)(exp->h * 1000.0);
  output.rotdx = (uint16_t)(exp->rotdx * 1000.0);
  output.rotdy = (uint16_t)(exp->rotdy * 1000.0);

  // and queue it up for the server thread to write out
  //pthread_mutex_lock( &outgoing_mutex );
  outgoing_queue.push( output );
  //pthread_mutex_unlock( &outgoing_mutex );

  dragging->x = x;
  dragging->y = y;
  dragging->th = th;


  //#ifdef DEBUG
  //printf( "XS: Moving object "
  //  "(%d,%d,%d) [%.2f,%.2f,%.2f]\n", 
  //  dragging->id.port, 
  //  dragging->id.type, 
  //  dragging->id.index,
  //  dragging->x, dragging->y, dragging->th );
   //#endif
}


void CXGui::DrawBackground( void ) 
{ 
  SetDrawMode( GXcopy );

  XSetForeground( display, gc, black );
  XFillRectangle( display, win, gc, 0,0, width, height );

  XSetForeground( display, gc, white ); 

  // if the background blocks are > 1 pixel, i'll draw them as boxes
  //double ratio = (double)iwidth/(double)env->width ;
  double ratio = ppm/env->ppm;

  if( ratio > 1.0 )
    {
      for( int i=0; i<env->num_pixels; i++ )
	XDrawRectangle( display,win,gc,
			env->pixels_scaled[i].x,
			env->pixels_scaled[i].y,
			(unsigned int)ratio, (unsigned int)ratio );
    }
  else
    XDrawPoints( display,win,gc,
		 env->pixels_scaled, env->num_pixels, 
		 CoordModeOrigin);

  // draw a scale grid over the world
  //SetForeground(  );
}

void CXGui::BoundsCheck( void )
{
  //  printf( "PRE  PAN %d, %d \twin %d, %d \timage %d, %d\n", 
  //  panx, pany, width, height, iwidth, iheight ); 

  if( panx + width > iwidth ) panx =  iwidth - width;
  if( panx < 0 ) panx = 0;

  if( pany + height > iheight ) pany =  iheight - height;
  if( pany < 0 ) pany = 0;

  //printf( "POST PAN (%d, %d) \twin %d, %d \timage %d, %d\n", 
  //  panx, pany, width, height, iwidth, iheight ); 
}

void CXGui::CalcPPM( void )
{
  double ppmw = ((double)iwidth / (double)env->width )*env->ppm; 
  double ppmh = ((double)iheight / (double)env->height )*env->ppm; 

  if( ppmw > ppmh )
    ppm =  ppmw;
  else
    ppm = ppmh;

  ScaleBackground();
}

void CXGui::HandleXEvent()
{    
  XEvent reportEvent;
  
  while( XCheckWindowEvent( display, win,
  			    StructureNotifyMask 
  			    | ButtonPressMask | ButtonReleaseMask 
  			    | ExposureMask | KeyPressMask 
  			    | PointerMotionHintMask, 
  			    &reportEvent ) )
    
    switch( reportEvent.type ) // there was an event waiting, so handle it...
      {
      case ConfigureNotify:  // window resized or modified
	HandleConfigureEvent( reportEvent ); 
	break;
	
      case Expose: // window area exposed - needs redrawn
	HandleExposeEvent( reportEvent );
  	break;
	
      case MotionNotify: // mouse pointer moved
  	HandleMotionEvent( reportEvent );
  	break;
	
      case ButtonRelease: // mouse button released
  	break;
	
      case ButtonPress: // mouse button pressed
	HandleButtonPressEvent( reportEvent );
	break;	
	
      case KeyPress: // guess what
	HandleKeyPressEvent( reportEvent );
  	break;
      }
  
  // try to sync the display to avoid flicker
  //XSync( display, false );
}

void CXGui::SetDrawMode( int mode )
{
  // the modes are defined in X11/Xlib.h
  XSetFunction( display, gc, mode );
}

 void CXGui::DrawPolygon( DPoint* pts, int numPts )
   {
     DPoint end[2];
     
     memcpy( &(end[0]), &(pts[0]), sizeof( DPoint ) );
     memcpy( &(end[1]), &(pts[numPts-1]), sizeof( DPoint ) );
     
     DrawLines( pts, numPts ); // draw the lines
     DrawLines( end, 2 ); // close the end
   }
 
 
void CXGui::DrawString( double x, double y, char* str, int len )
{ 
  if( str && strlen( str ) > 0 )
    {
      XDrawString( display,win,gc, 
  		   (ULI)(x * ppm) -panx,
  		   (ULI)( (iheight-pany)-((int)( y*ppm ))),
  		   str, len );
    }
  else
    cout << "Warning: XGUI::DrawString was passed a null pointer" << endl;
}

void CXGui::DrawLines( DPoint* dpts, int numPts )
{
  XPoint* xpts = new XPoint[numPts];
  
  // scale, pan and quantize the points
  for( int l=0; l<numPts; l++ )
    {
      xpts[l].x = (int)( dpts[l].x * ppm ) - panx;
      xpts[l].y = (iheight-pany) - ((int)( dpts[l].y * ppm ));
    }
  
  // and draw them
  XDrawLines( display, win, gc, xpts, numPts, CoordModeOrigin );
     
  delete [] xpts; 
}

void CXGui::DrawLine( DPoint& a, DPoint& b )
{
  XPoint p, q;
  
  // scale, pan and quantize the points
  
  p.x = (int)( a.x * ppm ) - panx;
  p.y = (iheight-pany) - ((int)( a.y * ppm ));
  q.x = (int)( b.x * ppm ) - panx;
  q.y = (iheight-pany) - ((int)( b.y * ppm ));
  
  // and draw the line
  XDrawLine( display, win, gc, p.x, p.y, q.x, q.y );
}


void CXGui::DrawPoints( DPoint* dpts, int numPts )
{
  XPoint* xpts = new XPoint[ numPts ];
  
  // scale, pan and quantize the points
  for( int l=0; l<numPts; l++ )
    {
      xpts[l].x = ((int)( dpts[l].x * (double)ppm )) - panx;
      xpts[l].y = (iheight-pany) - ((int)( dpts[l].y * (double)ppm ));
    }
  
  // and draw them
  XDrawPoints( display, win, gc, xpts, numPts, CoordModeOrigin );
    
  delete [] xpts; 
}

void CXGui::DrawCircle( double x, double y, double r )
{
    XDrawArc( display, win, gc, 
      (ULI)((x-r) * ppm -panx), 
      (ULI)((iheight-pany)-((y+r) * ppm)), 
      (ULI)(2.0*r*ppm), (ULI)(2.0*r*ppm), 0, 23040 );
}

void CXGui::FillCircle( double x, double y, double r )
{
    XFillArc( display, win, gc, 
      (ULI)((x-r) * ppm -panx), 
      (ULI)((iheight-pany)-((y+r) * ppm)), 
      (ULI)(2.0*r*ppm), (ULI)(2.0*r*ppm), 0, 23040 );
}

void CXGui::DrawArc( double x, double y, double w, double h, double th1, double th2 )
{
    XDrawArc( display, win, gc, 
      (ULI)((x-w/2) * ppm -panx), 
      (ULI)((iheight-pany)-((y+h/2) * ppm)), 
      (ULI)(w*ppm), (ULI)(h*ppm), (int)RTOD(th1)*64, (int)RTOD(th2)*64 );
}

void CXGui::DrawNoseBox( double x, double y, double w, double h, double th )
{
  DPoint pts[4];
  GetRect( x,y,w,h,th, pts );
  DrawPolygon( pts, 4 );

  DrawNose( x, y, w, th );
}

void CXGui::DrawRect( double x, double y, double w, double h, double th )
{
  DPoint pts[4];
  GetRect( x,y,w,h,th, pts );
  DrawPolygon( pts, 4 );
}


void CXGui::DrawNose( double x, double y, double l, double th  )
{
  DPoint pts[2];

  pts[0].x = x;
  pts[0].y = y;

  pts[1].x = x + cos( th ) * l;
  pts[1].y = y + sin( th ) * l;
  
  DrawLine( pts[0], pts[1] );
}


void CXGui::RefreshObjects( void )
{
  SetDrawMode( GXxor );

  for( TruthMap::iterator it = truth_map.begin();
       it != truth_map.end(); it++ )
    RenderObject( it->second );

  HighlightObject( dragging, false );

  SetDrawMode( GXcopy );
}

truth_t* CXGui::NearestEntity( double x, double y )
{
  //puts( "NEAREST" ); fflush( stdout );

  double dist, nearDist = 99999999.9;
  truth_t* close = 0;

  for( TruthMap::iterator it = truth_map.begin(); it != truth_map.end(); it++ )
    {
      //cout << "inspecting " << it->first << endl;
      
      dist = hypot( x - it->second.x, y - it->second.y );
     
      //printf( " dist: %.2f\n", dist );
      //fflush( stdout );
 
      if( dist < nearDist && it->second.parent.type == 0 ) 
    	{
    	  nearDist = dist;
    	  close = &(it->second); // the id of the closest entity so far
    	}
    }

  // printf( "XS: Nearest is (%d,%d,%d) at %.2f m",
  //close->id.port, close->id.type, close->id.index, nearDist );

  fflush( stdout );
  
  //cout << "inspecting " << it->first << endl;
   
  return close;
}


void CXGui::SetForeground( unsigned long col )
{ 
  XSetForeground( display, gc, col );
}



void CXGui::Move( void )
{
  XMoveWindow( display, win, x,y );
}

void CXGui::Size( void )
{
  XResizeWindow( display, win, width, height );
}

istream& operator>>(istream& s, CXGui& w )
{
  s >> w.x >> w.y >> w.width >> w.height;
  w.Move();
  w.Size();

  return s;
}

void CXGui::DrawBox( double px, double py, double boxdelta )
  {
      DPoint pts[5];
  
      pts[4].x = pts[0].x = px - boxdelta;
      pts[4].y = pts[0].y = py + boxdelta;
      pts[1].x = px + boxdelta;
      pts[1].y = py + boxdelta;
      pts[2].x = px + boxdelta;
      pts[2].y = py - boxdelta;
      pts[3].x = px - boxdelta;
      pts[3].y = py - boxdelta;
  
      DrawLines( pts, 5 );
  } 


void CXGui::GetRect( double x, double y, double dx, double dy, 
		     double rotateAngle, DPoint* pts )
{  
  double cosa = cos( rotateAngle );
  double sina = sin( rotateAngle );
  double dxcosa = dx * cosa;
  double dycosa = dy * cosa;
  double dxsina = dx * sina;
  double dysina = dy * sina;
  
  pts[2].x = x -dxcosa + dysina;
  pts[2].y = y -dxsina - dycosa;
  pts[3].x = x +dxcosa + dysina;
  pts[3].y = y +dxsina - dycosa;
  pts[1].x = x -dxcosa - dysina;
  pts[1].y = y -dxsina + dycosa;
  pts[0].x = x +dxcosa - dysina;
  pts[0].y = y +dxsina + dycosa;

  //printf( "a: %.2f %.2f\n", rotateAngle, RTOD( rotateAngle ) );
  //DrawCircle( pts[0].x, pts[0].y, 0.05 );
}


void CXGui::HighlightObject( truth_t* exp,  bool undraw )
{
  static double x = -1000.0, y = -1000.0, r = -1000.0, th = -1000.0;
  static char info[256];
  static DPoint heading_stick_pts[2];
      
  // setup the GC 
  SetForeground( white );
  SetDrawMode( GXxor );
  
  int infox = 5;
  int infoy = 15;
  
  // undraw the last stuff if there was any
  if( x != -1000.0 && y != -1000.0 && undraw )
    {
      XSetLineAttributes( display, gc, 0, LineOnOffDash, CapRound, JoinRound );
      DrawCircle( x, y, r );

      XSetLineAttributes( display, gc, 0, LineSolid, CapRound, JoinRound );
      DrawLines( heading_stick_pts, 2 );

      if( info[0] ) 	  
	XDrawString( display,win,gc,
		     infox, infoy, info, strlen(info) );
    }
  
  if( exp ) // if this is a valid object
    {
      x = exp->x;
      y = exp->y;
      r = max( exp->w, exp->h) * 0.8; // a circle a little larger than the object's largest dimension
      th = exp->th;
      
      XSetLineAttributes( display, gc, 0, LineOnOffDash, CapRound, JoinRound );
      DrawCircle( x,y, r ); // and highlight it
      
      double dx1 = r * cos( exp->th );
      double dy1 = r * sin( exp->th );
      
      double dx2 = r/2.0 * cos( exp->th );
      double dy2 = r/2.0 * sin( exp->th );
      
      heading_stick_pts[0].x = x + dx1;
      heading_stick_pts[0].y = y + dy1;
      
      heading_stick_pts[1].x = heading_stick_pts[0].x + dx2;
      heading_stick_pts[1].y = heading_stick_pts[0].y + dy2;
      
      XSetLineAttributes( display, gc, 0, LineSolid, CapRound, JoinRound );
      DrawLines( heading_stick_pts, 2 );
   

      sprintf( info, "%s (%.2f,%.2f,%d)",
	       StageNameOf(*exp),
	       exp->x, exp->y, (int)RTOD( exp->th ) );

      if( exp->id.port ) // ie. it's a player device
	{ // append the player id 
	  char buf[256];
	  strncpy( buf, info, sizeof(buf) );
	  
	  sprintf( info, "%s(%d,%s,%d)",
		   buf,
		   exp->id.port, PlayerNameOf(exp->id), exp->id.index );
	}

      if( exp->channel != -1 ) // ie. it is visible in an ACTS channel
	{
	  // append the channel num
	  char buf[256];
	  strncpy( buf, info, sizeof(buf) );
	  
	  sprintf( info, "%s(ACTS:%d)",
		   buf, exp->channel );
	}
      
      XDrawString( display,win,gc,
		   infox, infoy, info, strlen( info ) );
    }
  
  // reset the GC
  SetDrawMode( GXcopy );
}

void CXGui::ScaleBackground( void )
{
  double ratio = ppm / env->ppm;

  for( int i=0; i<env->num_pixels; i++ )
    {
      env->pixels_scaled[i].x = (short)(env->pixels[i].x * ratio) - panx;
      env->pixels_scaled[i].y = (short)(env->pixels[i].y * ratio) - pany;
    }
}

void CXGui::HandleCursorKeys( KeySym &key )
{
  int oldPanx = panx;
  int oldPany = pany;
  
  if( key == XK_Left )
    panx -= width/5;
  else if( key == XK_Right )
    panx += width/5;
  else if( key == XK_Up )
    pany -= height/5;
  else if( key == XK_Down )
    pany += height/5;
  
  BoundsCheck();
  
  // if the window has been panned, refresh everything
  if( panx != oldPanx || pany != oldPany )
    {
      CalcPPM();
      DrawBackground();
      RefreshObjects();
    }
}

void CXGui::Zoom( double ratio )
{
  iwidth = (int)( iwidth * ratio );
  iheight = (int)( iheight * ratio );
  
  //if( iwidth < width ) width = iwidth;
  //if( iheight < height ) height = iheight;
  //Size();

  CalcPPM();
  
  DrawBackground(); // black backround and draw walls
  RefreshObjects(); 
}

void CXGui::StartDragging( XEvent& reportEvent )
{

  // create an invisible mouse pointer
   Pixmap blank;
   XColor dummy;
   char data[1] = {0};
   Cursor blank_cursor;

   blank = XCreateBitmapFromData(display,win, data, 1, 1);
   
   blank_cursor 
     = XCreatePixmapCursor(display,blank,blank, &dummy, &dummy, 0, 0);
   XFreePixmap(display, blank);
   
   // retain the pointer while we're dragging, and make it invisible
   XGrabPointer( display, win, true, 
		 ButtonReleaseMask | ButtonPressMask | PointerMotionMask, 
		 GrabModeAsync, GrabModeAsync, win, blank_cursor, CurrentTime);
   
  // make a new local device for dragging around
  dragging = new truth_t();
  
    
  truth_t* nearest = NearestEntity( (reportEvent.xmotion.x+panx)/ppm,
			 ((iheight-pany)/ppm)-reportEvent.xmotion.y/ppm );
  // copy the details of the nearest device
  if(!nearest)
  {
    dragging = NULL;
    XUngrabPointer( display, CurrentTime );
    return;
  }
  memcpy( dragging,  nearest, sizeof( truth_t ) );
  
  //assert( dragging );
  
  //printf( "XS: Dragging (%d,%d,%d)",
  //dragging->id.port, dragging->id.type, dragging->id.index );
  //fflush( stdout );

  HighlightObject( dragging, false );
}
  

void CXGui::StopDragging( XEvent& reportEvent )
{
  XUngrabPointer( display, CurrentTime );
 
  // we're done with the dragging object now
  delete dragging;
  dragging = NULL; // we test the value elsewhere
  
  HighlightObject( NULL, true ); // erases the highlighting
} 

void CXGui::HandleButtonPressEvent( XEvent& reportEvent )
{
  switch( reportEvent.xbutton.button )
    {	      
    case Button1:  //cout << "BUTTON 1" << endl;
      if( dragging  ) 
	StopDragging( reportEvent );
      else 
	StartDragging( reportEvent );
      break;
      
    case Button2: //cout << "BUTTON 2" << endl;
      if( dragging )
	MoveObject( dragging,dragging->x, dragging->y, 
		    NORMALIZE(dragging->th + M_PI/10.0) );
	      break;
    case Button3: //cout << "BUTTON 3" << endl;
      if( dragging )
	{  			  
	  MoveObject( dragging, dragging->x, dragging->y, 
			      NORMALIZE(dragging->th - M_PI/10.0) );
	}	      
      break;
    }

  if( dragging ) HighlightObject( dragging, true );
}

void CXGui::HandleKeyPressEvent( XEvent& reportEvent )
{  
  KeySym key = XLookupKeysym( &reportEvent.xkey, 0 );
  
  if( IsCursorKey( key ) ) 
    HandleCursorKeys( key );

  // handle all the non-cursor keys here
  
  
  if( key == XK_v )
    {
      for( TruthMap::iterator it = truth_map.begin();
	   it != truth_map.end(); it++ )
	RenderObject( it->second );
      
      draw_all_devices = !draw_all_devices;

      for( TruthMap::iterator it = truth_map.begin();
	   it != truth_map.end(); it++ )
	RenderObject( it->second );
    }

  if( key == XK_q || key == XK_Q )
    {
      cout << "QUIT" << endl;
      exit( 0 );
    }
  
  if( key == XK_z || key == XK_Z )
    {
      //cout << "ZOOM IN" << endl;
      Zoom( 1.2 );
    }
  
  if( key == XK_x || key == XK_X )
    {
      //cout << "ZOOM OUT" << endl;
      Zoom( 0.8 );
    }

  if( key == XK_d )
    {
      // list the object database on stdout - handy for debug and status
     
      printf( "XS device list (%d devices):\n", truth_map.size() );

      int c=0;
      for( TruthMap::iterator it = truth_map.begin();
	   it != truth_map.end(); it++ )
	{
	  printf( "%4d: ", c++ );
	  PrintMetricTruth( it->first, it->second );
	}
    }

  if( key == XK_f )
    {
      // list the object database on stdout - handy for debug and status
     
      printf( "XS verbose device list (%d devices):\n", truth_map.size() );

      int c=0;
      for( TruthMap::iterator it = truth_map.begin();
	   it != truth_map.end(); it++ )
	{
	  // ugly to mix stdio and iostream, but it works OK if you flush.
	  printf( "%4d: key: ", c++ ); fflush( stdout );
	  cout << it->first << flush;
	  PrintMetricTruthVerbose( it->first, it->second );
	}
    }

  if( key == XK_p || key == XK_P )
    {
      // dump a screenshot in EPS format
      static int epscounter = 0;
      char outname[128];

      sprintf( outname, "xpsi_screenshot.%d.eps", epscounter++ );
      
      printf( "DUMPING SCREENSHOT to %s\n", outname );
      fflush( stdout );

      int pid;

      if(( pid = fork()) == 0) // fork off an `import' process
	  execlp( "import", "import", "+negate", 
		  "-w", window_title, outname, NULL );
    }

  if( key == XK_j || key == XK_J )
    {
      // dump a screenshot in JPEG format
      static int epscounter = 0;
      char outname[128];

      sprintf( outname, "xpsi_screenshot.%d.jpg", epscounter++ );
      
      printf( "SCREENSHOT: %s\n", outname );
      fflush( stdout );

      int pid;

      if(( pid = fork()) == 0) // fork off an `import' process
	  execlp( "import", "import", "-w", window_title, outname, NULL );
    }

  if( key == XK_s )
    {
      // send a save command to Stage

      // compose the structure
      stage_truth_t output;

      memset( &output, 0, sizeof( output ) );

      output.stage_id = 0; // this indicates a command

      output.x = (uint32_t) 1;
      
      // and queue it up for the server thread to write out
      //pthread_mutex_lock( &outgoing_mutex );
      outgoing_queue.push( output );
      //pthread_mutex_unlock( &outgoing_mutex );
    }

  if( key == XK_l || key == XK_L )
    {
      //cout << "LOAD" << endl;
      //world->Load();
      //RefreshObjects();
    }

//    if( key == XK_g || key == XK_G )
//      {
      
//        XSetForeground( display, gc, white );
//        XSetLineAttributes( display, gc, 0, LineOnOffDash, CapRound, JoinRound );
      
//        DPoint pts[2];
//        for( int i=0; i * ppm  < width ; i++ )
//  	{
//  	  pts[0].x = 0;
//  	  pts[0].y = i;
	  
//  	  pts[1].x = width;
//  	  pts[1].y = i; 
	  
//  	  DrawLine( pts[0], pts[1] );
	  
//  	  pts[0].y = 0;
//  	  pts[0].x = i;
	  
//  	  pts[1].y = width;
//  	  pts[1].x = i; 
	  
//  	  DrawLine( pts[0], pts[1] );
//  	}
  
//        XSetLineAttributes( display, gc, 0, LineSolid, CapRound, JoinRound );
//      }

}  

void CXGui::HandleExposeEvent( XEvent &reportEvent )
{  
  if( reportEvent.xexpose.count == 0 ) // on the last pending expose...
    {
      CalcPPM();
      DrawBackground(); // black backround and draw walls
      RefreshObjects();
    }
}

void CXGui::HandleMotionEvent( XEvent &reportEvent )
{
  if( dragging )
    {
      int xpixel = (reportEvent.xmotion.x+panx);
      int ypixel = ((iheight-pany)-reportEvent.xmotion.y);

      if( xpixel > iwidth  ) xpixel = iwidth;
      if( ypixel < 0 ) ypixel = 0;

      double xpos = xpixel / ppm;
      double ypos = ypixel / ppm; 
      
      //printf( "motion: %d %d \t %.2f %.2f \n", xpixel, ypixel, xpos, ypos );

      MoveObject( dragging, xpos, ypos, dragging->th );
      
      HighlightObject( dragging, true );
    }
}

void CXGui::HandleConfigureEvent( XEvent &reportEvent )
{
  //cout << "CONFIGURE NOTIFY" << endl;
  
  //if( width != reportEvent.xconfigure.width ||
  //  height != reportEvent.xconfigure.height )
  //{
  x = reportEvent.xconfigure.x;
  y = reportEvent.xconfigure.y;
  width = reportEvent.xconfigure.width;
  height = reportEvent.xconfigure.height;
  
  
  CalcPPM();
  
  DrawBackground(); // black backround and draw walls
  RefreshObjects(); 
  
  //printf( "w: %dx%d i: %dx%d ppm: %.2f px: %d py: %d\n",
  //  width, height, iwidth, iheight, ppm, panx, pany );
  //fflush( stdout );
} 
  
//  // hide the CRobot reference
//  //  unsigned long CXGui::RobotDrawColor( CRobot* r )
//  //  { 
//  //    unsigned long color;
//  //    switch( r->channel % 6 )
//  //      {
//  //      case 0: color = red; break;
//  //      case 1: color = green; break;
//  //      case 2: color = blue; break;
//  //      case 3: color = yellow; break;
//  //      case 4: color = magenta; break;
//  //      case 5: color = cyan; break;
//  //      }

//  //    return color;
//  //  }

//  // hide the CRobot reference
//  //  void CXGui::DrawInRobotColor( CRobot* r )
//  //  { 
//  //    XSetForeground( display, gc, RobotDrawColor( r ) );
//  //  }



#include "server.hh"

#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif
#if HAVE_SYS_TYPES_H
  #include <sys/types.h>
#endif

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
//#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

//#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

#define DEBUG
#define VERBOSE
//#undef DEBUG

//#include "world.hh"
//extern CWorld* world;
void PrintUsage(); // defined in main.cc

void ClientCatchSigPipe( int signo )
{
#ifdef VERBOSE
  cout << "** SIGPIPE! **" << endl;
#endif

}

CStageClient::CStageClient( int argc, char** argv, Library* lib )
  : CStageIO( argc, argv, lib )
{
  // start paused 
  m_enable = false;

  // parse out the hostname - that's all we need just here
  // (the parent stageio object gets the port)
  for( int a=1; a<argc; a++ )
  {
    // get the  number, overriding the default
    if( strcmp( argv[a], "-c" ) == 0 )
    {
      // if -c is the last argument the cmd line is bad
      if( a == argc-1 )
	{
	  PrintUsage();
	  exit(0); // bail
	}
     
      // the next argument is the hostname
      strcpy( m_remotehost, argv[a+1]);
      
      a++;
    }

    else if( strcmp( argv[a], "-cl" ) == 0 )
    {
      // -cl means client of localhost
      strcpy( m_remotehost, "localhost" );
      
      a++;
    }

  }

  // reassuring console output
  printf( "[Connecting to %s:%d]", m_remotehost, m_port );
  puts( "" );

  // clients don't use a device directory - nullify the string
  m_device_dir[0] = 0;


  // connect to the remote server and download the world data

  // get the IP of our host
  struct hostent* info = gethostbyname( m_remotehost );
  
  if( info )
  { // make sure this looks like a regular internet address
    assert( info->h_length == 4 );
    assert( info->h_addrtype == AF_INET );
  }
  else
  {
    PRINT_ERR1( "failed to resolve IP for remote host\"%s\"\n", 
                m_remotehost );
  }
  struct sockaddr_in servaddr;

  /* open socket for network I/O */
  m_pose_connections[0].fd = socket(AF_INET, SOCK_STREAM, 0);
  m_pose_connections[0].events = POLLIN; // notify me when data is available
  
  //  printf( "POLLFD = %d\n", m_pose_connections[0].fd );

  if( m_pose_connections[0].fd < 0 )
  {
    printf( "Error opening network socket. Exiting\n" );
    fflush( stdout );
    exit( -1 );
  }
  
  /* setup our server address (type, IP address and port) */
  bzero(&servaddr, sizeof(servaddr)); /* initialize */
  servaddr.sin_family = AF_INET;   /* internet address space */
  servaddr.sin_port = htons( m_port ); /*our command port */ 
  memcpy(&(servaddr.sin_addr), info->h_addr_list[0], info->h_length);
  
  if( connect( m_pose_connections[0].fd, 
               (sockaddr*)&servaddr, sizeof( servaddr) ) == -1 )
  {
    printf( "Can't find a Stage server on %s. Quitting.\n", 
            info->h_addr_list[0] ); 
    perror( "" );
    fflush( stdout );
    exit( -1 );
  }
  // send the connection type byte - we want an asynchronous connection
  
  char c = STAGE_ASYNC;
  int r;
  if( (r = write( m_pose_connections[0].fd, &c, 1 )) < 1 )
  {
    printf( "XS: failed to write ASYNC byte to Stage. Quitting\n" );
    if( r < 0 ) perror( "error on write" );
    exit( -1 );
  }

  m_pose_connection_count = 1; // made a connection
  m_dirty_subscribe[0] = 1; // this connection needs dirty updates

  // request a dump of the stage world
  WriteCommand( DOWNLOADc );
  
  m_downloading = true;

  // read will set the flag when it receives the "done" packet from the server.
  // read will create any objects that come down the wire
  while( m_downloading )
    Read();

  // now we have parameters for some other objects
  //assert( matrix = new CMatrix( 500, 500, 10) );

  // Test to see if the required things were created...
  // TODO: MORE CHECKS HERE
  assert( matrix ); // make sure a matrix was created in Read();
  assert( wall );
  
  // now we have the world parameters, we can configure things
  
#ifdef INCLUDE_RTK2
  RtkLoad(NULL); // uses default values for now
#endif
  
  this->wall->Startup(); // renders the image into the matrix
  
  // Startup all the objects
  // Devices will create and initialize their device files
  for (int i = 0; i < GetEntityCount(); i++)
  {
    CEntity* ent = GetEntity(i);
    
    assert( ent );

    if( !ent->Startup() )
    {
      PRINT_ERR("object startup failed");
      quit = true;
      return;// false;
    }
  }

  // now we've set everything up, we request updates as the world changes 
  // (we MUST have downloaded and started everything before subscribing)
  WriteCommand( SUBSCRIBEc );
} 



CStageClient::~CStageClient( void )
{
  // do nothing
}


// Save the world file
bool CStageClient::SaveFile( char* filename )
{
  // send a message to the server to save the world file.
  WriteCommand( SAVEc );
  return false;
}


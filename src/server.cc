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
 * Desc: This class implements the server, or main, instance of Stage.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 6 Jun 2002
 * CVS info: $Id: server.cc,v 1.15 2002-06-10 17:14:30 inspectorg Exp $
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iomanip.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strstream.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

//#define DEBUG
//#define VERBOSE

#include "server.hh"
#include "fixedobstacle.hh"

extern long int g_bytes_output;
extern long int g_bytes_input;

const int LISTENQ = 128;
//const long int MILLION = 1000000L;

void CatchSigPipe( int signo )
{
#ifdef VERBOSE
  puts( "** SIGPIPE! **" );
#endif
  exit( -1 );
}


CStageServer::CStageServer( int argc, char** argv ) 
  : CStageIO( argc, argv )
{ 
  // enable player services by default, the command lines may change this
  m_run_player = true;    

  //  one of our parent's constructors may have failed and set this flag
  if( quit ) return;
  
  ///////////////////////////////////////////////////////////////////////
  // Set and load the worldfile, creating entitys as we go
  if( !LoadFile(  argv[argc-1] ) )
  {
    quit = true;
    return;
  }
  
  // reassuring console output
  printf( "[World %s]", this->worldfilename );
  
  ///////////////////////////////////////////////////////////////////////
  // LOAD THE CONFIGURATION FOR THE GUI 
  // do this *after* the world has loaded, so we can configure the menus
  // correctly
  printf("\n");
  if (!RtkLoad(&this->worldfile))
  {
    PRINT_ERR( "Failed to load worldfile" );
    quit = true;
    return;
  }

  // See if there was anything we didnt understand in the world file
  this->worldfile.WarnUnused();

  /////////////////////////////////////////////////////////////////////////
  // COMMAND LINE PARSING - may override world file options
  if( !ParseCmdLine( argc, argv ) )
  {
    PRINT_ERR( "Failed to parse command line" );
    quit = true;
    return;
  }
  
  ///////////////////////////////////////////////////////////////////////////
  // Create the device directory, clock and lock devices 

  if( !CreateDeviceDirectory() )
  {
    PRINT_ERR( "Failed to create device directory" );
    quit = true;
    return;
  }

  if( !CreateClockDevice() )
  {
    PRINT_ERR( "Failed to create clock device" );
    quit = true;
    return;
  }
  
  if( !CreateLockFile() )
  {
    PRINT_ERR( "Failed to create lock file" );
    quit = true;
    return;
  }
  
  ///////////////////////////////////////////////////////////////////////
  // STARTUP
  
  // Initialise the wall entity
  if( !this->wall->Startup())
  {
    PRINT_ERR("Background startup failed");
    quit = true;
    return;
  }

  // Startup all the entities
  // Devices will create and initialize their device files
  for (int i = 0; i < GetEntityCount(); i++)
  {
    if( !GetEntity(i)->Startup() )
    {
      PRINT_ERR1("Entity %d  startup failed", i);
      quit = true;
      return;
    }
  }

  //////////////////////////////////////////////////////////////////////
  // SET UP THE SERVER TO ACCEPT INCOMING CONNECTIONS
  if( !SetupConnectionServer() )
  {
    quit = true;
    return;
  }

  // just to be reassuring, print the host details
  printf( "[Server %s:%d]",  m_hostname, m_port );
  puts( "" ); // end the startup line, flush stdout before starting player

  ////////////////////////////////////////////////////////////////////
  // STARTUP PLAYER
  if( m_run_player && !StartupPlayer() )
  {
    PRINT_ERR("Player startup failed");
    quit = true;
    return;
  }
  
}

CStageServer::~CStageServer( void )
{
  // do nothing
}


//////////////////////////////////////////////////////////////////////
// Load the world file and create all of the relevant entities.
bool CStageServer::LoadFile( char* filename )
{
  //////////////////////////////////////////////////////////////////////
  // FIGURE OUT THE WORLD FILE NAME
  
  this->worldfilename[0] = 0;
  // find the world file name (it's the last argument)
  strcpy( this->worldfilename, filename );
  // make sure something happened there...
  assert(  this->worldfilename[0] != 0 );
  
  //////////////////////////////////////////////////////////////////////
  // FIGURE OUT THE DEFAULT FULL HOST NAME & ADDRESS
  // the default hostname is this host's name
  char current_hostname[ HOSTNAME_SIZE ];
  strncpy( current_hostname, this->m_hostname, HOSTNAME_SIZE );
  
  // maintain a connection to the nameserver - speeds up lookups
  sethostent( true );
  
  struct hostent* info = gethostbyname( current_hostname );
  assert( info );
  struct in_addr current_hostaddr;
  
  // make sure this looks like a regular internet address
  assert( info->h_length == 4 );
  assert( info->h_addrtype == AF_INET );
  
  // copy the address out
  memcpy( &current_hostaddr.s_addr, info->h_addr_list[0], 4 ); 
  
  //printf( "\nHOSTNAME %s NAME %s LEN %d IP %s\n", 
  //  current_hostname,
  //  info->h_name, 
  //  info->h_length, 
  //  inet_ntoa( current_hostaddr ) );

  /////////////////////////////////////////////////////////////////
  // NOW LOAD THE WORLD FILE, CREATING THE ENTITIES AS WE GO
  
  // Load and parse the world file
  if (!this->worldfile.Load(worldfilename))
    return false;
  
  // Make sure there is an "environment" section
  int section = this->worldfile.LookupEntity("environment");
  if (section < 0)
  {
    PRINT_ERR("no environment specified");
    return false;
  }
  
  // Construct a single fixed obstacle representing
  // the environment.
  this->wall = new CFixedObstacle(this, NULL);
  
  // Load the settings for this entity
  if (!this->wall->Load(&this->worldfile, section))
  {
    PRINT_ERR( "failed to load environment" );
    return false;
  }
  
  // Get the resolution of the environment (meters per pixel in the file)
  this->ppm = 1.0 / this->worldfile.ReadLength( section, 
                                                "resolution", 
                                                1.0 / this->ppm );
  
  // Initialise the matrix, now that we know its size
  int w = (int) ceil(this->wall->size_x * this->ppm);
  int h = (int) ceil(this->wall->size_y * this->ppm);
  
  assert( this->matrix = new CMatrix(w, h, 1) );
  
  // Get the authorization key to pass to player
  const char *authkey = this->worldfile.ReadString(0, "auth_key", "");
  strncpy(m_auth_key, authkey, sizeof(m_auth_key));
  m_auth_key[sizeof(m_auth_key)-1] = '\0';
  
  // Get the real update interval
  m_real_timestep = this->worldfile.ReadFloat(0, "real_timestep", 0.100);

  // Get the simulated update interval
  m_sim_timestep = this->worldfile.ReadFloat(0, "sim_timestep", 0.100);
  
  // Iterate through sections and create entities as needs be
  for (int section = 1; section < this->worldfile.GetEntityCount(); section++)
  {
    // Find out what type of entity this is,
    // and what line it came from.
    const char *type = this->worldfile.GetEntityType(section);
    int line = this->worldfile.ReadInt(section, "line", -1);

    // Ignore some types, since we already have dealt will deal with them
    if (strcmp(type, "environment") == 0)
      continue;
    if (strcmp(type, "rtk") == 0)
      continue;

    // if this section defines the current host, we load it here
    // this breaks the context-free-ness of the syntax, but it's 
    // a simple way to do this. - RTV
    if( strcmp(type, "host") == 0 )
    {
      char candidate_hostname[ HOSTNAME_SIZE ];

      strncpy( candidate_hostname, 
               worldfile.ReadString( section, "hostname", 0 ),
               HOSTNAME_SIZE );
	
      // if we found a name
      if( strlen(candidate_hostname) > 0  )
      {
        struct hostent* cinfo = 0;
	    
        printf( "CHECKING HOSTNAME %s\n", candidate_hostname );

        //lookup this host's IP address:
        if( (cinfo = gethostbyname( candidate_hostname ) ) ) 
	      {
          // make sure this looks like a regular internet address
          assert( cinfo->h_length == 4 );
          assert( cinfo->h_addrtype == AF_INET );
		
          // looks good - we'll use this host from now on
          strncpy( current_hostname, candidate_hostname, HOSTNAME_SIZE );

          // copy the address out
          memcpy( &current_hostaddr.s_addr, cinfo->h_addr_list[0], 4 ); 
		
          printf( "LOADING HOSTNAME %s NAME %s LEN %d IP %s\n", 
                  current_hostname,
                  cinfo->h_name, 
                  cinfo->h_length, 
                  inet_ntoa( current_hostaddr ) );
	      }
        else // failed lookup - stick with the last host
	      {
          printf( "Can't resolve hostname \"%s\" in world file."
                  " Sticking with \"%s\".\n", 
                  candidate_hostname, current_hostname );
        }
      }
      else
        puts( "No hostname specified. Ignoring." );

      continue;
    }
    
    // otherwise it's a device so we handle those...

    // Find the parent entity
    CEntity *parent = NULL;
    int psection = this->worldfile.GetEntityParent(section);
    for (int i = 0; i < GetEntityCount(); i++)
    {
      CEntity *entity = GetEntity(i);
      if (GetEntity(i)->worldfile_section == psection)
        parent = entity;
    }

    // Work out whether or not its a local device if any if this
    // device's host IPs match this computer's IP, it's local
    bool local = m_hostaddr.s_addr == current_hostaddr.s_addr;

    // Create the entity
    CEntity *entity = CreateEntity(type, parent );

    if (entity != NULL)
    {      
      // these pokes should really be in the entities, but it's a loooooot
      // of editing to change each constructor...

      // Store which section it came from (so we know where to
      // save it to later).
      entity->worldfile_section = section;

      // store the IP of the computer responsible for updating this
      memcpy( &entity->m_hostaddr, &current_hostaddr,sizeof(current_hostaddr));
      
      // if true, this instance of stage will update this entity
      entity->m_local = local;
      
      //printf( "ent: %p host: %s local: %d\n",
      //    entity, inet_ntoa(entity->m_hostaddr), entity->m_local );
      
      // Let the entity load itself
      if (!entity->Load(&this->worldfile, section))
      {
        PRINT_ERR1( "Failed to load entity %d from world file", 
                    GetEntityCount() ); 
        return false;
      }
      
      // Add to list of entities
      AddEntity(entity);
    }
    else
      PRINT_ERR2("line %d : unrecognized type [%s]", line, type);
  }
  
  // disconnect from the nameserver
  endhostent();

  return true;
}


//////////////////////////////////////////////////////////////////////
// Save the world file
bool CStageServer::SaveFile( char* filename )
{
  // Store the new name of the world file (for Save As).
  if (filename != NULL)
  {
    this->worldfilename[0] = 0;
    strcpy(this->worldfilename, filename);
    assert(this->worldfilename[0] != 0);
  }
  
  PRINT_MSG1("saving world to [%s]", this->worldfilename);
  
  // Let each entity save itself
  for (int i = 0; i < GetEntityCount(); i++)
  {
    CEntity *entity = GetEntity(i);
    if (!entity->Save(&this->worldfile, entity->worldfile_section))
      return false;
  }
  
  // Save everything
  if (!this->worldfile.Save(this->worldfilename))
    return false;

  return false;
}


//////////////////////////////////////////////////////////////////////
// SET UP THE SERVER TO ACCEPT INCOMING CONNECTIONS
bool CStageServer::SetupConnectionServer( void )
{
  m_pose_listen.fd = socket(AF_INET, SOCK_STREAM, 0);
  m_pose_listen.events = POLLIN; // notify me when a connection request happens
  
  struct sockaddr_in servaddr;  
  bzero(&servaddr, sizeof(servaddr));
  
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(m_port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( m_pose_listen.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( bind(m_pose_listen.fd, (SA *) &servaddr, sizeof(servaddr) )  < 0 )
  {
    cout << "Port " << m_port 
         << " is in use. Quitting (but try again in a few seconds)." 
         <<endl;
    exit( -1 );
  }
  
  // catch signals generated by socket closures
  signal( SIGPIPE, CatchSigPipe );
  
  // listen for requests on this socket
  // we poll it in ListenForPoseConnections()
  assert( listen( m_pose_listen.fd, LISTENQ) == 0 );

  return true;
}

////////////////////////////////////////////////////////////////////////
// CREATE THE DEVICE DIRECTORY for external IO
// everything makes its external devices in directory m_device_dir
bool CStageServer::CreateDeviceDirectory( void )
{
  // get the user's name
  struct passwd* user_info = getpwuid( getuid() );
  
  // make a device directory from a base name and the user's name
  sprintf( m_device_dir, "%s.%s.%d", 
           IOFILENAME, user_info->pw_name, m_instance++ );
  
  if( mkdir( m_device_dir, S_IRWXU | S_IRWXG | S_IRWXO ) == -1 )
  {
    if( errno == EEXIST )
      PRINT_WARN( "Device directory exists. Possible error?" );
    else 
    {
      PRINT_ERR1( "Failed to make device directory", strerror(errno) );
      return false;
    }      
  }

  return true;
}

bool CStageServer::ParseCmdLine( int argc, char** argv )
{
  // we've loaded the worldfile, now give the cmdline a chance to change things
  for( int a=1; a<argc-1; a++ )
  {
    // DIS/ENABLE Player
    if((strcmp( argv[a], "-n" ) == 0 )|| 
       (strcmp( argv[a], "--noplayer" ) == 0))
    {
      m_run_player = false;
      printf( "[No Player]" );
    }

    // FAST MODE - run as fast as possible - don't attempt t match real time
    if((strcmp( argv[a], "--fast" ) == 0 ) || 
       (strcmp( argv[a], "-f" ) == 0))
    {
      m_real_timestep = 0.0;
      printf( "[Fast]" );
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Start the single instance of player
bool CStageServer::StartupPlayer( void )
{
  PRINT_DEBUG( "** STARTUP PLAYER **" );

  // startup any player devices - no longer hacky! - RTV
  // we need to have fixed up all the shared memory and pointers already
  
  // count the number of Players on this host
  int player_count = 0;
  for (int i = 0; i < GetEntityCount(); i++)
    if( GetEntity(i)->stage_type == PlayerType && GetEntity(i)->m_local ) 
      player_count++;
  
  //printf( "DETECTED %d players on this host\n", player_count );

  // if there is at least 1 player device, we start a copy of Player
  // running.
  if (player_count == 0)
    return true;

  // ----------------------------------------------------------------------
  // fork off a player process to handle robot I/O
  if( (this->player_pid = fork()) < 0 )
  {
    PRINT_ERR1("error forking for player: [%s]", strerror(errno));
    return false;
  }

  // If we are the nmew child process...
  if (this->player_pid == 0)
  {
    // release controlling tty so Player doesn't get signals
     setpgrp();

    // call player like this:
    // player -stage <device directory>
    // player will open every file in the device directory
    // and attempt to memory map it as a device. 

    // Player must be in the current path
    if( execlp( "player", "player",
                "-stage", DeviceDirectory(), 
                (strlen(m_auth_key)) ? "-key" : NULL,
                (strlen(m_auth_key)) ? m_auth_key : NULL,
                NULL) < 0 )
    {
      PRINT_ERR1("error executing player: [%s]\n"
                 "Make sure player is in your path.", strerror(errno));
      if(!kill(getppid(),SIGINT))
      {
        PRINT_ERR1("error killing player: [%s]", strerror(errno));
        exit(-1);
      }
    }
  }
  
  //PRINT_DEBUG( "** STARTUP PLAYER DONE **" );
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Stop player instance
void CStageServer::ShutdownPlayer()
{
  int wait_retval;

  if(this->player_pid)
  {
    if(kill(this->player_pid,SIGTERM))
      PRINT_ERR1("error killing player: [%s]", strerror(errno));
    if((wait_retval = waitpid(this->player_pid,NULL,0)) == -1)
      PRINT_ERR1("waitpid failed: [%s]", strerror(errno));
    this->player_pid = 0;
  }
}

// read stuff until we get a continue message on each channel
int CStageServer::Read( void )
{
  //PRINT_DEBUG( "Server::Read()" );

  // look for new connections 
  ListenForConnections();
  
  return CStageIO::Read();  
}


   
void CStageServer::ListenForConnections( void )
{
  int readable = 0;
  
  // poll for connection requests with a very fast timeout
  if((readable = poll( &m_pose_listen, 1, 0 )) == -1)
  {
    if( errno != EINTR ) // timer interrupts are OK
    { 
      perror( "Stage warning: poll error (not EINTR)");
      return;
    }
  }
  
  bool success = true;

  // if the socket had a request
  if( readable && (m_pose_listen.revents & POLLIN ) ) 
  {
    // set up a socket for this connection
    struct sockaddr_in cliaddr;  
    bzero(&cliaddr, sizeof(cliaddr));
    socklen_t clilen = sizeof(cliaddr);
      
    int connfd = 0;
      
    connfd = accept( m_pose_listen.fd, (SA *) &cliaddr, &clilen);
      

    // set the dirty flag for all entities on this connection
    //for( int i=0; i < world->GetEntityCount(); i++ )
    DirtyEntities( m_pose_connection_count );


    // determine the type of connection, sync or async, by reading
    // the first byte
    char b = 0;
    int r = 0;

    if( (r = read( connfd, &b, 1 )) < 1 ) 
    {
      puts( "failed to read sync type byte. Quitting\n" );
      if( r < 0 ) perror( "read error" );
      exit( -1 );
    }
      
    // record the total bytes input
    g_bytes_input += r; 


    // if this is a syncronized connection, increase the sync counter 
    switch( b )
    {
      case STAGE_SYNC: 
        m_sync_counter++; 
#ifdef VERBOSE      
        printf( "\nStage: SYNC pose connection accepted (id: %d fd: %d)\n", 
                m_pose_connection_count, connfd );
        fflush( stdout );
#endif            
        if( m_external_sync_required ) m_enable = true;

        success = true;
        break;
      case STAGE_ASYNC:
        //default:
#ifdef VERBOSE      
        printf( "\nStage: ASYNC pose connection accepted (id: %d fd: %d)\n", 
                m_pose_connection_count, connfd );
        fflush( stdout );
#endif            
        success = true;
        break;
	  
      default: printf( "Stage: unknown sync on %d. Closing connection\n",
                       connfd  );
        close( connfd );
        success = false;
        break;
    }
   
    if( success )
    {
      // add the new connection to the arrays
      // store the connection type to help us destroy it later
      m_conn_type[ m_pose_connection_count ] = b;
      // record the pollfd data
      m_pose_connections[ m_pose_connection_count ].fd = connfd;
      m_pose_connections[ m_pose_connection_count ].events = POLLIN;
      m_pose_connection_count++;
    }
  }
}


void CStageServer::Shutdown()
{
  PRINT_DEBUG( "server shutting down" );

  CStageIO::Shutdown();

  ShutdownPlayer();

  // zap the clock device and lock file 
  unlink( clockName );
  unlink( m_locks_name );

  // delete the device directory
  if( rmdir( m_device_dir ) != 0 )
    PRINT_WARN2("failed to delete device directory \"%s\" [%s]",
                m_device_dir,
                strerror(errno));

}

//////////////////////////////////////////////////////////////////////////
// Set up a file for record locking, 1 byte per entity
// the contents aren't used - we just use fcntl() to lock bytes
bool CStageServer::CreateLockFile( void )
{
  // store the filename
  sprintf( m_locks_name, "%s/%s", m_device_dir, STAGE_LOCK_NAME );

  m_locks_fd = -1;

  if( (m_locks_fd = open( m_locks_name, O_RDWR | O_CREAT | O_TRUNC, 
                          S_IRUSR | S_IWUSR )) < 0 )
  {
    perror("Failed to create lock device" );
    return false;
  } 
  
  // set the file size - 1 byte per entity
  off_t sz = GetEntityCount();
  
  if( ftruncate( m_locks_fd, sz ) < 0 )
  {
    perror( "Failed to set lock file size" );
    return false;
  }

  //printf( "Created lock file %s of %d bytes (fd %d)\n", 
  //  m_locks_name, m_entity_count, m_locks_fd );

  return true;
}

void CStageServer::Write( void )
{
  //PRINT_DEBUG( "SERVER WRITE" );
  
  // if player has changed the subscription count, we make the property dirty
  
  // fix this - it limits the number of entities
  // just move the memory into the entity
  static int subscribed_last_time[ 10000 ]; 
  static bool init = true;
  
  // is this necessary? can't hurt i guess.
  // first time round, we zero the static buffer
  if( init ) 
  {
    memset( subscribed_last_time, false, 10000 );
    init = false;
  }
  
  for( int i=0; i < GetEntityCount(); i++ )
  {
    //PRINT_DEBUG1( "checking subscription for entity %d", i );
      
    int currently_subscribed =  GetEntity(i)->Subscribed();
      
    //PRINT_DEBUG3( "Entity %d subscriptions: %d last time: %d\n", 
    //	    i, currently_subscribed, subscribed_last_time[i] );
      
    // if the current subscription state is different from the last time
    if( currently_subscribed != subscribed_last_time[i] )
    {
	  
  	  PRINT_DEBUG2( "Entity %d subscription change (%d subs)\n", 
                    i, GetEntity(i)->Subscribed() );
	  
      // remember this state for next time
      subscribed_last_time[i] = currently_subscribed;
	  
  	  // mark the subscription property as dirty so we pick it up
  	  // below
  	  GetEntity(i)->SetDirty( PropPlayerSubscriptions , 1);
  	}
  }
  
  CStageIO::Write();
}

/////////////////////////////////////////////////////////////////////////
// Clock device -- create the memory map for IPC with Player 
bool CStageServer::CreateClockDevice( void )
{   
  size_t clocksize = sizeof( stage_clock_t );
  
  sprintf( clockName, "%s/clock", m_device_dir );
  
  int tfd=-1;
  if( (tfd = open( clockName, O_RDWR | O_CREAT | O_TRUNC, 
                   S_IRUSR | S_IWUSR )) < 0 )
  {
    PRINT_ERR1("Failed to open clock device file", strerror(errno) );
    return false;
  } 
  
  // make the file the right size
  off_t sz = clocksize;

  if( ftruncate( tfd, sz ) < 0 )
  {
    PRINT_ERR1( "Failed to set clock file size", strerror(errno) );
    return false;
  }
  
  if( write( tfd, &m_sim_timeval, sizeof(m_sim_timeval)) < 0 )
  {
    PRINT_ERR1( "Failed to write time into clock device", strerror(errno) );
    return false;
  }

  void *map = mmap( NULL, clocksize, 
                    PROT_READ | PROT_WRITE, MAP_SHARED,
                    tfd, (off_t) 0);
  
  if (map == MAP_FAILED )
  {
    PRINT_ERR1( "Failed to map clock device memory", strerror(errno) );
    return false;
  }
  
  // Initialise space
  //
  memset( map, 0, clocksize );

  // store the pointer to the clock
  m_clock = (stage_clock_t*)map;
  
  // init the clock's semaphore
  if( sem_init( &m_clock->lock, 0, 1 ) < 0 )
  {
    PRINT_ERR1( "Failed to initialize record locking semaphore", 
                strerror(errno) );
    return false;
  }
  
  close( tfd ); // can close fd once mapped
  
  PRINT_DEBUG( "Successfully mapped clock device." );
  
  return true;
}

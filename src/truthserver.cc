#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iomanip.h>
#include <termios.h>
#include <strstream.h>
#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include <queue>

//#define DEBUG
//#define VERBOSE
#undef DEBUG
#undef VERBOSE

#include "world.hh"
#include "truthserver.hh"

extern  CWorld* world;

const int LISTENQ = 128;
const long int MILLION = 1000000L;

// these can be modified in world_load.cc...
int global_truth_port = TRUTH_SERVER_PORT;

void CatchSigPipe( int signo )
{
#ifdef VERBOSE
  cout << "** SIGPIPE! **" << endl;
#endif

}

// data for each truth connection
typedef struct
{
  int fd; // file descriptor
  int count; // number of truths
  stage_truth_t* database; // ptr to array of truths
} truth_connection_t;


void PrintTruth( stage_truth_t &truth )
{
  printf( "[%d] %s ID:(%s:%d,%d,%d)\tPID:(%d,%d,%d)\tpose: [%d,%d,%d]\tsize: [%d,%d]\techo: %d\n",
	  truth.stage_id,
	  world->StringType( truth.stage_type ),
	  truth.hostname,
	  truth.id.port, 
	  truth.id.type, 
	  truth.id.index,
	  truth.parent.port, 
	  truth.parent.type, 
	  truth.parent.index,
	  truth.x, truth.y, truth.th,
	  truth.w, truth.h,
	  truth.echo_request );
  
  fflush( stdout );
}


void CWorld::TruthRead( void )
{
  int readable = 1;
  
  int max_reads = 100;
  int reads = 0;
  
  // make a nice fresh truth for importing
  stage_truth_t truth;

  // while there is stuff on the socket (subject to a sanity-check bail out)
  while( readable && (reads++ < max_reads) )
    {
      // use poll to see which truth connections have data
      if((readable = poll( m_truth_connections, m_truth_connection_count, 0 )) == -1)
	{
	  perror( "poll interrupted!");
	  return;
	}
      
      if( readable > 0 ) // if something was available
	for( int t=0; t<m_truth_connection_count; t++ )   // for all the connections
	  if( m_truth_connections[t].revents & POLLIN ) // if there was data available
	    {
	      // get the file descriptor
	      int connfd = m_truth_connections[t].fd;
	
	      // freshen up the truth
	      memset( &truth, 0, sizeof(truth) );
	      
	      int r = 0;
	      // read until we have a whole truth packet
	      while( r < (int)sizeof(truth ) )
		{
		  int v=0;
		  // read bytes from the socket, quitting if read returns no bytes
		  if( (v = read( connfd, ((char*)&truth) + r, sizeof(truth) - r )) < 1 )
		    {
		      //#ifdef VERBOSE
		      puts( "Stage: TruthRead() read failed!" );
		      //#endif	  
		      close( connfd );
		      connfd = 0; // forces the writer to quit
		      //pthread_exit( 0 );
		    }
		  
		  r+=v;
		  
		  // THIS IS DEBUG OUTPUT
		  if( v < (int)sizeof(truth) )
		    printf( "STAGE: SHORT READ (%d/%d) r=%d\n",
			    v, (int)sizeof(truth), r );
		  
		}
	      
	      assert( r == sizeof( truth ) );
	      
	      PrintTruth( truth );
	      
	      // INPUT THE TRUTH HERE

	      if( truth.stage_id == -1 ) // its a command for the engine!
		{
		  switch( truth.x ) // used to identify the command
		    {
		      //case LOADc: Load( m_filename ); break;
		    case SAVEc: Save( m_filename ); break;
		    case PAUSEc: m_enable = !m_enable; break;
		    default: printf( "Stage Warning: "
				     "Received unknown command (%d); ignoring.\n",
				     truth.x );
		    }
		}
	      else // it is an entity update - move it now
		{
		  CEntity* ent = m_object[ truth.stage_id ];
		  assert( ent ); // there really ought to be one!
      
		  // check to see if we really need to move the entity
     
		  // this is where the entity is now
		  //double dx, dy, dth;
		  //ent->GetGlobalPose( dx, dy, dth );
      
		  // compress the doubles to match the truth data
		  //uint32_t uix = (uint32_t)( dx * 1000.0 );
		  //uint32_t uiy = (uint32_t)( dy * 1000.0 );
		  //int degrees = (int)RTOD( dth );
		  //if( degrees < 0 ) degrees += 360;	    
		  //uint16_t uith = (uint16_t)degrees;  
           
		  // if the pose is different
		  //if( uix != truth.x || uiy != truth.y || uith != truth.th )
		  //{
		      // update the entity with the truth
		      ent->SetGlobalPose( truth.x/1000.0, truth.y/1000.0, 
					  DTOR(truth.th) );
		      
		      // this ent is now dirty to all channels 
		      ent->MakeDirty();

		      // unless this channel doesn't want an echo
		      // in which case it is clean just here.
		      if( !truth.echo_request )
			ent->m_dirty[t]= false;
		      //}
		}
	    }
      
    }
}

void CWorld::TruthWrite( void )
{
  int i;

  // for all the connections
  for( int t=0; t< m_truth_connection_count; t++ )
    {
      int connfd = m_truth_connections[t].fd;
      
      
	  if( connfd == 0 ) // if it was been re-set by the reader on exit
	    {
#ifdef VERBOSE
	      puts( "Stage: TruthWrite detcted closed fd!" );
#endif
	      // delete the datastructures
	      //if( con->database ) delete [] con->database;
	      //if( con ) delete con;
	      
	      //pthread_exit( 0 );
	    }
	  
	  for( i=0; i < m_object_count; i++ )
	    {  
	      // is the entity marked dirty in this connection?
	      if( m_object[i]->m_dirty[t] )
		{
		  stage_truth_t truth;

		  m_object[i]->ComposeTruth( &truth, i );
		  
		  // mark it clean on this connection
		  // it won't get re-sent here until this flag is set again
		  m_object[i]->m_dirty[t] = false;
		  
		  //printf( "old truth: " );
		  //PrintTruth( ts_truths[i] );
		  //printf( "new truth: " );
		  //PrintTruth( truth );
		  //puts( "sending..." );
		  
		  // we don't want this echoed back to us
		  truth.echo_request = false;
		  // send the packet to the connected client
		  int v = write( connfd, &truth, sizeof(truth) );
		  
		  // we really should have written the whole packet
		  assert( v == (int)sizeof(truth) );
		  
		  // and store it
		  //memcpy( &(con->database[i]), &truth, sizeof( truth ) );
		  //}
		  //else
		  //puts( "same as last time" );
		  
		    if( v < 0 )
		      {
			perror( "Stage: TruthWrite: write error!" );	      
		      }
		  }
	    }
    }
}


void CWorld::SetupTruthServer( void )
{
  m_truth_listen.fd = socket(AF_INET, SOCK_STREAM, 0);
  m_truth_listen.events = POLLIN; // notify me when a connection request happens

  struct sockaddr_in servaddr;  
  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(global_truth_port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( m_truth_listen.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( bind(m_truth_listen.fd, (SA *) &servaddr, sizeof(servaddr) )  < 0 )
    {
      cout << "Port " << global_truth_port 
	   << " is in use. Quitting (but try again in a few seconds)." 
	   <<endl;
      exit( -1 );
    }
  
  // listen for requests on this socket
  // we poll it in ListenForTruthConnections()
  listen( m_truth_listen.fd, LISTENQ);


  // allocate the buffers 
  //m_old_truths = new stage_truth_t[ m_object_count ];
  //m_current_truths = new stage_truth_t[ m_object_count ];
  //m_truth_diff = new bool[ m_object_count ];
}


void CWorld::ListenForTruthConnections( void )
{
  int readable = 0;
  
  // poll for connection requests with a very fast timeout
  if((readable = poll( &m_truth_listen, 1, 0 )) == -1)
    {
      perror( "poll interrupted!");
      return;
    }
  
  // if the socket had a request
  if( readable && (m_truth_listen.revents | POLLIN ) ) 
    {
      // set up a socket for this connection
      struct sockaddr_in cliaddr;  
      bzero(&cliaddr, sizeof(cliaddr));
      socklen_t clilen = sizeof(cliaddr);
      
      int connfd = 0;
      
      connfd = accept( m_truth_listen.fd, (SA *) &cliaddr, &clilen);
      
      //#ifdef VERBOSE      
      printf( "Stage: :ListenForTruthConnections() connection accepted (socket %d)\n", 
	      connfd );
      fflush( stdout );
      //#endif            

      // add the new connection to the array
      m_truth_connections[ m_truth_connection_count ].fd = connfd;
      m_truth_connections[ m_truth_connection_count ].events = POLLIN;
      m_truth_connection_count++;
      
      //truth_connection_t* con = new truth_connection_t();
      
      //con->fd = connfd;
      //con->count = world->GetObjectCount();
      //con->database = new stage_truth_t[ con->count ];
      
      // zero the database
      //memset( con->database, 0, con->count * sizeof( stage_truth_t ) );
      

      //pthread_t tid_dummy;
      // start a thread to handle the connection
      // passing in the structure they share
      //pthread_create(&tid_dummy, NULL, &TruthWriter, con ); 
      //pthread_create(&tid_dummy, NULL, &TruthReader, con ); 
    }

}


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

// rather hacky - this gets poked into by incoming truths
// to avoid them being re-exported
stage_truth_t* ts_truths = 0;

void PrintTruth( stage_truth_t &truth )
{
  printf( "ID: (%d,%d,%d)\tPID:(%d,%d,%d)\tpose: [%d,%d,%d]\tsize: [%d,%d]\n", 
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

static void* TruthReader( void* arg )
{
#ifdef VERBOSE
  printf( "Stage: TruthReader thread (socket %d)\n", *connfd );
  fflush( stdout );
#endif

  int* connfd = (int*)arg;

  sigblock(SIGINT);
  sigblock(SIGQUIT);
  sigblock(SIGHUP);

  pthread_detach(pthread_self());

  stage_truth_t truth;
  
  int r = 0;
  while( 1 )
    {	      
      /* read will block until it has some bytes to return */
      r = read( *connfd, &truth, sizeof(truth) );
      
      //#ifdef DEBUG
      //printf( "Stage: TruthReader read (%d,%d/%d)\n", 
      //      *connfd, r, sizeof(truth) );    
      //      fflush( stdout );
      //#endif

      if( r > 0 )
	{
	  //#ifdef DEBUG
	  //printf( "Recv: " );
	  //PrintTruth( truth );
	  //#endif
	  
	  // we can't just impose the truth here as we're not
	  // in sync with the main simulator thread, so:
	  // copy the incoming truth into the update queue.
	  // the main thread will import the truth on the next update cycle
	  // the queue ought to be thread-safe (!)
	  world->input_queue.push( truth );
	}
      else
	{
#ifdef VERBOSE
	  puts( "Stage: TruthReader thread exit." );
#endif	  
	  close( *connfd );
	  *connfd = 0; // forces the writer to quit

	  pthread_exit( 0 );
	}
    }
}





static void * TruthWriter( void* arg )
{
#ifdef VERBOSE
  printf( "Stage: TruthWriter thread (socket %d)\n", *connfd );
  fflush( stdout );
#endif

  int* connfd = (int*)arg;

  sigblock(SIGINT);
  sigblock(SIGQUIT);
  sigblock(SIGHUP);

  pthread_detach(pthread_self());


  int v = 0;

  int obs = world->GetObjectCount();
  
  // store all the truths we publish for comparison
  ts_truths = new stage_truth_t[ obs ];
  memset( ts_truths, 0, obs * sizeof( stage_truth_t ) );
  
  while( 1 ) 
    {
      for( int i=0; i < obs; i++ )
	{
	  CEntity* ent =  world->GetObject( i );
	  
	  assert( ent );
	  
	  if( *connfd == 0 ) // if it hasn't been re-set by the reader on exit
	    {
#ifdef VERBOSE
	      puts( "Stage: TruthWriter thread exit." );
#endif
	      delete connfd;
	      pthread_exit( 0 );
	    }

	  stage_truth_t truth;
	  
	  ent->ComposeTruth( &truth, i );

	  // is the packet different from the last one?
	  if( memcmp( &truth, &(ts_truths[i]), sizeof( truth ) ) != 0  ) 
	    {
	      //printf( "old truth: " );
	      //PrintTruth( ts_truths[i] );
	      //printf( "new truth: " );
	      //PrintTruth( truth );
	      //puts( "sending..." );
	      
	      // send the packet to the connected client
	      v = write( *connfd, &truth, sizeof(truth) );
	      
	      // and store it
	      memcpy( &(ts_truths[i]), &truth, sizeof( truth ) );
	    }
	  //else
	  //puts( "same as last time" );

	  if( v < 0 )
	    {
	      perror( "Stage: TruthWriter: write error: quitting thread" );
	      close( *connfd );
	      pthread_exit( 0 );
	    }
	  
	} 
      usleep( 50000 ); // give the cpu a break for 10ms 
      //usleep( 100000 ); // give the cpu a break for 100ms
    }
}

void* TruthServer( void* )
{
  pthread_detach(pthread_self());
    
  int listenfd;

  //pid_t childpid;
  socklen_t clilen;
  struct sockaddr_in	cliaddr, servaddr;
  
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(global_truth_port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

  if( bind(listenfd, (SA *) &servaddr, sizeof(servaddr) )  < 0 )
      {
	cout << "Port " << global_truth_port 
	     << " is in use. Quitting (but try again in a few seconds)." 
	     <<endl;
	exit( -1 );
      }
 
  listen(listenfd, LISTENQ);
  
  pthread_t tid_dummy;
  
  while( 1 ) 
    {
      clilen = sizeof(cliaddr);

      int* connfd = new int( 0 );

      *connfd = accept(listenfd, (SA *) &cliaddr, &clilen);

#ifdef VERBOSE      
      printf( "Stage: TruthServer connection accepted (socket %d)\n", 
	      *connfd );
      fflush( stdout );
#endif            
      // start a thread to handle the connection
      pthread_create(&tid_dummy, NULL, &TruthWriter, connfd ); 
      pthread_create(&tid_dummy, NULL, &TruthReader, connfd ); 
    }
}







































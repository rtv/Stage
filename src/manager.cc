/*##########################################################################
# manager.cc - implements the Stage Manager for syncing distributed Stages
# $Id: manager.cc,v 1.15 2001-10-09 00:49:07 gerkey Exp $
*#########################################################################*/

#include <sys/types.h>	/* basic system data types */
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <iostream.h>
#include <values.h>
#include <netdb.h>     /* for gethostbyname(3) */
#include <sys/poll.h>

#include "truthserver.hh"

#define VERSION "0.1"

#define VERBOSE

// arrays are created to store the byte counts for each connection
int *bytes_in, *bytes_out;
double last_output_time = 0;
int bps; // bytes per second
int bytesThisInterval = 0;

// this is sent to all stages so they have a common timestep number
// it is incremented each update
uint32_t timestep = 0;


void PrintPose( stage_pose_t &pose )
{
  printf( "[%d] (%d,%d,%d) echo: %d\n",
	  pose.stage_id,
	  pose.x, pose.y, pose.th,
	  pose.echo_request );
  
  fflush( stdout );
}

int stages = 0;
struct pollfd* servers = 0;

void Activity( int reading )
{
  static int t = 0, s = 0;

  char b,c;
  
  // if there was activity, spin the active ticker
  // else spin the idle ticker
  ( reading > 0 ) ? t++ : s++;

  switch( t )
    {
    case 0: c = '|'; break;
    case 1: c = '/'; break;
    case 2: c = '-'; break;
    case 3: c = '\\'; 
      t = 0;
      break;
    }    

  switch( s )
    {
    case 0: b = '|'; break;
    case 1: b = '/'; break;
    case 2: b = '-'; break;
    case 3: b = '\\'; 
      s = 0;
      break;
    }    

     
    char* buf = new char[ stages+1 ];
    memset( buf, 0, stages+1 );
  
    for( int c=0; c<stages; c++ )
//      // is this one ready to read?
      (servers[c].revents & POLLIN) ?  buf[c] = '!' : buf[c] = '.';
    
    fprintf( stderr, "\rActivity: %c %c [%s] %d bytes/s   ", 
	     b, c, buf, bps );
 
  delete [] buf;

 fflush( stdout );
}


size_t ReadHeader( stage_header_t *hdr, int s )
{
  size_t packetlen = sizeof( stage_header_t );
  size_t recv = 0;

  while( recv < packetlen )
    {
      int r = read(servers[s].fd, ((char*)hdr)+recv,  packetlen - recv );
      
      if( r < 0 )
	perror( "Manager: read error" );
      else
	recv += r;
      
      bytes_in[s] += r;
      bytesThisInterval += r;

      //printf( "\nread %d bytes - num_poses\n", r );
    }
  
  return packetlen;
}

size_t ReadData( int s, char* data, size_t packetlen )
{
  
  int recv = 0;
  
  // read the first two bytes to see how many poses are coming
  
  while( recv < (int)packetlen )
    {
      /* read will block until it has some bytes to return */
      int r = read( servers[s].fd, data + recv,  packetlen - recv );
      
      if( r < 0 )
	perror( "Manager: read error" );
      else
	recv += r;

      bytes_in[s] += r; 
      bytesThisInterval += r;
    }
  return(  recv );
}

int ForwardData( int sender, char* buf, size_t buflen )
{
  // forward the packet to the other servers 
  for(int j=0; j<stages; j++)
    {
      if( j != sender ) 
	{
	  //puts( "forwarding!" );

	  unsigned int writecnt = 0;
	  int thiswritecnt;
	  while(writecnt < buflen)
	    {
              //printf("writing to %d\n",j);
	      thiswritecnt = write( servers[j].fd, 
				    buf+writecnt, 
				    buflen-writecnt);
              //puts("done");

	      assert(thiswritecnt >= 0);
	      writecnt += thiswritecnt;

	      bytes_out[j] += thiswritecnt;
	      bytesThisInterval += writecnt;
	      
	      //printf( "\nforwarded %d bytes\n", writecnt );
	    }
	}
    }
  return true;
}


int main(int argc, char **argv)
{
  // hello world
  fprintf( stderr, "** Manager v%s **\n", (char*) VERSION);

  //FILE* logfilep;
  //if(!(logfilep = fopen("managerlog", "w+")))
  //{
    //perror("fopen()");
    //exit(-1);
  //}

  /* client vars */
  int v;
  //int b, r, w; /* return values of various system calls */ 

  stages = argc - 1;

  // one fd for each stage
  servers = new struct pollfd[ stages ];
  
  // one receipt record for each stage
  bool *receipt = new bool[ stages ];
  
  // storage for the byte counts
  bytes_in = new int[ stages ];
  bytes_out = new int[ stages ];

  // initially zero
  memset( bytes_in, 0, sizeof(int)*stages );
  memset( bytes_out, 0, sizeof(int)*stages );

  for( int s=0; s<stages; s++ )
    {

      /* open socket for network I/O */
      servers[s].fd = socket(AF_INET, SOCK_STREAM, 0);
      servers[s].events = POLLIN; // notify me when data arrives
      
      if( servers[s].fd < 0 )
	{
	  printf( "Error opening network socket. Exiting\n" );
	  fflush( stdout );
	  exit( -1 );
	}
      
      struct sockaddr_in servaddr;
      
      char* hostname = argv[s+1];

      fprintf( stderr, 
	       "Connecting to %s on port %d... ", 
	       hostname, DEFAULT_POSE_PORT );

      struct hostent* entp;
      if( (entp = gethostbyname( hostname )) == NULL)
	{
	  fprintf(stdout, "unknown host \"%s\" (argument %d). Quitting\n",
		  hostname,s );
	  return(-1);
	}

      char* ip = argv[s+1];
      
      /* setup our server address (type, IP address and port) */
      bzero(&servaddr, sizeof(servaddr)); /* initialize */
      servaddr.sin_family = AF_INET;   /* internet address space */
      servaddr.sin_port = htons( DEFAULT_POSE_PORT ); /*our command port */ 
      // hostname
      memcpy(&(servaddr.sin_addr), entp->h_addr_list[0], entp->h_length);

      v = connect( servers[s].fd, (sockaddr*)&servaddr, sizeof( servaddr) );
      
      if( v < 0 )
	{
	  fprintf( stderr, 
		   "\nCan't find a Stage pose server on %s. Quitting.\n", 
		   ip ); 
	  fflush( stdout );
	  exit( -1 );
	}

      // send the connection type byte - we want a synchronous connection
      char c = STAGE_SYNC;
      int r;
      if( (r = write( servers[s].fd, &c, 1 )) < 1 )
	{
	  printf( "XS: failed to write SYNC byte to Stage. Quitting\n" );
	  if( r < 0 ) perror( "write errored" );
	  exit( -1 );
	}
      
      fprintf( stderr, " OK.\n" );
    }

  puts( "" );

  struct timeval start_tv;
  //struct timeval tv;
  gettimeofday( &start_tv, 0 );
  
  //char* tmstr = ctime( &start_tv.tv_sec);
  //tmstr[ strlen(tmstr)-1 ] = 0; // delete the newline
  
  fprintf( stdout, 
	   "# Manager data log\n"
	   "# Date: %s"
	   "# Format:\n"
	   "#STEP\tTIME\t\tBYTES/S\t\tINPUT\t\tOUTPUT\t", 
	   ctime( &start_tv.tv_sec) );

  for( int y=0; y<stages; y++ )
    fprintf( stdout, "\t\%d.IN\t%d.OUT", y, y );
  
  fprintf( stdout, "\n" );
  
  while( 1 )
    {

      // clear the receipt table
      memset( receipt, 0, sizeof(bool) * stages );
      bool received_all = false;
      
      while( !received_all )
	{
      
	  int num_to_read = 0;
      
	  // let's try this poll(2) thing 
	  // (with a timeout so we can be sure it is running)
	  if((num_to_read = poll( servers, stages, 100)) == -1)
	    {
	      perror( "poll failed");
	      return(-1);
	    }
 
	  //printf("poll returned %d to-read\n", num_to_read);
	  // call the corresponding Read() for each one that's ready
      
	  Activity( num_to_read );

	  //struct timeval start,end;
	  //gettimeofday(&start,NULL);
	  if( num_to_read > 0 ) for(int i=0; i<stages; i++)
	    {
	      // is this one ready to read?
	      if(servers[i].revents & POLLIN)
		{      
		  stage_header_t hdr;
		  ReadHeader( &hdr, i );
		  
		  receipt[i] = true;

		  // if this was a data header, read it and forward data.
		  if( hdr.type == PosePacket && hdr.data > 0 )
		    {
		      size_t bufsize = 
			sizeof(stage_header_t)+sizeof(stage_pose_t)*hdr.data;
		      
		      char* buf = new char[ bufsize ];
		  
		      // put the header in the beginning of the buffer
		      memcpy( buf, &hdr, sizeof( hdr ) );
		      
		      // read data into the rest of it
		      ReadData( i, buf+sizeof(hdr), 
				bufsize-sizeof(hdr) );
		  
		      // forward the whole buffer
		      ForwardData( i, buf, bufsize ); 

                      // get rid of the buffer
                      delete buf;
		    }
	      
		}
	    }

	  received_all = true;
	  for( int h=0; h<stages; h++ )
	    if( !receipt[h] ) 
	      {
		received_all = false;
		break;
	      }
	}

      // send an ACK byte to all the servers

      // if ACK == 1, Stage will continue immediately.  if ACK == 2,
      // Stage will read a 4 byte unsigned int which sets the timestep
      // number, then continue - this can sync all Stages nicely to
      // the same timestep

      // this is used as a timestep number so that all stages are in sync
      //puts( "XS: sending ACK" );
      
      
      stage_header_t ack;
      ack.type = Continue;
      ack.data = timestep;
      
      //printf( "[ACK (%d:%x)]\n", ack.type, ack.data );
    
      for( int h=0; h<stages; h++ )
	{
	  int r = 0;
	  for( size_t written=0; written < sizeof( ack ); written += r )
	    {
	      if( (r = write( servers[h].fd, &ack, sizeof(ack) ) ) == -1 )
		perror( "XS: Failed to write ACK" ); 
	      
	      bytesThisInterval += r;

	      bytes_out[h] += r;
	    }  
	}
      
      // increment the timestep
      timestep++;
      
      
      struct timeval tv;
      gettimeofday( &tv, NULL );

      double seconds = tv.tv_sec - start_tv.tv_sec;
      double useconds = tv.tv_usec - start_tv.tv_usec;
      
      seconds += useconds / 1000000.0;

      if( seconds >= last_output_time+1.0 )
	{
	  double interval = seconds - last_output_time;
	
	  bps = (int)rint( bytesThisInterval / interval);

	  // reset 
	  bytesThisInterval = 0;
	  last_output_time = seconds;
	 
	  unsigned int total_bytes_in = 0;
	  unsigned int total_bytes_out = 0;
	 
	  // count the total bytes in and out
	  for( int y=0; y<stages; y++ )
	    {
	      total_bytes_in += bytes_in[y];
	      total_bytes_out += bytes_out[y];
	    }

	  fprintf( stdout, "%d\t%.3f\t\t%d\t\t%u\t\t%u\t\t", 
		   timestep, seconds, bps , total_bytes_in, total_bytes_out );
	  
	  for(int i=0; i<stages; i++)
	    {
	      fprintf( stdout, "%d\t%d\t", bytes_in[i], bytes_out[i] );
	    }
	  fprintf( stdout, "\n" );
	}
    }
}     








/*##########################################################################
# manager.cc - implements the Stage Manager for syncing distributed Stages
# $Id: manager.cc,v 1.12 2001-09-28 21:55:42 gerkey Exp $
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
  
      printf( "\rActivity: %c %c [%s]", b, c, buf);
 
      //printf( "\rActivity: %c", c );

  delete [] buf;

 fflush( stdout );
}


size_t ReadHeader( int s )
{
  // read the first two bytes to see how many poses are coming
  size_t packetlen = sizeof(uint16_t);
  size_t recv = 0;
  
  uint16_t num_poses;
  
  while( recv < packetlen )
    {
      //printf( "Reading on %d\n", ffd ); fflush( stdout );
      
      int r = read(servers[s].fd, ((char*)&num_poses)+recv,  packetlen - recv );
      
      if( r < 0 )
	perror( "Manager: read error" );
      else
	recv += r;
    }
  
  return num_poses;
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
      
      //printf( "Read %d/%d bytes on %d\n",recv,packetlen, servers[s].fd ); 
    }
  return(  recv );
}

int ForwardData( int sender, char* buf, size_t buflen )
{
  // foward the packet to the other servers 
  for(int j=0; j<stages; j++)
    {
      if( j != sender ) 
	{
	  unsigned int writecnt = 0;
	  int thiswritecnt;
	  while(writecnt < buflen)
	    {
	      thiswritecnt = write( servers[j].fd, 
				    buf+writecnt, 
				    buflen-writecnt);

	      assert(thiswritecnt >= 0);
	      writecnt += thiswritecnt;
	    }
	}
    }
  return true;
}


int main(int argc, char **argv)
{
  // hello world
  printf("** Manager v%s ** ", (char*) VERSION);

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

      printf( "\nConnecting to %s on port %d... ", hostname, DEFAULT_POSE_PORT );

      struct hostent* entp;
      if( (entp = gethostbyname( hostname )) == NULL)
	{
	  fprintf(stderr, "unknown host \"%s\" (argument %d). Quitting\n",
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
	  printf( "\nCan't find a Stage pose server on %s. Quitting.\n", 
		  ip ); 
	  fflush( stdout );
	  exit( -1 );
	}
      else
	printf( " OK." );
    }

  puts( "" );

  struct timeval start_tv;
  //struct timeval tv;
  gettimeofday( &start_tv, 0 );
  
  //double seconds;

  while( 1 )
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
	      uint16_t num_poses = ReadHeader( i );
	      
	      size_t bufsize = sizeof(uint16_t)+sizeof(stage_pose_t)*num_poses;
	      char* buf = new char[ bufsize ];
	      
	      // put the size in the beginning of the buffer
	      *(uint16_t*)buf = num_poses;
	      
	      // read data into the rest of it
	      ReadData( i, buf+sizeof(uint16_t), bufsize-sizeof(uint16_t) );
	      
	      // forward the whole buffer
	      ForwardData( i, buf, bufsize ); 
	      
	    }
	}
    }
}     








#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif

#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "rtp.h"

CRTPPlayer::CRTPPlayer( char* address )
{
  // parse the string
  char ipstr[64], portstr[64];

  char* colon = index( address, ':' );

  // there should be a colon in there somewhere
  if( colon == NULL )
    {
      perror( "RTP address malformed; please specify <ip>:<port>" );
      exit( -1 );
    }
  
  *colon = 0; // replace the colon with a terminator
 
  strcpy( ipstr, address ); 
  strcpy( portstr, colon + 1 );

  //printf( "address: %s\n", address );
  // printf( "IPSTR: %s\n", ipstr );
  //printf( "PORTSTR: %s\n", portstr );

  struct in_addr ip;
#if HAVE_INET_ATON
  inet_aton( ipstr, &ip );
#endif

  int port = atoi( portstr );
  
  
  //printf( "RTPPlayer(char*) created for %s:%d", inet_ntoa(ip), port);
  //printf( "[RTP %s:%d]", inet_ntoa(ip), port);

  // have to byteswap here
  //ip.s_addr = ntohl( ip.s_addr );
  //port = htons( port );

  // use the other constructor
  //::CRTPPlayer( ip.s_addr, port );
  // seed the random number generator
#if HAVE_SRAND48
  srand48( time(NULL) );
#endif

      // set up the header
      hdr.version = 2;
      hdr.p = 0; // no padding
      hdr.x = 0; // no header extension
      hdr.cc = 0; // no csrc array
      hdr.m = 0; // no marker
      hdr.pt = 0; // zero type
      hdr.seq = 0; // first in the sequence
      hdr.ts = 0; // zero timesstamp
#if HAVE_LRAND48
      hdr.ssrc = lrand48(); // choose a random source ID
#endif

      // set up the socket
      sockfd = socket( AF_INET, SOCK_DGRAM, 0 );

      // and the destination address
      memset( &dest, 0, sizeof(dest) );
      dest.sin_family = AF_INET;
      dest.sin_addr.s_addr = ip.s_addr;//htonl(ip);
      dest.sin_port = htons(port);

      //printf( "RTPPlayer(int,int) created for %s:%d", 
      //      inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));

      //printf( "RTPPlayer constructor this %p\n", this );
}

CRTPPlayer::CRTPPlayer( uint32_t ip, uint16_t port )
{
  // seed the random number generator
#if HAVE_SRAND48
  srand48( time(NULL) );
#endif

      // set up the header
      hdr.version = 2;
      hdr.p = 0; // no padding
      hdr.x = 0; // no header extension
      hdr.cc = 0; // no csrc array
      hdr.m = 0; // no marker
      hdr.pt = 0; // zero type
      hdr.seq = 0; // first in the sequence
      hdr.ts = 0; // zero timesstamp
#if HAVE_LRAND48
      hdr.ssrc = lrand48(); // choose a random source ID
#endif

      // set up the socket
      sockfd = socket( AF_INET, SOCK_DGRAM, 0 );

      // and the destination address
      memset( &dest, 0, sizeof(dest) );
      dest.sin_family = AF_INET;
      dest.sin_addr.s_addr = htonl(ip);
      dest.sin_port = htons(port);

      //printf( "RTPPlayer(int,int) created for %s:%d", 
      //      inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));

      //printf( "RTPPlayer constructor this %p\n", this );
  
 }

void CRTPPlayer::SendData( uint8_t datatype, void *data, int len, uint32_t timestamp )
{
  //printf( "RTPPlayer send data this %p\n", this );

  // we just change the necessary parts of the header then send the packet
  hdr.ts = htonl(timestamp);
  hdr.pt = datatype;
  hdr.seq++; 
  
  // make a buffer to send the header and data
  size_t buflen = sizeof(rtp_hdr_t) + len;
  char* buf = new char[buflen];
  
  memcpy( buf, &hdr, sizeof( rtp_hdr_t ) );
  memcpy( buf + sizeof(rtp_hdr_t), data, len );
  
  //printf( "RTPPlayer sending %d:%d (%d) bytes to  %s:%d\n",
  //  sizeof( rtp_hdr_t ), 
  //  len, 
  //  buflen,
  //  inet_ntoa(dest.sin_addr), 
  //  ntohs(dest.sin_port) ); 
	  
  sendto( sockfd, buf, buflen, 0, (sockaddr*)&dest, sizeof(dest) );
};

void CRTPPlayer::Test( void )
{
  struct timeval tv;
  gettimeofday( &tv, 0 );
  
  char* test = "Hello world";
  SendData( 3, test, strlen(test), tv.tv_sec );
};


//  void InitRTP( void )
//  {
//    struct hostent* entp = 0;
//    struct sockaddr_in servaddr;
  
//    if((entp = gethostbyname( host )) == NULL)
//      {
//        fprintf(stderr, "\nCan't find host \"%s\"; quitting\n", 
//  	      host );
//        exit( -1 );
//      }	  
  
//    /* open socket for network I/O */
//    rtpfd = socket(AF_INET, SOCK_DGRAM, 0);
  
//    if( rtpfd < 0 )
//      {
//        printf( "Error opening UDP network socket. Exiting\n" );
//        fflush( stdout );
//        exit( -1 );
//      }


//    /* setup our server address (type, IP address and port) */
//    bzero(&servaddr, sizeof(servaddr)); /* initialize */
//    servaddr.sin_family = AF_INET;   /* internet address space */
//    servaddr.sin_port = htons( rtp_port ); /*our command port */ 
//    memcpy(&(servaddr.sin_addr), entp->h_addr_list[0], entp->h_length);

//    bind( rtpfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

//    printf( "[RECVING on %s:%d]", 
//    	  inet_ntoa(servaddr.sin_addr), 
//    	  ntohs(servaddr.sin_port) ); 

//    fflush( stdout );
//  }

//  bool ParseRTP( char* buf, size_t len, 
//  	       rtp_hdr_t** rtp, device_hdr_t **dev, char** data )
//  {
//    *rtp = (rtp_hdr_t*)buf; 
  
//    *dev = (device_hdr_t*)(buf + sizeof(rtp_hdr_t)); 
  
//    // idiot check! we should have exactly the right number of bytes
//    assert( len == sizeof(rtp_hdr_t) + sizeof(device_hdr_t) + (*dev)->len );
  
//    (*dev)->len == 0 ? *data = NULL : 
//      *data = buf+sizeof(rtp_hdr_t)+sizeof(device_hdr_t); 
  
//    //PrintRTPHeader( rtp );
//    //PrintDeviceHeader( dev );
//    //PrintData( data );
   
//    return true;
//  }

//  // read maximum len bytes into buf, return number of bytes used
//  size_t ReadRTP( char* buf, size_t len )
//  {
//    // set up the select data structure
//    fd_set fdset;
//    FD_ZERO( &fdset );
//    FD_SET( rtpfd, &fdset );
  
//    // create a timeval of zero seconds as we don't want to block
//    struct timeval tv;
//    memset( &tv, 0, sizeof(tv) );
  
//    // see if there is anything to read on the rtpfd
//    select( rtpfd+1, &fdset, NULL, NULL, &tv ); 
  
//    // if the socket is readable
//    if( FD_ISSET( rtpfd, &fdset ) )
//      {
//        struct sockaddr from;
//        socklen_t fsize = sizeof(from);
      
//        return recvfrom( rtpfd, buf, len, 0, &from, &fsize );
//      }

//    return (size_t)0;
//  }
      //if( figs[ dev.id ].body ) // seen this figure before?
      //UpdateFigures( &dev, data, dev->len );
      //else // unseen device - we need new figures
//	GenerateFigures( &dev, 0 );
//  }

//}


#include <string.h>
#include <sys/time.h>

#include "rtp.h"

CRTPPlayer::CRTPPlayer( uint32_t ip, uint16_t port )
{
  // seed the random number generator
  srand48( time(NULL) );

      // set up the header
      hdr.version = 2;
      hdr.p = 0; // no padding
      hdr.x = 0; // no header extension
      hdr.cc = 0; // no csrc array
      hdr.m = 0; // no marker
      hdr.pt = 0; // zero type
      hdr.seq = 0; // first in the sequence
      hdr.ts = 0; // zero timesstamp
      hdr.ssrc = lrand48(); // choose a random source ID

      // set up the socket
      sockfd = socket( AF_INET, SOCK_DGRAM, 0 );

      // and the destination address
      memset( &dest, 0, sizeof(dest) );
      dest.sin_family = AF_INET;
      dest.sin_addr.s_addr = htonl(ip);
      dest.sin_port = htons(port);
    }

void CRTPPlayer::SendData( uint8_t datatype, void *data, int len, uint32_t timestamp )
{
  // we just change the necessary parts of the header then send the packet
  hdr.ts = htonl(timestamp);
  hdr.pt = datatype;
  hdr.seq++; 
  
  // make a buffer to send the header and data
  size_t buflen = sizeof(rtp_hdr_t) + len;
  char* buf = new char[buflen];
  
  memcpy( buf, &hdr, sizeof( rtp_hdr_t ) );
  memcpy( buf + sizeof(rtp_hdr_t), data, len );
  
  sendto( sockfd, buf, buflen, 0, (sockaddr*)&dest, sizeof(dest) );
};

void CRTPPlayer::Test( void )
{
  struct timeval tv;
  gettimeofday( &tv, 0 );
  
  char* test = "Hello world";
  SendData( 3, test, strlen(test), tv.tv_sec );
};

///////////////////////////////////////////////////////////////////////////
//
// File: rtp.h
// Author: Richard Vaughan
// Date: 12 March 2002
// Desc: class for transmitting generic RTP data
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/rtp.h,v $
//  $Author: gerkey $
//  $Revision: 1.2 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#ifndef RTP_H
#define RTP_H

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#if HAVE_STDINT_H
  #include <stdint.h>
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>

/*
 * RTP packet header
 */
typedef struct {

#ifdef STAGE_BIG_ENDIAN // SPARC/POWERPC
    unsigned char version:2;   /* protocol version */
    unsigned char p:1;         /* padding flag */
    unsigned char x:1;         /* header extension flag */
    unsigned char cc:4;        /* CSRC count */
    unsigned char m:1;         /* marker bit */
    unsigned char pt:7;        /* payload type */
#else // little endian - X86
    unsigned char cc:4;        /* CSRC count */
    unsigned char x:1;         /* header extension flag */
    unsigned char p:1;         /* padding flag */
    unsigned char version:2;   /* protocol version */
    unsigned char pt:7;        /* payload type */
    unsigned char m:1;         /* marker bit */
#endif

    uint16_t seq;              /* sequence number */
    uint32_t ts;               /* timestamp */
    uint32_t ssrc;             /* synchronization source */
   //uint32_t csrc[1];          /* optional CSRC list */
} rtp_hdr_t;


// packet for RTP output
typedef struct
{
  uint32_t id;
  uint16_t major_type;
  uint16_t minor_type;

  double x, y, w, h; // meters 
  double th; // radians

  size_t len;

} __attribute ((packed)) device_hdr_t;

class CRTPPlayer
{
 public:
  rtp_hdr_t hdr;
  struct sockaddr_in dest;
  int sockfd;

  CRTPPlayer( uint32_t ip, uint16_t port );
  CRTPPlayer( char* address );
  void SendData( uint8_t datatype, void *data, int len, uint32_t timestamp );
  void Test( void );
};

#include <string.h> // yuk - for memset in the inline methods

class CDevice
{
public:

  rtp_hdr_t rtp;
  device_hdr_t dev;
  char* data;
  
  CDevice( void )
  {
    // default constructor zeroes contents
    memset( &dev, 0, sizeof(dev) );
    memset( &rtp, 0, sizeof(rtp) );
    data = 0;
  }
  
  CDevice( rtp_hdr_t *rtpp, device_hdr_t *devp, char *datap )
  {
    assert( rtpp );
    assert( devp );
    assert( datap );
    
    memcpy( &rtp, rtpp, sizeof(rtp_hdr_t) );
    memcpy( &dev, devp, sizeof(device_hdr_t) );
    
    //printf( "CDevice constructor: dev.len = %d\n", dev.len );

    //static int a = 0;
    if( dev.len > 0 )
      {
	data = new char[ dev.len ];
	//memset( data, a++, dev.len );
	memcpy( data, datap, dev.len );
      }
  }

/*    // copy constructor */
/*    CDevice( const CDevice &orig ) */
/*      { */
/*        assert( &orig.rtp ); */
/*        assert( &orig.dev ); */
       
/*        memcpy( &rtp, &orig.rtp, sizeof(rtp_hdr_t) ); */
/*        memcpy( &dev, &orig.dev, sizeof(device_hdr_t) ); */
/*        memcpy( data, orig.data, orig.dev.len ); */
/*      } */

  ~CDevice( void )
  {
    //printf( "CDevice destructor: dev.len = %d data = %p\n", 
    //    dev.len, data );

    if( data !=0 ) delete[] data;
  }
  
};


#endif

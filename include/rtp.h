///////////////////////////////////////////////////////////////////////////
//
// File: rtp.h
// Author: Richard Vaughan
// Date: 12 March 2002
// Desc: class for transmitting generic RTP data
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/rtp.h,v $
//  $Author: rtv $
//  $Revision: 1.1 $
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

#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


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

class CRTPPlayer
{
 public:
  rtp_hdr_t hdr;
  struct sockaddr_in dest;
  int sockfd;

  CRTPPlayer( uint32_t ip, uint16_t port );
  void SendData( uint8_t datatype, void *data, int len, uint32_t timestamp );
  void Test( void );
};

#endif

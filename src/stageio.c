
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "stage.h"

size_t WritePacket( int fd, char* data, size_t len )
{
  size_t writecnt = 0;
  int thiswritecnt;
  
  printf( "attempting to write a packet of %d bytes\n", len );

  while(writecnt < len )
  {
    thiswritecnt = write( fd, data+writecnt, len-writecnt);
      
    // check for error on write
    if( thiswritecnt == -1 )
      return -1; // fail
      
    writecnt += thiswritecnt;
  }

  //printf( "wrote %d/%d packet bytes\n", writecnt, len );

  return len; //success
}


size_t ReadPacket( int fd, char* buf, size_t len )
{
  printf( "attempting to read a packet of max %d bytes\n", len );

  assert( buf ); // data destination must be good

  int recv = 0;
  // read a header so we know what's coming
  while( recv < (int)len )
  {
    printf( "Reading on %d\n", fd );
    
    /* read will block until it has some bytes to return */
    size_t r = read( fd, buf+recv,  len - recv );
      
    if( r == -1 ) // ERROR
    {
      if( errno != EINTR )
	    {
	      printf( "ReadPacket: read returned %d\n", r );
	      perror( "code" );
	      break;
	    }
    }
    else if( r == 0 ) // EOF
      break; 
    else
      recv += r;
  }      

  printf( "read %d/%d bytes\n", recv, len );

  return recv; // the number of bytes read
}

size_t WriteHeader( int fd, stage_header_type_t type, uint32_t data )
{      
  stage_header_t hdr;
  
  hdr.type = type;
  hdr.data = data;
 
  puts( "attempting to write a header" );

  size_t len = WritePacket( fd, (char*)&hdr, sizeof(hdr) );

  printf( "wrote %d header bytes\n", len );

  return len;
}

size_t WriteCommand( int fd, stage_cmd_t cmd )
{
  puts( "attempting to write a command" );
  size_t len = WriteHeader( fd, StageCommand, cmd );
  printf( "wrote %d command bytes\n", len );
  return len;
}



int ReadHeader( int fd, stage_header_t* hdr  )
{
  return ReadPacket( fd, (char*)hdr, sizeof(stage_header_t) );
}

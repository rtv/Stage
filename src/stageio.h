#ifndef _STAGEIO_H
#define _STAGEIO_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stage.h"
   
   #include "stage.h"
   #include "cwrapper.h"

   int AcceptConnections( void );
   int InitServer( int argc, char** argv );
   int ServiceConnections( void );
   int ReportResults( double simtime );
   void DestroyConnection( int con );

   size_t WritePacket( int fd, char* data, size_t len );
   size_t WriteHeader( int fd, stage_header_t* hdr );
   
   stage_buffer_t* BufferPacket( char* buf, size_t len );
   stage_buffer_t* BufferHeader( stage_header_t* hdr );
   
   size_t ReadPacket( int fd, char* buf, size_t len );
   size_t ReadHeader( int fd, stage_header_t* hdr  );
      
   // returns a file descriptor connected to the Stage server
   int InitStageClient( int argc, char** argv );
   

   // ALL THESE FUNCTIONS RETURN -1 ON FAILURE, ELSE 0
   
   // reads num stage_model_t structs from the fd, and calls the function with each
   int ReadModels( int fd, int num );
   
   // reads len bytes from the fd, parses the buffer and calls the
   // callback with each property change
   int ReadProperties( int fd, size_t len );

   // this adds a header, so it needs a timestamp
   int WriteProperties( int fd, double simtime, char* data, size_t data_len );
   
   // sends a request to create the models. returns -1 on failure, else 0
   int WriteModels( int fd, double simtime, stage_model_t* ent, int count );
   
   // sends a command.  adds a header, so needs a timestamp
   int WriteCommand( int fd, double simtime, stage_cmd_t cmd );

   // buffers a command for sending later. adds a header, so needs a timestamp
   int BufferCommand( double simtime, stage_cmd_t cmd );


#ifdef __cplusplus
 }
#endif 

#endif

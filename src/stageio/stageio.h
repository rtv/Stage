#ifndef _STAGEIO_H
#define _STAGEIO_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stage.h"
   
   typedef int (*command_callback_t)(int, stage_cmd_t*);
   typedef int (*model_callback_t)(int, stage_model_t*);
   typedef int (*property_callback_t)(int, stage_property_t* );
   
   void* malloc_debug( size_t );
   void* realloc_debug( void*,  size_t );
   void free_debug( void* );


   /* EXTERNAL FUNCTIONS - clients should use these */
   
   /* SERVER-ONLY FUNCTIONS */   
   int SIOInitServer( int argc, char** argv );
   int SIOAcceptConnections( void );
   
   /* CLIENT-ONLY FUNCTIONS */
   
   // returns a file descriptor connected to the Stage server
   int SIOInitClient( int argc, char** argv );
   
   /* CLIENT & SERVER FUNCTIONS */
   
   // issue a 'continue' packet to tell the other end we're done talking
   int SIOContinue( int fd, double simtime );
   
   // read until we get a continue on all connections.
   // the functions are called to service incoming requests
   int SIOServiceConnections( command_callback_t, 
			      model_callback_t,
			      property_callback_t );
   
   // temporary...
   int SIOReportResults( double simtime, char* data, size_t len );
   
   // ALL THESE FUNCTIONS RETURN -1 ON FAILURE, ELSE 0
   
   // reads num stage_model_t structs from the fd, and calls the function with each
   int SIOReadModels( int fd, int num, model_callback_t callback );
   
   // reads len bytes from the fd, parses the buffer and calls the
   // callback with each property change
   int SIOReadProperties( int fd, size_t len, property_callback_t callback  );

   // this adds a header, so it needs a timestamp
   int SIOWriteProperties( int fd, double simtime, char* data, size_t data_len );
   
   // sends a request to create the models. returns -1 on failure, else 0
   int SIOWriteModels( int fd, double simtime, stage_model_t* ent, int count );
   
   // sends a command.  adds a header, so needs a timestamp
   int SIOWriteCommand( int fd, double simtime, stage_cmd_t cmd );

   // buffers a command for sending later. adds a header, so needs a timestamp
   int SIOBufferCommand( stage_buffer_t* buf, double simtime, stage_cmd_t cmd );

   int SIOBufferProperty( stage_buffer_t* bundle,
			  int id, 
			  stage_prop_id_t type,
			  char* data, size_t len );
   
   stage_buffer_t* SIOCreateBuffer( void );
   void SIOFreeBuffer( stage_buffer_t* bundle );

   /* INTERNAL FUNCTIONS - not intented for clients to call directly */
   void SIODebugBuffer( stage_buffer_t* buf );
   void SIODestroyConnection( int con );

   size_t SIOWritePacket( int fd, char* data, size_t len );
   size_t SIOBufferPacket( stage_buffer_t* buf, char* data, size_t len );
   size_t SIOReadPacket( int fd, char* data, size_t len );
      

#ifdef __cplusplus
 }
#endif 

#endif

#ifndef _STAGEIO_H
#define _STAGEIO_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "stage.h"
     
   typedef int (*stg_data_callback_t)(int, void* data, size_t len, 
				      stage_buffer_t* replies );
   typedef int (*stg_connection_callback_t)(int);
   
   void* malloc_debug( size_t );
   void* realloc_debug( void*,  size_t );
   void free_debug( void* );
   
   
   /* EXTERNAL FUNCTIONS - clients should use these */
   void SIOPackPose( stage_pose_t *pose, double x, double y, double a );

   /* maps enum'd property codes to strings */
   const char* SIOPropString( stage_prop_id_t id );

   /* maps enum'd header codes to strings */
   const char* SIOHdrString( stage_header_type_t type );

   /* returns the number of active connections */
   int SIOGetConnectionCount();

   /* SERVER-ONLY FUNCTIONS */   
   int SIOInitServer( int argc, char** argv );
   int SIOAcceptConnections( void );
   
   /* CLIENT-ONLY FUNCTIONS */
   
   // returns a file descriptor connected to the Stage server
   int SIOInitClient( int argc, char** argv );
   
   /* CLIENT & SERVER FUNCTIONS */
   
   // issue a 'continue' packet to tell the other end we're done talking
   //int SIOContinue( int con, double simtime );
   
   // read until we get a continue on all connections.
   // the functions are called to service incoming requests
   int SIOServiceConnections( stg_connection_callback_t lostconnection_cb,
			      stg_data_callback_t cmd_cb,
			      stg_data_callback_t prop_cb );

   // sends a buffer full of properties and places any replies in the
   // recv buffer (if non-null)
   int SIOPropertyUpdate( int con, double timestamp, 
			  stage_buffer_t* send, stage_buffer_t* recv  );

   // temporary...
   //int SIOReportResults( double simtime, char* data, size_t len );
   
   // given an array of models, ask stage to create them. If
   // successful, returns 0 and fills in the correct ID number in each
   // model
   int SIOCreateModels( int con, double timestamp, stage_model_t* models, int num );
   
   // ALL THESE FUNCTIONS RETURN -1 ON FAILURE, ELSE 0
   
   // reads datalen bytes of data, then passes it in recordlen sized chunks to
   // the callback function
   // function with each one in turn
   int SIOReadData( int con, size_t datalen, size_t recordlen, 
		    stg_data_callback_t callback );
   
   // reads len bytes from the con, parses the buffer and calls the
   // callback with each property change
   int SIOReadProperties( int con, size_t len, stg_data_callback_t callback  );

   // this adds a header, so it needs a timestamp
   int SIOWriteMessage( int con, double simtime, stage_header_type_t type,
			void* data, size_t len );
   
   // buffers a command for sending later. adds a header, so needs a timestamp
   int SIOBufferCommand( stage_buffer_t* buf, double simtime, stage_cmd_t cmd );

   int SIOBufferProperty( stage_buffer_t* bundle,
			  int id, 
			  stage_prop_id_t type,
			  void* data, size_t len,
			  stage_reply_mode_t reply );
   
   stage_buffer_t* SIOCreateBuffer( void );
   void SIOFreeBuffer( stage_buffer_t* bundle );
   void SIOClearBuffer( stage_buffer_t* buf );

   /* INTERNAL FUNCTIONS - not intented for clients to call directly */
   void SIODebugBuffer( stage_buffer_t* buf );
   void SIODestroyConnection( int con );

   size_t SIOWritePacket( int con, void* data, size_t len );
   size_t SIOBufferPacket( stage_buffer_t* buf, void* data, size_t len );
   size_t SIOReadPacket( int con, void* data, size_t len );
      

#ifdef __cplusplus
 }
#endif 

#endif

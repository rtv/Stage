#ifndef _STAGEIO_H
#define _STAGEIO_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stage.h"

size_t WritePacket( int fd, char* data, size_t len );
size_t WriteHeader( int fd, stage_header_t* hdr );
size_t WriteCommand( int fd, stage_cmd_t cmd );

stage_buffer_t* BufferPacket( char* buf, size_t len );
stage_buffer_t* BufferHeader( stage_header_t* hdr );
stage_buffer_t* BufferCommand( stage_cmd_t cmd );

size_t ReadPacket( int fd, char* buf, size_t len );
size_t ReadHeader( int fd, stage_header_t* hdr  );
size_t ReadModels( int fd, stage_model_t* models, int num );

// sends a request to create the models. returns -1 on failure, else 0
int CreateModels( int fd, stage_model_t* ent, int count );

// returns a file descriptor connected to the Stage server
int InitStageClient( int argc, char** argv );

#ifdef __cplusplus
 }
#endif 

#endif

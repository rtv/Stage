
#include "stage.h"

size_t WritePacket( int fd, char* data, size_t len );
size_t ReadPacket( int fd, char* buf, size_t len );
size_t WriteHeader( int fd, stage_header_type_t type, uint32_t data );
size_t WriteCommand( int fd, stage_cmd_t cmd );
size_t ReadHeader( int fd, stage_header_t* hdr  );


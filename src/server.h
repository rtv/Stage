
#ifndef STAGE_SERVER_H
#define STAGE_SERVER_H

#define STG_DEFAULT_SERVER_PORT 6601
#define STG_MAX_CONNECTIONS 128
#define STG_HOSTNAME_MAX  128
#define STG_LISTENQ  128


int AcceptConnections( void );
int InitServer( int argc, char** argv );
int ReadFromConnections( void );

#endif


#ifndef STAGE_SERVER_H
#define STAGE_SERVER_H

#ifdef __cplusplus
 extern "C" {
#endif 

int AcceptConnections( void );
int InitServer( int argc, char** argv );
int ServiceConnections( void );

#ifdef __cplusplus
 }
#endif 

#endif


#ifndef STAGE_SERVER_H
#define STAGE_SERVER_H

#ifdef __cplusplus
 extern "C" {
#endif 

   #include "stage.h"
   #include "cwrapper.h"

   int AcceptConnections( void );
   int InitServer( int argc, char** argv );
   int ServiceConnections( void );
   int ReportResults( double simtime );

#ifdef __cplusplus
 }
#endif 

#endif

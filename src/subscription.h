
#ifndef _SUBSCRIPTION_H
#define _SUBSCRIPTION_H

#include "stage.h"
#include "server.h"
#include "connection.h"

typedef struct
{
  stg_server_t* server;
  stg_target_t target;

  stg_connection_t* client;

  double interval; // gap between publications (seconds)
  double timestamp; // the last time the data was published (seconds)
} stg_subscription_t;


// SUBSCRIPTION
stg_subscription_t* subscription_create( void );
void  subscription_destroy( stg_subscription_t* sub );
void  subscription_destroy_cb( gpointer sub, gpointer user );
int   subscription_due( stg_subscription_t* sub, double timenow );
void  subscription_print( stg_subscription_t* sub, char* prefix );
void  subscription_print_cb( gpointer value, gpointer user );
void  subscription_update( stg_subscription_t* sub );

#endif

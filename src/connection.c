#include "connection.h"
#include "subscription.h"

void stg_connection_print( connection_t* cli )
{
  printf( " %s:%d fd %d subs %d ",
	  cli->host, cli->port, cli->fd, cli->subs->len );
  
  int i=0;
  for( i=0; i<cli->subs->len; i++ )
    subscription_print( g_ptr_array_index( cli->subs, i ), "" );
  
  // this'll work in glib >= 2.4
  //g_ptr_array_foreach( cli->subs, stg_subscription_print_cb, NULL );

  puts( "" );
}

void stg_connection_print_cb( gpointer key, gpointer value, gpointer user )
{
  stg_connection_print( (connection_t*)value );
}


connection_t* stg_connection_create( void )
{
  PRINT_DEBUG( "creating a connection" );

  connection_t* con = 
    (connection_t*)calloc( sizeof(connection_t), 1 );
  
  con->subs = g_ptr_array_new();
  
  return con;
}


// close the connection and free the memory allocated
void stg_connection_destroy( connection_t* con )
{
  PRINT_DEBUG( "destroying a connection" );

  if( con->fd > 0 ) close(con->fd);
  if( con->host ) free( con->host ); // string

  if( con->subs )
    {
      // free all the subscriptions, then free the array
      //g_ptr_array_foreach( con->subs, free_sub_cb, NULL );
      g_ptr_array_free( con->subs, TRUE ); // also free the contents
    }

      free( con );
} 

// wrappers 
ssize_t stg_connection_write( connection_t* con, void* data, size_t len )
{
  return stg_fd_packet_write( con->fd, data, len );
}

ssize_t stg_connection_write_msg( connection_t* con, stg_msg_t* msg )
{
  return stg_connection_write( con, msg, sizeof(stg_msg_t) + msg->payload_len );
}

size_t stg_connection_read( connection_t* con, void* buf, size_t len )
{
  return stg_fd_packet_read( con->fd, buf, len );
}

void stg_connection_sub_update( connection_t* con )
{ 
  // for each subscription
  int i;
  for( i=0; i<con->subs->len; i++ )
    subscription_update( g_ptr_array_index( con->subs, i ) );
}

void stg_connection_sub_update_cb( gpointer key, gpointer value, gpointer user )
{
  stg_connection_sub_update( (connection_t*)value );
}

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


connection_t* stg_connection_create( server_t* server )
{
  PRINT_DEBUG( "creating a connection" );

  connection_t* con = 
    (connection_t*)calloc( sizeof(connection_t), 1 );
  
  con->subs = g_ptr_array_new();
  
  con->server = server;
  con->worlds_owned = g_array_new( FALSE, TRUE, sizeof(stg_id_t) );
  
  con->outbuf = g_byte_array_new();
  
  return con;
}


void con_world_kill( connection_t* con, stg_id_t world_id )
{  
  printf( "killing world %d\n", world_id );
  server_world_destroy( con->server, world_id );
}

void con_world_kill_cb( gpointer data, gpointer userdata )
{
  con_world_kill( (connection_t*)userdata, *(stg_id_t*)data );  
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
  
  // if this connection created any worlds, kill those worlds

  int w;
  for( w=0; w<con->worlds_owned->len; w++ )
    con_world_kill( con, g_array_index( con->worlds_owned, stg_id_t, w) );
  
  g_array_free( con->worlds_owned, TRUE);

  g_byte_array_free( con->outbuf, TRUE );

  free( con );
} 

// wrappers 
ssize_t stg_connection_write( connection_t* con, void* data, size_t len )
{
  return stg_fd_packet_write( con->fd, data, len );
}

ssize_t stg_connection_write_msg( connection_t* con,  
				  stg_msg_type_t type, 
				  void* data, size_t len )
{
  stg_msg_t* msg = stg_msg_create( type, data, len );

  GByteArray* buf = g_byte_array_new();

  g_byte_array_append( buf, (guint8*)msg, sizeof(stg_msg_t) + msg->payload_len );

  // prepend a package header
  stg_package_t pkg;
  pkg.key = STG_PACKAGE_KEY;
  pkg.payload_len = buf->len;
  
  // a real-time timestamp for performance measurements
  struct timeval tv;
  gettimeofday( &tv, NULL );
  memcpy( &pkg.timestamp, &tv, sizeof(pkg.timestamp ) );
  
  stg_buffer_prepend( buf, &pkg, sizeof(pkg) );
  
  ssize_t retval = stg_connection_write( con, buf->data, buf->len );

  g_byte_array_free( buf, TRUE );

  stg_msg_destroy(msg);

  return retval;
}

size_t stg_connection_read( connection_t* con, void* buf, size_t len )
{
  return stg_fd_packet_read( con->fd, buf, len );
}

void stg_connection_sub_update( connection_t* con )
{ 
  // for each subscription
  int deltas_sent = 0;
  int i;
  for( i=0; i<con->subs->len; i++ )
    deltas_sent += subscription_update( g_ptr_array_index( con->subs, i ) );
  
  // if there are any outbound deltas
  if( con->outbuf->len > 0 )
    {
      // add a sync packet.
      //stg_msg_t*  msg = stg_msg_create( STG_MSG_CLIENT_CYCLEEND, 
      //				STG_RESPONSE_NONE, NULL, 0 );
  //stg_buffer_append_msg( con->outbuf, msg );
  //stg_msg_destroy( msg );
      
      // prepend a package header
      stg_package_t pkg;
      pkg.key = STG_PACKAGE_KEY;
      pkg.payload_len = con->outbuf->len;
      
      // a real-time timestamp for performance measurements
      struct timeval tv;
      gettimeofday( &tv, NULL );
      memcpy( &pkg.timestamp, &tv, sizeof(pkg.timestamp ) );
      
      stg_buffer_prepend( con->outbuf, &pkg, sizeof(pkg) );

      // send the whole buffer in one go
      //printf( "writing the buffer (%d bytes)\n", (int)con->outbuf->len );
      stg_connection_write( con, con->outbuf->data, con->outbuf->len );
      
      // empty the buffer  
      stg_buffer_clear( con->outbuf );
    }
}

void stg_connection_sub_update_cb( gpointer key, gpointer value, gpointer user )
{
  stg_connection_sub_update( (connection_t*)value );
}

void stg_connection_world_own( connection_t* con, stg_id_t wid )
{
  g_array_append_val( con->worlds_owned, wid );
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>

#include <glib.h>

//#define DEBUG

#include "server.h"
#include "world.h"
#include "model.h"
//#include "connection.h"
#include "subscription.h"

#define STG_LISTENQ 100 // max number of pending server connections
#define STG_ID_STRING "Stage multi-robot simulator."


void client_destroy_cb( gpointer obj )
{
  stg_connection_destroy( (connection_t*)obj );
}

server_t* server_create( int port )
{
  server_t* server = calloc( sizeof(server_t), 1 );
  
  server->port = port;
  server->host = strdup( "localhost" );

  //server->running = 0; // we start paused

  // record the time we were started
  server->start_time = stg_timenow();
  
  server->quit = FALSE;

  // todo = write destructor for clients
  server->clients = g_hash_table_new_full( g_int_hash, g_int_equal,
					   NULL, client_destroy_cb );
  
  server->worlds = g_hash_table_new_full( g_int_hash, g_int_equal,
					  NULL, world_destroy_cb );

  // init the connection poll array
  server->pfds = g_array_new( FALSE, FALSE, sizeof(struct pollfd) );
  
  
  // console output
  printf( "[%s:%d...", server->host, server->port );
  fflush(stdout);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  g_assert( fd > 0 );
  
  struct sockaddr_in servaddr;  
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(server->port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( bind(fd, (struct sockaddr*) &servaddr, sizeof(servaddr) )  < 0 )
    {
      perror("failed to bind server socket");      
      printf( "Stage: Port %d is probably in use\n", server->port );
      return NULL; // fail
    }
  
  //PRINT_DEBUG1( "server fd %d", fd );

  // listen for requests on this socket
  g_assert( listen( fd, STG_LISTENQ) == 0 );

  // configure a pollfd 
  struct pollfd pfd;
  memset( &pfd, 0, sizeof(pfd) );
  pfd.fd = fd;
  pfd.events = POLLIN; // notify me when data is available 
  
  // install this pollfd as the zeroth entry in our poll array
  g_array_insert_val( server->pfds, 0, pfd );

  // erase the dots and close the brace
  printf( "\b\b\b   \b\b\b]" );
  
  return server; // success
}


void server_destroy( server_t* server )
{
  if( server )
    {
      if( server->host ) // name string
	free( server->host );
      
      if( server->worlds ) // world table
	g_hash_table_destroy( server->worlds );
      
      if( server->clients ) // client table
	g_hash_table_destroy( server->clients );

      free( server );
    }
}


void server_update_subs( server_t* server )
{
  g_hash_table_foreach( server->clients, stg_connection_sub_update_cb, server );
}  


void server_update_worlds( server_t* server )
{
  g_hash_table_foreach( server->worlds, world_update_cb, server );
}


int server_connection_add( server_t* server, connection_t* con )
{
  assert( server );
  assert( con );
  
  g_hash_table_insert( server->clients, &con->fd, con );
  
  PRINT_DEBUG1( "added connection. total now %d", 
		g_hash_table_size( server->clients ) );
  
  return 0; // ok
}

int server_connection_remove( server_t* server, int fd )
{
  assert( server );
  assert( server->clients );
  
  g_hash_table_remove( server->clients, &fd );  
  
  PRINT_DEBUG1( "removed connection. total now %d",
		g_hash_table_size( server->clients ));
  return 0;
}

int server_accept_connection( server_t* server )
{
  // set up a socket for this connection
  struct sockaddr_in cliaddr;  
  bzero(&cliaddr, sizeof(cliaddr));
  socklen_t clilen;
  clilen  = sizeof(cliaddr);
  
  // get the server poll structure
  struct pollfd* server_pfd = &g_array_index( server->pfds, struct pollfd, 0 );

  assert( server_pfd );
  
  int connfd = accept( server_pfd->fd, 
		       (struct sockaddr*) &cliaddr, 
		       &clilen);
  
  g_assert( connfd > 0 );
  
  char* grt = "Welcome to Stage-1.4";
  write( connfd, grt, strlen(grt)+1 );
 
  // look for a well-formed greeting
  stg_greeting_t greet;
  
  PRINT_DEBUG2( "expecting %d byte greeting on %d", 
		(int)sizeof(greet), connfd );
  
  int res = 0;
  res = read( connfd, (char*)&greet, sizeof(greet) );
  
  if( res < 0 ) perror("");
  if( res < sizeof(greet) ) stg_err( "greeting read failed" );
  
  // check to make sure we have the right greeting code
  if( greet.code != STG_SERVER_GREETING )
    {
      PRINT_ERR1( "bad greeting from client (%d)", greet.code );
      close( connfd );
      return 1;
    }
  
  // write the reply
  stg_connect_reply_t reply;
  
  reply.code = STG_CLIENT_GREETING;
  snprintf( reply.id_string, STG_ID_STRING_MAX,
	    "%s version %s", STG_ID_STRING, VERSION );
  
  // todo - get these from autoconf via config.h
  reply.vmajor = 1;
  reply.vminor = 4;
  reply.vmicro = 0;

  write( connfd, (char*)&reply, sizeof(reply) );
  
  printf( "Accepted connection from %s\n",
	  "<todo>" );

  connection_t* con = stg_connection_create( server );
  con->fd = connfd;
  con->host = strdup( "<hostname>" );
  con->port = 9000;

  server_connection_add( server, con );

  // configure a pollfd
  struct pollfd pfd;
  memset( &pfd, 0, sizeof(pfd) );
  pfd.fd = connfd;
  pfd.events = POLLIN;
  
  // and add it to the server's pollfd array
  g_array_append_val( server->pfds, pfd );
  
  int i;
  for( i=0; i<server->pfds->len; i++ )
    PRINT_DEBUG2( "pfd %d poll fd %d", 
		  i, g_array_index( server->pfds, int, i ) );

  return 0; //ok
}

int server_poll( server_t* server )
{      
  //PRINT_DEBUG1( "polling all %d pollfds", server->pfds->len );
  
  int poll_result = 1;
  int error = 0; // this gets returned. nonzero indicates error
  
  // while there is any pending data to read on any fd
  while( poll_result > 0 )
    {
      struct pollfd* pfds = (struct pollfd*)server->pfds->data;
      int numpfds = server->pfds->len;
      
      // poll will sleep for the minimum time
      if( (poll_result = poll( pfds, numpfds, 1 )) > 0 )
	{	 
	  // loop through the pollfds looking for the POLLINs
	  int i;
	  for( i=0; i<numpfds; i++ )
	    {
	      if( pfds[i].revents & POLLIN )
		{		  
		  if( i == 0 ) // if it's the master server port
		    {
		      PRINT_DEBUG( "new connection poll in!" );
		      if( server_accept_connection( server ) )
			stg_err( "server failed to accept a new connection" );
		      
		      server_print( server );
		    }
		  else // it's a client
		    {
		      //PRINT_DEBUG1( "pollin on client fd %d", fd );
		      
		      int fd = pfds[i].fd;
		      
		      stg_package_t* pkg = calloc( sizeof(stg_package_t), 1 );
		      
		      ssize_t res = 
			stg_fd_packet_read( fd, pkg, sizeof(stg_package_t) );
		      
		      if( res != sizeof(stg_package_t) )
			{
			  PRINT_ERR2( "failed to read package header (%d/%d bytes)\n",
				      res, (int)sizeof(stg_package_t) );
			  error = 1;
			  free( pkg );
			  
			  // it's not safe to continue the loop on the array
			  // any more, so we'll drop out here. we'll be back
			  // very soon so it's not a problem.			  
			  break;
			}
		      
		      PRINT_DEBUG4( "package key:%d timestamp:%d.%d len:%d\n",
				    pkg->key, 
				    pkg->timestamp.tv_sec, 
				    pkg->timestamp.tv_usec, 
				    (int)(pkg->payload_len) );
		      
		      if( pkg->key != STG_PACKAGE_KEY )
			{
			  PRINT_ERR2( "package has incorrect key (%d not %d)\n",
				      pkg->key, STG_PACKAGE_KEY );
			  error = 2;
			  free( pkg );
			  break;
			}
		      
		      // read the body of the package
		      size_t total_size = sizeof(stg_package_t) + pkg->payload_len;
		      pkg = realloc( pkg, total_size );
		      
		      res = stg_fd_packet_read( fd, 
						pkg->payload, 
						pkg->payload_len );
		      
		      if( res != pkg->payload_len )
			{
			  PRINT_ERR2( "failed to read package payload (%d/%d bytes)\n",
				      res,(int)pkg->payload_len );
			  error = 3;
			  free( pkg );
			  break;
			}
		      
		      if( pkg  )
			{
			  PRINT_DEBUG1( "parsing a package from fd %d.",
					fd );
			  
			  server_package_parse( server, fd, pkg );
			  free( pkg );
			}
		      else
			{
			  error = 4; // shouldn't get this error at all
			  break;
			}
		      
		    }
		}
	    }
	  
	  if( error )
	    {
	      PRINT_DEBUG1( "read failure on fd %d. Shutting it down",
			    pfds[i].fd );
	      
	      // remove this fd's entry in the poll array
	      server->pfds = g_array_remove_index_fast( server->pfds, i );
	      
	      // zap the client on this fd.
	      g_hash_table_remove( server->clients, &pfds[i].fd  );
	      
	      server_print( server );
	    }
	}     
    }
  
  if( poll_result < 0 )
    {
#ifdef DEBUG
      perror( "server polling connections" );
#endif
    }
  
  return 0; //ok
}


// break the package into individual messages and handle them
int server_package_parse( server_t* server, int fd, stg_package_t* pkg )
{
  assert( server );
  assert( pkg );
  
  stg_msg_t* msg = (stg_msg_t*)pkg->payload;
  
  while( (char*)msg < (pkg->payload + pkg->payload_len) )
    {
      // eat this message
      server_msg_dispatch( server, fd, msg );
      
      // jump to the next message in the buffer
      (char*)msg += sizeof(stg_msg_t) + msg->payload_len;
    }

  return 0; // ok
}


int server_subscribe( server_t* server, int fd, stg_sub_t* sub )
{
  // find the client
  connection_t* client = g_hash_table_lookup( server->clients, &fd );
  assert( client );
  
  // find the model 
  world_t* world = g_hash_table_lookup( server->worlds, &sub->world );
  
  if( world == NULL )
    PRINT_WARN1( "subscription failed - no such world %d", sub->world );
  else
    {
      model_t* model = g_hash_table_lookup( world->models, &sub->model );
      
      if( model == NULL )
	PRINT_WARN1( "subscription failed - no such model %d", sub->model );
      else
	{
	  // register this subscription with the model
	  model_subscribe( model, sub->prop );

	  subscription_t* ssub = subscription_create();
	  
	  ssub->target.world = sub->world; 
	  ssub->target.model = sub->model; 
	  ssub->target.prop = sub->prop; 
	  ssub->interval = sub->interval;

	  ssub->timestamp = 0;
	  ssub->client = client;
	  ssub->server = server;

	  g_ptr_array_add( client->subs, ssub );  
	}
    }
  
  return 0; // ok
}


gboolean stg_targets_match( stg_target_t* t1, stg_target_t* t2 )
{
  if( t1 == NULL || t2 == NULL )
    return FALSE;
  
  if( memcmp( t1, t2, sizeof(stg_target_t) ) == 0 )
    return TRUE;

  return FALSE;
}

int server_unsubscribe( server_t* server, 
			int fd, stg_unsub_t* unsub )
{
  PRINT_DEBUG4( "con %d unsubscribing from to %d:%d:%s",
		fd, 
		unsub->world, 
		unsub->model, 
		stg_property_string(unsub->prop) );
  
  
  connection_t* client = g_hash_table_lookup( server->clients, &fd );
  assert( client );
  
  int i;
  int len = client->subs->len;

  printf( "checking subscriptions" );

  stg_target_t target;
  target.world = unsub->world;
  target.model = unsub->model;
  target.prop = unsub->prop;

  model_t* mod = server_get_model( server, target.world, target.model );
  if( mod == NULL )
    PRINT_WARN2( "strange unsubscription: no such model %d:%d",
		 target.world, target.model );
  else
    // unregister this sub with the model
    model_unsubscribe( mod, target.prop );

  for( i=0; i<len; i++ )
    {	
      //
      subscription_t* candidate = 
	g_ptr_array_remove_index( client->subs, 0 );
      
      /* printf( "%d:%d:%d compared with %d:%d:%d\n",
	      candidate->model->world->id,
	      candidate->model->id,
	      candidate->type,
	      target->world,
	      target->model,
	      target->type );
      */

      // if this subscription does NOT match the doomed one, put it
      // back in the array. 
      if( !stg_targets_match( &candidate->target, &target ) )
	{	  
	  //puts( "NO MATCH" );
	  g_ptr_array_add( client->subs, candidate );
	}
      //else
      //puts( "MATCH" );
    }

  return 0; // ok
}



// add a world to the server
stg_id_t server_world_create( server_t* server, 
			      connection_t* con, 
			      stg_createworld_t* cw )
{
  
  //if( g_hash_table_lookup( server->worlds, &id ) )
  //{
  //  PRINT_WARN1( "world %d already exists. Ignoring create request.", id );
  //  return 1; // error
  //}
  
  // find the lowest integer that has not yet been assigned to a world
  stg_id_t candidate = 0;
  while( g_hash_table_lookup( server->worlds, &candidate ) )
    candidate++;

  PRINT_DEBUG2( "creating world \"%s\" id %d", cw->token, candidate );
  
  world_t* world = world_create( server, con, candidate, cw );
  
  assert( world );
  
  g_hash_table_replace( server->worlds, &world->id, world );
  
  return candidate; 
}


int server_world_destroy( server_t* server, stg_id_t id )
{
  PRINT_DEBUG1( "destroying world %d", id );
  
  if( g_hash_table_remove( server->worlds, &id ) == 0 )
    {
      PRINT_WARN1( "attempt to remove non-existent world %d", id );
      return 1; // fail
    }
  return 0; // ok
}



void server_handle_msg( server_t* server, int fd, stg_msg_t* msg )
{
  PRINT_DEBUG( "received a message for the Stage server" );
  
  switch( msg->type )
    {
    case STG_MSG_SERVER_WORLDCREATE:
      {
	// create the world, fill in the resulting id and send the message back
	
	// look up the client that created this world
	connection_t* con = (connection_t*)g_hash_table_lookup( server->clients,
								&fd );
	assert(con);
	
	stg_id_t wid = server_world_create( server, con,
					    (stg_createworld_t*)msg->payload );

	// add this world to this client's list of owned worlds
	stg_connection_world_own( con, wid );
	
	printf( "replying with world id %d (%d bytes)\n",
		wid, sizeof(wid) );
	
	stg_fd_msg_write( fd, STG_MSG_CLIENT_REPLY, &wid, sizeof(wid) );
      }
      break;
      
    case STG_MSG_SERVER_WORLDDESTROY:
      server_world_destroy( server, *(stg_id_t*)msg->payload );
      break;
      
    case STG_MSG_SERVER_SUBSCRIBE:
      {
	stg_sub_t* sub = (stg_sub_t*)msg->payload;
	
	PRINT_DEBUG4( "client subscribed to %d:%d:%s interval %lu",
		      sub->world, sub->model, 
		      stg_property_string(sub->prop ),
		      sub->interval );
	
	if( server_subscribe( server, fd, sub ) != 0 )
	  PRINT_WARN3( "subscription to %d:%d:%d failed",
		       sub->world, sub->model, sub->prop );

      }
      break;
      
    case STG_MSG_SERVER_UNSUBSCRIBE:
      {
	stg_unsub_t* unsub = (stg_unsub_t*)msg->payload;
	
	if( server_unsubscribe( server, fd, unsub ) != 0 )
	  PRINT_WARN3( "unsubscription from %d:%d:%d failed",
		       unsub->world, unsub->model, unsub->prop );
      }
      break;

      /*
    case STG_MSG_SERVER_PAUSE:
      server->running = 0;
      break;

    case STG_MSG_SERVER_RUN:
      server->running = 1;
      break;
      */

    default:
      PRINT_WARN1( "Ignoring unrecognized server message subtype %d.", 
		   msg->type & STG_MSG_MASK_MINOR );
      break;	       		 
    }
  
  //server_print(server);
}



void client_remove_subs_of_model( connection_t* client, stg_target_t* tgt )
{
  if( client == NULL )
    PRINT_WARN( "no client to remove subscriptions" );
  /* else 
     (this code will work in glib-2.4 - not very common yet, so
     we'll do it the hard way for now)
  
     g_hash_table_foreach_remove( client->subs, sub_matches_model, user );
  */

  // loop through the pointer array, removing any subscriptions to this model 
  
  int len = client->subs->len;
  
  PRINT_DEBUG2( "removing subscriptions of model %d:%d", 
		tgt->world, tgt->model );
  
  int i;
  for( i=0; i<len; i++ )
    {	
      //
      subscription_t* candidate = 
	g_ptr_array_remove_index( client->subs, 0 );
      
      /* printf( "%d:%d:%d compared with %d:%d:%d\n",
	 candidate->model->world->id,
	 candidate->model->id,
	 candidate->type,
	 target->world,
	 target->model,
	 target->type );
      */
      
      // if this subscription does NOT match the doomed one, put it
      // back in the array. 
      if( !(candidate->target.world == tgt->world && 
	    candidate->target.model == tgt->model) )
	{	  
	  g_ptr_array_add( client->subs, candidate );
	}
    }
}


void client_remove_subs_of_model_cb( gpointer key, gpointer value, gpointer user )
{
  client_remove_subs_of_model( (connection_t*)value, (stg_target_t*)user );
}
  

void server_remove_subs_of_model( server_t* server, stg_target_t* tgt )
{
  // delete the model's subscriptions
  g_hash_table_foreach( server->clients, 
			client_remove_subs_of_model_cb, tgt );
}

double server_world_time( server_t* server, stg_id_t wid )
{
  world_t* wld = server_get_world( server, wid );
  
  if( wld ) 
    {
      //PRINT_DEBUG2( "world %d sim_time %.3f", wld->id, wld->sim_time );
      return wld->sim_time;
    }
  else
    return -1.0; // indicates no-world failure
}

world_t* server_get_world( server_t* server, stg_id_t wid )
{
  return( server ? g_hash_table_lookup( (gpointer)server->worlds, &wid ) : NULL );
}

model_t* server_get_model( server_t* server, stg_id_t wid, stg_id_t mid )
{
  world_t* world = server_get_world( server, wid );
  
  if( world == NULL )
    {
      PRINT_WARN1( "no such world %d", wid );
      return NULL;
    }

  model_t* model = world_get_model( world, mid );

  if( model == NULL )
    {
      PRINT_WARN1( "no such model %d", mid );
      return NULL;
    }
  
  return  model;
  
  //return( world_get_model( server_get_world( server, wid ), mid ) );
}

int server_get_prop( server_t* server,
		     stg_id_t wid, stg_id_t mid, stg_id_t pid, 
		     void** data, size_t *len )
{
  return model_get_prop( world_get_model(server_get_world(server,wid),mid),
			 pid,data,len);
}

void server_print( server_t* server )
{ 
  printf( "--\nStage-%s\nClients (%d):\n", 
	  VERSION, 
	  g_hash_table_size( server->clients ) );
  
  g_hash_table_foreach( server->clients, stg_connection_print_cb, NULL );
  
  printf( "Worlds (%d):\n", g_hash_table_size( server->worlds ) );
  
  g_hash_table_foreach( server->worlds, world_print_cb, NULL );

  //printf( "Properties (%d):\n", g_hash_table_size( server->props ) );
  //g_hash_table_foreach( server->props, stg_property_print_cb, NULL );
  
  puts( "--" );
}

int server_msg_dispatch( server_t* server, int fd, stg_msg_t* msg )
{
  //PRINT_DEBUG3( "handling message type %d.%d len %d", 
  //	msg->type >> 8,
  //	msg->type & STG_MSG_MASK_MINOR,
  //	(int)msg->payload_len );
  
  switch( msg->type & STG_MSG_MASK_MAJOR )
    {
    case STG_MSG_SERVER: 
      server_handle_msg( server, fd, msg );
      break;
      
    case STG_MSG_WORLD:
      {
	// first part of any world message should be a world id
	stg_id_t world_id = *(stg_id_t*)msg->payload;
	
	//printf( "Message for world id %d\n", world_id );
	
      	world_t* world = server_get_world( server, world_id );
	
	if( world )
	  world_handle_msg( world, fd, msg );
	else
	  PRINT_WARN1( "Ignoring message for non-existent world %d.", 
		       world_id ); 
	
      }
      break;
      
    case STG_MSG_MODEL: 
      {
	stg_target_t* tgt = (stg_target_t*)msg->payload;	

	PRINT_DEBUG2( "looking up model %d:%d", tgt->world, tgt->model ); 

	model_t* model = server_get_model( server, 
					       tgt->world,
					       tgt->model );
	if( model )
	  model_handle_msg( model, fd, msg );
	else
	  PRINT_WARN2( "Ignoring message for non-existent model %d:%d.", 
		       tgt->world, tgt->model ); 
	
      }
      break;
      
    case STG_MSG_CLIENT:
      PRINT_WARN( "Ignoring client message." ); 
      break;
      
      
    default:
      PRINT_WARN1( "Ignoring unrecognized message type %d.", msg->type ); 
      break;
    }
  
  return 0;
} 
    


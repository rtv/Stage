
#include <stdlib.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
//#include <pthread.h>

//#define DEBUG

#include "stage.h"

static int static_next_id = 0;

// function declarations for use inside this file only (mostly
// wrappers for use as callbacks)
void stg_world_pull_cb( gpointer key, gpointer value, gpointer user );
void stg_world_destroy_cb( gpointer key, gpointer value, gpointer user );
void stg_catch_pipe( int signo );



// types used only in this file
typedef struct
{
  stg_client_t* client;
  stg_id_t world_id_server;
  stg_id_t model_id_server;
} stg_prop_target_t;


stg_client_t* stg_client_create( void )
{
  stg_client_t* client = calloc( sizeof(stg_client_t), 1 ); 
  
  assert( client );

  client->host = strdup(STG_DEFAULT_SERVER_HOST);
  client->port = STG_DEFAULT_SERVER_PORT;

  client->worlds_id = g_hash_table_new( g_int_hash, g_int_equal );
  client->worlds_id_server = g_hash_table_new( g_int_hash, g_int_equal );
  client->worlds_name = g_hash_table_new( g_str_hash, g_str_equal );
  
  // init the pollfd
  memset( &client->pfd, 0, sizeof(client->pfd) );
  
  //pthread_mutex_init( &client->models_mutex, NULL );
  //pthread_mutex_init( &client->reply_mutex, NULL );
  //pthread_cond_init( &client->reply_cond, NULL );
  
  client->reply = g_byte_array_new();
  client->reply_ready = FALSE;

  // the thread isn;t kicked off until connect is called.

  return client;
}


/* // kick off the client thread */
/* int stg_client_thread_start( stg_client_t* client ) */
/* { */
/*   return pthread_create( &client->thread, NULL, &stg_client_thread, client); */
/* } */

/* void stg_mutex_error( int err ) */
/* { */
/*   switch( err ) */
/*     { */
/*     case EINVAL: PRINT_ERR( "invalid mutex" ); break; */
/*     case EFAULT: PRINT_ERR( "invalid mutex pointer" ); break; */
/*     case EDEADLK: PRINT_ERR( "mutex deadlock" ); break; */
/*     default: */
/*       PRINT_ERR1( "unknown mutex error %d", err ); */
/*     } */
/*   exit( -1 ); // a mutex error is bad. we surrender */
/* } */

/* void stg_mutex_lock( pthread_mutex_t* mutex ) */
/* { */
/*   int retval; */
/*   if( (retval = pthread_mutex_lock( mutex )) ) */
/*     stg_mutex_error( retval ); */
/* } */

/* void stg_mutex_unlock( pthread_mutex_t* mutex ) */
/* { */
/*   int retval; */
/*   if( (retval = pthread_mutex_unlock( mutex )) ) */
/*     stg_mutex_error( retval ); */
/* } */

/* void stg_client_thread( void* data ) */
/* {  */
/*   PRINT_DEBUG( "StageClient thread kicks off" ); */
  
/*   stg_client_t* client = (stg_client_t*)data; */

 
/*   while( 1 ) */
/*     {  */
/*       // test if we are supposed to cancel */
/*       pthread_testcancel(); */
 
/*       stg_client_update( client ); */
/*     }      */
    
/* } */

int stg_client_read( stg_client_t* client, int sleeptime )
{
  int err;
  
  // read a package - asking poll() to sleep briefly
  stg_package_t* pkg = 
    stg_client_read_package( client, sleeptime,  &err );
    
  if( pkg )
    {
      PRINT_DEBUG4( "player: stage package key:%d timestamp:%d.%d len:%d",
		    pkg->key, 
		    pkg->timestamp.tv_sec, 
		    pkg->timestamp.tv_usec,
		    pkg->payload_len );
	  
      // some timing stuff for performance testing

      //struct timeval tv;
      //gettimeofday( &tv, NULL );
      //printf( "arrival time %d.%d\n",
      //  tv.tv_sec, tv.tv_usec );
	  
      //double sent = pkg->timestamp.tv_sec + 
      //(double)(pkg->timestamp.tv_usec)/1e6; 
	  
      //double recvd = tv.tv_sec + 
      //(double)(tv.tv_usec)/1e6; 
	  
      //printf( "transport time: %.4f\n", recvd - sent );
	  
      // unpack the package and absorb its deltas	  
      stg_client_package_parse( client, pkg );	  
    }
  else if( err > 0 )
    {
      PRINT_ERR1( "Fatal error: failed to read a Stage package "
		  "(code %d). Quitting\n",
		  err );      
      exit( -1 );
    }
  
  return 0; // ok
}

void stg_client_destroy( stg_client_t* cli )
{
  PRINT_DEBUG( "destroying client" );

  // TODO - one of these should call world destructors
  if( cli->worlds_id ) g_hash_table_destroy( cli->worlds_id );
  if( cli->worlds_name ) g_hash_table_destroy( cli->worlds_name );
  if( cli->pfd.fd > 0 ) close( cli->pfd.fd );

  g_byte_array_free( cli->reply, TRUE );

  free(cli);
}

int stg_client_connect( stg_client_t* client, const char* host, const int port )
{
  PRINT_DEBUG( "client connecting" );

  // if host or port was specified, override the client's current values
  if( host ) 
    { 
      free(client->host); 
      client->host = strdup(host); 
    }
  
  if( port ) 
    client->port = port;
  
  PRINT_DEBUG2( "connecting to server %s:%d", host, port );
  
  // get the IP of our host
  struct hostent* info = gethostbyname(  host );
  
  if( info )
    { // make sure this looks like a regular internet address
      assert( info->h_length == 4 );
      assert( info->h_addrtype == AF_INET );
    }
  else
    {
      PRINT_ERR1( "failed to resolve IP for remote host\"%s\"", 
		  host );
      return NULL;
    }
  struct sockaddr_in servaddr;
  
  /* open socket for network I/O */
  client->pfd.fd = socket(AF_INET, SOCK_STREAM, 0);
  client->pfd.events = POLLIN; // notify me when data is available
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( client->pfd.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( client->pfd.fd < 0 )
    {
      stg_err( "Error opening network socket for connection"  );
      return NULL;
    }
  
  /* setup our server address (type, IP address and port) */
  bzero(&servaddr, sizeof(servaddr)); /* initialize */
  servaddr.sin_family = AF_INET;   /* internet address space */
  servaddr.sin_port = htons( port ); /*our command port */ 
  memcpy(&(servaddr.sin_addr), info->h_addr_list[0], info->h_length);
  
  /*
    if( signal( SIGPIPE, stg_catch_pipe ) == SIG_ERR )
    {
    PRINT_ERR( "error setting pipe signal catcher" );
    exit( -1 );
    }
  */
  
  if( connect( client->pfd.fd, 
               (struct sockaddr*)&servaddr, sizeof( servaddr) ) == -1 )
    {
      perror( "attempting to connect to Stage." );
      PRINT_ERR2( "Connection failed on %s:%d",
		  host, port ); 
      PRINT_ERR( "Did you forget to start Stage?");
      fflush( stdout );
    return NULL;
    }
  
  // read Stage's server string
  char buf[100];
  buf[0] = 0;
  read( client->pfd.fd, buf, 100 );
  printf( "[Stage says: \"%s\"]", buf ); fflush(stdout);
 
  // send the greeting
  stg_greeting_t greeting;
  greeting.code = STG_SERVER_GREETING;
  
  int r;
  if( (r = write( client->pfd.fd, &greeting, sizeof(greeting) )) < 1 )
    {
      PRINT_ERR( "failed to write STG_SERVER_GREETING to server." );
      if( r < 0 ) perror( "error on write" );
      return NULL;
    }
  
  stg_connect_reply_t reply;
  // wait for the reply
  if( (r =  stg_fd_packet_read( client->pfd.fd, &reply, sizeof(reply) )) < 1 )
    {
      PRINT_ERR( "failed to READ STG_CLIENT_GREETING from server." );
      if( r < 0 ) perror( "error on read" );
      return NULL;
    }
  
  if( reply.code != STG_CLIENT_GREETING ) 
    PRINT_ERR1( "received bad reply from server (%d)", greeting.code );
  else
    PRINT_DEBUG1( "received good reply from server (%d)", greeting.code );
  
  // announce the connection on the console
  PRINT_DEBUG4( "Connected to \"%s\" (version %d.%d.%d)",
	      reply.id_string, reply.vmajor, reply.vminor, reply.vmicro );
  

    
  return 0; //OK
}

void stg_client_pull( stg_client_t* cli )
{
  PRINT_DEBUG( "pulling everything from client" );
  
  // ask the server to close each of our worlds
  g_hash_table_foreach( cli->worlds_id, stg_world_pull_cb, cli );
}

stg_model_t* stg_client_get_model( stg_client_t* cli, 
				   const char* wname, const char* mname )
{
  stg_world_t* world = g_hash_table_lookup( cli->worlds_name, wname );

  if( world == NULL )
    {
      PRINT_DEBUG1("no such world \"%s\"", wname );
      return NULL;
    }

  stg_model_t* model = g_hash_table_lookup( world->models_name, mname );
  
  if( model == NULL )
    {
      PRINT_DEBUG2("no such model \"%s\" in world \"%s\"", mname, wname );
      return NULL;
    }
  
  return model;
} 

stg_model_t* stg_client_get_model_serverside( stg_client_t* cli, stg_id_t wid, stg_id_t mid )
{
  stg_world_t* world = g_hash_table_lookup( cli->worlds_id_server, &wid );
  
  if( world == NULL )
    {
      PRINT_DEBUG1("no such world with server-side id %d", wid );
      return NULL;
    }
  
  stg_model_t* model = g_hash_table_lookup( world->models_id_server, &mid );
  
  if( model == NULL )
    {
      PRINT_DEBUG2("no such model with server-side id %d in world %d", 
		   mid, wid );
      return NULL;
    }
  
  return model;
}



int stg_client_write_msg( stg_client_t* cli, 
			  stg_msg_type_t type, 
			  void* data, size_t datalen )
{
  return stg_fd_msg_write(  cli->pfd.fd, type, data, datalen );  
}


int stg_model_subscribe( stg_model_t* mod, stg_id_t prop, stg_msec_t interval )
{
  assert( mod );
  assert( mod->world );
  assert( mod->world->client );
  
  stg_sub_t sub;
  sub.world = mod->world->id_server;
  sub.model = mod->id_server;  
  sub.prop = prop;
  sub.interval = interval;
  
  stg_client_write_msg( mod->world->client, 
			STG_MSG_SERVER_SUBSCRIBE, 
			&sub, sizeof(sub) );
  
  return 0;
}


int stg_model_unsubscribe( stg_model_t* mod, stg_id_t prop )
{
  assert( mod );
  assert( mod->world );
  assert( mod->world->client );
  
  stg_unsub_t sub;
  sub.world = mod->world->id_server;
  sub.model = mod->id_server;  
  sub.prop = prop;
  
  stg_client_write_msg( mod->world->client, 
			STG_MSG_SERVER_UNSUBSCRIBE, 
			&sub, sizeof(sub) );
  
  return 0;
}


stg_world_t* stg_client_createworld( stg_client_t* client, 
				     int section, 
				     stg_token_t* token, 
				     double ppm, 
				     stg_msec_t interval_sim, 
				     stg_msec_t interval_real )
{
  stg_world_t* w = calloc( sizeof(stg_world_t), 1 );

  w->client = client;
  w->id_client = static_next_id++;
  w->id_server = 0;
  w->section = section;
  w->token = token;
  
  w->interval_sim = interval_sim;
  w->interval_real = interval_real;
  w->ppm = ppm;

  //w->props = g_hash_table_new( g_int_hash, g_int_equal );
  w->models_id = g_hash_table_new( g_int_hash, g_int_equal );
  w->models_id_server = g_hash_table_new( g_int_hash, g_int_equal );
  w->models_name = g_hash_table_new( g_str_hash, g_str_equal );
  
  PRINT_DEBUG2( "created world %d \"%s\"", 
		w->id_client, w->token->token );
  

  // index this new world in the client
  g_hash_table_replace( client->worlds_id, &w->id_client, w );
  g_hash_table_replace( client->worlds_name, w->token->token, w );

  // server id is useless right now.
  //g_hash_table_replace( cli->worlds_id_server, &world->id_server, world );
  
  return w;
} 

stg_model_t* stg_world_model_name_lookup( stg_world_t* world, 
					  char* modelname )
{
  return( (stg_model_t*)g_hash_table_lookup( world->models_name, modelname ));
}

stg_model_t* stg_world_createmodel( stg_world_t* world, 
				    stg_model_t* parent, 
				    int section,
				    stg_token_t* token )
{ 
  
  stg_model_t* mod = calloc( sizeof(stg_model_t), 1 );
  
  mod->id_client = static_next_id++;
  mod->id_server = 0;
  mod->section = section;
  mod->token = token;
  mod->world = world;
  mod->parent = parent;
  mod->props = g_hash_table_new( g_int_hash, g_int_equal );

  PRINT_DEBUG3( "created model %d:%d \"%s\"", 
		world->id_client, mod->id_client, mod->token->token );

  // index this new model in it's world
  g_hash_table_replace( world->models_id, &mod->id_client, mod );
  g_hash_table_replace( world->models_name, mod->token->token, mod );

  // server id is useless at this stage
  //g_hash_table_replace( world->models_id_server, &model->id_server, model );

  return mod;
}


void stg_model_destroy( stg_model_t* mod )
{
  if( mod == NULL )
    {
      PRINT_WARN( "attempting to destroy NULL model" );
      return;
    }
  
  PRINT_DEBUG2( "destroying model %d (%d)", mod->id_client, mod->id_server );

  //g_hash_table_remove( mod->world->models_id, &mod->id_client );
  //g_hash_table_remove( mod->world->models_name, mod->token->token );
  if( mod && mod->props ) g_hash_table_destroy( mod->props );
  if( mod ) free( mod );
}

void stg_world_destroy( stg_world_t* world )
{
  PRINT_DEBUG2( "destroying world %d (%d)", world->id_client, world->id_server );

  if( world == NULL )
    {
      PRINT_WARN( "attempting to destroy NULL world" );
      return;
    }

  if( world->models_id ) g_hash_table_destroy( world->models_id );
  if( world->models_id_server ) g_hash_table_destroy( world->models_id_server );
  if( world->models_name ) g_hash_table_destroy( world->models_name );
  free( world);
}

void stg_world_resume( stg_world_t* world )
{
  stg_id_t world_id = world->id_server;
  stg_client_write_msg( world->client, 
			STG_MSG_WORLD_RESUME,
			&world_id, sizeof(world_id) );
}

void stg_world_pause( stg_world_t* world )
{
  stg_id_t world_id = world->id_server;
  stg_client_write_msg( world->client, 
			STG_MSG_WORLD_PAUSE,
			&world_id, sizeof(world_id) );
}

void stg_world_destroy_cb( gpointer key, gpointer value, gpointer user )
{
  stg_world_destroy( (stg_world_t*)value );
}


void stg_model_attach_prop( stg_model_t* mod, stg_property_t* prop )
{
  // install the new property data in the model
  g_hash_table_replace( mod->props, &prop->id, prop );
}


stg_property_t* prop_create( stg_id_t id, void* data, size_t len )
{
  stg_property_t* prop = calloc( sizeof(stg_property_t), 1 );
  
  prop->id = id;
  prop->data = malloc(len);
  memcpy( prop->data, data, len );
  prop->len = len;
  
  //#ifdef DEBUG
  // printf( "debug: created a prop: " );
  //stg_property_print( prop );
  //#endif

  return prop;
}

void prop_destroy( stg_property_t* prop )
{
  free( prop->data );
  free( prop );
}


void stg_model_prop_with_data( stg_model_t* mod, 
			      stg_id_t type, void* data, size_t len )
{
  stg_property_t* prop = prop_create( type, data, len );
  stg_model_attach_prop( mod, prop );
}   

stg_property_t* stg_model_get_prop_cached( stg_model_t* mod, stg_id_t propid )
{
  return g_hash_table_lookup( mod->props, &propid );
}

// same as stg_client_request_reply(), but with fixed length reply data
int stg_client_request_reply_fixed( stg_client_t* client, 
				    int request_type, 
				    void* req_data, size_t req_len,
				    void* reply_data, size_t reply_len )
{
  //printf( "sending a %d byte request\n", (int)req_len );
  
  stg_client_write_msg( client, request_type, req_data, req_len );
  
  //puts( "waiting for reply" );
  
  while( !client->reply_ready )
    stg_client_read( client, 0 );

  //stg_mutex_lock( &client->reply_mutex );
  
  //while( !client->reply_ready )
  //pthread_cond_wait( &client->reply_cond, &client->reply_mutex );

  //puts( "waiting is over!" );
  
  if( client->reply->len != reply_len )
    {
      PRINT_ERR2( "reply is unexpected size (%d/%d bytes)",
		  client->reply->len, reply_len );
      return 1; // fail
    }
  
  memcpy( reply_data, client->reply->data, reply_len );
  
  client->reply_ready = FALSE; // consume the reply

  //stg_mutex_unlock( &client->reply_mutex );

  return 0; // success
}


int stg_client_request_reply( stg_client_t* client, 
			      int request_type, 
			      void* req_data, size_t req_len,
			      void** reply_data, size_t* reply_len )
{
  //printf( "sending a %d byte request\n", (int)req_len );
  
  stg_client_write_msg( client, request_type, req_data, req_len );
  
  //puts( "waiting for reply" );

  while( !client->reply_ready )
    stg_client_read( client, 0 );

  //stg_mutex_lock( &client->reply_mutex );
  
  //while( !client->reply_ready )
  //pthread_cond_wait( &client->reply_cond, &client->reply_mutex );

  //puts( "waiting is over!" );
  
  // copy the reply data from the client into a buffer, returned to
  // our caller (caller must free the buffer)
  *reply_data = malloc( client->reply->len );
  memcpy( *reply_data, client->reply->data, client->reply->len );
  *reply_len = client->reply->len;
  
  client->reply_ready = FALSE; // consume the reply

  //stg_mutex_unlock( &client->reply_mutex );
  
  //printf( "received a %d byte reply\n", (int)*reply_len );
  
  return 0; // OK
}

int stg_model_prop_delta( stg_model_t* mod, stg_id_t prop, void* data, size_t len )
{
  size_t mplen = len + sizeof(stg_prop_t);
  stg_prop_t* mp = calloc( mplen, 1);
  
  mp->world = mod->world->id_server;
  mp->model = mod->id_server;
  mp->prop = prop;
  mp->datalen = len;
  memcpy( mp->data, data, len );
  
  return stg_client_write_msg ( mod->world->client, 
				STG_MSG_MODEL_DELTA,
				mp, mplen );
}


int stg_model_prop_set( stg_model_t* mod, stg_id_t prop, void* data, size_t len )
{
  size_t mplen = len + sizeof(stg_prop_t);
  stg_prop_t* mp = calloc( mplen, 1);
  
  mp->world = mod->world->id_server;
  mp->model = mod->id_server;
  mp->prop = prop;
  mp->datalen = len;
  memcpy( mp->data, data, len );
  
  int ack;
  stg_client_request_reply_fixed( mod->world->client,
				  STG_MSG_MODEL_PROPSET,
				  mp, mplen,
				  &ack, sizeof(ack) ); 
  
  if( ack == STG_ACK )
    return 0; // OK
  // else
  return 1; // error
}


int stg_model_prop_get( stg_model_t* mod, stg_id_t propid, void* data, size_t len )
{
  stg_target_t tgt;
  tgt.world = mod->world->id_server;
  tgt.model = mod->id_server;
  tgt.prop = propid;
  
  PRINT_DEBUG3( "requesting prop %d:%d:%d\n", tgt.world, tgt.model, tgt.prop );
  
  stg_client_request_reply_fixed( mod->world->client,
				  STG_MSG_MODEL_PROPGET,
				  &tgt, sizeof(tgt),
				  data, len );  
  PRINT_DEBUG( "done" );
  return 0; // OK
}

int stg_model_prop_get_var( stg_model_t* mod, stg_id_t propid,
			    void** data, size_t* len )
{
  stg_target_t tgt;
  tgt.world = mod->world->id_server;
  tgt.model = mod->id_server;
  tgt.prop = propid;
  
  PRINT_DEBUG3( "requesting prop %d:%d:%d\n", tgt.world, tgt.model, tgt.prop );
  
  stg_client_request_reply( mod->world->client,
			    STG_MSG_MODEL_PROPGET,
			    &tgt, sizeof(tgt),
			    data, len );  

  PRINT_DEBUG( "done" );
  return 0; // OK
}

stg_package_t* stg_client_read_package( stg_client_t* cli, 
					int sleep, 
					int* err )
{
  //PRINT_DEBUG( "Checking for data on Stage connection" );
  
  assert( cli );
  assert( err );

  *err = 0;

  if( cli->pfd.fd < 1 )
    {
      PRINT_WARN( "not connected" );
      return NULL; 
    } 
  
  if( poll( &cli->pfd,1,sleep) && (cli->pfd.revents & POLLIN) )
    {
      //PRINT_DEBUG( "pollin on Stage connection" );
      
      stg_package_t* pkg = calloc( sizeof(stg_package_t), 1 );
      
      ssize_t res = stg_fd_packet_read( cli->pfd.fd, 
					pkg, sizeof(stg_package_t) );
      
      if( res != sizeof(stg_package_t) )
	{
	  PRINT_ERR2( "failed to read package header (%d/%d bytes)\n",
		      res, (int)sizeof(stg_package_t) );
	  free( pkg );
	  *err = 1;
	  return NULL;
	}
      
      double sec = pkg->timestamp.tv_sec + (double)pkg->timestamp.tv_usec / 1e6; 
      //printf( "package key:%d timestamp:%.3f len:%d\n",
      //      pkg->key, sec, pkg->payload_len );
      
      if( pkg->key != STG_PACKAGE_KEY )
	{
	  PRINT_ERR2( "package has incorrect key (%d not %d)\n",
		      pkg->key, STG_PACKAGE_KEY );
	  free( pkg );
	  *err = 2;
	  return NULL;
	}

      // read the body of the package
      size_t total_size = sizeof(stg_package_t) + pkg->payload_len;
      pkg = realloc( pkg, total_size );
      
      res = stg_fd_packet_read( cli->pfd.fd, 
				pkg->payload, 
				pkg->payload_len );
      
      if( res != pkg->payload_len )
	{
	  PRINT_ERR2( "failed to read package payload (%d/%d bytes)\n",
		      res,(int)pkg->payload_len );
	  free( pkg );
	  *err = 3;
	  return NULL;
	}
      
      return pkg;
    }

  return NULL; // ok
}

// break the package into individual messages and handle them
int stg_client_package_parse( stg_client_t* cli, stg_package_t* pkg )
{
  assert( cli );
  assert( pkg );
  
  stg_msg_t* msg = (stg_msg_t*)pkg->payload;
  
  while( (char*)msg < (pkg->payload + pkg->payload_len) )
    {
      // eat this message
      stg_client_handle_message( cli, msg );
      
      // jump to the next message in the buffer
      (char*)msg += sizeof(stg_msg_t) + msg->payload_len;
    }

  return 0; // ok
}


int stg_client_property_set( stg_client_t* cli, stg_id_t world, stg_id_t model, 
			     stg_id_t prop, void* data, size_t len )
{
  size_t mplen = len + sizeof(stg_prop_t);
  stg_prop_t* mp = calloc( mplen,1  );
  
  mp->world = world;
  mp->model = model;
  mp->prop = prop;
  mp->datalen = len;
  memcpy( mp->data, data, len );
  
  int retval = stg_client_write_msg( cli, 
				     STG_MSG_MODEL_DELTA, 
				     mp, mplen );
  
  free( mp );
  
  return retval;
}


stg_id_t stg_client_model_new(  stg_client_t* cli, 
				stg_id_t world,
				char* token )
{
  stg_createmodel_t mod;
  mod.world = world;
  strncpy( mod.token, token, STG_TOKEN_MAX );
  
  //printf( "creating model %s in world %d\n",  mod.token, mod.world );
  
  stg_id_t mid; // the server replies with the server-side id of the new model

  assert( stg_client_request_reply_fixed( cli, 
					  STG_MSG_WORLD_MODELCREATE, 
					  &mod, sizeof(mod),
					  &mid, sizeof(mid) ) 
	  == 0 );
  
    
  //printf( " received server-side model id %d\n", mid );
    
  return mid;
}

int stg_model_pull(  stg_model_t* mod ) 
{
  assert( mod );
  assert( mod->world );
  assert( mod->world->client );

  PRINT_DEBUG4( "pulling model %d:%d (%d:%d server-side) \n", 
		mod->world->id_client, mod->id_client,
		mod->world->id_server, mod->id_server );
  
  stg_destroymodel_t doomed;
  
  doomed.world = mod->world->id_server;
  doomed.model = mod->id_server;
  
  stg_client_write_msg( mod->world->client, 
			STG_MSG_WORLD_MODELDESTROY, 
			&doomed, sizeof(doomed) );
  return 0;
}

stg_id_t stg_client_world_new(  stg_client_t* cli, char* token, 
				stg_meters_t width, 
				stg_meters_t height, 
				int ppm, 
				stg_msec_t interval_sim, 
				stg_msec_t interval_real  )
{
  stg_createworld_t wmsg;
  
  wmsg.ppm = ppm;
  wmsg.interval_sim = interval_sim;
  wmsg.interval_real = interval_real;
  //wmsg.size.x = 0;//width;
  //wmsg.size.y = 0;//height;

  strncpy( wmsg.token, token, STG_TOKEN_MAX );
  
  PRINT_DEBUG4( "pushing world \"%s\" (sim: %lu real: %lu ppm: %d) ", 
		wmsg.token, wmsg.interval_sim, wmsg.interval_real, wmsg.ppm );
  
  stg_id_t wid;
  
  assert( stg_client_request_reply_fixed( cli,
					  STG_MSG_SERVER_WORLDCREATE,
					  &wmsg, sizeof(wmsg),
					  &wid, sizeof(wid) ) == 0 );
  
  PRINT_DEBUG1( " received server-side world id %d", wid );
  PRINT_DEBUG( " done." );  

  return wid;
}

int stg_world_pull( stg_world_t* world ) 
{
  assert( world );
  assert( world->client );
  
  PRINT_DEBUG2( "pulling world %d (%d)\n", 
		world->id_client, world->id_server );
  
  stg_client_write_msg( world->client, 
			STG_MSG_SERVER_WORLDDESTROY, 
			&world->id_server, sizeof(world->id_server) );
  return 0;
}

void stg_world_pull_cb( gpointer key, gpointer value, gpointer user )
{
  stg_world_pull( (stg_world_t*)value );
}


void stg_model_print( stg_model_t* mod )
{
  printf( "model %d:%d(%s) parent %d props: %d\n", 
	  mod->world->id_client, 
	  mod->id_client, 
	  mod->token->token, 
	  mod->parent->id_client,
	  (int)g_hash_table_size( mod->props) );
  
  g_hash_table_foreach( mod->props, stg_property_print_cb, NULL );
}

void stg_model_print_cb( gpointer key, gpointer value, gpointer user )
{
  stg_model_print( (stg_model_t*)value );
}


void stg_prop_push( stg_property_t* prop, stg_prop_target_t* pt )
{
  PRINT_DEBUG4( "  pushing prop %d:%d:%d(%s)",
		pt->world_id_server, pt->model_id_server,
		prop->id, stg_property_string(prop->id) );

  stg_client_property_set(  pt->client,
			    pt->world_id_server,
			    pt->model_id_server,
			    prop->id, 
			    prop->data, prop->len  );
}

void stg_prop_push_cb( gpointer key, gpointer value, gpointer user )
{
  stg_prop_push( (stg_property_t*)value, (stg_prop_target_t*)user );
}

int stg_model_property_set( stg_model_t* mod, stg_id_t prop, void* data, size_t len )
{
  stg_client_property_set( mod->world->client, mod->world->id_server, mod->id_server,
			   prop, data, len );
}

// create a model
void stg_model_push( stg_model_t* mod )
{ 
  assert( mod );
  assert( mod->world );
  assert( mod->world->client );

  // take this model out of the server-side id table
  g_hash_table_remove( mod->world->models_id_server, &mod->id_server );

  PRINT_DEBUG1( "  pushing model \"%s\" ", mod->token->token );
  
  mod->id_server = stg_client_model_new(  mod->world->client,
					  mod->world->id_server,
					  mod->token->token );

  PRINT_DEBUG( " done" );
  
  // re-index the model by the newly-discovered server-side id
  g_hash_table_replace( mod->world->models_id_server, &mod->id_server, mod );
  
  PRINT_DEBUG2( "pushed model %d and received server-side id %d", 
		mod->id_client, mod->id_server );

  // upload each of this model's properties
  stg_prop_target_t pt;
  pt.client = mod->world->client;
  pt.world_id_server = mod->world->id_server;
  pt.model_id_server = mod->id_server;
  
  g_hash_table_foreach( mod->props, stg_prop_push_cb, &pt );
}
  
void stg_model_push_cb( gpointer key, gpointer value, gpointer user )
{
  stg_model_push( (stg_model_t*)value );
}
 

void stg_world_push( stg_world_t* world )
{ 
  assert( world );
  assert( world->client );
  
  PRINT_DEBUG1( "pushing world \"%s\"\n", world->token->token );
  
  // take this world out of the server-side id table
  g_hash_table_remove( world->client->worlds_id_server, &world->id_server );
  
  world->id_server = stg_client_world_new( world->client, 
					   world->token->token, 
					   10, 10, 
					   world->ppm,
					   world->interval_sim,
					   world->interval_real );
  
  // index the world by its server-side id
  g_hash_table_replace( world->client->worlds_id_server, &world->id_server, world );
  
  PRINT_DEBUG2( "pushed world %d and received server-side id %d", 
		world->id_client, world->id_server );

  // upload all the models in this world
  g_hash_table_foreach( world->models_id, stg_model_push_cb, NULL );
}

void stg_world_push_cb( gpointer key, gpointer value, gpointer user )
{
  stg_world_push( (stg_world_t*)value );
}


// write the whole model table to the server
void stg_client_push( stg_client_t* cli )
{
  PRINT_DEBUG1( "pushing %d worlds", g_hash_table_size( cli->worlds_id ) );
  g_hash_table_foreach( cli->worlds_id, stg_world_push_cb, NULL );
}

// TODO - use the functions in parse.c to replace CWorldFile

/* int stg_client_load( stg_client_t* cli, char* filename ) */
/* { */
/*   FILE* wf = NULL; */
/*   if( (wf = fopen( filename, "r" )) == NULL ) */
/*     perror( "failed to open worldfile" ); */
  
/*   stg_token_t* tokens = stg_tokenize( wf ); */

/* #ifdef DEBUG */
/*   puts( "\nTokenizing file:" ); */
/*   stg_tokens_print( tokens );   */
/*   puts( "\nParsing token list:" ); */
/* #endif */
  
/*   // create a table of models from the named file   */
/*   while( tokens ) */
/*     stg_client_load_worldblock( cli, &tokens ); */
  
/*   return 0; // ok */
/* } */

void stg_model_attach_child( stg_model_t* parent, stg_model_t* child )
{
  // set the child's parent property (only a single parent is allowed)
  //stg_model_property_data_set( child, STG_MOD_PARENT, 
  //		       &parent->id, sizeof(parent->id) );

  // add the child's id to the parent's child property
  //stg_model_property_data_append( parent, STG_MOD_CHILDREN, 
  //			  &child->id, sizeof(child->id ) );
}



void stg_client_handle_message( stg_client_t* cli, stg_msg_t* msg )
{
  //PRINT_DEBUG2( "handling message type %d len %d", 
  //	msg->type,(int)msg->len );
  
  if( msg == NULL ) 
    return;
  
  switch( msg->type )
    {
    case STG_MSG_CLIENT_SAVE:
      if( msg->payload_len == sizeof(stg_id_t) )
	{
	  stg_id_t wid = *(stg_id_t*)msg->payload;
	  stg_client_save(cli, wid );
	}
      else
	PRINT_WARN2( "Received malformed SAVE message (%d/%d bytes)",
		    msg->payload_len, sizeof(stg_id_t) );
      
      break;
      
    case STG_MSG_CLIENT_LOAD:
      // TODO
      stg_client_load(cli, 0 );
      break;

    case STG_MSG_CLIENT_REPLY:
      PRINT_DEBUG( "MSG CLIENT REPLY" );
     
      //stg_mutex_lock( &cli->reply_mutex );     
      //printf( "reply msg len %d\n", msg->payload_len ); 
      //printf( "reply msg value %d\n", *(int*)msg->payload );

      // stash the reply data
      g_byte_array_set_size( cli->reply, 0 );
      g_byte_array_append( cli->reply, msg->payload, msg->payload_len );
      
      //printf( "reply buf len %d\n", cli->reply->len );
      //printf( "reply buf value %d\n", *(int*)cli->reply->data );
      
      cli->reply_ready = TRUE;

      //pthread_cond_signal( &cli->reply_cond );
      //stg_mutex_unlock( &cli->reply_mutex );
      break;
      
    case STG_MSG_CLIENT_DELTA: 
      {
	stg_prop_t* prop = (stg_prop_t*)msg->payload;

	
	if( prop->timestamp > cli->stagetime )	  
	  cli->stagetime = prop->timestamp;
	

#if 0
	printf( "[stage: %lu]  ", cli->stagetime );

	printf( "[prop: %lu] received property %d:%d:%d(%s) %d/%d bytes\n",
		prop->timestamp,
		prop->world, 
		prop->model, 
		prop->prop,
		stg_property_string(prop->prop),
		(int)prop->datalen,
		(int)msg->payload_len );
#endif	

	puts("");

	
	// don't bother stashing a time
	if( prop->prop == STG_PROP_TIME )
	  {
	    //puts( "TIME ONLY" );
	    break;
	  }

	// stash the data in the client-side model tree
	//PRINT_DEBUG2( "looking up client-side model for %d:%d",
	//      prop->world, prop->model );
	
	stg_model_t* mod = 
	  stg_client_get_model_serverside( cli, prop->world, prop->model );
	
	if( mod == NULL )
	  {
	    PRINT_WARN2( "no such model %d:%d found in the client",
			 prop->world, prop->model );
	  }
	else
	  { 
	    //PRINT_DEBUG4( "stashing prop %d(%s) bytes %d in model \"%s\"",
	    //	  prop->prop, stg_property_string(prop->prop),
	    //	  (int)prop->datalen, mod->token->token );

	    //stg_mutex_lock( &cli->models_mutex );
	    stg_model_prop_with_data( mod, prop->prop, 
				      prop->data, prop->datalen );
	    //stg_mutex_unlock( &cli->models_mutex );
	  }

#ifdef DEBUG	
	// human-readable output for some of the data
	switch( prop->prop )
	  {
	  case STG_PROP_POSE:
	    {
	      stg_pose_t* pose = (stg_pose_t*)prop->data;
	      
	      printf( "pose: %.3f %.3f %.3f\n",    
		      pose->x, pose->y, pose->a );
	    }
	    break;

	  case STG_PROP_RANGERCONFIG:
	    {
	      stg_ranger_config_t* cfgs = (stg_ranger_config_t*)prop->data;
	      int cfg_count = prop->datalen / sizeof(stg_ranger_config_t);	   
	      printf( "received configs for %d rangers (", cfg_count );

	      int r;
	      for( r=0; r<cfg_count; r++ )
		printf( "(%.2f,%.2f,%.2f) ", 
			cfgs[r].pose.x, cfgs[r].pose.y, cfgs[r].pose.a );
	      
	      printf( ")\n" );
	    }
	    break;
	    
	  case STG_PROP_RANGERDATA:
	    {
	      stg_ranger_sample_t* rngs = (stg_ranger_sample_t*)prop->data;
	      int rng_count = prop->datalen / sizeof(stg_ranger_sample_t);	   
	      printf( "received data for %d rangers (", rng_count );

	      int r;
	      for( r=0; r<rng_count; r++ )
		printf( "%.2f ", rngs[r] );
	      
	      printf( ")\n" );
	    }
	    break;
	    
	  case STG_PROP_LASERDATA:
	    break;
	    /* 	    { */
	    /* 	      stg */
	    /* 	      stg_laser_sample_t* samples = (stg_laser_sample_t*)prop->data; */
	    /* 	      int ls_count = prop->datalen / sizeof(stg_laser_sample_t); */
	    
	    /* 	      printf( "received %d laser samples\n", ls_count ); */
	    /* 	    } */
	    
	  case STG_PROP_TIME:
	    cli->stagetime = prop->timestamp;
	    printf( "<time packet> [%lu] ", cli->stagetime );
	 
	    break;

	  default:
	    PRINT_WARN2( "property type %d(%s) unprintable",
			 prop->prop, stg_property_string(prop->prop ) ); 
	  }
#endif
      }
      break;
  
    default:
      PRINT_WARN1( "message type %d unhandled", msg->type ); 
      break;
    }
}


void stg_client_install_save( stg_client_t* cli, stg_client_callback_t cb )
{
  cli->callback_save = cb;
}

void stg_client_install_load( stg_client_t* cli, stg_client_callback_t cb )
{
  cli->callback_load = cb;
}


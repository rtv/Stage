
#include <stdlib.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>

#define DEBUG

#include "stage.h"
#include "stageclient.h"

static int static_next_id = 0;

// function declarations for use inside this file only (mostly
// wrappers for use as callbacks)
void sc_world_pull_cb( gpointer key, gpointer value, gpointer user );
void sc_world_destroy_cb( gpointer key, gpointer value, gpointer user );
void stg_catch_pipe( int signo );


sc_t* sc_create( void )
{
  sc_t* client = calloc( sizeof(sc_t), 1 ); 
  
  assert( client );

  client->host = strdup(STG_DEFAULT_SERVER_HOST);
  client->port = STG_DEFAULT_SERVER_PORT;

  client->worlds_id = g_hash_table_new( g_int_hash, g_int_equal );
  client->worlds_id_server = g_hash_table_new( g_int_hash, g_int_equal );
  client->worlds_name = g_hash_table_new( g_str_hash, g_str_equal );
  
  // init the pollfd
  memset( &client->pfd, 0, sizeof(client->pfd) );
  
  return client;
}

void sc_destroy( sc_t* cli )
{
  PRINT_DEBUG( "destroying client" );

  // TODO - one of these should call world destructors
  if( cli->worlds_id ) g_hash_table_destroy( cli->worlds_id );
  if( cli->worlds_name ) g_hash_table_destroy( cli->worlds_name );
  if( cli->pfd.fd > 0 ) close( cli->pfd.fd );
  free(cli);
}

int sc_connect( sc_t* client, char* host, int port )
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
  
  PRINT_MSG2( "connecting to server %s:%d\n", host, port );
  
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
      PRINT_ERR2( "Connection failed on %s:%d",
		  info->h_addr_list[0], port ); 
      PRINT_ERR( "Did you forget to start Stage?");
      perror( "" );
      fflush( stdout );
    return NULL;
    }
  
  // read Stage's server string
  char buf[100];
  buf[0] = 0;
  read( client->pfd.fd, buf, 100 );
  printf( "Stage says: \"%s\"\n", buf );
 
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
  PRINT_MSG4( "Connected to \"%s\" (version %d.%d.%d)",
	      reply.id_string, reply.vmajor, reply.vminor, reply.vmicro );
  

  return 0; //OK
}


void sc_pull( sc_t* cli )
{
  PRINT_DEBUG( "pulling everything from client" );
  
  // ask the server to close each of our worlds
  g_hash_table_foreach( cli->worlds_id, sc_world_pull_cb, cli );
}

sc_model_t* sc_get_model( sc_t* cli, char* wname, char* mname )
{
  sc_world_t* world = g_hash_table_lookup( cli->worlds_name, wname );

  if( world == NULL )
    {
      PRINT_DEBUG1("no such world \"%s\"", wname );
      return NULL;
    }

  sc_model_t* model = g_hash_table_lookup( world->models_name, mname );
  
  if( model == NULL )
    {
      PRINT_DEBUG2("no such model \"%s\" in world \"%s\"", mname, wname );
      return NULL;
    }
  
  return model;
} 

sc_model_t* sc_get_model_serverside( sc_t* cli, stg_id_t wid, stg_id_t mid )
{
  sc_world_t* world = g_hash_table_lookup( cli->worlds_id_server, &wid );
  
  if( world == NULL )
    {
      PRINT_DEBUG1("no such world with server-side id %d", wid );
      return NULL;
    }
  
  sc_model_t* model = g_hash_table_lookup( world->models_id_server, &mid );
  
  if( model == NULL )
    {
      PRINT_DEBUG2("no such model with server-side id %d in world %d", 
		   mid, wid );
      return NULL;
    }
  
  return model;
}


int sc_write_msg( sc_t* cli, stg_msg_type_t type, void* data, size_t datalen )
{
  stg_msg_t* msg = stg_msg_create( type, data, datalen );
  assert( msg );
  
  int retval = 
    stg_fd_packet_write( cli->pfd.fd, msg, sizeof(stg_msg_t) + msg->payload_len );
  
  stg_msg_destroy( msg );

  return retval;
}


int sc_model_subscribe( sc_t* cli, sc_model_t* mod, int prop, double interval )
{
  stg_sub_t sub;
  sub.world = mod->world->id_server;
  sub.model = mod->id_server;  
  sub.prop = prop;
  sub.interval = interval;
  
  sc_write_msg( cli, STG_MSG_SERVER_SUBSCRIBE, &sub, sizeof(sub) );
 
  return 0;
}


int sc_model_unsubscribe( sc_t* cli, sc_model_t* mod, int prop )
{
  stg_unsub_t sub;
  sub.world = mod->world->id_server;
  sub.model = mod->id_server;  
  sub.prop = prop;
  
  sc_write_msg( cli, STG_MSG_SERVER_UNSUBSCRIBE, &sub, sizeof(sub) );
  
  return 0;
}


sc_world_t* sc_world_create( stg_token_t* token, 
			     double ppm, double interval_sim, double interval_real )
{
  sc_world_t* w = calloc( sizeof(sc_world_t), 1 );
  w->id_client = static_next_id++;
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
  
  return w;
} 



sc_model_t* sc_model_create( sc_world_t* world, 
			     sc_model_t* parent, 
			     stg_token_t* token )
{ 
  
  sc_model_t* mod = calloc( sizeof(sc_model_t), 1 );
  
  mod->id_client = static_next_id++;
  mod->token = token;
  mod->world = world;
  mod->parent = parent;
  mod->props = g_hash_table_new( g_int_hash, g_int_equal );

  PRINT_DEBUG3( "created model %d:%d \"%s\"", 
		world->id_client, mod->id_client, mod->token->token );

  return mod;
}


void sc_model_destroy( sc_model_t* mod )
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

void sc_world_destroy( sc_world_t* world )
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

void sc_world_destroy_cb( gpointer key, gpointer value, gpointer user )
{
  sc_world_destroy( (sc_world_t*)value );
}


void sc_model_attach_prop( sc_model_t* mod, stg_property_t* prop )
{
  // install the new property data in the model
  g_hash_table_replace( mod->props, &prop->id, prop );
}

void sc_model_prop_with_data( sc_model_t* mod, 
			      stg_id_t type, void* data, size_t len )
{
  stg_property_t* prop = prop_create( type, data, len );
  sc_model_attach_prop( mod, prop );
}   

stg_msg_t* sc_read( sc_t* cli )
{
  //PRINT_DEBUG( "Checking for data on Stage connection" );
  
  if( cli->pfd.fd < 1 )
    {
      PRINT_WARN( "not connected" );
      return NULL; 
    } 
  
  if( poll( &cli->pfd,1,1 ) && (cli->pfd.revents & POLLIN) )
    {
      //PRINT_DEBUG( "pollin on Stage connection" );
      
      stg_msg_t* msg = stg_read_msg( cli->pfd.fd );
      
      if( msg == NULL )
	{
	  PRINT_ERR( "failed read on Stage connection. Quitting." );
	  exit( 0 );
	}
      else
	{
	  //PRINT_WARN( "message received" );

	  return msg;
	}   
    }

  return NULL; // ok
}


int stg_property_set( sc_t* cli, stg_id_t world, stg_id_t model, 
		      stg_id_t prop, void* data, size_t len )
{
  size_t mplen = len + sizeof(stg_prop_t);
  stg_prop_t* mp = calloc( mplen,1  );
  
  mp->world = world;
  mp->model = model;
  mp->prop = prop;
  mp->datalen = len;
  memcpy( mp->data, data, len );
  
  int retval = sc_write_msg( cli, STG_MSG_MODEL_PROPERTY, mp, mplen );
  
  free( mp );
  
  return retval;
}

stg_id_t stg_model_new(  sc_t* cli, 
			 stg_id_t world,
			 char* token )
{
  stg_createmodel_t mod;
  
  mod.world = world;
  
  strncpy( mod.token, token, STG_TOKEN_MAX );
  
  printf( "creating model %s in world %d\n",  mod.token, mod.world );

  sc_write_msg( cli, STG_MSG_WORLD_MODELCREATE, &mod, sizeof(mod) );
  
  // read a reply - it contains the model's id
  stg_msg_t* reply = NULL;  
  
  while( (reply = sc_read( cli )) == NULL )
    {
      putchar( '.' ); fflush(stdout);
    }
  
  assert( reply->type == STG_MSG_CLIENT_MODELCREATEREPLY );
  assert( reply->payload_len == sizeof( stg_id_t ) );
  
  stg_id_t mid = *((stg_id_t*)reply->payload);

  printf( " received server-side model id %d\n", mid );

  stg_msg_destroy( reply );

  return mid;
}

int stg_model_pull(  sc_t* cli, sc_model_t* mod ) 
{
  PRINT_DEBUG4( "pulling model %d:%d (%d:%d server-side) \n", 
		mod->world->id_client, mod->id_client,
		mod->world->id_server, mod->id_server );
  
  stg_destroymodel_t doomed;
  
  doomed.world = mod->world->id_server;
  doomed.model = mod->id_server;
  
  sc_write_msg( cli, STG_MSG_WORLD_MODELDESTROY, &doomed, sizeof(doomed) );
  
  return 0;
}

stg_id_t stg_world_new(  sc_t* cli, char* token, 
			 double width, double height, int ppm, 
			 double interval_sim, double interval_real  )
{
  stg_createworld_t wmsg;
  
  wmsg.ppm = ppm;
  wmsg.interval_sim = interval_sim;
  wmsg.interval_real = interval_real;
  //wmsg.size.x = 0;//width;
  //wmsg.size.y = 0;//height;

  strncpy( wmsg.token, token, STG_TOKEN_MAX );
  
  printf( "creating world \"%s\" sim: %.3f real: %.3f ppm %d", 
	  wmsg.token, wmsg.interval_sim, wmsg.interval_real, wmsg.ppm );
  
  sc_write_msg( cli, STG_MSG_SERVER_WORLDCREATE, &wmsg, sizeof(wmsg) );
  
  // read a reply - it contains the world's id
  
  stg_msg_t* reply = NULL;  
  
  while( (reply = sc_read( cli )) == NULL )
    {
      putchar( '.' ); fflush(stdout);
    }
  
  assert( reply->type == STG_MSG_CLIENT_WORLDCREATEREPLY );
  assert( reply->payload_len == sizeof( stg_id_t ) );
  
  stg_id_t wid = *((stg_id_t*)reply->payload);

  printf( " received server-side world id %d\n", wid );

  stg_msg_destroy( reply );

  return wid;
}

int sc_world_pull( sc_t* cli, sc_world_t* world ) 
{
  PRINT_DEBUG2( "pulling world %d (%d)\n", 
		world->id_client, world->id_server );
  
  sc_write_msg( cli, STG_MSG_SERVER_WORLDDESTROY, 
		&world->id_server, sizeof(world->id_server) );
  return 0;
}

void sc_world_pull_cb( gpointer key, gpointer value, gpointer user )
{
  sc_world_pull( (sc_t*)user, (sc_world_t*)value );
}


void sc_model_print( sc_model_t* mod )
{
  printf( "model %d:%d(%s) parent %d props: %d\n", 
	  mod->world->id_client, 
	  mod->id_client, 
	  mod->token->token, 
	  mod->parent->id_client,
	  (int)g_hash_table_size( mod->props) );
  
  g_hash_table_foreach( mod->props, stg_property_print_cb, NULL );
}

void sc_model_print_cb( gpointer key, gpointer value, gpointer user )
{
  sc_model_print( (sc_model_t*)value );
}


void sc_world_push( sc_t* cli, sc_world_t* world )
{
  // create a world
  printf( "pushing world \"%s\"\n", world->token->token );
  
  world->id_server = stg_world_new( cli, world->token->token, 
				    10, 10, 
				    world->ppm,
				    world->interval_sim,
				    world->interval_real );
  
  // upload all the models in this world
  g_hash_table_foreach( world->models_id, sc_model_push_cb, cli );
}

void sc_world_push_cb( gpointer key, gpointer value, gpointer user )
{
  sc_world_push( (sc_t*)user, (sc_world_t*)value );
}


typedef struct
{
  sc_t* client;
  stg_id_t world_id_server;
  stg_id_t model_id_server;
} sc_prop_target_t;

void prop_push( stg_property_t* prop, sc_prop_target_t* pt )
{
  PRINT_DEBUG4( "  pushing prop %d:%d:%d(%s)\n",
		pt->world_id_server, pt->model_id_server,
		prop->id, stg_property_string(prop->id) );
  
  void* data;
  size_t len;
  //int color;
  switch( prop->id )
    {
      //case STG_PROP_COLOR: // convert the color string to an int
      //color = stg_lookup_color( prop->data );
      //data = &color;
      //len = sizeof(color);
      //break;	  
      
    default: // use the raw data we loaded
      {
	data = prop->data;
	len = prop->len;	    
      }
      break;
    }
  
  if( data && len > 0 )
    stg_property_set(  pt->client,
		       pt->world_id_server,
		       pt->model_id_server,
		       prop->id, 
		       data, len  );
}

void prop_push_cb( gpointer key, gpointer value, gpointer user )
{
  prop_push( (sc_t*)value, (sc_prop_target_t*)user );
}


void sc_model_push( sc_t* cli, sc_model_t* mod )
{
  // create a model
  printf( "  pushing model \"%s\"\n", mod->token->token );


  // take this model out of the server-side id table
  g_hash_table_remove( mod->world->models_id_server, &mod->id_server );
  
  mod->id_server = stg_model_new(  cli,
				   mod->world->id_server,
				   mod->token->token );
  
  // re-index the model by the newly-discovered server-side id
  g_hash_table_replace( mod->world->models_id_server, &mod->id_server, mod );
  
  // upload each of this model's properties
  sc_prop_target_t pt;
  pt.client = cli;
  pt.world_id_server = mod->world->id_server;
  pt.model_id_server = mod->id_server;
  
  g_hash_table_foreach( mod->props, prop_push_cb, &pt );
}
  
void sc_model_push_cb( gpointer key, gpointer value, gpointer user )
{
  sc_model_push( (sc_t*)user, (sc_model_t*)value );
}
 
// write the whole model table to the server
void sc_push( sc_t* cli )
{
  PRINT_DEBUG1( "pushing %d worlds", g_hash_table_size( cli->worlds_id ) );
  g_hash_table_foreach( cli->worlds_id, sc_world_push_cb, cli );
}

// TODO - use the functions in parse.c to replace CWorldFile

/* int sc_load( sc_t* cli, char* filename ) */
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
/*     sc_load_worldblock( cli, &tokens ); */
  
/*   return 0; // ok */
/* } */

void sc_model_attach_child( sc_model_t* parent, sc_model_t* child )
{
  // set the child's parent property (only a single parent is allowed)
  //stg_model_property_data_set( child, STG_MOD_PARENT, 
  //		       &parent->id, sizeof(parent->id) );

  // add the child's id to the parent's child property
  //stg_model_property_data_append( parent, STG_MOD_CHILDREN, 
  //			  &child->id, sizeof(child->id ) );
}


void sc_world_addmodel( sc_world_t* world, sc_model_t* model )
{
  // add the model to its world
  g_hash_table_replace( world->models_id, &model->id_client, model );
  g_hash_table_replace( world->models_id_server, &model->id_server, model );
  g_hash_table_replace( world->models_name, model->token->token, model );
}

// add the world to the client
void sc_addworld( sc_t* cli, sc_world_t* world )
{
  g_hash_table_replace( cli->worlds_id, &world->id_client, world );
  g_hash_table_replace( cli->worlds_id_server, &world->id_server, world );
  g_hash_table_replace( cli->worlds_name, world->token->token, world );
}





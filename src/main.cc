/*
 *  Stage : a multi-robot simulator.  
 *
 *  Copyright (C) 2001, 2002, 2003 Richard Vaughan, Andrew Howard and
 *  Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Program Entry point
 * Author: Richard Vaughan
 * Date: 3 July 2003
 * CVS: $Id: main.cc,v 1.81 2003-10-22 19:51:02 rtv Exp $
 */

/* NOTES this file is only C++ because it must create CEntity
 * objects. Stage is gradually becoming C with glib. */ 

#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>

#define DEBUG

//#include "stage.h"
#include "world.hh"
#include "entity.hh"
#include "rtkgui.hh" // for rtk_init()

#define HELLOWORLD "** Stage "VERSION" **"
#define BYEWORLD "Stage finished"

// limit on number of connection requests to queue up
#define STG_LISTENQ  128

int quit = 0; // set true by the GUI when it wants to quit 

// globals
GHashTable* global_model_table = g_hash_table_new(g_int_hash, g_int_equal);
GHashTable* global_world_table = g_hash_table_new(g_int_hash, g_int_equal);
GList* global_sub_clients = NULL; // list of subscription-based clients
int global_next_id = 1; // used to generate unique id number for all objects

// forward declare
gboolean SubClientRead( GIOChannel* channel, 
			GIOCondition condition, 
			gpointer data );

gboolean ClientHup( GIOChannel* channel, 
			GIOCondition condition, 
			gpointer data );

  // prints the object tree on stdout
void PrintModelTree( CEntity* ent, gpointer _prefix )
{
  GString* prefix = (GString*)_prefix;

  g_assert(ent);

  GString* pp = NULL;

  if( prefix )
    pp = g_string_new( prefix->str );
  else
    pp = g_string_new( "" );
  
  printf( "%s%d:%s\n", pp->str, ent->id, ent->name->str );

  pp = g_string_prepend( pp, "  " );
   
  g_string_free( pp, TRUE );
}
  
///////////////////////////////////////////////////////////////////////////

// write a stg_property_t on the channel returns TRUE on success, else FALSE
gboolean PropertyWrite( GIOChannel* channel, stg_property_t* prop )
{
  gboolean failed = FALSE;

  ssize_t result = 
    stg_fd_msg_write( g_io_channel_unix_get_fd(channel), 
		      STG_MSG_PROPERTY, 
		      prop, 
		      sizeof(stg_property_t) + prop->len );
  
  if( result < 0 )
    {
      PRINT_MSG1( "closing connection (fd %d)", 
		  g_io_channel_unix_get_fd(channel) );
      
      /* zap this connection */
      g_io_channel_shutdown( channel, TRUE, NULL );
      g_io_channel_unref( channel );
    }      
  
  return( !failed );
}

ss_client_t* stg_sub_client_create( GIOChannel* channel )
{

  ss_client_t* cli = 
    (ss_client_t*)calloc(1,sizeof(ss_client_t) );
  
  g_assert( cli );
  
  // set up this client
  cli->channel = channel;  
  cli->source_in  = g_io_add_watch( channel, G_IO_IN, SubClientRead, cli );
  cli->source_hup = g_io_add_watch( channel, G_IO_HUP, ClientHup, cli );
  cli->subs = NULL;
  cli->worlds = NULL;
  
  // add this client to the list of subscribers to this world
  global_sub_clients = g_list_append( global_sub_clients, cli );
  
  return cli;
}

void foreach_free( gpointer data, gpointer userdata )
{
  g_free( data );
}

void stg_client_destroy( ss_client_t* cli )
{ 
  // cancel watches we installed on this channel
  if( cli->source_in  ) g_source_remove( cli->source_in );
  if( cli->source_hup ) g_source_remove( cli->source_hup );
  
  // and kill it
  g_io_channel_shutdown( cli->channel, TRUE, NULL ); // zap this connection 
  g_io_channel_unref( cli->channel );
  
  // remove this client from the global list
  global_sub_clients = g_list_remove( global_sub_clients, cli );
  

  // destroy any worlds that were created by this client
  if( cli->worlds ) for( GList* l = cli->worlds; l; l=l->next )
    {
      if( l->data == NULL )
	PRINT_WARN1( "l = %p but l->data == NULL", l );
      
      ss_world_t* world = (ss_world_t*)l->data;
      g_assert( g_hash_table_remove( global_world_table, &world->id));
      PRINT_DEBUG1( "destroying world %p", world);
      ss_world_destroy( world );
    }
  
  //g_list_free( cli->worlds );

  PRINT_DEBUG( "finished destroying worlds" );
  
  // delete the subscription list
  g_list_foreach( cli->subs, foreach_free, NULL );
  g_list_free( cli->subs );

  free( cli );
}


// returns zero if the two subscriptions match
// used for looking up subscriptions in a GList
gint match_sub( gconstpointer a, gconstpointer b )
{
  stg_subscription_t* s1 = (stg_subscription_t*)a;
  stg_subscription_t* s2 = (stg_subscription_t*)b;

  if( s1->id < s2->id ) return -1;
  if( s1->id > s2->id ) return 1;
 
  if( s1->prop < s2->prop ) return -1;
  if( s1->prop > s2->prop ) return 1;

  return 0;
}

int HandleSubscription( ss_client_t* cli, stg_subscription_t* sub )
{
  PRINT_DEBUG( "subscription" );
  g_assert(cli);
  g_assert(sub);
 
  switch( sub->status )
    {
    case STG_SUB_SUBSCRIBED:
      {
	PRINT_DEBUG3( "adding subscription [%d:%s] to client %p",
		      sub->id, 
		      stg_property_string(sub->prop), 
		      cli );
	    
	stg_subscription_t *storesub = 
	  (stg_subscription_t*)g_malloc(sizeof(stg_subscription_t));
	g_assert(storesub);
	    
	memcpy( storesub, sub, sizeof(stg_subscription_t) );
	    
	cli->subs = g_list_append( cli->subs, storesub );
	    
	PRINT_DEBUG2( "client %p now has %d subscriptions", 
		      cli, g_list_length(cli->subs) );
	    
	PRINT_DEBUG3( "added subscription [%d:%s] to client %p",
		      storesub->id, 
		      stg_property_string(storesub->prop), 
		      cli );


	// write the packet back to indicate success
	int fd = g_io_channel_unix_get_fd(cli->channel);
	stg_fd_msg_write( fd, STG_MSG_SUBSCRIBE, sub, sizeof(sub) );	
      }
      break;
	  
    case STG_SUB_UNSUBSCRIBED:
      {
	PRINT_DEBUG3( "removing subscription [%d:%s] from client %p",
		      sub->id, 
		      stg_property_string(sub->prop), 
		      cli );
	// remove subscription and free its memory
	    
	GList* dead_link = g_list_find_custom( cli->subs, 
					       sub, match_sub );
	    
	cli->subs = g_list_remove_link( cli->subs, dead_link ); 
	
	g_assert( dead_link );
	g_free( dead_link->data );   
	    
	// write the packet back to indicate success
	int fd = g_io_channel_unix_get_fd(cli->channel);
	stg_fd_msg_write( fd, STG_MSG_SUBSCRIBE, sub, sizeof(sub) );
      }
      break;
	  
    default:
      PRINT_ERR1( "uknown subscription status (%d)", sub->status );
      return 1; // fail
      break;
    }

  return 0; //ok
}

stg_id_t CreateWorld( ss_client_t* cli, stg_world_create_t* wc )
{
  PRINT_DEBUG( "create world" );
  
  // create a world object
  ss_world_t* aworld = ss_world_create( global_next_id++, wc );
  g_assert( aworld );
  
  // add the new world to the hash table with its id as
  // the key (this ID should not already exist)
  //g_assert( g_hash_table_lookup(global_world_table, &aworld->id)
  //    == NULL ); 

  g_hash_table_insert(global_world_table, &aworld->id, 
		      aworld );  
  
  // add this world to the client's list
  cli->worlds = g_list_append( cli->worlds, aworld ); 
  
  PRINT_DEBUG3( "Created world id %d name \"%s\" at %p", 
		aworld->id, aworld->name->str, aworld );  
  
  return aworld->id;
}

int DestroyWorld( stg_id_t world_id )
{
  PRINT_DEBUG1( "destroy world id %d", world_id );
  
  g_assert( world_id > 0 );

  ss_world_t* doomed_world = (ss_world_t*)
    g_hash_table_lookup( global_model_table, &world_id );
  
  g_hash_table_remove( global_world_table, &world_id );

  if( doomed_world == NULL )
    {
      PRINT_ERR1( "couldn't find and destroy a world with id %d",
		    world_id );
      return 1; // error
    }
  
  ss_world_destroy(doomed_world);	

  return 0; // ok
}

stg_id_t CreateModel( stg_model_create_t* create )
{
  PRINT_DEBUG1( "creating model in world %d", create->world_id );
  
  g_assert(create);
  
  CEntity* ent = NULL;	    
  g_assert((ent = new CEntity( create, global_next_id++ )) );
  
  // add the new model to the world's hash table with
  // its id as the key (this ID should not already
  // exist)
  //g_assert( g_hash_table_lookup( global_model_table, &ent->id ) 
  //    == NULL ); 

  g_hash_table_insert( global_model_table, &ent->id, ent );
  
  PRINT_DEBUG3( "created model %d \"%s\" child of \"%s\"",
		ent->id,
		ent->name->str,
		ent->parent ? ent->parent->name->str:"<none>" ); 
  
  // reply with the id of the entity
  return ent->id;
}


int DestroyModel( stg_id_t doomed )
{
  PRINT_DEBUG1( "destroying model id %d", doomed );

  g_assert( doomed > 0 );

  CEntity* ent = (CEntity*)
    g_hash_table_lookup( global_model_table, &doomed );
  
  g_hash_table_remove( global_model_table, &doomed );
  
  if( ent == NULL )
    {
      PRINT_ERR1( "failed to find and destroy model %d",
		    doomed );      
      return 1; // error
    }
  
  delete ent;
  
  return 0; // ok
}


int HandleCreateWorld( ss_client_t* cli, stg_world_create_t* baby )
{
   PRINT_DEBUG( "reading a world for creation" );

  g_assert(cli);
  g_assert(baby);

  stg_world_create_reply_t reply;
  reply.id = CreateWorld( cli, baby );	
  reply.key = baby->key; // include the key that identifies the request
  
  if( reply.id < 1 )
    PRINT_WARN( "failed to create a world on demand" ); 

  int fd = g_io_channel_unix_get_fd(cli->channel);
  stg_fd_msg_write( fd, STG_MSG_WORLD_CREATE_REPLY, 
		    &reply, sizeof(reply));
  return 0;
}


int HandleCreateModel( ss_client_t* cli, stg_model_create_t* baby )
{
  PRINT_DEBUG( "reading a model for creation" );

  g_assert(cli);
  g_assert(baby);
  
  stg_model_create_reply_t reply;
  reply.id = CreateModel( baby );	
  reply.key = baby->key; // include the key that identifies the request
  
  if( reply.id < 1 )
    PRINT_WARN( "failed to create a mode on demand" ); 
 
  int fd = g_io_channel_unix_get_fd(cli->channel);
  stg_fd_msg_write( fd, STG_MSG_MODEL_CREATE_REPLY, 
		    &reply, sizeof(reply));
  return 0;
}


stg_property_t* ServerPropertyGet( stg_prop_id_t type )
{
  PRINT_WARN1( "requesting server property %s not implemented",
	       stg_property_string( type ) );
  return NULL;
}

void ServerPropertySet( stg_property_t* prop )
{
  PRINT_WARN1( "setting server property %s not implemented",
	       stg_property_string( prop->type ) );
}


stg_property_t* SetProperty( stg_property_t* prop )
{
  PRINT_DEBUG2( "setting property id %d prop %s",
		prop->id, 
		stg_property_string(prop->type) );

  g_assert(prop);
  
  stg_property_t* reply = NULL;
  
  if( prop->id == 0 )
    ServerPropertySet( prop );
  else
    {
      ss_world_t* world = (ss_world_t*)
	g_hash_table_lookup( global_world_table, &prop->id );
      
      if( world )
	// todo - world replies?
	ss_world_property_set( world, prop );
      else
	{
	  CEntity* ent = (CEntity*)
	    g_hash_table_lookup( global_model_table, &prop->id );
	  
	  if( ent == NULL )
	    {
	      PRINT_ERR1( "failed to find and configure object %d", 
			  prop->id );      
	      return NULL;
	    }
	  switch( prop->reply )
	    {
	    case STG_PR_NONE:
	      ent->SetProperty( prop->type, prop->data, prop->len );
	      break;
	    case STG_PR_PRE:
	      reply = ent->GetProperty( prop->type );
	      ent->SetProperty( prop->type, prop->data, prop->len );
	      break;
	    case STG_PR_POST:
	      ent->SetProperty( prop->type, prop->data, prop->len );
	      reply = ent->GetProperty(prop->type );
	      break;
	      
	    default: 
	      PRINT_ERR1( "unknown property reply mode (%d)", 
			  prop->reply );
	      break;
	    }
	}
    } 
  
  return reply;
}

int HandleProperty( ss_client_t* cli, stg_property_t* prop  )
{
  PRINT_DEBUG( "handling a property" );
  
  g_assert(cli);
  g_assert(prop);
  
  stg_property_t* reply = SetProperty( prop );
  
  if( reply )
    {
      PropertyWrite( cli->channel, reply );  
      stg_property_free( reply ); 
    }
  else
    {
      PRINT_DEBUG( "<no reply>" );
    }

  return 0;
}

int HandlePropertyRequest( ss_client_t* cli, stg_property_req_t* req )
{
  PRINT_DEBUG( "handling a property request" );
  
  g_assert(cli);
  g_assert(req);
  
  PRINT_DEBUG2( "property request %d : %s", 
		req->id, stg_property_string(req->type) );
  
  stg_property_t* prop = NULL; // this will be the response
  
  if( req->id == 0 )
    ServerPropertyGet( req->type );
  else
    {
      // see if the id refers to a world
      ss_world_t* world = (ss_world_t*)
	g_hash_table_lookup( global_world_table, &req->id );
      
      if( world )
	prop = ss_world_property_get( world, req->type );
      else
	{
	  CEntity* ent = (CEntity*)
	    g_hash_table_lookup( global_model_table, &req->id );
	  
	  if( ent == NULL )
	    {
	      PRINT_ERR1( "failed to find an object with id %d", 
			  req->id );      
	      return 1;
	    }
	  
	  prop = ent->GetProperty( req->type );
	}  
    }

  if( prop == NULL )
    {
      PRINT_ERR2( "failed to get property %d:%s", 
		  req->id, stg_property_string(req->type) );
      return 2;
    }
  
  PropertyWrite( cli->channel, prop );
  stg_property_free( prop );
  
  return 0;
}

// read from a subscription-based client
gboolean SubClientRead( GIOChannel* channel, 
			   GIOCondition condition, 
			   gpointer data )
{
  PRINT_DEBUG( "sub client read" );
  g_assert(channel);
  g_assert(data);
  
  // the user data is a pointer to a client that has data pending
  ss_client_t *cli = (ss_client_t*)data;
  
  int fd = g_io_channel_unix_get_fd(channel);

  // read a message of some kind
  stg_header_t* hdr = stg_fd_msg_read( fd );

  if( hdr == NULL )
    {
      PRINT_ERR1( "message read failed on fd %d. Killing client.", fd );
      stg_client_destroy(cli);
      return FALSE;
    }
  
  int result = 0;

  PRINT_DEBUG2( "message received: %s with %d byte payload",
		stg_message_string(hdr->type), hdr->len );
  
  switch( hdr->type )
    {      
    case STG_MSG_NOOP: // do nothing
      break;

    case STG_MSG_PAUSE:
      PRINT_DEBUG( "pause" );
      cli->paused = 1;
      break;

    case STG_MSG_RESUME:
      PRINT_DEBUG( "resume" );
      cli->paused = 0;
      break;

    case STG_MSG_SUBSCRIBE:
      g_assert( hdr->len == sizeof(stg_subscription_t) );
      if( (result = 
	   HandleSubscription( cli, (stg_subscription_t*)hdr->payload )) )
	PRINT_ERR1( "Failed to handle a subscription on fd %d.", fd );
      break;
      
    case STG_MSG_PROPERTY:
      // properties can be variable length 
      g_assert( hdr->len >= sizeof(stg_property_t) );
      if( (result = HandleProperty( cli, (stg_property_t*)hdr->payload ))  )
	PRINT_ERR1( "Failed to handle a property on fd %d.", fd );
      break;
      
    case STG_MSG_WORLD_CREATE:
      g_assert( hdr->len == sizeof(stg_world_create_t) );
      if( (result = 
	   HandleCreateWorld( cli, (stg_world_create_t*)hdr->payload )) )
	PRINT_ERR1( "Failed to handle a world creation on fd %d.", fd );
      break;
      
    case STG_MSG_WORLD_DESTROY:
      g_assert( hdr->len == sizeof(stg_id_t) );
      if( (result = 
	   DestroyWorld( *(stg_id_t*)hdr->payload )) ) 
	PRINT_ERR1( "Failed to handle a world destruction on fd %d.", fd );
      break;
      
    case STG_MSG_MODEL_CREATE:
      g_assert( hdr->len == sizeof(stg_model_create_t) );
      if( (result =
	   HandleCreateModel( cli, (stg_model_create_t*)hdr->payload ))  )
	PRINT_ERR1( "Failed to handle a model creation on fd %d.", fd );
      break;
      
    case STG_MSG_MODEL_DESTROY:
      g_assert( hdr->len == sizeof(stg_id_t) );
      if( (result = DestroyModel( *(stg_id_t*)hdr->payload ))  )
	PRINT_ERR1( "Failed to handle a model destruction on fd %d.", fd );
      break;
      
    case STG_MSG_PROPERTY_REQ:
      g_assert( hdr->len == sizeof(stg_property_req_t) );
      if( (result = HandlePropertyRequest( cli, 
					      (stg_property_req_t*)hdr->payload
					      )))
	PRINT_ERR1( "Failed to handle a model destruction on fd %d.", fd );
      break;
      
    default:
      PRINT_ERR1( "unrecognized message type (%s). Ignoring.", 
		  stg_message_string(hdr->type) );
      break;
    }

  // free the memory allocated for the message
  if( hdr )free(hdr);

  if( result != 0 )
    {
      PRINT_ERR( "Client error. Shutting it down." );
      stg_client_destroy(cli);
      return FALSE;
    }  
  
  return TRUE;
}


gboolean ClientHup( GIOChannel* channel, 
		       GIOCondition condition, 
		       gpointer data )
{
  PRINT_MSG( "client HUP. Closing connection." );
  
  g_io_channel_shutdown( channel, TRUE, NULL );
  g_io_channel_unref( channel );
  
  return FALSE; // cancels this callback
}

gboolean ClientAcceptConnection( GIOChannel* channel, GHashTable* table )
{
  // set up a socket for this connection
  struct sockaddr_in cliaddr;  
  bzero(&cliaddr, sizeof(cliaddr));
  socklen_t clilen;
  clilen  = sizeof(cliaddr);
  int connfd = accept( g_io_channel_unix_get_fd(channel), 
		       (struct sockaddr*) &cliaddr, 
		       &clilen);
  
  g_assert( connfd > 0 );
  
  GIOChannel* client = g_io_channel_unix_new( connfd );
  g_io_channel_set_encoding( client, NULL, NULL );
  
  
  // look for a well-formed greeting
  stg_greeting_t greet;
  gsize bytes_read, bytes_written;

  PRINT_DEBUG2( "expecting %d byte greeting on %d", 
		sizeof(greet), g_io_channel_unix_get_fd( client ) );

  if(  g_io_channel_read_chars( client, 
				(char*)&greet, sizeof(greet),
				&bytes_read, NULL )
       != G_IO_STATUS_NORMAL ) 
    {
       PRINT_DEBUG1( "abnormal read of %d bytes", bytes_read );
       perror( "read error" );
       return FALSE; // fail
    }

  PRINT_DEBUG1( "read %d bytes", bytes_read );
  
  // check to make sure we have the right greeting code
  if( greet.code != STG_SERVER_GREETING )
    {
      PRINT_ERR1( "bad greeting from client (%d)", greet.code );
      g_io_channel_shutdown( channel, TRUE, NULL );
      g_io_channel_unref( channel );
      return FALSE; // fail
    }
  
  g_assert( stg_sub_client_create( client ) );

  // write the reply
  stg_connect_reply_t reply;
  
  reply.code = STG_CLIENT_GREETING;
  snprintf( reply.id_string, STG_ID_STRING_MAX,
	    "%s - %s", STG_ID_STRING, VERSION );
  
  // todo - get these from autoconf via config.h
  reply.vmajor = 1;
  reply.vminor = 4;
  reply.vbugfix = 0;

  g_io_channel_write_chars( client, 
			    (char*)&reply, (gssize)sizeof(reply),
			    &bytes_written, NULL );
  
  g_assert( bytes_written == sizeof(reply) );
  g_io_channel_flush( client, NULL );

  return TRUE; // success
}      

gboolean ServiceWellKnownPort( GIOChannel* channel, 
			       GIOCondition condition, 
			       gpointer table )
{
  switch( condition )
    {
    case G_IO_IN: PRINT_DEBUG( "CONNECT" ); 
      if( ClientAcceptConnection( channel, (GHashTable*)table ) == -1 )
	PRINT_WARN( "Connection rejected" );
      else
	PRINT_MSG( "Connection accepted" );
      break;
      
    case G_IO_ERR:  PRINT_WARN( "ERROR" ); break;
    case G_IO_HUP:  PRINT_WARN( "HUP" ); break;
    case G_IO_NVAL: PRINT_WARN( "NVAL" ); break;
    default: PRINT_WARN1( "unknown channel condition %d", condition );
    }
 
  return TRUE;
}

GIOChannel* ServerCreate( void )
{ 
  // ignore SIGPIPE - we'll respond to the HUP instead
  if( signal( SIGPIPE, SIG_IGN ) == SIG_ERR )
    {
      PRINT_ERR( "error setting pipe signal catcher" );
      exit( -1 );
    }

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  g_assert( fd > 0 );
  
  struct sockaddr_in servaddr;  
  bzero(&servaddr, sizeof(servaddr));
  
  int server_port = STG_DEFAULT_SERVER_PORT;
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(server_port);
  
  // switch on the re-use-address option
  const int on = 1;
  setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
  
  if( bind(fd, (struct sockaddr*) &servaddr, sizeof(servaddr) )  < 0 )
    {
      perror("failed to bind server socket");      
      printf( "Stage: Port %d is probably in use\n", server_port );
      return NULL; // fail
    }
  
  // listen for requests on this socket
  g_assert( listen( fd, STG_LISTENQ) == 0 );
  
  printf( " [port %d]", server_port );
  
  return( g_io_channel_unix_new( fd ) );
}

// Signal catchers ---------------------------------------------------

void catch_interrupt(int signum)
{
  puts( "Interrupt signal." );
  exit(0);
}

void catch_quit(int signum)
{
  puts( "Quit signal." );
  exit(0);
}

void catch_exit( void )
{
  puts( BYEWORLD );
}
// ----------------------------------------------------------------------


stg_property_t* server_property_get( stg_prop_id_t id )
{
  stg_property_t* prop = NULL;  
  
  switch( id )
    {
      /*
    case STG_MOD_TIME:
      prop = stg_property_create();
      prop->id = -1;
      prop->type = STG_MOD_TIME;
      prop = stg_property_attach_data( prop, &time, sizeof(time) );
      break;
      */

    default:
      PRINT_WARN1( "server property %d unhandled", id );
      break;
    }

  return prop;
}

void client_sub_update(  gpointer data, gpointer userdata )
{
  stg_subscription_t *sub = (stg_subscription_t*)data;
  ss_client_t *cli = (ss_client_t*)userdata;
  
  PRINT_DEBUG3( "   updating subscription [%d:%s] for client %p\n",
		sub->id, stg_property_string(sub->prop), cli );
  
  stg_property_t* prop = NULL;

  // lookup the object in the hash table
  ss_world_t* world = (ss_world_t*)
    g_hash_table_lookup( global_world_table, &sub->id );
  
  if( world )
    prop = ss_world_property_get( world, sub->prop );
  else
    {
      CEntity* ent = (CEntity*)
	g_hash_table_lookup( global_model_table, &sub->id );
      prop = ent->GetProperty( sub->prop );
    }		
  
  // if a property was created, write it out
  if( prop )
    {
      PropertyWrite( cli->channel, prop );  
      stg_property_free( prop );      
    }
  else
    PRINT_ERR2( "failed to emit subscribed property %d:%s", 
		sub->id, stg_property_string(sub->prop) );
}

void world_update( gpointer value, gpointer userdata )
{
  ss_world_t* world = (ss_world_t*)value;
  
  PRINT_DEBUG2( "  updating world %p (%s)", 
		world, world->name->str );
  
  g_assert( world );
  ss_world_update( world );
}

void client_update(  gpointer data, gpointer userdata )
{
  ss_client_t *cli = (ss_client_t*) data;
  
  if( cli->paused )
    PRINT_DEBUG1( "  client %p paused", cli );
  else
    {
      PRINT_DEBUG1( "  updating client %p", cli );
      
      // update all my worlds
      g_list_foreach( cli->worlds, world_update, cli );
      // and all my subscriptions
      g_list_foreach( cli->subs, client_sub_update, cli );
    }
}

gboolean server_update( gpointer dummy )
{
  // update all my clients
  g_list_foreach( global_sub_clients, client_update, NULL );
  return TRUE; // keep this callback running
}

int main( int argc, char** argv )
{
  printf( HELLOWORLD );
  
  atexit( catch_exit );
  
 if( signal( SIGINT, catch_interrupt ) == SIG_ERR )
    {
      PRINT_ERR( "error setting interrupt signal catcher" );
      exit( -1 );
    }

 if( signal( SIGQUIT, catch_quit ) == SIG_ERR )
    {
      PRINT_ERR( "error setting quit signal catcher" );
      exit( -1 );
    }
 
  gui_init( &argc, &argv );

  // call server_update() at the right interval
  g_timeout_add( 100, server_update, NULL );

  // create a socket bound to Stage's well-known-port
  GIOChannel* channel = ServerCreate();
  g_assert( channel );
  
  // watch for these events on the well-known-port
  g_io_add_watch(channel, G_IO_IN, ServiceWellKnownPort, global_model_table);
  
  //PRINT_MSG1( "Accepting connections on port %d", STG_DEFAULT_SERVER_PORT );

  puts( "" ); // end the startup message line

  GMainLoop* mainloop = g_main_loop_new( NULL, TRUE );
  g_main_loop_run( mainloop );

  exit(0);
}



  

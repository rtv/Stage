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
 * CVS: $Id: main.cc,v 1.74 2003-10-12 19:30:32 rtv Exp $
 */


#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>

//#define DEBUG

//#include "stage.h"
#include "world.hh"
#include "entity.hh"
#include "rtkgui.hh" // for rtk_init()

#define HELLOWORLD "** Stage "VERSION" **"
#define BYEWORLD "Stage finished"

int quit = 0; // set true by the GUI when it wants to quit 

// globals
GHashTable* global_model_table = g_hash_table_new(g_int_hash, g_int_equal);
int global_next_available_id = 1;
int global_num_clients = 0; // might be interesting to know?
//GList* global_sub_clients = NULL; // list of subscription-based clients


// forward declare
gboolean StgClientRead( GIOChannel* channel, 
			GIOCondition condition, 
			gpointer data );

gboolean StgSubClientRead( GIOChannel* channel, 
			GIOCondition condition, 
			gpointer data );

gboolean StgClientHup( GIOChannel* channel, 
			GIOCondition condition, 
			gpointer data );

  // prints the object tree on stdout
void StgPrintTree( GNode* treenode, gpointer _prefix )
{
  GString* prefix = (GString*)_prefix;

  CEntity* foo = (CEntity*)(treenode->data);  
  g_assert( foo );
  
  GString* pp = NULL;

  if( prefix )
    pp = g_string_new( prefix->str );
  else
    pp = g_string_new( "" );
  
  printf( "%s%d:%s\n", pp->str, foo->id, foo->name->str );

  pp = g_string_prepend( pp, "  " );
   
  g_node_children_foreach( treenode, G_TRAVERSE_ALL, StgPrintTree, pp ) ;

  g_string_free( pp, TRUE );
}
  
///////////////////////////////////////////////////////////////////////////

// write a stg_property_t on the channel returns TRUE on success, else FALSE
gboolean StgPropertyWrite( GIOChannel* channel, stg_property_t* prop )
{
  gboolean failed = FALSE;

  ssize_t result = 
    stg_property_write_fd( g_io_channel_unix_get_fd(channel), prop );
  
  if( result < 0 )
    {
      PRINT_MSG1( "closing connection (fd %d)", 
		  g_io_channel_unix_get_fd(channel) );
      
      /* zap this connection */
      g_io_channel_shutdown( channel, TRUE, NULL );
      g_io_channel_unref( channel );
    }      
  
  PRINT_DEBUG( "done" );

  return( !failed );
}

stg_client_data_t* stg_rr_client_create( pid_t pid, GIOChannel* channel )
{
  // store the client's PID to we can send it signals 
  stg_client_data_t* cli = 
    (stg_client_data_t*)calloc(1,sizeof(stg_client_data_t) );
  
  g_assert( cli );
  
  // set up this client
  cli->pid = pid;
  cli->channel = channel;  
  cli->source_in  = g_io_add_watch( channel, G_IO_IN, StgClientRead, cli );
  cli->source_hup = g_io_add_watch( channel, G_IO_HUP, StgClientHup, cli );
  
  global_num_clients++; 
  
  return cli;
}

stg_client_data_t* stg_sub_client_create( pid_t pid, GIOChannel* channel )
{
  // store the client's PID to we can send it signals 
  stg_client_data_t* cli = 
    (stg_client_data_t*)calloc(1,sizeof(stg_client_data_t) );
  
  g_assert( cli );
  
  // set up this client
  cli->pid = pid;
  cli->channel = channel;  
  cli->source_in  = g_io_add_watch( channel, G_IO_IN, StgSubClientRead, cli );
  cli->source_hup = g_io_add_watch( channel, G_IO_HUP, StgClientHup, cli );
  
  // add this client to the list of subscribers to this world
  //global_sub_clients = g_list_append( global_sub_clients, cli );
  
  global_num_clients++; 
  
  return cli;
}

void stg_client_destroy( stg_client_data_t* cli )
{
  // cancel watches we installed on this channel
  if( cli->source_in  ) g_source_remove( cli->source_in );
  if( cli->source_hup ) g_source_remove( cli->source_hup );
  
  // and kill it
  g_io_channel_shutdown( cli->channel, TRUE, NULL ); // zap this connection 
  g_io_channel_unref( cli->channel );
  
  // destroy any worlds that were created by this client
  while( cli->worlds )
    {
      PRINT_DEBUG1( "destroying world %p", cli->worlds->data);
      stg_world_destroy( (stg_world_t*)cli->worlds->data );
    }
  
  PRINT_DEBUG( "finished destroying worlds" );

  // zap client
  //kill( cli->pid, SIGINT );

  free( cli );
  
  // one client fewer
  global_num_clients--;
}


gboolean StgClientRead( GIOChannel* channel, 
		     GIOCondition condition, 
		     gpointer data )
{
  PRINT_DEBUG( "client read" );
  g_assert(channel);
  g_assert(data);
  
  // the data tells us which client number this is
  stg_client_data_t *cli = (stg_client_data_t*)data;
  
  stg_property_t* prop = 
    stg_property_read_fd( g_io_channel_unix_get_fd(channel) );

  if( prop == NULL )
    {
      PRINT_MSG1( "Failed to read from client (fd %d). Shutting it down.",  
		  g_io_channel_unix_get_fd(channel) );
      
      stg_client_destroy( cli );      
      return FALSE; // cancel this callback (just in case - we
		    // probably cancelled it already in
		    // StgClientDestroy() ).
    }
  else
    {
      PRINT_DEBUG1( "got a property with id %d", prop->id );  
      
      if( prop->action == STG_NOOP )
	{
	  PRINT_WARN1( "ignoring a NOOP property for model %d", prop->id );
	}
      else
	{
	  // a property gets created below and returned to the client
	  stg_property_t* reply = NULL;

	  
	  switch( prop->property )
	    {
	    case STG_PROP_CREATE_WORLD:
	      {
		// check that we have the right size data
		g_assert( (prop->len == sizeof(stg_world_create_t)) );
		
		// create a world object
		stg_world_t* aworld = 
		  stg_world_create( cli, 
				    global_next_available_id++,
				    (stg_world_create_t*)prop->data );
		g_assert( aworld );
		
		// add the new world to the hash table with its id as
		// the key (this ID should not already exist)
		g_assert( g_hash_table_lookup(global_model_table, &aworld->id)
			  == NULL ); 
		g_hash_table_insert(global_model_table, &aworld->id, 
				    aworld->node );  

		PRINT_DEBUG2( "Created world %p on channel %p",
			      aworld, channel );
		
		// reply with the id of the world
		reply = stg_property_create();
		reply->id = aworld->id; 		
	      }
	      break;
	      
	    case STG_PROP_CREATE_MODEL:
	      {
		// check that we have the right size data
		g_assert( (prop->len == sizeof(stg_entity_create_t)) );
		
#ifdef DEBUG
		stg_entity_create_t* create = (stg_entity_create_t*)prop->data;
		PRINT_DEBUG2( "creating model name \"%s\" xparent %d",
			      create->name, create->parent_id );
#endif
		// create a new entity
		CEntity* ent = NULL;	    
		g_assert((ent = new CEntity((stg_entity_create_t*)prop->data,
					    global_next_available_id++ )));

		// add the new model to the world's hash table with
		// its id as the key (this ID should not already
		// exist)
		g_assert( g_hash_table_lookup( global_model_table, &ent->id ) 
			  == NULL ); 
		g_hash_table_insert( global_model_table, &ent->id, ent->node );
		
		//ent->Startup();

		// reply with the id of the entity
		reply = stg_property_create();
		reply->id = ent->id; 
	      }
	      break;
	      
	    default: // all other props we need to look up an existing object
	      {	    
		GNode* node = 
		  (GNode*)g_hash_table_lookup( global_model_table, &prop->id );
		
		if( node == NULL )
		  {
		    PRINT_WARN2( "Ignoring unknown model (%d %s).",
				 prop->id, 
				 stg_property_string(prop->property) );
		    
		    reply = stg_property_create();
		    reply->id = -1; // indicate failed request
		  }
		else 
		  switch( prop->property )		    
		    {
		    case STG_PROP_DESTROY_WORLD: 
		      {
			puts( "DESTROY WORLD REQUEST" );
			stg_world_t* world = (stg_world_t*)node->data;	      
			g_hash_table_remove( global_model_table, &world->id );
			stg_world_destroy( world );			
		      }
		      break;
		      
		    case STG_PROP_DESTROY_MODEL:
		      {
			CEntity* ent = (CEntity*)node->data;
			g_hash_table_remove( global_model_table, &ent->id );
			delete ent;
		      }
		      break;
		      
		    default: // it must be a model. 
		      {
			CEntity* ent = (CEntity*)node->data;
			
			switch( prop->action )
			  {
			  case STG_SET:
			    ent->SetProperty( prop->property,
					      prop->data, prop->len );
			    reply = stg_property_create();
			    reply->id = ent->id; // indicate success 
			    break;
			  case STG_GET:
			    reply = ent->GetProperty( prop->property );
			    break;
			  case STG_SETGET:
			    ent->SetProperty( prop->property, 
					      prop->data, prop->len );
			    reply = ent->GetProperty( prop->property );
			    break;
			  case STG_GETSET:
			    reply = ent->GetProperty( prop->property );
			    ent->SetProperty( prop->property, 
					      prop->data, prop->len );
			    break;
			    
			  default: PRINT_WARN1( "unknown prop action (%d)",
						prop->action );
			 
			    reply = stg_property_create();
			    reply->id = -1;  // indicate failed request
			    break;
			  }
		      }
		      break;
		    }
	      }
	    }
	  
	  // write reply
	  g_assert(reply);
	  StgPropertyWrite( channel, reply );  
	  stg_property_free( reply );      
	}
    }
  
  stg_property_free( prop );    
  return TRUE;
}


// read from a subscription-based client
gboolean StgSubClientRead( GIOChannel* channel, 
		

	   GIOCondition condition, 
			   gpointer data )
{
  PRINT_DEBUG( "sub client read" );
  g_assert(channel);
  g_assert(data);
  
  // the data tells us which client number this is
  stg_client_data_t *cli = (stg_client_data_t*)data;
  
  // read a subscription request
  stg_subscription_t sub;
  size_t bytes_read = 0;
  if(  g_io_channel_read_chars( channel, 
				(char*)&sub, sizeof(sub),
				&bytes_read, NULL )
       != G_IO_STATUS_NORMAL ) 
    {
      PRINT_DEBUG1( "abnormal read of %d bytes", bytes_read );
      perror( "read error" );
      return FALSE; // fail
    }
  
  PRINT_WARN2( "received subscription of %d:%d", 
	       sub.id, sub.prop );


  // lookup the world id
  GNode* node = 
    (GNode*)g_hash_table_lookup( global_model_table, &sub.id );

  stg_world_t* world = (stg_world_t*)node->data;

  world->subscribers = g_list_append( world->subscribers, cli );

  printf( "added subscriber client %p fd %d\n", 
	  cli, g_io_channel_unix_get_fd(cli->channel) );

  
  /* 
     PRINT_WARN( "sending ack" );

  stg_ack_t ack = STG_ACK;
  size_t bytes_written = 0;
  g_io_channel_write_chars( channel, 
			    (char*)&ack, (gssize)sizeof(ack),
			    &bytes_written, NULL );
  
  g_assert( bytes_written == sizeof(ack) );
  g_io_channel_flush( channel, NULL );
  
  PRINT_WARN( "ack sent" );
  */

  return TRUE;
}


gboolean StgClientHup( GIOChannel* channel, 
		    GIOCondition condition, 
		    gpointer data )
{
  PRINT_MSG( "client HUP. Closing connection." );

  g_io_channel_shutdown( channel, TRUE, NULL );
  g_io_channel_unref( channel );
  
  return FALSE; // cancels this callback
}

gboolean StgClientAcceptConnection( GIOChannel* channel, GHashTable* table )
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
  stg_greeting_t greet, reply;
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
  
  // read the type of service requested
  stg_tos_t tos;
   if(  g_io_channel_read_chars( client, 
				(char*)&tos, sizeof(tos),
				&bytes_read, NULL )
       != G_IO_STATUS_NORMAL ) 
    {
       PRINT_DEBUG1( "abnormal read of %d bytes", bytes_read );
       perror( "read error" );
       return FALSE; // fail
    }

   switch( tos )
     {
     case STG_TOS_REQUESTREPLY:
       PRINT_MSG( "Request/reply connection requested" );
       g_assert( stg_rr_client_create( greet.pid, client ) );
       break;
     case STG_TOS_SUBSCRIPTION:
       PRINT_MSG( "Subscription connection requested" );
       g_assert( stg_sub_client_create( greet.pid, client ) );
       break;
     default:
       PRINT_ERR1( "unknown connection type (%d) requested", 
		   tos );
       return FALSE; // fail
       break;
     }

  // write the reply
  reply.code = STG_CLIENT_GREETING;
  reply.pid = getpid(); // reply with my PID in case that's useful

  g_io_channel_write_chars( client, 
			    (char*)&reply, (gssize)sizeof(reply),
			    &bytes_written, NULL );
  
  g_assert( bytes_written == sizeof(reply) );
  g_io_channel_flush( client, NULL );

  return TRUE; // success
}      

gboolean StgServiceWellKnownPort( GIOChannel* channel, 
			       GIOCondition condition, 
			       gpointer table )
{
  switch( condition )
    {
    case G_IO_IN: PRINT_DEBUG( "CONNECT" ); 
      if( StgClientAcceptConnection( channel, (GHashTable*)table ) == -1 )
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

GIOChannel* StgServerCreate( void )
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

  return( g_io_channel_unix_new( fd ) );
}

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

int main( int argc, char** argv )
{
  puts( HELLOWORLD );
  
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
 
  stg_gui_init( &argc, &argv );
  
  // create a socket bound to Stage's well-known-port
  GIOChannel* channel = StgServerCreate();
  g_assert( channel );
  
  // watch for these events on the well-known-port
  g_io_add_watch(channel, G_IO_IN, StgServiceWellKnownPort, global_model_table);
  
  PRINT_MSG1( "Accepting connections on port %d", STG_DEFAULT_SERVER_PORT );

  GMainLoop* mainloop = g_main_loop_new( NULL, TRUE );
  g_main_loop_run( mainloop );

  exit(0);
}



  

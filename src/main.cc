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
 * CVS: $Id: main.cc,v 1.66 2003-08-26 18:59:58 rtv Exp $
 */


#include <stdlib.h>
#include <signal.h>

//#define DEBUG

//#include "stage.h"
#include "world.hh"
#include "entity.hh"
#include "rtkgui.hh" // for rtk_init()

#define HELLOWORLD "** Stage "VERSION" **"
#define BYEWORLD "Stage finished"

int quit = 0; // set true by the GUI when it wants to quit 

// globals
GHashTable* global_hash_table = g_hash_table_new(g_int_hash, g_int_equal);
int global_next_available_id = 1;

// contains the PIDs of all connected clients
GArray* global_client_pids = g_array_new( FALSE, TRUE, sizeof(pid_t) );


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


gboolean StgClientRead( GIOChannel* channel, 
		     GIOCondition condition, 
		     gpointer table )
{
  PRINT_DEBUG( "client read" );
  g_assert(channel);
  g_assert(table);
  
  // store associations between channels and worlds
  static GHashTable* channel_world_map = g_hash_table_new(NULL,NULL);
  
  stg_property_t* prop = 
    stg_property_read_fd( g_io_channel_unix_get_fd(channel) );
  
  if( prop == NULL )
    {
      PRINT_MSG1( "read failed on fd %d. Shutting it down.",  
		  g_io_channel_unix_get_fd(channel) );
      
      g_io_channel_shutdown( channel, TRUE, NULL ); // zap this connection 
      g_io_channel_unref( channel );
      
      PRINT_DEBUG1( "Destroying worlds associated with channel %p",
		    channel );
      
      // find the list of worlds created on this channel
      GList* doomed_list = 
	(GList*)g_hash_table_lookup( channel_world_map, channel );
      
      if( doomed_list )
	{
	  // delete all the worlds in the list
	  for( GList* lel = doomed_list; lel; lel = g_list_next(lel) )
	    {
	      PRINT_DEBUG2( "destroying world %p (node %p)", lel->data, lel);
	      //delete (CWorld*)lel->data;
	      stg_world_destroy( (stg_world_t*)lel->data );

	    }
	  
	  g_list_free( doomed_list );
	}
      
      // remove the channel from the static map
      g_hash_table_remove( channel_world_map, channel );
      
      return FALSE; // cancel this callback
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
	  stg_property_t* reply = NULL;
	  // indicates whether we need to free the reply's memory
	  bool free_reply = FALSE; 
	  
	  switch( prop->property )
	    {
	    case STG_PROP_CREATE_WORLD:
	      {
		// check that we have the right size data
		g_assert( (prop->len == sizeof(stg_world_create_t)) );
		
		// create a world object
		//CWorld* aworld = NULL;
		//g_assert( (aworld = new CWorld( channel, (stg_world_create_t*)prop->data)));
		//aworld->Startup();
		
		stg_world_t* aworld = 
		  stg_world_create( channel, (stg_world_create_t*)prop->data );
		g_assert( aworld );
		stg_world_startup( aworld );

		prop->id = aworld->id; // fill in the id of the created object

		// and reply with the completed request
		reply = prop;
		
		PRINT_DEBUG2( "Associating world %p with channel %p",
			      aworld, channel );
		
		// find the list of worlds created on this channel (possibly NULL)
		GList* world_list = 
		  (GList*)g_hash_table_lookup( channel_world_map, channel );
		
		// add the new world to the list
		world_list = g_list_append( world_list, aworld );
		
		// stash the new list in the hash table
		g_hash_table_insert( channel_world_map, channel, world_list );
	      }
	      break;
	      
	    case STG_PROP_CREATE_MODEL:
	      {
		// check that we have the right size data
		g_assert( (prop->len == sizeof(stg_entity_create_t)) );
		
#ifdef DEBUG
		stg_entity_create_t* create = (stg_entity_create_t*)prop->data;
		PRINT_DEBUG3( "creating model name \"%s\" token \"%s\ parent %d",
			      create->name, create->token, create->parent_id );
#endif
		// create a new entity
		CEntity* ent = NULL;	    
		g_assert((ent = new CEntity((stg_entity_create_t*)prop->data)));
		ent->Startup();
		
		prop->id = ent->id; // fill in the id of the created entity
		// and reply with the completed request
		reply = prop;		
	      }
	      break;
	      
	      
	    default: // all other props we need to look up an existing object
	      {	    
		GNode* node = 
		  (GNode*)g_hash_table_lookup( global_hash_table, &prop->id );
		
		if( node == NULL )
		  {
		    PRINT_WARN1( "Failed to handle message for unknown model"
				 " id (%d).", prop->id );
		    
		    // if a reply was required, we send the original
		    // request back with an id of -1 to indicate failure
		    if( prop->action == STG_SETGET || 
			prop->action == STG_GETSET || 
			prop->action == STG_GET )
		      {
			PRINT_WARN( "Replying with invalid model id" );
			prop->id = -1;
			reply = prop;
		      }
		  }
		else 
		  switch( prop->property )		    
		    {
		    case STG_PROP_DESTROY_WORLD: 
		      // delete the object, invalidate the id and
		      // reply with the original request
		      //delete (CWorld*)node->data;
		      stg_world_destroy( (stg_world_t*)node->data );
		      prop->id = -1; 
		      reply = prop;
		      break;
		      
		    case STG_PROP_DESTROY_MODEL:
		      delete (CEntity*)node->data;
		      prop->id = -1; 
		      reply = prop;
		      break;
		      
		    default: // it must be a model. 
		      {
			CEntity* ent = (CEntity*)node->data;
			
			switch( prop->action )
			  {
			  case STG_SET:

			    ent->SetProperty( prop->property,
					      prop->data, prop->len );
			    break;
			  case STG_GET:
			    reply = ent->GetProperty( prop->property );
			    free_reply = TRUE;
			    break;
			  case STG_SETGET:
			    ent->SetProperty( prop->property, 
					      prop->data, prop->len );
			    reply = ent->GetProperty( prop->property );
			    free_reply = TRUE;
			    break;
			  case STG_GETSET:
			    reply = ent->GetProperty( prop->property );
			    free_reply = TRUE;
			    ent->SetProperty( prop->property, 
					      prop->data, prop->len );
			    break;
			    
			  default: PRINT_WARN1( "unknown prop action (%d)",
						prop->action );
			    break;
			  }
		      }
		      break;
		    }
	      }
	    }
	  
	  if( reply )
	    {
	      // write reply, freeing memory if allocated
	      StgPropertyWrite( channel, reply );  
	      if( free_reply ) stg_property_free( reply );
	    }      
	}
    }
  
  stg_property_free( prop );    
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
  
  // store the client's PID to we can send it signals 
  global_client_pids = g_array_append_val( global_client_pids, greet.pid );
  
  // write the reply
  reply.code = STG_CLIENT_GREETING;
  reply.pid = getpid(); // reply with my PID in case that's useful

  g_io_channel_write_chars( client, 
			    (char*)&reply, (gssize)sizeof(reply),
			    &bytes_written, NULL );
  
  g_assert( bytes_written == sizeof(reply) );
  g_io_channel_flush( client, NULL );
  
  // request data from this channel
  g_io_add_watch( client, G_IO_IN, StgClientRead, table );
  g_io_add_watch( client, G_IO_HUP, StgClientHup, table );
  
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
    default: printf( "unknown channel condition %d\n", condition );
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
  g_io_add_watch(channel, G_IO_IN, StgServiceWellKnownPort, global_hash_table);
  
  GMainLoop* mainloop = g_main_loop_new( NULL, TRUE );
  g_main_loop_run( mainloop );

  exit(0);
}



  


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


#include <glib.h>

#define DEBUG

#include "replace.h"
#include "stageclient.h"
#include "bitmap.h"
#include "worldfile.hh"

double world_time = 0.0;

// Signal catchers ---------------------------------------------------

void catch_interrupt(int signum)
{
  puts( "\nInterrupt signal." );

  stg_quit_request();
}

void catch_exit( void )
{
  puts( "test client finished" );
}

int install_signal_catchers( void )
{
  atexit( catch_exit );
  
  if( signal( SIGINT, catch_interrupt ) == SIG_ERR )
    {
      PRINT_ERR( "error setting interrupt signal catcher" );
      exit( -1 );
    }

  if( signal( SIGQUIT, catch_interrupt ) == SIG_ERR )
    {
      PRINT_ERR( "error setting interrupt signal catcher" );
      exit( -1 );
    }

  return 0; // ok
}




void message_dispatch( sc_t* cli, stg_msg_t* msg )
{
  //PRINT_DEBUG2( "handling message type %d len %d", 
  //	msg->type,(int)msg->len );
  
  switch( msg->type )
    {
    case STG_MSG_WORLD_SAVE:
      PRINT_WARN( "SAVE" );
      
      break;

    case STG_MSG_CLIENT_PROPERTY: 
      {
	stg_prop_t* prop = (stg_prop_t*)msg->payload;

#if 1
	printf( "[%.3f] received property %d:%d:%d(%s) %d/%d bytes\n",
		prop->timestamp,
		prop->world, 
		prop->model, 
		prop->prop,
		stg_property_string(prop->prop),
		(int)prop->datalen,
		(int)msg->payload_len );
#endif	

	world_time = prop->timestamp;
	printf( "[%.3f] ", world_time );
	
	// stash the data in the client-side model tree
	PRINT_DEBUG2( "looking up client-side model for %d:%d",
		      prop->world, prop->model );
	
	sc_model_t* mod = sc_get_model_serverside( cli, prop->world, prop->model );
	
	if( mod == NULL )
	  {
	    PRINT_WARN2( "no such model %d:%d found in the client",
			 prop->world, prop->model );
	  }
	else
	  { 
	    PRINT_DEBUG4( "stashing prop %d(%s) bytes %d in model \"%s\"",
			  prop->prop, stg_property_string(prop->prop),
			  (int)prop->datalen, mod->token->token );

	    sc_model_prop_with_data( mod, prop->prop, prop->data, prop->datalen );
	  }	    
	

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
	    {
	      stg_laser_sample_t* samples = (stg_laser_sample_t*)prop->data;
	      int ls_count = prop->datalen / sizeof(stg_laser_sample_t);
	      
	      printf( "received %d laser samples\n", ls_count );
	    }
	    
	  case STG_PROP_TIME:
	    puts( "<time>" );
	    break;

	  default:
	    PRINT_WARN2( "property type %d(%s) unhandled",
			 prop->prop, stg_property_string(prop->prop ) ); 
	  }
      }
      break;
  
    default:
      PRINT_WARN1( "message type %d unhandled", msg->type ); 
      break;
    }
}

void model_save(  sc_model_t* model, CWorldFile* wf )
{
  PRINT_DEBUG1( "Saving model \"%s\"", model->token->token );
  
  // find a pose property in this model
  
  PRINT_DEBUG( "looking up pose property" );
  int type = STG_PROP_POSE;
  stg_property_t* prop = 
    (stg_property_t*)g_hash_table_lookup( model->props, &type ); 
  
  if( prop && prop->len == sizeof(stg_pose_t) )
    {
      stg_pose_t* pose = (stg_pose_t*)prop->data;
     
      PRINT_DEBUG( "writing pose property" );
 
      wf->WriteTupleLength( model->section, "pose", 0, pose->x );
      wf->WriteTupleLength( model->section, "pose", 1, pose->y );
      wf->WriteTupleAngle( model->section, "pose", 2,  pose->a );
    }
}  

void model_save_cb( gpointer key, gpointer value, gpointer user )
{
  model_save( (sc_model_t*)value, (CWorldFile*)user );
}

void world_save(  sc_world_t* world, CWorldFile* wf )
{
  PRINT_DEBUG1( "Saving world \"%s\"", world->token->token  );

  g_hash_table_foreach( world->models_id, model_save_cb, wf );
}

void world_save_cb( gpointer key, gpointer value, gpointer user )
{
  world_save( (sc_world_t*)value, (CWorldFile*)user );
}


void save( sc_t* cli, CWorldFile* wf )
{
  // get everything to write its latest data into the worldfile database
  g_hash_table_foreach( cli->worlds_id, world_save_cb, wf );
  
  // then export the database to a file
  wf->Save( NULL ); // uses current filename
}


int main( int argc, char* argv[] )
{
  printf( "TESTING Stage v%s\n", VERSION );
  install_signal_catchers();
  
  int num_worlds = 1;
  int num_models = 3;
  int server_port = STG_DEFAULT_SERVER_PORT;
  char* server_host = STG_DEFAULT_SERVER_HOST;
  
  // parse args
  int a;
  for( a=1; a<argc; a++ )
    {
      if( strcmp( "-w", argv[a] ) == 0 )
	{
	  num_worlds = atoi( argv[++a] );
	  printf( " [worlds %d]", num_worlds ); fflush(stdout);
	  continue;
	}
      
      if( strcmp( "-m", argv[a] ) == 0 )
	{
	  num_models = atoi(argv[++a]);
	  printf( " [models %d]", num_models ); fflush(stdout);
	  continue;
	}
      
      if( strcmp( "-p", argv[a] ) == 0 )
	{
	  server_port = atoi( argv[++a] );
	  printf( " [port %d]", server_port ); fflush(stdout);
	  continue;
	}	  
      
      if( strcmp( "-h", argv[a] ) == 0 )
	{
	  server_host = argv[++a];
	  printf( " [host %s]", server_host ); fflush(stdout);
	  continue;
	}
    }
  
  printf( "Loading world file \"%s\"\n", argv[1] );
  CWorldFile* worldfile = NULL;
  assert( worldfile = new CWorldFile() );  
  worldfile->Load( argv[1] );
  puts( "loading done" );
  
  printf( "building client-side models\n" );
  
  // create a client
  sc_t* client = sc_create();  

  // create a single world
  sc_world_t* world = 
    sc_world_create( stg_token_create( "Player world", STG_T_NUM, 99 ) );
  
  sc_addworld( client, world );
  
  // create a special model for the background
  stg_token_t* token = stg_token_create( "root", STG_T_NUM, 99 );
  sc_model_t* root = sc_model_create( world, NULL, token );
  root->section = 0;
  sc_world_addmodel( world, root );
  
  stg_size_t sz;
  sz.x = worldfile->ReadTupleLength(0, "size", 0, 10.0 );
  sz.y = worldfile->ReadTupleLength(0, "size", 1, 10.0 );
  sc_model_prop_with_data( root, STG_PROP_SIZE, &sz, sizeof(sz) );

  stg_pose_t origin;
  origin.x = sz.x/2.0;
  origin.y = sz.y/2.0;
  origin.a = 0.0;
  sc_model_prop_with_data( root, STG_PROP_ORIGIN, &origin, sizeof(origin) );

  stg_movemask_t mm = 0;
  sc_model_prop_with_data( root, STG_PROP_MOVEMASK, &mm, sizeof(mm) ); 
  
  const char* colorstr = worldfile->ReadString(root, "color", "black" );
  stg_color_t color = stg_lookup_color( colorstr );
  sc_model_prop_with_data( root, STG_PROP_COLOR, &color,sizeof(color));
  
  stg_bool_t boundary;
  boundary = worldfile->ReadInt(0, "boundary", 1 );
  sc_model_prop_with_data(root, STG_PROP_BOUNDARY, &boundary,sizeof(boundary));
  
  const char* bitmapfile = worldfile->ReadString(0, "bitmap", NULL );
  if( bitmapfile )
    {
      stg_rotrect_t* rects = NULL;
      int num_rects = 0;
      stg_load_image( bitmapfile, &rects, &num_rects );
      
      sc_model_prop_with_data( root, STG_PROP_RECTS, 
			       rects, num_rects * sizeof(stg_rotrect_t ));
    }
 

  // Iterate through sections and create client-side models
  for (int section = 1; section < worldfile->GetEntityCount(); section++)
    {
      const char *namestr = worldfile->GetEntityType(section);
      stg_token_t* token = stg_token_create( (char*)namestr, STG_T_NUM, 99 );
      sc_model_t* mod = sc_model_create( world, NULL, token );
      mod->section = section;
      sc_world_addmodel( world, mod );
      
      stg_pose_t pose;
      pose.x = worldfile->ReadTupleLength(section, "pose", 0, 0);
      pose.y = worldfile->ReadTupleLength(section, "pose", 1, 0);
      pose.a = worldfile->ReadTupleAngle(section, "pose", 2, 0);      
      sc_model_prop_with_data( mod, STG_PROP_POSE, &pose, sizeof(pose) );
      
      stg_size_t sz;
      sz.x = worldfile->ReadTupleLength(section, "size", 0, 0.4 );
      sz.y = worldfile->ReadTupleLength(section, "size", 1, 0.4 );
      sc_model_prop_with_data( mod, STG_PROP_SIZE, &sz, sizeof(sz) );
      
      stg_bool_t nose;
      nose = worldfile->ReadInt( section, "nose", 0 );
      sc_model_prop_with_data( mod, STG_PROP_NOSE, &nose, sizeof(nose) );
      
      stg_bool_t boundary;
      boundary = worldfile->ReadInt( section, "boundary", 0 );
      sc_model_prop_with_data( mod, STG_PROP_BOUNDARY, &boundary,sizeof(boundary));
      
      const char* colorstr = worldfile->ReadString( section, "color", "red" );
      if( colorstr )
	{
	  stg_color_t color = stg_lookup_color( colorstr );
	  PRINT_DEBUG2( "stage color %s = %X", colorstr, color );
	  sc_model_prop_with_data( mod, STG_PROP_COLOR, &color,sizeof(color));
	}

      const char* bitmapfile = worldfile->ReadString( section, "bitmap", NULL );
      if( bitmapfile )
	{
	  stg_rotrect_t* rects = NULL;
	  int num_rects = 0;
	  stg_load_image( bitmapfile, &rects, &num_rects );
	  
	  sc_model_prop_with_data( mod, STG_PROP_RECTS, 
				   rects, num_rects * sizeof(stg_rotrect_t ));
	}

      // Load the geometry of a ranger array
      int scount = worldfile->ReadInt( section, "scount", 0);
      if (scount > 0)
	{
	  char key[256];
	  stg_ranger_config_t* configs = calloc( sizeof(stg_ranger_config_t), scount );
	  int i;
	  for(i = 0; i < scount; i++)
	    {
	      snprintf(key, sizeof(key), "spose[%d]", i);
	      configs[i].pose.x = worldfile->ReadTupleLength(section, key, 0, 0);
	      configs[i].pose.y = worldfile->ReadTupleLength(section, key, 1, 0);
	      configs[i].pose.a = worldfile->ReadTupleAngle(section, key, 2, 0);

	      snprintf(key, sizeof(key), "ssize[%d]", i);
	      configs[i].size.x = worldfile->ReadTupleLength(section, key, 0, 0.01);
	      configs[i].size.y = worldfile->ReadTupleLength(section, key, 1, 0.05);

	      snprintf(key, sizeof(key), "sbounds[%d]", i);
	      configs[i].bounds_range.min = 
		worldfile->ReadTupleLength(section, key, 0, 0);
	      configs[i].bounds_range.max = 
		worldfile->ReadTupleLength(section, key, 1, 5.0);

	      snprintf(key, sizeof(key), "sfov[%d]", i);
	      configs[i].fov = worldfile->ReadAngle(section, key, 30 );
	    }
	  
	  PRINT_WARN1( "loaded %d ranger configs", scount );	  
	  sc_model_prop_with_data( mod, STG_PROP_RANGERCONFIG,
				   configs, scount * sizeof(stg_ranger_config_t) );

	  free( configs );
	}
      
      
    }
  
  printf( "building client-side models done.\n" );

  puts( "connecting to server" );
  sc_connect( client, server_host, server_port );
  puts( "connection ok" );
  
  puts( "uploading worldfile to server" );
  sc_push( client );
  puts( "uploading done" );
  
  sc_model_t* mod = sc_get_model( client, "Player world", "sonar" );
  if( mod )
    {
      sc_model_subscribe( client, mod, STG_PROP_RANGERDATA, 0.1 );
      sc_model_subscribe( client, mod, STG_PROP_LASERDATA, 0.2 );
      sc_model_subscribe( client, mod, STG_PROP_POSE, 1.0 );
    }
  else
    PRINT_ERR( "no such model" );

  mod = sc_get_model( client, "Player world", "pioneer" );
  if( mod )
    {
      sc_model_subscribe( client, mod, STG_PROP_RANGERDATA, 0.1 );
      sc_model_subscribe( client, mod, STG_PROP_POSE, 1.0 );
    }
  else
    PRINT_ERR( "no such model" );
  
  //int id = wf_model_id_from_name( "budgie" );
  //wc_subscribe( client, 0, id, STG_PROP_POSE, 1.0 ); 

  //sleep(1000);

  //exit(-1);

  //stg_connection_model_upload( con, models );
  
  //sleep(1);


  // srandomdev(); // seed the rng - Darwin
  //srandom( time(NULL) ); // seed the rng

/*   stg_id_t world = 0; */
/*   stg_id_t model= 0; */
  
/*   int c=1;  */
  
/*   stg_id_t* world_ids = calloc( sizeof(stg_id_t), num_worlds ); */
/*   stg_id_t** model_ids = calloc( sizeof(stg_id_t*), num_worlds ); */
  
/*   int w; */
/*   for( w=0; w<num_worlds; w++ ) */
/*     { */
/*       char name[64]; */
/*       snprintf( name, 64, "world_%d", w ); */
      
/*       world_ids[w] = stg_world_new( client, name, 20.0, 20.0, 50, 0.1, 0.1 ); */
/*       model_ids[w] = calloc( sizeof(stg_id_t), num_models ); */
      
/*       int t; */
/*       for( t=0; t<num_models; t++ ) */
/* 	{ */
/* 	  char name[64]; */
/* 	  snprintf( name, 64, "robot_%d", t ); */
/* 	  model_ids[w][t] = stg_model_new( client, world_ids[w], name ); */
/* 	} */
/*     } */



/*   stg_rotrect_t* rects = NULL; */
/*   int rect_count = 0; */

/*   load_image( "worlds/cave.png", &rects, &rect_count ); */

/*   //stg_rotrect_t* sub_rects = NULL; */
/*   //int sub_rect_count = 0; */

/*   //load_image( "worlds/submarine.png", &sub_rects, &sub_rect_count ); */


/*   printf( "rect_count %d\n", rect_count ); */

/*   /\*int r=0; */
/*   for( r=0; r<rect_count; r++ ) */
/*     printf( "[%d - %.2f %.2f %.2f %.2f %.2f] ",  */
/* 	    rect_count,  */
/* 	    rects[r].x,  */
/* 	    rects[r].y,  */
/* 	    rects[r].a,  */
/* 	    rects[r].w,  */
/* 	    rects[r].h );  */
/*   *\/ */


/*   // configure and subscribe in a separate loop so the incoming data doesn't confuse  */
/*   // the stg_X_new calls above */
/*   for( w=0; w<num_worlds; w++ ) */
/*     { */
/*       // configure the zeroth model as a wall */
/*       stg_property_set( client, world_ids[w], model_ids[w][0], */
/* 			STG_PROP_RECTS, rects, rect_count * sizeof(stg_rotrect_t) ); */
      
/*       stg_bool_t boundary = 1;       */
/*       stg_property_set( client, world_ids[w], model_ids[w][0],  */
/* 			STG_PROP_BOUNDARY, &boundary, sizeof(boundary) ); */

/*       stg_color_t col = 0; */
/*       stg_property_set( client, world_ids[w], model_ids[w][0],  */
/* 			STG_PROP_COLOR, &col, sizeof(col) ); */

/*       stg_size_t size; */
/*       size.x = 20.0; */
/*       size.y = 20.0; */
      
/*       stg_property_set( client, world_ids[w], model_ids[w][0],  */
/* 			STG_PROP_SIZE, &size, sizeof(size) ); */
      
/*       stg_pose_t pose; */
/*       pose.x = size.x / 2.0; */
/*       pose.y = size.y / 2.0; */
/*       pose.a = 0.0; */

/*       stg_property_set( client, world_ids[w], model_ids[w][0],  */
/* 			STG_PROP_POSE, &pose, sizeof(pose) ); */


/*       int movemask = 0; */
/*       stg_property_set( client, world_ids[w], model_ids[w][0],  */
/* 			STG_PROP_MOVEMASK, &movemask, sizeof(movemask) ); */
      
/*       //stg_pose_t origin; // shift the origin to the bottom left corner */
/*       //origin.x = size.x / 2.0; */
/*       //origin.y = size.y / 2.0; */
/*       //origin.a = 0.0; */
/*       //stg_property_set( client, world_ids[w], model_ids[w][0],  */
/*       //		STG_PROP_ORIGIN, &origin, sizeof(origin) ); */

      
/*       // configure the 1th model as a sub */
/*       //stg_property_set( client, world_ids[w], model_ids[w][1], */
/*       //		STG_PROP_RECTS, sub_rects,  */
/*       //	sub_rect_count * sizeof(stg_rotrect_t) ); */
      
/*       col = 0xFF0000; */
/*       stg_property_set( client, world_ids[w], model_ids[w][1],  */
/* 			STG_PROP_COLOR, &col, sizeof(col) ); */

/*       stg_pose_t origin; */
/*       origin.x = -0.3; */
/*       origin.y = 0.15; */
/*       origin.a = 1.0; */

/*       stg_property_set( client, world_ids[w], model_ids[w][1],  */
/* 			STG_PROP_ORIGIN, &origin, sizeof(origin) ); */

/*       size.x = 1.0; */
/*       size.y = 0.5; */
/*       stg_property_set( client, world_ids[w], model_ids[w][1],  */
/* 			STG_PROP_SIZE, &size, sizeof(size) ); */
      
/*       int t; */
/*       for( t=2; t<num_models; t++ ) */
/* 	{ */
/* 	  stg_id_t* parent = &model_ids[w][1]; */
	  
/* 	  stg_property_set( client, world_ids[w], model_ids[w][t],  */
/* 			    STG_PROP_PARENT, parent, sizeof(stg_id_t) ); */

/* 	  stg_velocity_t vel; */
/* 	  vel.x = 0.0; */
/* 	  vel.y = 0.0; */
/* 	  vel.a = drand48() * 6.28 - 3.14; */
	  
/* 	  stg_property_set( client, world_ids[w], model_ids[w][t],  */
/* 	  	    STG_PROP_VELOCITY, &vel, sizeof(vel) ); */
	  
/* 	  stg_pose_t pose; */
/* 	  pose.x = drand48() * 5.0; */
/* 	  pose.y = drand48() * 5.0; */
/* 	  pose.a = drand48() * M_PI; */
	  
/* 	  stg_property_set( client, world_ids[w], model_ids[w][t],  */
/* 			    STG_PROP_POSE, &pose, sizeof(pose) ); */
	  
/* 	  stg_size_t size; */
/* 	  //size.x = 0.1 + drand48() / 2.0; */
/* 	  //size.y = 0.1 + drand48() / 2.0; */
	  
/* 	  //stg_property_set( client, world_ids[w], model_ids[w][t],  */
/* 	  //	    STG_PROP_SIZE, &size, sizeof(size) ); */
	  
/* 	  stg_color_t col = random(); */
/* 	  stg_property_set( client, world_ids[w], model_ids[w][t],  */
/* 			    STG_PROP_COLOR, &col, sizeof(col) ); */
	  	  

/* 	  stg_bool_t nose = TRUE; */
/* 	  stg_property_set( client, world_ids[w], model_ids[w][t],  */
/* 			    STG_PROP_NOSE, &nose, sizeof(nose) ); */
	  
/* 	  stg_property_subscribe( client, world_ids[w], model_ids[w][t],  */
/* 				  STG_PROP_RANGERDATA, 0.1 ); */

/* 	}  */
/*     } */
  
/*   free( rects ); */
/*   rects = NULL; */
/*   rect_count = 0; */
  
/*   // this will bring us a timestamp as frequently as possible */
/*   stg_property_subscribe( client, world_ids[0], model_ids[0][0],  */
/* 			  STG_PROP_TIME, 0.0 ); */


/*   stg_property_subscribe( client, world_ids[0], model_ids[0][1],  */
/* 			  STG_PROP_LASERDATA, 0.1 ); */
  

  
/*  //stg_property_subscribe( client, world_ids[0], model_ids[0][0],  */
/*   //		  STG_PROP_POSE, 0.1 ); */

  
  while( !stg_quit_test() )
    {
      //putchar( '.' );

      //sleep(1);

      //stg_id_t type = random() % 5;

      //stg_property_set( client, world, model, type, &rect, sizeof(rect));

      //stg_property_unsubscribe( client, world, model, STG_MOD_POSE );


      stg_msg_t* msg = NULL;

      
      while( (msg = sc_read( client )) )
	{
	  //printf( "got a message!" );
	  //puts( "" );
	  
	  message_dispatch( client, msg );


	  free( msg );
	}
      
      // printf( "time: %.3f\n", world_time );

  /*     //if( c++ % 100 == 0 ) */
/*       if( 0 ) */
/* 	{ */
/* 	  stg_velocity_t vel; */
/* 	  vel.x = 0.0; */
/* 	  vel.y = 0.0; */
/* 	  vel.a = M_PI;//random() % 2 ? 1.0 : -1.0; */
	  
/* 	  stg_property_set( client, world, model,  */
/* 			    STG_PROP_VELOCITY, &vel, sizeof(vel) ); */


/* 	  stg_color_t col = random(); */
/* 	  stg_property_set( client, world, model,  */
/* 			    STG_PROP_COLOR, &col, sizeof(col) ); */
	  
/* 	} */


/*       //if( (c++ % 200) == 0  ) */
/*       //if( (c++) == 200  ) */

/*       if( 0 ) */
/* 	{ */
/* 	  stg_pose_t pose; */
/* 	  stg_property_set( client, world, model,  */
/* 			    STG_PROP_POSE, &pose, sizeof(pose)); */

/* 	  stg_property_unsubscribe( client, world, model, STG_PROP_POSE ); */

/* 	  puts( "killing things" ); */
/* 	  stg_model_kill( client, world, model ); */
/* 	  stg_world_kill( client, world ); */
/* 	} */

    }
  
  save( client, worldfile );

  sc_pull( client );
  puts( "done");

  puts( "destroyling client" );
  sc_destroy( client );

  puts( "quitting" );

  return 0; // done
}

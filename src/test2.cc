
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
  
  char* world_name = worldfile->ReadString(0, "name", "Player world" );

  double resolution = worldfile->ReadFloat(0, "resolution", 0.02 ); // 2cm default
  resolution = 1.0 / resolution; // invert res to ppm

  double interval_real = worldfile->ReadFloat(0, "interval_real", 0.1 );
  double interval_sim = worldfile->ReadFloat(0, "interval_sim", 0.1 );

  // create a single world
  sc_world_t* world = 
    sc_world_create( stg_token_create( world_name, STG_T_NUM, 99 ),
		     resolution, interval_sim, interval_real );
  
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


  stg_pose_t pose;
  pose.x = sz.x/2.0;
  pose.y = sz.y/2.0;
  pose.a = 0.0;
  sc_model_prop_with_data( root, STG_PROP_POSE, &pose, sizeof(pose) );

  stg_movemask_t mm = 0;
  sc_model_prop_with_data( root, STG_PROP_MOVEMASK, &mm, sizeof(mm) ); 
  
  const char* colorstr = worldfile->ReadString(0, "color", "black" );
  stg_color_t color = stg_lookup_color( colorstr );
  sc_model_prop_with_data( root, STG_PROP_COLOR, &color,sizeof(color));
  
  const char* bitmapfile = worldfile->ReadString(0, "bitmap", NULL );
  if( bitmapfile )
    {
      stg_rotrect_t* rects = NULL;
      int num_rects = 0;
      stg_load_image( bitmapfile, &rects, &num_rects );
      
      // convert rects to an array of lines
      int num_lines = 4 * num_rects;
      stg_line_t* lines = stg_rects_to_lines( rects, num_rects );
      stg_normalize_lines( lines, num_lines );
      stg_scale_lines( lines, num_lines, sz.x, sz.y );
      stg_translate_lines( lines, num_lines, -sz.x/2.0, -sz.y/2.0 );
      
      sc_model_prop_with_data( root, STG_PROP_LINES, 
			       lines, num_lines * sizeof(stg_line_t ));
      
      free( lines );
    }

  stg_bool_t boundary;
  boundary = worldfile->ReadInt(root, "boundary", 1 );
  sc_model_prop_with_data(root, STG_PROP_BOUNDARY, &boundary,sizeof(boundary));

  stg_bool_t grid;
  grid = worldfile->ReadInt( root, "grid", 1 );
  sc_model_prop_with_data( root, STG_PROP_GRID, &grid, sizeof(grid) );
    

  // Iterate through sections and create client-side models
  for (int section = 1; section < worldfile->GetEntityCount(); section++)
    {
      // a model has the macro name that defined it as it's name
      // unlesss a name is explicitly defined.  TODO - count instances
      // of macro names so we can autogenerate unique names
      const char *default_namestr = worldfile->GetEntityType(section);      
      char *namestr = worldfile->ReadString(section, "name", default_namestr );
      stg_token_t* token = stg_token_create( namestr, STG_T_NUM, 99 );
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

      stg_bool_t grid;
      grid = worldfile->ReadInt( section, "grid", 0 );
      sc_model_prop_with_data( mod, STG_PROP_GRID, &grid, sizeof(grid) );
      
      stg_bool_t boundary;
      boundary = worldfile->ReadInt( section, "boundary", 0 );
      sc_model_prop_with_data( mod, STG_PROP_BOUNDARY, &boundary,sizeof(boundary));
      
      stg_laser_config_t lconf;
      lconf.pose.x = worldfile->ReadTupleLength(section, "laser", 0, 0);
      lconf.pose.y = worldfile->ReadTupleLength(section, "laser", 1, 0);
      lconf.pose.a = worldfile->ReadTupleAngle(section, "laser", 2, 0);
      lconf.size.x = worldfile->ReadTupleLength(section, "laser", 3, 0);
      lconf.size.y = worldfile->ReadTupleLength(section, "laser", 4, 0);
      lconf.range_min = worldfile->ReadTupleLength(section, "laser", 5, 0);
      lconf.range_max = worldfile->ReadTupleLength(section, "laser", 6, 8);
      lconf.fov = worldfile->ReadTupleAngle(section, "laser", 7, 180);
      lconf.samples = worldfile->ReadTupleLength(section, "laser", 8, 180);
      
      sc_model_prop_with_data( mod, STG_PROP_LASERCONFIG, &lconf,sizeof(lconf));
  
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
	  
	  // convert rects to an array of lines
	  int num_lines = 4 * num_rects;
	  stg_line_t* lines = stg_rects_to_lines( rects, num_rects );
	  stg_normalize_lines( lines, num_lines );
	  stg_scale_lines( lines, num_lines, sz.x, sz.y );
	  stg_translate_lines( lines, num_lines, -sz.x/2.0, -sz.y/2.0 );
	  
	  sc_model_prop_with_data( mod, STG_PROP_LINES, 
				   lines, num_lines * sizeof(stg_line_t ));
	  	  
	  free( lines );
	  
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
      
      
      int linecount = worldfile->ReadInt( section, "line_count", 0 );
      if( linecount > 0 )
	{
	  char key[256];
	  stg_line_t* lines = (stg_line_t*)calloc( sizeof(stg_line_t), linecount );
	  int l;
	  for(l=0; l<linecount; l++ )
	    {
	      snprintf(key, sizeof(key), "line[%d]", l);

	      lines[l].x1 = worldfile->ReadTupleLength(section, key, 0, 0);
	      lines[l].y1 = worldfile->ReadTupleLength(section, key, 1, 0);
	      lines[l].x2 = worldfile->ReadTupleLength(section, key, 2, 0);
	      lines[l].y2 = worldfile->ReadTupleLength(section, key, 3, 0);	      
	    }
	  
	  printf( "NOTE: loaded line %d/%d (%.2f,%.2f - %.2f,%.2f)\n",
		  l, linecount, 
		  lines[l].x1, lines[l].y1, 
		  lines[l].x2, lines[l].y2 ); 
	  
	  sc_model_prop_with_data( mod, STG_PROP_LINES,
				   lines, linecount * sizeof(stg_line_t) );
	  
	  free( lines );
	}

      stg_velocity_t vel;
      vel.x = worldfile->ReadTupleLength(section, "velocity", 0, 0);
      vel.y = worldfile->ReadTupleLength(section, "velocity", 1, 0);
      vel.a = worldfile->ReadTupleAngle(section, "velocity", 2, 0);      
      sc_model_prop_with_data( mod, STG_PROP_VELOCITY, &vel, sizeof(vel) );


    }
  
  printf( "building client-side models done.\n" );

  puts( "connecting to server" );
  sc_connect( client, server_host, server_port );
  puts( "connection ok" );
  
  puts( "uploading worldfile to server" );
  sc_push( client );
  puts( "uploading done" );
  
  sc_model_t* mod = sc_get_model( client, world_name, "r0" );
  if( mod )
    {
      sc_model_subscribe( client, mod, STG_PROP_RANGERDATA, 0.1 );
      sc_model_subscribe( client, mod, STG_PROP_LASERDATA, 0.1 );
      sc_model_subscribe( client, mod, STG_PROP_POSE, 1.0 );
    }
  else
    PRINT_ERR( "no such model" );

  mod = sc_get_model( client, world_name, "r1" );
  if( mod )
    {
      sc_model_subscribe( client, mod, STG_PROP_RANGERDATA, 0.1 );
      sc_model_subscribe( client, mod, STG_PROP_LASERDATA, 0.1 );
      sc_model_subscribe( client, mod, STG_PROP_POSE, 1.0 );
    }
  else
    PRINT_ERR( "no such model" );

  mod = sc_get_model( client, world_name, "r2" );
  if( mod )
    {
      sc_model_subscribe( client, mod, STG_PROP_RANGERDATA, 0.1 );
      sc_model_subscribe( client, mod, STG_PROP_LASERDATA, 0.1 );
      sc_model_subscribe( client, mod, STG_PROP_POSE, 1.0 );
    }
  else
    PRINT_ERR( "no such model" );

  mod = sc_get_model( client, "Player world", "budgie" );
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

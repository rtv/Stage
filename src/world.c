
#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>
#include <string.h> // for strdup(3)

#define DEBUG 0


const double STG_DEFAULT_RESOLUTION = 0.02;  // 2cm pixels
const double STG_DEFAULT_INTERVAL_REAL = 100.0; // msec between updates
const double STG_DEFAULT_INTERVAL_SIM = 100.0;  // duration of a simulation timestep in msec
const double STG_DEFAULT_INTERVAL_GUI = 100.0; // msec between GUI updates
const double STG_DEFAULT_INTERVAL_MENU = 20.0; // msec between GUI updates
const double STG_DEFAULT_WORLD_WIDTH = 20.0; // meters
const double STG_DEFAULT_WORLD_HEIGHT = 20.0; // meters

static int init_occurred = 0;

#include "stage_internal.h"

extern int _stg_quit; // quit flag is returned by stg_world_update()
extern int _stg_disable_gui;


// HACK
stg_world_t* global_world = NULL;

/** @addtogroup stage
    @{ */

/** @defgroup world Worlds

Stage simulates a 'world' composed of `models', defined in a `world
file'. 

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
world
(
   name            "[filename of worldfile]"
   interval_real   100
   interval_sim    100
   gui_interval    100
   resolution      0.01
)
@endverbatim

@par Details
- name [string]
  - the name of the world, as displayed in the window title bar. Defaults to the worldfile file name.
- interval_sim [milliseconds]
  - the length of each simulation update cycle in milliseconds.
- interval_real [milliseconds]
  - the amount of real-world (wall-clock) time the siulator will attempt to spend on each simulation cycle.
- gui_interval [milliseconds]
  - the amount of real-world time between GUI updates
- resolution [meters]
  - specifies the resolution of the underlying bitmap model. Larger values speed up raytracing at the expense of fidelity in collision detection and sensing. 

@par More examples

The Stage source distribution contains several example world files in
<tt>(stage src)/worlds</tt> along with the worldfile properties
described on the manual page for each model type.

*/

/**@}*/

extern stg_type_record_t typetable[];


char* stg_world_clockstring( stg_world_t* world )
{
  char* clock = malloc(256);
#ifdef DEBUG
  snprintf( clock, 255, "Ticks %lu Time: %lu:%lu:%02lu:%02lu.%03lu (sim:%3d real:%3d ratio:%2.2f)\tsubs: %d  %s",
	    world->updates,
	    world->sim_time / (24*3600000), // days
	    world->sim_time / 3600000, // hours
	    (world->sim_time % 3600000) / 60000, // minutes
	    (world->sim_time % 60000) / 1000, // seconds
	    world->sim_time % 1000, // milliseconds
	    (int)world->sim_interval,
	    (int)world->real_interval_measured,
	    (double)world->sim_interval / (double)world->real_interval_measured,
	    world->subs,
	    world->paused ? "--PAUSED--" : "" );
#else

  snprintf( clock, 255, "Ticks %lu Time: %lu:%lu:%02lu:%02lu.%03lu\t(sim/real:%2.2f)\tsubs: %d  %s",
	    world->updates,
	    world->sim_time / (24*3600000), // days
	    world->sim_time / 3600000, // hours
	    (world->sim_time % 3600000) / 60000, // minutes
	    (world->sim_time % 60000) / 1000, // seconds
	    world->sim_time % 1000, // milliseconds
	    (double)world->sim_interval / (double)world->real_interval_measured,
	    world->subs,
	    world->paused ? "--PAUSED--" : "" );
#endif

  return clock;
}


void stg_world_set_interval_real( stg_world_t* world, unsigned int val )
{
  world->wall_interval = val;
}

void stg_world_set_interval_sim( stg_world_t* world, unsigned int val )
{
  world->sim_interval = val;
}

/* void model_clear_intersectors( gpointer key, stg_model_t* mod, gpointer user ) */
/* { */
/*   g_list_free( mod->intersectors ); */
/*   mod->intersectors = NULL; */
/* } */

/* void world_intercept_array_print( stg_world_t* world ) */
/* { */
/*   int i,j; */
/*  /\*  printf( "  " ); *\/ */
/* /\*   for( j=0; j<world->section_count; j++ ) *\/ */
/* /\*     printf( "%d", j); *\/ */
/* /\*   puts(""); *\/ */
  
/* /\*   for( i=0; i<world->section_count; i++ ) *\/ */
/* /\*     { *\/ */
/* /\*       printf( "%d ", i ); *\/ */
/* /\*       for( j=0; j<world->section_count; j++ ) *\/ */
/* /\* 	printf( "%d", world->intersections[i][j] ); *\/ */
/* /\*       puts(""); *\/ */
/* /\*     } *\/ */
/* /\*   puts(""); *\/ */

/*   printf( "  " ); */
/*   for( j=0; j<world->section_count; j++ ) */
/*     printf( "%d", j); */
/*   puts(""); */

/*   for( i=0; i<world->section_count; i++ ) */
/*     { */
/*       printf( "%d ", i ); */
/*       for( j=0; j<world->section_count; j++ ) */
/* 	printf( "%c", world->intersections[i * world->section_count + j] > 2 ? 'X' : ' ' ); */
/*       puts(""); */
/*     } */
/*   puts(""); */

/*   for( i=0; i<world->section_count; i++ ) */
/*     { */
/*       stg_model_t* mod = stg_world_get_model( world, i ); */

/*       if( mod ) */
/* 	{ */
/* 	  printf( "%s intersects with ", mod->token ); */
	  	 
/* 	  for( j=0; j<world->section_count; j++ ) */
/* 	    { */
/* 	      if( world->intersections[i* world->section_count + j ] > 2 ) */
/* 		{ */
/* 		  stg_model_t* hit =  stg_world_get_model( world, j ); */
/* 		  if( hit ) */
/* 		    printf( "%s ", hit->token ); */
/* 		  else */
/* 		    printf( "<bad pointer> " ); */
/* 		} */
/* 	    } */
/* 	  puts( "" ); */
/* 	} */
/*     } */
/* } */

/* void world_intersect_incr( stg_world_t* world,  */
/* 			   stg_model_t* a,  */
/* 			   stg_model_t* b ) */
/* { */
/*   int a_index = a->id * world->section_count + b->id; */
/*   int b_index = b->id * world->section_count + a->id; */
  
/*   world->intersections[ a_index ]++; */
/*   world->intersections[ b_index ]++; */
  
/*   if( world->intersections[ a_index ] == 3 ) */
/*     { */
/*       a->intersectors = g_list_prepend( a->intersectors, b ); */
/*       b->intersectors = g_list_prepend( b->intersectors, a ); */
/*     } */
/* // print_intercept_array( world ); */
/* } */

/* void world_intersect_decr( stg_world_t* world, */
/* 			   stg_model_t* a,  */
/* 			   stg_model_t* b ) */
/* { */
/*   int a_index = a->id * world->section_count + b->id; */
/*   int b_index = b->id * world->section_count + a->id; */
  
/*   world->intersections[ a_index ]--; */
/*   world->intersections[ b_index ]--; */
  
/*   if( world->intersections[ a_index ] == 2 ) */
/*     { */
/*       a->intersectors = g_list_remove( a->intersectors, b ); */
/*       b->intersectors = g_list_remove( b->intersectors, a ); */
/*     } */
  
/*   //print_intercept_array( world ); */
/* } */

/* static void compute_intersections( stg_endpoint_t* endpoint_list ) */
/* { */
/*   // compute the initial set of interections */
/*   GList* started = NULL; */
/*   stg_endpoint_t* it; */
/*   for( it=endpoint_list; it; it=it->next ) */
/*     { */
/*       // if this is the start of a model */
/*       switch( it->type ) */
/* 	{ */
/* 	case STG_BEGIN: */
/* 	  { */
/* 	    GList* it2; */
/* 	    for( it2=started; it2; it2=it2->next ) */
/* 	      world_intersect_incr( it->mod->world,  */
/* 				    it->mod,  (stg_model_t*)it2->data ); */
/* 	    started = g_list_prepend( started, it->mod ); */
/* 	  } */
/* 	  break; */
	  
/* 	case STG_END: */
/* 	  started = g_list_remove( started, it->mod ); */
/* 	  break; */
	  
/* 	default:  */
/* 	  PRINT_ERR1( "invalid endpoint type %d\n", it->type ); */
/* 	}     */
/*     }       */
/*   g_list_free( started ); */
/* } */


/* static void compute_intersections( GList* endpoint_list ) */
/* { */
/*   // compute the initial set of interections */
/*   GList* started = NULL; */
/*   GList* it; */
/*   for( it=endpoint_list; it; it=it->next ) */
/*     { */
/*       stg_endpoint_t* ep = (stg_endpoint_t*)it->data; */
	  
/*       // if this is the start of a model */
/*       switch( ep->type ) */
/* 	{ */
/* 	case STG_BEGIN: */
/* 	  { */
/* 	    GList* it2; */
/* 	    for( it2=started; it2; it2=it2->next ) */
/* 	      world_intersect_incr( ep->mod->world,  */
/* 				    ep->mod,  (stg_model_t*)it2->data ); */
/* 	    started = g_list_prepend( started, ep->mod ); */
/* 	  } */
/* 	  break; */
	  
/* 	case STG_END: */
/* 	  started = g_list_remove( started, ep->mod ); */
/* 	  break; */
	  
/* 	default:  */
/* 	  PRINT_ERR1( "invalid endpoint type %d\n", ep->type ); */
/* 	}     */
/*     }       */
/*   g_list_free( started ); */
/* } */

// create a world containing a passel of Stage models based on the
// worldfile
stg_world_t* stg_world_create_from_file( stg_id_t id, const char* worldfile_path )
{
  printf( " [Loading %s]", worldfile_path );      
  fflush(stdout);

  wf_load( (char*)worldfile_path );
  
  // end the output line of worldile components
  puts("");

  int section = 0;
  
  const char* world_name =
    wf_read_string( section, "name", (char*)worldfile_path );
  
  stg_msec_t interval_real = 
    wf_read_int( section, "interval_real", STG_DEFAULT_INTERVAL_REAL );

  stg_msec_t interval_sim = 
    wf_read_int( section, "interval_sim", STG_DEFAULT_INTERVAL_SIM );
      
  stg_msec_t interval_gui = 
    wf_read_int( section, "gui_interval", STG_DEFAULT_INTERVAL_GUI );

  double ppm = 
    1.0 / wf_read_float( section, "resolution", STG_DEFAULT_RESOLUTION ); 
  
  double width = 
    wf_read_tuple_float( section, "size", 0, STG_DEFAULT_WORLD_WIDTH ); 

  double height = 
    wf_read_tuple_float( section, "size", 1, STG_DEFAULT_WORLD_HEIGHT ); 
  
  
  //stg_msec_t gui_menu_interval = 
  //wf_read_int( section, "gui_menu_interval", STG_DEFAULT_INTERVAL_MENU );

  _stg_disable_gui = wf_read_int( section, "gui_disable", _stg_disable_gui );

  // create a single world
  stg_world_t* world = 
    stg_world_create( id, 
		      world_name, 
		      interval_sim, 
		      interval_real,
		      interval_gui,
		      ppm,
		      width,
		      height );

  if( world == NULL )
    return NULL; // failure
  
  int section_count = wf_section_count();
  
  // there can't be more models than sections, so we'll use the number
  // of sections to build an intersection test array
  //world->intersections = calloc( section_count*section_count, sizeof(unsigned short) );
  //world->intersections = calloc( 1024*1024, sizeof(unsigned short) );
  
  world->section_count = section_count;

  // Iterate through sections and create client-side models
  for( section = 1; section < section_count; section++ )
    {
      if( strcmp( wf_get_section_type(section), "window") == 0 )
	{
	  // configure the GUI
	  if( world->win )
	    gui_load( world, section ); 
	}
      else
	{
	  char *typestr = (char*)wf_get_section_type(section);      
	  
	  int parent_section = wf_get_parent_section( section );
	  
	  //PRINT_DEBUG2( "section %d parent section %d\n", 
	  //	section, parent_section );
	  
	  stg_model_t* parent = NULL;
	  
	  parent = (stg_model_t*)
	    g_hash_table_lookup( world->models, &parent_section );
	  
	  //PRINT_DEBUG3( "creating model from section %d parent section %d type %s\n",
	  //	section, parent_section, typestr );
	  
	  stg_model_t* mod = NULL;
	  stg_model_t* parent_mod = stg_world_get_model( world, parent_section );
	  
	  mod = stg_model_create( world, parent_mod, section, typestr );
	  assert( mod );
	  
	  // configure the model with properties from the world file
	  stg_model_load( mod );
	  
	  // add the new model to the world
	  stg_world_add_model( world, mod );
	}
    }

  // warn about unused WF linesa
  wf_warn_unused();


  // clear the intersection table (TODO - speed this up)
  //memset( world->intersections, 0, 
  //  world->section_count * world->section_count * sizeof(unsigned short));

  //compute_intersections( world->endpts.x );
  //compute_intersections( world->endpts.y );
  //compute_intersections( world->endpts.z );

  //puts( "FINISHED CREATING WORLD. ARRAY IS:" );
  //print_intercept_array( world );
  //print_endpoint_list( "Z lIST", world->endpts.z );

  global_world = world;

  return world;
}


stg_world_t* stg_world_create( stg_id_t id, 
			       const char* token, 
			       int sim_interval, 
			       int real_interval,
			       int gui_interval,
			       double ppm,
			       double width,
			       double height )
{
  if( ! init_occurred )
    {
      //puts( "STG_INIT" );
      stg_init( 0, NULL );
      init_occurred = 1;
    }

  stg_world_t* world = calloc( sizeof(stg_world_t),1 );
  
  world->id = id;
  world->token = strdup( token );
  world->models = g_hash_table_new_full( g_int_hash, g_int_equal,
					 NULL, stg_model_destroy );
  world->models_by_name = g_hash_table_new( g_str_hash, g_str_equal );
  world->sim_time = 0.0;
  world->sim_interval = sim_interval;
  world->wall_interval = real_interval;
  world->gui_interval = gui_interval;
  world->wall_last_update = 0;

  world->width = width;
  world->height = height;
  world->ppm = ppm; // this is the finest resolution of the matrix
  world->matrix = stg_matrix_create( ppm, width, height ); 
  
  world->endpts.x = NULL; 
  world->endpts.y = NULL;
  world->endpts.z = NULL; 
  
  world->paused = TRUE; // start paused.
  
  world->destroy = FALSE;
  
  if( _stg_disable_gui )
    world->win = NULL;
  else    
    world->win = gui_world_create( world );

  return world;
}

void stg_world_stop( stg_world_t* world )
{
  world->paused = TRUE;
}

void stg_world_start( stg_world_t* world )
{
  world->paused = FALSE;
}

void stg_world_set_title( stg_world_t* world, char* txt )
{
  gui_world_set_title( world, txt );
}

// calculate the bounding rectangle of everything in the world
void stg_world_dimensions( stg_world_t* world, 
			   double* min_x, double * min_y,
			   double* max_x, double * max_y )
{
  // TODO
  *min_x = *min_y =  MILLION;
  *max_x = *max_y = -MILLION;
}


void stg_world_destroy( stg_world_t* world )
{
  assert( world );
		   
  PRINT_DEBUG1( "destroying world %d", world->id );
  
  
  stg_matrix_destroy( world->matrix );

  free( world->token );
  g_hash_table_destroy( world->models );

  gui_world_destroy( world );

  free( world );
}

void world_destroy_cb( gpointer world )
{
  stg_world_destroy( (stg_world_t*)world );
}



int stg_world_update( stg_world_t* world, int sleepflag )
{
  //PRINT_WARN( "World update" );

  stg_msec_t timenow = stg_realtime_since_start();

#if DEBUG  
  double t = stg_realtime_since_start() / 1e3;

  static double calls = 0;
  calls++;  
  printf( "STG_WORLD_UPDATE: time %.2f calls: %.2f  %.2f/sec\n", t, calls, calls/t );
#endif

  
  //PRINT_DEBUG5( "timenow %lu last update %lu interval %lu diff %lu sim_time %lu", 
  //	timenow, world->wall_last_update, world->wall_interval,  
  //	timenow - world->wall_last_update, world->sim_time  );
  
  // if it's time for an update, update all the models
  stg_msec_t elapsed =  timenow - world->wall_last_update;
  
  if( (!world->paused) && (world->wall_interval <= elapsed) )
    {
      world->real_interval_measured = timenow - world->wall_last_update;
      world->updates++;

#if DEBUG     
      printf( " [%d %lu %f] sim:%lu real:%lu  ratio:%.2f freq:%.2f \n",
	      world->id, 
	      world->sim_time,
	      world->updates,
	      world->sim_interval,
	      world->real_interval_measured,
	      (double)world->sim_interval / (double)real_interval,
	      world->updates/t );
      
      fflush(stdout);
#endif
      
      GList* it;
      for( it=world->update_list; it; it=it->next )
	stg_model_update( (stg_model_t*)it->data );
      
      world->wall_last_update = timenow;	  
      world->sim_time += world->sim_interval;

      if( gui_world_update( world ) )
	stg_quit_request();
      
    }

/*   // is it time to redraw all the action? */
/*   if( world->win && world->gui_interval <= (timenow - world->gui_last_update) ) */
/*     { */
      
      
/*       world->gui_last_update = timenow; */
/*     } */
    

  //else
  //if( sleepflag )
  //  {
  ////puts( "sleeping" );
  //usleep( 10000 ); // sleep a little
  //  }

  return _stg_quit; // may have been set TRUE by the GUI or someone else
}

stg_model_t* stg_world_get_model( stg_world_t* world, stg_id_t mid )
{
  return( world ? g_hash_table_lookup( (gpointer)world->models, &mid ) : NULL );
}



void stg_world_add_model( stg_world_t* world, 
			  stg_model_t* mod  )
{
  //printf( "world added model %d %s\n", mod->id, mod->token );  
  g_hash_table_replace( world->models, &mod->id, mod );
  g_hash_table_replace( world->models_by_name, mod->token, mod );
  
  // if this is a top level model, it's a child of the world
  if( mod->parent == NULL )
    world->children = g_list_append( world->children, mod );  
}




/* // return a list of pointers to all the models who have bounds that */
/* // intersect the given bbox. */
/* GList* stg_world_models_in_bbox3d( stg_world_t* world, stg_bbox3d_t* bbox ) */
/* { */
/*   // TODO - this just does the X axis for now, and very inefficiently */
/*   // at that :) */

/*   /\* GList* x_intersectors = NULL; *\/ */
/* /\*   GList* it; *\/ */
/* /\*   for( it=world->endpts.x; it; it=it->next ) *\/ */
/* /\*     { *\/ */
/* /\*       stg_endpoint_t* ep = (stg_endpoint_t*)it->data; *\/ */

/* /\*       if( ep->value > bbox->x.min &&  *\/ */
/* /\* 	  ep->value < bbox->x.max &&  *\/ */
/* /\* 	  ep->mod *\/ */

/* /\* ) *\/ */
/* /\* 	intersectors = g_list_prepend( intersectors, ep->mod ); *\/ */
/* /\*     } *\/ */
  
/*   /\* GList* intersectors = NULL;  *\/ */
/* /\*   GList* it; *\/ */
/* /\*   for( it=world->children; it; it=it->next )  *\/ */
/* /\*     { *\/ */
/* /\*       stg_model_t* mod = (stg_model_t*)it->data; *\/ */

/* /\*       if( mod->endpts[0].value < bbox->x.max && *\/ */
/* /\* 	  mod->endpts[1].value > bbox->x.min &&  *\/ */
/* /\* 	  mod->endpts[2].value < bbox->y.max && *\/ */
/* /\* 	  mod->endpts[3].value > bbox->y.min &&  *\/ */
/* /\* 	  mod->endpts[4].value < bbox->z.max && *\/ */
/* /\* 	  mod->endpts[5].value > bbox->z.min ) *\/ */

/* /\* 	intersectors = g_list_prepend( intersectors, mod );  *\/ */
/* /\*     } *\/ */

/* /\*   return intersectors; *\/ */
/*   return NULL; */
/* } */

int stg_world_model_destroy( stg_world_t* world, stg_id_t model )
{
  // delete the model
  g_hash_table_remove( world->models, &model );
  
  return 0; // ok
}

void stg_world_print( stg_world_t* world )
{
  printf( " world %d:%s (%d models)\n", 
	  world->id, 
	  world->token,
	  g_hash_table_size( world->models ) );
  
   g_hash_table_foreach( world->models, model_print_cb, NULL );
}

void world_print_cb( gpointer key, gpointer value, gpointer user )
{
  stg_world_print( (stg_world_t*)value );
}

stg_model_t* stg_world_model_name_lookup( stg_world_t* world, const char* name )
{
  //printf( "looking up model name %s in models_by_name\n", name );
  return (stg_model_t*)g_hash_table_lookup( world->models_by_name, name );
}


void stg_model_save_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_save( (stg_model_t*)data );
}

void stg_model_reload_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_load( (stg_model_t*)data );
}

void stg_world_save( stg_world_t* world )
{
  // ask every model to save itself
  g_hash_table_foreach( world->models, stg_model_save_cb, NULL );
  
  if( world->win )
    gui_save( world );
  
  wf_save();
}

// reload the current worldfile
void stg_world_reload( stg_world_t* world )
{
  // can't reload the file yet - need to hack on the worldfile class. 
  //wf_load( NULL ); 

  // ask every model to load itself from the file database
  g_hash_table_foreach( world->models, stg_model_reload_cb, NULL );
}

void stg_world_start_updating_model( stg_world_t* world, stg_model_t* mod )
{
  if( g_list_find( world->update_list, mod ) == NULL )
    world->update_list = 
      g_list_append( world->update_list, mod ); 
}

void stg_world_stop_updating_model( stg_world_t* world, stg_model_t* mod )
{
  world->update_list = g_list_remove( world->update_list, mod ); 
}

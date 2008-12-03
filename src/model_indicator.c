///////////////////////////////////////////////////////////////////////////
//
// File: model_indicator.c
// Author: David Olsen
// Date: 4 October 2008
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_indicator.c,v $
//  $Author: rtv $
//  $Revision: 1.91 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>
#include "gui.h"

//#define DEBUG

#include "stage_internal.h"

#define STG_INDICATOR_WATTS 10
#define STG_DEFAULT_INDICATOR_SIZEX 0.15
#define STG_DEFAULT_INDICATOR_SIZEY 0.15

/**
@ingroup model
@defgroup model_indicator Indicator model
The indicator model simulates a visual indicator, such as a light.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
indicator
(
  # indicator properties

  # model properties
  size [0.15 0.15]
  color "blue"
  watts 10
)
@endverbatim

@par Details
*/

// Returns a copy of an array of polygons. There should really be a built in function for this.
stg_polygon_t* indicator_copy_polygons( stg_polygon_t* source, size_t count )
{
  stg_polygon_t* copy = stg_polygons_create( count );
  for ( int i = 0; i < count; i++ )
  {
	  g_array_append_vals( copy[i].points, source[i].points->data, source[i].points->len );
	  copy[i].unfilled = source[i].unfilled;
	  copy[i].color = source[i].color;
	  copy[i].bbox = source[i].bbox;
  }
  return copy;
}

void indicator_load( stg_model_t* mod )
{

  stg_indicator_config_t* cfg = (stg_indicator_config_t*)mod->cfg;

  // Take a copy of the polygon
  stg_indicator_data_t data;
  data.numPolys = mod->polygons_count;
  data.polys = indicator_copy_polygons( mod->polygons, data.numPolys );
  
  // Set the indicator to off initially
  data.on = FALSE;
  stg_model_set_polygons( mod, NULL, 0 );
  
  // Store the updated data (includes copy of polygon and original state)
  stg_model_set_data( mod, &data, sizeof(data) );

  model_change( mod, &mod->cfg ); // Notify model has been changed.
}

int indicator_update( stg_model_t* mod );
int indicator_startup( stg_model_t* mod );
int indicator_shutdown( stg_model_t* mod );

int indicator_render_data( stg_model_t* mod, void* userp );
int indicator_unrender_data( stg_model_t* mod, void* userp );
//int indicator_render_cfg( stg_model_t* mod, void* userp );
//int indicator_unrender_cfg( stg_model_t* mod, void* userp );


int indicator_init( stg_model_t* mod )
{
  // override the default methods
  mod->f_startup = indicator_startup;
  mod->f_shutdown = indicator_shutdown;
  mod->f_update = NULL; // indicator_update is installed startup, removed on shutdown
  mod->f_load = indicator_load;

  // sensible indicator defaults
  stg_geom_t geom;
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = STG_DEFAULT_INDICATOR_SIZEX;
  geom.size.y = STG_DEFAULT_INDICATOR_SIZEY;
  stg_model_set_geom( mod, &geom );


  // create a single rectangle body
  stg_polygon_t* square = stg_unit_polygon_create();
  stg_model_set_polygons( mod, square, 1 );


  // set up an indicator-specific config structure
  stg_indicator_config_t iconf;
  memset(&iconf,0,sizeof(iconf));
  stg_model_set_cfg( mod, &iconf, sizeof(iconf) );

  // sensible starting command
  stg_dio_cmd_t cmd; 
  cmd.count = 1;
  cmd.digout = 0;  
  stg_model_set_cmd( mod, &cmd, sizeof(cmd) ) ;
  
  // Null data till load
  stg_model_set_data( mod, NULL, 0 );

  // set default color
  stg_model_set_color( mod, 0xF0F0FF );

  // adds a menu item and associated on-and-off callbacks
  /*stg_model_add_property_toggles( mod,
				  &mod->data,
				  indicator_render_data, // called when toggled on
				  NULL,
				  indicator_unrender_data, // called when toggled off
				  NULL,
				  "indicatordata",
				  "indicator data",
				  TRUE );*/

  return 0;
}

void stg_indicator_config_print( stg_indicator_config_t* slc )
{
  printf( "indicator config" );
}

int indicator_update( stg_model_t* mod )
{
  PRINT_DEBUG2( "[%lu] indicator update (%d subs)",
		mod->world->sim_time, mod->subs );
  
  stg_dio_cmd_t* cmd = (stg_dio_cmd_t*)mod->cmd;
  stg_indicator_data_t* indData = stg_model_get_data(mod, 0);
  
  if (cmd && indData) // If we have all the appropriate data
  {
	  // If the command turns indicator on, and the previous state was off
	  if ( (cmd->digout & 1) && !indData->on )
	  {
		  indData->on = TRUE; // Set the state to on
		  // Stage is quite happy to delete any memory given to it, so make a copy.
		  stg_polygon_t* polysCopy = indicator_copy_polygons( indData->polys, indData->numPolys );
			  /*stg_polygons_create( indData->numPolys );
		  for ( int i = 0; i < indData->numPolys; i++ )
		  {
			  g_array_append_vals( polysCopy[i].points, indData->polys[i].points->data, indData->polys[i].points->len );
			  polysCopy[i].unfilled = indData->polys[i].unfilled;
			  polysCopy[i].color = indData->polys[i].color;
			  polysCopy[i].bbox = indData->polys[i].bbox;
		  }*/
		  stg_model_set_polygons( mod, polysCopy, indData->numPolys); // Show polygons
	  }
	  // If the command turns the indicator off, and the previous state was on
	  if ( !( cmd->digout & 1 ) && indData->on )
	  {
		  indData->on = FALSE; // Set the state to off
		  stg_model_set_polygons( mod, NULL, 0); // Hide polygons
	  }
  }

  return 0; //ok
}


int indicator_unrender_data( stg_model_t* mod, void* userp )
{
  // TODO: Unrender?
  //stg_model_fig_clear( mod, "indicator_data_fig" );

  return 1; // callback just runs one time
}


int indicator_render_data( stg_model_t* mod, void* enabled )
{
  // TODO: Render?

  return 0; // callback runs until removed
}

int indicator_startup( stg_model_t* mod )
{
  PRINT_DEBUG( "indicator startup" );

  // start consuming power
  stg_model_set_watts( mod, STG_INDICATOR_WATTS );

  // install the update function
  mod->f_update = indicator_update;

  return 0; // ok
}

int indicator_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "indicator shutdown" );

  // uninstall the update function
  mod->f_update = NULL;

  // stop consuming power
  stg_model_set_watts( mod, 0 );


  // clear the data - this will unrender it too
  stg_indicator_data_t* indData = stg_model_get_data(mod, 0);
  stg_polygons_destroy( indData->polys, indData->numPolys );
  stg_model_set_data( mod, NULL, 0 );

  return 0; // ok
}



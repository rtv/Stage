///////////////////////////////////////////////////////////////////////////
//
// File: model_guifeatures.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_guifeatures.c,v $
//  $Author: rtv $
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"
#include "gui.h"

int model_guifeatures_update( model_t* model );
int model_guifeatures_set( model_t* mod, void* data, size_t len );
void model_guifeatures_init( model_t* mod );
void gui_model_features( model_t* mod );



void model_guifeatures_register(void)
{ 
  PRINT_DEBUG( "GUIFEATURES INIT" );
  
  model_register_init( STG_PROP_GUIFEATURES, model_guifeatures_init );
  model_register_set( STG_PROP_GUIFEATURES, model_guifeatures_set );
}


void model_guifeatures_init( model_t* mod )
{
  // install a default guifeatures
  stg_guifeatures_t gf;
  gf.boundary =  STG_DEFAULT_BOUNDARY;
  gf.nose =  STG_DEFAULT_NOSE;
  gf.grid = STG_DEFAULT_GRID;
  gf.movemask = STG_DEFAULT_MOVEMASK;
  model_set_prop_generic( mod, STG_PROP_GUIFEATURES, &gf,  sizeof(gf) );
}


stg_guifeatures_t* model_guifeatures_get( model_t* mod )
{
  stg_guifeatures_t* guifeatures = model_get_prop_data_generic( mod, STG_PROP_GUIFEATURES );
  assert(guifeatures);
  return guifeatures;
}


int model_guifeatures_set( model_t* mod, void* data, size_t len )
{
  if( len != sizeof(stg_guifeatures_t) )
    {
      PRINT_WARN2( "received wrong size guifeatures (%d/%d)",
		   (int)len, (int)sizeof(stg_guifeatures_t) );
      return 1; // error
    }
  
  // store the guifeatures
  model_set_prop_generic( mod, STG_PROP_GUIFEATURES, data, len );
  
  // redraw the fancy features
  gui_model_features( mod );

  return 0;
}

// add a nose  indicating heading  
void gui_model_features( model_t* mod )
{
  stg_guifeatures_t* gf = model_guifeatures_get( mod );

  
  PRINT_DEBUG4( "model %d gui features grid %d nose %d boundary mask %d",
		(int)gf->grid, (int)gf->nose, (int)gf->boundary, (int)gf->movemask );

  
  gui_window_t* win = mod->world->win;

  if( gf->nose )
    { 
      rtk_fig_t* fig = gui_model_figs(mod)->top;      
      rtk_fig_color_rgb32( fig, model_color_get(mod) );
      
      stg_geom_t* geom = model_geom_get(mod);
      
      // draw a line from the center to the front of the model
      rtk_fig_line( fig, 
		    geom->pose.x, 
		    geom->pose.y, 
		    geom->size.x/2, 0 );
    }

  // we can only manipulate top-level figures
  //if( ent->parent == NULL )
  
  // figure gets the same movemask as the model, ONLY if it is a
  // top-level object
  if( mod->parent == NULL )
    rtk_fig_movemask( gui_model_figs(mod)->top, gf->movemask);  
  
  if( gf->movemask )
    // Set the mouse handler
    rtk_fig_add_mouse_handler( gui_model_figs(mod)->top, gui_model_mouse );


  
  if( gui_model_figs(mod)->grid )
    rtk_fig_destroy( gui_model_figs(mod)->grid );
  
  stg_geom_t* geom = model_geom_get(mod);

  if( gui_model_figs(mod)->grid == NULL )
    {
      gui_model_figs(mod)->grid = 
	rtk_fig_create( win->canvas, gui_model_figs(mod)->top, STG_LAYER_GRID );
      
	rtk_fig_color_rgb32( gui_model_figs(mod)->grid, stg_lookup_color(STG_GRID_MAJOR_COLOR ) );
    }
  
    if( gf->grid )
      rtk_fig_grid( gui_model_figs(mod)->grid, 0, 0, geom->size.x, geom->size.y, 1.0  ) ;
    
    
    if( gf->boundary )
      rtk_fig_rectangle( gui_model_figs(mod)->grid, 
			 geom->pose.x, geom->pose.y, geom->pose.a, 
			 geom->size.x, geom->size.y, 0 ); 
}


/* rtk_fig_t* gui_grid_create( rtk_canvas_t* canvas, rtk_fig_t* parent,  */
/* 			    double origin_x, double origin_y, double origin_a,  */
/* 			    double width, double height,  */
/* 			    double major, double minor ) */
/* { */
/*   rtk_fig_t* grid = NULL; */
  
/*   grid = rtk_fig_create( canvas, parent, STG_LAYER_GRID ); */
  
/*   if( minor > 0) */
/*     { */
/*       rtk_fig_color_rgb32( grid, stg_lookup_color(STG_GRID_MINOR_COLOR ) ); */
/*       rtk_fig_grid( grid, origin_x, origin_y, width, height, minor); */
/*     } */
/*   if( major > 0) */
/*     { */
/*       rtk_fig_color_rgb32( grid, stg_lookup_color(STG_GRID_MAJOR_COLOR ) ); */
/*       rtk_fig_grid( grid, origin_x, origin_y, width, height, major); */
/*     } */

/*   return grid; */
/* } */

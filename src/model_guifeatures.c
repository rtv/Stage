///////////////////////////////////////////////////////////////////////////
//
// File: model_guifeatures.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_guifeatures.c,v $
//  $Author: rtv $
//  $Revision: 1.5 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"
#include "gui.h"

void gui_model_features( model_t* mod );


stg_guifeatures_t* model_get_guifeatures( model_t* mod )
{
  return &mod->guifeatures;
}


int model_set_guifeatures( model_t* mod, stg_guifeatures_t* gf )
{
  memcpy( &mod->guifeatures, gf, sizeof(mod->guifeatures));
  
  // redraw the fancy features
  gui_model_features( mod );

  return 0;
}

// add a nose  indicating heading  
void gui_model_features( model_t* mod )
{
  stg_guifeatures_t* gf = model_get_guifeatures( mod );

  
  PRINT_DEBUG4( "model %d gui features grid %d nose %d boundary mask %d",
		(int)gf->grid, (int)gf->nose, (int)gf->boundary, (int)gf->movemask );

  
  gui_window_t* win = mod->world->win;

  if( gf->nose )
    { 
      rtk_fig_t* fig = gui_model_figs(mod)->top;      
      rtk_fig_color_rgb32( fig, model_get_color(mod) );
      
      stg_geom_t* geom = model_get_geom(mod);
      
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
  
  stg_geom_t* geom = model_get_geom(mod);

  if( gui_model_figs(mod)->grid == NULL )
    {
      gui_model_figs(mod)->grid = 
	rtk_fig_create( win->canvas, gui_model_figs(mod)->top, STG_LAYER_GRID );
      
	rtk_fig_color_rgb32( gui_model_figs(mod)->grid, stg_lookup_color(STG_GRID_MAJOR_COLOR ) );
    }
  
    if( gf->grid )
      rtk_fig_grid( gui_model_figs(mod)->grid, geom->pose.x, geom->pose.y, geom->size.x, geom->size.y, 1.0  ) ;
    
    
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

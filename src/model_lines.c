///////////////////////////////////////////////////////////////////////////
//
// File: model_lines.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_lines.c,v $
//  $Author: rtv $
//  $Revision: 1.3 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"
#include "gui.h"
#include <math.h>


stg_line_t* model_get_lines( model_t* mod, size_t* lines_count )
{
  
  *lines_count = mod->lines_count;
  return mod->lines;
}


int model_set_lines( model_t* mod, stg_line_t* lines, size_t lines_count )
{
  PRINT_DEBUG1( "received %d lines\n", lines_count );

  model_map( mod, 0 );
  
  stg_geom_t* geom = model_get_geom(mod); 
  
  // fit the rectangle inside the model's size
  stg_normalize_lines( lines, lines_count );
  stg_scale_lines( lines, lines_count, geom->size.x, geom->size.y );
  stg_translate_lines( lines, lines_count, -geom->size.x/2.0, -geom->size.y/2.0 );
    
  assert( mod->lines = realloc( mod->lines, sizeof(stg_line_t)*lines_count) );
  mod->lines_count = lines_count;
  
  // redraw my image 
  model_map( mod, 1 );

  model_lines_render( mod );
 
  return 0; // OK
}

void model_lines_render( model_t* mod )
{
  rtk_fig_t* fig = gui_model_figs(mod)->top;
  
  rtk_fig_clear( fig );

  rtk_fig_color_rgb32( fig, model_get_color(mod) );

  size_t count=0;
  stg_line_t* lines = model_get_lines(mod,&count);

  PRINT_DEBUG1( "rendering %d lines", count );

  stg_geom_t* geom = model_get_geom(mod);

  double localx = geom->pose.x;
  double localy = geom->pose.y;
  double locala = geom->pose.a;
  
  double cosla = cos(locala);
  double sinla = sin(locala);
  
  // draw lines too
  int l;
  for( l=0; l<count; l++ )
    {
      stg_line_t* line = &lines[l];
      
      double x1 = localx + line->x1 * cosla - line->y1 * sinla;
      double y1 = localy + line->x1 * sinla + line->y1 * cosla;
      double x2 = localx + line->x2 * cosla - line->y2 * sinla;
      double y2 = localy + line->x2 * sinla + line->y2 * cosla;
      
      rtk_fig_line( fig, x1,y1, x2,y2 );
    }
}


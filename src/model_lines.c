///////////////////////////////////////////////////////////////////////////
//
// File: model_lines.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_lines.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"
#include "gui.h"

void model_lines_init( model_t* mod );
int model_lines_set( model_t* mod, void*data, size_t len );

void model_lines_register(void)
{ 
  PRINT_DEBUG( "LINE INIT" );  
  model_register_init( STG_PROP_LINES, model_lines_init );
  model_register_set( STG_PROP_LINES, model_lines_set );
}

void model_lines_init( model_t* mod )
{
  // define a unit rectangle from 4 lines
  stg_line_t alines[4];
  alines[0].x1 = 0; 
  alines[0].y1 = 0;
  alines[0].x2 = 1; 
  alines[0].y2 = 0;
  
  alines[1].x1 = 1; 
  alines[1].y1 = 0;
  alines[1].x2 = 1; 
  alines[1].y2 = 1;
  
  alines[2].x1 = 1; 
  alines[2].y1 = 1;
  alines[2].x2 = 0; 
  alines[2].y2 = 1;
  
  alines[3].x1 = 0; 
  alines[3].y1 = 1;
  alines[3].x2 = 0; 
  alines[3].y2 = 0;
  
  stg_geom_t* geom = model_geom_get(mod); 
  
  // fit the rectangle inside the model's size
  stg_normalize_lines( alines, 4 );
  stg_scale_lines( alines, 4, geom->size.x, geom->size.y );
  stg_translate_lines( alines, 4, -geom->size.x/2.0, -geom->size.y/2.0 );
  
  model_set_prop_generic( mod, STG_PROP_LINES, alines,  sizeof(stg_line_t)*4 );
}


stg_line_t* model_lines_get( model_t* mod, size_t* count )
{
  stg_property_t* prop = model_get_prop_generic( mod, STG_PROP_LINES );
  assert(prop);
  
  stg_line_t* lines = prop->data;
  assert(lines);
  *count = prop->len / sizeof(stg_line_t);
  return lines;
}


int model_lines_set( model_t* mod, void*data, size_t len )
{
  int line_count = len / sizeof(stg_line_t);
  stg_line_t* lines = (stg_line_t*)data;
  
  PRINT_DEBUG1( "received %d lines\n", line_count );

  model_map( mod, 0 );
  
  stg_geom_t* geom = model_geom_get(mod); 
  
  // fit the rectangle inside the model's size
  stg_normalize_lines( lines, line_count );
  stg_scale_lines( lines, line_count, geom->size.x, geom->size.y );
  stg_translate_lines( lines, line_count, -geom->size.x/2.0, -geom->size.y/2.0 );
  
  model_set_prop_generic( mod, STG_PROP_LINES, lines, sizeof(stg_line_t)*line_count );
  
  // redraw my image 
  model_map( mod, 1 );

  model_lines_render( mod );
 
  return 0; // OK
}

void model_lines_render( model_t* mod )
{
  rtk_fig_t* fig = gui_model_figs(mod)->top;
  
  rtk_fig_clear( fig );

  rtk_fig_color_rgb32( fig, model_color_get(mod) );

  int count=0;
  stg_line_t* lines = model_lines_get(mod,&count);

  PRINT_DEBUG1( "rendering %d lines", count );

  stg_geom_t* geom = model_geom_get(mod);

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


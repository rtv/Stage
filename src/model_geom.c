//////////////////////////////////////////////////////////////////////////
//
// File: model_color.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_geom.c,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#define SHOW_GEOM 0

#include "model.h"
#include "gui.h"

stg_geom_t* model_get_geom( model_t* mod )
{
  return &mod->geom;
}


int model_set_geom( model_t* mod, stg_geom_t* geom )
{ 
  
  // if the new geom is different
  if( memcmp( &mod->geom, geom, sizeof(stg_geom_t) ))
    {
      // unrender from the matrix
      model_map( mod, 0 );
      
      // store the geom
      memcpy( &mod->geom, geom, sizeof(mod->geom) );
      
      size_t count=0;
      stg_line_t* lines = model_get_lines( mod, &count );
      
      // force the body lines to fit inside this new rectangle
      stg_normalize_lines( lines, count );
      stg_scale_lines(  lines, count, geom->size.x, geom->size.y );
      stg_translate_lines( lines, count, -geom->size.x/2.0, -geom->size.y/2.0 );
      
      // set the new lines (this will cause redraw)
      model_set_lines( mod, lines, count );

#if SHOW_GEOM
      gui_model_geom( mod );
#endif
     
      // re-render int the matrix
      model_map( mod, 1 );
    }

  return 0;
}

void gui_model_geom( model_t* mod )
{
  rtk_fig_t* fig = gui_model_figs(mod)->geom;
  rtk_fig_clear( fig );

  rtk_fig_color_rgb32( fig, 0 );
  
  stg_geom_t* geom = model_get_geom(mod);
  
  double localx = geom->pose.x;
  double localy = geom->pose.y;
  double locala = geom->pose.a;
  
  // draw the origin and the offset arrow
  double orgx = 0.05;
  double orgy = 0.03;
  rtk_fig_arrow_ex( fig, -orgx, 0, orgx, 0, 0.02 );
  rtk_fig_line( fig, 0,-orgy, 0, orgy );
  rtk_fig_line( fig, 0, 0, localx, localy );
  //rtk_fig_line( fig, localx-orgx, localy, localx+orgx, localy );
  rtk_fig_arrow( fig, localx, localy, locala, orgx, 0.02 );  
  rtk_fig_arrow( fig, localx, localy, locala-M_PI/2.0, orgy, 0.0 );
  rtk_fig_arrow( fig, localx, localy, locala+M_PI/2.0, orgy, 0.0 );
  rtk_fig_arrow( fig, localx, localy, locala+M_PI, orgy, 0.0 );
  //rtk_fig_arrow( fig, localx, localy, 0.0, orgx, 0.0 );  
}


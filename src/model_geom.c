///////////////////////////////////////////////////////////////////////////
//
// File: model_color.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_geom.c,v $
//  $Author: rtv $
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#define SHOW_GEOM 0

#include "model.h"
#include "gui.h"

int model_geom_update( model_t* model );
int model_geom_set( model_t* mod, void* data, size_t len );
void model_geom_init( model_t* mod );

void model_geom_register(void)
{ 
  PRINT_DEBUG( "GEOM INIT" );
  
  model_register_init( STG_PROP_GEOM, model_geom_init );
  model_register_set( STG_PROP_GEOM, model_geom_set );
}


void model_geom_init( model_t* mod )
{
  // install a default geom
  stg_geom_t ageom;
  ageom.pose.x = STG_DEFAULT_ORIGINX;
  ageom.pose.y = STG_DEFAULT_ORIGINY;
  ageom.pose.a = STG_DEFAULT_ORIGINA;
  ageom.size.x = STG_DEFAULT_SIZEX;
  ageom.size.y = STG_DEFAULT_SIZEY;
  model_set_prop_generic( mod, STG_PROP_GEOM, &ageom,  sizeof(ageom) );
}


stg_geom_t* model_geom_get( model_t* mod )
{
  stg_geom_t* geom = model_get_prop_data_generic( mod, STG_PROP_GEOM );
  assert(geom);
  return geom;
}


int model_geom_set( model_t* mod, void* data, size_t len )
{
  if( len != sizeof(stg_geom_t) )
    {
      PRINT_WARN2( "received wrong size pose (%d/%d)",
		   (int)len, (int)sizeof(stg_pose_t) );
      return 1; // error
    }
  
  
  stg_geom_t* geom = (stg_geom_t*)data;
  
  // store the geom
  model_set_prop_generic( mod, STG_PROP_GEOM, geom, len );

  size_t count=0;
  stg_line_t* lines = model_lines_get( mod, &count );
  
  // force the body lines to fit inside this new rectangle
  stg_normalize_lines( lines, count );
  stg_scale_lines(  lines, count, geom->size.x, geom->size.y );
  stg_translate_lines( lines, count, -geom->size.x/2.0, -geom->size.y/2.0 );
    
  // set the new lines (this will cause redraw)
  model_set_prop( mod, STG_PROP_LINES, lines, count*sizeof(stg_line_t));

#if SHOW_GEOM
  gui_model_geom( mod );
#endif

  return 0;
}

void gui_model_geom( model_t* mod )
{
  rtk_fig_t* fig = gui_model_figs(mod)->geom;
  rtk_fig_clear( fig );

  rtk_fig_color_rgb32( fig, 0 );
  
  stg_geom_t* geom = model_geom_get(mod);
  
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


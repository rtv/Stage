#include "model.h"
#include "gui.h"
#include <math.h>

void model_render_lines( model_t* mod )
{
  rtk_fig_t* fig = gui_model_figs(mod)->top;
  
  rtk_fig_clear( fig );

  rtk_fig_color_rgb32( fig, model_get_color(mod) );

  size_t count=0;
  stg_line_t* lines = model_get_lines(mod,&count);

  PRINT_DEBUG1( "rendering %d lines", (int)count );

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

void gui_render_geom( model_t* mod )
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

void gui_render_pose( model_t* mod )
{ 
  stg_pose_t* pose = model_get_pose( mod );
  //PRINT_DEBUG( "gui model pose" );
  rtk_fig_origin( gui_model_figs(mod)->top, 
		  pose->x, pose->y, pose->a );
}

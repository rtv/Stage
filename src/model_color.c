///////////////////////////////////////////////////////////////////////////
//
// File: model_color.c
// Author: Richard Vaughan
// Date: 20 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_color.c,v $
//  $Author: rtv $
//  $Revision: 1.3 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"


stg_color_t model_get_color( model_t* mod )
{
  return mod->color;
}


int model_set_color( model_t* mod, stg_color_t* col )
{
  memcpy( &mod->color, col, sizeof(mod->color) );

  // redraw my image
  model_lines_render(mod);

  return 0; // OK
}

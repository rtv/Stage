///////////////////////////////////////////////////////////////////////////
//
// File: model_color.c
// Author: Richard Vaughan
// Date: 20 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_color.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"

void model_color_init( model_t* mod );
int model_color_set( model_t* mod, void*data, size_t len );

void model_color_register(void)
{ 
  PRINT_DEBUG( "COLOR INIT" );  
  model_register_init( STG_PROP_COLOR, model_color_init );
  model_register_set( STG_PROP_COLOR, model_color_set );
}



void model_color_init( model_t* mod )
{
  // install a default color
  stg_color_t acolor = STG_DEFAULT_COLOR;
  model_set_prop_generic( mod, STG_PROP_COLOR, &acolor,  sizeof(acolor) );
}


stg_color_t model_color_get( model_t* mod )
{
  stg_color_t* color = model_get_prop_data_generic( mod, STG_PROP_COLOR );
  assert(color);
  return *color;
}


int model_color_set( model_t* mod, void*data, size_t len )
{
  if( len != sizeof(stg_color_t) )
    {
      PRINT_WARN2( "received wrong size color (%d/%d)",
		  len, sizeof(stg_color_t) );
      return 1; // error
    }
  
  // receive the new pose
  model_set_prop_generic( mod, STG_PROP_COLOR, data, len );
  
  PRINT_DEBUG2( "Color has changed for model %d. New color is %X", 
		mod->id, *(stg_color_t*)data );
  
  // redraw my image
  model_lines_render(mod);

  return 0; // OK
}

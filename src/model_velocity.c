///////////////////////////////////////////////////////////////////////////
//
// File: model_velocity.c
// Author: Richard Vaughan
// Date: 20 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_velocity.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"

void model_velocity_init( model_t* mod );

void model_velocity_register(void)
{ 
  PRINT_DEBUG( "VELOCITY INIT" );  
  model_register_init( STG_PROP_VELOCITY, model_velocity_init );
}

void model_velocity_init( model_t* mod )
{
  // install a default velocity
  stg_velocity_t avelocity;
  avelocity.x = 0;
  avelocity.y = 0;
  avelocity.a = 0;
  model_set_prop_generic( mod, STG_PROP_VELOCITY, &avelocity,  sizeof(avelocity) );
}


stg_velocity_t* model_velocity_get( model_t* mod )
{
  stg_velocity_t* velocity = model_get_prop_data_generic( mod, STG_PROP_VELOCITY );
  assert(velocity);
  return velocity;
}



///////////////////////////////////////////////////////////////////////////
//
// File: model_velocity.c
// Author: Richard Vaughan
// Date: 20 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_velocity.c,v $
//  $Author: rtv $
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"

stg_velocity_t* model_get_velocity( model_t* mod )
{
  return &mod->velocity;
}

int model_set_velocity( model_t* mod, stg_velocity_t* vel )
{
  memcpy( &mod->velocity, vel, sizeof(mod->velocity) );
  return 0;
}

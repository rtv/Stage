///////////////////////////////////////////////////////////////////////////
//
// File: model_mass.c
// Author: Richard Vaughan
// Date: 20 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_mass.c,v $
//  $Author: rtv $
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"

stg_kg_t* model_get_mass( model_t* mod )
{
  return &mod->mass;
}

int model_set_mass( model_t* mod, stg_kg_t* mass )
{
  memcpy( &mod->mass, mass, sizeof(mod->mass) );
  return 0;
}

///////////////////////////////////////////////////////////////////////////
//
// File: model_mass.c
// Author: Richard Vaughan
// Date: 20 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_mass.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include "model.h"

void model_mass_init( model_t* mod );
int model_mass_set( model_t* mod, void*data, size_t len );

void model_mass_register(void)
{ 
  PRINT_DEBUG( "MASS INIT" );  
  model_register_init( STG_PROP_MASS, model_mass_init );
  //model_register_set( STG_PROP_MASS, model_mass_set );
}


void model_mass_init( model_t* mod )
{
  // install a default mass
  stg_kg_t amass = STG_DEFAULT_MASS;
  model_set_prop_generic( mod, STG_PROP_MASS, &amass,  sizeof(amass) );
}


stg_kg_t* model_mass_get( model_t* mod )
{
  stg_kg_t* mass = model_get_prop_data_generic( mod, STG_PROP_MASS );
  assert(mass);
  return mass;
}


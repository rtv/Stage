///////////////////////////////////////////////////////////////////////////
//
// File: model_loadcell.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_loadcell.cc,v $
//  $Author: rtv $
//  $Revision: 1.7 $
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include "stage.hh"

using namespace Stg;

/**
@ingroup model
@defgroup model_loadcell Load cell model
The load cell model simulates a load sensor. It provides a single voltage on an aio interface which represents the mass of it's child models combined.
*/

ModelLoadCell::ModelLoadCell( World* world,
			      Model* parent )
  : Model( world, parent, MODEL_TYPE_LOADCELL )
{
}

ModelLoadCell::~ModelLoadCell()
{
}

float ModelLoadCell::GetVoltage()
{
  if (Parent()->Stalled()) // Assume parent is what moves
    return 0.0f; // Load has hit something
  else
    return float(this->GetTotalMass() - this->mass); // Don't include the mass of the loadcell itself
}

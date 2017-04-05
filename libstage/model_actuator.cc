///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source:
//  /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_position.cc,v
//  $
//  $Author: rtv $
//  $Revision$
//
///////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdlib.h>
#include <sys/time.h>

//#define DEBUG
#include "stage.hh"
#include "worldfile.hh"

using namespace Stg;

/**
@ingroup model
@defgroup model_actuator Actuator model

The actuator model simulates a simple actuator, where child objects can be
displaced in a
single dimension, or rotated about the Z axis.

API: Stg::ModelActuator

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
actuator
(
  # actuator properties
  type ""
  axis [x y z]

  min_position
  max_position
  max_speed
)
@endverbatim

@par Details

- type "linear" or "rotational"\n
  the style of actuator. Rotational actuators may only rotate about the z axis
(i.e. 'yaw')
- axis
  if a linear actuator the axis that the actuator will move along
 */

// static const double WATTS_KGMS = 5.0; // cost per kg per meter per second
static const double WATTS_BASE = 2.0; // base cost of position device

ModelActuator::ModelActuator(World *world, Model *parent, const std::string &type)
    : Model(world, parent, type), goal(0), pos(0), max_speed(1), min_position(0), max_position(1),
      start_position(0), cosa(0.0), sina(0.0), control_mode(CONTROL_VELOCITY),
      actuator_type(TYPE_LINEAR), axis(0, 0, 0)
{
  this->SetWatts(WATTS_BASE);

  // sensible position defaults
  // this->SetVelocity( Velocity(0,0,0,0) );
  this->SetBlobReturn(true);

  // Allow the models to move
  // VelocityEnable();
}

ModelActuator::~ModelActuator(void)
{
  // nothing to do
}

void ModelActuator::Load(void)
{
  Model::Load();

  // load steering mode
  if (wf->PropertyExists(wf_entity, "type")) {
    const std::string &type_str = wf->ReadString(wf_entity, "type", "linear");

    if (type_str == "linear")
      actuator_type = TYPE_LINEAR;
    else if (type_str == "rotational")
      actuator_type = TYPE_ROTATIONAL;
    else {
      PRINT_ERR1("invalid actuator type specified: \"%s\" - should be one of: "
                 "\"linear\" or \"rotational\". Using \"linear\" as default.",
                 type_str.c_str());
    }
  }

  if (actuator_type == TYPE_LINEAR) {
    // if we are a linear actuator find the axis we operate in
    if (wf->PropertyExists(wf_entity, "axis")) {
      wf->ReadTuple(wf_entity, "axis", 0, 3, "fff", &axis.x, &axis.y, &axis.z);

      // normalise the axis
      double length = sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
      if (length == 0) {
        PRINT_ERR("zero length vector specified for actuator axis, using "
                  "(1,0,0) instead");
        axis.x = 1;
      } else {
        axis.x /= length;
        axis.y /= length;
        axis.z /= length;
      }
    }
  }

  if (wf->PropertyExists(wf_entity, "max_speed")) {
    max_speed = wf->ReadFloat(wf_entity, "max_speed", 1);
  }

  if (wf->PropertyExists(wf_entity, "max_position")) {
    max_position = wf->ReadFloat(wf_entity, "max_position", 1);
  }

  if (wf->PropertyExists(wf_entity, "min_position")) {
    min_position = wf->ReadFloat(wf_entity, "min_position", 0);
  }

  if (wf->PropertyExists(wf_entity, "start_position")) {
    start_position = wf->ReadFloat(wf_entity, "start_position", 0);

    Pose DesiredPose = InitialPose;

    cosa = cos(InitialPose.a);
    sina = sin(InitialPose.a);

    switch (actuator_type) {
    case TYPE_LINEAR: {
      double cosa = cos(DesiredPose.a);
      double sina = sin(DesiredPose.a);

      DesiredPose.x += (axis.x * cosa - axis.y * sina) * start_position;
      DesiredPose.y += (axis.x * sina + axis.y * cosa) * start_position;
      DesiredPose.z += axis.z * start_position;
      SetPose(DesiredPose);
    } break;

    case TYPE_ROTATIONAL: {
      DesiredPose.a += start_position;
      SetPose(DesiredPose);
    } break;
    default: PRINT_ERR1("unrecognized actuator type %d", actuator_type);
    }
  }
}

void ModelActuator::Update(void)
{
  PRINT_DEBUG1("[%d] actuator update", 0);

  // update current position
  Pose CurrentPose = GetPose();
  // just need to get magnitude difference;
  Pose PoseDiff(CurrentPose.x - InitialPose.x, CurrentPose.y - InitialPose.y,
                CurrentPose.z - InitialPose.z, CurrentPose.a - InitialPose.a);

  switch (actuator_type) {
  case TYPE_LINEAR: {
    // When the velocity is applied, it will automatically be rotated by the
    // angle the model is at
    // So, rotate the axis of movement by the model angle before doing a dot
    // product to find the actuator position
    pos = (PoseDiff.x * cosa - PoseDiff.y * sina) * axis.x
          + (PoseDiff.x * sina + PoseDiff.y * cosa) * axis.y + PoseDiff.z * axis.z;
  } break;
  case TYPE_ROTATIONAL: {
    pos = PoseDiff.a;
  } break;
  default: PRINT_ERR1("unrecognized actuator type %d", actuator_type);
  }

  if (this->subs) // no driving if noone is subscribed
  {
    // stop by default
    double velocity = 0;
    switch (control_mode) {
    case CONTROL_VELOCITY: {
      PRINT_DEBUG("actuator velocity control mode");
      PRINT_DEBUG2("model %s command(%.2f)", this->token.c_str(), this->goal);
      if ((pos <= min_position && goal < 0) || (pos >= max_position && goal > 0))
        velocity = 0;
      else
        velocity = goal;
    } break;

    case CONTROL_POSITION: {
      PRINT_DEBUG("actuator position control mode");

      // limit our axis
      if (goal < min_position)
        goal = min_position;
      else if (goal > max_position)
        goal = max_position;

      double error = goal - pos;
      velocity = error;

      PRINT_DEBUG1("error: %.2f\n", error);

    } break;

    default: PRINT_ERR1("unrecognized actuator command mode %d", control_mode);
    }

    // simple model of power consumption
    // TODO power consumption

    // speed limits for controller
    if (velocity < -max_speed)
      velocity = -max_speed;
    else if (velocity > max_speed)
      velocity = max_speed;

    Velocity outvel;
    switch (actuator_type) {
    case TYPE_LINEAR: {
      outvel.x = axis.x * velocity;
      outvel.y = axis.y * velocity;
      outvel.z = axis.z * velocity;
      outvel.a = 0;
    } break;
    case TYPE_ROTATIONAL: {
      outvel.x = outvel.y = outvel.z = 0;
      outvel.a = velocity;
    } break;
    default: PRINT_ERR1("unrecognized actuator type %d", actuator_type);
    }

    // TODO - deal with velocity
    // this->SetVelocity( outvel );
    // this->GPoseDirtyTree();
    PRINT_DEBUG4(" Set Velocity: [ %.4f %.4f %.4f %.4f ]\n", outvel.x, outvel.y, outvel.z,
                 outvel.a);
  }

  Model::Update();
}

void ModelActuator::Startup(void)
{
  Model::Startup();

  PRINT_DEBUG("position startup");
}

void ModelActuator::Shutdown(void)
{
  PRINT_DEBUG("position shutdown");

  // safety features!
  goal = 0;

  //	velocity.Zero();

  Model::Shutdown();
}

void ModelActuator::SetSpeed(double speed)
{
  control_mode = CONTROL_VELOCITY;
  goal = speed;
}

void ModelActuator::GoTo(double pos)
{
  control_mode = CONTROL_POSITION;
  goal = pos;
}

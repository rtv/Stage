///////////////////////////////////////////////////////////////////////////
//
// File: model_laser.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_position.cc,v $
//  $Author: rtv $
//  $Revision: 1.5 $
//
///////////////////////////////////////////////////////////////////////////


#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

//#define DEBUG
#include "stage.hh"
#include "worldfile.hh"

using namespace Stg;

/**
@ingroup model
@defgroup model_actuator Actuator model

The actuator model simulates a simple actuator, where child objects can be displaced in a
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
  the style of actuator. Rotational actuators may only rotate about the z axis (i.e. 'yaw')
- axis
  if a linear actuator the axis that the actuator will move along
 */
  
static const double STG_ACTUATOR_WATTS_KGMS = 5.0; // cost per kg per meter per second
static const double STG_ACTUATOR_WATTS = 2.0; // base cost of position device

ModelActuator::ModelActuator( World* world,
										Model* parent )
  : Model( world, parent, MODEL_TYPE_ACTUATOR ),
	 goal(0), 
	 pos(0), 
	 max_speed(1), 
	 min_position(0), 
	 max_position(1),
	 control_mode( STG_ACTUATOR_CONTROL_VELOCITY ),
	 actuator_type( STG_ACTUATOR_TYPE_LINEAR ),
	 axis(0,0,0)
{
  // no power consumed until we're subscribed
  this->SetWatts( 0 );
  
  // sensible position defaults
  this->SetVelocity( Velocity(0,0,0,0) );
  this->SetBlobReturn( TRUE );  
}


ModelActuator::~ModelActuator( void )
{
	// nothing to do
}

void ModelActuator::Load( void )
{
	Model::Load();
	InitialPose = GetPose();

	// load steering mode
	if( wf->PropertyExists( wf_entity, "type" ) )
	{
		const char* type_str =
			wf->ReadString( wf_entity, "type", NULL );

		if( type_str )
		{
			if( strcmp( type_str, "linear" ) == 0 )
				actuator_type = STG_ACTUATOR_TYPE_LINEAR;
			else if( strcmp( type_str, "rotational" ) == 0 )
				actuator_type = STG_ACTUATOR_TYPE_ROTATIONAL;
			else
			{
				PRINT_ERR1( "invalid actuator type specified: \"%s\" - should be one of: \"linear\" or \"rotational\". Using \"linear\" as default.", type_str );
			}
		}
	}

	if (actuator_type == STG_ACTUATOR_TYPE_LINEAR)
	{
		// if we are a linear actuator find the axis we operate in
		if( wf->PropertyExists( wf_entity, "axis" ) )
		{
			axis.x = wf->ReadTupleLength( wf_entity, "axis", 0, 0 );
			axis.y = wf->ReadTupleLength( wf_entity, "axis", 1, 0 );
			axis.z = wf->ReadTupleLength( wf_entity, "axis", 2, 0 );

			// normalise the axis
			double length = sqrt(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);
			if (length == 0)
			{
				PRINT_ERR( "zero length vector specified for actuator axis, using (1,0,0) instead" );
				axis.x = 1;
			}
			else
			{
				axis.x /= length;
				axis.y /= length;
				axis.z /= length;
			}
		}
	}

	if( wf->PropertyExists( wf_entity, "max_speed" ) )
	{
		max_speed = wf->ReadFloat ( wf_entity, "max_speed", 1 );
	}

	if( wf->PropertyExists( wf_entity, "max_position" ) )
	{
		max_position = wf->ReadFloat( wf_entity, "max_position", 1 );
	}

	if( wf->PropertyExists( wf_entity, "min_position" ) )
	{
		min_position = wf->ReadFloat( wf_entity, "min_position", 0 );
	}

}

void ModelActuator::Update( void  )
{
	PRINT_DEBUG1( "[%lu] actuator update", 0 );

	// stop by default
	double velocity = 0;

	// update current position
	Pose CurrentPose = GetPose();
	// just need to get magnitude difference;
	Pose PoseDiff( CurrentPose.x - InitialPose.x,
						CurrentPose.y - InitialPose.y,
						CurrentPose.z - InitialPose.z,
						CurrentPose.a - InitialPose.a );

	switch (actuator_type)
	{
		case STG_ACTUATOR_TYPE_LINEAR:
		{
			pos = PoseDiff.x * axis.x + PoseDiff.y * axis.y + PoseDiff.z * axis.z; // Dot product to find distance along axis
		} break;
		case STG_ACTUATOR_TYPE_ROTATIONAL:
		{
			pos = PoseDiff.a;
		} break;
		default:
			PRINT_ERR1( "unrecognized actuator type %d", actuator_type );
	}


	if( this->subs )   // no driving if noone is subscribed
	{
		switch( control_mode )
		{
			case STG_ACTUATOR_CONTROL_VELOCITY :
				{
					PRINT_DEBUG( "actuator velocity control mode" );
					PRINT_DEBUG2( "model %s command(%.2f)",
							this->token,
							this->goal);
					if ((pos <= min_position && goal < 0) || (pos >= max_position && goal > 0))
						velocity = 0;
					else
						velocity = goal;
				} break;

			case STG_ACTUATOR_CONTROL_POSITION:
				{
					PRINT_DEBUG( "actuator position control mode" );


					// limit our axis
					if (goal < min_position)
						goal = min_position;
					else if (goal > max_position)
						goal = max_position;

					double error = goal - pos;
					velocity = error;

					PRINT_DEBUG1( "error: %.2f\n", error);

				}
				break;

			default:
				PRINT_ERR1( "unrecognized actuator command mode %d", control_mode );
		}

		// simple model of power consumption
		//TODO power consumption

		// speed limits for controller
		if (velocity < -max_speed)
			velocity = -max_speed;
		else if (velocity > max_speed)
			velocity = max_speed;

		Velocity outvel;
		switch (actuator_type)
		{
			case STG_ACTUATOR_TYPE_LINEAR:
			{
				outvel.x = axis.x * velocity;
				outvel.y = axis.y * velocity;
				outvel.z = axis.z * velocity;
				outvel.a = 0;
			} break;
			case STG_ACTUATOR_TYPE_ROTATIONAL:
			{
				outvel.x = outvel.y = outvel.z = 0;
				outvel.a = velocity;
			}break;
			default:
				PRINT_ERR1( "unrecognized actuator type %d", actuator_type );
		}

		this->SetVelocity( outvel );
		//this->GPoseDirtyTree();
		PRINT_DEBUG4( " Set Velocity: [ %.4f %.4f %.4f %.4f ]\n",
				outvel.x, outvel.y, outvel.z, outvel.a );
	}

	Model::Update();
}

void ModelActuator::Startup( void )
{
	Model::Startup();

	PRINT_DEBUG( "position startup" );

	this->SetWatts( STG_ACTUATOR_WATTS );

}

void ModelActuator::Shutdown( void )
{
	PRINT_DEBUG( "position shutdown" );

	// safety features!
	goal = 0;

	velocity.Zero();

	this->SetWatts( 0 );

	Model::Shutdown();
}

void ModelActuator::SetSpeed( double speed)
{
	control_mode = STG_ACTUATOR_CONTROL_VELOCITY;
	goal = speed;
}


void ModelActuator::GoTo( double pos)
{
	control_mode = STG_ACTUATOR_CONTROL_POSITION;
	goal = pos;
}

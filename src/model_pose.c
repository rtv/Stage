///////////////////////////////////////////////////////////////////////////
//
// File: model_pose.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_pose.c,v $
//  $Author: rtv $
//  $Revision: 1.10 $
//
///////////////////////////////////////////////////////////////////////////

#include <math.h>

//#define DEBUG

#include "stage.h"
#include "raytrace.h"
#include "gui.h"

int model_pose_update( model_t* model );
int model_pose_set( model_t* mod, void* data, size_t len );
void model_pose_init( model_t* mod );
void model_obstacle_return_init( model_t* mod );

void model_pose_register(void)
{ 
  PRINT_DEBUG( "POSE INIT" );
  
  model_register_init( STG_PROP_OBSTACLERETURN, model_obstacle_return_init );

  model_register_init( STG_PROP_POSE, model_pose_init );
  model_register_update( STG_PROP_POSE, model_pose_update );
  model_register_set( STG_PROP_POSE, model_pose_set );
}


void model_pose_init( model_t* mod )
{
  // install a default pose
  stg_pose_t newpose;
  newpose.x = 0;
  newpose.y = 0;
  newpose.a = 0; 
  model_set_prop_generic( mod, STG_PROP_POSE, &newpose, sizeof(newpose) );
}

void model_obstacle_return_init( model_t* mod )
{
  stg_bool_t val = TRUE;
  model_set_prop_generic( mod, STG_PROP_OBSTACLERETURN, &val, sizeof(val) );
}

// convencience
stg_bool_t model_obstacle_get( model_t* model )
{
  stg_bool_t* obs = model_get_prop_data_generic( model, STG_PROP_OBSTACLERETURN );
  assert(obs);
  return *obs;
}

int model_pose_set( model_t* mod, void*data, size_t len )
{
  if( len != sizeof(stg_pose_t) )
    {
      PRINT_WARN2( "received wrong size pose (%d/%d)",
		  len, sizeof(stg_pose_t) );
      return 1; // error
    }
  
  // if the new pose is different
  stg_pose_t* current_pose  = model_pose_get( mod );
  
  if( memcmp( current_pose, data, sizeof(stg_pose_t) ))
    {
      model_map( mod, 0 );
      
      // receive the new pose
      model_set_prop_generic( mod, STG_PROP_POSE, data, len );
      
      model_map( mod, 1 );
      
    // move the rtk figure to match
      gui_model_pose( mod );
    }
  
  return 0; // OK
}



///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision with obstacles.
// Returns a pointer to the first entity we are in collision with, and stores
// the location of the hit in hitx,hity (if non-null)
// Returns NULL if not collisions.
// This function is useful for writing position devices.
model_t* model_test_collision( model_t* mod, double* hitx, double* hity )
{
  //return NULL;
  
  // raytrace along all our rectangles. expensive, but most vehicles
  // will just be a single rect, grippers 3 rects, etc. not too bad.
  
  int count=0;
  stg_line_t* lines = model_lines_get(mod, &count);

  // no body? no collision
  if( count < 1 )
    return NULL;

  int l;
  for( l=0; l<count; l++ )
    {
      // find the global coords of this line
      stg_line_t* line = &lines[l];

      stg_pose_t p1;
      p1.x = line->x1;
      p1.y = line->y1;
      p1.a = 0;

      stg_pose_t p2;
      p2.x = line->x2;
      p2.y = line->y2;
      p2.a = 0;

      model_local_to_global( mod, &p1 );
      model_local_to_global( mod, &p2 );

      itl_t* itl = itl_create( p1.x, p1.y, p2.x, p2.y, 
			       mod->world->matrix, 
			       PointToPoint );

      model_t* hitmod;
      while( (hitmod = itl_next( itl )) ) 
	{
	  if( hitmod != mod && model_obstacle_get(hitmod) ) //&& !IsDescendent(ent) &&
	    //if( ent != this && ent->obstacle_return )
	    {
	      if( hitx || hity ) // if the caller needs to know hit points
		{
		  if( hitx ) *hitx = itl->x; // report them
		  if( hity ) *hity = itl->y;

		}
	      return hitmod; // we hit this object! stop raytracing
	    }
	}

      itl_destroy( itl );
    }
  return NULL;  // done 
}


// convencience - get the model's pose.
stg_pose_t* model_pose_get( model_t* model )
{
  stg_pose_t* pose = model_get_prop_data_generic( model, STG_PROP_POSE );
  assert(pose);
  return pose;
}

int model_pose_update( model_t* model )
{ 
  PRINT_DEBUG1( "pose update method model %d", model->id );
 
  stg_velocity_t* vel = model_velocity_get(model);  
  stg_pose_t* pose = model_pose_get( model );
  
  if( vel->x || vel->y || vel->a )
  {
    stg_pose_t oldpose;
    memcpy( &oldpose, pose, sizeof(oldpose) );
    
    double interval = (double)model->world->sim_interval / 1000.0;

    // global mode
    //pose.x += vel->x * interval;
    //pose.y += vel->y * interval;
    //pose.a += vel->a * interval;

    // local mode
    pose->x += interval * (vel->x * cos(pose->a) - vel->y * sin(pose->a));
    pose->y += interval * (vel->x * sin(pose->a) + vel->y * cos(pose->a));
    pose->a += interval * vel->a;
    
    stg_pose_t newpose; // store the new pose
    memcpy( &newpose, pose, sizeof(newpose) );
    
    if( //model->energy.joules > 0 && 
	model_test_collision( model, NULL, NULL ) == NULL )
      {
	// reset the old pose so that unmapping works properly
	memcpy( pose, &oldpose, sizeof(oldpose) );
	
	// now set the new pose handling matrix & gui redrawing 
	model_set_prop( model, STG_PROP_POSE, &newpose, sizeof(newpose) );

	
	// ignore acceleration in energy model for now, we just pay
	// something to move.	
	stg_kg_t mass = *model_mass_get( model );
	model_energy_consume( model, STG_ENERGY_COST_MOTIONKG * mass );

      }
    else // reset the old pose
      memcpy( pose, &oldpose, sizeof(oldpose) );
  }
}

void gui_model_pose( model_t* mod )
{ 
  stg_pose_t* pose = model_pose_get( mod );
  //PRINT_DEBUG( "gui model pose" );
  rtk_fig_origin( gui_model_figs(mod)->top, 
		  pose->x, pose->y, pose->a );
}


#include <math.h>

#include "stage.h"
#include "raytrace.h"

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
  
  // no body? no collision
  if( mod->lines->len < 1 )
    return NULL;

  int l;
  for( l=0; l<(int)mod->lines->len; l++ )
    {
      // find the global coords of this line
      stg_line_t* line = &g_array_index( mod->lines, stg_line_t, l );

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
	  if( hitmod != mod && hitmod->obstacle_return ) //&& !IsDescendent(ent) &&
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


void model_update_pose( model_t* model )
{  
  stg_velocity_t* vel = &model->velocity;
  stg_pose_t* pose = &model->pose;
  stg_energy_t* en = &model->energy;
  
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
    
    if( model->energy.joules > 0 && 
	model_test_collision( model, NULL, NULL ) == NULL )
      {
	// reset the old pose so that unmapping works properly
	memcpy( pose, &oldpose, sizeof(oldpose) );
	
	// now set the new pose handling matrix & gui redrawing 
	model_set_prop( model, STG_PROP_POSE, &newpose, sizeof(newpose) );


	// ignore acceleration in energy model for now, we just pay
	// something to move.
	en->joules -= en->move_cost;
	if( en->joules < 0 ) en->joules = 0;
      }
    else // reset the old pose
      memcpy( pose, &oldpose, sizeof(oldpose) );
  }
}


///////////////////////////////////////////////////////////////////////////
//
// File: model_pose.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_pose.c,v $
//  $Author: rtv $
//  $Revision: 1.18 $
//
///////////////////////////////////////////////////////////////////////////

#include <math.h>

//#define DEBUG

#include "raytrace.h"
#include "gui.h"
#include "model.h"


// convencience
stg_bool_t model_get_obstaclereturn( model_t* mod   )
{
  return mod->obstacle_return;
}

int model_set_pose( model_t* mod, stg_pose_t* pose )
{
  // if the new pose is different
  if( memcmp( &mod->pose, pose, sizeof(stg_pose_t) ))
    {
      // unrender from the matrix
      model_map_with_children( mod, 0 );
      
      // copy the new pose
      memcpy( &mod->pose, pose, sizeof(mod->pose) );
      
      // render in the matrix
      model_map_with_children( mod, 1 );
      
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
model_t* model_test_collision_at_pose( model_t* mod, 
				       stg_pose_t* pose, 
				       double* hitx, double* hity )
{
  //return NULL;
  
  // raytrace along all our rectangles. expensive, but most vehicles
  // will just be a single rect, grippers 3 rects, etc. not too bad.
  
  size_t count=0;
  stg_line_t* lines = model_get_lines(mod, &count);

  // no body? no collision
  if( count < 1 )
    return NULL;

  int l;
  for( l=0; l<count; l++ )
    {
      // find the global coords of this line
      stg_line_t* line = &lines[l];

      stg_pose_t pp1;
      pp1.x = line->x1;
      pp1.y = line->y1;
      pp1.a = 0;

      stg_pose_t pp2;
      pp2.x = line->x2;
      pp2.y = line->y2;
      pp2.a = 0;

      stg_pose_t p1;
      stg_pose_t p2;

      // shift the line points into the global coordinate system
      stg_pose_sum( &p1, pose, &pp1 );
      stg_pose_sum( &p2, pose, &pp2 );

      //model_local_to_global( mod, &p1 );
      //model_local_to_global( mod, &p2 );


      //printf( "tracing %.2f %.2f   %.2f %.2f\n",  p1.x, p1.y, p2.x, p2.y );

      itl_t* itl = itl_create( p1.x, p1.y, p2.x, p2.y, 
			       mod->world->matrix, 
			       PointToPoint );

      model_t* hitmod;
      while( (hitmod = itl_next( itl )) ) 
	{
	  if( hitmod != mod && model_get_obstaclereturn(hitmod) ) //&& !IsDescendent(ent) &&
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
stg_pose_t* model_get_pose( model_t* model )
{
  //stg_pose_t* pose = model_get_prop_data_generic( model, STG_PROP_POSE );
  //assert(pose);
  return &model->pose;
}

int model_update_pose( model_t* model )
{ 
  PRINT_DEBUG1( "pose update model %d", model->id );
 
  stg_velocity_t* vel = model_get_velocity(model);  


  stg_pose_t pose;
  memcpy( &pose, model_get_pose( model ), sizeof(pose));
  
  stg_pose_t oldpose;
  memcpy( &oldpose, &pose, sizeof(pose));

  stg_energy_data_t* en = model_get_energy_data( model );

  //if( en->joules > 0 && (vel->x || vel->y || vel->a ) )
  if( (vel->x || vel->y || vel->a ) )
    {
      
      // convert msec to sec
      double interval = (double)model->world->sim_interval / 1000.0;
      
      // global mode
      //pose.x += vel->x * interval;
      //pose.y += vel->y * interval;
      //pose.a += vel->a * interval;
      
      // local mode
      pose.x += interval * (vel->x * cos(pose.a) - vel->y * sin(pose.a));
      pose.y += interval * (vel->x * sin(pose.a) + vel->y * cos(pose.a));
      pose.a += interval * vel->a;

      if( model_test_collision_at_pose( model, &pose, NULL, NULL ) )
	{
	  PRINT_WARN( "HIT" );
	  // add the stall flag to the current pose
	  if( oldpose.stall == 0 )
	    {
	      oldpose.stall = 1;

	      // now set the new pose handling matrix & gui redrawing 
	      model_set_pose( model, &oldpose );

	    }	  
	}
      else	  
	{
	  pose.stall = 0;

	  // now set the new pose handling matrix & gui redrawing 
	  model_set_pose( model, &pose );
	  
	  // ignore acceleration in energy model for now, we just pay
	  // something to move.	
	  stg_kg_t mass = *model_get_mass( model );
	  model_energy_consume( model, STG_ENERGY_COST_MOTIONKG * mass ); 
	}      
    }
  
  return 0; // ok
}

void gui_model_pose( model_t* mod )
{ 
  stg_pose_t* pose = model_get_pose( mod );
  //PRINT_DEBUG( "gui model pose" );
  rtk_fig_origin( gui_model_figs(mod)->top, 
		  pose->x, pose->y, pose->a );
}

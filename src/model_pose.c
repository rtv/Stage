///////////////////////////////////////////////////////////////////////////
//
// File: model_pose.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_pose.c,v $
//  $Author: rtv $
//  $Revision: 1.30 $
//
///////////////////////////////////////////////////////////////////////////

#include <math.h>

#define DEBUG

#include "stage.h"
#include "gui.h"

extern rtk_fig_t* fig_debug; 

int lines_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{
  // Ignore myself, my children, and my ancestors.
  if( (!stg_model_is_related(mod,hitmod))  &&  
      hitmod->obstacle_return ) 
    return 1;
  
  return 0; // no match
}	


///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision with obstacles.
// Returns a pointer to the first entity we are in collision with, and stores
// the location of the hit in hitx,hity (if non-null)
// Returns NULL if not collisions.
// This function is useful for writing position devices.
stg_model_t* stg_model_test_collision_at_pose( stg_model_t* mod, 
					   stg_pose_t* pose, 
					   double* hitx, double* hity )
{
  //return NULL;
  
  // raytrace along all our rectangles. expensive, but most vehicles
  // will just be a single rect, grippers 3 rects, etc. not too bad.
  
  size_t count=0;
  stg_line_t* lines = stg_model_get_lines(mod, &count);

  // no body? no collision
  if( count < 1 )
    return NULL;

  //if( fig_debug ) rtk_fig_clear( fig_debug );

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

      //printf( "tracing %.2f %.2f   %.2f %.2f\n",  p1.x, p1.y, p2.x, p2.y );

      itl_t* itl = itl_create( p1.x, p1.y, p2.x, p2.y, 
			       mod->world->matrix, 
			       PointToPoint );

      stg_model_t* hitmod = itl_first_matching( itl, lines_raytrace_match, mod );
      
      if( hitmod )
	{
	  if( hitx ) *hitx = itl->x; // report them
	  if( hity ) *hity = itl->y;	  
	}

      itl_destroy( itl );

      return hitmod; // we hit this object! stop raytracing
    }

  return NULL;  // done 
}



int stg_model_update_pose( stg_model_t* model )
{ 
  PRINT_DEBUG4( "pose update model %d (vel %.2f, %.2f %.2f)", 
		model->id, model->velocity.x, model->velocity.y, model->velocity.a );
 
  stg_velocity_t* vel = stg_model_get_velocity(model);  


  stg_pose_t pose;
  memcpy( &pose, stg_model_get_pose( model ), sizeof(pose));
  
  stg_pose_t oldpose;
  memcpy( &oldpose, &pose, sizeof(pose));

  stg_energy_data_t* en = stg_model_get_energy_data( model );

  // see if we get any velocity from impacts
  stg_model_impact( model );

  //if( en->joules > 0 && (vel->x || vel->y || vel->a ) )
  if( (vel->x || vel->y || vel->a ) )
    {
      
      // convert msec to sec
      double interval = (double)model->world->sim_interval / 1000.0;
      
      // global mode
      pose.x += vel->x * interval;
      pose.y += vel->y * interval;
      pose.a += vel->a * interval;
      
      double hitx=0, hity=0;
      stg_model_t* hitthing = 
	stg_model_test_collision_at_pose( model, &pose, &hitx, &hity );
      
      

      if( hitthing )
	{
	  PRINT_DEBUG( "HIT something!" );

	  model->stall = 1;

	  if( 0 )
	    {
	      puts( "hitting" );
	      // Get the velocity of the thing we hit
	      //stg_velocity_t* vel = stg_model_get_velocity( hitthing );
	      double impact_vel = hypot( vel->x, vel->y );
	      
	      // TODO - use relative mass and velocity properly

	      // and mass
	      //stg_kg_t* mass = stg_model_get_mass( hitthing );
	     
	      // find the position of both objects	      
	      //stg_pose_t p;
	      //memset(&p,0,sizeof(p));
	      //stg_model_global_pose( model, &p );

	      stg_pose_t o;
	      memset(&o,0,sizeof(o));
	      stg_model_global_pose( hitthing, &o );

	      // Compute bearing FROM impacted ent
	      // Align ourselves so we are facing away from this point
	      double pth = atan2( o.y-pose.y, o.x-pose.x );
	      
	      if( impact_vel )
		{
		  double vr =  fabs(impact_vel);

		  stg_velocity_t given;
		  given.x = vr * cos(pth);
		  given.y = vr * sin(pth);
		  given.a = 0.0;
		  
		  // get some velocity from the impact
		  //hitthing->velocity.x = vr * cos(pth);
		  //hitthing->velocity.y = vr * sin(pth);		  

		  printf( "gave %.2f %.2f vel\n",
			  given.x, given.y );

		  stg_model_set_velocity( hitthing, &given );
		}
	      
	    }
	  
	}
      else	  
	{
	  model->stall = 0;

	  // now set the new pose handling matrix & gui redrawing 
	  stg_model_set_pose( model, &pose );
	  
	  // ignore acceleration in energy model for now, we just pay
	  // something to move.	
	  stg_kg_t mass = *stg_model_get_mass( model );
	  stg_model_energy_consume( model, STG_ENERGY_COST_MOTIONKG * mass ); 

	  // record the movement in odometry
	  model->odom.x += vel->x * interval;
	  model->odom.y += vel->y * interval;
	  model->odom.a += vel->a * interval;
	  model->odom.a = NORMALIZE( model->odom.a );
	}      
    }
  
  return 0; // ok
}

void stg_model_impact(  stg_model_t* model )
{

}

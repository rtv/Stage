///////////////////////////////////////////////////////////////////////////
//
// File: model_pose.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_pose.c,v $
//  $Author: rtv $
//  $Revision: 1.34 $
//
///////////////////////////////////////////////////////////////////////////

#include <math.h>

//#define DEBUG

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

  if( fig_debug ) rtk_fig_clear( fig_debug );

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
      
      itl_destroy( itl );

      if( hitmod )
	{
	  if( hitx ) *hitx = itl->x; // report them
	  if( hity ) *hity = itl->y;	  
	  return hitmod; // we hit this object! stop raytracing
	}
    }

  return NULL;  // done 
}



int stg_model_update_pose( stg_model_t* mod )
{ 
  PRINT_DEBUG4( "pose update model %d (vel %.2f, %.2f %.2f)", 
		mod->id, mod->velocity.x, mod->velocity.y, mod->velocity.a );
 
  stg_velocity_t gvel;
  stg_model_global_velocity( mod, &gvel );
      
  stg_velocity_t gpose;
  stg_model_global_pose( mod, &gpose );

  // convert msec to sec
  double interval = (double)mod->world->sim_interval / 1000.0;
  
  // compute new global position
  gpose.x += gvel.x * interval;
  gpose.y += gvel.y * interval;
  gpose.a += gvel.a * interval;
      
  double hitx=0, hity=0;
  stg_model_t* hitthing =
    stg_model_test_collision_at_pose( mod, &gpose, &hitx, &hity );
      
  if( mod->friction )
    {
      // compute a new velocity, based on "friction"
      double vr = hypot( gvel.x, gvel.y );
      double va = atan2( gvel.y, gvel.x );
      vr -= vr * mod->friction;
      gvel.x = vr * cos(va);
      gvel.y = vr * sin(va);
      gvel.a -= gvel.a * mod->friction; 

      // lower bounds
      if( fabs(gvel.x) < 0.001 ) gvel.x == 0.0;
      if( fabs(gvel.y) < 0.001 ) gvel.y == 0.0;
      if( fabs(gvel.a) < 0.01 ) gvel.a == 0.0;
	  
    }

  if( hitthing )
    {
      if( hitthing->friction == 0 ) // hit an immovable thing
	{
	  PRINT_DEBUG( "HIT something immovable!" );
	  mod->stall = 1;
	}
      else
	{
	  puts( "hit something with non-zero friction" );

	  // Get the velocity of the thing we hit
	  //stg_velocity_t* vel = stg_model_get_velocity( hitthing );
	  double impact_vel = hypot( gvel.x, gvel.y );
	      
	  // TODO - use relative mass and velocity properly
	  //stg_kg_t* mass = stg_model_get_mass( hitthing );
	     
	  // Compute bearing from my center of mass to the impact point
	  double pth = atan2( hity-gpose.y, hitx-gpose.x );
	      
	  // Compute bearing TO impacted ent
	  //double pth2 = atan2( o.y-pose.y, o.x-pose.x );
	      
	  if( impact_vel )
	    {
	      double vr =  fabs(impact_vel);

	      stg_velocity_t given;
	      given.x = vr * cos(pth);
	      given.y = vr * sin(pth);
	      given.a = 0;//vr * sin(pth2);
		  
	      // get some velocity from the impact
	      //hitthing->velocity.x = vr * cos(pth);
	      //hitthing->velocity.y = vr * sin(pth);		  

	      printf( "gave %.2f %.2f vel\n",
		      given.x, given.y );

	      stg_model_set_global_velocity( hitthing, &given );
	    }
	      
	}
	  
    }
  else	  
    {
      mod->stall = 0;

      // now set the new pose
      stg_model_set_global_pose( mod, &gpose );
	  
      // ignore acceleration in energy model for now, we just pay
      // something to move.	
      stg_kg_t mass = *stg_model_get_mass( mod );
      stg_model_energy_consume( mod, STG_ENERGY_COST_MOTIONKG * mass ); 

      // record the movement in odometry
      mod->odom.x += mod->velocity.x * interval;
      mod->odom.y += mod->velocity.y * interval;
      mod->odom.a += mod->velocity.a * interval;
      mod->odom.a = NORMALIZE( mod->odom.a );
    }      
  
  return 0; // ok
}


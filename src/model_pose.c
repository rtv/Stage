
#include <math.h>

#include "stage.h"
#include "raytrace.h"

void model_update_pose( stg_model_t* model )
{  
  stg_velocity_t* vel = &model->velocity;
  
  if( vel->x || vel->y || vel->a )
  {
    double interval = model->world->sim_interval;

    stg_pose_t pose;
    memcpy( &pose, &model->pose, sizeof(pose) );

    // global mode
    //pose.x += vel->x * interval;
    //pose.y += vel->y * interval;
    //pose.a += vel->a * interval;

    // local mode
    pose.x += interval * (vel->x * cos(pose.a) - vel->y * sin(pose.a));
    pose.y += interval * (vel->x * sin(pose.a) + vel->y * cos(pose.a));
    pose.a += interval * vel->a;
    
    model_set_prop( model, STG_PROP_POSE, &pose, sizeof(pose) );
  }
}


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
    
    pose.x += vel->x * interval;
    pose.y += vel->y * interval;
    pose.a += vel->a * interval;
    
    model_set_prop( model, STG_PROP_POSE, &pose, sizeof(pose) );
  }
}


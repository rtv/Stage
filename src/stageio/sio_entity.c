

#include "sio_entity.h"

int SIOBufferPose( int id, stage_pose_t* pose )
{
  SIOBufferHeader( 0, STG_HDR_PROPS, sizeof(stage_pose_t) );
  
  stage_property_t prop;
  prop.id = id; // the property is destined for this model
  prop.property = STG_PROP_ENTITY_POSE; // type of property
  prop.len = sizeof(stage_pose_t); // size of data
  
  // buffer the prop header
  SIOBufferPacket( (char*)&prop, sizeof(prop) );
  
  // buffer the prop data
  SIOBufferPacket( (char*)pose, sizeof(pose) );
}


int SIOBufferSize( int id, stage_size_t* size );
int SIOBufferSize( int id, stage_size_t* size );


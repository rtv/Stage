#ifndef _STAGEIO_H
#define _STAGEIO_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "sio.h"   
#include "stage.h"

   int SIOBufferPose( int id, stage_pose_t* pose );
   int SIOBufferSize( int id, stage_size_t* size );
     
#ifdef __cplusplus
 }
#endif 

#endif

#include <gl.h>
#include "stage.hh"


// transform the current coordinate frame by the given pose
void gl_coord_shift( double x, double y, double z, double a  )
{
  glTranslatef( x,y,z );
  glRotatef( RTOD(a), 0,0,1 );
}

// transform the current coordinate frame by the given pose
void gl_pose_shift( stg_pose_t* pose )
{
  gl_coord_shift( pose->x, pose->y, pose->z, pose->a );
}

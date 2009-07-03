
#include "stage.hh"
using namespace Stg;

Waypoint::Waypoint( const Pose& pose, Color color ) 
  : pose(pose), color(color)
{ 
}

Waypoint::Waypoint( stg_meters_t x, stg_meters_t y, stg_meters_t z, stg_radians_t a, Color color ) 
  : pose(x,y,z,a), color(color)
{ 
}


Waypoint::Waypoint()
{ 
  pose = Pose( 0,0,0,0 ); 
  color = 0; 
};


void Waypoint::Draw()
{
  GLdouble d[4];
  
  d[0] = color.r;
  d[1] = color.g;
  d[2] = color.b;
  d[3] = color.a;
  
  glColor4dv( d );
  
  glBegin(GL_POINTS);
  glVertex3f( pose.x, pose.y, pose.z );
  glEnd();

  stg_meters_t quiver_length = 0.15;

  double dx = cos(pose.a) * quiver_length;
  double dy = sin(pose.a) * quiver_length;

  glBegin(GL_LINES);
  glVertex3f( pose.x, pose.y, pose.z );
  glVertex3f( pose.x+dx, pose.y+dy, pose.z );
  glEnd();	 
}

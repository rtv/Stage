
#include "stage_internal.hh"

Waypoint::Waypoint( stg_meters_t x, stg_meters_t y, stg_meters_t z, stg_radians_t a, stg_color_t color ) 
  : color(color)
{ 
  pose.x = x; 
  pose.y = y; 
  pose.z = z; 
  pose.a = a; 
}

Waypoint::Waypoint()
{ 
  pose = stg_pose_t( 0,0,0,0 ); 
  color = 0; 
};


void Waypoint::Draw()
{
 	GLdouble d[4];

 	d[0] = ((color & 0x00FF0000) >> 16) / 256.0;
 	d[1] = ((color & 0x0000FF00) >> 8)  / 256.0;
 	d[2] = ((color & 0x000000FF) >> 0)  / 256.0;
 	d[3] = (((color & 0xFF000000) >> 24) / 256.0);

   glColor4dv( d );

  glBegin(GL_POINTS);
  glVertex3f( pose.x, pose.y, pose.z );
  glEnd();

  stg_meters_t quiver_length = 0.1;

  double dx = cos(pose.a) * quiver_length;
  double dy = sin(pose.a) * quiver_length;

  glBegin(GL_LINES);
  glVertex3f( pose.x, pose.y, pose.z );
  glVertex3f( pose.x+dx, pose.y+dy, pose.z );
  glEnd();	 
}

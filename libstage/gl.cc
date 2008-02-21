
#include "stage_internal.hh"


// transform the current coordinate frame by the given pose
void Stg::gl_coord_shift( double x, double y, double z, double a  )
{
  glTranslatef( x,y,z );
  glRotatef( rtod(a), 0,0,1 );
}

// transform the current coordinate frame by the given pose
void Stg::gl_pose_shift( stg_pose_t* pose )
{
  gl_coord_shift( pose->x, pose->y, pose->z, pose->a );
}

// TODO - this could be faster, but we don't draw a lot of text
void Stg::gl_draw_string( float x, float y, float z, char *str ) 
{  
  char *c;
  glRasterPos3f(x, y,z);
  for (c=str; *c != '\0'; c++) 
    glutBitmapCharacter( GLUT_BITMAP_HELVETICA_12, *c);
}

void Stg::gl_draw_grid( stg_bounds3d_t vol )
{
  glBegin(GL_LINES);
  
  for( double i = floor(vol.x.min); i < vol.x.max; i++)
    {
      glVertex2f(  i, vol.y.min );
      glVertex2f(  i, vol.y.max );
    }
  
  for( double i = floor(vol.y.min); i < vol.y.max; i++)
    {
      glVertex2f(  vol.x.min, i );
      glVertex2f(  vol.x.max, i );
    }
  
  glEnd();
  
  char str[16];

  for( double i = floor(vol.x.min); i < vol.x.max; i++)
    {
      snprintf( str, 16, "%d", (int)i );
      gl_draw_string(  i, 0, 0.01, str );
    }

  for( double i = floor(vol.y.min); i < vol.y.max; i++)
    {
      snprintf( str, 16, "%d", (int)i );
      gl_draw_string(  0, i, 0.01, str );
    }
}



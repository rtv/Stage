
#include "stage_internal.hh"

// transform the current coordinate frame by the given pose
void Stg::gl_coord_shift( double x, double y, double z, double a  )
{
	glTranslatef( x,y,z );
	glRotatef( rtod(a), 0,0,1 );
}

// transform the current coordinate frame by the given pose
void Stg::gl_pose_shift( const stg_pose_t &pose )
{
	gl_coord_shift( pose.x, pose.y, pose.z, pose.a );
}

void Stg::gl_pose_inverse_shift( const stg_pose_t &pose )
{
  gl_coord_shift( 0,0,0, -pose.a );
  gl_coord_shift( -pose.x, -pose.y, -pose.z, 0 );
}


void Stg::gl_draw_string( float x, float y, float z, const char *str ) 
{  
	glRasterPos3f( x, y, z );
	//printf( "[%.2f %.2f %.2f] string %u %s\n", x,y,z,(unsigned int)strlen(str), str ); 
	gl_draw(str);
}

void Stg::gl_speech_bubble( float x, float y, float z, const char* str )
{  
	gl_draw_string( x, y, z, str );
}

// draw an octagon with center rectangle dimensions w/h
//   and outside margin m
void Stg::gl_draw_octagon( float w, float h, float m )
{
	glBegin(GL_POLYGON);
	glVertex2f( m+w, 0 );
	glVertex2f( w+2*m, m );
	glVertex2f( w+2*m, h+m );
	glVertex2f( m+w, h+2*m );
	glVertex2f( m, h+2*m );
	glVertex2f( 0, h+m );
	glVertex2f( 0, m );
	glVertex2f( m, 0 );
	glEnd();
}

void Stg::gl_draw_vector( double x, double y, double z )
{
  glBegin( GL_LINES );
  glVertex3f( 0,0,0 );
  glVertex3f( x,y,z );
  glEnd();
}

void Stg::gl_draw_origin( double len )
{
  gl_draw_vector( len,0,0 );
  gl_draw_vector( 0,len,0 );
  gl_draw_vector( 0,0,len );
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
		gl_draw_string(  i, 0, 0.00, str );
	}

	for( double i = floor(vol.y.min); i < vol.y.max; i++)
	{
		snprintf( str, 16, "%d", (int)i );
		gl_draw_string(  0, i, 0.00, str );
	}
}



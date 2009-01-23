
#include "stage.hh"
using namespace Stg;

// transform the current coordinate frame by the given pose
void Stg::Gl::coord_shift( double x, double y, double z, double a  )
{
	glTranslatef( x,y,z );
	glRotatef( rtod(a), 0,0,1 );
}

// transform the current coordinate frame by the given pose
void Stg::Gl::pose_shift( const Pose &pose )
{
	coord_shift( pose.x, pose.y, pose.z, pose.a );
}

void Stg::Gl::pose_inverse_shift( const Pose &pose )
{
  coord_shift( 0,0,0, -pose.a );
  coord_shift( -pose.x, -pose.y, -pose.z, 0 );
}


void Stg::Gl::draw_string( float x, float y, float z, const char *str ) 
{  
	glRasterPos3f( x, y, z );
	//printf( "[%.2f %.2f %.2f] string %u %s\n", x,y,z,(unsigned int)strlen(str), str ); 
	gl_draw(str);
}

void Stg::Gl::draw_speech_bubble( float x, float y, float z, const char* str )
{  
	draw_string( x, y, z, str );
}

// draw an octagon with center rectangle dimensions w/h
//   and outside margin m
void Stg::Gl::draw_octagon( float w, float h, float m )
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

void Stg::Gl::draw_vector( double x, double y, double z )
{
  glBegin( GL_LINES );
  glVertex3f( 0,0,0 );
  glVertex3f( x,y,z );
  glEnd();
}

void Stg::Gl::draw_origin( double len )
{
  draw_vector( len,0,0 );
  draw_vector( 0,len,0 );
  draw_vector( 0,0,len );
}

void Stg::Gl::draw_grid( stg_bounds3d_t vol )
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
		draw_string(  i, 0, 0.00, str );
	}

	for( double i = floor(vol.y.min); i < vol.y.max; i++)
	{
		snprintf( str, 16, "%d", (int)i );
		draw_string(  0, i, 0.00, str );
	}
}



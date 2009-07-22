
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

void Stg::Gl::draw_array( float x, float y, float w, float h, 
								  float* data, size_t len, size_t offset,
								  float min, float max )
{
  float sample_spacing = w / (float)len;
  float yscale = h / (max-min);
  
  //printf( "min %.2f max %.2f\n", min, max );

  glBegin( GL_LINE_STRIP );

  for( unsigned int i=0; i<len; i++ )
	 glVertex3f( x + (float)i*sample_spacing, y+(data[(i+offset)%len]-min)*yscale, 0.01 );
  
  glEnd();
  
  glColor3f( 0,0,0 );
  char buf[64];
  snprintf( buf, 63, "%.2f", min );
  Gl::draw_string( x,y,0,buf );
  snprintf( buf, 63, "%.2f", max );
  Gl::draw_string( x,y+h-fl_height(),0,buf );

}

void Stg::Gl::draw_array( float x, float y, float w, float h, 
								  float* data, size_t len, size_t offset )
{
  // wild initial bounds
  float smallest = 1e16;
  float largest = -1e16;
  
  for( size_t i=0; i<len; i++ )
	 {
		smallest = std::min( smallest, data[i] );
		largest = std::max( largest, data[i] );
	 }

  draw_array( x,y,w,h,data,len,offset,smallest,largest );
}

void Stg::Gl::draw_string( float x, float y, float z, const char *str ) 
{  
  glRasterPos3f( x, y, z );
  //printf( "[%.2f %.2f %.2f] string %u %s\n", x,y,z,(unsigned int)strlen(str), str ); 
  gl_draw( str );
}

void Stg::Gl::draw_string_multiline( float x, float y, float w, float h,  const char *str, Fl_Align align ) 
{  
  //glRasterPos3f( x, y, z );
	//printf( "[%.2f %.2f %.2f] string %u %s\n", x,y,z,(unsigned int)strlen(str), str ); 
  gl_draw(str, x, y, w, h, align ); // fltk function
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

// draw an octagon with center rectangle dimensions w/h
//   and outside margin m
void Stg::Gl::draw_octagon( float x, float y, float w, float h, float m )
{
	glBegin(GL_POLYGON);
	glVertex2f( x + m+w, y );
	glVertex2f( x+w+2*m, y+m );
	glVertex2f( x+w+2*m, y+h+m );
	glVertex2f( x+m+w, y+h+2*m );
	glVertex2f( x+m, y+h+2*m );
	glVertex2f( x, y+h+m );
	glVertex2f( x, y+m );
	glVertex2f( x+m, y );
	glEnd();
}


void Stg::Gl::draw_centered_rect( float x, float y, float dx, float dy )
{
  glRectf( x-0.5*dx, y-0.5*dy, x+0.5*dx, y+0.5*dy );
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



/** powerpack.cc
	 Simple model of energy storage
	 Richard Vaughan
	 Created 2009.1.15
    SVN: $Id: stage.hh 7279 2009-01-18 00:10:21Z rtv $
*/

#include "stage.hh"
using namespace Stg;

PowerPack::PowerPack( Model* mod ) :
  mod( mod), stored( 0.0 ), capacity( 0.0 ), charging( false )
{ 
  // nothing to do 
};


void PowerPack::Print( char* prefix )
{
  printf( "%s stored %.2f/%.2f joules\n", prefix, stored, capacity );
}

/** OpenGL visualization of the powerpack state */
void PowerPack::Visualize( Camera* cam )
{
  const double height = 0.5;
  const double width = 0.2;

  double percent = stored/capacity * 100.0;

  if( percent > 50 )		
	 glColor4f( 0,1,0, 0.7 ); // green
  else if( percent > 25 )
	 glColor4f( 1,0,1, 0.7 ); // magenta
  else
	 glColor4f( 1,0,0, 0.7 ); // red
  
  static char buf[6];
  snprintf( buf, 6, "%.0f", percent );

  glTranslatef( -width, 0.0, 0.0 );

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  GLfloat fullness =  height * (percent * 0.01);
  glRectf( 0,0,width, fullness);

  // outline the charge-o-meter
  glTranslatef( 0,0,0.001 );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

  glColor4f( 0,0,0,0.7 );
  
  glRectf( 0,0,width, height );
  
  glBegin( GL_LINES );
  glVertex2f( 0, fullness );
  glVertex2f( width, fullness );
  glEnd();
  
  if( charging )
	 {
		glLineWidth( 6.0 );
		glColor4f( 1,0,0,0.7 );
		
		glRectf( 0,0,width, height );
		
		glLineWidth( 1.0 );
	 }

  // draw the percentage
  //gl_draw_string( -0.2, 0, 0, buf );

  // ?
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

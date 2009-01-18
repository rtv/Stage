/** powerpack.cc
	 Simple model of energy storage
	 Richard Vaughan
	 Created 2009.1.15
	 $Id$
*/

#include "stage_internal.hh"

PowerPack::PowerPack( Model* mod ) :
  mod( mod), stored( 0.0 ), capacity( 0.0 )
{ 
  // nothing to do 
};


/** OpenGL visualization of the powerpack state */
void PowerPack::Visualize( Camera* cam )
{
  const double height = 0.5;
  const double width = 0.3;

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

  // draw the percentage
  //gl_draw_string( -0.2, 0, 0, buf );

  // ?
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

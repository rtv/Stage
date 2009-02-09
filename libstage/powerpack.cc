/** powerpack.cc
	 Simple model of energy storage
	 Richard Vaughan
	 Created 2009.1.15
    SVN: $Id: stage.hh 7279 2009-01-18 00:10:21Z rtv $
*/

#include "stage.hh"
#include "texture_manager.hh"
using namespace Stg;

PowerPack::PowerPack( Model* mod ) :
  mod( mod), stored( 0.0 ), capacity( 0.0 ), charging( false )
{ 
  // tell the world about this new pp
  mod->world->AddPowerPack( this );
};

PowerPack::~PowerPack()
{
  // tell the world about this new pp
  mod->world->RemovePowerPack( this );
}


void PowerPack::Print( char* prefix )
{
  printf( "%s stored %.2f/%.2f joules\n", prefix, stored, capacity );
}

// this is called every world update cycle - can we get away without doing this?
void PowerPack::Update()
{ 
  charging = false;
}


/** OpenGL visualization of the powerpack state */
void PowerPack::Visualize( Camera* cam )
{
  const double height = 0.5;
  const double width = 0.2;
  
		// draw an electric zap
// 		glPolygonMode( GL_FRONT, GL_LINE );
// 		glBegin( GL_POLYGON );
// 		glVertex2i( 0, 0 );
// 		glVertex2i( 3, 2 );
// 		glVertex2i( 1, 2 );
// 		glEnd();

// 		glVertex2i( 1, 3 );
// 		glVertex2i( 0, 3 );
// 		glVertex2i( 1, 5 );
// 		glVertex2i( 3, 5 );
// 		glVertex2i( 4, 3 );
// 		glVertex2i( 5, 3 );
// 		glVertex2i( 4, 4 );
// 		glVertex2i( 5, 4);
// 		glEnd();
  //}
  
  double percent = stored/capacity * 100.0;
  
  const double alpha = 0.5;

  if( percent > 50 )		
	 glColor4f( 0,1,0, alpha ); // green
		else if( percent > 25 )
		  glColor4f( 1,0,1, alpha ); // magenta
		else
		  glColor4f( 1,0,0, alpha ); // red
  
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
  
  if( stored < 0.0 ) // inifinite supply!
	 {
		// draw an arrow toward the top
		glBegin( GL_LINES );
		glVertex2f( width/3.0,     height/3.0 );
		glVertex2f( 2.0 * width/3, height/3.0 );
		
		glVertex2f( width/3.0,     height/3.0 );
		glVertex2f( width/3.0,     height - height/5.0 );
		
		glVertex2f( width/3.0,     height - height/5.0 );
		glVertex2f( 0,     height - height/5.0 );
		
		glVertex2f( 0,     height - height/5.0 );
		glVertex2f( width/2.0,     height );
		
		glVertex2f( width/2.0,     height );
		glVertex2f( width,     height - height/5.0 );
		
		glVertex2f( width,     height - height/5.0 );
		glVertex2f( 2.0 * width/3.0, height - height/5.0 );
		
		glVertex2f( 2.0 * width/3.0, height - height/5.0 );
		glVertex2f( 2.0 * width/3, height/3.0 );			 
		
		glEnd();		
	 }
  

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


stg_joules_t PowerPack::RemainingCapacity()
{
  return( capacity - stored );
}

void PowerPack::Add( stg_joules_t j )
{
  stored += MIN( RemainingCapacity(), j );
}

void PowerPack::Subtract( stg_joules_t j )
{
  if( stored >= 0.0 )
	 stored -= MIN( stored, j );  
}

void PowerPack::TransferTo( PowerPack* dest, stg_joules_t amount )
{
  //printf( "amount %.2f stored %.2f dest capacity %.2f\n",
  //	 amount, stored, dest->RemainingCapacity() );

  // if stored is non-negative we can't transfer more than the stored
  // amount. If it is negative, we have infinite energy stored
  if( stored >= 0.0 )
	 amount = MIN( stored, amount );
  
  // we can't transfer more than he can take
  amount = MIN( amount, dest->RemainingCapacity() );
 

  //printf( "%s gives %.3f J to %s\n",
  //	 mod->Token(), amount, dest->mod->Token() );
  
  Subtract( amount );
  dest->Add( amount );
}

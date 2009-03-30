/** powerpack.cc
	 Simple model of energy storage
	 Richard Vaughan
	 Created 2009.1.15
    SVN: $Id: stage.hh 7279 2009-01-18 00:10:21Z rtv $
*/

#include "stage.hh"
#include "texture_manager.hh"
using namespace Stg;


stg_joules_t PowerPack::global_stored = 0.0;
stg_joules_t PowerPack::global_input = 0.0;
stg_joules_t PowerPack::global_capacity = 0.0;
stg_joules_t PowerPack::global_dissipated = 0.0;
stg_watts_t PowerPack::global_power = 0.0;
stg_watts_t PowerPack::global_power_smoothed = 0.0;
double PowerPack::global_smoothing_constant = 0.05;


PowerPack::PowerPack( Model* mod ) :
  event_vis( 32,32,1.0 ),
  mod( mod), 
  stored( 0.0 ), 
  capacity( 0.0 ), 
  charging( false )  
{ 
  // tell the world about this new pp
  mod->world->AddPowerPack( this );  
  mod->AddVisualizer( &event_vis, false );
};

PowerPack::~PowerPack()
{
  mod->world->RemovePowerPack( this );
  mod->RemoveVisualizer( &event_vis );
}


void PowerPack::Print( char* prefix ) const
{
  printf( "%s stored %.2f/%.2f joules\n", prefix, stored, capacity );
}

/** OpenGL visualization of the powerpack state */
void PowerPack::Visualize( Camera* cam ) const
{
  const double height = 0.5;
  const double width = 0.2;
  
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
  // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}


stg_joules_t PowerPack::RemainingCapacity() const
{
  return( capacity - stored );
}

void PowerPack::Add( stg_joules_t j )
{
  stg_joules_t amount = MIN( RemainingCapacity(), j );
  stored += amount;
  global_stored += amount;
  
  if( amount > 0 ) charging = true;
}

void PowerPack::Subtract( stg_joules_t j )
{
  if( stored < 0 ) // infinte supply!
	 {
		global_input += j; // record energy entering the system
		return;
	 }
  stg_joules_t amount = MIN( stored, j );  

  stored -= amount;  
  global_stored -= amount;
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

  mod->NeedRedraw();
}


void PowerPack::SetCapacity( stg_joules_t cap )
{
  global_capacity -= capacity;
  capacity = cap;
  global_capacity += capacity;
  
  if( stored > cap )
	 {
		global_stored -= stored;
		stored = cap;
		global_stored += stored;		
	 }
}

stg_joules_t PowerPack::GetCapacity() const
{
  return capacity;
}

stg_joules_t PowerPack::GetStored() const
{
  return stored;
}

void PowerPack::SetStored( stg_joules_t j ) 
{
  global_stored -= stored;
  stored = j;
  global_stored += stored;  
}

void PowerPack::Dissipate( stg_joules_t j )
{
  stg_joules_t amount = MIN( stored, j );
  
  Subtract( amount );
  global_dissipated += amount;

  //stg_watts_t w = j / (interval / 1e6);
}

void PowerPack::Dissipate( stg_joules_t j, const Pose& p )
{
  Dissipate( j );
  event_vis.Accumulate( p.x, p.y, j );
}

//------------------------------------------------------------------------------
// Dissipation Visualizer class


PowerPack::DissipationVis::DissipationVis( stg_meters_t width, 
														 stg_meters_t height,
														 stg_meters_t cellsize )
  : Visualizer( "energy dissipation", "energy_dissipation" ),
	 columns(width/cellsize),
	 rows(height/cellsize),
	 width(width),
	 height(height),
	 cells( new stg_joules_t[columns*rows] ),
	 peak_value(0),
	 cellsize(cellsize)
{ /* nothing to do */ }

PowerPack::DissipationVis::~DissipationVis()
{
  if( cells )
	 delete[] cells;
}

void PowerPack::DissipationVis::Visualize( Model* mod, Camera* cam )
{
  // go into world coordinates
  
  glPushMatrix();

  Gl::pose_inverse_shift( mod->GetPose() );

  glTranslatef( -width/2.0, -height/2.0, 0 );
  glScalef( cellsize, cellsize, 1 );
  
  for( unsigned int y=0; y<rows; y++ )
	 for( unsigned int x=0; x<columns; x++ )
		{
		  stg_joules_t j = cells[ y*columns + x ];
		  if( j > 0 )
			 {
				glColor4f( 1.0, 0, 0, j/peak_value );
				glRectf( x,y,x+1,y+1 );
			 }
		}

  glPopMatrix();
}



void PowerPack::DissipationVis::Accumulate( stg_meters_t x, 
														  stg_meters_t y, 
														  stg_joules_t amount )
{
  int ix = (x+width/2.0)/cellsize;
  int iy = (y+height/2.0)/cellsize;

  assert( ix >= 0 );
  assert( ix < (int)columns );
  assert( iy >= 0 );
  assert( iy < (int)rows );

  stg_joules_t* j = cells + (iy*columns + ix );
  
  (*j) += amount;
  if( (*j) > peak_value )
	 peak_value  = (*j);
}

/*
  region.cc
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

#include "region.hh"
using namespace Stg;

Region::Region( SuperRegion* sr) : 
  cells(), 
  superregion(sr),
  count(0)
{ 
}

Region::~Region()
{
}

SuperRegion::SuperRegion( World* world, stg_point_int_t origin ) 
  : regions(),
	 origin(origin), 
	 world(world), 
	 count(0)	 
{
  // populate the regions
  regions.insert( regions.begin(), SUPERREGIONSIZE, Region( this ) );			   
}

SuperRegion::~SuperRegion()
{
}

void SuperRegion::DrawOccupancy()
{
  glPushMatrix();	    
  GLfloat scale = 1.0/world->Resolution();
  glScalef( scale, scale, 1.0 ); // XX TODO - this seems slightly
  glTranslatef( origin.x<<SRBITS, origin.y<<SRBITS,0);
  
  glEnable( GL_DEPTH_TEST );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  
  // outline superregion
  glColor3f( 0,0,1 );   
  glRecti( 0,0, 1<<SRBITS, 1<<SRBITS );
  
  // outline regions
  const Region* r = GetRegion(0,0);
  char buf[32];

  glColor3f( 0,1,0 );    
  for( int y=0; y<SUPERREGIONWIDTH; y++ )
	 for( int x=0; x<SUPERREGIONWIDTH; x++ )
		{
		  if( r->count ) // region contains some occupied cells
			 {
				// outline the region
				glRecti( x<<RBITS, y<<RBITS, 
							(x+1)<<RBITS, (y+1)<<RBITS );
				
				// show how many cells are occupied
				snprintf( buf, 15, "%lu", r->count );
				Gl::draw_string( x<<RBITS, y<<RBITS, 0, buf );
				
				// draw a rectangle around each occupied cell
				for( int p=0; p<REGIONWIDTH; p++ )
				  for( int q=0; q<REGIONWIDTH; q++ )
					 if( r->cells[p+(q*REGIONWIDTH)].blocks.size() )
						{					 
						  GLfloat xx = p+(x<<RBITS);
						  GLfloat yy = q+(y<<RBITS);						  
						  glRecti( xx, yy, xx+1, yy+1);
						}
			 }
		  else if( ! r->cells.empty() ) // empty but used previously 
			 {
				double left = x << RBITS;
				double right = (x+1) << RBITS;
				double bottom = y << RBITS;
				double top = (y+1) << RBITS;
				
				double d = 3.0;
				
				// draw little corner markers for regions with memory
				// allocated but no contents
				glBegin( GL_LINES );
				glVertex2f( left, bottom );
				glVertex2f( left+d, bottom );
				glVertex2f( left, bottom );
				glVertex2f( left, bottom+d );
				glVertex2f( left, top );
				glVertex2f( left+d, top );
				glVertex2f( left, top );
				glVertex2f( left, top-d );
				glVertex2f( right, top );
				glVertex2f( right-d, top );
				glVertex2f( right, top );
				glVertex2f( right, top-d );
				glVertex2f( right, bottom );
				glVertex2f( right-d, bottom );
				glVertex2f( right, bottom );
				glVertex2f( right, bottom+d );
				glEnd();
			 }

		  r++; // next region quickly
		}
  
  snprintf( buf, 15, "%lu", count );
  Gl::draw_string( 1<<SBITS, 1<<SBITS, 0, buf );
    
  glPopMatrix();    
}


static void DrawBlock( GLfloat x, GLfloat y, GLfloat zmin, GLfloat zmax )
{
  glBegin( GL_QUADS );
  
  // TOP
  glVertex3f( x, y, zmax );
  glVertex3f( 1+x, y, zmax );
  glVertex3f( 1+x, 1+y, zmax );
  glVertex3f( x, 1+y, zmax );
  
  // sides
  glVertex3f( x, y, zmax );
  glVertex3f( x, 1+y, zmax );
  glVertex3f( x, 1+y, zmin );
  glVertex3f( x, y, zmin );
  
  glVertex3f( 1+x, y, zmax );
  glVertex3f( x, y, zmax );
  glVertex3f( x, y, zmin );
  glVertex3f( 1+x, y, zmin );
  
  glVertex3f( 1+x, 1+y, zmax );
  glVertex3f( 1+x, y, zmax );
  glVertex3f( 1+x, y, zmin );
  glVertex3f( 1+x, 1+y, zmin );
  
  glVertex3f( x, 1+y, zmax );
  glVertex3f( 1+x, 1+y, zmax );
  glVertex3f( 1+x, 1+y, zmin );
  glVertex3f( x, 1+y, zmin );

  glEnd();  
} 

void SuperRegion::DrawVoxels()
{
  glPushMatrix();	    
  GLfloat scale = 1.0/world->Resolution();
  glScalef( scale, scale, 1.0 ); // XX TODO - this seems slightly
  glTranslatef( origin.x<<SRBITS, origin.y<<SRBITS,0);


  glEnable( GL_DEPTH_TEST );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  
  Region* r = GetRegion( 0, 0);
  
  for( int y=0; y<SUPERREGIONWIDTH; y++ )
	 for( int x=0; x<SUPERREGIONWIDTH; x++ )
		{		  
		  if( r->count )
			 for( int p=0; p<REGIONWIDTH; p++ )
				for( int q=0; q<REGIONWIDTH; q++ )
				  {
					 Cell* c = (Cell*)&r->cells[p+(q*REGIONWIDTH)];
					 
					 if( c->blocks.size() )
						{					 
						  GLfloat xx = p+(x<<RBITS);
						  GLfloat yy = q+(y<<RBITS);
						  
						  FOR_EACH( it, c->blocks )
							 {
								Block* block = *it;
								// first draw filled polygons
								Color c = block->GetColor();
								glColor4f( c.r, c.g, c.b, 1.0 );
								
								glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
								glEnable(GL_POLYGON_OFFSET_FILL);
								// TODO - these numbers need tweaking for
								// better-looking rendering
								glPolygonOffset(0.01, 0.1);								
								DrawBlock( xx, yy, block->global_z.min, block->global_z.max );
								
								// draw again in outline
								glDisable(GL_POLYGON_OFFSET_FILL);								
								glColor4f( c.r/2.0, c.g/2.0, c.b/2.0, c.a );
								glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );								
								DrawBlock( xx, yy, block->global_z.min, block->global_z.max );								
						  }
					 }
				}
		  
		  ++r;
		}
  glPopMatrix();    
}

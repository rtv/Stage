/*
  region.cc
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

#include "region.hh"
using namespace Stg;

const uint32_t Region::WIDTH = REGIONWIDTH;
const uint32_t Region::SIZE = REGIONSIZE;

const uint32_t SuperRegion::WIDTH = SUPERREGIONWIDTH;
const uint32_t SuperRegion::SIZE = SUPERREGIONSIZE;


Region::Region() 
  : cells(NULL), count(0)
{ 
  //for( unsigned int i=0; i<Region::SIZE; i++ )
  //cells[i].region = this;
}

Region::~Region()
{
  if(cells)
	 delete[] cells;
}


SuperRegion::SuperRegion( World* world, stg_point_int_t origin )
  : count(0), origin(origin), world(world)	 
{
  //static int srcount=0;
  //printf( "created SR number %d\n", ++srcount );
  
  // initialize the parent pointer for all my child regions
  for( unsigned int i=0; i<SuperRegion::SIZE; i++ )
	 regions[i].superregion = this;
}

SuperRegion::~SuperRegion()
{
  //printf( "deleting SR %p at [%d,%d]\n", this, origin.x, origin.y );
}

void SuperRegion::Draw( bool drawall )
{
  glEnable( GL_DEPTH_TEST );

  glPushMatrix();
  glTranslatef( origin.x<<SRBITS, origin.y<<SRBITS,0);

  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  
  // outline superregion
  glColor3f( 0,0,1 );   
   glRecti( 0,0, 1<<SRBITS, 1<<SRBITS );

	// outline regions
  glColor3f( 0,1,0 );    
  for( unsigned int x=0; x<SuperRegion::WIDTH; x++ )
	 for( unsigned int y=0; y<SuperRegion::WIDTH; y++ )
		{
		  Region* r = GetRegion(x,y);

		  if( r->count )
			 // outline regions with contents
			 glRecti( x<<RBITS, y<<RBITS, 
						 (x+1)<<RBITS, (y+1)<<RBITS );
		  else if( r->cells )
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
		}

  char buf[32];
  snprintf( buf, 15, "%lu", count );
  Gl::draw_string( 1<<SBITS, 1<<SBITS, 0, buf );
    
  glColor3f( 1.0,0,0 );

  for( unsigned int x=0; x<SuperRegion::WIDTH; x++ )
	 for( unsigned int y=0; y<SuperRegion::WIDTH; y++ )
		{
		  Region* r = GetRegion(x,y);
		  
		  if( r->count < 1 )
			 continue;		  

		  snprintf( buf, 15, "%lu", r->count );
		  Gl::draw_string( x<<RBITS, y<<RBITS, 0, buf );
		  
		  for( unsigned int p=0; p<Region::WIDTH; p++ )
			 for( unsigned int q=0; q<Region::WIDTH; q++ )
				if( r->cells[p+(q*Region::WIDTH)].blocks.size() )
				  {					 
					 GLfloat xx = p+(x<<RBITS);
					 GLfloat yy = q+(y<<RBITS);
					 
					 if( ! drawall ) // draw a rectangle on the floor
						{
						  glRecti( xx, yy,
									  xx+1, yy+1);
						}
					 else // draw a rectangular solid
						{
						  Cell* c = &r->cells[p+(q*Region::WIDTH)];
						  for( std::list<Block*>::iterator it = c->blocks.begin();
								 it != c->blocks.end();
								 ++it )					 
							 {
								Block* block = *it;//(Block*)it->data;
						  
								//printf( "zb %.2f %.2f\n", ent->zbounds.min, ent->zbounds.max );
						  
								double r,g,b,a;
								stg_color_unpack( block->GetColor(), &r, &g, &b, &a );
								glColor4f( r,g,b, 1.0 );
						  
								glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
								glEnable(GL_POLYGON_OFFSET_FILL);
								// TODO - these numbers need tweaking for
								// better-looking rendering
								glPolygonOffset(0.01, 0.1);
						  
								// 						  // TOP
								glBegin( GL_POLYGON );
								glVertex3f( xx, yy, block->global_z.max );
								glVertex3f( 1+xx, yy, block->global_z.max );
								glVertex3f( 1+xx, 1+yy, block->global_z.max );
								glVertex3f( xx, 1+yy, block->global_z.max );
								glEnd();
						  
								// sides
								glBegin( GL_QUADS );
								glVertex3f( xx, yy, block->global_z.max );
								glVertex3f( xx, 1+yy, block->global_z.max );
								glVertex3f( xx, 1+yy, block->global_z.min );
								glVertex3f( xx, yy, block->global_z.min );
						  
								glVertex3f( 1+xx, yy, block->global_z.max );
								glVertex3f( xx, yy, block->global_z.max );
								glVertex3f( xx, yy, block->global_z.min );
								glVertex3f( 1+xx, yy, block->global_z.min );
						  
								glVertex3f( 1+xx, 1+yy, block->global_z.max );
								glVertex3f( 1+xx, yy, block->global_z.max );
								glVertex3f( 1+xx, yy, block->global_z.min );
								glVertex3f( 1+xx, 1+yy, block->global_z.min );
						  
								glVertex3f( xx, 1+yy, block->global_z.max );
								glVertex3f( 1+xx, 1+yy, block->global_z.max );
								glVertex3f( 1+xx, 1+yy, block->global_z.min );
								glVertex3f( xx, 1+yy, block->global_z.min );
								glEnd();
						  
								glDisable(GL_POLYGON_OFFSET_FILL);
						  
								glColor4f( r/2.0,g/2.0,b/2.0, 1.0-a );
								glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
						  
								// TOP
								glBegin( GL_POLYGON );
								glVertex3f( xx, yy, block->global_z.max );
								glVertex3f( 1+xx, yy, block->global_z.max );
								glVertex3f( 1+xx, 1+yy, block->global_z.max );
								glVertex3f( xx, 1+yy, block->global_z.max );
								glEnd();
						  
								// sides
								glBegin( GL_QUADS );
								glVertex3f( xx, yy, block->global_z.max );
								glVertex3f( xx, 1+yy, block->global_z.max );
								glVertex3f( xx, 1+yy, block->global_z.min );
								glVertex3f( xx, yy, block->global_z.min );
						  
								glVertex3f( 1+xx, yy, block->global_z.max );
								glVertex3f( xx, yy, block->global_z.max );
								glVertex3f( xx, yy, block->global_z.min );
								glVertex3f( 1+xx, yy, block->global_z.min );
						  
								glVertex3f( 1+xx, 1+yy, block->global_z.max );
								glVertex3f( 1+xx, yy, block->global_z.max );
								glVertex3f( 1+xx, yy, block->global_z.min );
								glVertex3f( 1+xx, 1+yy, block->global_z.min );
						  
								glVertex3f( xx, 1+yy, block->global_z.max );
								glVertex3f( 1+xx, 1+yy, block->global_z.max );
								glVertex3f( 1+xx, 1+yy, block->global_z.min );
								glVertex3f( xx, 1+yy, block->global_z.min );
								glEnd();
							 }
						}
				  }
		}
    glPopMatrix();    
}


//inline
void SuperRegion::Floor()
{
  glPushMatrix();
  glTranslatef( origin.x<<SRBITS, origin.y<<SRBITS, 0 );        
  glRecti( 0,0, 1<<SRBITS, 1<<SRBITS );
  glPopMatrix();    
}


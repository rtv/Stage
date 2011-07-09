/*
  region.cc
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

#include <pthread.h>
#include "region.hh"
using namespace Stg;

std::vector<Stg::Cell*> Region::dead_pool;

Region::Region() : 
  cells(NULL), 
  count(0),
  superregion(NULL)
{
}

Region::~Region()
{
  if( cells )
    delete[] cells;
}

void Region::AddBlock()
{ 
  ++count; 
  assert(count>0);
  superregion->AddBlock();
}

void Region::RemoveBlock()
{
  --count; 
  assert(count>=0); 
  superregion->RemoveBlock();
	
  // if there's nothing in this region, we can garbage collect the
  // cells to keep memory usage under control
  if( count == 0 )
    {
      if( cells )
	{
	  // stash this on the pool for reuse
	  dead_pool.push_back(cells);					
	  //printf( "retiring cells @ %p (pool %u)\n", cells, dead_pool.size() );
	  cells = NULL;
	}
      else
	PRINT_ERR( "region.count == 0 but cells == NULL" );			
    }		
}

SuperRegion::SuperRegion( World* world, point_int_t origin ) 
  : count(0),
    origin(origin), 
    regions(),
    world(world)
{
  for( int32_t c=0; c<SUPERREGIONSIZE;++c)
    regions[c].superregion = this;
}

SuperRegion::~SuperRegion()
{
}


void SuperRegion::AddBlock()
{ 
  ++count; 
  assert(count>0);
}

void SuperRegion::RemoveBlock()
{
  --count; 
  assert(count>=0); 
}		


void SuperRegion::DrawOccupancy( unsigned int layer ) const
{
  //printf( "SR origin (%d,%d) this %p\n", origin.x, origin.y, this );

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
  if( regions )
    {
      const Region* r = &regions[0];
      char buf[32];
		
      switch( layer )
	{
	case 0: // moveable 1
	  glColor3f( 0,1,0 );    
	  break;
	case 1: // moveable 2
	  glTranslatef( 0.05, 0.05, 0);
	  glColor3f( 0,0,1 );    
	  break;
	  // 		  case 2: // fixed 
	  // 			 glTranslatef( 0.1, 0.1, 0);
	  // 			 glColor3f( 1,0,0 );    
	  // 			 break;
	default:
	  PRINT_ERR1( "error: wrong layer %d", layer );			 
	}
		
      for( int y=0; y<SUPERREGIONWIDTH; ++y )
	for( int x=0; x<SUPERREGIONWIDTH; ++x )
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
		for( int p=0; p<REGIONWIDTH; ++p )
		  for( int q=0; q<REGIONWIDTH; ++q )
		    if( r->cells[p+(q*REGIONWIDTH)].blocks[layer].size() )
		      {					 
			GLfloat xx = p+(x<<RBITS);
			GLfloat yy = q+(y<<RBITS);						  
			glRecti( xx, yy, xx+1, yy+1);
		      }
	      }
	    /*				else if( r->cells ) // empty but used previously 
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
	    */
	    ++r; // next region quickly
	  }			
    }
  else
    {  // outline region-collected superregion
      glColor3f( 1,1,0 );   
      glRecti( 0,0, (1<<SRBITS)-1, (1<<SRBITS)-1 );
      glColor3f( 0,0,1 );   
    }
  
  char buf[32];
  snprintf( buf, 15, "%lu", count );
  Gl::draw_string( 1<<SBITS, 1<<SBITS, 0, buf );
  
  glPopMatrix();    
}


static inline void DrawBlock( GLfloat x, GLfloat y, GLfloat zmin, GLfloat zmax )
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

void SuperRegion::DrawVoxels(unsigned int layer) const
{
  glPushMatrix();	    
  GLfloat scale = 1.0/world->Resolution();
  glScalef( scale, scale, 1.0 ); // XX TODO - this seems slightly
  glTranslatef( origin.x<<SRBITS, origin.y<<SRBITS,0);


  glEnable( GL_DEPTH_TEST );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  
  const Region* r = &regions[0];
  
  for( int y=0; y<SUPERREGIONWIDTH; ++y )
    for( int x=0; x<SUPERREGIONWIDTH; ++x )
      {		  
	if( r->count ) // not an empty region
	  for( int p=0; p<REGIONWIDTH; ++p )
	    for( int q=0; q<REGIONWIDTH; ++q )
	      {
		const std::vector<Block*>& blocks = 
		  r->cells[p+(q*REGIONWIDTH)].blocks[layer];
					 
		if( blocks.size() ) // not an empty cell
		  {					 
		    const GLfloat xx(p+(x<<RBITS));
		    const GLfloat yy(q+(y<<RBITS));
						  
		    FOR_EACH( it, blocks )
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

void Stg::Cell::AddBlock( Block* b, unsigned int layer )
{			
  blocks[layer].push_back( b );   
  b->rendered_cells[layer].push_back(this);
  region->AddBlock();
}

void Stg::Cell::RemoveBlock( Block* b, unsigned int layer )
{
  std::vector<Block*>& blks( blocks[layer] );
  const size_t len( blks.size() );
  if( len )
    {
#if 1	
      // Use conventional STL style		
		
      // this special-case test is faster for worlds with simple models,
      // which are the ones we want to be really fast. It's a small
      // extra cost for worlds with several models in each cell. It
      // gives a 5% overall speed increase in fasr.world.
		
      if( (blks.size() == 1) &&
	  (blks[0] == b) ) // special but common case
	{
	  blks.clear(); // cheap
	}
      else // the general but relatively expensive case
	{
	  EraseAll( b, blks );		
	}
#else	
      // attempt faster removal loop
      // O(n) * low constant array element removal
      // this C-style pointer work looks to be very slightly faster than the STL way
      Block **start = &blks[0]; // start of array
      Block **r     = &blks[0]; // read from here
      Block **w     = &blks[0]; // write to here
      
      while( r < start + len ) // scan down array, skipping 'this' 
	{
	  if( *r != b ) 
	    *w++ = *r;				
	  ++r;
	}
      blks.resize( w-start );
#endif
    }
  
  region->RemoveBlock();
}

/*
  region.cc
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

#include "stage_internal.hh"

const uint32_t Region::WIDTH = REGIONWIDTH;
const uint32_t Region::SIZE = REGIONSIZE;

const uint32_t SuperRegion::WIDTH = SUPERREGIONWIDTH;
const uint32_t SuperRegion::SIZE = SUPERREGIONSIZE;

//inline
void Cell::RemoveBlock( StgBlock* b )
{
  // linear time removal, but these lists should be very short.
  list = g_slist_remove( list, b );
  
  assert( region );
  region->DecrementOccupancy();
}

void Cell::AddBlock( StgBlock* b )
{
  // constant time prepend
  list = g_slist_prepend( list, b );

  region->IncrementOccupancy();
  b->RecordRendering( this );
}

void Cell::AddBlockNoRecord( StgBlock* b )
{
  list = g_slist_prepend( list, b );

  // don't add this cell to the block - we assume it's already there
}


Region::Region() 
  : count(0)
{ 
  for( int i=0; i<Region::SIZE; i++ )
	 cells[i].region = this;
}

Region::~Region()
{
  delete[] cells;
}

inline void Region::DecrementOccupancy()
{ 
  assert( superregion );
  superregion->DecrementOccupancy();
  --count; 
}

inline void Region::IncrementOccupancy()
{ 
  assert( superregion );
  superregion->IncrementOccupancy();
  ++count; 
}

Cell* Region::GetCell( int32_t x, int32_t y )
{
  uint32_t index = x + (y*Region::WIDTH);
  assert( x < Region::WIDTH );
  assert( index >=0 );
  assert( index < Region::SIZE );
  return &cells[index];    
}


SuperRegion::SuperRegion( int32_t x, int32_t y )
  : count(0)
{
  origin.x = x;
  origin.y = y;

  // initialize the parent pointer for all my child regions
  for( int i=0; i<SuperRegion::SIZE; i++ )
	 regions[i].superregion = this;
}

SuperRegion::~SuperRegion()
{
  // nothing to do
}

// get the region x,y from the region array
Region* SuperRegion::GetRegion( int32_t x, int32_t y )
{
  int32_t index =  x + (y*SuperRegion::WIDTH);
  assert( index >=0 );
  assert( index < (int)SuperRegion::SIZE );
  return &regions[ index ];
} 

inline void SuperRegion::DecrementOccupancy()
{ 
  --count; 
}

inline void SuperRegion::IncrementOccupancy()
{ 
  ++count; 
}

void SuperRegion::Draw( bool drawall )
{
  glEnable( GL_DEPTH_TEST );

  glPushMatrix();
  glTranslatef( origin.x<<SRBITS, origin.y<<SRBITS,0);

  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  
  // outline regions
  glColor3f( 0,1,0 );    
  for( unsigned int x=0; x<SuperRegion::WIDTH; x++ )
	 for( unsigned int y=0; y<SuperRegion::WIDTH; y++ )
 		glRecti( x<<RBITS, y<<RBITS, 
 					(x+1)<<RBITS, (y+1)<<RBITS );
  
  // outline superregion
  glColor3f( 0,0,1 );    
  glRecti( 0,0, 1<<SRBITS, 1<<SRBITS );
  
  for( unsigned int x=0; x<SuperRegion::WIDTH; x++ )
	 for( unsigned int y=0; y<SuperRegion::WIDTH; y++ )
		{
		  Region* r = GetRegion(x,y);
		  
		  if( r->count < 1 )
			 continue;		  

		  char buf[16];
		  snprintf( buf, 15, "%lu", r->count );
		  gl_draw_string( x<<RBITS, y<<RBITS, 0, buf );

		  
		  for( unsigned int p=0; p<Region::WIDTH; p++ )
			 for( unsigned int q=0; q<Region::WIDTH; q++ )
				if( r->cells[p+(q*Region::WIDTH)].list )
				  {					 
					 GLfloat xx = p+(x<<RBITS);
					 GLfloat yy = q+(y<<RBITS);
					 
					 if( ! drawall ) // draw a rectangle on the floor
						{
						  glColor3f( 1.0,0,0 );
						  glRecti( xx, yy,
									  xx+1, yy+1);
						}
					 else // draw a rectangular solid
						{
						  for( GSList* it =  r->cells[p+(q*Region::WIDTH)].list;
								 it;
								 it=it->next )
							 {
								StgBlock* block = (StgBlock*)it->data;
						  
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

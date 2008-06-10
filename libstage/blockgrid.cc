#include "stage_internal.hh"

// todo - make these configurable

static const uint32_t NBITS = 5;
static const uint32_t NSIZE = 1<<NBITS;
static const uint32_t NSQR = NSIZE*NSIZE;
static const uint32_t MASK = NSIZE-1;

StgBlockGrid::StgBlockGrid( uint32_t width, uint32_t height )
{
  this->width = width;
  this->height = height;
  this->cells = (GSList**)calloc( sizeof(GList*), width * height );

  assert(this->cells);
}    

StgBlockGrid::~StgBlockGrid()
{
  for( uint32_t x=0; x<width; x++ )
    for( uint32_t y=0; y<height; y++ )
      {
	if( cells[x + y*width] )
	  g_slist_free( cells[x+y*width] );
      }	
  
  delete[] cells;
  //delete[] map;
}

void StgBlockGrid::AddBlock( uint32_t x, uint32_t y, StgBlock* block )
{
  //printf( "add block %u %u\n", x, y );

  if( x < width && y < height )
    {  
      cells[ x+y*width ] = g_slist_prepend( cells[ x+y*width ], block );
      block->RecordRenderPoint( x, y );
    }
}

void StgBlockGrid::RemoveBlock( uint32_t x, uint32_t y, StgBlock* block )
{
  //printf( "remove block %u %u\n", x, y );
  
  if( x < width && y < height )
    {
      cells[ x+y*width ] = g_slist_remove( cells[ x+y*width ], block );
    }
}

GSList* StgBlockGrid::GetList( uint32_t x, uint32_t y )
{
  if( x < width && y < height )
    return cells[ x+y*width ];
  else
    return NULL;
}

void StgBlockGrid::GlobalRemoveBlock( StgBlock* block )
{
  for( uint32_t x=0; x<width; x++ )
    for( uint32_t y=0; y<height; y++ )
      RemoveBlock(x,y,block );
}

void StgBlockGrid::Draw( bool drawall )
{
  for( uint32_t x=0; x<width; x++ )
    for( uint32_t y=0; y<height; y++ )
      {
	//stg_bigblock_t* bb = &map[ x + y*bwidth ];
		
	//if( drawall || bb->lists )
// 	  {
// 	    glRecti( x<<numbits,y<<numbits,(x+1)<<numbits,(y+1)<<numbits );
	    
// 	    if( bb->lists )
// 	      for( uint32_t a=0; a<NSQR; a++ )
// 		{		  
// 		  if( drawall || bb->lists[ a ] )		  
// 		    glRecti( (x<<numbits)+a%NSIZE,
// 			     (y<<numbits)+a/NSIZE,
// 			     (x<<numbits)+a%NSIZE+1,
// 			     (y<<numbits)+a/NSIZE+1 );		
		  
//  		}
// 	  }	

	if( cells[x+y*width] )
	  glRecti( x,y, x+1, y+ 1 );
	  
      }
} 


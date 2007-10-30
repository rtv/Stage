#include "stage.hh"

#define NBITS 4
#define NSIZE (1<<NBITS)
#define MASK (NSIZE-1)


StgBlockGrid::StgBlockGrid( uint32_t width, uint32_t height )
{
  this->width = width;
  this->height = height;
  
  this->bwidth = width >> NBITS;
  this->bheight = height >> NBITS;
  
  printf( "Creating StgBlockGrid of [%u,%u](%u,%u) elements\n", 
	  width, height, bwidth, bheight );
  
  
  size_t bits = bwidth * bheight;
  this->map = new stg_bigblock_t[ bits ];
  bzero( map, sizeof(stg_bigblock_t)*bits );
}    

StgBlockGrid::~StgBlockGrid()
{
  //delete[] lists;
  delete[] map;
}

void StgBlockGrid::AddBlock( uint32_t x, uint32_t y, StgBlock* block )
{
  if( x < width && y < height )
    {  
      uint32_t a = x>>NBITS;
      uint32_t b = y>>NBITS;

      stg_bigblock_t* bb = &map[ a + b*bwidth ];
      assert(bb); // it really should be there

      //printf( "incrementing block at %u %u (width %u height %u\n",
      //      a,b, bwidth, bheight );

      if( bb->lists == NULL  )
 	{	  
	  assert( bb->lists == NULL );
	  bb->lists = new GSList*[ NSIZE*NSIZE ]; 
	  bzero( bb->lists, NSIZE*NSIZE * sizeof(GSList*));
 	}
      
      bb->counter++;

      // use only the NBITS least significant bits of the address
      uint32_t index = (x&MASK) + (y&MASK)*NSIZE;     
      bb->lists[index] = g_slist_prepend( bb->lists[index], block );

      block->RecordRenderPoint( x, y );
    }
}


GSList* StgBlockGrid::GetList( uint32_t x, uint32_t y )
{
  if( x < width && y < height )
    {  
      uint32_t a = x>>NBITS;
      uint32_t b = y>>NBITS;
      
      stg_bigblock_t* bb = &map[ a + b*bwidth ];      

      if( bb->lists )
	{      
	  uint32_t index = (x&MASK) + (y&MASK)*NSIZE;     
	  return bb->lists[index];
	}
    }
  return NULL;
}

void StgBlockGrid::RemoveBlock( uint32_t x, uint32_t y, StgBlock* block )
{
  //printf( "remove block %u %u\n", x, y );
  
  if( x < width && y < height )
    {
      uint32_t a = x>>NBITS;
      uint32_t b = y>>NBITS;
      
      stg_bigblock_t* bb = &map[ a + b*bwidth ];      
      assert( bb ); // it really should be there

      // remove list entry
      uint32_t index = (x&MASK) + (y&MASK)*NSIZE;     
      assert( bb->lists[index] );
      bb->lists[index] = g_slist_remove( bb->lists[index], block );

      bb->counter--;
      
      if( bb->counter == 0 ) // this was the last entry in this block;
 	{
 	  delete[] bb->lists; 
 	  bb->lists = NULL;
 	}
    }
}

void StgBlockGrid::RemoveBlock( StgBlock* block )
{
  // todo - speed this up a great deal
  for( uint32_t x=0; x<width; x++ )
    for( uint32_t y=0; y<height; y++ )
      RemoveBlock(x,y,block );    
}

void StgBlockGrid::Draw( bool drawall )
{
  for( uint32_t x=0; x<bwidth; x++ )
    for( uint32_t y=0; y<bheight; y++ )
      {
	stg_bigblock_t* bb = &map[ x + y*bwidth ];
		
	if( drawall || bb->lists )
	  {
	    glRecti( x<<NBITS,y<<NBITS,(x+1)<<NBITS,(y+1)<<NBITS );
	    
	    for( uint32_t a=0; a<NSIZE*NSIZE; a++ )
	      {		  
		if( drawall || bb->lists[ a ] )		  
		  glRecti( (x<<NBITS)+a%NSIZE,
			   (y<<NBITS)+a/NSIZE,
			   (x<<NBITS)+a%NSIZE+1,
			   (y<<NBITS)+a/NSIZE+1 );		
		
 		}
	  }	
      }
} 


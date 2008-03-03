#include "stage_internal.hh"

StgFlag::StgFlag( stg_color_t color, double size )
{ 
  this->color = color; 
  this->size = size;
}

StgFlag* StgFlag::Nibble( double chunk )
{
  StgFlag* piece = NULL;

  if( size > 0 )
    {
      chunk = MIN( chunk, this->size );
      piece = new StgFlag( this->color, chunk );
      this->size -= chunk;
    }

  return piece;
}

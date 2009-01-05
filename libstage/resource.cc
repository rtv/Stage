#include "stage_internal.hh"

Flag::Flag( stg_color_t color, double size )
{ 
	this->color = color;
	this->size = size;
}

Flag* Flag::Nibble( double chunk )
{
	Flag* piece = NULL;

	if( size > 0 )
	{
		chunk = MIN( chunk, this->size );
		piece = new Flag( this->color, chunk );
		this->size -= chunk;
	}

	return piece;
}

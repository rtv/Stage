#include "stage.hh"
using namespace Stg;

Flag::Flag( Color color, double size )
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

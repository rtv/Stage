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


void Flag::Draw(  GLUquadric* quadric )
{
  glColor4f( color.r, color.g, color.b, color.a );
    
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  gluQuadricDrawStyle( quadric, GLU_FILL );
  gluSphere( quadric, size/2.0, 4,2  );
  glDisable(GL_POLYGON_OFFSET_FILL);
  
  // draw the edges darker version of the same color
  glColor4f( color.r/2.0, color.g/2.0, color.b/2.0, color.a/2.0 );
    
  gluQuadricDrawStyle( quadric, GLU_LINE );
  gluSphere( quadric, size/2.0, 4,2 );
}

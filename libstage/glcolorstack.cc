#include "stage_internal.hh"

GlColorStack::GlColorStack()
{
	colorstack = g_queue_new();
}

GlColorStack::~GlColorStack()
{

}

void GlColorStack::Push( GLdouble col[4] )
{
	size_t sz =  4 * sizeof(col[0]);
	GLdouble *keep = (GLdouble*)malloc( sz );
	memcpy( keep, col, sz );

	g_queue_push_head( colorstack, keep );

	// and select this color in GL
	glColor4dv( col );
}


void GlColorStack::Push( double r, double g, double b )
{
	GLdouble col[4];
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = 1.0;

	Push( col );
}

// a convenience wrapper around push_color()
void GlColorStack::Push( double r, double g, double b, double a )
{
	GLdouble col[4];
	col[0] = r;
	col[1] = g;
	col[2] = b;
	col[3] = a;

	Push( col );
}

void GlColorStack::Push( stg_color_t col )
{
	GLdouble d[4];

	d[0] = ((col & 0x00FF0000) >> 16) / 256.0;
	d[1] = ((col & 0x0000FF00) >> 8)  / 256.0;
	d[2] = ((col & 0x000000FF) >> 0)  / 256.0;
	d[3] = (((col & 0xFF000000) >> 24) / 256.0);

	Push( d );
}


// reset GL to the color we stored using store_color()
void GlColorStack::Pop( void )
{
	if( g_queue_is_empty( colorstack ) )
		PRINT_WARN1( "Attempted to ColorStack.Pop() but ColorStack %p is empty",
				this );
	else
	{
		GLdouble *col = (GLdouble*)g_queue_pop_head( colorstack );
		glColor4dv( col );
		free( col );
	}
}

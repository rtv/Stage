#include "stage_internal.h"

#include <GL/gl.h>

/** Create a new block. A model's body is a list of these
    blocks. The point data is copied, so pts can safely be freed
    after calling this.*/
stg_block_t* stg_block_create( double x, double y, double z, double a,
			       double height,
			       stg_color_t color,
			       stg_point_t* pts, 
			       size_t pt_count )
{
  //printf( "Creating block with %d points\n", (int)pt_count );
  
  stg_block_t* block = g_new( stg_block_t, 1 );

  block->pose.x = x;
  block->pose.y = y;
  block->pose.a = a;
  block->height = height;
  memcpy( &block->color, &color, sizeof(stg_color_t) );
  block->pt_count = pt_count;
  block->pts = g_memdup( pts, pt_count * sizeof(stg_point_t));

  // todo - build an OpenGL displaylist to draw this object

  block->display_list = glGenLists( 1 );
  
  // generates a displaylist for this block
  stg_block_update( block );

  return block;
}

/** destroy a block, freeing all memory */
void stg_block_destroy( stg_block_t* block )
{
  assert( block );
  assert( block->pts );

  glDeleteLists( block->display_list, 1 );
  g_free( block->pts );
  g_free( block );
}
  
void stg_block_render( stg_block_t* b )
{
  //printf( "rendering block @ %p with %d points\n", b, (int)b->pt_count );
  glCallList( b->display_list );
}

static void block_top( stg_block_t* b )
{
  // draw a top that fits over the stripa
  glBegin(GL_POLYGON);
  int p;
  for( p=0; p<b->pt_count; p++)
    glVertex3f( b->pts[p].x, b->pts[p].y, 1.0 );//b->height );
  glEnd();
}

static void block_sides( stg_block_t* b )
{
  // construct a strip that wraps around the polygon
  glBegin(GL_QUAD_STRIP);
  int p;
  for( p=0; p<b->pt_count; p++)
    {
      glVertex3f( b->pts[p].x, b->pts[p].y, 1.0 );//b->height );
      glVertex3f( b->pts[p].x, b->pts[p].y, 0 );
    }
  // close the strip
  glVertex3f( b->pts[0].x, b->pts[0].y, 1.0 );//>height );
  glVertex3f( b->pts[0].x, b->pts[0].y, 0 );
  glEnd();
}

void stg_block_update( stg_block_t* b )
{ 
  glNewList( b->display_list, GL_COMPILE );

  glPushMatrix();
  // move into the block's local CS
  glTranslatef( b->pose.x, b->pose.y, b->pose.z );
  glRotatef( RTOD(b->pose.a), 0,0,1 );

  // draw filled color polygons
  double gcol[4];	  
  stg_color_to_glcolor4dv( b->color, gcol );


  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
  push_color( gcol );
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  block_top( b );
  block_sides( b );
  glDisable(GL_POLYGON_OFFSET_FILL);
  
  pop_color();

  // draw the block outline in a darker version of the same color
  gcol[0]/=2.0;
  gcol[1]/=2.0;
  gcol[2]/=2.0;  
  push_color( gcol );
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
  block_top( b );
  block_sides( b );  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
  pop_color();


  // move back into parent's CS
  glPopMatrix();
  
  glEndList();
}

void stg_block_list_scale( GList* blocks, 
			   double width, 
			   double height )
{
  if( g_list_length( blocks ) < 1 )
    return;

  // assuming the blocks currently fit in a square +/- one billion units
  double minx, miny, maxx, maxy;
  minx = miny = BILLION;
  maxx = maxy = -BILLION;
  
  GList* it;
  for( it=blocks; it; it=it->next ) // examine all the blocks
    {
      // examine all the points in the polygon
      stg_block_t* block = (stg_block_t*)it->data;
      
      int p;
      for( p=0; p < block->pt_count; p++ )
	{
	  stg_point_t* pt = &block->pts[p];
	  if( pt->x < minx ) minx = pt->x;
	  if( pt->y < miny ) miny = pt->y;
	  if( pt->x > maxx ) maxx = pt->x;
	  if( pt->y > maxy ) maxy = pt->y;

	  assert( ! isnan( pt->x ) );
	  assert( ! isnan( pt->y ) );
	}
    }

  // now normalize all lengths so that the lines all fit inside
  // the specified rectangle
  double scalex = (maxx - minx);
  double scaley = (maxy - miny);

  for( it=blocks; it; it=it->next ) // examine all the blocks again
    { 
      stg_block_t* block = (stg_block_t*)it->data;
      
      // scale all the points in the block
      // TODO - scaling data in the block instead?
      int p;
      for( p=0; p < block->pt_count; p++ )
	{
	  stg_point_t* pt = &block->pts[p];
	  
	  pt->x = ((pt->x - minx) / scalex * width) - width/2.0;
	  pt->y = ((pt->y - miny) / scaley * height) - height/2.0;

	  assert( ! isnan( pt->x ) );
	  assert( ! isnan( pt->y ) );
	}

      // recalculate the GL drawlist
      stg_block_update( block );
    }
}

#include "stage_internal.h"
#include "model.hh"

#include <GL/gl.h>

/** Create a new block. A model's body is a list of these
    blocks. The point data is copied, so pts can safely be freed
    after calling this.*/
stg_block_t* stg_block_create( StgModel* mod,
			       stg_point_t* pts, 
			       size_t pt_count,
			       stg_meters_t zmin,
			       stg_meters_t zmax,
			       stg_color_t color,
			       bool inherit_color )
{
  //printf( "Creating block with %d points\n", (int)pt_count );
  
  stg_block_t* block = g_new( stg_block_t, 1 );
  
  block->mod = mod;
  block->pt_count = pt_count;
  block->pts = (stg_point_t*)g_memdup( pts, pt_count * sizeof(stg_point_t));
  block->zmin = zmin;
  block->zmax = zmax;
  block->color = color;
  block->inherit_color = inherit_color;

  // generate an OpenGL displaylist for this block
  block->display_list = glGenLists( 1 );  
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
 
void stg_block_list_destroy( GList* list )
{
  GList* it;
  for( it=list; it; it=it->next )
      stg_block_destroy( (stg_block_t*)it->data );
  g_list_free( list );
}
 
void stg_block_render( stg_block_t* b )
{
  //printf( "rendering block @ %p with %d points\n", b, (int)b->pt_count );
  glCallList( b->display_list );
}

static void block_top( stg_block_t* b )
{
  //stg_geom_t geom;      
  //b->mod->GetGeom( &geom );

  // draw a top that fits over the stripa
  glBegin(GL_POLYGON);
  for( unsigned int p=0; p<b->pt_count; p++)
    {
    glVertex3f( b->pts[p].x, b->pts[p].y, b->zmax );
    }
  glEnd();
}

static void block_sides( stg_block_t* b )
{
  //stg_geom_t geom;      
  //b->mod->GetGeom( &geom );
  
  // construct a strip that wraps around the polygon
  glBegin(GL_QUAD_STRIP);
  for( unsigned int p=0; p<b->pt_count; p++)
    {
      glVertex3f( b->pts[p].x, b->pts[p].y, b->zmax );
      glVertex3f( b->pts[p].x, b->pts[p].y, b->zmin );
    }
  // close the strip
  glVertex3f( b->pts[0].x, b->pts[0].y, b->zmax );
  glVertex3f( b->pts[0].x, b->pts[0].y, b->zmin );
  glEnd();
}

void stg_block_update( stg_block_t* b )
{ 
  // draw filled color polygons

  glNewList( b->display_list, GL_COMPILE );

  stg_color_t color;
  if( b->inherit_color )
    b->mod->GetColor( &color );
  else
    color = b->color;
    
  double gcol[4];	  
  stg_color_to_glcolor4dv( color, gcol );

  //glDisable(GL_BLEND);
  //glDisable(GL_LINE_SMOOTH);
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
  push_color( gcol );
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  block_sides( b );
  block_top( b );
  glDisable(GL_POLYGON_OFFSET_FILL);
  

  // draw the block outline in a darker version of the same color
  gcol[0]/=2.0;
  gcol[1]/=2.0;
  gcol[2]/=2.0;  
  push_color( gcol );

  glLineWidth( 0 );
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
  block_top( b );
  block_sides( b );  

  //glEnable(GL_BLEND);
  //glEnable(GL_LINE_SMOOTH);

  pop_color();
  pop_color();

  glEndList();
}

void stg_block_list_scale( GList* blocks, 
			   stg_size_t* size )
{
  if( g_list_length( blocks ) < 1 )
    return;

  // assuming the blocks currently fit in a square +/- one billion units
  double minx, miny, maxx, maxy; 
  minx = miny =  BILLION;
  maxx = maxy = -BILLION;

  double maxz = 0;
  
  GList* it;
  for( it=blocks; it; it=it->next ) // examine all the blocks
    {
      // examine all the points in the polygon
      stg_block_t* block = (stg_block_t*)it->data;
      
      for( unsigned int p=0; p < block->pt_count; p++ )
	{
	  stg_point_t* pt = &block->pts[p];
	  if( pt->x < minx ) minx = pt->x;
	  if( pt->y < miny ) miny = pt->y;
	  if( pt->x > maxx ) maxx = pt->x;
	  if( pt->y > maxy ) maxy = pt->y;

	  assert( ! isnan( pt->x ) );
	  assert( ! isnan( pt->y ) );
	}      

      
      if( block->zmax > maxz )
	maxz = block->zmax;
    }

  // now normalize all lengths so that the lines all fit inside
  // the specified rectangle
  double scalex = (maxx - minx);
  double scaley = (maxy - miny);

  double scalez = size->z / maxz;

  for( it=blocks; it; it=it->next ) // examine all the blocks again
    { 
      stg_block_t* block = (stg_block_t*)it->data;
      
      // scale all the points in the block
      // TODO - scaling data in the block instead?
      for( unsigned int p=0; p < block->pt_count; p++ )
	{
	  stg_point_t* pt = &block->pts[p];
	  
	  pt->x = ((pt->x - minx) / scalex * size->x) - size->x/2.0;
	  pt->y = ((pt->y - miny) / scaley * size->y) - size->y/2.0;

	  assert( ! isnan( pt->x ) );
	  assert( ! isnan( pt->y ) );
	}

      // todo - scale min properly
      block->zmax *= scalez;
      block->zmin *= scalez;

      // recalculate the GL drawlist
      stg_block_update( block );
    }
}

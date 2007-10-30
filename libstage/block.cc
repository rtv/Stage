#include <GL/gl.h>

//#define DEBUG 1

#include "stage.hh"


/** Create a new block. A model's body is a list of these
    blocks. The point data is copied, so pts can safely be freed
    after calling this.*/
StgBlock::StgBlock( StgModel* mod,
		    stg_point_t* pts, 
		    size_t pt_count,
		    stg_meters_t zmin,
		    stg_meters_t zmax,
		    stg_color_t color,
		    bool inherit_color )
{ 
  this->mod = mod;
  this->pt_count = pt_count;
  this->pts = (stg_point_t*)g_memdup( pts, pt_count * sizeof(stg_point_t));
  //this->pts_global = (stg_point_t*)g_memdup( pts, pt_count * sizeof(stg_point_t));
  // allocate space for the integer version of the block vertices
  this->pts_global = new stg_point_int_t[pt_count];
  this->zmin = zmin;
  this->zmax = zmax;
  this->color = color;
  this->inherit_color = inherit_color;
  this->rendered_points = g_array_new( FALSE, FALSE, sizeof(stg_point_int_t));
}

StgBlock::~StgBlock()
{ 
  this->UnMap();
  g_free( pts );
  g_array_free( rendered_points, TRUE );
}

void stg_block_list_destroy( GList* list )
{
  GList* it;
  for( it=list; it; it=it->next )
    delete (StgBlock*)it->data;
  g_list_free( list );
}


void StgBlock::DrawTop()
{
  // draw a top that fits over the side strip
  glBegin(GL_POLYGON);

  for( unsigned int p=0; p<pt_count; p++ )
    {
      glVertex3f( pts[p].x, pts[p].y, zmax );
    }
  
  glEnd();
}       

void StgBlock::DrawSides()
{
  // construct a strip that wraps around the polygon
  
  glBegin(GL_QUAD_STRIP);
  for( unsigned int p=0; p<pt_count; p++)
    {
      glVertex3f( pts[p].x, pts[p].y, zmax );
      glVertex3f( pts[p].x, pts[p].y, zmin );
    }
  // close the strip
  glVertex3f( pts[0].x, pts[0].y, zmax );
  glVertex3f( pts[0].x, pts[0].y, zmin );
  glEnd();
}


void StgBlock::Draw()
{
  // draw filled color polygons
  
  stg_color_t color;
  if( inherit_color )
    mod->GetColor( &color );
  else
    color = color;

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
  PushColor( color );
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  DrawSides();
  DrawTop();
  glDisable(GL_POLYGON_OFFSET_FILL);
  
  // draw the block outline in a darker version of the same color
  double r,g,b,a;
  stg_color_unpack( color, &r, &g, &b, &a );  
  PushColor( stg_color_pack( r/2.0, g/2.0, b/2.0, a ));

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
  glDepthMask(GL_FALSE); 
  DrawTop();
  DrawSides();  
  glDepthMask(GL_TRUE); 
  
  PopColor();
  PopColor();
}


void StgBlock::Map()
{
  PRINT_DEBUG2( "%s mapping block with %d points", 
	       mod->Token(),
	       (int)pt_count );

  double ppm = mod->world->ppm;
 
  // update the global coordinate list
  stg_pose_t gpose;
  for( unsigned int p=0; p<pt_count; p++ )
    {
      gpose.x = pts[p].x;
      gpose.y = pts[p].y;
      //gpose.z = pts[p].z;
      gpose.a = 0.0;
      
      mod->LocalToGlobal( &gpose );
            
      pts_global[p].x = (int32_t)floor((gpose.x+mod->world->width/2.0)*ppm);
      pts_global[p].y = (int32_t)floor((gpose.y+mod->world->height/2.0)*ppm);
      //pts_global[p].z = gpose.z;

      PRINT_DEBUG2("loc [%.2f %.2f]", 
		   pts[p].x,
 		   pts[p].y );
      
      PRINT_DEBUG2("glb [%d %d]", 
		   pts_global[p].x,
 		   pts_global[p].y );
    }
  
  mod->world->MapBlock( this );
}

void StgBlock::UnMap()
{
  PRINT_DEBUG2( "UnMapping block of model %s with %d points", 
		mod->Token(),
		(int)pt_count);
  
  for( unsigned int p=0; p<rendered_points->len; p++ )    
    {
      stg_point_int_t* pt = 
	&g_array_index( rendered_points, stg_point_int_t, p);
      
      mod->world->bgrid->RemoveBlock( pt->x, pt->y, this );

    }

  // forget the points we have unrendered (but keep their storage)
  g_array_set_size( rendered_points,0 );
}  

void StgBlock::RecordRenderPoint( uint32_t x, uint32_t y )
{
  // store this index in the block for later fast deletion
  stg_point_int_t pt;
  pt.x = x;
  pt.y = y;
  g_array_append_val( rendered_points, pt ); 
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
      StgBlock* block = (StgBlock*)it->data;
     
      block->UnMap();
 
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
      StgBlock* block = (StgBlock*)it->data;
      
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

      //block->Map();
    }
}


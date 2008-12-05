
#include "stage_internal.hh"
#include <libgen.h> // for dirname(3)

#undef DEBUG

using namespace Stg;

BlockGroup::BlockGroup() 
  : displaylist(0),
	 blocks(NULL), 
	 count(0), 
	 minx(0),
	 maxx(0),
	 miny(0),
	 maxy(0)
{ /* empty */ }

BlockGroup::~BlockGroup()
{	 
  Clear();
}

void BlockGroup::AppendBlock( StgBlock* block )
{
  blocks = g_list_append( blocks, block );
  ++count;

  block->mod->map_caches_are_invalid = true;
}

void BlockGroup::Clear()
{
  while( blocks )
	 {
		delete (StgBlock*)blocks->data;
		blocks = blocks->next;
	 }
  
  g_list_free( blocks );
  blocks = NULL;
}


void BlockGroup::SwitchToTestedCells()
{
  // confirm the tentative pose for all blocks
  LISTMETHOD( blocks, StgBlock*, SwitchToTestedCells );  
}

StgModel* BlockGroup::TestCollision()
{
  //printf( "blockgroup %p test collision...\n", this );

  StgModel* hitmod = NULL;
  
  for( GList* it=blocks; it; it = it->next )
	 if( (hitmod = ((StgBlock*)it->data)->TestCollision()))
		break; // bail on the earliest collision
  
  //printf( "blockgroup %p test collision done.\n", this );

  return hitmod;
}


// establish the min and max of all the blocks, so we can scale this
// group later
void BlockGroup::CalcSize()
{  
  // assuming the blocks currently fit in a square +/- one billion units
  //double minx, miny, maxx, maxy, minz, maxz;
  minx = miny =  billion;
  maxx = maxy = -billion;
  
  size.z = 0.0; // grow to largest z we see

  if( blocks )
	 ((StgBlock*)blocks->data)->mod->map_caches_are_invalid = true;
  
  for( GList* it=blocks; it; it=it->next ) // examine all the blocks
	 {
		// examine all the points in the polygon
		StgBlock* block = (StgBlock*)it->data;
		
		for( unsigned int p=0; p < block->pt_count; p++ )
		  {
			 stg_point_t* pt = &block->pts[p];
			 if( pt->x < minx ) minx = pt->x;
			 if( pt->y < miny ) miny = pt->y;
			 if( pt->x > maxx ) maxx = pt->x;
			 if( pt->y > maxy ) maxy = pt->y;
		  }
		
		size.z = MAX( block->local_z.max, size.z );
	 }
  
  // store these bounds for normalization purposes
  size.x = maxx-minx;
  size.y = maxy-miny;
  
  offset.x = minx + size.x/2.0;
  offset.y = miny + size.y/2.0;
  offset.z = 0; // todo?

  // normalize blocks
  //  for( GList* it = blocks; itl it=it->next )
  //((StgBlock*)it->data)->Normalize( size.x, size.y, size.z, offset.x
}


void BlockGroup::Map()
{
  LISTMETHOD( blocks, StgBlock*, Map );
}

void BlockGroup::UnMap()
{
  LISTMETHOD( blocks, StgBlock*, UnMap );
}

void BlockGroup::DrawSolid( const stg_geom_t & geom )
{
  glPushMatrix();

  gl_pose_shift( geom.pose );

  glScalef( geom.size.x / size.x,
				geom.size.y / size.y,				
				geom.size.z / size.z );
  
  glTranslatef( -offset.x, -offset.y, -offset.z );

  LISTMETHOD( blocks, StgBlock*, DrawSolid );

  glPopMatrix();
}

void BlockGroup::DrawFootPrint( const stg_geom_t & geom )
{
  glPushMatrix();
  
  glScalef( geom.size.x / size.x,
				geom.size.y / size.y,				
				geom.size.z / size.z );
  
  glTranslatef( -offset.x, -offset.y, -offset.z );

  LISTMETHOD( blocks, StgBlock*, DrawFootPrint);

  glPopMatrix();
}

void BlockGroup::BuildDisplayList( StgModel* mod )
{
  //printf( "display list for model %s\n", mod->token );

  if( displaylist == 0 )
	 {
		displaylist = glGenLists(1);
		CalcSize();
	 }

  glNewList( displaylist, GL_COMPILE );	
    

  stg_geom_t geom = mod->GetGeom();

  gl_pose_shift( geom.pose );

  glScalef( geom.size.x / size.x,
				geom.size.y / size.y,				
				geom.size.z / size.z );
  
  glTranslatef( -offset.x, -offset.y, -offset.z );
  
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  
  mod->PushColor( mod->color );
  
  for( GList* it=blocks; it; it=it->next )
	 {
		StgBlock* blk = (StgBlock*)it->data;
		
		if( (!blk->inherit_color) && (blk->color != mod->color) )
		  {
			 mod->PushColor( blk->color );		
			 blk->DrawSolid();
			 mod->PopColor();
		  }
		else
		  blk->DrawSolid();
	 }
  
  mod->PopColor();
  
  glDisable(GL_POLYGON_OFFSET_FILL);
  
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glDepthMask(GL_FALSE);
  
  double r,g,b,a;
  stg_color_unpack( mod->color, &r, &g, &b, &a );
  mod->PushColor( stg_color_pack( r/2.0, g/2.0, b/2.0, a ));
  
  for( GList* it=blocks; it; it=it->next )
	 {
		StgBlock* blk = (StgBlock*)it->data;
		
		if( (!blk->inherit_color) && (blk->color != mod->color) )
		  {
			 stg_color_unpack( blk->color, &r, &g, &b, &a );
			 mod->PushColor( stg_color_pack( r/2.0, g/2.0, b/2.0, a ));
			 blk->DrawSolid();
			 mod->PopColor();
		  }
		else
		  blk->DrawSolid();
	 }

  glDepthMask(GL_TRUE);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  mod->PopColor();

  glEndList();
}

void BlockGroup::CallDisplayList( StgModel* mod )
{
  if( displaylist == 0 )
	 BuildDisplayList( mod );
  
  glCallList( displaylist );
}

void BlockGroup::LoadBlock( StgModel* mod, Worldfile* wf, int entity )
{
  AppendBlock( new StgBlock( mod, wf, entity ));
}				
  
void BlockGroup::LoadBitmap( StgModel* mod, const char* bitmapfile, Worldfile* wf )
{
  PRINT_DEBUG1( "attempting to load bitmap \"%s\n", bitmapfile );
  
  char full[_POSIX_PATH_MAX];
  
  if( bitmapfile[0] == '/' )
	 strcpy( full, bitmapfile );
  else
	 {
		char *tmp = strdup(wf->filename);
		snprintf( full, _POSIX_PATH_MAX,
					 "%s/%s",  dirname(tmp), bitmapfile );
		free(tmp);
	 }
  
  PRINT_DEBUG1( "attempting to load image %s", full );
  
  stg_rotrect_t* rects = NULL;
  unsigned int rect_count = 0;
  unsigned int width, height;
  if( stg_rotrects_from_image_file( full,
												&rects,
												&rect_count,
												&width, &height ) )
	 {
		PRINT_ERR1( "failed to load rects from image file \"%s\"",
						full );
		return;
	 }
  
  //printf( "found %d rects in \"%s\" at %p\n", 
  //	  rect_count, full, rects );
  
  if( rects && (rect_count > 0) )
	 {
		// shift the origin from bottom-left to center of the image
		//double dx = width/2.0;
		//double dy = height/2.0;
		
		//puts( "loading rects" );
		for( unsigned int r=0; r<rect_count; r++ )
		  {
			 stg_point_t pts[4];
			 
			 double x = rects[r].pose.x;
			 double y = rects[r].pose.y;
			 double w = rects[r].size.x;
			 double h = rects[r].size.y;
			 
// 			 pts[0].x = x - dx;
// 			 pts[0].y = y - dy;
// 			 pts[1].x = x + w - dx;
// 			 pts[1].y = y -dy;
// 			 pts[2].x = x + w -dx;
// 			 pts[2].y = y + h -dy;
// 			 pts[3].x = x - dx;
// 			 pts[3].y = y + h -dy;							 

			 pts[0].x = x;
			 pts[0].y = y;
			 pts[1].x = x + w;
			 pts[1].y = y;
			 pts[2].x = x + w;
			 pts[2].y = y + h;
			 pts[3].x = x;
			 pts[3].y = y + h;							 
			 
			 // TODO fix this
			 stg_color_t col = stg_color_pack( 1.0, 0,0,1.0 ); 
			 
			 AppendBlock( new StgBlock( mod,
												 pts,4,
												 0,1,
												 col,
												 true ) );		 
		  }			 
		free( rects );
	 }  

  CalcSize();
}

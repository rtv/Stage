
#include "stage.hh"
#include "worldfile.hh"

#include <libgen.h> // for dirname(3)
#include <limits.h> // for _POSIX_PATH_MAX

#undef DEBUG

using namespace Stg;
using namespace std;

BlockGroup::BlockGroup() 
  : displaylist(0),
    blocks() 
{ /* empty */ }

BlockGroup::~BlockGroup()
{	 
  Clear();
}

void BlockGroup::AppendBlock( const Block& block )
{
  blocks.push_back( block );
}

void BlockGroup::Clear()
{
  //FOR_EACH( it, blocks )
  // delete *it;
  
  blocks.clear();
}

void BlockGroup::AppendTouchingModels( std::set<Model*>& v )
{
  FOR_EACH( it, blocks )
    it->AppendTouchingModels( v );  
}

Model* BlockGroup::TestCollision()
{
  Model* hitmod = NULL;
   
  FOR_EACH( it, blocks )
    if( (hitmod = it->TestCollision()))
      break; // bail on the earliest collision

  return hitmod; // NULL if no collision
}


/** find the 3d bounding box of all the blocks in the group */
bounds3d_t BlockGroup::BoundingBox()
{
  // assuming the blocks currently fit in a square +/- one billion units
  double minx, miny, maxx, maxy, minz, maxz;
  minx = miny = minz = billion;
  maxx = maxy = maxz = -billion;
  
  FOR_EACH( it, blocks )
    {
      // examine all the points in the polygon
      FOR_EACH( pit, it->pts )
	{
	  if( pit->x < minx ) minx = pit->x;
	  if( pit->y < miny ) miny = pit->y;
	  if( pit->x > maxx ) maxx = pit->x;
	  if( pit->y > maxy ) maxy = pit->y;
	}
      
      if( it->local_z.min < minz ) minz = it->local_z.min;	  
      if( it->local_z.max > maxz ) maxz = it->local_z.max;
     }
      
   return bounds3d_t( Bounds( minx,maxx), Bounds(miny,maxy), Bounds(minz,maxz));
}

// scale all blocks to fit into the bounding box of this group's model
void BlockGroup::CalcSize()
{  
  const bounds3d_t b = BoundingBox();
  
  const Size size( b.x.max - b.x.min, 
		   b.y.max - b.y.min, 
		   b.z.max - b.z.min );

  const Size offset( b.x.min + size.x/2.0, b.y.min + size.y/2.0,  0 );
  
  // now scale the blocks to fit in the model's 3d bounding box, so
  // that the original points are now in model coordinates
  const Size modsize = blocks[0].mod->geom.size;
  
  FOR_EACH( it, blocks )
    {
      // polygon edges
      FOR_EACH( pit, it->pts  )
	{
	  pit->x = (pit->x - offset.x) * (modsize.x/size.x);
	  pit->y = (pit->y - offset.y) * (modsize.y/size.y);     
	}

      // vertical bounds
      it->local_z.min = (it->local_z.min - offset.z) * (modsize.z/size.z);
      it->local_z.max = (it->local_z.max - offset.z) * (modsize.z/size.z);
    }
}

void BlockGroup::Map( unsigned int layer )
{
  FOR_EACH( it, blocks )
    it->Map(layer);
}

void BlockGroup::UnMap( unsigned int layer )
{
  FOR_EACH( it, blocks )
    it->UnMap(layer);
}

void BlockGroup::DrawSolid( const Geom & geom )
{
  glPushMatrix();
  
  Gl::pose_shift( geom.pose );    
  
  FOR_EACH( it, blocks )
    it->DrawSolid(false);
  
  glPopMatrix();
}

void BlockGroup::DrawFootPrint( const Geom & geom )
{
  FOR_EACH( it, blocks )
    it->DrawFootPrint();
}

void BlockGroup::BuildDisplayList( Model* mod )
{
  //puts( "build" );
  
  if( ! mod->world->IsGUI() )
    return;
  
  //printf( "display list for model %s\n", mod->token );
  if( displaylist == 0 )
    {
      displaylist = glGenLists(1);
      CalcSize();
    }
  
  
  glNewList( displaylist, GL_COMPILE );	
  
  
  // render each block as a polygon extruded into Z
  
  Geom geom = mod->GetGeom();
  
  Gl::pose_shift( geom.pose );
    
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  
  mod->PushColor( mod->color );
  
  //const bool topview =  dynamic_cast<WorldGui*>(mod->world)->IsTopView();
  
  FOR_EACH( blk, blocks )
    {
      if( (!blk->inherit_color) && (blk->color != mod->color) )
	{
	  mod->PushColor( blk->color );					 
	  blk->DrawSolid(false);			
	  mod->PopColor();
	}
      else
	blk->DrawSolid(false);
    }
  
  mod->PopColor();
  
  // outline each poly in a darker version of the same color

  glDisable(GL_POLYGON_OFFSET_FILL);
  
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glDepthMask(GL_FALSE);
  
  Color c = mod->color;
  c.r /= 2.0;
  c.g /= 2.0;
  c.b /= 2.0;
  mod->PushColor( c );
  
  FOR_EACH( blk, blocks )
    {
      if( (!blk->inherit_color) && (blk->color != mod->color) )
	{
	  Color c = blk->color;
	  c.r /= 2.0;
	  c.g /= 2.0;
	  c.b /= 2.0;
	  mod->PushColor( c );
	  blk->DrawSolid(false);
	  mod->PopColor();
	}
      else
	blk->DrawSolid(false);
    }
  
  glDepthMask(GL_TRUE);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  mod->PopColor();

  glEndList();
}

void BlockGroup::CallDisplayList( Model* mod )
{
  if( displaylist == 0 || mod->rebuild_displaylist )
    {
      BuildDisplayList( mod );
      mod->rebuild_displaylist = 0;
    }
  
  glCallList( displaylist );
}

void BlockGroup::LoadBlock( Model* mod, Worldfile* wf, int entity )
{
  AppendBlock( Block( mod, wf, entity ));
  //  CalcSize();
}				
  
void BlockGroup::LoadBitmap( Model* mod, const std::string& bitmapfile, Worldfile* wf )
{
  PRINT_DEBUG1( "attempting to load bitmap \"%s\n", bitmapfile );
	
  std::string full;
  
  if( bitmapfile[0] == '/' )
    full = bitmapfile;
  else
    {
      char* workaround_const = strdup(wf->filename.c_str());
      full = std::string(dirname(workaround_const)) + "/" + bitmapfile;			
      free( workaround_const );
    }
  
  PRINT_DEBUG1( "attempting to load image %s", full );
  
  std::vector<rotrect_t> rects;
  if( rotrects_from_image_file( full,
				rects ) )
    {
      PRINT_ERR1( "failed to load rects from image file \"%s\"",
		  full.c_str() );
      return;
    }
  
  //printf( "found %d rects in \"%s\" at %p\n", 
  //	  rect_count, full, rects );
			 
  // TODO fix this
  Color col( 1.0, 0.0, 1.0, 1.0 );
	
  FOR_EACH( rect, rects )
    {
      std::vector<point_t> pts(4);
			
      const double x = rect->pose.x;
      const double y = rect->pose.y;
      const double w = rect->size.x;
      const double h = rect->size.y;
			
      pts[0].x = x;
      pts[0].y = y;
      pts[1].x = x + w;
      pts[1].y = y;
      pts[2].x = x + w;
      pts[2].y = y + h;
      pts[3].x = x;
      pts[3].y = y + h;							 
      
      AppendBlock( Block( mod,
			  pts,
			  0,1,
			  col,
			  true ) );		 
    }			 
  
  CalcSize();
}


void BlockGroup::Rasterize( uint8_t* data, 
			    unsigned int width, 
			    unsigned int height,
			    meters_t cellwidth,
			    meters_t cellheight )
{  
  FOR_EACH( it, blocks )
    it->Rasterize( data, width, height, cellwidth, cellheight );
}

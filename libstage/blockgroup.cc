
#include "stage.hh"
#include "worldfile.hh"

#include <libgen.h> // for dirname(3)
#include <limits.h> // for _POSIX_PATH_MAX

#undef DEBUG

using namespace Stg;
using namespace std;

BlockGroup::BlockGroup() 
  : displaylist(0),
    blocks(), 
    minx(0),
    maxx(0),
    miny(0),
    maxy(0)
{ /* empty */ }

BlockGroup::~BlockGroup()
{	 
  Clear();
}

void BlockGroup::AppendBlock( Block* block )
{
  blocks.push_back( block );
}

void BlockGroup::Clear()
{
  FOR_EACH( it, blocks )
    delete *it;
  
  blocks.clear();
}

void BlockGroup::AppendTouchingModels( std::set<Model*>& v )
{
  FOR_EACH( it, blocks )
    (*it)->AppendTouchingModels( v );  
}

Model* BlockGroup::TestCollision()
{
  Model* hitmod = NULL;
   
  FOR_EACH( it, blocks )
    if( (hitmod = (*it)->TestCollision()))
      break; // bail on the earliest collision

  return hitmod; // NULL if no collision
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
  
  FOR_EACH( it, blocks )
    {
      // examine all the points in the polygon
      Block* block = *it;
      
      FOR_EACH( it, block->pts )
	{
	  if( it->x < minx ) minx = it->x;
	  if( it->y < miny ) miny = it->y;
	  if( it->x > maxx ) maxx = it->x;
	  if( it->y > maxy ) maxy = it->y;
	}
      
      size.z = std::max( block->local_z.max, size.z );
    }
  
  // store these bounds for normalization purposes
  size.x = maxx-minx;
  size.y = maxy-miny;
  
  offset.x = minx + size.x/2.0;
  offset.y = miny + size.y/2.0;
  offset.z = 0; // todo?

  InvalidateModelPointCache();
}


void BlockGroup::Map( unsigned int layer )
{
  FOR_EACH( it, blocks )
    (*it)->Map(layer);
}

// defined in world.cc
//void SwitchSuperRegionLock( SuperRegion* current, SuperRegion* next );

void BlockGroup::UnMap( unsigned int layer )
{
  FOR_EACH( it, blocks )
    (*it)->UnMap(layer);
}

void BlockGroup::DrawSolid( const Geom & geom )
{
  glPushMatrix();
  
  Gl::pose_shift( geom.pose );
  
  glScalef( geom.size.x / size.x,
	    geom.size.y / size.y,				
	    geom.size.z / size.z );
  
  glTranslatef( -offset.x, -offset.y, -offset.z );
  
  FOR_EACH( it, blocks )
    (*it)->DrawSolid(false);
  
  glPopMatrix();
}

void BlockGroup::DrawFootPrint( const Geom & geom )
{
  glPushMatrix();
  
  glScalef( geom.size.x / size.x,
	    geom.size.y / size.y,				
	    geom.size.z / size.z );
  
  glTranslatef( -offset.x, -offset.y, -offset.z );
  
  FOR_EACH( it, blocks )
    (*it)->DrawFootPrint();
  
  glPopMatrix();
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
  
  glScalef( geom.size.x / size.x,
	    geom.size.y / size.y,				
	    geom.size.z / size.z );
  
  glTranslatef( -offset.x, -offset.y, -offset.z );
  
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  
  mod->PushColor( mod->color );
  
  //const bool topview =  dynamic_cast<WorldGui*>(mod->world)->IsTopView();
  
  FOR_EACH( it, blocks )
    {
      Block* blk = (*it);
      
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
  
  FOR_EACH( it, blocks )
    {
      Block* blk = *it;
		
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
  AppendBlock( new Block( mod, wf, entity ));
  CalcSize();
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
      
      AppendBlock( new Block( mod,
			      pts,
			      0,1,
			      col,
			      true,
			      false) );		 
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
    (*it)->Rasterize( data, width, height, cellwidth, cellheight );
}

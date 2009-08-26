
#include "region.hh"
#include "worldfile.hh"

using namespace Stg;

/** Create a new block. A model's body is a list of these
    blocks. The point data is copied, so pts can safely be freed
    after calling this.*/
Block::Block( Model* mod,
	      stg_point_t* pts,
	      size_t pt_count,
	      stg_meters_t zmin,
	      stg_meters_t zmax,
	      Color color,
	      bool inherit_color ) :
  mod( mod ),
  mpts(),
  pt_count( pt_count ),
  pts(),
  local_z( zmin, zmax ),
  color( color ),
  inherit_color( inherit_color ),
  glow( 0.0 ),
  rendered_cells( new CellPtrVec ), 
  candidate_cells( new CellPtrVec ),
  gpts()
{
  assert( mod );
  assert( pt_count > 0 );
  
  // copy the argument point data into the member vector
  this->pts.reserve( pt_count );
  for( size_t p=0; p<pt_count; p++ )
    this->pts.push_back( pts[p] );	
}

/** A from-file  constructor */
Block::Block(  Model* mod,
	       Worldfile* wf,
	       int entity)
  : mod( mod ),
    mpts(),
    pt_count(0),
    pts(),
    color(0),
    inherit_color(true),
    rendered_cells( new CellPtrVec ), 
    candidate_cells( new CellPtrVec ) 
{
  assert(mod);
  assert(wf);
  assert(entity);
  
  Load( wf, entity );
}

Block::~Block()
{
  if( mapped ) UnMap();
  
  delete rendered_cells;
  delete candidate_cells;
}

void Block::Translate( double x, double y )
{
  for( unsigned int p=0; p<pt_count; p++)
    {
      pts[p].x += x;
      pts[p].y += y;
    }
  
  mod->blockgroup.BuildDisplayList( mod );
}

double Block::CenterY()
{
  double min = billion;
  double max = -billion;
  
  for( unsigned int p=0; p<pt_count; p++)
    {
      if( pts[p].y > max ) max = pts[p].y;
      if( pts[p].y < min ) min = pts[p].y;
    }
  
  // return the value half way between max and min
  return( min + (max - min)/2.0 );
}

double Block::CenterX()
{
  double min = billion;
  double max = -billion;
  
  for( unsigned int p=0; p<pt_count; p++)
    {
      if( pts[p].x > max ) max = pts[p].x;
      if( pts[p].x < min ) min = pts[p].x;
    }

  // return the value half way between maxx and min
  return( min + (max - min)/2.0 );
}

void Block::SetCenter( double x, double y )
{
  // move the block by the distance required to bring its center to
  // the requested position
  Translate( x-CenterX(), y-CenterY() );
}

void Block::SetCenterY( double y )
{
  // move the block by the distance required to bring its center to
  // the requested position
  Translate( 0, y-CenterY() );
}

void Block::SetCenterX( double x )
{
  // move the block by the distance required to bring its center to
  // the requested position
  Translate( x-CenterX(), 0 );
}

void Block::SetZ( double min, double max )
{
  local_z.min = min;
  local_z.max = max;

  // force redraw
  mod->blockgroup.BuildDisplayList( mod );
}

const Color& Block::GetColor()
{
  return( inherit_color ? mod->color : color );
}

void Block::AppendTouchingModels( ModelPtrSet& touchers )
{
  // for every cell we are rendered into
  FOR_EACH( cell_it, *rendered_cells )
	// for every block rendered into that cell
	FOR_EACH( block_it, (*cell_it)->blocks )
	{
	  if( !mod->IsRelated( (*block_it)->mod ))
		touchers.insert( (*block_it)->mod );
	}
}

Model* Block::TestCollision()
{
  //printf( "model %s block %p test collision...\n", mod->Token(), this );

  // find the set of cells we would render into given the current global pose
  GenerateCandidateCells();
  
  if( mod->vis.obstacle_return )
    // for every cell we may be rendered into
	 FOR_EACH( cell_it, *candidate_cells )
      {
		  // for every block rendered into that cell
		  FOR_EACH( block_it, (*cell_it)->blocks )
			 {
				Block* testblock = *block_it;
				Model* testmod = testblock->mod;
				
				//printf( "   testing block %p of model %s\n", testblock, testmod->Token() );
				
				// if the tested model is an obstacle and it's not attached to this model
				if( (testmod != this->mod) &&
					 testmod->vis.obstacle_return &&
					 (!mod->IsRelated( testmod )) && 
					 // also must intersect in the Z range
					 testblock->global_z.min <= global_z.max && 
					 testblock->global_z.max >= global_z.min )
				  {
					 //puts( "HIT");
					 return testmod; // bail immediately with the bad news
				  }
			 }
		}

  //printf( "model %s block %p collision done. no hits.\n", mod->Token(), this );
  return NULL; // no hit
}

void Block::Map()
{
  // TODO - if called often, we may not need to generate each time
  GenerateCandidateCells();
  SwitchToTestedCells();
  mapped = true;
}

void Block::UnMap()
{
  RemoveFromCellArray( rendered_cells );
  rendered_cells->clear();
  mapped = false;
}

void Block::RemoveFromCellArray( CellPtrVec *cells )
{
  FOR_EACH( it, *cells )
	 {
		Cell* cell = *it;
		
		// remove me from the cell
		EraseAll( this, cell->blocks );		
		--cell->region->count;
		--cell->region->superregion->count;  	 	 
	 }
}

void Block::AddToCellArray( CellPtrVec *cells )
{
  FOR_EACH( it, *cells )
	 {
		Cell* cell = *it;

		// record that I am rendered in this cell
		rendered_cells->push_back( cell ); 
		
		// store me in the cell
		cell->blocks.push_back( this );   
		++cell->region->count;
		++cell->region->superregion->count;		
	 }
}

void Block::SwitchToTestedCells()
{
  RemoveFromCellArray( rendered_cells );
  AddToCellArray( candidate_cells );

  // switch current and candidate cell pointers
  CellPtrVec *tmp = rendered_cells;
  rendered_cells = candidate_cells;
  candidate_cells = tmp;

  // XXXX todo 

  mapped = true;
}

inline stg_point_t Block::BlockPointToModelMeters( const stg_point_t& bpt )
{
  Size bgsize = mod->blockgroup.GetSize();
  stg_point3_t bgoffset = mod->blockgroup.GetOffset();

  return stg_point_t( (bpt.x - bgoffset.x) * (mod->geom.size.x/bgsize.x),
					  (bpt.y - bgoffset.y) * (mod->geom.size.y/bgsize.y));
}

void Block::InvalidateModelPointCache()
{
  // this doesn't happen often, so this simple strategy isn't too wasteful
  mpts.clear();
}

void Block::GenerateCandidateCells()
{
  candidate_cells->clear();

  if( mpts.size() == 0 )
	{
	  // no valid cache of model coord points, so generate them
	  mpts.resize( pt_count );
	  for( unsigned int i=0; i<pt_count; i++ )
		mpts[i] = BlockPointToModelMeters( pts[i] );
	}
  
  // convert the mpts in model coords into global pixel coords
  gpts.resize(pt_count);
  
  for( unsigned int i=0; i<pt_count; i++ )
	gpts[i] = mod->world->MetersToPixels( mod->LocalToGlobal( mpts[i] ));
  
  for( unsigned int i=0; i<pt_count; i++ )
	mod->world->ForEachCellInLine( gpts[i], 
								   gpts[(i+1)%pt_count], 
								   *candidate_cells );  
  // set global Z
  Pose gpose = mod->GetGlobalPose();
  gpose.z += mod->geom.pose.z;
  double scalez = mod->geom.size.z /  mod->blockgroup.GetSize().z;
  stg_meters_t z = gpose.z - mod->blockgroup.GetOffset().z;
  
  // store the block's absolute z bounds at this rendering
  global_z.min = (scalez * local_z.min) + z;
  global_z.max = (scalez * local_z.max) + z;
  
  mapped = true;
}

void swap( int& a, int& b )
{
  int tmp = a;
  a = b;
  b = tmp;
}

void Block::Rasterize( uint8_t* data,
					   unsigned int width,
					   unsigned int height,
					   stg_meters_t cellwidth,
					   stg_meters_t cellheight )
{
  //printf( "rasterize block %p : w: %u h: %u  scale %.2f %.2f  offset %.2f %.2f\n",
  //	 this, width, height, scalex, scaley, offsetx, offsety );
  
  for( unsigned int i=0; i<pt_count; i++ )
    {
	  // convert points from local to model coords
	  stg_point_t mpt1 = BlockPointToModelMeters( pts[i] );
	  stg_point_t mpt2 = BlockPointToModelMeters( pts[(i+1)%pt_count] );
	  
	  // record for debug visualization
	  mod->rastervis.AddPoint( mpt1.x, mpt1.y );
	  
	  // shift to the bottom left of the model
	  mpt1.x += mod->geom.size.x/2.0;
	  mpt1.y += mod->geom.size.y/2.0;
	  mpt2.x += mod->geom.size.x/2.0;
	  mpt2.y += mod->geom.size.y/2.0;
	  
	  // convert from meters to cells
	  stg_point_int_t a( floor( mpt1.x / cellwidth  ),
						 floor( mpt1.y / cellheight ));
	  stg_point_int_t b( floor( mpt2.x / cellwidth  ),
						 floor( mpt2.y / cellheight ) );
	  
	  bool steep = abs( b.y-a.y ) > abs( b.x-a.x );
	  if( steep )
		{
		  swap( a.x, a.y );
		  swap( b.x, b.y );
		}
	  
	  if( a.x > b.x )
		{
		  swap( a.x, b.x );
		  swap( a.y, b.y );
		}
	  
	  double dydx = (double) (b.y - a.y) / (double) (b.x - a.x);
	  double y = a.y;
	  for(int x=a.x; x<=b.x; x++)
		{
		  if( steep )
			{
			  if( ! (floor(y) >= 0) ) continue;
			  if( ! (floor(y) < (int)width) ) continue;
			  if( ! (x >= 0) ) continue;
			  if( ! (x < (int)height) ) continue;
			}
		  else
			{
			  if( ! (x >= 0) ) continue;
			  if( ! (x < (int)width) ) continue;
			  if( ! (floor(y) >= 0) ) continue;
			  if( ! (floor(y) < (int)height) ) continue;
			}
		  
		  if( steep )
			data[ (int)floor(y) + (x * width)] = 1;
		  else
			data[ x + ((int)floor(y) * width)] = 1;
		  y += dydx;
		}
	}
}

void Block::DrawTop()
{
  // draw the top of the block - a polygon at the highest vertical
  // extent
  glBegin( GL_POLYGON);
  for( unsigned int i=0; i<pt_count; i++ )
    glVertex3f( pts[i].x, pts[i].y, local_z.max );
  glEnd();
}

void Block::DrawSides()
{
  // construct a strip that wraps around the polygon
  glBegin(GL_QUAD_STRIP);
  for( unsigned int p=0; p<pt_count; p++)
    {
      glVertex3f( pts[p].x, pts[p].y, local_z.max );
      glVertex3f( pts[p].x, pts[p].y, local_z.min );
    }
  // close the strip
  glVertex3f( pts[0].x, pts[0].y, local_z.max );
  glVertex3f( pts[0].x, pts[0].y, local_z.min );
  glEnd();
}

void Block::DrawFootPrint()
{
  glBegin(GL_POLYGON);
  for( unsigned int p=0; p<pt_count; p++ )
    glVertex2f( pts[p].x, pts[p].y );
  glEnd();
}

void Block::Draw( Model* mod )
{
  // draw filled color polygons
  Color col = inherit_color ? mod->color : color;
  
  col.Print( "block color" );

  mod->PushColor( col );
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  DrawSides();
  DrawTop();
  glDisable(GL_POLYGON_OFFSET_FILL);
  
  // draw the block outline in a darker version of the same color
  mod->PushColor( Color( col.r/2.0, col.g/2.0, col.b/2.0, col.a ));
  
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glDepthMask(GL_FALSE);
  DrawTop();
  DrawSides();
  glDepthMask(GL_TRUE);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
 
  if( glow > 0.0 )
	 {
		
	 }
 
  mod->PopColor();
  mod->PopColor();
}

void Block::DrawSolid()
{
  DrawSides();
  DrawTop();
}

void Block::Load( Worldfile* wf, int entity )
{
  //printf( "Block::Load entity %d\n", entity );

  pt_count = wf->ReadInt( entity, "points", 0);
  pts.resize( pt_count );

  //printf( "reading %d points\n",
  //	 pt_count );
  
  char key[128];
  for( unsigned int p=0; p<pt_count; p++ )	      {
    snprintf(key, sizeof(key), "point[%d]", p );
	
    pts[p].x = wf->ReadTupleLength(entity, key, 0, 0);
    pts[p].y = wf->ReadTupleLength(entity, key, 1, 0);
  }
  
  local_z.min = wf->ReadTupleLength( entity, "z", 0, 0.0 );
  local_z.max = wf->ReadTupleLength( entity, "z", 1, 1.0 );
  
  const char* colorstr = wf->ReadString( entity, "color", NULL );
  if( colorstr )
    {
      color = Color( colorstr );
      inherit_color = false;
    }
  else
    inherit_color = true;

  glow = wf->ReadFloat( entity, "glow", glow );
}

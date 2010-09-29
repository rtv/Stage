#include "region.hh"
#include "worldfile.hh"

using namespace Stg;
using std::vector;

static void canonicalize_winding(vector<stg_point_t>& pts);


/** Create a new block. A model's body is a list of these
    blocks. The point data is copied, so pts can safely be freed
    after calling this.*/
Block::Block( Model* mod,
							stg_point_t* pts,
							size_t pt_count,
							stg_meters_t zmin,
							stg_meters_t zmax,
							Color color,
							bool inherit_color,
							bool wheel ) :
  mod( mod ),
  mpts(),
  pt_count( pt_count ),
  pts(),
  local_z( zmin, zmax ),
  color( color ),
  inherit_color( inherit_color ),
	wheel(wheel),
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
  canonicalize_winding(this->pts);
}

/** A from-file  constructor */
Block::Block(  Model* mod,
					Worldfile* wf,
					int entity)
  : mod( mod ),
    mpts(),
    pt_count(0),
    pts(),
    color(),
    inherit_color(true),
    rendered_cells( new CellPtrVec ), 
    candidate_cells( new CellPtrVec ) 
{
  assert(mod);
  assert(wf);
  assert(entity);
  
  Load( wf, entity );
  canonicalize_winding(this->pts);
}

Block::~Block()
{
  if( mapped ) UnMap();
  
  delete rendered_cells;
  delete candidate_cells;
}

void Block::Translate( double x, double y )
{
	FOR_EACH( it, pts )
    {
      it->x += x;
      it->y += y;
    }
  
  mod->blockgroup.BuildDisplayList( mod );
}

double Block::CenterY()
{
  double min = billion;
  double max = -billion;
  
	FOR_EACH( it, pts )
    {
      if( it->y > max ) max = it->y;
      if( it->y < min ) min = it->y;
    }
  
  // return the value half way between max and min
  return( min + (max - min)/2.0 );
}

double Block::CenterX()
{
  double min = billion;
  double max = -billion;
  
	FOR_EACH( it, pts )
    {
      if( it->x > max ) max = it->x;
      if( it->x < min ) min = it->x;
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
  {
    if ( global_z.min < 0 )
      return this->mod->world->GetGround();
	  
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

 		size_t len = cell->blocks.size();
		if( len )
			{
#if 0		// Use conventional STL style		

				// this special-case test is faster for worlds with simple models,
				// which are the ones we want to be really fast. It's a small
				// extra cost for worlds with several models in each cell. It
				// gives a 5% overall speed increase in fasr.world.
				
				if( (cell->blocks.size() == 1) &&
						(cell->blocks[0] == this) ) // special but common case
					{
						cell->blocks.clear(); // cheap
					}
				else // the general but relatively expensive case
					{
						EraseAll( this, cell->blocks );		
					}
#else		// attempt faster removal loop
				// O(n) * low constant array element removal
				// this C-style pointer work looks to be very slightly faster than the STL way
				Block **start = &cell->blocks[0]; // start of array
				Block **r     = &cell->blocks[0]; // read from here
				Block **w     = &cell->blocks[0]; // write to here
				
				while( r < start + len ) // scan down array, skipping 'this' 
					{
						if( *r != this ) 
							*w++ = *r;				
						r++;
					}
				cell->blocks.resize( w-start );
#endif
			}
		
		--cell->region->count;
		--cell->region->superregion->count;  	 	 
	 }

  //printf( "%d %d %.2f\n", count1, countmore, ((float)count1)/float(countmore));
}

void Block::SwitchToTestedCells()
{
  // todo: 

  // 1. find the set of cells in rendered but not candidate and remove
  // them

  // 2. find the set of cells in candidate but not rendered and insert
  // them

  // .. and see if that is faster than the current method

	//printf( "rendered_cells %lu\n", rendered_cells->size() );
	//printf( "candidate_cells %lu\n\n", candidate_cells->size() );	

  RemoveFromCellArray( rendered_cells );

  // render the block into each of the candidate cells
  FOR_EACH( it, *candidate_cells )
	 {
		Cell* cell = *it;
		// record that I am rendered in this cell
		rendered_cells->push_back( cell ); 
		// store me in the cell
		cell->blocks.push_back( this );   

		++cell->region->count;
		++cell->region->superregion->count;		
	 }

  // switch current and candidate cell pointers
  CellPtrVec *tmp = rendered_cells;
  rendered_cells = candidate_cells;
  candidate_cells = tmp;

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
  
  gpts.clear();
  mod->LocalToPixels( mpts, gpts );
  
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
  FOR_EACH( it, pts )
	 glVertex3f( it->x, it->y, local_z.max );
  glEnd();
}

void Block::DrawSides()
{
  // construct a strip that wraps around the polygon
  glBegin(GL_QUAD_STRIP);

  FOR_EACH( it, pts )
	 {
      glVertex3f( it->x, it->y, local_z.max );
      glVertex3f( it->x, it->y, local_z.min );
	 }
  // close the strip
  glVertex3f( pts[0].x, pts[0].y, local_z.max );
  glVertex3f( pts[0].x, pts[0].y, local_z.min );
  glEnd();
}

void Block::DrawFootPrint()
{
  glBegin(GL_POLYGON);	
  FOR_EACH( it, pts )
	 glVertex2f( it->x, it->y );
  glEnd();
}

void Block::DrawSolid()
{
// 	if( wheel )
// 		{
// 			glPushMatrix();

// 			glRotatef( 90,0,1,0 );
// 			glRotatef( 90,1,0,0 );

// 			glTranslatef( -local_z.max /2.0, 0, 0 );


// 			GLUquadric* quadric = gluNewQuadric();		
// 			gluQuadricDrawStyle( quadric, GLU_FILL );
// 			gluCylinder( quadric, local_z.max, local_z.max, size.x, 16, 16 );
// 			gluDeleteQuadric( quadric );

// 			glPopMatrix();
// 		}
//   else
		{
			DrawSides();
			DrawTop();
		}
}

void Block::Load( Worldfile* wf, int entity )
{
  pt_count = wf->ReadInt( entity, "points", 0);
  pts.resize( pt_count );
  
  char key[128];
  for( unsigned int p=0; p<pt_count; p++ )	      
	 {
		snprintf(key, sizeof(key), "point[%d]", p );
		
		pts[p].x = wf->ReadTupleLength(entity, key, 0, 0);
		pts[p].y = wf->ReadTupleLength(entity, key, 1, 0);
	 }
  
  local_z.min = wf->ReadTupleLength( entity, "z", 0, 0.0 );
  local_z.max = wf->ReadTupleLength( entity, "z", 1, 1.0 );
  
  const std::string& colorstr = wf->ReadString( entity, "color", "" );  	

  if( colorstr != "" )
    {
      color = Color( colorstr );
      inherit_color = false;
    }
  else
    inherit_color = true;  

	wheel = wf->ReadInt( entity, "wheel", wheel );
}

/////////////////////////////////////////////////////////////////////////////////////////
// utility functions to ensure block winding is consistent and matches OpenGL's default

static
/// util; puts angle into [0, 2pi)
void positivize(stg_radians_t& angle)
{
    while (angle < 0) angle += 2 * M_PI;
}

static
/// util; puts angle into -pi/2, pi/2
void pi_ize(stg_radians_t& angle)
{
    while (angle < -M_PI) angle += 2 * M_PI;
    while (M_PI < angle)  angle -= 2 * M_PI;
}

typedef stg_point_t V2;

static
/// util; How much was v1 rotated to get to v2?
stg_radians_t angle_change(V2 v1, V2 v2)
{
    stg_radians_t a1 = atan2(v1.y, v1.x);
    positivize(a1);

    stg_radians_t a2 = atan2(v2.y, v2.x);
    positivize(a2);

    stg_radians_t angle_change = a2 - a1;
    pi_ize(angle_change);

    return angle_change;
}

static
/// util; find vectors between adjacent points, pts[next] - pts[cur]
vector<stg_point_t> find_vectors(vector<stg_point_t> const& pts)
{
    vector<stg_point_t> vs;
    assert(2 <= pts.size());
    for (unsigned i = 0, n = pts.size(); i < n; ++i)
    {
        unsigned j = (i + 1) % n;
        vs.push_back(V2(pts[j].x - pts[i].x, pts[j].y - pts[i].y));
    }
    assert(vs.size() == pts.size());
    return vs;
}

static
/// util; finds sum of angle changes, from each vertex to the
/// next one (in current ordering), wrapping around.
stg_radians_t angles_sum(vector<stg_point_t> const& vs)
{
    stg_radians_t angle_sum = 0;
    for (unsigned i = 0, n = vs.size(); i < n; ++i)
    {
        unsigned j = (i + 1) % n;
        angle_sum += angle_change(vs[i], vs[j]);
    }
    return angle_sum;
}

static
/// Util
bool is_canonical_winding(vector<stg_point_t> const& ps)
{
    // reuse stg_point_t as vector
    vector<stg_point_t> vs = find_vectors(ps);
    stg_radians_t sum = angles_sum(vs);
    bool bCanon = 0 < sum;

    return bCanon;
}

static
/// util; sums angle changes to see whether it's 2pi or -2pi.
/// 2pi is counter-clockwise winding (which OpenGL requires),
/// -2pi is clockwise. Reverses <pts> when winding is clockwise.
// Note that a simple line that doubles back on itself has an
// angle sum of 0, but that's intrinsic to a line - its winding could
// be either way.
void canonicalize_winding(vector<stg_point_t>& ps)
{
    if (not is_canonical_winding(ps))
    {
        std::reverse(ps.begin(), ps.end());
    }
}

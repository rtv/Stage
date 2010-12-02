#include "region.hh"
#include "worldfile.hh"

using namespace Stg;
using std::vector;

static void canonicalize_winding(vector<point_t>& pts);


/** Create a new block. A model's body is a list of these
    blocks. The point data is copied, so pts can safely be freed
    after calling this.*/
Block::Block( Model* mod,
							const std::vector<point_t>& pts,
							meters_t zmin,
							meters_t zmax,
							Color color,
							bool inherit_color,
							bool wheel ) :
  mod( mod ),
  mpts(),
  pts(pts),
  local_z( zmin, zmax ),
  color( color ),
  inherit_color( inherit_color ),
	wheel(wheel),
  rendered_cells(), 
  gpts()
{
  assert( mod );
  canonicalize_winding(this->pts);
}

/** A from-file  constructor */
Block::Block(  Model* mod,
					Worldfile* wf,
					int entity)
  : mod( mod ),
    mpts(),
    pts(),
		local_z(),
    color(),
    inherit_color(true),
		wheel(),
    rendered_cells(),
		gpts()
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
  FOR_EACH( cell_it, rendered_cells )
		// for every block rendered into that cell
		FOR_EACH( block_it, (*cell_it)->GetBlocks() )
	 {
		if( !mod->IsRelated( (*block_it)->mod ))
		  touchers.insert( (*block_it)->mod );
	 }
}

Model* Block::TestCollision()
{
  //printf( "model %s block %p test collision...\n", mod->Token(), this );

  // find the set of cells we would render into given the current global pose
  //GenerateCandidateCells();
  
  if( mod->vis.obstacle_return )
  {
    if ( global_z.min < 0 )
      return this->mod->world->GetGround();
	  
    // for every cell we may be rendered into
	 FOR_EACH( cell_it, rendered_cells )
      {
		  // for every block rendered into that cell
				FOR_EACH( block_it, (*cell_it)->GetBlocks() )
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
	// clear out of the old cells
  RemoveFromCellArray( rendered_cells );
	
	// now calculate the local coords of the block vertices
	const unsigned int pt_count = pts.size();
	
  if( mpts.size() == 0 )
		{
			// no valid cache of model coord points, so generate them
			mpts.resize( pts.size() );
			
			for( unsigned int i=0; i<pt_count; ++i )
				mpts[i] = BlockPointToModelMeters( pts[i] );
		}
  
	// now calculate the global pixel coords of the block vertices
  gpts.clear();
  mod->LocalToPixels( mpts, gpts );
  
  for( unsigned int i=0; i<pt_count; ++i )
		MapLine( gpts[i], 
						 gpts[(i+1)%pt_count] );
	
  // update the block's absolute z bounds at this rendering
  Pose gpose = mod->GetGlobalPose();
  gpose.z += mod->geom.pose.z;
  double scalez = mod->geom.size.z /  mod->blockgroup.GetSize().z;
  meters_t z = gpose.z - mod->blockgroup.GetOffset().z;  
  global_z.min = (scalez * local_z.min) + z;
  global_z.max = (scalez * local_z.max) + z;
  
  mapped = true;	
}

void Block::UnMap()
{
  RemoveFromCellArray( rendered_cells );
  rendered_cells.clear();
  mapped = false;
}

#include <algorithm>
#include <functional>

inline void Block::RemoveFromCellArray( CellPtrVec& cells )
{
 	//	FOR_EACH( it, *cells )
	//	(*it)->RemoveBlock(this);
	
	// this is equivalent to the above commented code - experimenting
	// with optimizations
	std::for_each( cells.begin(), 
								 cells.end(), 
								 std::bind2nd( std::mem_fun(&Cell::RemoveBlock), this));
}

void Block::SwitchToTestedCells()
{
}

inline point_t Block::BlockPointToModelMeters( const point_t& bpt )
{
  Size bgsize = mod->blockgroup.GetSize();
  point3_t bgoffset = mod->blockgroup.GetOffset();

  return point_t( (bpt.x - bgoffset.x) * (mod->geom.size.x/bgsize.x),
							 (bpt.y - bgoffset.y) * (mod->geom.size.y/bgsize.y));
}

void Block::InvalidateModelPointCache()
{
  // this doesn't happen often, so this simple strategy isn't too wasteful
  mpts.clear();
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
							  meters_t cellwidth,
							  meters_t cellheight )
{
  //printf( "rasterize block %p : w: %u h: %u  scale %.2f %.2f  offset %.2f %.2f\n",
  //	 this, width, height, scalex, scaley, offsetx, offsety );
	
	const unsigned int pt_count = pts.size();
  for( unsigned int i=0; i<pt_count; ++i )
    {
		// convert points from local to model coords
		point_t mpt1 = BlockPointToModelMeters( pts[i] );
		point_t mpt2 = BlockPointToModelMeters( pts[(i+1)%pt_count] );
	  
		// record for debug visualization
		mod->rastervis.AddPoint( mpt1.x, mpt1.y );
	  
		// shift to the bottom left of the model
		mpt1.x += mod->geom.size.x/2.0;
		mpt1.y += mod->geom.size.y/2.0;
		mpt2.x += mod->geom.size.x/2.0;
		mpt2.y += mod->geom.size.y/2.0;
	  
		// convert from meters to cells
		point_int_t a( floor( mpt1.x / cellwidth  ),
								 floor( mpt1.y / cellheight ));
		point_int_t b( floor( mpt2.x / cellwidth  ),
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
		for(int x=a.x; x<=b.x; ++x)
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
	const unsigned int pt_count = wf->ReadInt( entity, "points", 0);

  char key[128];
  for( unsigned int p=0; p<pt_count; ++p )	      
	 {
		snprintf(key, sizeof(key), "point[%d]", p );
		
		pts.push_back( point_t(  wf->ReadTupleLength(entity, key, 0, 0),
														 wf->ReadTupleLength(entity, key, 1, 0) ));
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


void Block::MapLine( const point_int_t& start,
										 const point_int_t& end )
{  
  // line rasterization adapted from Cohen's 3D version in
  // Graphics Gems II. Should be very fast.  
  const int32_t dx( end.x - start.x );
  const int32_t dy( end.y - start.y );
  const int32_t sx(sgn(dx));  
  const int32_t sy(sgn(dy));  
  const int32_t ax(abs(dx));  
  const int32_t ay(abs(dy));  
  const int32_t bx(2*ax);	
  const int32_t by(2*ay);	 
  int32_t exy(ay-ax); 
  int32_t n(ax+ay);

  int32_t globx(start.x);
  int32_t globy(start.y);
  

	World* w = mod->GetWorld();

  while( n ) 
    {				
      Region* reg( w->GetSuperRegionCreate( point_int_t(GETSREG(globx), GETSREG(globy)))
									 ->GetRegion( GETREG(globx), GETREG(globy) ));
			
			//printf( "REGION %p\n", reg );

      // add all the required cells in this region before looking up
      // another region			
      int32_t cx( GETCELL(globx) ); 
      int32_t cy( GETCELL(globy) );
			
      // need to call Region::GetCell() before using a Cell pointer
      // directly, because the region allocates cells lazily, waiting
      // for a call of this method
      Cell* c( reg->GetCell( cx, cy ) );

			// while inside the region, manipulate the Cell pointer directly
      while( (cx>=0) && (cx<REGIONWIDTH) && 
						 (cy>=0) && (cy<REGIONWIDTH) && 
						 n > 0 )
				{					
					c->AddBlock(this);
					
					// cleverly skip to the next cell (now it's safe to
					// manipulate the cell pointer)
					if( exy < 0 ) 
						{
							globx += sx;
							exy += by;
							c += sx;
							cx += sx;
						}
					else 
						{
							globy += sy;
							exy -= bx; 
							c += sy * REGIONWIDTH;
							cy += sy;
						}
					--n;
				}
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// utility functions to ensure block winding is consistent and matches OpenGL's default

static
/// util; puts angle into [0, 2pi)
void positivize(radians_t& angle)
{
    while (angle < 0) angle += 2 * M_PI;
}

static
/// util; puts angle into -pi/2, pi/2
void pi_ize(radians_t& angle)
{
    while (angle < -M_PI) angle += 2 * M_PI;
    while (M_PI < angle)  angle -= 2 * M_PI;
}

typedef point_t V2;

static
/// util; How much was v1 rotated to get to v2?
radians_t angle_change(V2 v1, V2 v2)
{
    radians_t a1 = atan2(v1.y, v1.x);
    positivize(a1);

    radians_t a2 = atan2(v2.y, v2.x);
    positivize(a2);

    radians_t angle_change = a2 - a1;
    pi_ize(angle_change);

    return angle_change;
}

static
/// util; find vectors between adjacent points, pts[next] - pts[cur]
vector<point_t> find_vectors(vector<point_t> const& pts)
{
    vector<point_t> vs;
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
radians_t angles_sum(vector<point_t> const& vs)
{
    radians_t angle_sum = 0;
    for (unsigned i = 0, n = vs.size(); i < n; ++i)
    {
        unsigned j = (i + 1) % n;
        angle_sum += angle_change(vs[i], vs[j]);
    }
    return angle_sum;
}

static
/// Util
bool is_canonical_winding(vector<point_t> const& ps)
{
    // reuse point_t as vector
    vector<point_t> vs = find_vectors(ps);
    radians_t sum = angles_sum(vs);
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
void canonicalize_winding(vector<point_t>& ps)
{
    if (not is_canonical_winding(ps))
    {
        std::reverse(ps.begin(), ps.end());
    }
}

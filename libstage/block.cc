
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
	      stg_color_t color,
	      bool inherit_color
	      ) :
  mod( mod ),
  pt_count( pt_count ),
  pts( (stg_point_t*)g_memdup( pts, pt_count * sizeof(stg_point_t)) ),
  color( color ),
  inherit_color( inherit_color ),
  rendered_cells( g_ptr_array_sized_new(32) ),
  candidate_cells( g_ptr_array_sized_new(32) )
{
  assert( mod );
  assert( pt_count > 0 );
  assert( pts );

  local_z.min = zmin;
  local_z.max = zmax;
}

/** A from-file  constructor */
Block::Block(  Model* mod,  
	       Worldfile* wf, 
	       int entity) 
  : mod( mod ),
    pt_count(0), 
    pts(NULL), 
    color(0),
    inherit_color(true),
    rendered_cells( g_ptr_array_sized_new(32) ), 
    candidate_cells( g_ptr_array_sized_new(32) )
{
  assert(mod);
  assert(wf);
  assert(entity);
  
  Load( wf, entity );
}

Block::~Block()
{ 
  if( mapped ) UnMap();
  
  stg_points_destroy( pts );
  
  g_ptr_array_free( rendered_cells, TRUE );
  g_ptr_array_free( candidate_cells, TRUE );
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


stg_color_t Block::GetColor()
{
  return( inherit_color ? mod->color : color );
}

GList* Block::AppendTouchingModels( GList* list )
{
  // for every cell we are rendered into
  for( unsigned int i=0; i<rendered_cells->len; i++ )
    {
      Cell* cell = (Cell*)g_ptr_array_index( rendered_cells, i);
		
      // for every block rendered into that cell
      for( GSList* it = cell->list; it; it=it->next )
	{
	  Block* testblock = (Block*)it->data;
	  Model* testmod = testblock->mod;
			 
	  if( !mod->IsRelated( testmod ))
	    if( ! g_list_find( list, testmod ) )
	      list = g_list_append( list, testmod );
	}
    }

  return list;
}

Model* Block::TestCollision()
{
  //printf( "model %s block %p test collision...\n", mod->Token(), this );

  // find the set of cells we would render into given the current global pose
  GenerateCandidateCells();
  
  if( mod->vis.obstacle_return )
    // for every cell we may be rendered into
    for( unsigned int i=0; i<candidate_cells->len; i++ )
      {
	Cell* cell = (Cell*)g_ptr_array_index(candidate_cells, i);
		
	// for every rendered into that cell
	for( GSList* it = cell->list; it; it=it->next )
	  {
	    Block* testblock = (Block*)it->data;
	    Model* testmod = testblock->mod;
			 
	    //printf( "   testing block %p of model %s\n", testblock, testmod->Token() );
			 
	    // if the tested model is an obstacle and it's not attached to this model
	    if( (testmod != this->mod) &&  
		testmod->vis.obstacle_return && 
		!mod->IsRelated( testmod ))
	      {
		//puts( "HIT");
		return testmod; // bail immediately with the bad news
	      }		  
	  }
      }
  
  //printf( "model %s block %p collision done. no hits.\n", mod->Token(), this );
  return NULL; // no hit
}


void Block::RemoveFromCellArray( GPtrArray* ptrarray )
{  
  for( unsigned int i=0; i<ptrarray->len; i++ )	 
    ((Cell*)g_ptr_array_index(ptrarray, i))->RemoveBlock( this );  
}

void Block::AddToCellArray( GPtrArray* ptrarray )
{  
  for( unsigned int i=0; i<ptrarray->len; i++ )	 
    ((Cell*)g_ptr_array_index(ptrarray, i))->AddBlock( this );  
}


// used as a callback to gather an array of cells in a polygon
void AppendCellToPtrArray( Cell* c, GPtrArray* a )
{
  g_ptr_array_add( a, c );
}

// used as a callback to gather an array of cells in a polygon
void AddBlockToCell( Cell* c, Block* block )
{
  c->AddBlock( block );
}

void Block::Map()
{
  // TODO - if called often, we may not need to generate each time
  GenerateCandidateCells();
  SwitchToTestedCells();
  return;

  mapped = true;
}

void Block::UnMap()
{
  RemoveFromCellArray( rendered_cells );
  
  g_ptr_array_set_size( rendered_cells, 0 );
  mapped = false;
}

void Block::SwitchToTestedCells()
{
  RemoveFromCellArray( rendered_cells );
  AddToCellArray( candidate_cells );
  
  // switch current and candidate cell pointers
  GPtrArray* tmp = rendered_cells;
  rendered_cells = candidate_cells;
  candidate_cells = tmp;

  mapped = true;
}

stg_point_t Block::BlockPointToModelMeters( const stg_point_t& bpt )
{
  Pose gpose = mod->GetGlobalPose();
  gpose = pose_sum( gpose, mod->geom.pose ); // add local offset
  
  Size bgsize = mod->blockgroup.GetSize();
  stg_point3_t bgoffset = mod->blockgroup.GetOffset();
  
  stg_point3_t scale;
  scale.x = mod->geom.size.x / bgsize.x;
  scale.y = mod->geom.size.y / bgsize.y;
  scale.z = mod->geom.size.z / bgsize.z;

  stg_point_t mpt;
  mpt.x = (bpt.x - bgoffset.x) * scale.x;
  mpt.y = (bpt.y - bgoffset.y) * scale.y;
  return mpt;
}

void Block::GenerateCandidateCells()

{
  Pose gpose = mod->GetGlobalPose();

  // add local offset
  gpose = pose_sum( gpose, mod->geom.pose );

  Size bgsize = mod->blockgroup.GetSize();
  stg_point3_t bgoffset = mod->blockgroup.GetOffset();

  stg_point3_t scale;
  scale.x = mod->geom.size.x / bgsize.x;
  scale.y = mod->geom.size.y / bgsize.y;
  scale.z = mod->geom.size.z / bgsize.z;


  g_ptr_array_set_size( candidate_cells, 0 );
  
  // compute the global location of the first point
  Pose local( (pts[0].x - bgoffset.x) * scale.x ,
	      (pts[0].y - bgoffset.y) * scale.y, 
	      -bgoffset.z, 
	      0 );

  Pose first_gpose, last_gpose;
  first_gpose = last_gpose = pose_sum( gpose, local );
  
  // store the block's absolute z bounds at this rendering
  global_z.min = (scale.z * local_z.min) + last_gpose.z;
  global_z.max = (scale.z * local_z.max) + last_gpose.z;		
  
  // now loop from the the second to the last
  for( unsigned int p=1; p<pt_count; p++ )
    {
      Pose local( (pts[p].x - bgoffset.x) * scale.x ,
		  (pts[p].y - bgoffset.y) * scale.y, 
		  -bgoffset.z, 
		  0 );		
		
      Pose gpose2 = pose_sum( gpose, local );
		
      // and render the shape of the block into the global cells			 
      mod->world->ForEachCellInLine( last_gpose.x, last_gpose.y, 
				     gpose2.x, gpose2.y, 
				     (stg_cell_callback_t)AppendCellToPtrArray,
				     candidate_cells );
      last_gpose = gpose2;
    }
  
  // close the polygon
  mod->world->ForEachCellInLine( last_gpose.x, last_gpose.y,
				 first_gpose.x, first_gpose.y,
				 (stg_cell_callback_t)AppendCellToPtrArray, 
				 candidate_cells );
  
  mapped = true;
}

// void Block::Rasterize( uint8_t* data, 
// 							  unsigned int width, 
// 							  unsigned int height,
// 							  stg_meters_t cellwidth,
// 							  stg_meters_t cellheight )
// {
//   // add local offset
//   // pose = pose_sum( pose, mod->geom.pose );
  
//   Size bgsize = mod->blockgroup.GetSize();
  
//   double scalex = (width*cellwidth) / bgsize.x;
//   double scaley = (height*cellheight) / bgsize.y;
 
//   Rasterize( data, width, height, scalex, scaley, 0,0 );  
// }

void swap( int& a, int& b )
{
  int tmp = a;
  a = b;
  b = tmp;
}

// void Block::Rasterize( uint8_t* data, 
// 							  unsigned int width, unsigned int height, 
// 							  double scalex, double scaley, 
// 							  double offsetx, double offsety )
// {
//   //printf( "rasterize block %p : w: %u h: %u  scale %.2f %.2f  offset %.2f %.2f\n",
//   //	 this, width, height, scalex, scaley, offsetx, offsety );

//   for( unsigned int i=0; i<pt_count; i++ )
//     {
// 		double px = pts[i].x;
// 		double py = pts[i].y;

// 		//unsigned int keep_i = i;

//  		int xa = floor( (pts[i             ].x + offsetx) * scalex );
//  		int ya = floor( (pts[i             ].y + offsety) * scaley );
//  		int xb = floor( (pts[(i+1)%pt_count].x + offsetx) * scalex );
//  		int yb = floor( (pts[(i+1)%pt_count].y + offsety) * scaley );

// 		mod->rastervis.AddPoint( px, py );

// 		//printf( "  line (%d,%d) to (%d,%d)\n", xa,ya,xb,yb );
		
//   		bool steep = abs( yb-ya ) > abs( xb-xa );
// 		if( steep )
// 		  {
// 			 swap( xa, ya );
// 			 swap( xb, yb );
// 		  }
		
//   		if( xa > xb )
//   		  {
//   			 swap( xa, xb );
//   			 swap( ya, yb );
//   		  }
		
// 		double dydx = (double) (yb - ya) / (double) (xb - xa);
// 		double y = ya;
// 		for(int x=xa; x<=xb; x++) 
// 		  {
// 			 if( steep )
// 				{
// 				  if( ! (floor(y) >= 0) ) continue;
// 				  if( ! (floor(y) < (int)width) ) continue;
// 				  if( ! (x >= 0) ) continue;
// 				  if( ! (x < (int)height) ) continue;
// 				}
// 			 else
// 				{
// 				  if( ! (x >= 0) ) continue;
// 				  if( ! (x < (int)width) ) continue;
// 				  if( ! (floor(y) >= 0) ) continue;
// 				  if( ! (floor(y) < (int)height) ) continue;
// 				}
			 
// 			 if( steep )
// 				data[ (int)floor(y) + (x * width)] = 1;			 
// 			 else
// 				data[ x + ((int)floor(y) * width)] = 1;
// 			 y += dydx;
// 		  }
// 	 }
// }

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
 		int xa = floor( mpt1.x / cellwidth  );
		int ya = floor( mpt1.y / cellheight );
 		int xb = floor( mpt2.x / cellwidth  );
 		int yb = floor( mpt2.y / cellheight );

		//printf( "  line (%d,%d) to (%d,%d)\n", xa,ya,xb,yb );
		
  		bool steep = abs( yb-ya ) > abs( xb-xa );
		if( steep )
		  {
			 swap( xa, ya );
			 swap( xb, yb );
		  }
		
  		if( xa > xb )
  		  {
  			 swap( xa, xb );
  			 swap( ya, yb );
  		  }
		
		double dydx = (double) (yb - ya) / (double) (xb - xa);
		double y = ya;
		for(int x=xa; x<=xb; x++) 
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
  stg_color_t col = inherit_color ? mod->color : color; 
  
   mod->PushColor( col );
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(1.0, 1.0);
   DrawSides();
   DrawTop();
   glDisable(GL_POLYGON_OFFSET_FILL);
  
  //   // draw the block outline in a darker version of the same color
  double r,g,b,a;
  stg_color_unpack( col, &r, &g, &b, &a );
  mod->PushColor( stg_color_pack( r/2.0, g/2.0, b/2.0, a ));
  
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glDepthMask(GL_FALSE);
  DrawTop();
  DrawSides();
  glDepthMask(GL_TRUE);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  mod->PopColor();
  mod->PopColor();
}

void Block::DrawSolid()
{
  DrawSides();
  DrawTop();
}


//#define DEBUG 1


void Block::Load( Worldfile* wf, int entity )
{
  //printf( "Block::Load entity %d\n", entity );
  
  if( pts )
    stg_points_destroy( pts );
  
  pt_count = wf->ReadInt( entity, "points", 0);
  pts = stg_points_create( pt_count );
  
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
      color = stg_lookup_color( colorstr );
      inherit_color = false;
    }
  else
    inherit_color = true;
}

	
  

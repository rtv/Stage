
#include "stage_internal.hh"

//GPtrArray* StgBlock::global_verts = g_ptr_array_sized_new( 1024 );

/** Create a new block. A model's body is a list of these
	 blocks. The point data is copied, so pts can safely be freed
	 after calling this.*/
StgBlock::StgBlock( StgModel* mod,  
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
  //  _gpts( NULL )
{
  assert( mod );
  assert( pt_count > 0 );
  assert( pts );

  local_z.min = zmin;
  local_z.max = zmax;
  
  // add this block's global coords array to a global list
  //g_ptr_array_add( global_verts, this );
}

/** A from-file  constructor */
StgBlock::StgBlock(  StgModel* mod,  
							Worldfile* wf, 
							int entity) 
  : mod( mod ),
	 pts(NULL), 
	 pt_count(0), 
	 color(0),
	 inherit_color(true),
	 rendered_cells( g_ptr_array_sized_new(32) ), 
	 candidate_cells( g_ptr_array_sized_new(32) )
	 //	 _gpts( NULL )
{
  assert(mod);
  assert(wf);
  assert(entity);
  
  Load( wf, entity );

  // add this block's global coords array to a global list
  //g_ptr_array_add( global_verts, this );
}


StgBlock::~StgBlock()
{ 
  if( mapped ) UnMap();
  
  stg_points_destroy( pts );
  
  g_ptr_array_free( rendered_cells, TRUE );
  g_ptr_array_free( candidate_cells, TRUE );

  //free( _gpts );
  //g_ptr_array_remove( global_verts, this );
}

stg_color_t StgBlock::GetColor()
{
  return( inherit_color ? mod->color : color );
}



StgModel* StgBlock::TestCollision()
{
  //printf( "model %s block %p test collision...\n", mod->Token(), this );

  // find the set of cells we would render into given the current global pose
  GenerateCandidateCells();
  
  // for every cell we may be rendered into
  for( int i=0; i<candidate_cells->len; i++ )
	 {
		Cell* cell = (Cell*)g_ptr_array_index(candidate_cells, i);
		
		// for every rendered into that cell
		for( GSList* it = cell->list; it; it=it->next )
		  {
			 StgBlock* testblock = (StgBlock*)it->data;
			 StgModel* testmod = testblock->mod;
			 
			 //printf( "   testing block %p of model %s\n", testblock, testmod->Token() );
			 
			 // if the tested model is an obstacle and it's not attached to this model
			 if( (testmod != this->mod) &&  
					testmod->obstacle_return && 
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


void StgBlock::RemoveFromCellArray( GPtrArray* ptrarray )
{  
  for( unsigned int i=0; i<ptrarray->len; i++ )	 
	 ((Cell*)g_ptr_array_index(ptrarray, i))->RemoveBlock( this );  
}

void StgBlock::AddToCellArray( GPtrArray* ptrarray )
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
void AddBlockToCell( Cell* c, StgBlock* block )
{
  c->AddBlock( block );
}

void StgBlock::Map()
{
  // TODO - if called often, we may not need to generate each time
  GenerateCandidateCells();
  SwitchToTestedCells();
  return;

  mapped = true;
}

void StgBlock::UnMap()
{
  RemoveFromCellArray( rendered_cells );
  
  g_ptr_array_set_size( rendered_cells, 0 );
  mapped = false;
}

void StgBlock::SwitchToTestedCells()
{
  RemoveFromCellArray( rendered_cells );
  AddToCellArray( candidate_cells );
  
  // switch current and candidate cell pointers
  GPtrArray* tmp = rendered_cells;
  rendered_cells = candidate_cells;
  candidate_cells = tmp;

  mapped = true;
}

void StgBlock::GenerateCandidateCells()

{
  stg_pose_t gpose = mod->GetGlobalPose();

  // add local offset
  gpose = pose_sum( gpose, mod->geom.pose );

  stg_point3_t scale;
  scale.x = mod->geom.size.x / mod->blockgroup.size.x;
  scale.y = mod->geom.size.y / mod->blockgroup.size.y;
  scale.z = mod->geom.size.z / mod->blockgroup.size.z;

  g_ptr_array_set_size( candidate_cells, 0 );
  
  // compute the global location of the first point
  stg_pose_t local( (pts[0].x - mod->blockgroup.offset.x) * scale.x ,
						  (pts[0].y - mod->blockgroup.offset.y) * scale.y, 
						  -mod->blockgroup.offset.z, 
						  0 );

  stg_pose_t first_gpose, last_gpose;
  first_gpose = last_gpose = pose_sum( gpose, local );
  
  // store the block's absolute z bounds at this rendering
  global_z.min = (scale.z * local_z.min) + last_gpose.z;
  global_z.max = (scale.z * local_z.max) + last_gpose.z;		
  
  // now loop from the the second to the last
  for( int p=1; p<pt_count; p++ )
	 {
		stg_pose_t local( (pts[p].x - mod->blockgroup.offset.x) * scale.x ,
								(pts[p].y - mod->blockgroup.offset.y) * scale.y, 
								-mod->blockgroup.offset.z, 
								0 );		
		
		stg_pose_t gpose2 = pose_sum( gpose, local );
		
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


void StgBlock::DrawTop()
{
  // draw the top of the block - a polygon at the highest vertical
  // extent
  glBegin( GL_POLYGON);
  for( int i=0; i<pt_count; i++ )
	 glVertex3f( pts[i].x, pts[i].y, local_z.max );
  glEnd();
}       

void StgBlock::DrawSides()
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

void StgBlock::DrawFootPrint()
{
  glBegin(GL_POLYGON);
  for( unsigned int p=0; p<pt_count; p++ )
	 glVertex2f( pts[p].x, pts[p].y );
  glEnd();
}

void StgBlock::Draw( StgModel* mod )
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

void StgBlock::DrawSolid()
{
  DrawSides();
  DrawTop();
}


//#define DEBUG 1


void StgBlock::Load( Worldfile* wf, int entity )
{
  //printf( "StgBlock::Load entity %d\n", entity );
  
  if( pts )
	 stg_points_destroy( pts );
  
  pt_count = wf->ReadInt( entity, "points", 0);
  pts = stg_points_create( pt_count );
  
  //printf( "reading %d points\n",
  //	 pt_count );
  
  char key[128];  
  int p;
  for( p=0; p<pt_count; p++ )	      {
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

	
  

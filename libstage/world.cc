/** @defgroup world World

  Stage simulates a 'world' composed of `models', defined in a `world
  file'.

API: Stg::StgWorld

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim

interval_real   100
interval_sim    100
resolution      0.02

@endverbatim

@par Details
- interval_sim <int>\n
the length of each simulation update cycle in milliseconds.
- interval_real <int>\n
the amount of real-world (wall-clock) time the siulator will attempt to spend on each simulation cycle.
- resolution <float>\n
The resolution (in meters) of the underlying bitmap model. Larger values speed up raytracing at the expense of fidelity in collision detection and sensing. 

@par More examples
The Stage source distribution contains several example world files in
<tt>(stage src)/worlds</tt> along with the worldfile properties
described on the manual page for each model type.

*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

//#define DEBUG 

#include <stdlib.h>
#include <assert.h>
#include <string.h> // for strdup(3)
#include <locale.h> 
#include <glib-object.h> // for g_type_init() used by GDKPixbuf objects
#include <limits.h>
#include <libgen.h> // for dirname(3)
#include "stage_internal.hh"
//#include "region.hh"
#include "file_manager.hh"

// static data members
unsigned int StgWorld::next_id = 0;
bool StgWorld::quit_all = false;
GList* StgWorld::world_list = NULL;

const unsigned int THREAD_COUNT = 8;

gint update_jobs_pending=0;

static guint PointIntHash( stg_point_int_t* pt )
{
	return( pt->x + (pt->y<<16 ));
}

static gboolean PointIntEqual( stg_point_int_t* p1, stg_point_int_t* p2 )
{
	return( memcmp( p1, p2, sizeof(stg_point_int_t) ) == 0 );
}


SuperRegion* StgWorld::CreateSuperRegion( int32_t x, int32_t y )
{
	SuperRegion* sr = new SuperRegion( x, y );
	g_hash_table_insert( superregions, &sr->origin, sr );

	dirty = true; // force redraw
	return sr;
}

void StgWorld::DestroySuperRegion( SuperRegion* sr )
{
	g_hash_table_remove( superregions, &sr->origin );
	delete sr;
}

bool StgWorld::UpdateAll()
{  
  bool quit = true;
  for( GList* it = StgWorld::world_list; it; it=it->next ) 
	 {
		if( ((StgWorld*)it->data)->Update() == false )
		  quit = false;
	 }
  return quit;
}

void StgWorld::update_thread_entry( StgModel* mod, void* dummy )
{
  mod->Update();
  
  g_mutex_lock( mod->world->thread_mutex );
  
  update_jobs_pending--;
  
  if( update_jobs_pending == 0 )
	 g_cond_signal( mod->world->worker_threads_done );
  
  g_mutex_unlock( mod->world->thread_mutex );
}


StgWorld::StgWorld( const char* token, 
						  stg_msec_t interval_sim,
						  double ppm )
  : 
  ray_list( NULL ),
  quit_time( 0 ),
  quit( false ),
  updates( 0 ),
  wf( NULL ),
  graphics( false ), 
  models_by_name( g_hash_table_new( g_str_hash, g_str_equal ) ),
  sim_time( 0 ),
  interval_sim( (stg_usec_t)thousand * interval_sim ),
  ppm( ppm ), // raytrace resolution
  real_time_start( RealTimeNow() ),
  superregions( g_hash_table_new( (GHashFunc)PointIntHash, 
											 (GEqualFunc)PointIntEqual ) ),
  total_subs( 0 ),
  destroy( false ),
  real_time_now( 0 ),
  velocity_list( NULL ),
  update_list( NULL ),
  sr_cached(NULL),
  worker_threads( 0 ),
  threadpool( NULL ),
  thread_mutex( g_mutex_new() ),
  worker_threads_done( g_cond_new() )
{
  if( ! Stg::InitDone() )
	 {
		PRINT_WARN( "Stg::Init() must be called before a StgWorld is created." );
		exit(-1);
	 }
  
  StgWorld::world_list = g_list_append( StgWorld::world_list, this );

  bzero( &this->extent, sizeof(this->extent));
}


StgWorld::~StgWorld( void )
{
	PRINT_DEBUG2( "destroying world %d %s", id, token );

	if( wf ) delete wf;

	g_hash_table_destroy( models_by_name );
	g_free( token );

	world_list = g_list_remove( world_list, this );
}


void StgWorld::RemoveModel( StgModel* mod )
{
	g_hash_table_remove( models_by_name, mod );
}

// wrapper to startup all models from the hash table
void init_models( gpointer dummy1, StgModel* mod, gpointer dummy2 )
{
	mod->Init();
}

void StgWorld::LoadBlock( Worldfile* wf, int entity, GHashTable* entitytable )
{ 
  // lookup the group in which this was defined
  StgModel* mod = (StgModel*)g_hash_table_lookup( entitytable, 
																	  (gpointer)wf->GetEntityParent( entity ) );
  
  if( ! mod )
	 PRINT_ERR( "block has no model for a parent" );
  
  mod->LoadBlock( wf, entity );
}

void StgWorld::LoadModel( Worldfile* wf, int entity, GHashTable* entitytable )
{ 
  int parent_entity = wf->GetEntityParent( entity );
  
  PRINT_DEBUG2( "wf entity %d parent entity %d\n", 
					 entity, parent_entity );
  
  StgModel *mod, *parent;
  
  parent = (StgModel*)g_hash_table_lookup( entitytable, 
														 (gpointer)parent_entity );
  
  // find the creator function pointer in the hash table. use the
  // vanilla model if the type is NULL.
  stg_creator_t creator = NULL;
  
  //printf( "creating model of type %s\n", typestr );
  
  char *typestr = (char*)wf->GetEntityType(entity);      	  

  if( typestr ) // look up the string in the typetable
	 for( int i=0; i<MODEL_TYPE_COUNT; i++ )
		if( strcmp( typestr, typetable[i].token ) == 0 )
		  {
			 creator = typetable[i].creator;
			 break;
		  }
  
  // if we found a creator function, call it
  if( creator )
	 {
		//printf( "creator fn: %p\n", creator );
		mod = (*creator)( this, parent );
	 }
  else
	 {
		PRINT_ERR1( "Unknown model type %s in world file.", 
						typestr );
		exit( 1 );
	 }
  
  //printf( "created model %s\n", mod->Token() );
  
  // configure the model with properties from the world file
  mod->Load(wf, entity );
  
  // record the model we created for this worlfile entry
  g_hash_table_insert( entitytable, (gpointer)entity, mod );

  // store the new model's blockgroup so we can find it for later blocks
  //g_hash_table_insert( blockgroups_by_entity, (gpointer)entity, mod->blockgroup );
}
  
void StgWorld::Load( const char* worldfile_path )
{
  GHashTable* entitytable = g_hash_table_new( g_direct_hash, g_direct_equal );

	printf( " [Loading %s]", worldfile_path );
	fflush(stdout);

	stg_usec_t load_start_time = RealTimeNow();

	this->wf = new Worldfile();
	wf->Load( worldfile_path );
	PRINT_DEBUG1( "wf has %d entitys", wf->GetEntityCount() );

	// end the output line of worldfile components
	//puts("");

	int entity = 0;

	this->token = (char*)
		wf->ReadString( entity, "name", token );

	this->interval_sim = (stg_usec_t)thousand * 
	  wf->ReadInt( entity, "interval_sim", 
		       (int)(this->interval_sim/thousand) );

	if( wf->PropertyExists( entity, "quit_time" ) ) {
		this->quit_time = (stg_usec_t) ( million * 
			wf->ReadFloat( entity, "quit_time", 0 ) );
	}

	if( wf->PropertyExists( entity, "resolution" ) )
		this->ppm = 
		  1.0 / wf->ReadFloat( entity, "resolution", 1.0 / this->ppm );

	//_stg_disable_gui = wf->ReadInt( entity, "gui_disable", _stg_disable_gui );

	if( wf->PropertyExists( entity, "threadpool" ) )
	  {
		 int count = wf->ReadInt( entity, "threadpool", worker_threads );
		 
		 if( count && (count != (int)worker_threads) )
			{
			  worker_threads = count;
			  
			  if( threadpool == NULL )
				 threadpool = g_thread_pool_new( (GFunc)update_thread_entry, 
															NULL,
															worker_threads, 
															true,
															NULL );
			  else
				 g_thread_pool_set_max_threads( threadpool, 
														  worker_threads, 
														  NULL );

			  printf( "[threadpool %u]", worker_threads );
			}
	  }

	// Iterate through entitys and create objects of the appropriate type
	for( int entity = 1; entity < wf->GetEntityCount(); entity++ )
	  {
		 const char *typestr = (char*)wf->GetEntityType(entity);      	  
		 
		 // don't load window entries here
		 if( strcmp( typestr, "window" ) == 0 )
			{
			  /* do nothing here */
			}
		 //else if( strcmp( typestr, "blockgroup" ) == 0 )
		 //LoadBlockGroup( wf, entity, entitytable );		
		 else if( strcmp( typestr, "block" ) == 0 )
			LoadBlock( wf, entity, entitytable );
		 else
			LoadModel( wf, entity, entitytable );
	  }
	

	// warn about unused WF lines
	wf->WarnUnused();
	
	// now we're done with the worldfile entry lookup
	g_hash_table_destroy( entitytable );
	
	LISTMETHOD( children, StgModel*, InitRecursive );
	
	stg_usec_t load_end_time = RealTimeNow();
	
	printf( "[Load time %.3fsec]\n", 
			  (load_end_time - load_start_time) / 1000000.0 );
}


// delete a model from the hash table
static void destroy_model( gpointer dummy1, StgModel* mod, gpointer dummy2 )
{
	free(mod);
}

// delete a model from the hash table
static void destroy_sregion( gpointer dummy1, SuperRegion* sr, gpointer dummy2 )
{
	free(sr);
}

void StgWorld::UnLoad()
{
	if( wf ) delete wf;

	g_list_foreach( children, (GFunc)destroy_model, NULL );
	g_list_free( children );
	children = NULL;
	
	g_hash_table_remove_all( models_by_name );
		
	g_list_free( update_list );
	update_list = NULL;
	
	g_list_free( ray_list );
	ray_list = NULL;
	
	g_hash_table_foreach( superregions, (GHFunc)destroy_sregion, NULL );
	g_hash_table_remove_all( superregions );

	token = NULL;
}

stg_usec_t StgWorld::RealTimeNow()
{
	struct timeval tv;
	gettimeofday( &tv, NULL );  // slow system call: use sparingly

	return( tv.tv_sec*1000000 + tv.tv_usec );
}

stg_usec_t StgWorld::RealTimeSinceStart()
{
	stg_usec_t timenow = RealTimeNow();

	// subtract the start time from the current time to get the elapsed
	// time

	return timenow - real_time_start;
}


bool StgWorld::PastQuitTime() 
{ 
  return( (quit_time > 0) && (sim_time >= quit_time) ); 
}

bool StgWorld::Update()
{
  PRINT_DEBUG( "StgWorld::Update()" );
  
	// if we've run long enough, exit
	if( PastQuitTime() ) {
		if( IsGUI() == false )
			return true;		
	}

	// upate all positions first
	LISTMETHOD( velocity_list, StgModel*, UpdatePose );
	
	// then update all sensors	
	if( worker_threads == 0 ) // do all the work in this thread
	  {
		 LISTMETHOD( update_list, StgModel*, UpdateIfDue );
	  }
	else // use worker threads
	  {
		 // push the update for every model that needs it into the thread pool
		 for( GList* it = update_list; it; it=it->next )
			{
			  StgModel* mod = (StgModel*)it->data;
			  
			  if( mod->UpdateDue()  )
				 {
					if( mod->thread_safe ) // do update in a worker thread
					  {
						 g_mutex_lock( thread_mutex );
						 update_jobs_pending++;
						 g_mutex_unlock( thread_mutex );						 
						 g_thread_pool_push( threadpool, mod, NULL );
					  }
					else
					  mod->Update(); // do update in this thread
				 }
			}	
		 
		 // wait for all the last update job to complete - it will
		 // signal the worker_threads_done condition var
		 g_mutex_lock( thread_mutex );
		 while( update_jobs_pending )
			g_cond_wait( worker_threads_done, thread_mutex );
		 g_mutex_unlock( thread_mutex );		 
	  }

  this->sim_time += this->interval_sim;
  this->updates++;
	
  return false;
}


void StgWorld::AddModel( StgModel*  mod  )
{
	g_hash_table_insert( this->models_by_name, (gpointer)mod->Token(), mod );
}

StgModel* StgWorld::GetModel( const char* name )
{
	PRINT_DEBUG1( "looking up model name %s in models_by_name", name );
	StgModel* mod = (StgModel*)g_hash_table_lookup( this->models_by_name, name );

	if( mod == NULL )
		PRINT_WARN1( "lookup of model name %s: no matching name", name );

	return mod;
}

void StgWorld::RecordRay( double x1, double y1, double x2, double y2 )
{  
	float* drawpts = new float[4];
	drawpts[0] = x1;
	drawpts[1] = y1;
	drawpts[2] = x2;
	drawpts[3] = y2;
	ray_list = g_list_append( ray_list, drawpts );
}

void StgWorld::ClearRays()
{
	for( GList* it = ray_list; it; it=it->next )
	{
		float* pts = (float*)it->data;
		delete [] pts;
	}

	g_list_free(ray_list );
	ray_list = NULL;
}


void StgWorld::Raytrace( stg_pose_t pose, // global pose
		stg_meters_t range,
		stg_radians_t fov,
		stg_ray_test_func_t func,
		StgModel* model,			 
		const void* arg,
		stg_raytrace_result_t* samples, // preallocated storage for samples
		uint32_t sample_count,
		bool ztest )  // number of samples
{
	pose.a -= fov/2.0; // direction of first ray
	stg_radians_t angle_incr = fov/(double)sample_count;

	for( uint32_t s=0; s < sample_count; s++ )
	{
		samples[s] = Raytrace( pose, range, func, model, arg, ztest );
		pose.a += angle_incr;
	}
}


// inline functions for converting from global cell coordinates to
// superregions, regions and local cells

inline int32_t SUPERREGION( const int32_t& x )
{  
  return( x >> SRBITS );
}

inline int32_t REGION( const int32_t& x )
{  
  const int32_t _region_coord_mask = ~ ( ( ~ 0x00 ) << SRBITS );
  return( ( x & _region_coord_mask ) >> RBITS );
}

inline int32_t CELL( const int32_t& x )
{  
  static const int32_t _cell_coord_mask = ~ ( ( ~ 0x00 ) << RBITS );
  return( x & _cell_coord_mask );															  
}

inline stg_point_int_t SUPERREGION( const stg_point_int_t& glob )
{
  stg_point_int_t sr;
  sr.x = SUPERREGION( glob.x );
  sr.y = SUPERREGION( glob.y );
  return sr;
}

inline stg_point_int_t REGION( const stg_point_int_t& glob )
{
  stg_point_int_t reg;
  reg.x = REGION( glob.x );
  reg.y = REGION( glob.y );
  return reg;
}

inline stg_point_int_t CELL( const stg_point_int_t& glob )
{
  stg_point_int_t c;
  c.x = CELL( glob.x );
  c.y = CELL( glob.y );
  return c;
}

stg_raytrace_result_t StgWorld::Raytrace( stg_pose_t gpose, 
														stg_meters_t range,
														stg_ray_test_func_t func,
														StgModel* mod,		
														const void* arg,
														bool ztest ) 
{
  stg_raytrace_result_t sample;

  //  printf( "raytracing at [ %.2f %.2f %.2f %.2f ] for %.2f \n",
	// 	  pose.x,
	// 	  pose.y,
	// 	  pose.z,
	// 	  pose.a,
	// 	  range );
	
	// initialize the sample
	sample.pose = gpose;
	sample.range = range; // we might change this below
	sample.mod = NULL; // we might change this below
	
	// find the global integer bitmap address of the ray  
	stg_point_int_t glob;
	glob.x = (int32_t)(gpose.x*ppm);
	glob.y = (int32_t)(gpose.y*ppm);
	
	// record our starting position
	stg_point_int_t start = glob;
	
	// and the x and y offsets of the ray
	int32_t dx = (int32_t)(ppm*range * cos(gpose.a));
	int32_t dy = (int32_t)(ppm*range * sin(gpose.a));
	
	//   if( finder->debug )
	//     RecordRay( pose.x, 
	// 	       pose.y, 
	// 	       pose.x + range.max * cos(pose.a),
	// 	       pose.y + range.max * sin(pose.a) );
	
	// fast integer line 3d algorithm adapted from Cohen's code from
	// Graphics Gems IV

	int sx = sgn(dx);   // sgn() is a fast macro
	int sy = sgn(dy);  
	int ax = abs(dx); 
	int ay = abs(dy);  
	int bx = 2*ax;	
	int by = 2*ay;	
	int exy = ay-ax; 
	int n = ax+ay;
	
	//  printf( "Raytracing from (%d,%d) steps (%d,%d) %d\n",
	//  x,y,  dx,dy, n );
	
	// superregion coords
	stg_point_int_t lastsup;
	lastsup.x = INT_MAX; // an unlikely first raytrace
	lastsup.y = INT_MAX;
	
	stg_point_int_t lastreg;
	lastreg.x = INT_MAX; // an unlikely first raytrace
	lastreg.y = INT_MAX;
	
	SuperRegion* sr = NULL;
	Region* r = NULL;
	bool nonempty_region = false;
	
	// superregion coords (must be updated every time x or y changes
	// but only one variable changes at a time in the loop, so its 
	// more efficient to compute at the end of the loop, when we know what's changed)
	stg_point_int_t sup = SUPERREGION( glob );

	// find the region coords inside this superregion (again: only updated when x or y changes)
	stg_point_int_t reg = REGION( glob );
	
	// compute the pixel offset inside this region (again: only updated when x or y changes)
	stg_point_int_t cell = CELL( glob );
		
	// fix a little issue where rays are not drawn long enough when
	// drawing to the right or up
	if( dx > 0 )
	  n++;
	else if( dy > 0 )
	  n++;
	
	// find the starting superregion 
	sr = GetSuperRegionCached( sup ); // possibly NULL, but unlikely

	while ( n-- ) 
    {         
		
		//printf( "pixel [%d %d]\tS[ %d %d ]\t",
		//	  x, y, sup.x, sup.y );
		
		if( sr )
		{	  
			//  printf( "R[ %d %d ]\t", reg.x, reg.y );
			
			if( reg.x != lastreg.x || reg.y != lastreg.y )
			{
			  r = sr->GetRegion( reg.x, reg.y );
				lastreg = reg;
				nonempty_region = ( r && r->count );
			}

			if( nonempty_region )
			{
			  //printf( "C[ %d %d ]\t", cell.x, cell.y );
			  
			  Cell* c = r->GetCell( cell.x, cell.y );
			  assert(c);
			  
			  for( GSList* list = c->list;
					 list; 
					 list = list->next )      
				 {	      	      
					StgBlock* block = (StgBlock*)list->data;
					assert( block );
					
					//printf( "ENT %p mod %p z [%.2f %.2f] color %X\n",
					//		ent,
					//		ent->mod, 
					//		ent->zbounds.min, 
					//		ent->zbounds.max, 
					//		ent->color );
					
					// skip if not in the right z range
					if( ztest && 
						 ( gpose.z < block->global_z.min || 
							gpose.z > block->global_z.max ) )
 				 {
					//max( "failed ztest: ray at %.2f block at [%.2f %.2f]\n",
					//	 pose.z, ent->zbounds.min, ent->zbounds.max );
					continue; 
				 }
				 
				 //printf( "RT: mod %p testing mod %p at %d %d\n",
				 //		mod, ent->mod, x, y );
				 // printf( "RT: mod %p %s testing mod %p %s at %d %d\n",
				 //		mod, mod->Token(), ent->mod, ent->mod->Token(), x, y );
				 
					 // test the predicate we were passed
					 if( (*func)( block->mod, mod, arg )) // TODO
					 {
						// a hit!
						sample.color = block->GetColor();
						sample.mod = block->mod;
						sample.range = hypot( (glob.x-start.x)/ppm, (glob.y-start.y)/ppm );
						return sample;
					 }
				  }
			}
		}      
		
		//      printf( "\t step %d n %d   pixel [ %d, %d ] block [ %d %d ] index [ %d %d ] \n", 
		//      //coarse [ %d %d ]\n",
		//      count++, n, x, y, blockx, blocky, b_dx, b_dy );
		
		// increment our pixel in the correct direction
		if( exy < 0 ) // we're iterating along X
		  {
			 glob.x += sx; 
			 exy += by;
			 
			 sup.x = SUPERREGION( glob.x );
			 if( sup.x != lastsup.x ) 
				{
				  sr = (SuperRegion*)g_hash_table_lookup( superregions, (void*)&sup );
				  lastsup = sup; // remember these coords
				}
			 
			 // recompute current region and cell (we can skip these if
			 // sr==NULL, but profiling suggests it's faster to do them
			 // than add a conditional)
			 reg.x = REGION( glob.x); 
			 cell.x = CELL( glob.x ); 
		  }
		else  // we're iterating along Y
		  {
			 glob.y += sy; 
			 exy -= bx; 
			 
			 sup.y = SUPERREGION( glob.y );
			 if( sup.y != lastsup.y )
				{
				  sr = (SuperRegion*)g_hash_table_lookup( superregions, (void*)&sup );
				  lastsup = sup; // remember these coords
				}
			 
			 // recompute current region and cell (we can skip these if
			 // sr==NULL, but profiling suggests it's faster to do them
			 // than add a conditional)
			 reg.y = REGION( glob.y ); 			
			 cell.y = CELL( glob.y );
		  }
    }
	
	// hit nothing
	sample.mod = NULL;
	return sample;
}

static int _save_cb( StgModel* mod, void* dummy )
{
	mod->Save();
}

bool StgWorld::Save( const char *filename )
{
	ForEachDescendant( _save_cb, NULL );

	return this->wf->Save( filename );
}

static int _reload_cb(  StgModel* mod, void* dummy )
{
	mod->Load();
}

// reload the current worldfile
void StgWorld::Reload( void )
{
	ForEachDescendant( _reload_cb, NULL );
}

SuperRegion* StgWorld::AddSuperRegion( const stg_point_int_t& sup )
{
  //printf( "Creating super region [ %d %d ]\n", sup.x, sup.y );
  SuperRegion* sr = CreateSuperRegion( sup.x, sup.y );
  
  // the bounds of the world have changed
  stg_point3_t pt;
  pt.x = (sup.x << SRBITS) / ppm;
  pt.y = (sup.y << SRBITS) / ppm;
  pt.z = 0;
  Extend( pt ); // lower left corner of the new superregion
  
  pt.x = ((sup.x+1) << SRBITS) / ppm;
  pt.y = ((sup.y+1) << SRBITS) / ppm;
  pt.z = 0;
  Extend( pt ); // top right corner of the new superregion
  
  return sr;
}

inline SuperRegion* StgWorld::GetSuperRegionCached( const stg_point_int_t& sup )
{
  // around 99% of the time the SR is the same as last
  // lookup - cache  gives a 4% overall speed up :)
  
  if( sr_cached && sr_cached->origin.x == sup.x && sr_cached->origin.y == sup.y )
	 return sr_cached;
  //else
  return GetSuperRegion( sup);
}

inline SuperRegion* StgWorld::GetSuperRegion( const stg_point_int_t& sup )
{
  // no, so we try to fetch it out of the hash table
  SuperRegion* sr = (SuperRegion*)
	 g_hash_table_lookup( superregions, (gpointer)&sup );		  
  
  if( sr == NULL ) // no superregion exists! make a new one
	 sr = AddSuperRegion( sup );
  
  // cache for next time around
  sr_cached = sr;
  
  assert( sr ); 
  return sr;
}

inline Cell* StgWorld::GetCell( const int32_t x, const int32_t y )
{
  stg_point_int_t glob;
  glob.x = x;
  glob.y = y;

  return( GetSuperRegionCached( SUPERREGION(glob) )
			 ->GetRegion( REGION(x), REGION(y) )
			 ->GetCell( CELL(x), CELL(y) )) ;
}

void StgWorld::ForEachCellInLine( stg_meters_t x1, stg_meters_t y1,
											 stg_meters_t x2, stg_meters_t y2,
											 stg_cell_callback_t cb,
											 void* cb_arg )
{  
  int32_t x = MetersToPixels( x1 ); // global pixel coords
  int32_t y = MetersToPixels( y1 );
  
  int32_t dx = MetersToPixels( x2 - x1 );
  int32_t dy = MetersToPixels( y2 - y1 );
  
  // line rasterization adapted from Cohen's 3D version in
  // Graphics Gems II. Should be very fast.
  
  int sx = sgn(dx);  // sgn() is a fast macro
  int sy = sgn(dy);  
  int ax = abs(dx);  
  int ay = abs(dy);  
  int bx = 2*ax;	
  int by = 2*ay;	 
  int exy = ay-ax; 
  int n = ax+ay;
  
  // fix a little issue where the edges are not drawn long enough
  // when drawing to the right or up
  if( (dx > 0) || ( dy > 0 ) )
	 n++;
  
  while( n-- ) 
	 {				
		// find the cell at this location, then call the callback
		// with the cell, block and user-defined argument
		(*cb)( GetCell( x,y ), cb_arg );
		
		// cleverly skip to the next cell			 
		if( exy < 0 ) 
		  {
			 x += sx;
			 exy += by;
		  }
		else 
		  {
			 y += sy;
			 exy -= bx; 
		  }
	 }
}


void StgWorld::Extend( stg_point3_t pt )
{
	extent.x.min = MIN( extent.x.min, pt.x);
	extent.x.max = MAX( extent.x.max, pt.x );
	extent.y.min = MIN( extent.y.min, pt.y );
	extent.y.max = MAX( extent.y.max, pt.y );
	extent.z.min = MIN( extent.z.min, pt.z );
	extent.z.max = MAX( extent.z.max, pt.z );
}




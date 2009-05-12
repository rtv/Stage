/** @defgroup world World

    Stage simulates a 'world' composed of `models', defined in a `world
    file'.

    API: Stg::World

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

#include "stage.hh"
#include "file_manager.hh"
#include "worldfile.hh"
#include "region.hh"
#include "option.hh"
using namespace Stg;


// static data members
unsigned int World::next_id = 0;
bool World::quit_all = false;
GList* World::world_list = NULL;

static guint PointIntHash( stg_point_int_t* pt )
{
  return( pt->x + (pt->y<<16 ));
}

static gboolean PointIntEqual( stg_point_int_t* p1, stg_point_int_t* p2 )
{
  return( memcmp( p1, p2, sizeof(stg_point_int_t) ) == 0 );
}


World::World( const char* token, 
				  stg_msec_t interval_sim,
				  double ppm )
  : 
  // private
  charge_list( NULL ),
  destroy( false ),
  dirty( true ),
  models_by_name( g_hash_table_new( g_str_hash, g_str_equal ) ),
  ppm( ppm ), // raytrace resolution
  quit( false ),
  quit_time( 0 ),
  real_time_now( RealTimeNow() ),
  real_time_start( real_time_now ),
  show_clock( false ),
  show_clock_interval( 100 ), // 10 simulated seconds using defaults
  thread_mutex( g_mutex_new() ),
  threadpool( NULL ),
  total_subs( 0 ), 
  update_jobs_pending(0),
  velocity_list( NULL ),
  worker_threads( 0 ),
  worker_threads_done( g_cond_new() ),

  // protected
  cb_list(NULL),
  extent(),
  graphics( false ), 
  interval_sim( (stg_usec_t)thousand * interval_sim ),
  option_table( g_hash_table_new( g_str_hash, g_str_equal ) ), 
  powerpack_list( NULL ),
  ray_list( NULL ),  
  sim_time( 0 ),
  superregions( g_hash_table_new( (GHashFunc)PointIntHash, 
											 (GEqualFunc)PointIntEqual ) ),
  sr_cached(NULL),
  reentrant_update_list( NULL ),
  nonreentrant_update_list( NULL ),
  updates( 0 ),
  wf( NULL )
{
  if( ! Stg::InitDone() )
    {
      PRINT_WARN( "Stg::Init() must be called before a World is created." );
      exit(-1);
    }
  
  World::world_list = g_list_append( World::world_list, this );
}


World::~World( void )
{
  PRINT_DEBUG2( "destroying world %d %s", id, token );

  if( wf ) delete wf;

  g_hash_table_destroy( models_by_name );
  g_free( token );

  world_list = g_list_remove( world_list, this );
}


SuperRegion* World::CreateSuperRegion( stg_point_int_t origin )
{
  SuperRegion* sr = new SuperRegion( this, origin );
  g_hash_table_insert( superregions, &sr->origin, sr );

  dirty = true; // force redraw
  return sr;
}

void World::DestroySuperRegion( SuperRegion* sr )
{
  g_hash_table_remove( superregions, &sr->origin );
  delete sr;
}

bool World::UpdateAll()
{  
  bool quit = true;
  for( GList* it = World::world_list; it; it=it->next ) 
    {
      if( ((World*)it->data)->Update() == false )
		  quit = false;
    }
  return quit;
}

void World::update_thread_entry( Model* mod, World* world )
{
  mod->Update();
  
  g_mutex_lock( world->thread_mutex );
  
  world->update_jobs_pending--;
  
  if( world->update_jobs_pending == 0 )
    g_cond_signal( world->worker_threads_done );
  
  g_mutex_unlock( world->thread_mutex );
}


void World::RemoveModel( Model* mod )
{
  g_hash_table_remove( models_by_name, mod );
}

// wrapper to startup all models from the hash table
void init_models( gpointer dummy1, Model* mod, gpointer dummy2 )
{
  mod->Init();
}

void World::LoadBlock( Worldfile* wf, int entity, GHashTable* entitytable )
{ 
  // lookup the group in which this was defined
  Model* mod = (Model*)g_hash_table_lookup( entitytable, 
														  (gpointer)wf->GetEntityParent( entity ) );
  
  if( ! mod )
    PRINT_ERR( "block has no model for a parent" );
  
  mod->LoadBlock( wf, entity );
}

// void World::LoadPuck( Worldfile* wf, int entity, GHashTable* entitytable )
// { 
//   Puck* puck = new Puck();
//   puck->Load( wf, entity );  
//   puck_list = g_list_prepend( puck_list, puck );
// }



Model* World::CreateModel( Model* parent, const char* typestr )
{
  Model* mod = NULL; // new model to return
  
  // find the creator function pointer in the hash table. use the
  // vanilla model if the type is NULL.
  stg_creator_t creator = NULL;
  
  //printf( "creating model of type %s\n", typestr );
  
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
  
  return mod;
}


void World::LoadModel( Worldfile* wf, int entity, GHashTable* entitytable )
{ 
  int parent_entity = wf->GetEntityParent( entity );
  
  PRINT_DEBUG2( "wf entity %d parent entity %d\n", 
					 entity, parent_entity );
  
  Model* parent = (Model*)g_hash_table_lookup( entitytable, 
															  (gpointer)parent_entity );
    
  char *typestr = (char*)wf->GetEntityType(entity);      	  
  assert(typestr);
  
  Model* mod = CreateModel( parent, typestr );
  
  // configure the model with properties from the world file
  mod->Load(wf, entity );
 
  // record the model we created for this worlfile entry
  g_hash_table_insert( entitytable, (gpointer)entity, mod );

  // store the new model's blockgroup so we can find it for later blocks
  //g_hash_table_insert( blockgroups_by_entity, (gpointer)entity, mod->blockgroup );
}
  
// delete a model from the hash table
static void destroy_sregion( gpointer dummy1, SuperRegion* sr, gpointer dummy2 )
{
  free(sr);
}

void World::Load( const char* worldfile_path )
{
  // note: must call Unload() before calling Load() if a world already
  // exists TODO: unload doesn't clean up enough right now

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
														  this,
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
      else if( strcmp( typestr, "block" ) == 0 )
		  LoadBlock( wf, entity, entitytable );
		// 		else if( strcmp( typestr, "puck" ) == 0 )
		// 		  LoadPuck( wf, entity, entitytable );
		else
		  LoadModel( wf, entity, entitytable );
    }
  

  // warn about unused WF lines
  wf->WarnUnused();
	
  // now we're done with the worldfile entry lookup
  g_hash_table_destroy( entitytable );
	
  LISTMETHOD( children, Model*, InitRecursive );
	
  stg_usec_t load_end_time = RealTimeNow();

  if( debug )
    printf( "[Load time %.3fsec]\n", 
				(load_end_time - load_start_time) / 1000000.0 );
  else
    putchar( '\n' );
}

// delete a model from the hash table
static void destroy_model( gpointer dummy1, Model* mod, gpointer dummy2 )
{
  free(mod);
}


void World::UnLoad()
{
  if( wf ) delete wf;

  g_list_foreach( children, (GFunc)destroy_model, NULL );
  g_list_free( children );
  children = NULL;
	
  g_hash_table_remove_all( models_by_name );
		
  g_list_free( reentrant_update_list );
  reentrant_update_list = NULL;

  g_list_free( nonreentrant_update_list );
  nonreentrant_update_list = NULL;
	
  g_list_free( ray_list );
  ray_list = NULL;
	
  g_hash_table_foreach( superregions, (GHFunc)destroy_sregion, NULL );
  g_hash_table_remove_all( superregions );

  token = NULL;
}

stg_usec_t World::RealTimeNow()
{
  // TODO - move to GLib's portable timing code, so we can port to Windows more easily?
  struct timeval tv;
  gettimeofday( &tv, NULL );  // slow system call: use sparingly

  return( tv.tv_sec*1000000 + tv.tv_usec );
}

stg_usec_t World::RealTimeSinceStart()
{
  stg_usec_t timenow = RealTimeNow();

  // subtract the start time from the current time to get the elapsed
  // time

  return timenow - real_time_start;
}


bool World::PastQuitTime() 
{ 
  return( (quit_time > 0) && (sim_time >= quit_time) ); 
}

std::string World::ClockString()
{
  const uint32_t usec_per_hour   = 3600000000U;
  const uint32_t usec_per_minute = 60000000U;
  const uint32_t usec_per_second = 1000000U;
  const uint32_t usec_per_msec = 1000U;
	
  uint32_t hours   = sim_time / usec_per_hour;
  uint32_t minutes = (sim_time % usec_per_hour) / usec_per_minute;
  uint32_t seconds = (sim_time % usec_per_minute) / usec_per_second;
  uint32_t msec    = (sim_time % usec_per_second) / usec_per_msec;
	
  std::string str;  
  char buf[256];

  if( hours > 0 )
	 {
		snprintf( buf, 255, "%uh", hours );
		str += buf;
	 }

  snprintf( buf, 255, " %um %02us %03umsec", minutes, seconds, msec);
  str += buf;
  
  return str;
}

void World::AddUpdateCallback( stg_world_callback_t cb, 
										 void* user )
{
  // add the callback & argument to the list
  std::pair<stg_world_callback_t,void*> p(cb, user);
  cb_list.push_back( p );
}

int World::RemoveUpdateCallback( stg_world_callback_t cb, 
											void* user )
{
  std::pair<stg_world_callback_t,void*> p( cb, user );
  
  std::list<std::pair<stg_world_callback_t,void*> >::iterator it;  
  for( it = cb_list.begin();
		 it != cb_list.end();
		 it++ )
	 {
		if( (*it) == p )
		  {
			 cb_list.erase( it );		
			 break;
		  }
	 }
   
  // return the number of callbacks now in the list. Useful for
  // detecting when the list is empty.
  return cb_list.size();
}

void World::CallUpdateCallbacks()
{
  
  // for each callback in the list
  for( std::list<std::pair<stg_world_callback_t,void*> >::iterator it = cb_list.begin();
		 it != cb_list.end();
		 it++ )
	 {  
		//printf( "cbs %p data %p cvs->next %p\n", cbs, cbs->data, cbs->next );
		
		if( ((*it).first )( this, (*it).second ) )
		  {
			 //printf( "callback returned TRUE - schedule removal from list\n" );
			 it = cb_list.erase( it );
		  }
	 }      
}

bool World::Update()
{
  PRINT_DEBUG( "World::Update()" );
  
  // if we've run long enough, exit
  if( PastQuitTime() ) {
    if( IsGUI() == false )
      return true;		
  }
  
  dirty = true; // need redraw 

  // upate all positions first
  LISTMETHOD( velocity_list, Model*, UpdatePose );
  
  // test all models that supply charge to see if they are touching
  // something that takes charge
  LISTMETHOD( charge_list, Model*, UpdateCharge );
  
  // then update all models on the update lists
  LISTMETHOD( nonreentrant_update_list, Model*, UpdateIfDue );
  
  if( worker_threads == 0 ) // do all the work in this thread
    {
      LISTMETHOD( reentrant_update_list, Model*, UpdateIfDue );
    }
  else // use worker threads
    {
      // push the update for every model that needs it into the thread pool
      for( GList* it = reentrant_update_list; it; it=it->next )
		  {
			 Model* mod = (Model*)it->data;
			 
			 if( mod->UpdateDue()  )
				{
				  // printf( "updating model %s in WORKER thread\n", mod->Token() );
				  g_mutex_lock( thread_mutex );
				  update_jobs_pending++;
				  g_mutex_unlock( thread_mutex );						 
				  g_thread_pool_push( threadpool, mod, NULL );
				}
		  }	
		
      // wait for all the last update job to complete - it will
      // signal the worker_threads_done condition var
      g_mutex_lock( thread_mutex );
      while( update_jobs_pending )
		  g_cond_wait( worker_threads_done, thread_mutex );
      g_mutex_unlock( thread_mutex );		 

		// now call all the callbacks - ignores dueness, but not a big deal
      LISTMETHOD( reentrant_update_list, Model*, CallUpdateCallbacks );
    }
  
  if( show_clock && ((this->updates % show_clock_interval) == 0) )
	 {
		printf( "\r[Stage: %s]", ClockString().c_str() );
		fflush( stdout );
	 }

  CallUpdateCallbacks();
  
  this->updates++;  
  this->sim_time += this->interval_sim;
	
  return false;
}


void World::AddModel( Model*  mod  )
{
  g_hash_table_insert( this->models_by_name, (gpointer)mod->Token(), mod );
}

Model* World::GetModel( const char* name )
{
  PRINT_DEBUG1( "looking up model name %s in models_by_name", name );
  Model* mod = (Model*)g_hash_table_lookup( this->models_by_name, name );

  if( mod == NULL )
    PRINT_WARN1( "lookup of model name %s: no matching name", name );

  return mod;
}

void World::RecordRay( double x1, double y1, double x2, double y2 )
{  
  float* drawpts = new float[4];
  drawpts[0] = x1;
  drawpts[1] = y1;
  drawpts[2] = x2;
  drawpts[3] = y2;
  ray_list = g_list_append( ray_list, drawpts );
}

void World::ClearRays()
{
  for( GList* it = ray_list; it; it=it->next )
    {
      float* pts = (float*)it->data;
      delete [] pts;
    }

  g_list_free(ray_list );
  ray_list = NULL;
}


void World::Raytrace( std::vector<Ray>& rays )
{
  for( std::vector<Ray>::iterator it = rays.begin();
 		 it != rays.end();
 		 ++it )					 
 	 {
 		Ray& r = *it;
 		r.result = Raytrace( r.origin, r.range, r.func, r.mod, r.arg, r.ztest );
 	 }
}

void World::Raytrace( const Pose &gpose, // global pose
							 const stg_meters_t range,
							 const stg_radians_t fov,
							 const stg_ray_test_func_t func,
							 const Model* model,			 
							 const void* arg,
							 stg_raytrace_result_t* samples, // preallocated storage for samples
							 const uint32_t sample_count, // number of samples
							 const bool ztest ) 
{
  // find the direction of the first ray
  Pose raypose = gpose;
  double starta = fov/2.0 - raypose.a;

  for( uint32_t s=0; s < sample_count; s++ )
    {
		raypose.a = (s * fov / (double)sample_count) - starta;
      samples[s] = Raytrace( raypose, range, func, model, arg, ztest );
    }
}


// inline functions for converting from global cell coordinates to
// superregions, regions and local cells

inline int32_t SUPERREGION( const int32_t x )
{ 
  return( x >> SRBITS );
}

inline int32_t REGION( const int32_t x )
{  
  const int32_t _region_coord_mask = ~ ( ( ~ 0x00 ) << SRBITS );
  return( ( x & _region_coord_mask ) >> RBITS );
}

inline int32_t CELL( const int32_t x )
{  
  const int32_t _cell_coord_mask = ~ ( ( ~ 0x00 ) << RBITS );
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

stg_raytrace_result_t World::Raytrace( const Pose &gpose, 
													const stg_meters_t range,
													const stg_ray_test_func_t func,
													const Model* mod,		
													const void* arg,
													const bool ztest ) 
{
  stg_raytrace_result_t sample;
  
  //   printf( "raytracing at [ %.2f %.2f %.2f %.2f ] for %.2f \n",
  // 			 gpose.x,
  // 			 gpose.y,
  // 			 gpose.z,
  // 			 gpose.a,
  // 			 range );
  
  // initialize the sample
  sample.pose = gpose;
  sample.pose.a = normalize( sample.pose.a );
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

//   // the number of regions we will travel through
//   int32_t rdx = dx / Region::WIDTH;
//   int32_t rdy = dy / Region::WIDTH;
	
  //   if( finder->debug )
  //     RecordRay( pose.x, 
  // 	       pose.y, 
  // 	       pose.x + range.max * cos(pose.a),
  // 	       pose.y + range.max * sin(pose.a) );
	
  // fast integer line 3d algorithm adapted from Cohen's code from
  // Graphics Gems IV

  // cell unti
  int sx = sgn(dx);   // sgn() is a fast macro
  int sy = sgn(dy);  
  int ax = abs(dx); 
  int ay = abs(dy);  
  int bx = 2*ax;	
  int by = 2*ay;	
  int exy = ay-ax; 
  int n = ax+ay;

//   // region units
//   int rsx = sgn(rdx);   // sgn() is a fast macro
//   int rsy = sgn(rdy);  
//   int rax = abs(rdx); 
//   int ray = abs(rdy);  
//   int rbx = 2*rax;	
//   int rby = 2*ray;	
//   int rexy = ray-rax; 
//   int rn = rax+ray;

  //  printf( "Raytracing from (%d,%d) steps (%d,%d) %d\n",
  //  x,y,  dx,dy, n );
	
  // superregion coords
  stg_point_int_t lastsup( INT_MAX, INT_MAX );
  stg_point_int_t lastreg( INT_MAX, INT_MAX );
	
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
			 
			 // if the region coordinate has changed since the last loop
			 if( reg.x != lastreg.x || reg.y != lastreg.y )
				{
				  // lookup the new region
				  r = sr->GetRegion( reg.x, reg.y );
				  lastreg = reg;
				  nonempty_region = ( r && r->count );
				}

			 if( nonempty_region )
				{
				  //printf( "C[ %d %d ]\t", cell.x, cell.y );
				  
				  Cell* c = r->GetCellNoCreate( cell.x, cell.y );
				  assert(c);
				  
				  for( std::list<Block*>::iterator it = c->blocks.begin();
						 it != c->blocks.end();
						 ++it )					 
					 {	      	      
						Block* block = *it;
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
						if( (*func)( block->mod, (Model*)mod, arg )) 
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

static int _save_cb( Model* mod, void* dummy )
{
  mod->Save();
  return 0;
}

bool World::Save( const char *filename )
{
  ForEachDescendant( _save_cb, NULL );

  return this->wf->Save( filename );
}

static int _reload_cb(  Model* mod, void* dummy )
{
  mod->Load();
  return 0;
}

// reload the current worldfile
void World::Reload( void )
{
  ForEachDescendant( _reload_cb, NULL );
}

SuperRegion* World::AddSuperRegion( const stg_point_int_t& sup )
{
  //printf( "Creating super region [ %d %d ]\n", sup.x, sup.y );
  SuperRegion* sr = CreateSuperRegion( sup );

  // the bounds of the world have changed
  stg_point3_t pt;
  pt.x = (sup.x << SRBITS) / ppm;
  pt.y = (sup.y << SRBITS) / ppm;
  pt.z = 0;
  //printf( "lower left (%.2f,%.2f,%.2f)\n", pt.x, pt.y, pt.z );
  Extend( pt ); // lower left corner of the new superregion
  
  pt.x = ((sup.x+1) << SRBITS) / ppm;
  pt.y = ((sup.y+1) << SRBITS) / ppm;
  pt.z = 0;
  //printf( "top right (%.2f,%.2f,%.2f)\n", pt.x, pt.y, pt.z );
  Extend( pt ); // top right corner of the new superregion
  
  return sr;
}


inline SuperRegion* World::GetSuperRegionCached( const stg_point_int_t& sup )
{
  // around 99% of the time the SR is the same as last
  // lookup - cache  gives a 4% overall speed up :)
  
  if( sr_cached && sr_cached->origin.x == sup.x && sr_cached->origin.y == sup.y )
    return sr_cached;
  //else
  return GetSuperRegion( sup);
}

inline SuperRegion* World::GetSuperRegion( const stg_point_int_t& sup )
{
  //printf( "SUP[ %d %d ] ", sup.x, sup.y );

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

inline Cell* World::GetCell( const int32_t x, const int32_t y )
{
  stg_point_int_t glob;
  glob.x = x;
  glob.y = y;

  //printf( "GC[ %d %d ] ", glob.x, glob.y );

  return( GetSuperRegionCached( SUPERREGION(glob) )
			 ->GetRegion( REGION(x), REGION(y) )
			 ->GetCellCreate( CELL(x), CELL(y) )) ;
}

void World::ForEachCellInPolygon( const stg_point_t pts[], 
											 const unsigned int pt_count,
											 stg_cell_callback_t cb,
											 void* cb_arg )
{
  for( unsigned int i=0; i<pt_count; i++ )
	 ForEachCellInLine( pts[i], pts[(i+1)%pt_count], cb, cb_arg );

}
  
void World::ForEachCellInLine( const stg_point_t& pt1,
										 const stg_point_t& pt2,
										 stg_cell_callback_t cb,
										 void* cb_arg )
{  
  int32_t x = MetersToPixels( pt1.x ); // global pixel coords
  int32_t y = MetersToPixels( pt1.y );
  
  int32_t dx = MetersToPixels( pt2.x - pt1.x );
  int32_t dy = MetersToPixels( pt2.y - pt1.y );
  
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

void World::Extend( stg_point3_t pt )
{
  extent.x.min = MIN( extent.x.min, pt.x);
  extent.x.max = MAX( extent.x.max, pt.x );
  extent.y.min = MIN( extent.y.min, pt.y );
  extent.y.max = MAX( extent.y.max, pt.y );
  extent.z.min = MIN( extent.z.min, pt.z );
  extent.z.max = MAX( extent.z.max, pt.z );
}


void World::AddPowerPack( PowerPack* pp )
{
  powerpack_list = g_list_append( powerpack_list, pp ); 
}

void World::RemovePowerPack( PowerPack* pp )
{
  powerpack_list = g_list_remove( powerpack_list, pp ); 
}

/// Register an Option for pickup by the GUI
void World:: RegisterOption( Option* opt )
{
  g_hash_table_insert( option_table, (void*)opt->htname, opt );
}

void World::StartUpdatingModel( Model* mod )
{ 
  //if( ! g_list_find( update_list, mod ) )		  
  // update_list = g_list_append( update_list, mod ); 
  
  if( mod->thread_safe )
	 {
		if( ! g_list_find( reentrant_update_list, mod ) )		  
		  reentrant_update_list = g_list_append( reentrant_update_list, mod ); 
	 }
  else
	 {
		if( ! g_list_find( nonreentrant_update_list, mod ) )		  
		  nonreentrant_update_list = g_list_append( nonreentrant_update_list, mod ); 
	 }
}

void World::StopUpdatingModel( Model* mod )
{ 
  // update_list = g_list_remove( update_list, mod ); 
  
  if( mod->thread_safe )
	 reentrant_update_list = g_list_remove( reentrant_update_list, mod ); 
  else
	 nonreentrant_update_list = g_list_remove( nonreentrant_update_list, mod ); 		
}

void World::StartUpdatingModelPose( Model* mod )
{ 
  if( ! g_list_find( velocity_list, mod ) )
	 velocity_list = g_list_append( velocity_list, mod ); 
}

void World::StopUpdatingModelPose( Model* mod )
{ 
  // TODO XX figure out how to handle velcoties a bit better
  //velocity_list = g_list_remove( velocity_list, mod ); 
}

stg_usec_t World::SimTimeNow(void)
{ return sim_time; }

void World::Log( Model* mod )
{
  LogEntry( sim_time, mod);

  printf( "log entry count %lu\n", LogEntry::Count() );
  //LogEntry::Print();
}
 

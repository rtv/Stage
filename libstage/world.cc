
/** @defgroup world Worlds

Stage simulates a 'world' composed of `models', defined in a `world
file'. 

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
world
(
   name            "[filename of worldfile]"
   interval_real   100
   interval_sim    100
   gui_interval    100
   resolution      0.01
)
@endverbatim

@par Details
- name [string]
  - the name of the world, as displayed in the window title bar. Defaults to the worldfile file name.
- interval_sim [milliseconds]
  - the length of each simulation update cycle in milliseconds.
- interval_real [milliseconds]
  - the amount of real-world (wall-clock) time the siulator will attempt to spend on each simulation cycle.
- gui_interval [milliseconds]
  - the amount of real-world time between GUI updates
- resolution [meters]
  - specifies the resolution of the underlying bitmap model. Larger values speed up raytracing at the expense of fidelity in collision detection and sensing. 

@par More examples

The Stage source distribution contains several example world files in
<tt>(stage src)/worlds</tt> along with the worldfile properties
described on the manual page for each model type.

*/

#define _GNU_SOURCE

//#define DEBUG 

#include <stdlib.h>
#include <assert.h>
#include <string.h> // for strdup(3)
#include <locale.h> 
#include <glib-object.h> // fior g_type_init() used by GDKPixbuf objects

#include "stage_internal.hh"

const double STG_DEFAULT_WORLD_PPM = 50;  // 2cm pixels
const stg_msec_t STG_DEFAULT_WORLD_INTERVAL_REAL = 100; ///< real time between updates
const stg_msec_t STG_DEFAULT_WORLD_INTERVAL_SIM = 100;  ///< duration of sim timestep

// TODO: fix the quadtree code so we don't need a world size
const stg_meters_t STG_DEFAULT_WORLD_WIDTH = 20.0;
const stg_meters_t STG_DEFAULT_WORLD_HEIGHT = 20.0; 

// static data members
unsigned int StgWorld::next_id = 0;
bool StgWorld::quit_all = false;

// todo: get rid of width and height

StgWorld::StgWorld( void )
{
  Initialize( "MyWorld",
	      STG_DEFAULT_WORLD_INTERVAL_SIM, 
	      STG_DEFAULT_WORLD_INTERVAL_REAL,
	      STG_DEFAULT_WORLD_PPM,
	      STG_DEFAULT_WORLD_WIDTH,
	      STG_DEFAULT_WORLD_HEIGHT );
}  

StgWorld::StgWorld( const char* token, 
 		    stg_msec_t interval_sim, 
 		    stg_msec_t interval_real,
 		    double ppm, 
 		    double width,
 		    double height )
{
  Initialize( token, interval_sim, interval_real, ppm, width, height );
}


void StgWorld::Initialize( const char* token, 
			   stg_msec_t interval_sim, 
			   stg_msec_t interval_real,
			   double ppm, 
			   double width,
			   double height ) 
{
  if( ! Stg::InitDone() )
    {
      PRINT_WARN( "Stg::Init() must be called before a StgWorld is created." );
      exit(-1);
    }
  
  this->id = StgWorld::next_id++;
  this->ray_list = NULL;
  this->quit_time = 0; 
  
  assert(token);
  this->token = (char*)malloc(Stg::TOKEN_MAX);
  snprintf( this->token, Stg::TOKEN_MAX, "%s", token );//this->id );

  this->quit = false;
  this->updates = 0;
  this->wf = NULL;
  this->graphics = false; // subclasses that provide GUIs should
			  // change this

  this->models_by_id = g_hash_table_new( g_direct_hash, g_direct_equal );
  this->models_by_name = g_hash_table_new( g_str_hash, g_str_equal );
  this->sim_time = 0;
  this->interval_sim = 1e3 * interval_sim;
  this->interval_real = 1e3 * interval_real;

  this->real_time_start = RealTimeNow();
  this->real_time_next_update = 0;

  this->update_list = NULL;
  this->velocity_list = NULL;

  this->width = width;
  this->height = height;
  this->ppm = ppm; // this is the finest resolution of the matrix
  
  this->bgrid = new StgBlockGrid( (uint32_t)(width * ppm),
				  (uint32_t)(height * ppm) );
  
  this->total_subs = 0;
  this->paused = true; 
  this->destroy = false;   

  for( unsigned int i=0; i<INTERVAL_LOG_LEN; i++ )
    this->interval_log[i] = this->interval_real;
  this->real_time_now = 0;
}

StgWorld::~StgWorld( void )
{
  PRINT_DEBUG2( "destroying world %d %s", id, token );
  
  delete wf;
  delete bgrid;

  g_hash_table_destroy( models_by_id );
  g_hash_table_destroy( models_by_name );
  
  // TODO : finish clean up
}


void StgWorld::RemoveModel( StgModel* mod )
{
  g_hash_table_remove( models_by_id, mod );  
  g_hash_table_remove( models_by_name, mod );  
}

void StgWorld::ClockString( char* str, size_t maxlen )
{
  uint32_t hours   = sim_time / 3600000000; 
  uint32_t minutes = (sim_time % 3600000000) / 60000000; 
  uint32_t seconds = (sim_time % 60000000) / 1000000; 
  uint32_t msec    = (sim_time % 1000000) / 1000; 
  
  // find the average length of the last few realtime intervals;
  stg_usec_t average_real_interval = 0;
  for( uint32_t i=0; i<INTERVAL_LOG_LEN; i++ )
    average_real_interval += interval_log[i];
  average_real_interval /= INTERVAL_LOG_LEN;
  
  double localratio = (double)interval_sim / (double)average_real_interval;
  
#ifdef DEBUG
  if( hours > 0 )
    snprintf( str, maxlen, "Time: %uh%02um%02u.%03us\t[%.6f]\tsubs: %d  %s",
	      hours, minutes, seconds, msec,
	      localratio,
	      total_subs,
	      paused ? "--PAUSED--" : "" );
  else
    snprintf( str, maxlen, "Time: %02um%02u.%03us\t[%.6f]\tsubs: %d  %s",
	      minutes, seconds, msec,
	      localratio,
	      total_subs,
	      paused ? "--PAUSED--" : "" );
#else
  if( hours > 0 )
    snprintf( str, maxlen, "%uh%02um%02u.%03us\t[%.2f] %s",
	      hours, minutes, seconds, msec,
	      localratio,
	      paused ? "--PAUSED--" : "" );
  else
    snprintf( str, maxlen, "%02um%02u.%03us\t[%.2f] %s",
	      minutes, seconds, msec,
	      localratio,
	      paused ? "--PAUSED--" : "" );
#endif
}

void StgWorld::Load( const char* worldfile_path )
{
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
  
  this->interval_real = 1e3 *  
    wf->ReadInt( entity, "interval_real", this->interval_real/1e3 );

  this->interval_sim = 1e3 * 
    wf->ReadInt( entity, "interval_sim", this->interval_sim/1e3 );
  
  if( wf->PropertyExists( entity, "quit_time" ) )
    this->quit_time = 1e6 * 
      wf->ReadInt( entity, "quit_time", 0 );
  
  this->ppm = 
    1.0 / wf->ReadFloat( entity, "resolution", this->ppm ); 
  
  this->width = 
    wf->ReadTupleFloat( entity, "size", 0, this->width ); 
  
  this->height = 
    wf->ReadTupleFloat( entity, "size", 1, this->height ); 
  
  this->paused = 
    wf->ReadInt( entity, "paused", this->paused );

  if( this->bgrid )
    delete bgrid;

  int32_t vwidth = (int32_t)(width * ppm);
  int32_t vheight = (int32_t)(height * ppm);
  this->bgrid = new StgBlockGrid( vwidth, vheight );

  //_stg_disable_gui = wf->ReadInt( entity, "gui_disable", _stg_disable_gui );
     
  // Iterate through entitys and create client-side models
  for( entity = 1; entity < wf->GetEntityCount(); entity++ )
    {
      char *typestr = (char*)wf->GetEntityType(entity);      	  
      
      // don't load window entries here
      if( strcmp( typestr, "window" ) == 0 )
	  continue;

      int parent_entity = wf->GetEntityParent( entity );
      
      PRINT_DEBUG2( "wf entity %d parent entity %d\n", 
      	    entity, parent_entity );
      
      StgModel *mod, *parent;
      
      parent = GetModel( parent_entity );
      
      // find the creator function pointer in the hash table
      stg_creator_t creator = (stg_creator_t) 
	g_hash_table_lookup( Stg::Typetable(), typestr );
      
      // if we found a creator function, call it
      if( creator )
	{
	  //printf( "creator fn: %p\n", creator );
	  mod = (*creator)( this, parent, entity, typestr );
	}
      else
	{
	  PRINT_ERR1( "Unknown model type %s in world file.", 
		      typestr ); 
	  exit( 1 );
	}
      
      // configure the model with properties from the world file
      mod->Load();
    }
  
  // warn about unused WF linesa
  wf->WarnUnused();

  stg_usec_t load_end_time = RealTimeNow();

  printf( "[Load time %.3fsec]", (load_end_time - load_start_time) / 1000000.0 );
  
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

void StgWorld::PauseUntilNextUpdateTime( void )
{
  // sleep until it's time to update  
  stg_usec_t timenow = RealTimeSinceStart();
  
  /*  printf( "\ntimesincestart %llu interval_real %llu interval_sim %llu real_time_next_update %llu\n",
	  timenow,
	  interval_real,
	  interval_sim,
	  real_time_next_update );
  */

  if( timenow < real_time_next_update )
    {
      usleep( real_time_next_update - timenow );
    }

  interval_log[updates%INTERVAL_LOG_LEN] = timenow - real_time_now;

  real_time_now = timenow;
  real_time_next_update += interval_real;
}

bool StgWorld::Update()
{
  //PRINT_DEBUG( "StgWorld::Update()" );

  if( paused )
    return false;
  
  // update any models that are due to be updated
  for( GList* it=this->update_list; it; it=it->next )
    ((StgModel*)it->data)->UpdateIfDue();
  
  // update any models with non-zero velocity
  for( GList* it=this->velocity_list; it; it=it->next )
    ((StgModel*)it->data)->UpdatePose();
  
  this->sim_time += this->interval_sim;
  this->updates++;
  
  // if we've run long enough, set the quit flag
  if( (quit_time > 0) && (sim_time >= quit_time) )
    quit = true;
  
  return true;
}

bool StgWorld::RealTimeUpdate()
  
{
  //PRINT_DEBUG( "StageWorld::RealTimeUpdate()" );  
  bool updated = Update();
  PauseUntilNextUpdateTime();
  
  return updated;
}

void StgWorld::AddModel( StgModel*  mod  )
{
  //PRINT_DEBUG3( "World %s adding model %d %s to hash tables ", 
  //        token, mod->id, mod->Token() );  

  g_hash_table_insert( this->models_by_id, (gpointer)mod->Id(), mod );
  AddModelName( mod );
}


void StgWorld::AddModelName( StgModel* mod )
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

StgModel* StgWorld::GetModel( const stg_id_t id )
{
  PRINT_DEBUG1( "looking up model id %d in models_by_id", id );
  return (StgModel*)g_hash_table_lookup( this->models_by_id, (gpointer)id );
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

// raytrace wrapper that returns the model hit, rather than the block
// stg_meters_t StgWorld::Raytrace( StgModel* finder,
// 				 stg_pose_t* pose,
// 				 stg_meters_t max_range,
// 				 stg_block_match_func_t func,
// 				 const void* arg,
// 				 StgModel** hit_model )
// {
//   StgBlock* block = NULL;
//   stg_meters_t m = RayTrace( finder, pose, max_range, func, arg, &block );
  
//   if( block && hit_model )
//     *hit_model = block->Model();
  
//   return range;
// }


void StgWorld::Raytrace( stg_pose_t pose, // global pose
			 stg_meters_t range,
			 stg_radians_t fov,
			 stg_block_match_func_t func,
			 StgModel* model,			 
			 const void* arg,
			 stg_raytrace_sample_t* samples, // preallocated storage for samples
			 uint32_t sample_count )  // number of samples
{
  pose.a -= fov/2.0; // direction of first ray
  stg_radians_t angle_incr = fov/(double)sample_count; 
  
  for( uint32_t s=0; s < sample_count; s++ )
    {
      Raytrace( pose, range, func, model, arg, &samples[s] );
      pose.a += angle_incr;       
    }
}


void StgWorld::Raytrace( stg_pose_t pose, // global pose
			 stg_meters_t range,
			 stg_block_match_func_t func,
			 StgModel* mod,		
			 const void* arg,
			 stg_raytrace_sample_t* sample ) 
{
  // initialize the sample
  memcpy( &sample->pose, &pose, sizeof(stg_pose_t)); // pose stays fixed
  sample->range = range; // we might change this below
  sample->block = NULL; // we might change this below

  // find the global integer bitmap address of the ray  
  int32_t x = (int32_t)((pose.x+width/2.0)*ppm);
  int32_t y = (int32_t)((pose.y+height/2.0)*ppm);
  int32_t z = 0;

  int32_t xstart = x;
  int32_t ystart = y;

  // and the x and y offsets of the ray
  int32_t dx = (int32_t)(ppm*range * cos(pose.a));
  int32_t dy = (int32_t)(ppm*range * sin(pose.a));
  int32_t dz = 0;

//   if( finder->debug )
//     RecordRay( pose.x, 
// 	       pose.y, 
// 	       pose.x + range.max * cos(pose.a),
// 	       pose.y + range.max * sin(pose.a) );
	   
  // fast integer line 3d algorithm adapted from Cohen's code from
  // Graphics Gems IV
  int n, sx, sy, sz, exy, exz, ezy, ax, ay, az, bx, by, bz;  
  sx = sgn(dx);  sy = sgn(dy);  sz = sgn(dz);
  ax = abs(dx);  ay = abs(dy);  az = abs(dz);
  bx = 2*ax;	 by = 2*ay;	bz = 2*az;
  exy = ay-ax;   exz = az-ax;	ezy = ay-az;
  n = ax+ay+az;

  uint32_t bbx_last = 0;
  uint32_t bby_last = 0;
  uint32_t bbx = 0;
  uint32_t bby = 0;
  
  const uint32_t NBITS = bgrid->numbits;

  bool lookup_list = true;
  
  while ( n-- ) 
    {          
      // todo - avoid calling this multiple times for a single
      // bigblock.
      
      bbx = x>>NBITS;
      bby = y>>NBITS;
      
      // if we just changed grid square
      if( ! ((bbx == bbx_last) && (bby == bby_last)) )
	{
	  // if this square has some contents, we need to look it up
	  lookup_list = !(bgrid->BigBlockOccupancy(bbx,bby) < 1 );

	  bbx_last = bbx;
	  bby_last = bby;
	}
      
      if( lookup_list )
	{	  
	  for( GSList* list = bgrid->GetList(x,y);
	       list;
	       list = list->next )      
	    {	      	      
	      StgBlock* block = (StgBlock*)list->data;       
	      assert( block );
	      
	      // if this block does not belong to the searching model and it
	      // matches the predicate and it's in the right z range
	      if( //block && (block->Model() != finder) && 
		  //block->IntersectGlobalZ( pose.z ) &&
		  (*func)( block, mod, arg ) )
		{
		  // a hit!
		  //if( hit_model )
		  //*hit_model = block->Model();	      
		  sample->block = block;
		  sample->range = hypot( (x-xstart)/ppm, (y-ystart)/ppm );
		  return;
		}
	    }
	}
	  
      if ( exy < 0 ) {
	if ( exz < 0 ) {
	  x += sx;
	  exy += by; exz += bz;
	}
	else  {
	  z += sz;
	  exz -= bx; ezy += by;
	}
      }
      else {
	if ( ezy < 0 ) {
	  z += sz;
	  exz -= bx; ezy += by;
	}
	else  {
	  y += sy;
	  exy -= bx; ezy -= bz;
	}
      }
    }

  // hit nothing
  return;
}

static void _save_cb( gpointer key, gpointer data, gpointer user )
{
  ((StgModel*)data)->Save();
}

void StgWorld::Save( void )
{
  // ask every model to save itself
  //g_hash_table_foreach( this->models_by_id, stg_model_save_cb, NULL );
  
  ForEachModel( _save_cb, NULL );

  this->wf->Save(NULL);
}

static void _reload_cb( gpointer key, gpointer data, gpointer user )
{
  ((StgModel*)data)->Load();
}

// reload the current worldfile
void StgWorld::Reload( void )
{
  // can't reload the file yet - need to hack on the worldfile class. 
  //wf->load( NULL ); 
  
  // ask every model to load itself from the file database
  //g_hash_table_foreach( this->models_by_id, stg_model_reload_cb, NULL );

  ForEachModel( _reload_cb, NULL );
}

void StgWorld::StartUpdatingModel( StgModel* mod )
{
  if( g_list_find( this->update_list, mod ) == NULL )
    this->update_list = g_list_append( this->update_list, mod ); 
}

void StgWorld::StopUpdatingModel( StgModel* mod )
{
  this->update_list = g_list_remove( this->update_list, mod ); 
}

void StgWorld::MetersToPixels( stg_meters_t mx, stg_meters_t my, 
			       int32_t *px, int32_t *py )
{
  *px = (int32_t)floor((mx+width/2.0) * ppm);
  *py = (int32_t)floor((my+height/2.0) * ppm);
}

int StgWorld::AddBlockPixel( int x, int y, int z,
			     stg_render_info_t* rinfo )
{
  rinfo->world->bgrid->AddBlock( x, y, rinfo->block );
  return FALSE;
}


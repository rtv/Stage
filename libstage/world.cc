




/** @addtogroup stage
    @{ */

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

/**@}*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <string.h> // for strdup(3)
#include <locale.h> 

#define DEBUG 

#include "stage.hh"

const double STG_DEFAULT_WORLD_PPM = 50;  // 2cm pixels
const stg_msec_t STG_DEFAULT_WORLD_INTERVAL_REAL = 100; ///< real time between updates
const stg_msec_t STG_DEFAULT_WORLD_INTERVAL_SIM = 100;  ///< duration of sim timestep
const stg_msec_t STG_DEFAULT_MAX_SLEEP = 20;

// TODO: fix the quadtree code so we don't need a world size
const stg_meters_t STG_DEFAULT_WORLD_WIDTH = 20.0;
const stg_meters_t STG_DEFAULT_WORLD_HEIGHT = 20.0; 

// static data members
bool StgWorld::init_done = false;
unsigned int StgWorld::next_id = 0;
GHashTable* StgWorld::typetable = NULL;
bool StgWorld::quit_all = false;



typedef	StgModel* (*stg_creator_t)(StgWorld*, 
				   StgModel*, 
				   stg_id_t id, 
				   char* typestr );


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

void StgWorld::Init( int* argc, char** argv[] )
{
  PRINT_DEBUG( "StgWorld::Init()" );

  g_type_init();
  
  if(!setlocale(LC_ALL,"POSIX"))
    PRINT_WARN("Failed to setlocale(); config file may not be parse correctly\n" );
  
  StgWorld::typetable = stg_create_typetable();      
  StgWorld::init_done = true;  
}


void StgWorld::Initialize( const char* token, 
			   stg_msec_t interval_sim, 
			   stg_msec_t interval_real,
			   double ppm, 
			   double width,
			   double height ) 
{
  if( ! StgWorld::init_done )
    {
      PRINT_WARN( "StgWorld::Init() must be called before a StgWorld is created." );
      exit(-1);
    }
  
  this->id = StgWorld::next_id++;
  
  assert(token);
  this->token = (char*)malloc(STG_TOKEN_MAX);
  snprintf( this->token, STG_TOKEN_MAX, "%s:%d", token, this->id );

  this->quit = false;
  this->updates = 0;
  this->wf = NULL;

  this->models_by_id = g_hash_table_new( g_int_hash, g_int_equal );
  this->models_by_name = g_hash_table_new( g_str_hash, g_str_equal );
  this->sim_time = 0;
  this->interval_sim = interval_sim;
  this->interval_real = interval_real;
  this->interval_sleep_max = STG_DEFAULT_MAX_SLEEP;
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
  this->paused = false; 
  this->destroy = false;   
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
  long unsigned int days = this->sim_time / (24*3600000);
  long unsigned int hours = this->sim_time / 3600000;
  long unsigned int minutes = (this->sim_time % 3600000) / 60000;
  long unsigned int seconds = (this->sim_time % 60000) / 1000; // seconds
  long unsigned int msec =  this->sim_time % 1000; // milliseconds
  
  if( days > 0 )
    snprintf( str, maxlen, "Time: %lu:%lu:%02lu:%02lu.%03lu\tsubs: %d  %s",
	      days, hours, minutes, seconds, msec,
	      this->total_subs,
	      this->paused ? "--PAUSED--" : "" );
  else if( hours > 0 )
    snprintf( str, maxlen, "Time: %lu:%02lu:%02lu.%03lu\tsubs: %d  %s",
	      hours, minutes, seconds, msec,
	      this->total_subs,
	      this->paused ? "--PAUSED--" : "" );
  else
    snprintf( str, maxlen, "Time: %02lu:%02lu.%03lu\tsubs: %d  %s",
	      minutes, seconds, msec,
	      this->total_subs,
	      this->paused ? "--PAUSED--" : "" );
}

void StgWorld::Load( const char* worldfile_path )
{
  printf( " [Loading %s]", worldfile_path );      
  fflush(stdout);

  this->wf = new CWorldFile();
  wf->Load( worldfile_path );
  PRINT_DEBUG1( "wf has %d entitys", wf->GetEntityCount() );

  // end the output line of worldfile components
  puts("");

  int entity = 0;
  
  this->token = (char*)
    wf->ReadString( entity, "name", token );
  
  this->interval_real = 
    wf->ReadInt( entity, "interval_real", this->interval_real );

  this->interval_sim = 
    wf->ReadInt( entity, "interval_sim", this->interval_sim );
  
  this->interval_sleep_max = 
    wf->ReadInt( entity, "interval_sleep_max", this->interval_sleep_max );    

  this->ppm = 
    1.0 / wf->ReadFloat( entity, "resolution", this->ppm ); 
  
  this->width = 
    wf->ReadTupleFloat( entity, "size", 0, this->width ); 
  
  this->height = 
    wf->ReadTupleFloat( entity, "size", 1, this->height ); 
  
  
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
      
      //PRINT_DEBUG2( "wf entity %d parent entity %d\n", 
      //	    entity, parent_entity );
      
      StgModel *mod, *parent;
      
      parent = (StgModel*)
	g_hash_table_lookup( this->models_by_id, &parent_entity );
      
      // find the creator function pointer in the hash table
      stg_creator_t creator = (stg_creator_t) 
	g_hash_table_lookup( typetable, typestr );
      
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
  //wf->WarnUnused();
}

void StgWorld::Stop()
{
  this->paused = true;
}

void StgWorld::Start()
{
  this->paused = false;

  wf->WarnUnused();
}

stg_msec_t StgWorld::RealTimeNow(void)
{
  struct timeval tv;
  gettimeofday( &tv, NULL );  // slow system call: use sparingly
  return (stg_msec_t)( tv.tv_sec*1000 + tv.tv_usec/1000 );
}

stg_msec_t StgWorld::RealTimeSinceStart(void)
{
  stg_msec_t timenow = RealTimeNow();
  stg_msec_t diff = timenow - real_time_start;

  //PRINT_DEBUG3( "timenow %lu start %lu diff %lu", timenow, real_time_start, diff );

  return diff;
  //return( RealTimeNow() - real_time_start );
}

void StgWorld::PauseUntilNextUpdateTime( void )
{
  // sleep until it's time to update  
  stg_msec_t timenow = 0;
  for( timenow = RealTimeSinceStart();
       timenow < real_time_next_update;
       timenow = RealTimeSinceStart() )
    {
      stg_msec_t sleeptime = real_time_next_update - timenow; // must be >0
      
      PRINT_DEBUG3( "timenow %lu nextupdate %lu sleeptime %lu",
		    timenow, real_time_next_update, sleeptime );
      
      usleep( sleeptime * 1000 ); // sleep for few microseconds
    }
  
#ifdef DEBUG     
  printf( "[%u %lu %lu] ufreq:%.2f\n",
	  this->id, 
	  this->sim_time,
	  this->updates,
	  //this->interval_sim,
	  //this->real_interval_measured,
	  //(double)this->interval_sim / (double)this->real_interval_measured,
	  this->updates/(timenow/1e3));
  
  fflush(stdout);
#endif
  
  real_time_next_update = timenow + interval_real;      
}

bool StgWorld::Update()
{
  if( !paused )
    {  
      //PRINT_DEBUG( "StgWorld::Update()" );
      
      // update any models that are due to be updated
      for( GList* it=this->update_list; it; it=it->next )
	((StgModel*)it->data)->UpdateIfDue();
      
      // update any models with non-zero velocity
      for( GList* it=this->velocity_list; it; it=it->next )
	((StgModel*)it->data)->UpdatePose();
      
      this->sim_time += this->interval_sim;
      this->updates++;
    }

  return !( quit || quit_all );
}

bool StgWorld::RealTimeUpdate()
  
{
  //PRINT_DEBUG( "StageWorld::RealTimeUpdate()" );  
  bool ok = Update();
  
  if( ok )
    PauseUntilNextUpdateTime();
  
  return ok;
}



void StgWorld::AddModel( StgModel*  mod  )
{
  //PRINT_DEBUG3( "World %s adding model %d %s to hash tables ", 
  //        token, mod->id, mod->Token() );  

  g_hash_table_insert( this->models_by_id, &mod->id, mod );
  g_hash_table_insert( this->models_by_name, (gpointer)mod->Token(), mod );
}


// void stg_world_print( StgWorld* world )
// {
//   printf( " world %d:%s (%d models)\n", 
// 	  world->id, 
// 	  world->token,
// 	  g_hash_table_size( world->models ) );
  
//    g_hash_table_foreach( world->models, model_print_cb, NULL );
// }

// void world_print_cb( gpointer key, gpointer value, gpointer user )
// {
//   stg_world_print( (StgWorld*)value );
// }

StgModel* StgWorld::GetModel( const char* name )
{
  //printf( "looking up model name %s in models_by_name\n", name );
  return (StgModel*)g_hash_table_lookup( this->models_by_name, name );
}

StgModel* StgWorld::GetModel( const stg_id_t id )
{
  //printf( "looking up model name %s in models_by_name\n", name );
  return (StgModel*)g_hash_table_lookup( this->models_by_id, (gpointer)id );
}


// int raytest( int x, int y, int z,
// 	     void* arg1, void* arg2 )
// {
//   glRecti( x,y, x+1,y+1 );
//   return FALSE;
// }

typedef struct
{
  StgWorld* world;
  StgModel* finder;
  StgModel* hit;
  int32_t x, y, z; // leave point
  stg_block_match_func_t func;
  const void* arg;
} _raytrace_info_t;

int raytest( int32_t x, int32_t y, int32_t z,
	     _raytrace_info_t* rti )
{
  //glRecti( x,y, x+1,y+1 );

   // for each block recorded at his location
   for( GSList* list = rti->world->bgrid->GetList( x, y );
        list;
        list = list->next )
     {
       StgBlock* block = (StgBlock*)list->data;       
       assert( block );
       
       // if this block does not belong to the searching model and it
       // matches the predicate and it's in the right z range
       if( block && (block->mod != rti->finder) && 
	   (*rti->func)( block, rti->arg ) )//&&
	 //z >= block->zmin &&
	 // z < block->zmax )	 
	 {
 	  // a hit!
 	  rti->hit = block->mod;
 	  rti->x = x;
 	  rti->y = y;
 	  rti->z = z;	  
 	  return TRUE;
 	}
     }
  
  return FALSE;  
}
 
	    
stg_meters_t StgWorld::Raytrace( StgModel* finder,
				 stg_pose_t* pose,
				 stg_meters_t max_range,
				 stg_block_match_func_t func,
				 const void* arg,
				 StgModel** hit_model )
{
  // find the global integer bitmap address of the ray  
  int32_t x = (int32_t)((pose->x+width/2.0)*ppm);
  int32_t y = (int32_t)((pose->y+height/2.0)*ppm);
  
  // and the x and y offsets of the ray
  int32_t dx = (int32_t)(ppm*max_range * cos(pose->a));
  int32_t dy = (int32_t)(ppm*max_range * sin(pose->a));
  
  //glPushMatrix();
  //glTranslatef( -width/2.0, -height/2.0, 0 );
  //glScalef( 1.0/ppm, 1.0/ppm, 0 );
	   
  _raytrace_info_t rinfo;
  rinfo.world = this;
  rinfo.finder = finder;
  rinfo.func = func;
  rinfo.arg = arg;
  rinfo.hit = NULL;
  
  if( stg_line_3d( x, y, 0, 
   		   dx, dy, 0,
   		   (stg_line3d_func_t)raytest,
   		   &rinfo ) )
    {
      //glPopMatrix();      

      *hit_model = rinfo.hit;

      // how far away was that strike?
      return hypot( (rinfo.x-x)/ppm, (rinfo.y-y)/ppm );
    }


  //glPopMatrix();
  // return the range from ray start to object strike
  
  // hit nothing, so return max range
  return max_range;
}


void stg_model_save_cb( gpointer key, gpointer data, gpointer user )
{
  ((StgModel*)data)->Save();
}

void stg_model_reload_cb( gpointer key, gpointer data, gpointer user )
{
  ((StgModel*)data)->Load();
}

void StgWorld::Save( void )
{
  // ask every model to save itself
  g_hash_table_foreach( this->models_by_id, stg_model_save_cb, NULL );
  
  this->wf->Save(NULL);
}

// reload the current worldfile
void StgWorld::Reload( void )
{
  // can't reload the file yet - need to hack on the worldfile class. 
  //wf->load( NULL ); 
  
  // ask every model to load itself from the file database
  g_hash_table_foreach( this->models_by_id, stg_model_reload_cb, NULL );
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


void draw_voxel( int x, int y, int z )
{
  glTranslatef( x,y,z );
  
  glBegin(GL_QUADS);		// Draw The Cube Using quads
    glColor3f(0.0f,1.0f,0.0f);	// Color Blue
    glVertex3f( 1.0f, 1.0f,-1.0f);	// Top Right Of The Quad (Top)
    glVertex3f(-1.0f, 1.0f,-1.0f);	// Top Left Of The Quad (Top)
    glVertex3f(-1.0f, 1.0f, 1.0f);	// Bottom Left Of The Quad (Top)
    glVertex3f( 1.0f, 1.0f, 1.0f);	// Bottom Right Of The Quad (Top)
    glColor3f(1.0f,0.5f,0.0f);	// Color Orange
    glVertex3f( 1.0f,-1.0f, 1.0f);	// Top Right Of The Quad (Bottom)
    glVertex3f(-1.0f,-1.0f, 1.0f);	// Top Left Of The Quad (Bottom)
    glVertex3f(-1.0f,-1.0f,-1.0f);	// Bottom Left Of The Quad (Bottom)
    glVertex3f( 1.0f,-1.0f,-1.0f);	// Bottom Right Of The Quad (Bottom)
    glColor3f(1.0f,0.0f,0.0f);	// Color Red	
    glVertex3f( 1.0f, 1.0f, 1.0f);	// Top Right Of The Quad (Front)
    glVertex3f(-1.0f, 1.0f, 1.0f);	// Top Left Of The Quad (Front)
    glVertex3f(-1.0f,-1.0f, 1.0f);	// Bottom Left Of The Quad (Front)
    glVertex3f( 1.0f,-1.0f, 1.0f);	// Bottom Right Of The Quad (Front)
    glColor3f(1.0f,1.0f,0.0f);	// Color Yellow
    glVertex3f( 1.0f,-1.0f,-1.0f);	// Top Right Of The Quad (Back)
    glVertex3f(-1.0f,-1.0f,-1.0f);	// Top Left Of The Quad (Back)
    glVertex3f(-1.0f, 1.0f,-1.0f);	// Bottom Left Of The Quad (Back)
    glVertex3f( 1.0f, 1.0f,-1.0f);	// Bottom Right Of The Quad (Back)
    glColor3f(0.0f,0.0f,1.0f);	// Color Blue
    glVertex3f(-1.0f, 1.0f, 1.0f);	// Top Right Of The Quad (Left)
    glVertex3f(-1.0f, 1.0f,-1.0f);	// Top Left Of The Quad (Left)
    glVertex3f(-1.0f,-1.0f,-1.0f);	// Bottom Left Of The Quad (Left)
    glVertex3f(-1.0f,-1.0f, 1.0f);	// Bottom Right Of The Quad (Left)
    glColor3f(1.0f,0.0f,1.0f);	// Color Violet
    glVertex3f( 1.0f, 1.0f,-1.0f);	// Top Right Of The Quad (Right)
    glVertex3f( 1.0f, 1.0f, 1.0f);	// Top Left Of The Quad (Right)
    glVertex3f( 1.0f,-1.0f, 1.0f);	// Bottom Left Of The Quad (Right)
    glVertex3f( 1.0f,-1.0f,-1.0f);	// Bottom Right Of The Quad (Right)
  glEnd();			// End Drawing The Cube

  glTranslatef( -x, -y, -z );
}



// int stg_hash3d( int x, int y, int z )
// {
//   return (x+y+z);
// }


void StgWorld::MetersToPixels( stg_meters_t mx, stg_meters_t my, 
			       int32_t *px, int32_t *py )
{
  *px = (int32_t)floor((mx+width/2.0) * ppm);
  *py = (int32_t)floor((my+height/2.0) * ppm);
}

typedef struct
{
  StgBlockGrid* grid;
  StgBlock* block;
} _render_info_t;

int stg_add_block_to_grid( int x, int y, int z,
			   _render_info_t* rinfo )
{
  rinfo->grid->AddBlock( x, y, rinfo->block );
  return FALSE;
}


void StgWorld::MapBlock( StgBlock* block )
{
  stg_point_int_t* pts = block->pts_global;
  unsigned int pt_count = block->pt_count;
  
  assert( pt_count >=2 );
  
  _render_info_t rinfo;
  rinfo.grid = bgrid;
  rinfo.block - block;

  // TODO - could be a little bit faster - currently considers each
  // vertex twice
  for( unsigned int p=0; p<pt_count; p++ )
    {
      int32_t x = pts[p].x; // line start
      int32_t y = pts[p].y; // line end
      
      int32_t dx = pts[(p+1)%pt_count].x - x;
      int32_t dy = pts[(p+1)%pt_count].y - y;

      
      // pass this structure into our per-pixel callback
      _render_info_t rinfo;
      rinfo.grid = bgrid;
      rinfo.block = block;
      
      // ignore return value
      stg_line_3d( x, y, 0,
		   dx, dy, 0,
		   (stg_line3d_func_t)stg_add_block_to_grid, 
		   &rinfo );
    }
}


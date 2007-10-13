




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

  int32_t dx = width/2.0 * ppm;
  int32_t dy = height/2.0 * ppm;
  this->root = new StgCell( NULL, -dx, +dx, -dy, +dy );

  this->total_subs = 0;
  this->paused = false; 
  this->destroy = false;   
}

StgWorld::~StgWorld( void )
{
  PRINT_DEBUG2( "destroying world %d %s", id, token );
  
  delete root;
  delete wf;
  
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
  
  if( this->root )
    delete root;
  
  int32_t dx = width/2.0 * ppm;
  int32_t dy = height/2.0 * ppm;
  this->root = new StgCell( NULL, -dx, +dx, -dy, +dy );

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
  wf->WarnUnused();
}

void StgWorld::Stop()
{
  this->paused = true;
}

void StgWorld::Start()
{
  this->paused = false;
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

//	itl->a = atan2( b-y, a-x );
//itl->max_range = hypot( a-x, b-y );       



StgModel* StgWorld::TestCandidateList( StgModel* finder, 
				       stg_itl_test_func_t func,
				       stg_meters_t z,
				       GSList* list )
{
  assert(list);
  
  for( GSList* it = list; it; it=it->next )
    {
      StgBlock* block = (StgBlock*)it->data;		
      StgModel* candidate = block->mod;		
      assert( candidate );
      
      //printf( "block of %s\n", candidate->Token() );
      
      stg_pose_t gpose;
      candidate->GetGlobalPose( &gpose );
      
      // test to see if the block exists at height z
      double block_min = gpose.z + block->zmin;
      double block_max = gpose.z + block->zmax;
      
      //printf( "[%.2f %.2f %.2f] %s testing %s (laser return %d)\n",
      //      x,y,z,
      //      finder->Token(), candidate->Token(), candidate->LaserReturn() );
      
      
      if( LT(block_min,z) && 
	  GT(block_max,z) && 
	  (*func)( finder, candidate ) )
	return candidate;
    }

  return NULL;
}

stg_meters_t StgWorld::Raytrace( StgModel* finder,
				 stg_pose_t* pose,
				 stg_meters_t max_range,
				 stg_itl_test_func_t func,
				 StgModel** hit_model )
{
  const double epsilon = 0.0001;

  double x = pose->x;
  double y = pose->y;
  double z = pose->z;
  double a = pose->a;

  double tana = tan( a );
  
  StgCell* cell = root;
  double range = 0.0;

  StgModel* hit = NULL;

  while( LT(range,max_range) )
    {
      printf( "locating cell at (%.2f %.2f)\n", x, y );
      
      // locate the leaf cell at X,Y
      //cell = StgCell::Locate( cell, x, y );      
      cell = cell->Locate( x*ppm, y*ppm );

      // the cell is null iff the point was outside the root
      if( cell == NULL )
	{
	  printf( "ray at angle %.2f out of bounds at [%d, %d]",
		  a, x, y );

	  break;
	}
      
      if( finder->debug )
	{
	  PushColor( 1,0,0, 0.3 );
	  glPolygonMode( GL_FRONT, GL_LINE );
	  cell->Draw();
	  PopColor();
	}
      
      if( cell->list ) 
	{ 
	  hit = TestCandidateList( finder, func, z, cell->list );
	  if( hit )
	    break; // we're done looking	  
	}	    
      
      double c = y - tana * x; // line offset
      
      double xleave = x;
      double yleave = y;
      
      if( GT(a,0) ) // up
	{
	  // ray could leave through the top edge
	  // solve x for known y      
	  yleave = cell->ymax; // top edge
	  xleave = (yleave - c) / tana;
	  
	  // if the edge crossing was not in cell bounds     
	  if( !((xleave >= cell->xmin) && (xleave < cell->xmax)) )
	    {
	      // it must have left the cell on the left or right instead 
	      // solve y for known x
	      
	      if( GT(a,M_PI/2.0) ) // left side
		{
		  xleave = cell->xmin-epsilon;
		}
	      else // right side
		{
		  xleave = cell->xmax;
		}
	      
	      yleave = tana * xleave + c;
	    }
	}	 
      else 
	{
	  // ray could leave through the bottom edge
	  // solve x for known y      
	  yleave = cell->ymin; // bottom edge
	  xleave = (yleave - c) / tana;
	  
	  // if the edge crossing was not in cell bounds     
	  if( !((xleave >= cell->xmin) && (xleave < cell->xmax)) )
	    {
	      // it must have left the cell on the left or right instead 
	      // solve y for known x	  
	      if( LT(a,-M_PI/2.0) ) // left side
		{
		  xleave = cell->xmin-epsilon;
		}
	      else
		{
		  xleave = cell->xmax;
		}
	      
	      yleave = tana * xleave + c;
	    }
	  else
	    yleave-=epsilon;
	}

      cell->Print( NULL );
      printf( "(%.2f %.2f)\n", hypot( yleave - y, xleave - x ), range );
      
      // jump to the leave point
      range += hypot( yleave - y, xleave - x );
      
      if( finder->debug )
	{
	  PushColor( 0,0,0, 0.2 );
	  glBegin( GL_LINES );
	  glVertex3f( x,y, z );
	  glVertex3f( xleave, yleave, z );
	  glEnd();
	  PopColor();

	  glPointSize( 4 );

	  PushColor( 1,0,0, 1 );
	  glBegin( GL_POINTS );
	  glVertex3f( x,y, z );
	  glEnd();

	  PushColor( 0,0,1, 1 );
	  glBegin( GL_POINTS );
	  glVertex3f( xleave+0.01, yleave+0.01, z );
	  glEnd();
	  PopColor();

	  glPointSize( 1 );
	}

      x = xleave;
      y = yleave;      
    }
  
  if( hit )
    printf( "HIT %s at [%d %d %.2f] range %.2f\n",
	    hit->Token(),
	    x,y,z, range );
  else
    printf( "no hit at range %.2f\n", range );
  
  // if the caller supplied a place to store the hit model pointer,
  // give it to 'em.
  if( hit_model )
    *hit_model = hit;
  
  // return the range
  return MIN( range, max_range );
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

void StgWorld::MapBlock( StgBlock* block )
{
  stg_point_t* pts = block->pts_global;
  unsigned int pt_count = block->pt_count;
  
  assert( pt_count >=2 );
  
  for( unsigned int p=0; p<pt_count-1; p++ )
    MapBlockLine( block, 
		  pts[p].x, pts[p].y,
		  pts[p+1].x, pts[p+1].y );
  
  // close the loop
  MapBlockLine( block, 
		pts[0].x, pts[0].y,
		pts[pt_count-1].x, pts[pt_count-1].y );
}



StgCell* myPixel( StgCell* cell, StgBlock* block, long int x, long int y) 
{
  StgCell* found = cell->LocateAtom( x, y );
  
  if( found ) 
    {
      found->AddBlock( block );  
      cell = found;
    }
    
  return cell;
}


void StgWorld::MapBlockLine( StgBlock* block, 
			     double mx1, double my1, 
			     double mx2, double my2 )
{
  int32_t x1 = mx1 * ppm - 0.5;
  int32_t x2 = mx2 * ppm - 0.5;
  int32_t y1 = my1 * ppm - 0.5;
  int32_t y2 = my2 * ppm - 0.5;
  
  StgCell* cell = root;
  
  // Extremely Fast Line Algorithm Var A (Division)
  // Copyright 2001-2, By Po-Han Lin
  
  // Freely useable in non-commercial applications as long as credits 
  // to Po-Han Lin and link to http://www.edepot.com is provided in 
  // source code and can been seen in compiled executable.  
  // Commercial applications please inquire about licensing the algorithms.
  //
  // Lastest version at http://www.edepot.com/phl.html
  
  // constant string will appear in executable
  const char* EFLA_CREDIT = 
    "Contains an implementation of the Extremely Fast Line Algorithm (EFLA)"
    "by Po-Han Lin (http://www.edepot.com/phl.html)";
  
  bool yLonger=false;
  int incrementVal;
  
  int shortLen = y2-y1;
  int longLen = x2-x1;
  if( abs(shortLen) > abs(longLen) ) 
    {
      int swap=shortLen;
      shortLen=longLen;
      longLen=swap;
      yLonger=true;
    }
  
  if( longLen<0 ) 
    incrementVal=-1;
  else 
    incrementVal=1;
  
  double divDiff;
  if( shortLen == 0 ) 
    divDiff=longLen;
  else 
    divDiff=(double)longLen/(double)shortLen;
  
  if (yLonger)
    for (int i=0;i!=longLen;i+=incrementVal) 
      cell = myPixel( cell, block, x1+(int)((double)i/divDiff),y1+i);
  else
    for (int i=0;i!=longLen;i+=incrementVal) 
      cell = myPixel( cell, block, x1+i,y1+(int)((double)i/divDiff));

  // END OF EFLA code
}



#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>
#include <string.h> // for strdup(3)
#include <locale.h> 
//#include <GL/gl.h>

//#define DEBUG 

#include "stage.hh"

const double STG_DEFAULT_WORLD_PPM = 50.0;  // 2cm pixels
const stg_msec_t STG_DEFAULT_WORLD_INTERVAL_REAL = 100; // msec between updates
const stg_msec_t STG_DEFAULT_WORLD_INTERVAL_SIM = 100;  // duration of a simulation timestep in msec
const stg_msec_t STG_DEFAULT_MAX_SLEEP = 20;
const double STG_DEFAULT_WORLD_WIDTH = 20.0; // meters
const double STG_DEFAULT_WORLD_HEIGHT = 20.0; // meters

// static data members
bool StgWorld::init_done = false;
unsigned int StgWorld::next_id = 0;
GHashTable* StgWorld::typetable = NULL;
bool StgWorld::quit_all = false;

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


  //this->matrix = stg_matrix_create( ppm, width, height ); 
  // new - have matrix implemented directly in the world
  double sz = 1.0/ppm;
  while( sz < MAX(width,height) )
    sz *= 2.0;
  this->root = new StgCell( NULL, 0.0, 0.0, sz );

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
      
      usleep( sleeptime * 1e3 ); // sleep for few microseconds
    }
  
#if DEBUG     
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
  //printf( "World adding model %d %s to hash tables ", mod->id, mod->Token() );  
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


stg_meters_t StgWorld::Raytrace( StgModel* finder,
				 stg_pose_t* pose,
				 stg_meters_t max_range,
				 stg_itl_test_func_t func,
				 StgModel** hit_model )
{
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
      // locate the leaf cell at X,Y
      cell = StgCell::Locate( cell, x, y );      
      
      // the cell is null iff the point was outside the root
      if( cell == NULL )
	{
	  range = max_range; // stop the ray here
	  break;//return NULL;
	}
      
      //push_color_rgb( 0,1,0 );
      //double delta = cell->size/2.0;
      //glBegin(GL_LINE_LOOP);
      //glVertex3f( cell->x-delta, cell->y-delta, itl->z );
      //glVertex3f( cell->x+delta, cell->y-delta, itl->z );
      //glVertex3f( cell->x+delta, cell->y+delta, itl->z );
      //glVertex3f( cell->x-delta, cell->y+delta, itl->z );
      //glEnd();
      //pop_color();

      if( cell->list ) 
	{ 
	  for( GSList* it = cell->list; it; it=it->next )
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
	      
	      if( block_min < z && 
		  block_max > z && 
		  (*func)( finder, candidate ) )
		{
		  hit = candidate;
		  break;
		}
	    }
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
	  if( !(GTE(xleave,cell->xmin) && LT(xleave,cell->xmax)) )
	    {
	      // it must have left the cell on the left or right instead 
	      // solve y for known x
	      
	      if( GT(a,M_PI/2.0) ) // left side
		{
		  xleave = cell->xmin-0.00001;
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
	  if( !(GTE(xleave,cell->xmin) && LT(xleave,cell->xmax)) )
	    {
	      // it must have left the cell on the left or right instead 
	      // solve y for known x	  
	      if( LT(a,-M_PI/2.0) ) // left side
		{
		  xleave = cell->xmin-0.00001;
		}
	      else
		{
		  xleave = cell->xmax;
		}
	      
	      yleave = tana * xleave + c;
	    }
	  else
	    yleave-=0.00001;
	}
      
      // jump to the leave point
      range += hypot( yleave - y, xleave - x );
      
      x = xleave;
      y = yleave;      
    }
  
  //push_color_rgb( 0,1,0 );
  //glBegin( GL_LINES );
  //glVertex3f( x,y,z );
  //glVertex3f( itl->x, itl->y, z );
  //glEnd();
  //pop_color();

  // if the caller supplied a place to store the hit model pointer,
  // give it to 'em.
  if( hit_model )
    *hit_model = hit;

  // return the range
  return MAX( range, max_range );
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

void StgWorld::MapBlockLine( StgBlock* block, 
			     double x1, double y1, 
			     double x2, double y2 )
{
  //printf( "matrix line %.2f,%.2f to %.2f,%.2f\n",
  //      x1,y1, x2, y2 );
  
  // theta is constant so we compute it outside the loop
  double theta = atan2( y2-y1, x2-x1 );
  double m = tan(theta); // line gradient 
      
  StgCell* cell = root;
  double res = 1.0/ppm;
      
  while( (GTE(fabs(x2-x1),res) || GTE(fabs(y2-y1),res)) && cell )
    {
      /*printf( "step %d angle %.2f start (%.2f,%.2f) now (%.2f,%.2f) end (%.2f,%.2f)\n",
	step++,
	theta,
	lines[l].x1, lines[l].y1, 
	x1, y1,
	x2, y2 );
      */

      // find out where we leave the leaf node after x1,y1 and adjust
      // x1,y1 to point there
	  
      // locate the leaf cell at X,Y
      cell = StgCell::Locate( cell, x1, y1 );
      if( cell == NULL )
	break;
	  
      //stg_cell_print( cell, "" );

      // if the cell isn't small enough, we need to create children
      while( GT(cell->size,res) )
	{
	  cell->Split();
	      
	  // we have to drop down into a child. but which one?
	  // which quadrant are we in?
	  int index;
	  if( LT(x1,cell->x) )
	    index = LT(y1,cell->y) ? 0 : 2; 
	  else
	    index = LT(y1,cell->y) ? 1 : 3; 
	      
	  // now point to the correct child containing the point, and loop again
	  cell = cell->children[ index ];           
	}

      // now the cell small enough, we add the block here
      cell->AddBlock( block );
	  
      // now figure out where we leave this cell
	  
      /*	  printf( "point %.2f,%.2f is in cell %p at (%.2f,%.2f) size %.2f xmin %.2f xmax %.2f ymin %.2f ymax %.2f\n",
		  x1, y1, cell, cell->x, cell->y, cell->size, cell->xmin, cell->xmax, cell->ymin, cell->ymax );
      */

	   
      if( EQ(y1,y2) ) // horizontal line
	{
	  //puts( "horizontal line" );
	      
	  if( GT(x1,x2) ) // x1 gets smaller
	    x1 = cell->xmin-0.001; // left edge
	  else
	    x1 = cell->xmax; // right edge		
	}
      else if( EQ(x1,x2) ) // vertical line
	{
	  //puts( "vertical line" );
	      
	  if( GT(y1,y2) ) // y1 gets smaller
	    y1 = cell->ymin-0.001; // bottom edge
	  else
	    y1 = cell->ymax; // max edge		
	}
      else
	{
	  //print_thing( "\nbefore calc", cell, x1, y1 );

	  //break;
	  double c = y1 - m * x1; // line offset
	      
	  if( GT(theta,0.0) ) // up
	    {
	      //puts( "up" );

	      // ray could leave through the top edge
	      // solve x for known y      
	      y1 = cell->ymax; // top edge
	      x1 = (y1 - c) / m;
		  
	      // printf( "x1 %.4f = (y1 %.4f - c %.4f) / m %.4f\n", x1, y1, c, m );		  
	      // if the edge crossing was not in cell bounds     
	      if( !(GTE(x1,cell->xmin) && LT(x1,cell->xmax)) )
		{
		  // it must have left the cell on the left or right instead 
		  // solve y for known x
		      
		  if( GT(theta,M_PI/2.0) ) // left side
		    x1 = cell->xmin-0.00001;
		  //{ x1 = cell->xmin-0.001; puts( "left side" );}
		  else // right side
		    x1 = cell->xmax;
		  //{ x1 = cell->xmax; puts( "right side" );}

		  y1 = m * x1 + c;
		      
		  //  printf( "%.4f = %.4f * %.4f + %.4f\n",
		  //      y1, m, x1, c );
		}           
	      //else
	      //puts( "top" );
	    }	 
	  else // down 
	    {
	      //puts( "down" );

	      // ray could leave through the bottom edge
	      // solve x for known y      
	      y1 = cell->ymin-0.00001; // bottom edge
	      x1 = (y1 - c) / m;
		  
	      // if the edge crossing was not in cell bounds     
	      if( !(GTE(x1,cell->xmin) && LT(x1,cell->xmax)) )
		{
		  // it must have left the cell on the left or right instead 
		  // solve y for known x	  
		  if( theta < -M_PI/2.0 ) // left side
		    //{ x1 = cell->xmin-0.001; puts( "left side" );}
		    x1 = cell->xmin-0.00001;
		  else
		    //{ x1 = cell->xmax; puts( "right side"); }
		    x1 = cell->xmax; 
		      
		  y1 = m * x1 + c;
		}
	      //else
	      //puts( "bottom" );
	    }
	      
	  // jump slightly into the next cell
	  //x1 += 0.0001 * cos(theta);
	  //y1 += 0.0001 * sin(theta);      

	  //printf( "jumped to %.7f,%.7f\n",
	  //      x1, y1 );

	}
    }
}


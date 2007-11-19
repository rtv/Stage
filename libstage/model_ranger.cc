///////////////////////////////////////////////////////////////////////////
//
// File: model_ranger.cc
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_ranger.cc,v $
//  $Author: rtv $
//  $Revision: 1.1.2.4 $
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_ranger Ranger model 
The ranger model simulates an array of sonar or infra-red (IR) range sensors.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
ranger
(
  # ranger properties
  scount 16
  spose[0] [? ? ?]
  spose[1] [? ? ?]
  spose[2] [? ? ?]
  spose[3] [? ? ?]
  spose[4] [? ? ?]
  spose[5] [? ? ?]
  spose[6] [? ? ?]
  spose[7] [? ? ?]
  spose[8] [? ? ?]
  spose[9] [? ? ?]
  spose[10] [? ? ?]
  spose[11] [? ? ?]
  spose[12] [? ? ?]
  spose[13] [? ? ?]
  spose[14] [? ? ?]
  spose[15] [? ? ?]
   
  ssize [0.01 0.03]
  sview [0.0 5.0 5.0]

  # model properties
  watts 2.0
)
@endverbatim

@par Notes

The ranger model allows configuration of the pose, size and view parameters of each transducer seperately (using spose[index], ssize[index] and sview[index]). However, most users will set a common size and view (using ssize and sview), and just specify individual transducer poses.

@par Details
- scount int 
  - the number of range transducers
- spose[\<transducer index\>] [float float float]
  - [x y theta] 
  - pose of the transducer relative to its parent.
- ssize [float float]
  - [x y] 
  - size in meters. Has no effect on the data, but controls how the sensor looks in the Stage window.
- ssize[\<transducer index\>] [float float]
  - per-transducer version of the ssize property. Overrides the common setting.
- sview [float float float]
   - [range_min range_max fov] 
   - minimum range and maximum range in meters, field of view angle in degrees. Currently fov has no effect on the sensor model, other than being shown in the confgiuration graphic for the ranger device.
- sview[\<transducer index\>] [float float float]
  - per-transducer version of the sview property. Overrides the common setting.

*/

//#define DEBUG

#include <math.h>
#include "stage.hh"

#define STG_RANGER_WATTS 2.0 // ranger power consumption

StgModelRanger::StgModelRanger( StgWorld* world, 
				StgModel* parent,
				stg_id_t id,
				char* typestr )
  : StgModel( world, parent, id, typestr )
{
  PRINT_DEBUG2( "Constructing StgModelRanger %d (%s)\n", 
		id, typestr );
    
  // Set up sensible defaults
  
  stg_color_t col = stg_lookup_color( STG_RANGER_CONFIG_COLOR );
  this->SetColor( col );
  
  // remove the polygon: ranger has no body
  this->ClearBlocks();

  //stg_geom_t geom;
  //memset( &geom, 0, sizeof(geom)); // no size
  //this->SetGeom( &geom );
  
  sensor_count = 16;
  sensors= g_new0( stg_ranger_sensor_t, sensor_count );

  double offset = MIN(geom.size.x, geom.size.y) / 2.0;

  // create default ranger config
  for( unsigned int c=0; c<sensor_count; c++ )
    {
      sensors[c].pose.a = (2.0*M_PI)/sensor_count * c;
      sensors[c].pose.x = offset * cos( sensors[c].pose.a );
      sensors[c].pose.y = offset * sin( sensors[c].pose.a );
      
      sensors[c].size.x = 0.01; // a small device
      sensors[c].size.y = 0.04;
      
      sensors[c].bounds_range.min = 0;
      sensors[c].bounds_range.max = 5.0;
      sensors[c].fov = (2.0*M_PI)/(sensor_count+1);
      sensors[c].ray_count = 3;
    }
}

StgModelRanger::~StgModelRanger()
{
}

void StgModelRanger::Startup( void )
{
  StgModel::Startup();
  
  PRINT_DEBUG( "ranger startup" );
  
  this->SetWatts( STG_RANGER_WATTS );
}


void StgModelRanger::Shutdown( void )
{
  PRINT_DEBUG( "ranger shutdown" );

  this->SetWatts( 0 );
  //this->SetData( NULL, 0 ); // this will unrender data too.

  StgModel::Shutdown();
}

void StgModelRanger::Load( void )
{
  StgModel::Load();
  
  CWorldFile* wf = world->wf;

  if( wf->PropertyExists(id, "scount" ) )
    {
      PRINT_DEBUG( "Loading ranger array" );

      // Load the geometry of a ranger array
      sensor_count = wf->ReadInt( id, "scount", 0);
      assert( sensor_count > 0 );
      
      char key[256];
      sensors = g_renew( stg_ranger_sensor_t, sensors, sensor_count );
      
      stg_size_t common_size;
      common_size.x = wf->ReadTupleLength( id, "ssize", 0, sensors[0].size.x );
      common_size.y = wf->ReadTupleLength( id, "ssize", 1, sensors[0].size.y );
      
      double common_min = wf->ReadTupleLength( id, "sview", 0, sensors[0].bounds_range.min );
      double common_max = wf->ReadTupleLength( id, "sview", 1, sensors[0].bounds_range.max );
      double common_fov = wf->ReadTupleAngle( id, "sview", 2, sensors[0].fov );

      int common_ray_count = wf->ReadInt( id, "sraycount", sensors[0].ray_count );

      // set all transducers with the common settings
      for( unsigned int i = 0; i < sensor_count; i++)
	{
	  sensors[i].size.x = common_size.x;
	  sensors[i].size.y = common_size.y;
	  sensors[i].bounds_range.min = common_min;
	  sensors[i].bounds_range.max = common_max;
	  sensors[i].fov = common_fov;
	  sensors[i].ray_count = common_ray_count;	  
	}

      // TODO - do this properly, without the hard-coded defaults
      // allow individual configuration of transducers
      for( unsigned int i = 0; i < sensor_count; i++)
	{
	  snprintf(key, sizeof(key), "spose[%d]", i);
	  sensors[i].pose.x = wf->ReadTupleLength( id, key, 0, 0);
	  sensors[i].pose.y = wf->ReadTupleLength( id, key, 1, 0);
	  sensors[i].pose.a = wf->ReadTupleAngle( id, key, 2, 0);
	  
/* 	  snprintf(key, sizeof(key), "ssize[%d]", i); */
/* 	  sensors[i].size.x = wf->ReadTuplelength(mod->id, key, 0, 0.01); */
/* 	  sensors[i].size.y = wf->ReadTuplelength(mod->id, key, 1, 0.05); */
	  
/* 	  snprintf(key, sizeof(key), "sview[%d]", i); */
/* 	  sensors[i].bounds_range.min = */
/* 	    wf->ReadTuplelength(mod->id, key, 0, 0); */
/* 	  sensors[i].bounds_range.max =   // set up sensible defaults */

/* 	    wf->ReadTuplelength(mod->id, key, 1, 5.0); */
/* 	  sensors[i].fov */
/* 	    = DTOR(wf->ReadTupleangle(mod->id, key, 2, 5.0 )); */


/* 	  sensors[i].ray_count = common_ray_count; */

	}
      
      PRINT_DEBUG1( "loaded %d ranger configs", (int)sensor_count );	  
    }
}

int ranger_raytrace_match( StgBlock* block, StgModel* finder )
{
  // Ignore myself, my children, and my ancestors.
  return( block->mod->RangerReturn() && !block->mod->IsRelated( finder ) );
  //return true;
}	

void StgModelRanger::Update( void )
{     
  StgModel::Update();
  
  if( this->subs < 1 )
    return;
  
  if( (sensors == NULL) || (sensor_count < 1 ))
    return;

  //PRINT_DEBUG2( "[%d] updating ranger %s", (int)world->sim_time_ms, token );

  // raytrace new range data for all sensors
  for( unsigned int t=0; t<sensor_count; t++ )
    {
      // get the sensor's pose in global coords
      //stg_pose_t pz;
      //memcpy( &pz, &sensors[t].pose, sizeof(pz) ); 
      //this->LocalToGlobal( &pz );
      
      double range;
      
      int r;
      for( r=0; r<sensors[t].ray_count; r++ )
	{	  
	  //double angle_per_ray = sensors[t].fov / sensors[t].ray_count;
	  //double ray_angle = -sensors[t].fov/2.0 + angle_per_ray * r + angle_per_ray/2.0;
	  
	  range = Raytrace( &sensors[t].pose,
			    sensors[t].bounds_range.max,
			    (stg_block_match_func_t)ranger_raytrace_match,
			    this,
			    NULL );			       

// 	  // if the ray hits anything, 'range' will be changed
// 	  if( stg_first_model_on_ray( pz.x, pz.y, pz.z + 0.01, // TODO: UNHACK 
// 				      pz.a + ray_angle, 
// 				      sensors[t].bounds_range.max, 
// 				      world->matrix, 
// 				      PointToBearingRange, 
// 				      ranger_raytrace_match,
// 				      this,
// 				      &hitrange, &hitx, &hity ) )
// 	    {
// 	      if( hitrange < range )
// 		range = hitrange;
// 	    } 

	  
	  //double a = sensors[t].pose.a + ray_angle;
	  //double dx = range * cos(a);//' + range * sin(a);
	  //double dy = range * sin(a);// - range * cos(a);

	  //glBegin( GL_LINES );
	  //glVertex3f( sensors[t].pose.x, sensors[t].pose.y, 0 );
	  //glVertex3f( sensors[t].pose.x+dx, sensors[t].pose.y+dy, 0 );
	  //glEnd();
	  //pop_color();
	}
      // low-threshold the range
      if( range < sensors[t].bounds_range.min )
	range = sensors[t].bounds_range.min;	
      
      sensors[t].range = range;
      //sensors[t].error = TODO;
      
      
    }   
  
  //pop_color();
  //glEndList();

  // data graphics will be updated before being drawn onscreen again
  data_dirty = true;
}

/*
int ranger_noise_test( stg_ranger_sample_t* data, size_t count,  )
{
  int s;
  for( s=0; s<count; s++ )
    {
      // add 10mm random error
      ranges[s].range *= 0.1 * drand48();
    }
}
*/

void StgModelRanger::Print( char* prefix )
{
  StgModel::Print( prefix );
    
  printf( "\tRanges[ " );
  
  for( unsigned int i=0; i<sensor_count; i++ )
    printf( "%.2f ", sensors[i].range );
  puts( " ]" );
}


void render_rangers( stg_ranger_sensor_t* sensors, size_t sensor_count )
{
  for( unsigned int s=0; s<sensor_count; s++ ) 
    { 
      if( sensors[s].range > 0.0 ) 
	{ 
	  stg_ranger_sensor_t* rngr = &sensors[s]; 
	  
	  // sensor FOV 
	  double sidelen = sensors[s].range; 
	  double da = rngr->fov/2.0; 
	      
	  double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - da ); 
	  double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - da ); 
	  double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + da ); 
	  double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + da ); 
	  
	  //printf( "%.2f %.2f  to %.2f %.2f\n",
	  //      rngr->pose.x, rngr->pose.y, x1, y1 );
	  
	  glBegin( GL_POLYGON); 		
	  glVertex3f( rngr->pose.x, rngr->pose.y, 0 ); 
	  glVertex3f( x1, y1, 0 ); 
	  glVertex3f( x2, y2, 0 ); 
	  glEnd();	   	      
	} 
    } 
}

void StgModelRanger::DataVisualize( void )
{
  // recreate the display list for this data
  //glNewList( this->dl_data, GL_COMPILE );
  
  if( sensors && sensor_count ) 
    {
      //puts( "ranger graphics" );

      glDepthMask( GL_FALSE ); 

      // now get on with rendering the ranger data 
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		     //		     world->win->show_alpha ? GL_FILL : GL_LINES );
      PushColor( 0, 1, 0, 0.2 ); // transparent pale green 
      render_rangers( sensors, sensor_count );
      PopColor(); 

      // restore state 
      glDepthMask( GL_TRUE ); 
    }

  //glEndList();
}
   
   

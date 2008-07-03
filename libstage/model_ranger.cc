///////////////////////////////////////////////////////////////////////////
//
// File: model_ranger.cc
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_ranger.cc,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

/**
  @ingroup model
  @defgroup model_ranger Ranger model 
  The ranger model simulates an array of sonar or infra-red (IR) range sensors.

API: Stg::StgModelRanger

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

//#define DEBUG 1
#include "stage_internal.hh"
#include <math.h>

static const stg_watts_t DEFAULT_RANGER_WATTSPERSENSOR = 0.2;
static const stg_meters_t DEFAULT_RANGER_SIZEX = 0.01;
static const stg_meters_t DEFAULT_RANGER_SIZEY = 0.04;
static const stg_meters_t DEFAULT_RANGER_SIZEZ = 0.04;
static const stg_meters_t DEFAULT_RANGER_RANGEMAX = 5.0;
static const stg_meters_t DEFAULT_RANGER_RANGEMIN = 0.0;
static const unsigned int DEFAULT_RANGER_RAYCOUNT = 3;
static const unsigned int DEFAULT_RANGER_SENSORCOUNT = 16;

static const char RANGER_COLOR[] = "gray75";
static const char RANGER_CONFIG_COLOR[] = "gray90";
static const char RANGER_GEOM_COLOR[] = "orange";

Option StgModelRanger::showRangerData( "Show Ranger Data", "show_ranger", "", true );
Option StgModelRanger::showRangerTransducers( "Show Ranger transducer locations", "show_ranger_transducers", "", false );


StgModelRanger::StgModelRanger( StgWorld* world, 
										  StgModel* parent ) 
  : StgModel( world, parent, MODEL_TYPE_RANGER )
{
	PRINT_DEBUG2( "Constructing StgModelRanger %d (%s)\n", 
			id, typestr );

	// Set up sensible defaults

	stg_color_t col = stg_lookup_color( RANGER_CONFIG_COLOR );
	this->SetColor( col );

	// remove the polygon: ranger has no body
	this->ClearBlocks();

	stg_geom_t geom;
	memset( &geom, 0, sizeof(geom)); // no size
	this->SetGeom( geom );

	samples = NULL;
	sensor_count = DEFAULT_RANGER_SENSORCOUNT;
	sensors = new stg_ranger_sensor_t[sensor_count];

	double offset = MIN(geom.size.x, geom.size.y) / 2.0;

	// create default ranger config
	for( unsigned int c=0; c<sensor_count; c++ )
	{
		sensors[c].pose.a = (2.0*M_PI)/sensor_count * c;
		sensors[c].pose.x = offset * cos( sensors[c].pose.a );
		sensors[c].pose.y = offset * sin( sensors[c].pose.a );
		sensors[c].pose.z = 0;//geom.size.z / 2.0; // half way up

		sensors[c].size.x = DEFAULT_RANGER_SIZEX;
		sensors[c].size.y = DEFAULT_RANGER_SIZEY;
		sensors[c].size.z = DEFAULT_RANGER_SIZEZ;

		sensors[c].bounds_range.min = DEFAULT_RANGER_RANGEMIN;
		sensors[c].bounds_range.max = DEFAULT_RANGER_RANGEMAX;;
		sensors[c].fov = (2.0*M_PI)/(sensor_count+1);
		sensors[c].ray_count = DEFAULT_RANGER_RAYCOUNT;
	}
	
	registerOption( &showRangerData );
	registerOption( &showRangerTransducers );
}

StgModelRanger::~StgModelRanger()
{
}

void StgModelRanger::Startup( void )
{
	StgModel::Startup();

	PRINT_DEBUG( "ranger startup" );

	this->SetWatts( DEFAULT_RANGER_WATTSPERSENSOR * sensor_count );
}


void StgModelRanger::Shutdown( void )
{
	PRINT_DEBUG( "ranger shutdown" );

	this->SetWatts( 0 );

	if( this->samples )
	{
		delete[] samples;
		samples = NULL;
	}

	StgModel::Shutdown();
}

void StgModelRanger::Load( void )
{
	StgModel::Load();

	if( wf->PropertyExists( wf_entity, "scount" ) )
	{
		PRINT_DEBUG( "Loading ranger array" );

		// Load the geometry of a ranger array
		sensor_count = wf->ReadInt( wf_entity, "scount", 0);
		assert( sensor_count > 0 );

		char key[256];
		//sensors = g_renew( stg_ranger_sensor_t, sensors, sensor_count );
		delete [] sensors;
		sensors = new stg_ranger_sensor_t[sensor_count];

		stg_size_t common_size;
		common_size.x = wf->ReadTupleLength( wf_entity, "ssize", 0, DEFAULT_RANGER_SIZEX );
		common_size.y = wf->ReadTupleLength( wf_entity, "ssize", 1, DEFAULT_RANGER_SIZEY );

		double common_min = wf->ReadTupleLength( wf_entity, "sview", 0,  DEFAULT_RANGER_RANGEMIN );
		double common_max = wf->ReadTupleLength( wf_entity, "sview", 1, DEFAULT_RANGER_RANGEMAX );
		double common_fov = wf->ReadTupleAngle( wf_entity, "sview", 2, M_PI / (double)sensor_count );

		int common_ray_count = wf->ReadInt( wf_entity, "sraycount", sensors[0].ray_count );

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
			sensors[i].pose.x = wf->ReadTupleLength( wf_entity, key, 0, sensors[i].pose.x );
			sensors[i].pose.y = wf->ReadTupleLength( wf_entity, key, 1, sensors[i].pose.y );
			sensors[i].pose.z = 0.0;
			sensors[i].pose.a = wf->ReadTupleAngle( wf_entity, key, 2, sensors[i].pose.a );

			snprintf(key, sizeof(key), "spose3[%d]", i);
			sensors[i].pose.x = wf->ReadTupleLength( wf_entity, key, 0, sensors[i].pose.x );
			sensors[i].pose.y = wf->ReadTupleLength( wf_entity, key, 1, sensors[i].pose.y );
			sensors[i].pose.z = wf->ReadTupleLength( wf_entity, key, 2, sensors[i].pose.z );
			sensors[i].pose.a = wf->ReadTupleAngle( wf_entity, key, 3, sensors[i].pose.a );

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

static bool ranger_match( StgBlock* block, StgModel* finder, const void* dummy )
{
	//printf( "ranger match sees %s %p %d \n", 
	//    block->Model()->Token(), block->Model(), block->Model()->LaserReturn() );

	// Ignore myself, my children, and my ancestors.
	return( block->Model()->GetRangerReturn() && 
			!block->Model()->IsRelated( finder ) );
}	

void StgModelRanger::Update( void )
{     
	StgModel::Update();

	if( (sensors == NULL) || (sensor_count < 1 ))
		return;

	if( samples ==  NULL )
		samples = new stg_meters_t[sensor_count];
	assert( samples );

	//PRINT_DEBUG2( "[%d] updating ranger %s", (int)world->sim_time_ms, token );

	// raytrace new range data for all sensors
	for( unsigned int t=0; t<sensor_count; t++ )
	{

		// TODO - reinstate multi-ray rangers
		//for( int r=0; r<sensors[t].ray_count; r++ )
		//{	  
		stg_raytrace_sample_t ray;
		Raytrace( sensors[t].pose,
				sensors[t].bounds_range.max,
				ranger_match,
				NULL,
				&ray );

		samples[t] = MAX( ray.range, sensors[t].bounds_range.min );
		//sensors[t].error = TODO;
	}   
	}

// TODO: configurable ranger noise model
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
		printf( "%.2f ", samples[i] );
	puts( " ]" );
}

void StgModelRanger::DataVisualize( void )
{
	if( ! (samples && sensors && sensor_count) )
		return;

	if( showRangerTransducers )
	  {
		 glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		 PushColor( 0,0,0,1 );
		 
		 for( unsigned int s=0; s<sensor_count; s++ ) 
			{ 
			  if( samples[s] > 0.0 ) 
				 { 
					stg_ranger_sensor_t* rngr = &sensors[s];

					glPointSize( 4 );
					glBegin( GL_POINTS );
					glVertex3f( rngr->pose.x, rngr->pose.y, rngr->pose.z );
					glEnd();
				 }
			}
		 PopColor();
	  }

	if ( !showRangerData )
		return;
	
	// if all models have the same number of sensors, this is fast as
	// it will probably not use a system call
	static float* pts = NULL;
	size_t memsize =  9 * sensor_count * sizeof(float);
	pts = (float*)g_realloc( pts, memsize );
	bzero( pts, memsize );

	// calculate a triangle for each non-zero sensor range
	for( unsigned int s=0; s<sensor_count; s++ ) 
	{ 
		if( samples[s] > 0.0 ) 
		{ 
			stg_ranger_sensor_t* rngr = &sensors[s];

			//double dx =  rngr->size.x/2.0;
			//double dy =  rngr->size.y/2.0;
			//double dz =  rngr->size.z/2.0;


			// sensor FOV 
			double sidelen = samples[s];
			double da = rngr->fov/2.0;

			unsigned int index = s*9;
			pts[index+0] = rngr->pose.x;
			pts[index+1] = rngr->pose.y;
			pts[index+2] = rngr->pose.z;

			pts[index+3] = rngr->pose.x + sidelen*cos(rngr->pose.a - da );
			pts[index+4] = rngr->pose.y + sidelen*sin(rngr->pose.a - da );
			pts[index+5] = rngr->pose.z;

			pts[index+6] = rngr->pose.x + sidelen*cos(rngr->pose.a + da );
			pts[index+7] = rngr->pose.y + sidelen*sin(rngr->pose.a + da );
			pts[index+8] = rngr->pose.z;
		}
	}
	
	// draw the filled triangles in transparent blue
	glDepthMask( GL_FALSE );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	PushColor( 0, 1, 0, 0.1 ); // transparent pale green       
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, pts );
	glDrawArrays( GL_TRIANGLES, 0, 3 * sensor_count );

	// restore state 
	glDepthMask( GL_TRUE );
	PopColor();
}



///////////////////////////////////////////////////////////////////////////
//
// File: model_camera.cc
// Author: Alex Couture-Beil
// Date: 09 June 2008
//
// CVS info:
//
///////////////////////////////////////////////////////////////////////////


//#define DEBUG 1
#include "stage_internal.hh"
#include <sstream>


StgModelCamera::StgModelCamera( StgWorld* world, 
		StgModel* parent,
		stg_id_t id,
		char* typestr )
: StgModel( world, parent, id, typestr ),
_frame_data( NULL ), _frame_data_width( 0 ), _frame_data_height( 0 )
{
	PRINT_DEBUG2( "Constructing StgModelCamera %d (%s)\n", 
			id, typestr );

	StgWorldGui* world_gui = dynamic_cast< StgWorldGui* >( world );
	
	if( world_gui == NULL ) {
		printf( "Unable to use Camera Model - it must be run with a GUI world\n" );
		assert( 0 );
	}
	
	_canvas = world_gui->GetCanvas();
	
	// Set up sensible defaults

	SetColor( stg_lookup_color( "green" ) );

	stg_geom_t geom;
	memset( &geom, 0, sizeof(geom)); // no size
	//TODO can't draw this as it blocks the laser
	SetGeom( geom );

	Startup();
}

StgModelCamera::~StgModelCamera()
{
	if( _frame_data != NULL )
		delete _frame_data;
}

void StgModelCamera::Load( void )
{
	StgModel::Load();
	Worldfile* wf = world->GetWorldFile();
	int fov = wf->ReadLength( id, "fov",    -1 );
	if( fov > 0 ) {
		_camera.setFov( fov );
	}
	
}


void StgModelCamera::Update( void )
{   
	StgModel::Update();
}

float* StgModelCamera::laser()
{
	//TODO allow the h and w to be passed by user
	int h = 320;
	int w = 320;
	
	static GLfloat* data_gl = NULL;
	static GLfloat* data = NULL;
	if( data == NULL ) {
		data = new GLfloat[ h * w ];
		data_gl = new GLfloat[ h * w ];
		
	}
	
	glViewport( 0, 0, w, h );
	_camera.update();
	_camera.SetProjection( 1.0 );
	
	_camera.setPose( parent->GetGlobalPose().x, parent->GetGlobalPose().y, 0.3 );
	_camera.setYaw( rtod( parent->GetGlobalPose().a ) - 90.0 );
	_camera.Draw();

	_canvas->renderFrame( true );
	
	glReadPixels(0, h / 2, w, 1, GL_DEPTH_COMPONENT, GL_FLOAT, data_gl );

	for( int i = 0; i < w; i++ ) {
		data[ w-1-i ] = _camera.realDistance( data_gl[ i ] );
	}
	for( int i = 0; i < 32; i++ ) {
		for( int j = 1; j < 10; j++ )
			data[ i ] += data[ i * 10 + j ];
		data[ i ] /= 10.0;
	}
	_canvas->invalidate();
	return data;
}

const char* StgModelCamera::GetFrame( int width, int height, bool depth_buffer )
{
	static StgPerspectiveCamera camera; //shared between _ALL_ instances
	
	if( width == 0 || height == 0 )
		return NULL;
	
	if( width != _frame_data_width || height != _frame_data_height ) {
		if( _frame_data != NULL )
			delete _frame_data;
		_frame_data = new char[ 4 * width * height ]; //assumes a max of depth 4
	}

	
	glViewport( 0, 0, width, height );
	camera.update();
	camera.SetProjection( width, height, 0.0, 0.0 );
	camera.setPose( parent->GetGlobalPose().x, parent->GetGlobalPose().y, 0.1 );
	camera.setYaw( rtod( parent->GetGlobalPose().a ) - 90.0 );
	camera.Draw();
	
	_canvas->renderFrame( true );
	
	if( depth_buffer == true ) {
		glReadPixels(0, 0, width, height,
					 GL_DEPTH_COMPONENT, //GL_RGB,
					 GL_FLOAT, //GL_UNSIGNED_BYTE,
					 _frame_data );
	} else {
		glReadPixels(0, 0, width, height,
					 GL_RGB,
					 GL_UNSIGNED_BYTE,
					 _frame_data );		
	}

		
	_canvas->invalidate();
	return _frame_data;
}


void StgModelCamera::Draw( uint32_t flags, StgCanvas* canvas )
{
	StgModel::Draw( flags, canvas );
}


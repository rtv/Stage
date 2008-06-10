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

}


void StgModelCamera::Update( void )
{   
	StgModel::Update();
}


const char* StgModelCamera::GetFrame( int width, int height )
{
	static StgPerspectiveCamera camera; //shared between _ALL_ instances
	
	if( width == 0 || height == 0 )
		return NULL;
	
	if( width != _frame_data_width || height != _frame_data_height ) {
		if( _frame_data != NULL )
			delete _frame_data;
		_frame_data = new char[ 3 * width * height ];
	}
	
	glViewport( 0, 0, width, height );
	
	camera.update();
	camera.SetProjection( width, height, 0.01, 200.0 );
	camera.setPose( parent->GetGlobalPose().x, parent->GetGlobalPose().y, 0.1 );
	camera.setYaw( rtod( parent->GetGlobalPose().a ) - 90.0 );
	camera.Draw();
	
	_canvas->renderFrame( true );
	
	glReadPixels(0, 0, width, height,
				 GL_RGB,
				 GL_UNSIGNED_BYTE,
				 _frame_data );
	
	_canvas->invalidate();
	return _frame_data;
}


void StgModelCamera::Draw( uint32_t flags, StgCanvas* canvas )
{
	StgModel::Draw( flags, canvas );
}


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
_frame_data( NULL ), _frame_data_width( 0 ), _frame_data_height( 0 ), _width( 0 )
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

	_camera.setFov( wf->ReadLength( id, "fov", _camera.fov() ) );
	_camera.setYaw( wf->ReadLength( id, "yaw", _camera.yaw() ) );
	_camera.setPitch( wf->ReadLength( id, "pitch", _camera.pitch() ) );

	_width = wf->ReadLength( id, "width", _width );

	//TODO move to constructor
	_frame_data_width = _width;
	_frame_data_height = 100;
	
}


void StgModelCamera::Update( void )
{   
	GetFrame( _frame_data_width, _frame_data_height, true );
	
	StgModel::Update();
}

float* StgModelCamera::laser()
{
	return NULL;
//	//TODO allow the h and w to be passed by user
//	int w = _width;
//	int h = 1;
//	
//	static GLfloat* data_gl = NULL;
//	static GLfloat* data = NULL;
//	if( data == NULL ) {
//		data = new GLfloat[ h * w ];
//		data_gl = new GLfloat[ h * w ];
//		
//	}
//	
//	glViewport( 0, 0, w, h );
//	_camera.update();
//	_camera.SetProjection( w, h, 0.1, 40.0 );
//	
//	_camera.setPose( parent->GetGlobalPose().x, parent->GetGlobalPose().y, 0.3 );
//	_camera.setYaw( rtod( parent->GetGlobalPose().a ) - 90.0 );
//	_camera.Draw();
//
//	_canvas->renderFrame( true );
//	
//	glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, data_gl );
//
//	for( int i = 0; i < w; i++ ) {
//		data[ w-1-i ] = _camera.realDistance( data_gl[ i ]);
//	}
//
//	_canvas->invalidate();
//	return data;
}

const char* StgModelCamera::GetFrame( int width, int height, bool depth_buffer )
{
	static StgPerspectiveCamera camera; //shared between _ALL_ instances
	
	if( width == 0 || height == 0 )
		return NULL;
	
	if( width != _frame_data_width || height != _frame_data_height || _frame_data == NULL ) {
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
		//transform length into linear length
		float* data = ( float* )( _frame_data ); //TODO use static_cast here
		int buf_size = width * height;
		for( int i = 0; i < buf_size; i++ )
			data[ i ] = _camera.realDistance( data[ i ] ) + 0.1;
	} else {
		glReadPixels(0, 0, width, height,
					 GL_RGB,
					 GL_UNSIGNED_BYTE,
					 _frame_data );		
	}

		
	_canvas->invalidate();
	return _frame_data;
}

//TODO create lines outlineing camera frustrum, then iterate over each depth measurement and create a square
void StgModelCamera::DataVisualize( void )
{
	float w_fov = _camera.fov();
	float h_fov = _camera.fov();
	float length = 8.0;
	
	float start_fov = 90 + w_fov / 2.0;
	float end_fov = 90 - w_fov / 2.0;
	
	float x, y;
		
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //TODO this doesn't seem to work.

	float z = cos( dtor( h_fov / 2.0 ) ) * length;
	
	glColor4f(1.0, 0.0, 0.0, 1.0 );	
	
	const float* data = ( float* )( _frame_data ); //TODO use static_cast here
	int w = _frame_data_width, h = _frame_data_height;
	for( int i = 0; i < w; i++ ) {
		float z_a = h_fov / _width * static_cast< float >( i );
		for( int j = 0; j < h; j++ ) {
			float length = data[ i + j * w ];
			float a;
			float z = cos( z_a ) * length;
			//TODO rename j to not conflict with loop
			int j = i + 1;
			if( j < _width/2 ) 
				a = 90 - w_fov / _width * static_cast< float >( _width/2 - 1 - j );
			else
				a = 90 + w_fov / _width * static_cast< float >( j - _width/2 );
			x = sin( dtor( a ) ) * length;
			y = cos( dtor( a ) ) * length;
			glBegin( GL_LINE_LOOP );
			glVertex3f( 0.0, 0.0, 0.0 );
			glVertex3f( x, y, 0.0 );
			glEnd();
			
		}
	}
	
}

void StgModelCamera::Draw( uint32_t flags, StgCanvas* canvas )
{
	StgModel::Draw( flags, canvas );
}


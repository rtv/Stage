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
										  StgModel* parent ) 
  : StgModel( world, parent, MODEL_TYPE_CAMERA ),
_frame_data( NULL ), _width( 0 ), _height( 0 ), _yaw_offset( 0 )
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
	
	_camera.setFov( wf->ReadLength( wf_entity, "horizfov",  _camera.horizFov() ), wf->ReadLength( wf_entity, "vertfov",  _camera.vertFov() ) );
	_camera.setPitch( wf->ReadLength( wf_entity, "pitch", _camera.pitch() ) );

	_yaw_offset = wf->ReadLength( wf_entity, "yaw", _yaw_offset );
	_width = wf->ReadLength( wf_entity, "width", _width );
	_height = wf->ReadLength( wf_entity, "height", _height );
}


void StgModelCamera::Update( void )
{   
	GetFrame( true );
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

const char* StgModelCamera::GetFrame( bool depth_buffer )
{	
	if( _width == 0 || _height == 0 )
		return NULL;
	
	if( _frame_data == NULL ) {
		if( _frame_data != NULL )
			delete _frame_data;
		_frame_data = new char[ 4 * _width * _height ]; //assumes a max of depth 4
	}

	
	glViewport( 0, 0, _width, _height );
	_camera.update();
	_camera.SetProjection();
	_camera.setPose( parent->GetGlobalPose().x, parent->GetGlobalPose().y, 0.1 );
	_camera.setYaw( rtod( parent->GetGlobalPose().a ) - 90.0 + _yaw_offset ); //-90.0 points the camera infront of the robot instead of pointing right
	_camera.Draw();
	
	_canvas->renderFrame( true );
	
	if( depth_buffer == true ) {
		glReadPixels(0, 0, _width, _height,
					 GL_DEPTH_COMPONENT, //GL_RGB,
					 GL_FLOAT, //GL_UNSIGNED_BYTE,
					 _frame_data );
		//transform length into linear length
		float* data = ( float* )( _frame_data ); //TODO use static_cast here
		int buf_size = _width * _height;
		for( int i = 0; i < buf_size; i++ )
			data[ i ] = _camera.realDistance( data[ i ] ) + 0.1;
	} else {
		glReadPixels(0, 0, _width, _height,
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
	float w_fov = _camera.horizFov();
	float h_fov = _camera.vertFov();
	float length = 8.0;
	
	float start_fov = 90 + w_fov / 2.0;
	float end_fov = 90 - w_fov / 2.0;
	
	float x, y;
		
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //TODO this doesn't seem to work.

	float z = cos( dtor( h_fov / 2.0 ) ) * length;
	
	glColor4f(1.0, 0.0, 0.0, 1.0 );	
	
	const float* data = ( float* )( _frame_data ); //TODO use static_cast here
	int w = _width;
	int h = _height;
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


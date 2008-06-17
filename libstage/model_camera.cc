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
_frame_data( NULL ), _frame_color_data( NULL ), _vertexbuf( NULL ),  _width( 0 ), _height( 0 ), _yaw_offset( 0 )
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
	if( _frame_data != NULL ) {
		delete[] _frame_data;
		delete[] _frame_color_data;
		delete[] _vertexbuf;
	}
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
		if( _frame_data != NULL ) {
			delete[] _frame_data;
			delete[] _frame_color_data;
			delete[] _vertexbuf;	
		}
		_frame_data = new char[ 4 * _width * _height ]; //assumes a max of depth 4
		_frame_color_data = new char[ 3 * _width * _height ]; //for RGB
		_vertexbuf = new float[ 3 * _width * _height ]; //for RGB
	}

	
	glViewport( 0, 0, _width, _height );
	_camera.update();
	_camera.SetProjection();
	_camera.setPose( parent->GetGlobalPose().x, parent->GetGlobalPose().y, 0.1 );
	_camera.setYaw( rtod( parent->GetGlobalPose().a ) - 90.0 + _yaw_offset ); //-90.0 points the camera infront of the robot instead of pointing right
	_camera.Draw();
	
	_canvas->renderFrame( true );
	
	//read depth buffer
		glReadPixels(0, 0, _width, _height,
					 GL_DEPTH_COMPONENT, //GL_RGB,
					 GL_FLOAT, //GL_UNSIGNED_BYTE,
					 _frame_data );
		//transform length into linear length
		float* data = ( float* )( _frame_data ); //TODO use static_cast here
		int buf_size = _width * _height;
		for( int i = 0; i < buf_size; i++ )
			data[ i ] = _camera.realDistance( data[ i ] ) + 0.1;

	//read color buffer
	glReadPixels(0, 0, _width, _height,
				 GL_RGB,
				 GL_UNSIGNED_BYTE,
				 _frame_color_data );		


	_canvas->invalidate();
	return _frame_data;
}

//TODO create lines outlineing camera frustrum, then iterate over each depth measurement and create a square
void StgModelCamera::DataVisualize( void )
{
	if( _frame_data == NULL )
		return;
	
	float w_fov = _camera.horizFov();
	float h_fov = _camera.vertFov();
	
	float center_horiz = - _yaw_offset;
	float center_vert = 0; // - _pitch_offset;
	
	float start_fov = center_horiz + w_fov / 2.0; //start at right
	float start_vert_fov = center_vert + h_fov / 2.0; //start at top
		
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //TODO this doesn't seem to work.

//	float z = cos( dtor( h_fov / 2.0 ) ) * length;
	
	glColor4f(1.0, 0.0, 0.0, 1.0 );	
	
	static float foo = 0.0;
	foo += 1.0;
	
	const float* data = ( float* )( _frame_data ); //TODO use static_cast here
	int w = _width;
	int h = _height;
	float a;
	float vert_a;
	float a_space = w_fov / w; //degrees between each sample
	float vert_a_space = h_fov / h; //degrees between each vertical sample
	float x, y, z;
	float tmp_x, tmp_y, tmp_z;
	float *buf;

	glBegin( GL_POINTS );
	for( int i = 0; i < w; i++ ) {
		for( int j = 0; j < h; j++ ) {
			float length = data[ i + j * h ];
			
			a = start_fov - static_cast< float >( i ) * a_space;
			vert_a = start_vert_fov - static_cast< float >( j ) * vert_a_space;
			
			x = length;
			y = 0;
			z = 0;
			
			tmp_x = x * cos( dtor( a ) ); // - y *sin( but y=0)
			tmp_y = x * sin( dtor( a ) ); // + y * cos( but y=0 )
			tmp_z = z;
			
			x = tmp_x; y = tmp_y; z = tmp_z;
			
			tmp_z = - x * sin( dtor( vert_a ) );
			tmp_x = x * cos( dtor( vert_a ) );

			x = tmp_x; y = tmp_y; z = tmp_z;
			
			buf = _vertexbuf + ( i + j * h  ) * 3;
			
			buf[ 0 ] = x;
			buf[ 1 ] = y;
			buf[ 2 ] = z;
			
			//TODO push into a buffer
			//glVertex3f( x, y, z );

		}
	}
	glEnd();
	
	//TODO use vertex array buffer
	glEnableClientState( GL_VERTEX_ARRAY );

	glVertexPointer( 3, GL_FLOAT, 0, _vertexbuf );
	glDrawArrays( GL_POINTS, 0, _width * _height );
	
	glDisableClientState( GL_VERTEX_ARRAY );
	
//	for( int i = 0; i < w; i++ ) {
//		float z_a = h_fov / _width * static_cast< float >( i );
//		for( int j = 0; j < h; j++ ) {
//			float length = data[ i + j * w ];
//			float a;
//			//float z = cos( z_a ) * length;
//			//TODO rename j to not conflict with loop
//			int j = i + 1;
//			if( j < _width/2 ) 
//				a = center_horiz - w_fov / _width * static_cast< float >( _width/2 - 1 - j );
//			else
//				a = center_horiz + w_fov / _width * static_cast< float >( j - _width/2 );
//			x = sin( dtor( a ) ) * length;
//			y = cos( dtor( a ) ) * length;
//			glBegin( GL_LINE_LOOP );
//			glVertex3f( 0.0, 0.0, 0.0 );
//			glVertex3f( x, y, 0.0 );
//			glEnd();
//			
//		}
//	}
	
}

void StgModelCamera::Draw( uint32_t flags, StgCanvas* canvas )
{
	StgModel::Draw( flags, canvas );
}


///////////////////////////////////////////////////////////////////////////
//
// File: model_camera.cc
// Author: Alex Couture-Beil
// Date: 09 June 2008
//
// CVS info:
//
///////////////////////////////////////////////////////////////////////////

#define CAMERA_HEIGHT 0.5
#define CAMERA_NEAR_CLIP 0.2
#define CAMERA_FAR_CLIP 8.0

//#define DEBUG 1
#include "stage_internal.hh"
#include <sstream>
#include <iomanip>

Option StgModelCamera::showCameraData( "Show Camera Data", "show_camera", "", true );

/**
 @ingroup model
 @defgroup model_camera Camera model 
 The camera model simulates a camera
 
 API: Stg::StgModelCamera
 
 <h2>Worldfile properties</h2>
 
 @par Summary and default values
 
 @verbatim
 camera
 (
 # laser properties
 width 32
 height 32
 range_min 0.0
 range_max 8.0
 horizfov 60.0
 vertfov 60.0
 yaw 0.0
 pitch 90.0
 
 # model properties
 size [0.15 0.15]
 color "black"
 watts 100.0 # TODO find watts for sony pan-tilt camera
 )
 @endverbatim
 
 @par Details
 - width int
 - the number of pixel samples
 - height int
 - the number of pixel samples
 - range_min float
 -  the minimum range reported by the camera, in meters. Objects closer than this will not be displayed. The smaller the number, the less persision in depth - don't set this value too close to 0.
 - range_max float
 - the maximum range reported by the camera, in meters. Objects farther than this will not be displayed.
 - horizfov float
 - angle, in degrees, for the horizontal field of view.
 - vertfov float
 - angle, in degrees, for the vertical field of view.
 - yaw float
 - angle, in degrees, where the camera is panned to.
 - pitch float
 - angle, in degrees, where the camera is tilted to.
 */

//caclulate the corss product, and store results in the first vertex
void cross( float& x1, float& y1, float& z1, float x2, float y2, float z2 )
{	
	float x3, y3, z3;
	
	x3 = y1 * z2 - z1 * y2;
	y3 = z1 * x2 - x1 * z2;
	z3 = x1 * y2 - y1 * x2;
	
	x1 = x3;
	y1 = y3;
	z1 = z3;
}


StgModelCamera::StgModelCamera( StgWorld* world, StgModel* parent ) 
  : StgModel( world, parent, MODEL_TYPE_CAMERA ),
_canvas( NULL ),
_frame_data( NULL ),
_frame_color_data( NULL ),
_valid_vertexbuf_cache( false ),
_vertexbuf_cache( NULL ),
_width( 32 ),
_height( 32 ),
_camera_quads_size( 0 ),
_camera_quads( NULL ),
_camera_colors( NULL ),
_camera(),
_yaw_offset( 0.0 ),
_pitch_offset( 0.0 )

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
	_camera.setPitch( 90.0 );
	
	SetColor( stg_lookup_color( "green" ) );

	stg_geom_t geom;
	memset( &geom, 0, sizeof(geom)); // no size
	//TODO can't draw this as it blocks the laser
	SetGeom( geom );
	
	registerOption( &showCameraData );

	Startup();
}

StgModelCamera::~StgModelCamera()
{
	if( _frame_data != NULL ) {
		//dont forget about GetFrame() //TODO merge these together
		delete[] _frame_data;
		delete[] _frame_color_data;
		delete[] _vertexbuf_cache;
		delete[] _camera_quads;
		delete[] _camera_colors;
	}
}

void StgModelCamera::Load( void )
{
	StgModel::Load();
	
	_camera.setFov( wf->ReadLength( wf_entity, "horizfov",  _camera.horizFov() ), wf->ReadLength( wf_entity, "vertfov",  _camera.vertFov() ) );
	
	_camera.setClip( wf->ReadLength( wf_entity, "range_min",  CAMERA_NEAR_CLIP ), wf->ReadLength( wf_entity, "range_max",  CAMERA_FAR_CLIP ) );

	_yaw_offset = wf->ReadFloat( wf_entity, "yaw", _yaw_offset );
	_pitch_offset = wf->ReadFloat( wf_entity, "pitch", _pitch_offset );
	_width = wf->ReadInt( wf_entity, "width", _width );
	_height = wf->ReadInt( wf_entity, "height", _height );
}


void StgModelCamera::Update( void )
{   
	GetFrame();
	StgModel::Update();
	static float y = 0;
	y += 0.1;
//	setYaw( y );
	setPitch( y );
}

bool StgModelCamera::GetFrame( void )
{	
	if( _width == 0 || _height == 0 )
		return false;
	
	if( _frame_data == NULL ) {
		if( _frame_data != NULL ) {
			//don't forget about destructor
			delete[] _frame_data;
			delete[] _frame_color_data;
			delete[] _vertexbuf_cache;
			delete[] _camera_quads;
			delete[] _camera_colors;
		}
		_frame_data = new GLfloat[ _width * _height ]; //assumes a max of depth 4
		_frame_color_data = new GLubyte[ 4 * _width * _height ]; //for RGBA

		_vertexbuf_cache = new ColoredVertex[ _width * _height ]; //for unit vectors
		
		_camera_quads_size = _height * _width * 4 * 3; //one quad per pixel, 3 vertex per quad
		_camera_quads = new GLfloat[ _camera_quads_size ];
		_camera_colors = new GLubyte[ _camera_quads_size ];
	}

	//TODO overcome issue when glviewport is set LARGER than the window side
	//currently it just clips and draws outside areas black - resulting in bad glreadpixel data
	if( _width > _canvas->w() )
		_width = _canvas->w();
	if( _height > _canvas->h() )
		_height = _canvas->h();
	
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	
	glViewport( 0, 0, _width, _height );
	_camera.update();
	_camera.SetProjection();
	//TODO reposition the camera so it isn't inside the model ( or don't draw the parent when calling renderframe )
	_camera.setPose( parent->GetGlobalPose().x, parent->GetGlobalPose().y, CAMERA_HEIGHT ); //TODO use something smarter than a #define - make it configurable
	_camera.setYaw( rtod( parent->GetGlobalPose().a ) - 90.0 - _yaw_offset ); //-90.0 points the camera infront of the robot instead of pointing right
	_camera.setPitch( 90.0 - _pitch_offset );
	_camera.Draw();
	
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	_canvas->DrawFloor();
	_canvas->DrawBlocks();
	
	//read depth buffer
	glReadPixels(0, 0, _width, _height,
					 GL_DEPTH_COMPONENT, //GL_RGB,
					 GL_FLOAT, //GL_UNSIGNED_BYTE,
					 _frame_data );
	//transform length into linear length
	float* data = ( float* )( _frame_data ); //TODO use static_cast here
	int buf_size = _width * _height;
	for( int i = 0; i < buf_size; i++ )
		data[ i ] = _camera.realDistance( data[ i ] );

	//read color buffer
	glReadPixels(0, 0, _width, _height,
				 GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 _frame_color_data );		


	glViewport( viewport[0], viewport[1], viewport[2], viewport[3] );
	_canvas->invalidate();
	return true;
}

//TODO create lines outlining camera frustrum, then iterate over each depth measurement and create a square
void StgModelCamera::DataVisualize( void )
{
	if( _frame_data == NULL || !showCameraData )
		return;

	// TODO - shift to global CS?

	float w_fov = _camera.horizFov();
	float h_fov = _camera.vertFov();
	
	float center_horiz = - _yaw_offset;
	float center_vert = - _pitch_offset;
	
	float start_fov = center_horiz + w_fov / 2.0 + 180.0; //start at right
	float start_vert_fov = center_vert + h_fov / 2.0 + 90.0; //start at top
		
	int w = _width;
	int h = _height;
	float a_space = w_fov / w; //degrees between each sample
	float vert_a_space = h_fov / h; //degrees between each vertical sample

	//Create unit vectors representing a sphere - and cache it
	if( _valid_vertexbuf_cache == false ) {
		for( int j = 0; j < h; j++ ) {
			for( int i = 0; i < w; i++ ) {
				
				float a = start_fov - static_cast< float >( i ) * a_space;
				float vert_a = start_vert_fov - static_cast< float >( h - j - 1 ) * vert_a_space;
				
				int index = i + j * w;
				ColoredVertex* vertex = _vertexbuf_cache + index;
				
				//calculate and cache normal unit vectors of the sphere
				vertex->x = -sin( dtor( vert_a ) ) * cos( dtor( a ) );
				vertex->y = -sin( dtor( vert_a ) ) * sin( dtor( a ) );
				vertex->z = -cos( dtor( vert_a ) );			
			}
		}
		_valid_vertexbuf_cache = true;
	}
	
	glTranslatef( 0, 0, CAMERA_HEIGHT / 2.0 );
	glDisable( GL_CULL_FACE );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	//glBegin( GL_QUADS );
	
	
	//Scale cached unit vectors with depth-buffer
	float* depth_data = ( float* )( _frame_data );
	for( int j = 0; j < h; j++ ) {
		for( int i = 0; i < w; i++ ) {
			int index = i + j * w;
			const ColoredVertex* unit_vertex = _vertexbuf_cache + index;
			const float length = depth_data[ index ];
			
			//scale unitvectors with depth-buffer
			float sx = unit_vertex->x * length;
			float sy = unit_vertex->y * length;
			float sz = unit_vertex->z * length;
			
			//create a quad based on the current camera pixel, and normal vector
			//the quad size is porpotial to the depth distance
			float x, y, z;
			x = 0; y = 0; z = length * M_PI * a_space / 360.0;
			cross( x, y, z, unit_vertex->x, unit_vertex->y, unit_vertex->z );
			
			z = length * M_PI * vert_a_space / 360.0;
			
			GLfloat* p = _camera_quads + index * 4 * 3;
			p[ 0 ] = sx - x; p[  1 ] = sy - y; p[  2 ] = sz - z;
			p[ 3 ] = sx - x; p[  4 ] = sy - y; p[  5 ] = sz + z;
			p[ 6 ] = sx + x; p[  7 ] = sy + y; p[  8 ] = sz + z;
			p[ 9 ] = sx + x; p[ 10 ] = sy + y; p[ 11 ] = sz - z;

			//copy color for each vertex
			//TODO using a color index would be smarter
			const GLubyte* color = _frame_color_data + index * 4;
			for( int i = 0; i < 4; i++ ) {
				GLubyte* cp = _camera_colors + index * 4 * 3 + i * 3;
				memcpy( cp, color, sizeof( GLubyte ) * 3 );
			}

		}
	}
	

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	
	glVertexPointer( 3, GL_FLOAT, 0, _camera_quads );
	glColorPointer( 3, GL_UNSIGNED_BYTE, 0, _camera_colors );
	glDrawArrays( GL_QUADS, 0, w * h * 4 );
	
	glDisableClientState( GL_COLOR_ARRAY );
//	glDisableClientState( GL_VERTEX_ARRAY ); //TODO FIXME - something is using vertex arrays without properly enabling it

}

void StgModelCamera::Draw( uint32_t flags, StgCanvas* canvas )
{
	StgModel::Draw( flags, canvas );
}

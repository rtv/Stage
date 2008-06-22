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


StgModelCamera::StgModelCamera( StgWorld* world, StgModel* parent ) 
  : StgModel( world, parent, MODEL_TYPE_CAMERA ),
_frame_data( NULL ), _frame_color_data( NULL ), 
_vertexbuf_scaled( NULL ),  _width( 0 ), _height( 0 ), _valid_vertexbuf_cache( false ), _yaw_offset( 0 )
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
		delete[] _vertexbuf_cache;
		delete[] _vertexbuf_scaled;
		delete[] _vertexbuf_scaled_index;
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
}

const char* StgModelCamera::GetFrame( bool depth_buffer )
{	
	if( _width == 0 || _height == 0 )
		return NULL;
	
	if( _frame_data == NULL ) {
		if( _frame_data != NULL ) {
			delete[] _frame_data;
			delete[] _frame_color_data;
			delete[] _vertexbuf_cache;
			delete[] _vertexbuf_scaled;
			delete[] _vertexbuf_scaled_index;
		}
		_frame_data = new GLfloat[ _width * _height ]; //assumes a max of depth 4
		_frame_color_data = new GLubyte[ 4 * _width * _height ]; //for RGBA

		_vertexbuf_cache = new ColoredVertex[ _width * _height ]; //for unit vectors
		_vertexbuf_scaled = new ColoredVertex[ _width * _height ]; //scaled with z-buffer
		_vertexbuf_scaled_index = new GLushort[ 4 * (_width-1) * (_height-1) ]; //for indicies to draw a quad
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
				 GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 _frame_color_data );		


	_canvas->invalidate();
	return NULL; //_frame_data;
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

	
	int w = _width;
	int h = _height;
	float a;
	float vert_a;
	float a_space = w_fov / w; //degrees between each sample
	float vert_a_space = h_fov / h; //degrees between each vertical sample
	float x, y, z;
	float tmp_x, tmp_y, tmp_z;


	//TODO - there are still some vertices which aren't accurate - possibly due to a buffer overflow / memory corruption.
	//In some cases the vertices appear to be below the floor which shouldn't be possible once the unit vectors are scaled (and should simply hit the floor, but not pass beneath them)
	if( _valid_vertexbuf_cache == false ) {
		for( int j = 0; j < h; j++ ) {
			for( int i = 0; i < w; i++ ) {
				
				a = start_fov - static_cast< float >( i ) * a_space;
				vert_a = start_vert_fov - static_cast< float >( j ) * vert_a_space;
				
				//calculate based on a unit vector, which is scaled later on (this reduces all the sin/cos/dtor calls)
				x = 1;
				y = 0;
				z = 0;

				//rotate about z by a
				tmp_x = x * cos( dtor( a ) ); // - y *sin( but y=0)
				tmp_y = x * sin( dtor( a ) ); // + y * cos( but y=0 )
				tmp_z = z;
				
				x = tmp_x; y = tmp_y; z = tmp_z;
				
				//rotate about x
				tmp_z = - x * sin( dtor( vert_a ) );
				tmp_x = x * cos( dtor( vert_a ) );

				x = tmp_x;
				y = tmp_y;
				z = tmp_z;
				
				int index = i + j * w;
				ColoredVertex* vertex = _vertexbuf_cache + index;
				
				vertex->x = x;
				vertex->y = y;
				vertex->z = z;
				
				//TODO move the quad generate to the same section as scalling code, and only insert a quad when the distance between vertices isn't too large (i.e. spanning from a wall to sky)
				//It might also be possible to write a shader which would scale vertices and create quads instead of doing it here on the CPU.
				
				//skip top and left boarders
				if( i == 0 || j == 0 )
					continue;
				
				//decrement i and j, since the first boarders were skipped
				//note: the width of this array is (w-1) since it contains QUADS between the vertices
				GLushort* p = _vertexbuf_scaled_index + ( (i-1) + (j-1) * (w-1) ) * 4;
				
				//create quad growing adjacent point on top left (use width of `w' since these are VERTICES)
				p[ 0 ] = i     + j * w;
				p[ 1 ] = i - 1 + j * w;
				p[ 2 ] = i - 1 + ( j - 1 ) * w;
				p[ 3 ] = i     + ( j - 1 ) * w;
				
			}
		}
		_valid_vertexbuf_cache = true;
	}
	
	//Scale cached unit vectors with depth-buffer
	float* depth_data = ( float* )( _frame_data );
	for( int j = 0; j < h; j++ ) {
		for( int i = 0; i < w; i++ ) {
			int index = i + j * w;
			const ColoredVertex* unit_vertex = _vertexbuf_cache + index;
			ColoredVertex* scaled_vertex = _vertexbuf_scaled + index;
			const GLubyte* color = _frame_color_data + index * 4; //TODO might be buggy indexing
			const float length = depth_data[ index ];
			
			//scale unitvectors with depth-buffer
			scaled_vertex->x = unit_vertex->x * length;
			scaled_vertex->y = unit_vertex->y * length;
			scaled_vertex->z = unit_vertex->z * length;
			
			//colour the points
			//TODO color is buggy
			scaled_vertex->r = 0x00; //color[ 0 ];
			scaled_vertex->g = 0x00; //color[ 1 ];
			scaled_vertex->b = 0x00; //color[ 2 ];
			scaled_vertex->a = 0xFF;
			
		}
	}


	//draw then camera data
	glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //Can also be GL_FILL - but harder to debug
	glInterleavedArrays( GL_C4UB_V3F, 0, _vertexbuf_scaled );
	glDrawElements( GL_QUADS, 4 * (w-1) * (h-1), GL_UNSIGNED_SHORT, _vertexbuf_scaled_index );
	glPopClientAttrib();


}

void StgModelCamera::Draw( uint32_t flags, StgCanvas* canvas )
{
	StgModel::Draw( flags, canvas );
}


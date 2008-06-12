/*
 *  camera.cc
 *  Stage
 *
 *  Created by Alex Couture-Beil on 06/06/08.
 *
 */


#include "stage_internal.hh"

#include <iostream>

//perspective camera
//perspective camera
StgPerspectiveCamera::StgPerspectiveCamera( void ) : 
		_x( 0 ), _y( 0 ), _z( 0 ), _pitch( 90 ), _yaw( 0 ), _z_near( 0.1 ), _z_far( 20.0 )
{
}

void StgPerspectiveCamera::Draw( void ) const
{	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	
	glRotatef( - _pitch, 1.0, 0.0, 0.0 );
	glRotatef( - _yaw, 0.0, 0.0, 1.0 );

	glTranslatef( - _x, - _y, - _z );
	//zooming needs to happen in the Projection code (don't use glScale for zoom)

}

void StgPerspectiveCamera::SetProjection( float pixels_width, float pixels_height, float y_min, float y_max ) const
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	
	gluPerspective( 60.0, pixels_width/pixels_height, _z_near, _z_far );
	
	glMatrixMode (GL_MODELVIEW);
}

void StgPerspectiveCamera::SetProjection( float aspect ) const
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	
	gluPerspective( 60.0, aspect, _z_near, _z_far );
	
	glMatrixMode (GL_MODELVIEW);
}

void StgPerspectiveCamera::update( void )
{	

}







//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Ortho camera
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StgOrthoCamera::Draw( void ) const
{	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	glRotatef( _pitch, 1.0, 0.0, 0.0 );
	glRotatef( _yaw, 0.0, 0.0, 1.0 );

	glTranslatef( - _x, - _y, 0.0 );
	//zooming needs to happen in the Projection code (don't use glScale for zoom)

}

void StgOrthoCamera::SetProjection( float pixels_width, float pixels_height, float y_min, float y_max ) const
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	glOrtho( -pixels_width/2.0 / _scale, pixels_width/2.0 / _scale,
			-pixels_height/2.0 / _scale, pixels_height/2.0 / _scale,
			y_min * _scale * 2, y_max * _scale * 2 );	

	glMatrixMode (GL_MODELVIEW);
}

//TODO re-evaluate the way the camera is shifted when the mouse zooms - it might be possible to simplify
void StgOrthoCamera::scale( float scale, float shift_x, float w, float shift_y, float h )
{
	float to_scale = -scale;
	const float old_scale = _scale;

	//TODO setting up the factor can use some work
	float factor = 1.0 + fabs( to_scale ) / 25;
	if( factor < 1.1 )
		factor = 1.1; //this must be greater than 1.
	else if( factor > 2.5 )
		factor = 2.5;

	//convert the shift distance to the range [-0.5, 0.5]
	shift_x = shift_x / w - 0.5;
	shift_y = shift_y / h - 0.5;

	//adjust the shift values based on the factor (this represents how much the positions grows/shrinks)
	shift_x *= factor - 1.0;
	shift_y *= factor - 1.0;

	if( to_scale > 0 ) {
		//zoom in
		_scale *= factor;
		move( shift_x * w / _scale * _scale, 
				- shift_y * h / _scale * _scale );
	}
	else {
		//zoom out
		_scale /= factor;
		if( _scale < 1 ) {
			_scale = 1;
		} else {
			//shift camera to follow where mouse zoomed out
			move( - shift_x * w / old_scale * _scale, 
					shift_y * h / old_scale * _scale );
		}
	}
}

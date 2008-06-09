/*
 *  camera.cc
 *  Stage
 *
 *  Created by Alex Couture-Beil on 06/06/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */


#include "stage_internal.hh"

#include <iostream>

void StgCamera::Draw( void ) const
{	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	
	glRotatef( _pitch, 1.0, 0.0, 0.0 );
	glRotatef( _yaw, 0.0, 0.0, 1.0 );
	
	glTranslatef( - _x, - _y, 0.0 );
	//zooming needs to happen in the Projection code (don't use glScale for zoom)
	
}

void StgCamera::SetProjection( float pixels_width, float pixels_height, float y_min, float y_max ) const
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	
	glOrtho( -pixels_width/2.0 / _scale, pixels_width/2.0 / _scale,
			-pixels_height/2.0 / _scale, pixels_height/2.0 / _scale,
			y_min * _scale * 2, y_max * _scale * 2 );	
	
	glMatrixMode (GL_MODELVIEW);
}

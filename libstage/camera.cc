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

void StgCamera::Draw( void )
{	
	static float i = 0;
	i += 1;
//	glRotatef( rtod(-_stheta), fabs(cos(_sphi)), 0, 0 );
//	glRotatef( rtod(_sphi), 0,0,1 );   // rotate about z - yaw
//	
//	glTranslatef(  -_panx, -_pany, 0 );
//	glScalef( _scale, _scale, _scale ); 

	
	glRotatef( _pitch, 1.0, 0.0, 0.0 );
	glRotatef( _yaw, 0.0, 0.0, 1.0 );
	
	//both of these are handled in the glOrtho call
//	glScalef( _scale, _scale, _scale );
	glTranslatef( - _x * _scale, - _y * _scale, 0.0 );
	glScalef( _scale, _scale, _scale );
		
//	glRotatef( 60.0, 1.0, 0.0, 0.0 );

	//glRotatef( 60, 0.0, 1.0, 0.0 );
//	glRotatef( i, 0.0, 0.0, 1.0 );
	
}

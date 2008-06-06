/*
 *  texture_manager.cc
 *  Stage
 *
 *  Created by Alex Couture-Beil on 03/06/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "texture_manager.hh"
#include <sstream>

//TODO Windows Port
Fl_Shared_Image* TextureManager::loadImage( const char* filename )
{
	if( filename[ 0 ] == '/' || filename[ 0 ] == '~' )
		return Fl_Shared_Image::get( filename );
	
	const char* prefixes[] = {
		".",
		INSTALL_PREFIX "/share/stage",
		NULL
	};

	Fl_Shared_Image *img = NULL;
	int i = 0;	
	while( img == NULL && prefixes[ i ] != NULL ) {
		std::ostringstream oss;
		oss << prefixes[ i ] << "/" << filename;
		img = Fl_Shared_Image::get( oss.str().c_str() );
		std::cout << "loading from: " << oss.str() << std::endl;
		i++;
	}
	return img;
}

GLuint TextureManager::loadTexture( const char *filename )
{
	GLuint texName;
	Fl_Shared_Image *img = loadImage( filename );
	if( img == NULL ) {
		printf( "unable to open image: %s\n", filename );
		return 0;
	}
	
	//TODO display an error for incorrect depths
	if( img->d() != 3 && img->d() != 4 ) {
		printf( "unable to open image: %s - incorrect depth - should be 3 or 4\n", filename );
		return 0;
	}
	
	//TODO check for correct width/height - or convert it.
	
	guchar* pixels = (guchar*)(img->data()[0]);
	
	//vertically flip the image
	int img_size = img->w() * img->h() * img->d();
	guchar* img_flip = new guchar[ img_size ];
	
	const int row_width = img->w() * img->d();
	for( int i = 0; i < img->h(); i++ )
	memcpy( img_flip + ( i * row_width ), pixels + ( ( img->h() - i - 1) * row_width ), row_width );
	
	
	//create room for texture
	glGenTextures(1, &texName);
	std::cout << "loading image to: " << texName << std::endl;
	
	glBindTexture(GL_TEXTURE_2D, texName);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); //or use GL_DECAL or GL_MODULATE instead of DECAL to mix colours and textures
	
	gluBuild2DMipmaps (GL_TEXTURE_2D, img->d(), img->w(), img->h(), ( img->d() == 3 ? GL_RGB : GL_RGBA ), 
					   GL_UNSIGNED_BYTE, img_flip );	
	
	glBindTexture( GL_TEXTURE_2D, 0 );
	return texName;
	}

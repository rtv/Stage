/*
 *  texture_manager.hh
 *  Stage
 *
 * Singleton class for loading textures
 *
 */
#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#include "stage.hh"

#include <FL/Fl_Shared_Image.H>
#include <iostream>

namespace Stg
{
  ///Singleton for loading textures (not threadsafe)
  class TextureManager {
  private:
	 TextureManager( void ) { }
	 
  public:
		
		//TODO figure out where to store standard textures
		GLuint _stall_texture_id;
		GLuint _mains_texture_id;
		
		//TODO make this threadsafe
		static TextureManager& getInstance( void ) 
		{ 
			static TextureManager* the_instance = NULL;
			//TODO add a lock here
			if( the_instance == NULL ) {
				the_instance = new TextureManager;
			}
			return *the_instance;
		}
		
		///load a texture on the GPU, returned value is the texture ID, or 0 for failure
		GLuint loadTexture( const char *filename );
		
  };
}
#endif //_TEXTURE_MANAGER_H_

/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.c,v 1.3 2004-09-22 20:47:22 rtv Exp $
// License: GPL
/////////////////////////////////

#include "stage.h"

int main( int argc, char* argv[] )
{ 
  gui_startup( NULL, NULL );
  stg_world_t* world = stg_world_create_from_file( argv[1] );
  
  // stg_world_update returns 0 until it wants to quit or an error occurs
  while( 1  )
    {
      gui_poll(); // update the Stage window	  
      stg_world_update( world );
    }
  
  stg_world_destroy( world );
  
  exit( 0 );
}

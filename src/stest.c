/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.c,v 1.2 2004-09-16 06:57:36 rtv Exp $
// License: GPL
/////////////////////////////////

#include "stage.h"
#include "world.h" // kill


int main( int argc, char* argv[] )
{
  world_t* world = stg_world_create( 0, "Test World", 100, 100, 0.01 );
  //stg_world_t* world = stg_world_create();
  
  // stg_world_update returns 0 until it wants to quit or an error occurs
  while( world_update( world ) == 0  )
    //while( stg_world_update( world ) == 0  )
    {}
  
  world_destroy( world );
  //stg_world_destroy( world );
  
  exit( 0 );
}

/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.cc,v 1.1.2.5 2007-07-22 22:11:22 rtv Exp $
// License: GPL
/////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model.hh"
#include "config.h" // results of autoconf's system configuration tests

const size_t MAX_LASER_SAMPLES = 361;

double minfrontdistance = 0.750;
double speed = 0.400;
double turnrate = DTOR(60);

int randint;
int randcount = 0;
int avoidcount = 0;
int obs = FALSE;

int main( int argc, char* argv[] )
{ 
  printf( "Stage v%s test program.\n", VERSION );

  if( argc < 3 )
    {
      puts( "Usage: stest <worldfile> <robotname>" );
      exit(0);
    }
      
  // initialize libstage
  stg_init( argc, argv );

  stg_world_t* world = stg_world_create_from_file( 0, argv[1] );
  
  char* robotname = argv[2];

  // generate the name of the laser attached to the robot
  char lasername[64];
  snprintf( lasername, 63, "%s.laser:0", robotname ); 

  // generate the name of the sonar attached to the robot
  char rangername[64];
  snprintf( rangername, 63, "%s.ranger:0", robotname ); 
  
  StgModelPosition* position = (StgModelPosition*)stg_world_model_name_lookup( world, robotname );  
  assert(position);

  StgModelLaser* laser = (StgModelLaser*)stg_world_model_name_lookup( world, lasername );
  assert(laser);

  StgModelRanger* ranger = (StgModelRanger*)stg_world_model_name_lookup( world, rangername );
  assert(ranger);

  // subscribe to the laser - starts it collecting data
  position->Subscribe();
  laser->Subscribe();
  ranger->Subscribe();

  position->Print( "Subscribed to model" );
  laser->Print( "Subscribed to model" );
  ranger->Print( "Subscribed to model" );

  printf( "Starting world clock..." ); fflush(stdout);
  // start the clock
  stg_world_start( world );
  puts( "done" );


  double newspeed = 0.0;
  double newturnrate = 0.0;

  //stg_world_set_interval_real( world, 0 );

  while( (stg_world_update( world,0 )==0) )
    {
      usleep( 1000 );

      // get some laser data
      size_t laser_sample_count = laser->sample_count;      
      stg_laser_sample_t* laserdata = laser->samples;

      if( laserdata == NULL )
	continue;
      
      // THIS IS ADAPTED FROM PLAYER'S RANDOMWALK C++ EXAMPLE

      /* See if there is an obstacle in front */
      obs = FALSE;
      for( unsigned int i = 0; i < laser_sample_count; i++)
	{
	  if(laserdata[i].range < minfrontdistance)
	    obs = TRUE;
	}
      
      if(obs || avoidcount )
	{
	  newspeed = 0;
	  
	  /* once we start avoiding, continue avoiding for 2 seconds */
	  /* (we run at about 10Hz, so 20 loop iterations is about 2 sec) */
	  if(!avoidcount)
	    {
	      avoidcount = 15;
	      randcount = 0;
	      
	      // find the minimum on the left and right
	      
	      double min_left = 1e9;
	      double min_right = 1e9;
	      
	      for( unsigned int i=0; i<laser_sample_count; i++ )
		{
		  if(i>(laser_sample_count/2) && laserdata[i].range < min_left)
		    min_left = laserdata[i].range;
		  else if(i<(laser_sample_count/2) && laserdata[i].range < min_right)
		    min_right = laserdata[i].range;
		}

	      if( min_left < min_right)
		newturnrate = -turnrate;
	      else
		newturnrate = turnrate;
	    }
	  
	  avoidcount--;
	}
      else
	{
	  avoidcount = 0;
	  newspeed = speed;
	  
	  /* update turnrate every 3 seconds */
	  if(!randcount)
	    {
	      /* make random int tween -20 and 20 */
	      randint = rand() % 41 - 20;
	      
	      newturnrate = DTOR(randint);
	      randcount = 50;
	    }
	  randcount--;
	}
      
      position->Do( newspeed, 0, newturnrate );

    }
  
  stg_world_destroy( world );
  
  exit( 0 );
}

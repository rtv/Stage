/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.c,v 1.17.2.1 2006-09-14 07:03:25 rtv Exp $
// License: GPL
/////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stage.h"
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
  
  char sonarname[64];
  snprintf( sonarname, 63, "%s.ranger:0", robotname ); 
  
  stg_model_t* position = stg_world_model_name_lookup( world, robotname );  
  stg_model_t* laser = stg_world_model_name_lookup( world, lasername );

  // subscribe to the laser - starts it collecting data
  stg_model_subscribe( laser );
  stg_model_subscribe( position);

  stg_model_print( position, "Subscribed to model" );
  stg_model_print( laser, "Subscribed to model" );

  printf( "Starting world clock..." ); fflush(stdout);
  // start the clock
  stg_world_start( world );
  puts( "done" );


  double newspeed = 0.0;
  double newturnrate = 0.0;

  //stg_world_set_interval_real( world, 0 );

  while( (stg_world_update( world,0 )==0) )
    {
      // get some laser data
      size_t laser_sample_count = 0;

      stg_laser_sample_t* laserdata = 
	stg_model_get_data( laser, &laser_sample_count );
      laser_sample_count /= sizeof(stg_laser_sample_t);
      
      // THIS IS ADAPTED FROM PLAYER'S RANDOMWALK C++ EXAMPLE

      /* See if there is an obstacle in front */
      obs = FALSE;
      int i;
      for(i = 0; i < laser_sample_count; i++)
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
	      
	      for(i=0; i<laser_sample_count; i++ )
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
	      randcount = 20;
	    }
	  randcount--;
	}
      
      stg_position_cmd_t cmd;
      memset(&cmd,0,sizeof(cmd));
      cmd.x = newspeed;
      cmd.y = 0;
      cmd.a = newturnrate;

      stg_model_set_cmd( position, &cmd, sizeof(cmd));

    }
  
  stg_world_destroy( world );
  
  exit( 0 );
}

/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.cc,v 1.1.2.11 2007-10-31 03:55:11 rtv Exp $
// License: GPL
/////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stage.hh"
//#include "world.hh"
//#include "worldgtk.hh"
//#include "config.h" // results of autoconf's system configuration tests

double minfrontdistance = 0.750;
double speed = 0.400;
double turnrate = DTOR(60);

int randint;
int randcount = 0;
int avoidcount = 0;
int obs = FALSE;

int POPSIZE = 50;

int main( int argc, char* argv[] )
{ 
  printf( "%s test program.\n", stg_package_string() );

  if( argc < 3 )
    {
      puts( "Usage: stest <worldfile> <robotname>" );
      exit(0);
    }
      
  // initialize libstage
  StgWorldGtk::Init( &argc, &argv );

  StgWorldGtk world;
  world.Load( argv[1] );
  
  //StgModel mod( &world, NULL, 0, "model" );
  //StgModel mod2( &world, NULL, 0, "model" );
  
  //stg_pose_t pz;
  // pz.x = 5;
  //pz.y = 0;
  //pz.z = 0;
  //pz.a = 4;//M_PI/2.0;
   //mod.SetPose( &pz );

   //pz.y = 3;
   // mod2.SetPose( &pz );

  //for( int i=0; i<10; i++ )
  //mod.AddBlockRect( i/2, 0, 0.3, 0.3 );
   
   //char* robotname = argv[2];
   
   //   // generate the name of the laser attached to the robot
   //char lasername[64];
   //snprintf( lasername, 63, "%s.laser:0", robotname ); 
   
   //   // generate the name of the sonar attached to the robot
   //   char rangername[64];
   //   snprintf( rangername, 63, "%s.ranger:0", robotname ); 
   
   char namebuf[256];
   


   //StgModelPosition* position = (StgModelPosition*)world.GetModel( "MyWorld:0.position:0" );
   //assert(position);
   
   //StgModelLaser* laser = (StgModelLaser*)world.GetModel("MyWorld:0.position:0.laser:0" );
   //assert( laser );
   
   StgModelPosition* positions[POPSIZE];
   StgModelLaser* lasers[POPSIZE];

   for( int i=0; i<POPSIZE; i++ )
     {
       sprintf( namebuf, "MyWorld:0.position:%d", i+1 );
       positions[i] = (StgModelPosition*)world.GetModel( namebuf );
       assert(positions[i]);
       positions[i]->Subscribe();
       
       sprintf( namebuf, "MyWorld:0.position:%d.laser:0", i+1 );
       lasers[i] = (StgModelLaser*)world.GetModel( namebuf );
       assert(lasers[i]);
       lasers[i]->Subscribe();
     }
   
//   StgModelRanger* ranger = (StgModelRanger*)world.GetModel( rangername );
//   assert(ranger);

//   // subscribe to the laser - starts it collecting data

   //position->Subscribe();   
   //laser->Subscribe();

   //position->Print( "Subscribed to model" );
   //laser->Print( "Subscribed to model" );
  //ranger->Print( "Subscribed to model" );

  //printf( "Starting world clock..." ); fflush(stdout);
  // start the clock
  //world.Start();
  //puts( "done" );

  double newspeed = 0.0;
  double newturnrate = 0.0;

  //stg_world_set_interval_real( world, 0 );
  
  //  StgModelLaser* laser = lasers[1];

  //while( world.RealTimeUpdate() )
  while( world.Update() )
    {
      //       nothing
      //while( ! laser->DataIsFresh() )
      //if( ! world.RealTimeUpdate() )
      //break;
     
      for( int i=0; i<POPSIZE; i++ )
	{
	  StgModelPosition* position = positions[i];
	  StgModelLaser* laser = lasers[i];
	  
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

    }
  
  //#endif  

  exit( 0 );
}

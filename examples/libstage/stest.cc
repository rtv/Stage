/////////////////////////////////
// File: stest.c
// Desc: Stage library test program
// Created: 2004.9.15
// Author: Richard Vaughan <vaughan@sfu.ca>
// CVS: $Id: stest.cc,v 1.1 2008-01-15 01:29:10 rtv Exp $
// License: GPL
/////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>
#include <iostream>
#include <cmath>

#include "stage.hh"

class Robot
{
public:
    Stg::ModelPosition* position;
    Stg::ModelRanger* ranger;
};

class Logic
{
public:
    static int Callback(Stg::World* world, void* userarg)
    {
        Logic* lg = reinterpret_cast<Logic*>(userarg);
        
        lg->Tick(world);
        
        // never remove this call-back
        return 0;
    }
    
    Logic(unsigned int popsize)
    : 
    population_size(popsize),
    robots(new Robot[population_size])
    {

    }
    
    void connect(Stg::World* world)
    {
        // connect the first population_size robots to this controller
        for(unsigned int idx = 0; idx < population_size; idx++)
        {
            // the robots' models are named r0 .. r1999
            std::stringstream name;
            name << "r" << idx;
            
            // get the robot's model and subscribe to it
            Stg::ModelPosition* posmod = reinterpret_cast<Stg::ModelPosition*>(
                world->GetModel(name.str())
                );
            assert(posmod != 0);
            
            robots[idx].position = posmod;
            robots[idx].position->Subscribe();
            
            // get the robot's ranger model and subscribe to it
            Stg::ModelRanger* rngmod = reinterpret_cast<Stg::ModelRanger*>(
                robots[idx].position->GetChild( "ranger:0" )
                );
            assert(rngmod != 0);
            
            robots[idx].ranger = rngmod;
            robots[idx].ranger->Subscribe();
        }
        
        // register with the world
        world->AddUpdateCallback(Logic::Callback, reinterpret_cast<void*>(this));    
    }
    
    ~Logic()
    {
        delete[] robots;
    }
    

    void Tick(Stg::World*)
    {
        // the controllers parameters
        const double vspeed = 0.4; // meters per second
        const double wgain  = 1.0; // turn speed gain
        const double safe_dist = 0.1; // meters
        const double safe_angle = 0.3; // radians
    
        // each robot has a group of ir sensors
        // each sensor takes one sample
        // the forward sensor is the middle sensor
        for(unsigned int idx = 0; idx < population_size; idx++)
        {
            Stg::ModelRanger* rgr = robots[idx].ranger;

            // compute the vector sum of the sonar ranges	      
            double dx=0, dy=0;

            // the range model has multiple sensors
            typedef std::vector<Stg::ModelRanger::Sensor>::const_iterator sensor_iterator;
            const std::vector<Stg::ModelRanger::Sensor> sensors = rgr->GetSensors();
            
            for(sensor_iterator sensor = sensors.begin(); sensor != sensors.end(); sensor++)
            {
                // each sensor takes a single sample (as specified in the .world)
                const double srange = (*sensor).ranges[0];
                const double angle  = (*sensor).pose.a;
                
                dx += srange * std::cos(angle);
                dy += srange * std::sin(angle);
            } 
            
            if(dx == 0)
                continue;
            
            if(dy == 0)
                continue;
            
            // calculate the angle towards the farthest obstacle
            const double resultant_angle = std::atan2( dy, dx );

            // check whether the front is clear
            const unsigned int forward_idx = sensors.size() / 2u - 1u;
            
            const double forwardm_range = sensors[forward_idx - 1].ranges[0];
            const double forward_range  = sensors[forward_idx + 0].ranges[0]; 
            const double forwardp_range = sensors[forward_idx + 1].ranges[0]; 
            
            bool front_clear = (
                    (forwardm_range > safe_dist / 5.0) &&
                    (forward_range > safe_dist) &&
                    (forwardp_range > safe_dist / 5.0) &&
                    (std::abs(resultant_angle) < safe_angle)
                );
            
            // turn the sensor input into movement commands

            // move forwards if the front is clear
            const double forward_speed = front_clear ? vspeed : 0.0;
            // do not strafe
            const double side_speed = 0.0;	   
            
            // turn towards the farthest obstacle
            const double turn_speed = wgain * resultant_angle;
            
            // finally, relay the commands to the robot
            robots[idx].position->SetSpeed( forward_speed, side_speed, turn_speed );
        }
    }
    
protected:
    unsigned int population_size;   
    Robot* robots;
};

int main( int argc, char* argv[] )
{ 
    // check and handle the argumets
    if( argc < 3 )
    {
        puts( "Usage: stest <worldfile> <number of robots>" );
        exit(0);
    }
    
    const int popsize = atoi(argv[2] );

    // initialize libstage
    Stg::Init( &argc, &argv );
  
    // create the world
    //Stg::World world;
    Stg::WorldGui world(800, 700, "Stage Benchmark Program");
    world.Load( argv[1] );
  
    // create the logic and connect it to the world
    Logic logic(popsize);
    logic.connect(&world);
    
    // and then run the simulation
    world.Run();
    
    return 0;
}

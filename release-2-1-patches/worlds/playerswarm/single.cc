#include <libplayerc++/playerc++.h>
#include <iostream>
#include <assert.h>

// For argument parsing
#include "args.h"

#define POP 100 // population size
#define VSPEED 0.4 // meters per second
#define WGAIN 1.0 // turn speed gain
#define SAFE_DIST 0.3 // meters
#define SAFE_ANGLE 0.4 // radians

int main(int argc, char **argv)
{
  // Parse command line options
  parse_args(argc,argv);

  // We throw exceptions on creation if we fail
  try
  {
    using namespace PlayerCc;
    using namespace std;

    // Create a player client object, using the variables assigned by the
    // call to parse_args()
    PlayerClient robot (gHostname, gPort);
    robot.SetDataMode( PLAYER_DATAMODE_PULL );
    robot.SetReplaceRule( true );
    
    Position2dProxy pp( &robot, gIndex );
    SonarProxy      sp( &robot, gIndex );
    Graphics2dProxy gp( &robot, gIndex );

    sp.RequestGeom(); 	// query the server for sonar positions
        
    while(1)
      {
	// blocks until new data comes from Player
	robot.Read();
	
	for( int i=0; i<POP; i++ )
	  {
	    // compute the vector sum of the sonar ranges	      
	    double dx=0, dy=0;
	    
	    int num_ranges = sp.GetCount();
	    for( int s=0; s<num_ranges; s++ )
	      {
		player_pose3d_t spose = sp.GetPose(s);
		double srange = sp.GetScan(s);
		
		dx += srange * cos( spose.pyaw );
		dy += srange * sin( spose.pyaw );
	      }
	    
	    // compute the direction of the resultant vector 
	    double resultant_angle = atan2( dy, dx );
	    //double resultant_magnitude = hypot( dy, dx );
	    
	    double forward_speed = 0.0;
	    double side_speed = 0.0;	   
	    double turn_speed = WGAIN * resultant_angle;
	    	    
	    // find the index of the forward-pointing sensor
	    int forward = num_ranges/2 -1 ;
	    
	    // if the front is clear, drive forwards
	    if( (sp.GetScan(forward-1) > SAFE_DIST) &&
		(sp.GetScan(forward) > SAFE_DIST) &&
		(sp.GetScan(forward+1) > SAFE_DIST) && 
		(fabs( resultant_angle ) < SAFE_ANGLE) )
	      {
		forward_speed = VSPEED;
	      }
	
	    // send a command to the robot's position device
	    pp.SetSpeed( forward_speed, side_speed, turn_speed );
	    
	    // draw the resultant vector on the robot to show what it
	    // is thinking
	    if( forward_speed > 0 )	      
	      gp.Color( 0,255,0,0 );
	    else
	      gp.Color( 0,255,255,0 );

	    player_point_2d_t pts[2];
	    pts[0].px = 0;
	    pts[0].py = 0;
	    pts[1].px = dx;
	    pts[1].py = dy;	
	    
	    gp.Clear();
	    gp.DrawPolyline( pts, 2 );
	    
	  }    
      }
  }
  catch (PlayerCc::PlayerError e)
    {
      std::cerr << e << std::endl;
      return -1;
    }
}

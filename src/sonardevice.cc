// $Id: sonardevice.cc,v 1.5.2.1 2000-12-05 23:17:34 ahoward Exp $
#include <math.h>

#include "world.h"
#include "robot.h"
#include "sonardevice.h"

const double TWOPI = 6.283185307;

// constructor

CSonarDevice::CSonarDevice( CRobot* rr, 
			      void *buffer, 
			      size_t data_len, 
			      size_t command_len, 
			      size_t config_len)
  : CPlayerDevice(rr, buffer, data_len, command_len, config_len)
{
  updateInterval = 0.1; //seconds
  lastUpdate = 0;
  
  // zero the data
   memset( sonar, 0, sizeof( unsigned short ) * SONARSAMPLES );

    undrawRequired = false;
}

bool CSonarDevice::GUIDraw()
{  
  // dump out if noone is subscribed
  if( !IsSubscribed() || !m_robot->showDeviceDetail ) return true;
  
  // copy first to last hit point to draw a closed polygon
  hitPts[SONARSAMPLES].x = hitPts[0].x;
  hitPts[SONARSAMPLES].y = hitPts[0].y;

  m_world->win->SetForeground( m_world->win->RobotDrawColor( m_robot) );
  m_world->win->DrawLines( hitPts, SONARSAMPLES+1 );

  memcpy( oldHitPts, hitPts, sizeof( XPoint ) * (SONARSAMPLES+1) );

  undrawRequired = true;

  return true; 
};  

bool CSonarDevice::GUIUnDraw()
{ 
  // dump out if noone is subscribed
  if( undrawRequired )
    {
      m_world->win->SetForeground( m_world->win->RobotDrawColor( m_robot) );
      m_world->win->DrawLines( oldHitPts, SONARSAMPLES+1 );
      
      undrawRequired = false;
    }
      
  return true; 
};


void CSonarDevice::Update() 
{
  Nimage* img = m_robot->world->img;

  // dump out if noone is subscribed
  if( !IsSubscribed() ) return;

  // if its time to recalculate vision
  if( m_robot->world->timeNow - lastUpdate > updateInterval )
    {
      lastUpdate = m_robot->world->timeNow;
      
      float ppm = m_robot->world->ppm;
      
      float tangle; 
      // is the angle from the robot's nose to the transducer itself
      // and angle is the direction in which the transducer points
      // trace the new sonar beams to discover their ranges
      for( int s=0; s < SONARSAMPLES; s++ )
	{
	  float dx, dy, angle;
	  float xx = m_robot->x;
	  float yy = m_robot->y;
	  
	  float a = m_robot->a;

	  switch( s )
	    {
#ifdef PIONEER1
	    case 0: angle = a - 1.57; break; //-90 deg
	    case 1: angle = a - 0.52; break; // -30 deg
	    case 2: angle = a - 0.26; break; // -15 deg
	    case 3: angle = a       ; break;
	    case 4: angle = a + 0.26; break; // 15 deg
	    case 5: angle = a + 0.52; break; // 30 deg
	    case 6: angle = a + 1.57; break; // 90 deg
#else
	    case 0:
	      angle = a - 1.57;
	      tangle = a - 0.900;
	      xx += 0.172 * cos( tangle ) * ppm;
	      yy += 0.172 * sin( tangle ) * ppm;
	      break; //-90 deg
	    case 1: angle = a - 0.87;
	      tangle = a - 0.652;
	      xx += 0.196 * cos( tangle ) * ppm;
	      yy += 0.196 * sin( tangle ) * ppm;
	      break; // -50 deg
	    case 2: angle = a - 0.52;
	      tangle = a - 0.385;
	      xx += 0.208 * cos( tangle ) * ppm;
	      yy += 0.208 * sin( tangle ) * ppm;
	      break; // -30 deg
	    case 3: angle = a - 0.17;
	      tangle = a - 0.137;
	      xx += 0.214 * cos( tangle ) * ppm;
	      yy += 0.214 * sin( tangle ) * ppm;
	      break; // -10 deg
	    case 4: angle = a + 0.17;
	      tangle = a + 0.137;
	      xx += 0.214 * cos( tangle ) * ppm;
	      yy += 0.214 * sin( tangle ) * ppm;
	      break; // 10 deg
	    case 5: angle = a + 0.52;
	      tangle = a + 0.385;
	      xx += 0.208 * cos( tangle ) * ppm;
	      yy += 0.208 * sin( tangle ) * ppm;
	      break; // 30 deg
	    case 6: angle = a + 0.87;
	      tangle = a + 0.652;
	      xx += 0.196 * cos( tangle ) * ppm;
	      yy += 0.196 * sin( tangle ) * ppm;
	      break; // 50 deg
	    case 7: angle = a + 1.57;
	      tangle = a + 0.900;
	      xx += 0.172 * cos( tangle ) * ppm;
	      yy += 0.172 * sin( tangle ) * ppm;
	      break; // 90 deg
	    case 8: angle = a + 1.57;
	      tangle = a + 2.240;
	      xx += 0.172 * cos( tangle ) * ppm;
	      yy += 0.172 * sin( tangle ) * ppm;
	      break; // 90 deg
	    case 9: angle = a + 2.27;
	      tangle = a + 2.488;
	      xx += 0.196 * cos( tangle ) * ppm;
	      yy += 0.196 * sin( tangle ) * ppm;
	      break; // 130 deg
	    case 10: angle = a + 2.62;
	      tangle = a + 2.755;
	      xx += 0.208 * cos( tangle ) * ppm;
	      yy += 0.208 * sin( tangle ) * ppm;
	      break; // 150 deg
	    case 11: angle = a + 2.97;
	      tangle = a + 3.005;
	      xx += 0.214 * cos( tangle ) * ppm;
	      yy += 0.214 * sin( tangle ) * ppm;
	      break; // 170 deg
	    case 12: angle = a - 2.97;
	      tangle = a - 3.005;
	      xx += 0.214 * cos( tangle ) * ppm;
	      yy += 0.214 * sin( tangle ) * ppm;
	      break; // -170 deg
	    case 13: angle = a - 2.62;
	      tangle = a - 2.755;
	      xx += 0.208 * cos( tangle ) * ppm;
	      yy += 0.208 * sin( tangle ) * ppm;
	      break; // -150 deg
	    case 14: angle = a - 2.27;
	      tangle = a - 2.488;
	      xx += 0.196 * cos( tangle ) * ppm;
	      yy += 0.196 * sin( tangle ) * ppm;
	      break; // -130 deg
	    case 15: angle = a - 1.57;
	      tangle = a - 2.240;
	      xx += 0.172 * cos( tangle ) * ppm;
	      yy += 0.172 * sin( tangle ) * ppm;
	      break; // -90 deg
#endif
	    }
	  
	  dx = cos( angle );
	  dy = sin(  angle);
	  
	  int maxRange = (int)(5.0 * ppm); // 5m times pixels-per-meter
	  int rng = 0;
	  
	  while( rng < maxRange &&
		 (img->get_pixel( (int)xx, (int)yy ) == 0 ||
		  img->get_pixel( (int)xx, (int)yy ) == m_robot->color ) &&
		 (xx > 0) &&  (xx < img->width)
		 && (yy > 0) && (yy < img->height)  )
	    {
	      xx+=dx;
	      yy+=dy;
	      rng++;
	    }
	  
	  // set sonar value, scaled to current ppm in mm 
	  // and switched to network byte order
	  sonar[s] = htons((UINT16) ( (rng / ppm) * 1000.0 ));

	  // store the hit points if we need to draw them on the GUI
	  //if( GUIrender )
	   {
	     hitPts[s].x = (int)xx;
	     hitPts[s].y = (int)yy;
	   }
	}

      PutData( sonar, sizeof( unsigned short ) * SONARSAMPLES );
    }
}

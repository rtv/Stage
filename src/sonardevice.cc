///////////////////////////////////////////////////////////////////////////
//
// File: sonardevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 30 Nov 2000
// Desc: Simulates the pioneer sonars
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/sonardevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.21 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////
#include <iostream.h>
#include <math.h>
#include "world.hh"
#include "sonardevice.hh"
#include "raytrace.hh"

// constructor

CSonarDevice::CSonarDevice(CWorld *world, CEntity *parent )
  : CEntity(world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_sonar_data_t );
  m_command_len = 0;
  m_config_len  = 1;
  m_reply_len  = 1;
  
  m_player.code = PLAYER_SONAR_CODE; // from player's messages.h
  m_stage_type = SonarType;
  
  SetColor(SONAR_COLOR);

  m_sonar_count = SONARSAMPLES;
  m_min_range = 0.20;
  m_max_range = 5.0;
    
  size_x = size_y = 0.3;

  // Initialise the sonar poses
  //
  for (int i = 0; i < m_sonar_count; i++)
    GetSonarPose(i, m_sonar[i][0], m_sonar[i][1], m_sonar[i][2]);
  
  // zero the data
  memset( &m_data, 0, sizeof(m_data) );
}


void CSonarDevice::Update( double sim_time ) 
{
#ifdef DEBUG
  CEntity::Update( sim_time ); // inherit some debug output
#endif

  // dump out if noone is subscribed
  if(!Subscribed())
    return;

  // Check to see if it is time to update
  //  - if not, return right away.
  if( sim_time - m_last_update < m_interval) return;
  
  m_last_update = sim_time;

  // Get configs  
  uint16_t cmd;
  void* client;
  if(GetConfig(&client, &cmd, sizeof(cmd)) >  0)
  {
    // we got a config
    if((cmd & 0xFF) == PLAYER_SONAR_POWER_REQ)
    {
      // we got a sonar power toggle - i just ignore them.
      puts( "sonar power toggled" );
      PutReply(client, PLAYER_MSGTYPE_RESP_ACK);
    }
    else
    {
      // don't recognize this request
      PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
    }
  }
  
  // Check bounds
  //
  ASSERT((size_t) m_sonar_count <= sizeof(m_data.ranges) 
	 / sizeof(m_data.ranges[0]));
  
  // Do each sonar
  //
  for (int s = 0; s < m_sonar_count; s++)
    {
      // Compute parameters of scan line
      double ox = m_sonar[s][0];
      double oy = m_sonar[s][1];
      double oth = m_sonar[s][2];
      LocalToGlobal(ox, oy, oth);

      double range = m_max_range;
      
      CLineIterator lit( ox, oy, oth, m_max_range, 
                         m_world->ppm, m_world->matrix, PointToBearingRange );
      CEntity* ent;
      
      while( (ent = lit.GetNextEntity()) )
      {
          if( ent != this && ent != m_parent_object && ent->sonar_return ) 
          {
              range = lit.GetRange();
              break;
          }
      }
      
      uint16_t v = (uint16_t)(1000.0 * range);
      
      // Store range in mm in network byte order
      //
      m_data.ranges[s] = htons(v);
    }
  
  PutData( &m_data, sizeof(m_data) );

  return;
}

///////////////////////////////////////////////////////////////////////////
// *** HACK -- this could be put in an array ahoward
// Get the pose of the sonar
//

/* TODO */
/*these are the offsets of each sonar sensor from the Pioneer 2's center
   in meters - first array is x, second is y */
/* const double player_sonar_offsets[PLAYER_NUM_SONAR_SAMPLES][2] = { 
 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }, 
 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 } 
};
*/
void CSonarDevice::GetSonarPose(int s, double &px, double &py, double &pth)
{
    double xx = 0;
    double yy = 0;
    double a = 0;
    double angle, tangle;
        
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
	      xx += 0.172 * cos( tangle );
	      yy += 0.172 * sin( tangle );
	      break; //-90 deg
	    case 1: angle = a - 0.87;
	      tangle = a - 0.652;
	      xx += 0.196 * cos( tangle );
	      yy += 0.196 * sin( tangle );
	      break; // -50 deg
	    case 2: angle = a - 0.52;
	      tangle = a - 0.385;
	      xx += 0.208 * cos( tangle );
	      yy += 0.208 * sin( tangle );
	      break; // -30 deg
	    case 3: angle = a - 0.17;
	      tangle = a - 0.137;
	      xx += 0.214 * cos( tangle );
	      yy += 0.214 * sin( tangle );
	      break; // -10 deg
	    case 4: angle = a + 0.17;
	      tangle = a + 0.137;
	      xx += 0.214 * cos( tangle );
	      yy += 0.214 * sin( tangle );
	      break; // 10 deg
	    case 5: angle = a + 0.52;
	      tangle = a + 0.385;
	      xx += 0.208 * cos( tangle );
	      yy += 0.208 * sin( tangle );
	      break; // 30 deg
	    case 6: angle = a + 0.87;
	      tangle = a + 0.652;
	      xx += 0.196 * cos( tangle );
	      yy += 0.196 * sin( tangle );
	      break; // 50 deg
	    case 7: angle = a + 1.57;
	      tangle = a + 0.900;
	      xx += 0.172 * cos( tangle );
	      yy += 0.172 * sin( tangle );
	      break; // 90 deg
	    case 8: angle = a + 1.57;
	      tangle = a + 2.240;
	      xx += 0.172 * cos( tangle );
	      yy += 0.172 * sin( tangle );
	      break; // 90 deg
	    case 9: angle = a + 2.27;
	      tangle = a + 2.488;
	      xx += 0.196 * cos( tangle );
	      yy += 0.196 * sin( tangle );
	      break; // 130 deg
	    case 10: angle = a + 2.62;
	      tangle = a + 2.755;
	      xx += 0.208 * cos( tangle );
	      yy += 0.208 * sin( tangle );
	      break; // 150 deg
	    case 11: angle = a + 2.97;
	      tangle = a + 3.005;
	      xx += 0.214 * cos( tangle );
	      yy += 0.214 * sin( tangle );
	      break; // 170 deg
	    case 12: angle = a - 2.97;
	      tangle = a - 3.005;
	      xx += 0.214 * cos( tangle );
	      yy += 0.214 * sin( tangle );
	      break; // -170 deg
	    case 13: angle = a - 2.62;
	      tangle = a - 2.755;
	      xx += 0.208 * cos( tangle );
	      yy += 0.208 * sin( tangle );
	      break; // -150 deg
	    case 14: angle = a - 2.27;
	      tangle = a - 2.488;
	      xx += 0.196 * cos( tangle );
	      yy += 0.196 * sin( tangle );
	      break; // -130 deg
	    case 15: angle = a - 1.57;
	      tangle = a - 2.240;
	      xx += 0.172 * cos( tangle );
	      yy += 0.172 * sin( tangle );
	      break; // -90 deg
        default:
          angle = 0;
          xx = 0;
          yy = 0;
          break;
#endif
	    }

      px = xx;
      py = -yy;
      pth = -angle;
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CSonarDevice::RtkStartup()
{
  CEntity::RtkStartup();
  
  // Create a figure representing this object
  this->scan_fig = rtk_fig_create(m_world->canvas, NULL, 49);

  // Set the color -  blue
  double r = 0.6;
  double g = 1.0;
  double b = 0.6;
  rtk_fig_color(this->scan_fig, r, g, b);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CSonarDevice::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->scan_fig);

  CEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CSonarDevice::RtkUpdate()
{
  CEntity::RtkUpdate();
   
  // Get global pose
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);

  rtk_fig_clear(this->scan_fig);

  // see the comment in CLaserDevice for why this gets the data out of
  // the buffer instead of storing hit points in ::Update() - RTV
  player_sonar_data_t data;
  
  if( Subscribed() > 0 )
    {
      size_t res = GetData( &data, sizeof(data));

      if( res == sizeof(data) )
	for( int l=0; l < PLAYER_NUM_SONAR_SAMPLES; l++ )
	  {
	    double xoffset, yoffset, angle;
	    GetSonarPose( l, xoffset, yoffset, angle );
	    
	    double range = (double)ntohs(data.ranges[l]);
	    
	    angle += gth;
	    
	    double x1 = gx + xoffset * cos(gth) - yoffset * sin(gth);
	    double y1 = gy + xoffset * sin(gth) + yoffset * cos(gth);
	    
	    double x2 = x1 + range/1000.0 * cos( angle ); 
	    double y2 = y1 + range/1000.0 * sin( angle );       
	    
	    rtk_fig_line(this->scan_fig, x1, y1, x2, y2 );
	  }
      else
	PRINT_WARN2( "GET DATA RETURNED WRONG AMOUNT (%d/%d bytes)", 
		     res, sizeof(data) );
    }  
}

#endif


///////////////////////////////////////////////////////////////////////////
//
// File: sonardevice.cc
// Author: Richard Vaughan, Andrew Howard
// Date: 30 Nov 2000
// Desc: Simulates the pioneer sonars
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/sonardevice.cc,v $
//  $Author: ahoward $
//  $Revision: 1.7 $
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

const double TWOPI = 6.283185307;

// constructor

CSonarDevice::CSonarDevice(CWorld *world, CEntity *parent, CPlayerServer *server)
        : CPlayerDevice(world, parent, server,
                        SONAR_DATA_START,
                        SONAR_TOTAL_BUFFER_SIZE,
                        SONAR_DATA_BUFFER_SIZE,
                        SONAR_COMMAND_BUFFER_SIZE,
                        SONAR_CONFIG_BUFFER_SIZE)
{
    updateInterval = 0.1; //seconds
    lastUpdate = 0;

    m_sonar_count = SONARSAMPLES;
    m_min_range = 0.20;
    m_max_range = 8.0;
    
    // Initialise the sonar poses
    //
    for (int i = 0; i < m_sonar_count; i++)
        GetSonarPose(i, m_sonar[i][0], m_sonar[i][1], m_sonar[i][2]);
    
    // zero the data
    memset( m_range, 0, sizeof(m_range) );

    exp.objectType = sonar_o; 
    exporting = true;
    exp.objectId = this; // used both as ptr and as a unique ID
    exp.data = (char*)&expSonar;
    expSonar.hitCount = 0;
}


void CSonarDevice::Update() 
{
    CPlayerDevice::Update();

   
    // dump out if noone is subscribed
    if(!IsSubscribed())
      {
	// have to invalidate the exported scan data so it gets undrawn
	memset( &expSonar, 0, expSonar.hitCount * sizeof( DPoint ) );
        return;
      }

    // if its time to recalculate vision
    if( m_world->GetTime() - lastUpdate <= updateInterval )
        return;
    lastUpdate = m_world->GetTime();

    expSonar.hitCount = 0;

    // Initialise gui data
    //
    #ifdef INCLUDE_RTK
        m_hit_count = 0;
    #endif

    // Compute fov, range, etc
    //
    double dr = 1.0 / m_world->ppm;
    double max_range = m_max_range;
    double min_range = m_min_range;


    // Check bounds
    //
    ASSERT((size_t) m_sonar_count <= sizeof(m_range) / sizeof(m_range[0]));
    
    // Do each sonar
    //
    for (int s = 0; s < m_sonar_count; s++)
    {
        // Compute parameters of scan line
        //
        double ox = m_sonar[s][0];
        double oy = m_sonar[s][1];
        double oth = m_sonar[s][2];
        LocalToGlobal(ox, oy, oth);

        double px = ox;
        double py = oy;
        double pth = oth;
        
        // Compute the step for simple ray-tracing
        //
        double dx = dr * cos(pth);
        double dy = dr * sin(pth);

        // Look along scan line for obstacles
        // Could make this an int again for a slight speed-up.
        //
        double range;
        for (range = 0; range < max_range; range += dr)
        {
            // Look in the laser layer for obstacles
            //
            uint8_t cell = m_world->GetCell(px, py, layer_obstacle);
            if (range > min_range && cell != 0)           
                break;
            px += dx;
            py += dy;
        }

        // Store range in mm in network byte order
        //
        m_range[s] = htons((uint16_t) (range * 1000));

        // Update the gui data
        //
        #ifdef INCLUDE_RTK
            m_hit[m_hit_count][0][0] = ox;
            m_hit[m_hit_count][0][1] = oy;
            m_hit[m_hit_count][1][0] = px;
            m_hit[m_hit_count][1][1] = py;
            m_hit_count++;
        #endif

	expSonar.hitPts[expSonar.hitCount].x = px;// * m_world->ppm;
	expSonar.hitPts[expSonar.hitCount].y = py;// * m_world->ppm;
	expSonar.hitCount++;
    }
	
    PutData(m_range, sizeof( unsigned short ) * SONARSAMPLES );
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CSonarDevice::OnUiUpdate(RtkUiDrawData *pData)
{
    // Draw our children
    //
    CEntity::OnUiUpdate(pData);
    
    // Draw ourself
    //
    pData->begin_section("global", "sonar");
    
    if (pData->draw_layer("scan", true))
        if (IsSubscribed())
            DrawScan(pData);
    
    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CSonarDevice::OnUiMouse(RtkUiMouseData *pData)
{
    CEntity::OnUiMouse(pData);
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser scan
//
void CSonarDevice::DrawScan(RtkUiDrawData *pData)
{
    #define SCAN_COLOR RTK_RGB(0, 255, 255)
    
    pData->set_color(SCAN_COLOR);

    // Draw rays coming out of each sonar
    //
    for (int s = 0; s < m_sonar_count; s++)
        pData->line(m_hit[s][0][0], m_hit[s][0][1], m_hit[s][1][0], m_hit[s][1][1]);
}

#endif


///////////////////////////////////////////////////////////////////////////
// *** HACK -- this could be put in an array ahoward
// Get the pose of the sonar
//
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


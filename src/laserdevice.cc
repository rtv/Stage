///////////////////////////////////////////////////////////////////////////
//
// File: laserdevice.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Simulates the Player CLaserDevice (the SICK laser)
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/laserdevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.23.2.3 $
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
#include <stage.h>
#include <math.h>
#include "world.hh"
#include "laserdevice.hh"

#define DEBUG
#undef VERBOSE


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CLaserDevice::CLaserDevice(CWorld *world, 
			   CEntity *parent )
        : CEntity(world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_laser_data_t );
  m_command_len = 0;
  m_config_len  = 0;//sizeof( player_laser_config_t );
  
  m_player_type = PLAYER_LASER_CODE; // from player's messages.h

  m_stage_type = LaserTurretType;

  laser_return = 1;
  //obstacle_return = 1;
   
  // Laser update rate (readings/sec)
  //
  //m_update_rate = 360 / 0.200; // 5Hz

  m_interval = 0.2; 

  m_last_update = 0;
  m_scan_res = DTOR(0.50);
  m_scan_min = DTOR(-90);
  m_scan_max = DTOR(+90);
  m_scan_count = 361;
  m_intensity = false;
  
  m_max_range = 8.0;

  // Set this flag to make the laser transparent to other lasers
  //
  m_transparent = false;
  
  // Dimensions of laser
  //
  m_size_x = 0.155;
  m_size_y = 0.155;
  
#ifdef INCLUDE_RTK
  m_draggable = true; 
  m_mouse_radius = sqrt(m_size_x * m_size_x + m_size_y * m_size_y);
  m_hit_count = 0;
#endif

}

///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CLaserDevice::Load(int argc, char **argv)
{
    if (!CEntity::Load(argc, argv))
        return false;

    for (int i = 0; i < argc;)
    {
        if (strcmp(argv[i], "transparent") == 0)
        {
            m_transparent = true;
            i += 1;
        }
        else
            i++;
    }
    return true;
}

// Save the object
//
bool CLaserDevice::Save(int &argc, char **argv)
{
    if (!CEntity::Save(argc, argv))
        return false;

    if (m_transparent)
        argv[argc++] = strdup("transparent");
    
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the laser data
//
void CLaserDevice::Update( double sim_time )
{
    CEntity::Update( sim_time ); // inherit useful debug output

    ASSERT(m_world != NULL);
    
    // UPDATE OUR RENDERING

    double x, y, th;
    GetGlobalPose( x,y,th );
    
    // if we've moved 
    if( (m_map_px != x) || (m_map_py != y) || (m_map_pth != th ) )
      {
	// Undraw ourselves from the world
	//
	if (!m_transparent) // should replace m_transparent with laser_return
	  {
	    m_world->matrix->mode = mode_unset;
	    m_world->SetRectangle( m_map_px, m_map_py, m_map_pth, 
				   m_size_x, m_size_y, this );
	  }
	
	m_map_px = x; // update our render position
	m_map_py = y;
	m_map_pth = th;
	
	// Redraw outselves in the world
	//
	if (!m_transparent )
	  {
	    m_world->matrix->mode = mode_set;
	    m_world->SetRectangle( m_map_px, m_map_py, m_map_pth, 
				   m_size_x, m_size_y, this );
	  }
      }
    
    // UPDATE OUR SENSOR DATA

    // Check to see if it's time to update the laser scan
    //
    if( sim_time - m_last_update > m_interval )
      {
	m_last_update = sim_time;
	
	if( Subscribed() )
	  {
	    // Check to see if the configuration has changed
	    //
	    CheckConfig();
	    // Generate new scan data and copy to data buffer
	    //
	    player_laser_data_t scan_data;
	    GenerateScanData( &scan_data );
	    PutData( &scan_data, sizeof( scan_data) );
	  }
	else
	  {
	    // If not subscribed,
	    // reset configuration to default.
	    //
	    m_scan_res = DTOR(0.50);
	    m_scan_min = DTOR(-90);
	    m_scan_max = DTOR(+90);
	    m_scan_count = 361;
	    m_intensity = false;
	  }
      }
}

///////////////////////////////////////////////////////////////////////////
// Check to see if the configuration has changed
//
bool CLaserDevice::CheckConfig()
{
    player_laser_config_t config;
    if (GetConfig( &config, sizeof(config) ) == 0)
        return false;  

    // Swap some bytes
    //
    config.resolution = ntohs(config.resolution);
    config.min_angle = ntohs(config.min_angle);
    config.max_angle = ntohs(config.max_angle);

    // Emulate behaviour of SICK laser range finder
    //
    if (config.resolution == 25)
    {
        config.min_angle = max((int)(config.min_angle), -5000);
        config.min_angle = min((int)config.min_angle, +5000);
        config.max_angle = max((int)config.max_angle, -5000);
        config.max_angle = min((int)config.max_angle, +5000);
        
        m_scan_res = DTOR((double) config.resolution / 100.0);
        m_scan_min = DTOR((double) config.min_angle / 100.0);
        m_scan_max = DTOR((double) config.max_angle / 100.0);
        m_scan_count = (int) ((m_scan_max - m_scan_min) / m_scan_res) + 1;
    }
    else if (config.resolution == 50 || config.resolution == 100)
    {
        if (abs(config.min_angle) > 9000 || abs(config.max_angle) > 9000)
            PRINT_MSG("warning: invalid laser configuration request");
        
        m_scan_res = DTOR((double) config.resolution / 100.0);
        m_scan_min = DTOR((double) config.min_angle / 100.0);
        m_scan_max = DTOR((double) config.max_angle / 100.0);
        m_scan_count = (int) ((m_scan_max - m_scan_min) / m_scan_res) + 1;
    }
    else
    {
        // Ignore invalid configurations
        //  
        PRINT_MSG("invalid laser configuration request");
        return false;
    }
        
    m_intensity = config.intensity;

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Generate scan data
//
bool CLaserDevice::GenerateScanData( player_laser_data_t *data )
{    
  // i put much of this back to integer, a

    // Get the pose of the laser in the global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Compute laser fov, range, etc
    //
    //double dr = 1.0 / m_world->ppm;
    double max_range = m_max_range * m_world->ppm;

    // See how many scan readings to interpolate.
    // To save time generating laser scans, we can
    // generate a scan with lower resolution and interpolate
    // the intermediate values.
    // We will interpolate <skip> out of <skip+1> readings.
    //
    int skip = (int) (m_world->m_laser_res / m_scan_res - 0.5);

#ifdef INCLUDE_RTK
    // Initialise gui data
    //
    m_hit_count = 0;
#endif

    // initialise our beacon detecting array
    m_visible_beacons.clear(); 


    // Set the header part of the data packet
    //
    data->range_count = htons(m_scan_count);
    data->resolution = htons((int) (100 * RTOD(m_scan_res)));
    data->min_angle = htons((int) (100 * RTOD(m_scan_min)));
    data->max_angle = htons((int) (100 * RTOD(m_scan_max)));

    // Make sure the data buffer is big enough
    //
    ASSERT(m_scan_count <= ARRAYSIZE(data->ranges));
            
    // Do each scan
    //
    for (int s = 0; s < m_scan_count;)
      {
	int intensity = 0;
	int range = 0;
	
	double bearing = s * m_scan_res + m_scan_min;
	
	// Compute parameters of scan line
	double pth = oth + bearing;
	
	//printf( "Ray: %d angle: %.2f\n", s, RTOD(pth) );

	// MOVED THIS BACK TO MAINLY INTEGER MATH, DID PIXEL/M SCALING
	// OUTSIDE THE LOOP AND ELIMINATED A FUNCTION CALL (yay!)
	// - RTV 8/20/01

	// hops along each axis
	double cospth = cos( pth );
	double sinpth = sin( pth );
 
	// start at the scan origin (pixel coords for speed)
	double cellx = (int)(ox * m_world->ppm);
	double celly = (int)(oy * m_world->ppm);
	
        // Look along scan line for obstacles
        for( range = 0; range < max_range; range++ ) // increase range 1 pixel at a time
        {
	  cellx += cospth;
	  celly += sinpth;
 
	  CEntity **ent = m_world->matrix->get_cell( (int)cellx, (int)celly );
	  if( !ent[0] ) ent = m_world->matrix->get_cell( (int)cellx+1, (int)celly );
	  
	  uint8_t retval = 0;
	  // see if any of the entities (other than this one) intercept the laser beam
	  // and store the first non-negative laser_return value
	  int s=0;
	  while( ent[s] ) // scan the pointer list
	    {
	      if( ent[s]->m_stage_type == LaserBeaconType )// we see a beacon		  
		{
		  CEntityPointer lbp;
		  lbp.ptr = ent[s];
		  m_visible_beacons.push_back( lbp ); // add it to the list
		}

	      if( ent[s] != this ) retval = ent[s]->laser_return;
	      if( retval > 0 ) break; // stop scanning the pointer list	      
	      s++;
	    }

	  if( retval > 0 ) break; // end the ray trace
        }
	
	//cout << s << ' ' << range << endl;

        // set laser value in mm
        //
        uint16_t v = (uint16_t) (1000.0 * ((double)range/m_world->ppm));

        // Add in the intensity values in the top 3 bits
        //
        //if (m_intensity)
	//  v = v | (((uint16_t) intensity) << 13);
        
        // Set the range
        //
        data->ranges[s++] = htons(v);

        // Skip some values to save time
        //
        for (int i = 0; i < skip && s < m_scan_count; i++)
            data->ranges[s++] = htons(v);

#ifdef INCLUDE_RTK
        // Update the gui data
        //
        m_hit[m_hit_count][0] = px;
        m_hit[m_hit_count][1] = py;
        m_hit_count++;
#endif
      }

    // remove all duplicates from the beacon list
    m_visible_beacons.unique(); 

    return true;
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CLaserDevice::OnUiUpdate(RtkUiDrawData *event)
{
    // Draw our children
    //
    CEntity::OnUiUpdate(event);
    
    // Draw ourself
    //
    event->begin_section("global", "laser");
    
    if (event->draw_layer("", true))
        DrawTurret(event);

    if (event->draw_layer("data", true))
    {
      if(Subscribed())
      {
        DrawScan(event);
        // call Update(), because we may have stolen the truth_poked
        Update(m_world->GetTime());
      }
    }
    
    event->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CLaserDevice::OnUiMouse(RtkUiMouseData *event)
{
    CEntity::OnUiMouse(event);
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser turret
//
void CLaserDevice::DrawTurret(RtkUiDrawData *event)
{
    #define TURRET_COLOR RTK_RGB(0, 0, 255)
    
    event->set_color(TURRET_COLOR);

    // Turret dimensions
    //
    double dx = m_size_x;
    double dy = m_size_y;

    // Get global pose
    //
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);
    
    // Draw the outline of the turret
    //
    event->ex_rectangle(gx, gy, gth, dx, dy); 
}


///////////////////////////////////////////////////////////////////////////
// Draw the laser scan
//
void CLaserDevice::DrawScan(RtkUiDrawData *event)
{
    #define SCAN_COLOR RTK_RGB(0, 0, 255)
    
    event->set_color(SCAN_COLOR);

    // Get global pose
    //
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);

    double qx, qy;
    qx = gx;
    qy = gy;
    
    for (int i = 0; i < m_hit_count; i++)
    {
        double px = m_hit[i][0];
        double py = m_hit[i][1];
        event->line(qx, qy, px, py);
        qx = px;
        qy = py;
    }
    event->line(qx, qy, gx, gy);
}

#endif




///////////////////////////////////////////////////////////////////////////
//
// File: ptzdevice.cc
// Author: Andrew Howard
// Date: 01 Dev 2000
// Desc: Simulates the Sony pan-tilt-zoom device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/ptzdevice.cc,v $
//  $Author: vaughan $
//  $Revision: 1.4 $
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

//#define ENABLE_TRACE 1

#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations
#include "world.h"
#include "robot.h"
#include "ptzdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPtzDevice::CPtzDevice(CRobot *robot,
                       void *buffer, size_t data_len, 
                       size_t command_len, size_t config_len)
        : CPlayerDevice(robot, buffer, data_len, command_len, config_len)
{   
    m_update_interval = 0.1;
    m_last_update = 0;

    // *** WARNING -- I just made these numbers up. ahoward
    //
    
    // these pans are the right numbers for our PTZ - RTV
    m_pan_min = -100;
    m_pan_max = +100;

    // and it's probably just as well to disable tilt for the moment. - RTV
    m_tilt_max = 0;
    m_tilt_min = 0;

    m_zoom_min = 0;
    m_zoom_max = 1024;

    // Field of view (for scaling zoom values)
    //
    // should look in the Sony manual to get these numbers right,
    // but they'll change with the lens, and we have 2 lenses
    // eventually all this stuff'll come from config files 
    m_fov_min = DTOR(100);
    m_fov_max = DTOR(10);

    m_pan = m_tilt = m_zoom = 0;

    undrawRequired = 0;
}


///////////////////////////////////////////////////////////////////////////
// Update the device data
//
bool CPtzDevice::Update()
{
    //TRACE0("updating");

    // Dont update anything if we are not subscribed
    //
    if (!IsSubscribed())
        return true;
    //TRACE0("is subscribed");
    
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);
    
    // Get pointers to the various bitmaps
    //
    Nimage *img = m_world->img;
    ASSERT(img != NULL);

    // if its time to recalculate ptz
    //
    if( m_world->timeNow - m_last_update <= m_update_interval )
        return false;
    m_last_update = m_world->timeNow;

    // Get the command string
    //
    //TRACE0("getting command");
    short command[3];
    if (GetCommand(command, sizeof(command)) != sizeof(command))
    {
        TRACE0("command buffer has incorrect length -- ignored");
        return false;
    }

    // Parse the command string
    //
    double pan = (short) ntohs(command[0]);
    double tilt = (short) ntohs(command[1]);
    double zoom = (unsigned short) ntohs(command[2]);

    // Threshold
    //
    pan = min(pan, m_pan_max);
    pan = max(pan, m_pan_min);

    tilt = min(tilt, m_tilt_max);
    tilt = max(tilt, m_tilt_min);

    zoom = min(zoom, m_zoom_max);
    zoom = max(zoom, m_zoom_min);

    // Set the current values
    // This basically assumes instantaneous changes
    // We could add a velocity in here later. ahoward
    //
    m_pan = pan;
    m_tilt = tilt;
    m_zoom = zoom;

    // Construct the return data buffer
    //
    short data[3];
    data[0] = htons((short) m_pan);
    data[1] = htons((short) m_tilt);
    data[2] = htons((unsigned short) m_zoom);

    //TRACE3("ptz %d %d %d", (int) m_pan, (int) m_tilt, (int) m_zoom);

    // Pass back the data
    //
    //TRACE0("returning data");
    PutData(data, sizeof(data));

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Get the pan/tilt/zoom values
// The pan and tilt are angles (in radians)
// The zoom is a focal length (in m)
//
void CPtzDevice::GetPTZ(double &pan, double &tilt, double &zoom)
{
    pan = DTOR(m_pan);
    tilt = DTOR(m_tilt);
    zoom = m_fov_min + (m_zoom - m_zoom_min) *
        (m_fov_max - m_fov_min) / (m_zoom_max - m_zoom_min);
};




bool CPtzDevice::GUIDraw( void )
{
  // dump out if noone is subscribed
  if( !IsSubscribed() || !m_robot->showDeviceDetail ) return true;
 
  // height of the cone - hard coded number is in meters
  double len = 1.0 * m_robot->world->ppm;
   
  double p, t, z;
  GetPTZ( p, t, z );

  double startAngle =  m_robot->a + p - z/2.0;
  double stopAngle  =  startAngle + z;

    drawPts[0].x = drawPts[3].x = (short)m_robot->x;
    drawPts[0].y = drawPts[3].y = (short)m_robot->y;
  
    drawPts[1].x = (short)(m_robot->x + len * cos( startAngle ));  
    drawPts[1].y = (short)(m_robot->y + len * sin( startAngle ));  

    drawPts[2].x = (short)(m_robot->x + len * cos( stopAngle ));  
    drawPts[2].y = (short)(m_robot->y + len * sin( stopAngle ));  

    m_world->win->SetForeground( m_world->win->magenta );
    //m_world->win->SetForeground( m_world->win->RobotDrawColor( m_robot )  );
    m_world->win->DrawLines( drawPts, 4 );
    
    undrawRequired = true;
    
    memcpy( unDrawPts, drawPts, sizeof( XPoint ) * 4 );
    
    return true; 
};  


bool CPtzDevice::GUIUnDraw()
{ 
  // dump out if noone is subscribed
  if( undrawRequired )
    {
      m_world->win->SetForeground( m_world->win->magenta );
      //m_world->win->SetForeground( m_world->win->RobotDrawColor( m_robot) );
      m_world->win->DrawLines( unDrawPts, 4 );
      
      undrawRequired = false;
    }
      
  return true; 
};







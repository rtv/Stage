///////////////////////////////////////////////////////////////////////////
//
// File: ptzdevice.cc
// Author: Andrew Howard
// Date: 01 Dev 2000
// Desc: Simulates the Sony pan-tilt-zoom device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/ptzdevice.cc,v $
//  $Author: inspectorg $
//  $Revision: 1.14 $
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

//#define ENABLE_RTK_TRACE 1

#include <math.h> // RTV - RH-7.0 compiler needs explicit declarations
#include "world.hh"
#include "ptzdevice.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPtzDevice::CPtzDevice(CWorld *world, CEntity *parent )
  : CEntity(world, parent )
{   
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_ptz_data_t ); 
  m_command_len = sizeof( player_ptz_cmd_t );
  m_config_len  = 0;//sizeof( player_ptz_config_t );
 
  m_player_type = PLAYER_PTZ_CODE;
  m_stage_type = PtzType;

  m_interval = 0.1;
  m_last_update = 0;
    
  SetColor(PTZ_COLOR);

  // Set default shape and geometry
  this->shape = ShapeRect;
  this->size_x = 0.165;
  this->size_y = 0.145;

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
  //
  m_fov_min = DTOR(100);
  m_fov_max = DTOR(10);
  
  m_pan = m_tilt = m_zoom = 0;
}


///////////////////////////////////////////////////////////////////////////
// Update the device data
//
void CPtzDevice::Update( double sim_time )
{
#ifdef DEBUG
  CEntity::Update( sim_time ); // inherit debug output
#endif
  
  ASSERT(m_world != NULL);
  
  
  // if its time to recalculate ptz
  //
  if( sim_time - m_last_update < m_interval )  return;
    
  m_last_update = sim_time;
  
  // Dont update anything if we are not subscribed
  //
  if(!Subscribed())
    {
      // reset the camera to (0,0,0) to better simulate the physical camera
      m_pan = 0;
      m_tilt = 0;
      m_zoom = 0;
      return;
    }
  
  // we're subscribed, so do the update
  
  // Get the command string
  //
  player_ptz_cmd_t cmd;
  int len = GetCommand(&cmd, sizeof(cmd));
  if (len > 0 && len != sizeof(cmd))
    {
      PRINT_MSG("command buffer has incorrect length -- ignored");
      return;
    }
  
  // Parse the command string (if there is one)
  //
  if (len > 0)
    {
      double pan = (short) ntohs(cmd.pan);
      double tilt = (short) ntohs(cmd.tilt);
      double zoom = (unsigned short) ntohs(cmd.zoom);
      
      // Threshold
      //
      pan = (pan < m_pan_max ? pan : m_pan_max);
      pan = (pan > m_pan_min ? pan : m_pan_min);
      
      tilt = (tilt < m_tilt_max ? tilt : m_tilt_max);
      tilt = (tilt > m_tilt_min ? tilt : m_tilt_min);
      
      zoom = (zoom < m_zoom_max ? zoom : m_zoom_max);
      zoom = (zoom > m_zoom_min ? zoom : m_zoom_min);
      
      // Set the current values
      // This basically assumes instantaneous changes
      // We could add a velocity in here later. ahoward
      //
      m_pan = pan;
      m_tilt = tilt;
      m_zoom = zoom;
    }
  
  // Construct the return data buffer
  //
  player_ptz_data_t data;
  data.pan = htons((short) m_pan);
  data.tilt = htons((short) m_tilt);
  data.zoom = htons((unsigned short) m_zoom);
  
  // Pass back the data
  //
  PutData(&data, sizeof(data));
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
}



#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CPtzDevice::OnUiUpdate(RtkUiDrawData *pData)
{
    CEntity::OnUiUpdate(pData);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CPtzDevice::OnUiMouse(RtkUiMouseData *pData)
{
    CEntity::OnUiMouse(pData);
}


#endif


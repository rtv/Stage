///////////////////////////////////////////////////////////////////////////
//
// File: ptzdevice.cc
// Author: Andrew Howard
// Date: 01 Dev 2000
// Desc: Simulates the Sony pan-tilt-zoom device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/ptzdevice.cc,v $
//  $Author: gerkey $
//  $Revision: 1.3.8.2 $
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
CPtzDevice::CPtzDevice(LibraryItem* libit,CWorld *world, CEntity *parent )
  : CPlayerEntity(libit,world, parent )
{   
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_ptz_data_t ); 
  m_command_len = sizeof( player_ptz_cmd_t );
  m_config_len  = 0;
  m_reply_len  = 0;
 
  m_player.code = PLAYER_PTZ_CODE;

  m_interval = 0.1;
  m_last_update = 0;

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
  
  m_zoom_min = 10;
  m_zoom_max = 120;
  
  m_pan = m_tilt = 0;
  m_zoom = m_zoom_max;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CPtzDevice::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CPtzDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  // Field of view (for scaling zoom values)
  //
  // should look in the Sony manual to get these numbers right,
  // but they'll change with the lens, and we have 2 lenses
  // eventually all this stuff'll come from config files
  //
  
  // these are my guesses, based on a vague memory that the basic Sony 
  // cameras lens has a 60-degree FOV and 12x zoom (and an assumption of linear
  // zoom.  note that the normal/wide lens issue is dealt with via a 
  // .world file option - BPG
  double normal_lens_fov = 60.0;
  double max_zoom = 12.0;
  double wide_lens_multiplier = 2.0;

  // Read ptz settings
  const char *lens = worldfile->ReadString(section, "lens", "normal");

  if(!strcmp(lens, "normal"))
    m_zoom_max = normal_lens_fov;
  else if(!strcmp(lens, "wide"))
    m_zoom_max = normal_lens_fov * wide_lens_multiplier;
  else
  {
    PRINT_ERR1(" ignoring unknown lens specifier \"%s\"\n", lens);
    m_zoom_max = normal_lens_fov;
  }

  m_zoom_min = m_zoom_max / max_zoom;
  m_zoom = m_zoom_max;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the device data
//
void CPtzDevice::Update( double sim_time )
{
  char buffer[PLAYER_MAX_REQREP_SIZE];
  void *client;

  CPlayerEntity::Update( sim_time );
  
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
    m_zoom = m_zoom_max;
    return;
  }
  
  // we're subscribed, so do the update

  // we don't support any config requests, but some exist for the ptz
  // interface, so we'll NACK to all of them
  if(GetConfig(&client, buffer, sizeof(buffer)) > 0)
  {
    PRINT_WARN("tried to make unsupported configuration change to simulated ptz device");
                
    PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
  }

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
    // TODO: put a little controller in here to pan gradually
    // rather than instantly for better realism - rtv
    double pan = (short) ntohs(cmd.pan);
    double tilt = (short) ntohs(cmd.tilt);
    double zoom = (short) ntohs(cmd.zoom);
      
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
  data.zoom = htons((short) m_zoom);
  data.panspeed = data.tiltspeed = 0;
  
  // Pass back the data
  //
  PutData(&data, sizeof(data));
}


///////////////////////////////////////////////////////////////////////////
// Get the pan/tilt/zoom values
// All values are angles.
void CPtzDevice::GetPTZ(double &pan, double &tilt, double &zoom)
{
  pan = DTOR(m_pan);
  tilt = DTOR(m_tilt);
  zoom = DTOR(m_zoom);
  return;
}


#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CPtzDevice::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // Create a figure representing this object
  this->fov_fig = rtk_fig_create(m_world->canvas, NULL, 49);

  // Set the color
  rtk_fig_color_rgb32(this->fov_fig, this->color);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CPtzDevice::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->fov_fig);

  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CPtzDevice::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();
 
  // Get global pose
  double gx, gy, gth;
  GetGlobalPose(gx, gy, gth);

  rtk_fig_clear(this->fov_fig);
  rtk_fig_origin(this->fov_fig, gx, gy, gth );

  player_ptz_data_t data;

  // if a client is subscribed to this device
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->lib_entry->type_num) )
  {
    // attempt to get the right size chunk of data from the mmapped buffer
    if( GetData( &data, sizeof(data) ) == sizeof(data) )
    { 
      
      short pan_deg = (short)ntohs(data.pan);
      //short tilt_deg = (short)ntohs(data.tilt); // unused..?
      //unsigned short zoom = ntohs(data.zoom);

      double pan = DTOR((double)pan_deg);
      
      double ox, oy;
      double fx;

      // Camera field of view in x-direction (radians), based on zoom
      fx = DTOR(m_zoom);
      
      ox = 100 * cos(pan);
      oy = 100 * sin(pan);
      rtk_fig_line(this->fov_fig, 0.0, 0.0, ox, oy);
      ox = 100 * cos(pan + fx / 2);
      oy = 100 * sin(pan + fx / 2);
      rtk_fig_line(this->fov_fig, 0.0, 0.0, ox, oy);
      ox = 100 * cos(pan - fx / 2);
      oy = 100 * sin(pan - fx / 2);
      rtk_fig_line(this->fov_fig, 0.0, 0.0, ox, oy);      
    }
  }
}

#endif


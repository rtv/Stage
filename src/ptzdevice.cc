///////////////////////////////////////////////////////////////////////////
//
// File: ptzdevice.cc
// Author: Andrew Howard
// Date: 01 Dev 2000
// Desc: Simulates the Sony pan-tilt-zoom device
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/ptzdevice.cc,v $
//  $Author: rtv $
//  $Revision: 1.25 $
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

// register this device type with the Library
CEntity ptz_bootstrap( "ptz", 
		       PtzType, 
		       (void*)&CPtzDevice::Creator ); 

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPtzDevice::CPtzDevice(CWorld *world, CEntity *parent )
  : CPlayerEntity(world, parent )
{   
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_ptz_data_t ); 
  m_command_len = sizeof( player_ptz_cmd_t );
  m_config_len  = 0;
  m_reply_len  = 0;
 
  m_player.code = PLAYER_PTZ_CODE;
  this->stage_type = PtzType;
  this->color = ::LookupColor(PTZ_COLOR);
  
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
  
  m_zoom_min = 0;
  m_zoom_max = 1024;
  
  m_pan = m_tilt = m_zoom = 0;
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CPtzDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  char lens_str[32];
  
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
  double normal_lens_fov = DTOR(60.0);
  double max_zoom = 12.0;
  double wide_lens_multiplier = 2.0;

  // Read ptz settings
  strncpy(lens_str, worldfile->ReadString(section, "lens", "normal"),
          sizeof(lens_str));
  lens_str[sizeof(lens_str)-1] = '\0';

  if(!strcmp(lens_str, "normal"))
    m_fov_min = normal_lens_fov;
  else if(!strcmp(lens_str, "wide"))
    m_fov_min = normal_lens_fov * wide_lens_multiplier;
  else
  {
    fprintf(stderr, "CPtzDevice: Warning: ignoring unknown lens "
            "specifier \"%s\"\n", lens_str);
    m_fov_min = normal_lens_fov;
  }

  m_fov_max = m_fov_min / max_zoom;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Update the device data
//
void CPtzDevice::Update( double sim_time )
{
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
      // TODO: put a little controller in here to pan gradually
      // rather than instantly for better realism - rtv
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
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->stage_type) )
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
      fx = m_fov_min + (m_zoom - m_zoom_min) * 
              (m_fov_max - m_fov_min) / (m_zoom_max - m_zoom_min);
      
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


///////////////////////////////////////////////////////////////////////////
//
// File: entity.cc
// Author: Andrew Howard
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
//  $Id: entity.cc,v 1.30 2001-12-20 03:11:47 vaughan Exp $
//
///////////////////////////////////////////////////////////////////////////
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h> 
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream.h>
#include <values.h>  // for MAXFLOAT

#include "entity.hh"
#include "world.hh"

//#define DEBUG
#undef DEBUG
//#undef VERBOSE

// entities use this port until the config file
// changes global_player_port
int global_current_player_port = PLAYER_PORTNUM;

// init static member
CWorld* CEntity::m_world = 0;
  

///////////////////////////////////////////////////////////////////////////
// Minimal constructor
// Requires a pointer to the parent and a pointer to the world.
//
CEntity::CEntity(CWorld *world, CEntity *parent_object )
{
    m_world = world;
    m_parent_object = parent_object;
    m_default_object = this;

    m_stage_type = NullType; // overwritten by subclasses
  
    m_line = -1;
    m_type[0] = 0;
    m_name[0] = 0;
    
    // all truth connections should send this truth
    memset( &m_dirty, true, sizeof(bool) * MAX_POSE_CONNECTIONS );
    m_last_pixel_x = m_last_pixel_y = m_last_degree = 0;

    // by default, entities don't show up in any sensors
    // these must be enabled explicitly in each subclass
    laser_return = LaserTransparent;
    sonar_return = false;
    obstacle_return = false;
    puck_return = false;
    channel_return = -1; // transparent
    idar_return = IDARReflect;

    m_dependent_attached = false;

    m_player_index = 0; // these are all set in load() 
    m_player_port = global_current_player_port;
    m_player_type = 0;
    
    // the default mananger of this entity is this computer
    strcpy( m_hostname, "localhost" );
    
    // init all the sizes
    m_lx = m_ly = m_lth = 0;
    m_size_x = 0; m_size_y = 0;
    m_offset_x = m_offset_y = 0;

    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;

    // initialize color description and values
    strcpy(m_color_desc, "red");
    m_color.red = 255;
    m_color.green = 0;
    m_color.blue = 0;

    m_com_vr = 0; // doesn't move
    
    m_mass = 1000.0; // unmoveably MASSIVE! by default

    m_interval = 0.1; // update interval in seconds 
    m_last_update = -MAXFLOAT; // initialized 

    m_info_len    = sizeof( player_stage_info_t ); // same size for all types
    m_data_len    = 0; // type specific - set in subclasses
    m_command_len = 0; // ditto
    m_config_len  = 0; // ditto

    m_info_io     = NULL; // instance specific pointers into mmap
    m_data_io     = NULL; 
    m_command_io  = NULL;
    m_config_io   = NULL;

#ifdef INCLUDE_RTK
    m_draggable = false; 
    m_mouse_radius = 0;
    m_mouse_ready = false;
    m_dragging = false;
    m_rtk_color = RTK_RGB(0, 0, 0);
#endif

}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CEntity::~CEntity()
{
  // remove our name in the filesystem
  unlink( m_device_filename );
}

///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CEntity::Load(int argc, char **argv)
{
    for (int i = 0; i < argc;)
    {
        // Extract a name
        //
        if (strcmp(argv[i], "name") == 0 && i + 1 < argc)
        {
            strncpy(m_name, argv[i + 1], sizeof(m_name) - 1);
            i += 2;
        }
        

        // Extract pose from the argument list
        //
        else if (strcmp(argv[i], "pose") == 0 && i + 3 < argc)
        {
            double px = atof(argv[i + 1]) * m_world->unit_multiplier;
            double py = atof(argv[i + 2]) * m_world->unit_multiplier;
            double pa = atof(argv[i + 3]) * m_world->angle_multiplier;
            SetPose(px, py, pa);
            i += 4;
        }

        // set obstacle flag
        //
        else if (strcmp(argv[i], "obstacle") == 0 && i + 1 < argc)
	  {
            if (strcmp(argv[i + 1], "true") == 0)
	      obstacle_return = true;
            else if (strcmp(argv[i + 1], "false") == 0)
	      obstacle_return = false;
            else
	      PLAYER_MSG2("unrecognized token [%s %s]", argv[i], argv[i + 1]);
	    i+=2;
	  }
	    
        // set sonar flag
        //
        else if (strcmp(argv[i], "sonar") == 0 && i + 1 < argc)
	  {
            if (strcmp(argv[i + 1], "true") == 0)
	      sonar_return = true;
            else if (strcmp(argv[i + 1], "false") == 0)
	      sonar_return = false;
            else
	      PLAYER_MSG2("unrecognized token [%s %s]", argv[i], argv[i + 1]);
	    i+=2;
	  }
	    
        // set laser return
        //
        else if (strcmp(argv[i], "laser") == 0 && i + 1 < argc)
	  {
            if (strcmp(argv[i + 1], "true") == 0)
	      laser_return = LaserReflect;
            else if (strcmp(argv[i + 1], "false") == 0)
	      laser_return = LaserTransparent;
            else if (strcmp(argv[i + 1], "bright1") == 0)
	      laser_return = LaserBright1;
            else if (strcmp(argv[i + 1], "bright2") == 0)
	      laser_return = LaserBright2;
            else if (strcmp(argv[i + 1], "bright3") == 0)
	      laser_return = LaserBright3;
            else if (strcmp(argv[i + 1], "bright4") == 0)
	      laser_return = LaserBright4;
            else
	      PLAYER_MSG2("unrecognized token [%s %s]", argv[i], argv[i + 1]);
	    i+=2;
	  }
	    
        // Extract color
        //
        else if (strcmp(argv[i], "color") == 0 && i + 1 < argc)
        {
            strcpy(m_color_desc, argv[i + 1]);
            i += 2;
        }

        // Extract size
        //
        else if (strcmp(argv[i], "size") == 0 && i + 2 < argc)
        {
            m_size_x = atof(argv[i + 1]);
            m_size_y = atof(argv[i + 2]);
            i += 3;
        }
    
        // extract port number
        // one day we'll inherit our parent's port by default.
	// for now this becomes the current global port
	// i.e. everything takes this port until we specify another one
        else if (strcmp(argv[i], "port") == 0 && i + 1 < argc)
        {
            m_player_port = atoi(argv[i + 1]);
            i += 2;

	    global_current_player_port = m_player_port;
        }

        // extract index number
        else if (strcmp(argv[i], "index") == 0 && i + 1 < argc)
        {
            m_player_index = atoi(argv[i + 1]);
            i += 2;
        }

        else
        {
            PLAYER_MSG1("unrecognized token [%s]", argv[i]);
            i += 1;
        }
    }

    // don;t bother looking up red colors - the default m_color is red
    if( strcmp( m_color_desc, "red" ) != 0 )
      {
	// resolve the RGB value of the color name
	if( !m_world->ColorFromString( &m_color, m_color_desc ) )
	  {
	    printf( "warning: invalid color name %s; using default red instead\n", 
		    m_color_desc );
	    
	    m_color.red = 255;
	    m_color.green = m_color.blue = 0;
	  }
      }

#ifdef INCLUDE_RTK
    m_rtk_color = rtk_color_lookup(m_color_desc);
#endif
        
    return true;
} 


///////////////////////////////////////////////////////////////////////////
// Save the object
//
bool CEntity::Save(int &argc, char **argv)
{
    // Get the robot pose (relative to parent)
    //
    double px, py, pa;
    GetPose(px, py, pa);

    // Convert to strings
    //
    char sx[32];
    snprintf(sx, sizeof(sx), "%.2f", px);
    char sy[32];
    snprintf(sy, sizeof(sy), "%.2f", py);
    char sa[32];
    snprintf(sa, sizeof(sa), "%.0f", RTOD(pa));

    // Save pose
    //
    argv[argc++] = strdup("pose");
    argv[argc++] = strdup(sx);
    argv[argc++] = strdup(sy);
    argv[argc++] = strdup(sa);

    // Save color
    //
    argv[argc++] = strdup("color");
    argv[argc++] = strdup(m_color_desc);

    // Save name
    //
    if (m_name[0] != 0)
    {
        argv[argc++] = strdup("name");
        argv[argc++] = strdup(m_name);
    }
    
    // Save player port
    //
    char port[32];
    snprintf(port, sizeof(port), "%d", m_player_port);
    argv[argc++] = strdup("port");
    argv[argc++] = strdup(port);

    // save player index
    char index[32];
    snprintf(index, sizeof(index), "%d", m_player_index);
    argv[argc++] = strdup("index");
    argv[argc++] = strdup(index);
        
    // Save render settings
    //
    argv[argc++] = strdup("obstacle");    
    obstacle_return ? 
      argv[argc++] = strdup("true") : argv[argc++] = strdup("false"); 
    
    argv[argc++] = strdup("sonar");    
    sonar_return ? 
      argv[argc++] = strdup("true") : argv[argc++] = strdup("false");
    
    argv[argc++] = strdup("laser");    
    switch( laser_return )
      {
      case LaserTransparent: 
	argv[argc++] = strdup("false"); break;
      case LaserReflect: 
	argv[argc++] = strdup("true"); break;
      case LaserBright1: 
	argv[argc++] = strdup("bright1"); break;
      case LaserBright2: 
	argv[argc++] = strdup("bright2"); break;
      case LaserBright3: 
	argv[argc++] = strdup("bright3"); break;
      }
	
    return true;
}

///////////////////////////////////////////////////////////////////////////
// Startup routine
// a virtual function that lets some objects calculate the amount
// of shared memory space they'll need. (they may peek at other objects 
// or into the world, os we must do this after everything is created)
bool CEntity::Startup( void )
{
#ifdef DEBUG
  cout << "CEntity::Startup()" << endl;
#endif

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Setup  routine
// set up the IO pointers (data, command, config, truth)
// this has to be done before we can GetX or SetX
// but Startup() must be called before this to set the io sizes correctly  
bool CEntity::SetupIOPointers( char* io )
{
  //  puts( "CEntity::SetupIOPointers()" );

  ASSERT( io  ); //io pointer must be good
   
  m_world->LockShmem();

  // set the pointers into the shared memory region
  m_info_io    = (player_stage_info_t*)io; // info is the first record
  m_data_io    = (uint8_t*)m_info_io + m_info_len;
  m_command_io = (uint8_t*)m_data_io + m_data_len; 
  m_config_io  = (uint8_t*)m_command_io + m_command_len;

  m_info_io->len = SharedMemorySize(); // total size of all the shared data

  // set the lengths in the info structure

  m_info_io->data_len    =  (uint32_t)m_data_len;
  m_info_io->command_len =  (uint32_t)m_command_len;
  m_info_io->config_len  =  (uint32_t)m_config_len;

  m_info_io->data_avail    =  0;
  m_info_io->command_avail =  0;
  m_info_io->config_avail  =  0;

  m_info_io->player_id.port = m_player_port;
  m_info_io->player_id.index = m_player_index;
  m_info_io->player_id.type = m_player_type;
  m_info_io->subscribed = 0;

#ifdef DEBUG
  printf( "\t\t(%p) (%d,%d,%d) IO at %p\n"
	  "\t\ttotal: %d\tinfo: %d\tdata: %d (%d)\tcommand: %d (%d)\tconfig: %d (%d)\n ",
	  this,
	  m_info_io->player_id.port,
	  m_info_io->player_id.type,
	  m_info_io->player_id.index,
	  m_info_io,
	  m_info_len + m_data_len + m_command_len + m_config_len,
	  m_info_len,
	  m_info_io->data_len, m_data_len,
	  m_info_io->command_len, m_command_len,
	  m_info_io->config_len, m_config_len );

  fflush( stdout );
#endif

  m_world->UnlockShmem();

  // Put some initial data in the shared memory for Player to read when
  // it starts up.

  // publish some dummy data - all zeroed for now
  unsigned char dummy_data[ m_data_len ];
  memset( dummy_data, 0, m_data_len );
  PutData( dummy_data, m_data_len );

  return true;
}

int CEntity::SharedMemorySize( void )
{
  return( m_info_len + m_data_len + m_command_len + m_config_len );
}

///////////////////////////////////////////////////////////////////////////
// Shutdown routine
// 
void CEntity::Shutdown()
{
}


///////////////////////////////////////////////////////////////////////////
// Update the object's representation
//
void CEntity::Update( double sim_time )
{
#ifdef DEBUG
  //    printf( "S: Entity::Update() (%d,%d,%d) %d subs at %.2f\n",  
  //  m_player_port, m_player_type, m_player_index, 
  //    Subscribed(), sim_time ); 
#endif 
}


///////////////////////////////////////////////////////////////////////////
// Convert local to global coords
//
void CEntity::LocalToGlobal(double &px, double &py, double &pth)
{
    // Get the pose of our origin wrt global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Compute pose based on the parent's pose
    //
    double sx = ox + px * cos(oth) - py * sin(oth);
    double sy = oy + px * sin(oth) + py * cos(oth);
    double sth = oth + pth;

    px = sx;
    py = sy;
    pth = sth;
}


///////////////////////////////////////////////////////////////////////////
// Convert global to local coords
//
void CEntity::GlobalToLocal(double &px, double &py, double &pth)
{
    // Get the pose of our origin wrt global cs
    //
    double ox, oy, oth;
    GetGlobalPose(ox, oy, oth);

    // Compute pose based on the parent's pose
    //
    double sx =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
    double sy = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
    double sth = pth - oth;

    px = sx;
    py = sy;
    pth = sth;
}


///////////////////////////////////////////////////////////////////////////
// Set the objects pose in the parent cs
//
void CEntity::SetPose(double px, double py, double pth)
{
    // Set our pose wrt our parent
    //
    m_lx = px;
    m_ly = py;
    m_lth = pth;
}


///////////////////////////////////////////////////////////////////////////
// Get the objects pose in the parent cs
//
void CEntity::GetPose(double &px, double &py, double &pth)
{
    px = m_lx;
    py = m_ly;
    pth = m_lth;
}


///////////////////////////////////////////////////////////////////////////
// Set the objects pose in the global cs
//
void CEntity::SetGlobalPose(double px, double py, double pth)
{
    // Get the pose of our parent in the global cs
    //
    double ox = 0;
    double oy = 0;
    double oth = 0;
    if (m_parent_object)
        m_parent_object->GetGlobalPose(ox, oy, oth);
    
    // Compute our pose in the local cs
    //
    m_lx =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
    m_ly = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
    m_lth = pth - oth;

    // our position has probably changed, so we need to re-transmit our truth
    //MakeDirty();
}


///////////////////////////////////////////////////////////////////////////
// Get the objects pose in the global cs
//
void CEntity::GetGlobalPose(double &px, double &py, double &pth)
{
    // Get the pose of our parent in the global cs
    //
    double ox = 0;
    double oy = 0;
    double oth = 0;
    if (m_parent_object)
        m_parent_object->GetGlobalPose(ox, oy, oth);
    
    // Compute our pose in the global cs
    //
    px = ox + m_lx * cos(oth) - m_ly * sin(oth);
    py = oy + m_lx * sin(oth) + m_ly * cos(oth);
    pth = oth + m_lth;
}


////////////////////////////////////////////////////////////////////////////
// See if the given entity is one of our descendents
bool CEntity::IsDescendent(CEntity *entity)
{
    while (entity)
    {
        entity = entity->m_parent_object;
        if (entity == this)
            return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////////////////
// Copy the device data and info into the shared memory segment
//
size_t CEntity::PutData( void* data, size_t len )
{
  m_world->LockShmem();
  
#ifdef DEBUG  
  //  printf( "S: Entity::PutData() (%d,%d,%d) at %p\n", 
  //  m_info_io->player_id.port, 
  //  m_info_io->player_id.type, 
  //  m_info_io->player_id.index, data);
#endif  
 
  // the data mustn't be too big!
  if( len <= m_info_io->data_len )
    {
      // indicate that some data is available
      // and update the timestamp
      m_info_io->data_avail = len;
      m_info_io->data_timestamp_sec = m_world->m_sim_timeval.tv_sec;
      m_info_io->data_timestamp_usec = m_world->m_sim_timeval.tv_usec;
      
      // set the flag to show we're controlled locally or remotely
      m_info_io->local = m_local;
     
      memcpy( m_data_io, data, len); // export data
    }
  else
    {
      printf( "PutData() failed trying to put %d bytes; io buffer is "
	      "%d bytes.)\n", 
	      len, m_info_io->data_len  );
      fflush( stdout );
    }
  
  m_world->UnlockShmem();
  
  return len;
}

///////////////////////////////////////////////////////////////////////////
// Copy data from the shared memory segment
//
size_t CEntity::GetData( void* data, size_t len )
{   
#ifdef DEBUG
  printf( "S: getting type %d data at %p - ", 
	    m_player_type, m_data_io );
    fflush( stdout );
#endif

  m_world->LockShmem();

  // the data must be the right size!
  if( len <= m_info_io->data_avail )
    memcpy( data, m_data_io, len); // copy the data
  else
    {
#ifdef DEBUG
      printf( "error: requested %d data (%d bytes available)\n", 
	      len, m_info_io->data_avail );
      fflush( stdout );
#endif
      len = 0; // set the return value to indicate failure 
    }
  
  m_world->UnlockShmem();
  
  return len;
}



///////////////////////////////////////////////////////////////////////////
// Copy a command from the shared memory segment
//
size_t CEntity::GetCommand( void* cmd, size_t len )
{   
#ifdef DEBUG
  printf( "S: getting type %d cmd at %p - ", 
	    m_player_type, m_command_io );
    fflush( stdout );
#endif

  m_world->LockShmem();

  // the command must be the right size!
  if( len == m_info_io->command_len && len == m_info_io->command_avail )
    memcpy( cmd, m_command_io, len); // import device-specific data
  else
    {
#ifdef DEBUG
      printf( "no command found (%d bytes available)\n", 
	      m_info_io->command_avail );
      fflush( stdout );
#endif
      len = 0; // set the return value to indicate failure 
    }
  
  m_world->UnlockShmem();
  
  return len;
}


///////////////////////////////////////////////////////////////////////////
// Read a configuration from the shared memory
// this is a little trickier 'cos configs are consumed
size_t CEntity::GetConfig( void* config, size_t len )
{  
  m_world->LockShmem();

  // the config data must be the right size!
  if( len == m_info_io->config_len && len == m_info_io->config_avail )
    memcpy( config, m_config_io, len); // import device-specific data
  else
    {
#ifdef DEBUG
      printf( "GetConfig() found no config (requested %d bytes;"
	      " io buffer is %d bytes; %d bytes are available)\n", 
	      len, m_info_io->config_len, m_info_io->config_avail );
      fflush( stdout );
#endif
      len = 0; // set the return value to indicate failure 
    }
  
  // reset the available size in the info structure to show that 
  // we consumed the message
  m_info_io->config_avail = 0;    
  
  m_world->UnlockShmem();
  
  return len;
}

///////////////////////////////////////////////////////////////////////////
// See if the PlayerDevice is subscribed
//
int CEntity::Subscribed() 
{
  //return true;

  m_world->LockShmem();
  int subscribed = m_info_io->subscribed;
  m_world->UnlockShmem();
  
  return( subscribed );
}

void CEntity::ComposeTruth( stage_truth_t* truth, int index )
{
  // compose a truth packet
  memset( truth, 0, sizeof(stage_truth_t) ); // just in case
  
  truth->stage_id = index;
  
  if( m_parent_object )
   {
     // find the index of our parent to use as an id
     for( int h=0; h<m_world->GetObjectCount(); h++ )
       if( m_world->GetObject(h) == m_parent_object )
	 {
	   truth->parent_id = h;
	   break;
	 }
   }
  else
    truth->parent_id = -1;

  assert( m_hostname );

  //printf( "hostname: %s\n", m_hostname );

  strncpy( truth->hostname, m_hostname, HOSTNAME_SIZE );

  truth->id.port = m_player_port;
  truth->id.type = m_player_type;
  truth->id.index = m_player_index;

  truth->stage_type = m_stage_type;

  // we don't want an echo
  truth->echo_request = false;

  truth->red   = (uint16_t)m_color.red;
  truth->green = (uint16_t)m_color.green;
  truth->blue  = (uint16_t)m_color.blue;

  double x, y, th;
  GetGlobalPose( x,y,th );
  
  // position and extents
  truth->x = (uint32_t)( x * 1000.0 );
  truth->y = (uint32_t)( y * 1000.0 );
  truth->w = (uint16_t)( m_size_x * 1000.0 );
  truth->h = (uint16_t)( m_size_y * 1000.0 );
 
  // center of rotation offsets
  truth->rotdx = (int16_t)( m_offset_x * 1000.0 );
  truth->rotdy = (int16_t)( m_offset_y * 1000.0 );

  // normalize degrees 0-360 (th is -/+PI)
  int degrees = (int)RTOD( th );
  if( degrees < 0 ) degrees += 360;

  truth->th = (uint16_t)degrees;  
}

void CEntity::MakeDirtyIfPixelChanged( void )
{
  double px, py, pth;
  GetGlobalPose( px, py, pth );
  
  // calculate the pixel we're at:
  int x = (int) (px * m_world->ppm);
  int y = (int) (py * m_world->ppm);
  int degree = (int)RTOD( NORMALIZE(pth) );
  
  // if we've moved pixel or 1 degree, then we should mark dirty to
  // update external clients (GUI or distributed stage siblings)
  if( m_last_pixel_x != x || m_last_pixel_y != y ||
    m_last_degree != degree )
    {
      memset( m_dirty, true, sizeof(m_dirty[0]) * MAX_POSE_CONNECTIONS );
     
      //puts( "dirty!" );
    }

  // store these quantized locations for next time
  m_last_pixel_x = x;
  m_last_pixel_y = y;
  m_last_degree = degree;
};


#ifdef INCLUDE_RTK


///////////////////////////////////////////////////////////////////////////
// UI property message handler
//
void CEntity::OnUiProperty(RtkUiPropertyData* pData)
{
}


///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CEntity::OnUiUpdate(RtkUiDrawData *pData)
{
    pData->begin_section("global", "");

    // Draw a marker to show we are being dragged
    //
    if (pData->draw_layer("focus", true))
    {
        if (m_mouse_ready && m_draggable)
        {
            if (m_mouse_ready)
                pData->set_color(RTK_RGB(128, 128, 255));
            if (m_dragging)
                pData->set_color(RTK_RGB(0, 0, 255));
            
            double ox, oy, oth;
            GetGlobalPose(ox, oy, oth);
            
            pData->ellipse(ox - m_mouse_radius, oy - m_mouse_radius,
                           ox + m_mouse_radius, oy + m_mouse_radius);
            if (m_name[0] != 0)
                pData->draw_text(ox + m_mouse_radius, oy + m_mouse_radius, m_name);
        }
    }

    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CEntity::OnUiMouse(RtkUiMouseData *pData)
{
    pData->begin_section("global", "move");

    if (pData->use_mouse_mode("object"))
    {
        if (MouseMove(pData))
            pData->reset_button();
    }
    
    pData->end_section();
}


///////////////////////////////////////////////////////////////////////////
// Move object with the mouse
//
bool CEntity::MouseMove(RtkUiMouseData *pData)
{
    // Get current pose
    //
    double px, py, pth;
    GetGlobalPose(px, py, pth);
    
    // Get the mouse position
    //
    double mx, my;
    pData->get_point(mx, my);
    double mth = pth;
    
    double dx = mx - px;
    double dy = my - py;
    double dist = sqrt(dx * dx + dy * dy);
    
    // See of we are within range
    //
    m_mouse_ready = (dist < m_mouse_radius);
    
    // If the mouse is within range and we are draggable...
    //
    if (m_mouse_ready && m_draggable)
    {
        if (pData->is_button_down())
        {
            // Drag on left
            //
            if (pData->which_button() == 1)
                m_dragging = true;
            
            // Rotate on right
            //
            else if (pData->which_button() == 3)
            {
                m_dragging = true;
                mth += M_PI / 8;
            }
        }
        else if (pData->is_button_up())
            m_dragging = false;
    }   
    
    // If we are dragging, set the pose
    //
    if (m_dragging)
        SetGlobalPose(mx, my, mth);

    return (m_mouse_ready);
}

#endif


/////////////////////////////////////////////////////////////////////////
// -- create the memory map for IPC with Player 
//
bool CEntity::CreateDevice( void )
{
  // if this is not a player device, we bail right here
  if( m_player_type == 0 )
    return true;

  // otherwise, we set up a mmapped file in the tmp filesystem to share
  // data with player

  size_t mem = SharedMemorySize();
  
#ifdef DEBUG
  cout << "Shared memory allocation: " << mem 
       << " (" << mem << ")" << endl;
#endif
  
  sprintf( m_device_filename, "%s/%d.%d.%d", 
	  m_world->m_device_dir,
	   m_player_port,
	   m_player_type,
	   // m_world->StringType( m_stage_type), 
	   m_player_index );

#ifdef DEBUG
  printf( "Stage: creating device %s\n", m_device_filename );
#endif

  int tfd = open( m_device_filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
   
    
  // make the file the right size
  if( ftruncate( tfd, mem ) < 0 )
    {
      perror( "Failed to set file size" );
      return false;
    }
  
  caddr_t playerIO = (caddr_t)mmap( NULL, mem, PROT_READ | PROT_WRITE, 
				    MAP_SHARED, tfd, (off_t) 0);
  
  if (playerIO == MAP_FAILED )
    {
      perror( "Failed to map memory" );
      return false;
    }
  
  close( tfd ); // can close fd once mapped
  
  // Initialise entire space
  //
  memset(playerIO, 0, mem);
  
#ifdef DEBUG
  cout << "Successfully mapped shared memory." << endl;  
#endif  


  // Setup IO for all the objects - MUST DO THIS AFTER MAPPING SHARING MEMORY
  // each is passed a pointer to the start of its space in shared memory
  int entityOffset = 0;
  
  if( !SetupIOPointers( playerIO + entityOffset  ) )
    puts( "Stage warning: failed to setup entity IO pointers." );
     
  return true;
}

///////////////////////////////////////////////////////////////////////////
//
// File: entity.cc
// Author: Andrew Howard
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/entity.cc,v $
//  $Author: vaughan $
//  $Revision: 1.6.2.3 $
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

#include <math.h>
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
    m_id[0] = 0;

    // by default, entities show up in some sensors
    laser_return = 1;
    sonar_return = 1;
    obstacle_return = 1;

    m_dependent_attached = false;

    m_channel = -1;// we don't show up as an acts color channel by default

    m_player_index = 0; // these are all set in load() 
    m_player_port = 0;
    m_player_type = 0;
       
    // init all the sizes
    m_lx = m_ly = m_lth = 0;
    m_size_x = 0; m_size_y = 0;
    m_offset_x = m_offset_y = 0;

    // Set the initial map pose
    //
    m_map_px = m_map_py = m_map_pth = 0;

    strcpy(m_color_desc, "black");

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
    m_color = RTK_RGB(0, 0, 0);
#endif

}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CEntity::~CEntity()
{
}

///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CEntity::Load(int argc, char **argv)
{
    for (int i = 0; i < argc;)
    {
        // Extract pose from the argument list
        //
        if (strcmp(argv[i], "pose") == 0 && i + 3 < argc)
        {
            double px = atof(argv[i + 1]);
            double py = atof(argv[i + 2]);
            double pa = DTOR(atof(argv[i + 3]));
            SetPose(px, py, pa);
            i += 4;
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
    
	// Extract channel
        //
        else if (strcmp(argv[i], "channel") == 0 && i + 1 < argc)
        {
            m_channel = atoi(argv[i + 1]);
            i += 2;
        }

	// extract port number
	// one day we'll inherit our parent's port by default.
        else if (strcmp(argv[i], "port") == 0 && i + 1 < argc)
        {
            m_player_port = atoi(argv[i + 1]);
            i += 2;
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

#ifdef INCLUDE_RTK
    m_color = rtk_color_lookup(m_color_desc);
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

    // Add to argument list
    //
    argv[argc++] = strdup("pose");
    argv[argc++] = strdup(sx);
    argv[argc++] = strdup(sy);
    argv[argc++] = strdup(sa);

    // Save color
    //
    argv[argc++] = strdup("color");
    argv[argc++] = strdup(m_color_desc);

    // Save channel
    //
    char z[128];
    snprintf(z, sizeof(z), "%d", (int) m_channel);
    argv[argc++] = strdup("channel");
    argv[argc++] = strdup(z);

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
// Copy the device data and info into the shared memory segment
//
size_t CEntity::PutData( void* data, size_t len )
{
  // get the time for the timestamp
  //
  struct timeval curr;
  gettimeofday(&curr,NULL);
  
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
      m_info_io->data_timestamp_sec = curr.tv_sec;
      m_info_io->data_timestamp_usec = curr.tv_usec;
      
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

  // returns > 0 if we have subs or dependents or we've been poked
  // and cancels the poke flag
  //return(  subscribed || m_dependent_attached || truth_poked-- ); 
  //int retval = subscribed || truth_poked;
  //if(truth_poked)
  //truth_poked = !truth_poked;
  
  return( subscribed );
}

void CEntity::ComposeTruth( stage_truth_t* truth )
{
  // compose a truth packet
  memset( truth, 0, sizeof(stage_truth_t) ); // just in case
  
  truth->stage_id = (int)this;

  truth->id.port = m_player_port;
  truth->id.type = m_player_type;
  truth->id.index = m_player_index;

  truth->stage_type = m_stage_type;

  truth->channel = m_channel;

  if( m_parent_object )
    {
      truth->parent.port = m_parent_object->m_player_port;
      truth->parent.type = m_parent_object->m_player_type;
      truth->parent.index = m_parent_object->m_player_index;
    }
  else
    {
      truth->parent.port = 0;
      truth->parent.type = 0;
      truth->parent.index = 0;
    }
  
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
   
#ifdef VERBOSE
  //    printf( "Compose Truth: "
  //  "(%d,%d,%d) parent (%d,%d,%d) [%d,%d,%d]\n", 
  //  truth->id.port, 
  //  truth->id.type, 
  //  truth->id.index,
  //  truth->parent.port, 
  //  truth->parent.type, 
  //  truth->parent.index,
  //  truth->x, truth->y, truth->th );
  
  //fflush( stdout );
#endif
}



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
            pData->draw_text(ox + m_mouse_radius, oy + m_mouse_radius, m_id);
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
    {
        SetGlobalPose(mx, my, mth);
        // also poke the truth so the underlying representation changes
        truth_poked = 1;
    }

    return (m_mouse_ready);
}

#endif

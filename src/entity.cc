///////////////////////////////////////////////////////////////////////////
//
// File: entity.cc
// Author: Andrew Howard
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
//  $Id: entity.cc,v 1.50 2002-03-15 04:01:29 gsibley Exp $
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
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>

//#define DEBUG
//#define VERBOSE
#undef DEBUG
#undef VERBOSE

#include "entity.hh"
#include "raytrace.hh"
#include "world.hh"
#include "worldfile.hh"

// static rtp object sending to 127.0.0.1:7777
//CRTPPlayer rtp_player( 0x7F000001, 7777 );

///////////////////////////////////////////////////////////////////////////
// Minimal constructor
// Requires a pointer to the parent and a pointer to the world.
CEntity::CEntity(CWorld *world, CEntity *parent_object )
{
  PRINT_DEBUG( "CEntity::CEntity()" );
  
  //m_lock = NULL;

  this->lock_byte = world->GetObjectCount();
  //this->rtp_p = &rtp_player; // they all point to the same object just now

  m_world = world; 
  m_parent_object = parent_object;
  m_default_object = this;

  this->name = NULL;
  m_stage_type = NullType; // overwritten by subclasses

  memset(m_device_filename, 0, sizeof(m_device_filename));

  // Set default pose
  this->local_px = this->local_py = this->local_pth = 0;

  // Set global velocity to stopped
  this->vx = this->vy = this->vth = 0;

  // unmoveably MASSIVE! by default
  m_mass = 1000.0;
    
  // Set the default shape and geometry
  this->shape = ShapeNone;
  this->size_x = this->size_y = 0;
  this->origin_x = this->origin_y = 0;
  
  // initialize color description and values
  this->color.red = 255;
  this->color.green = 0;
  this->color.blue = 0;

  // by default, entities don't show up in any sensors
  // these must be enabled explicitly in each subclass
  obstacle_return = false;
  sonar_return = false;
  puck_return = false;
  vision_return = false;
  laser_return = LaserTransparent;
  idar_return = IDARTransparent;

  // Set the initial mapped pose to a dummy value
  this->map_px = this->map_py = this->map_pth = 0;
  
  m_dependent_attached = false;

  m_player_index = 0; // these are all set in load() 
  m_player_port = -1;
  m_player_type = 0;
    
  // all truth connections should send this truth
  memset( &m_dirty, true, sizeof(bool) * MAX_POSE_CONNECTIONS );
  m_last_pixel_x = m_last_pixel_y = m_last_degree = 0;

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

#ifdef INCLUDE_RTK2
  this->fig = NULL;
  this->fig_label = NULL;
  // By default, we can both translate and rotate objects
  this->movemask = RTK_MOVE_TRANS | RTK_MOVE_ROT;
#endif
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CEntity::~CEntity()
{
  //close( m_fd ); 
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CEntity::Load(CWorldFile *worldfile, int section)
{
  // Read the name
  this->name = worldfile->ReadString(section, "name", "");
      
  // Read the pose
  this->init_px = worldfile->ReadTupleLength(section, "pose", 0, 0);
  this->init_py = worldfile->ReadTupleLength(section, "pose", 1, 0);
  this->init_pth = worldfile->ReadTupleAngle(section, "pose", 2, 0);
  SetPose(this->init_px, this->init_py, this->init_pth);

  // Read the shape
  const char *shape_desc = worldfile->ReadString(section, "shape", NULL);
  if (shape_desc)
  {
    if (strcmp(shape_desc, "rect") == 0)
      this->shape = ShapeRect;
    else if (strcmp(shape_desc, "circle") == 0)
      this->shape = ShapeCircle;
    else
      PRINT_WARN1("invalid shape desc [%s]; using default", shape_desc);
  }

  // Read the size
  this->size_x = worldfile->ReadTupleLength(section, "size", 0, this->size_x);
  this->size_y = worldfile->ReadTupleLength(section, "size", 1, this->size_y);

  // Read the object color
  const char *color_desc = worldfile->ReadString(section, "color", NULL);
  if (color_desc)
    SetColor(color_desc);
  
  // Read the sensor flags
  this->obstacle_return = worldfile->ReadBool(section, "obstacle",
                                              this->obstacle_return);
  this->sonar_return = worldfile->ReadBool(section, "sonar",
                                           this->sonar_return);
  this->vision_return = worldfile->ReadBool(section, "vision",
                                            this->vision_return);

  if (worldfile->ReadBool(section, "laser", this->laser_return != LaserTransparent))
  {
    if (worldfile->ReadBool(section, "laser_bright", this->laser_return >= LaserBright1))
      this->laser_return = LaserBright1;
    else
      this->laser_return = LaserReflect;
  }
  else
    this->laser_return = LaserTransparent;
  
  // Read the player port
  // Default to the parent's port
  m_player_port = worldfile->ReadInt(section, "port", -1);
  if (m_player_port < 0)
  {
    if (m_parent_object)
      m_player_port = m_parent_object->m_player_port;
    else
      m_player_port = 0;
  }

  // Read the device index
  m_player_index = worldfile->ReadInt(section, "index", 0);
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Save the entity to the world file
bool CEntity::Save(CWorldFile *worldfile, int section)
{
  // Write the pose (but only if it has changed)
  double px, py, pth;
  GetPose(px, py, pth);
  if (px != this->init_px || py != this->init_py || pth != this->init_pth)
  {
    worldfile->WriteTupleLength(section, "pose", 0, px);
    worldfile->WriteTupleLength(section, "pose", 1, py);
    worldfile->WriteTupleAngle(section, "pose", 2, pth);
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
// A virtual function that lets objects do some initialization after
// everything has been loaded.
bool CEntity::Startup( void )
{
#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  RtkStartup();
#endif

  PRINT_DEBUG( "STARTUP" );

  // if this is not a player device, we bail right here
  if( m_player_type == 0 )
    return true;

  // otherwise, we set up a mmapped file in the tmp filesystem to share
  // data with player

  size_t mem = SharedMemorySize();
  PRINT_DEBUG1("shared memory alloc: %d", mem);
  
  snprintf( m_device_filename, sizeof(m_device_filename), "%s/%d.%d.%d", 
            m_world->m_device_dir, m_player_port, m_player_type, m_player_index );
  PRINT_DEBUG1("creating device %s", m_device_filename);

  int tfd;
  tfd = open( m_device_filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
    
  // make the file the right size
  if( ftruncate( tfd, mem ) < 0 )
  {
    PRINT_ERR1( "failed to set file size: %s", strerror(errno) );
    return false;
  }
  
  // it's a little larger than a player_stage_info_t, but that's the 
  // first part of the buffer, so we'll call it that
  player_stage_info_t* playerIO = 
    (player_stage_info_t*)mmap( NULL, mem, PROT_READ | PROT_WRITE, 
				MAP_SHARED, tfd, (off_t) 0);
  
  if (playerIO == MAP_FAILED )
    {
      PRINT_ERR1( "Failed to map memory: %s", strerror(errno) );
      return false;
    }
  
  //close( tfd ); // can close fd once mapped
  
  // Initialise entire space
  memset(playerIO, 0, mem);
  PRINT_DEBUG("successfully mapped shared memory");
  
  PRINT_DEBUG2( "S: mmapped %d bytes at %p\n", mem, playerIO );

  // we use the lock field in the player_stage_info_t structure to
  // control access with a semaphore.
  
  
  // initialise a POSIX semaphore
  //
  // we'd use this to create a semaphore that is shared across
  // processes if only linux supported it.  hopefully it'll show up in
  // the next kernel, so keep this around.  this should work on
  // BSD,Solaris, etc. (conditional compilation?)
  // 
  //m_lock = &playerIO->lock;
  //if( sem_init( m_lock, 1, 1 ) < 0 )
  //perror( "sem_init failed" );

  // we'll do file locking instead

  
  //PRINT_DEBUG1( "Stage: device lock at %p\n", m_lock );
  
  // try a lock
  assert( Lock() );
  
  // set the pointers into the shared memory region
  m_info_io    = playerIO; // info is the first record
  m_data_io    = (uint8_t*)m_info_io + m_info_len;
  m_command_io = (uint8_t*)m_data_io + m_data_len; 
  m_config_io  = (uint8_t*)m_command_io + m_command_len;
  
  m_info_io->len = SharedMemorySize(); // total size of all the shared data
  m_info_io->lockbyte = this->lock_byte; // record lock on this byte

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

//  #ifdef DEBUG
//    printf( "\t\t(%p) (%d,%d,%d) IO at %p\n"
//  	  "\t\ttotal: %d\tinfo: %d\tdata: %d (%d)\tcommand: %d (%d)\tconfig: %d (%d)\n ",
//  	  this,
//  	  m_info_io->player_id.port,
//  	  m_info_io->player_id.type,
//  	  m_info_io->player_id.index,
//  	  m_info_io,
//  	  m_info_len + m_data_len + m_command_len + m_config_len,
//  	  m_info_len,
//  	  m_info_io->data_len, m_data_len,
//  	  m_info_io->command_len, m_command_len,
//  	  m_info_io->config_len, m_config_len );
//  #endif  

  // try  an unlock
  assert( Unlock() );
  
  // Put some initial dummy data in the shared memory for Player to read when
  // it starts up.
  unsigned char dummy_data[ m_data_len ];
  memset( dummy_data, 0, m_data_len );
  PutData( dummy_data, m_data_len );

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine
void CEntity::Shutdown()
{
#ifdef INCLUDE_RTK2
  // Clean up the figure we created
  rtk_fig_destroy(this->fig);
#endif

  // if this is not a player device, we bail right here
  if( m_player_type == 0 )
    return;

  // remove our name in the filesystem
  if (m_device_filename[0])
    unlink( m_device_filename );
}


///////////////////////////////////////////////////////////////////////////
// Compute the total shared memory size for the device
int CEntity::SharedMemorySize( void )
{
  return( m_info_len + m_data_len + m_command_len + m_config_len );
}


///////////////////////////////////////////////////////////////////////////
// Update the object's representation
void CEntity::Update( double sim_time )
{
  //PRINT_DEBUG( "UPDATE" );

#ifdef INCLUDE_RTK2
  // Update the rtk gui
  RtkUpdate();
#endif
 
  //PRINT_DEBUG( "S: Entity::Update() (%d,%d,%d) %d subs at %.2f\n",  
  //       m_player_port, m_player_type, m_player_index, 
  //       Subscribed(), sim_time ); 
}


///////////////////////////////////////////////////////////////////////////
// Render the entity into the world
void CEntity::Map(double px, double py, double pth)
{
  MapEx(px, py, pth, true);
  this->map_px = px;
  this->map_py = py;
  this->map_pth = pth;
}


///////////////////////////////////////////////////////////////////////////
// Remove the entity from the world
void CEntity::UnMap()
{
  MapEx(this->map_px, this->map_py, this->map_pth, false);
}


///////////////////////////////////////////////////////////////////////////
// Remap ourself if we have moved
void CEntity::ReMap(double px, double py, double pth)
{
  if (fabs(px - this->map_px) < 1 / m_world->ppm &&
      fabs(py - this->map_py) < 1 / m_world->ppm &&
      pth == this->map_pth)
    return;
  
  UnMap();
  Map(px, py, pth);
}


///////////////////////////////////////////////////////////////////////////
// Primitive rendering function
void CEntity::MapEx(double px, double py, double pth, bool render)
{
  double qx = px + this->origin_x * cos(pth) - this->origin_y * sin(pth);
  double qy = py + this->origin_y * sin(pth) + this->origin_y * cos(pth);
  double qth = pth;
  double sx = this->size_x;
  double sy = this->size_y;

  switch (this->shape)
  {
    case ShapeRect:
      m_world->SetRectangle(qx, qy, qth, sx, sy, this, render);
      break;
    case ShapeCircle:
      m_world->SetCircle(qx, qy, sx / 2, this, render);
      break;
  }
}




///////////////////////////////////////////////////////////////////////////
// Check to see if the given pose will yield a collision with obstacles.
// Returns a pointer to the first entity we are in collision with, and stores
// the location of the hit in hitx,hity
// Returns NULL if not collisions.
// This function is useful for writing position devices.
CEntity *CEntity::TestCollision(double px, double py, double pth)
{
  double qx = px + this->origin_x * cos(pth) - this->origin_y * sin(pth);
  double qy = py + this->origin_y * sin(pth) + this->origin_y * cos(pth);
  double qth = pth;
  double sx = this->size_x;
  double sy = this->size_y;

  switch( this->shape ) 
  {
    case ShapeRect:
    {
      CRectangleIterator rit( qx, qy, qth, sx, sy, m_world->ppm, m_world->matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
	    return ent;
      }
      return NULL;
    }
    case ShapeCircle:
    {
      CCircleIterator rit( px, py, sx / 2, m_world->ppm, m_world->matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
	    return ent;
      }
      return NULL;
    }
  }
  return NULL;

}

// same as the above method, but stores the hit location in hitx, hity
CEntity *CEntity::TestCollision(double px, double py, double pth, 
				double &hitx, double &hity )
{
  double qx = px + this->origin_x * cos(pth) - this->origin_y * sin(pth);
  double qy = py + this->origin_y * sin(pth) + this->origin_y * cos(pth);
  double qth = pth;
  double sx = this->size_x;
  double sy = this->size_y;

  switch( this->shape ) 
  {
    case ShapeRect:
    {
      CRectangleIterator rit( qx, qy, qth, sx, sy, m_world->ppm, m_world->matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
        {
          rit.GetPos( hitx, hity );
          return ent;
        }
      }
      return NULL;
    }
    case ShapeCircle:
    {
      CCircleIterator rit( px, py, sx / 2, m_world->ppm, m_world->matrix );

      CEntity* ent;
      while( (ent = rit.GetNextEntity()) )
      {
        if( ent != this && ent->obstacle_return )
        {
          rit.GetPos( hitx, hity );
          return ent;
        }
      }
      return NULL;
    }
  }
  return NULL;

}



///////////////////////////////////////////////////////////////////////////
// Set the color of the entity
void CEntity::SetColor(const char *desc)
{
  if (!m_world->ColorFromString( &this->color, desc ) )
    PRINT_WARN1("invalid color name [%s]; using default color", desc );
}


///////////////////////////////////////////////////////////////////////////
// Convert local to global coords
void CEntity::LocalToGlobal(double &px, double &py, double &pth)
{
  // Get the pose of our origin wrt global cs
  double ox, oy, oth;
  GetGlobalPose(ox, oy, oth);

  // Compute pose based on the parent's pose
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
  double ox, oy, oth;
  GetGlobalPose(ox, oy, oth);

  // Compute pose based on the parent's pose
  double sx =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
  double sy = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
  double sth = pth - oth;

  px = sx;
  py = sy;
  pth = sth;
}


///////////////////////////////////////////////////////////////////////////
// Set the objects pose in the parent cs
void CEntity::SetPose(double px, double py, double pth)
{
  // Set our pose wrt our parent
  this->local_px = px;
  this->local_py = py;
  this->local_pth = pth;
}


///////////////////////////////////////////////////////////////////////////
// Get the objects pose in the parent cs
void CEntity::GetPose(double &px, double &py, double &pth)
{
  px = this->local_px;
  py = this->local_py;
  pth = this->local_pth;
}


///////////////////////////////////////////////////////////////////////////
// Set the objects pose in the global cs
void CEntity::SetGlobalPose(double px, double py, double pth)
{
  // Get the pose of our parent in the global cs
  double ox = 0;
  double oy = 0;
  double oth = 0;
  if (m_parent_object)
    m_parent_object->GetGlobalPose(ox, oy, oth);
    
  // Compute our pose in the local cs
  this->local_px =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
  this->local_py = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
  this->local_pth = pth - oth;
}


///////////////////////////////////////////////////////////////////////////
// Get the objects pose in the global cs
void CEntity::GetGlobalPose(double &px, double &py, double &pth)
{
  // Get the pose of our parent in the global cs
  double ox = 0;
  double oy = 0;
  double oth = 0;
  if (m_parent_object)
    m_parent_object->GetGlobalPose(ox, oy, oth);
    
  // Compute our pose in the global cs
  px = ox + this->local_px * cos(oth) - this->local_py * sin(oth);
  py = oy + this->local_px * sin(oth) + this->local_py * cos(oth);
  pth = oth + this->local_pth;
}


////////////////////////////////////////////////////////////////////////////
// Set the objects velocity in the global cs
void CEntity::SetGlobalVel(double vx, double vy, double vth)
{
  this->vx = vx;
  this->vy = vy;
  this->vth = vth;
}


////////////////////////////////////////////////////////////////////////////
// Get the objects velocity in the global cs
void CEntity::GetGlobalVel(double &vx, double &vy, double &vth)
{
  vx = this->vx;
  vy = this->vy;
  vth = this->vth;
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
  Lock();
  
  /*
  printf( "S: Entity::PutData() (%d,%d,%d) at %p\n", 
	  m_info_io->player_id.port, 
	  m_info_io->player_id.type, 
	  m_info_io->player_id.index, data);
  */

  // the data mustn't be too big!
  //if( len <= m_info_io->data_len )

  // RTV - the data must be EXACTLY the right size!
  if( len == m_info_io->data_len )
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
 
  Unlock();

  // if we have an rtp object we announce this data to the world
  //if( this->rtp_p ) rtp_p->SendData( data, len );

  return len;
}

///////////////////////////////////////////////////////////////////////////
// Copy data from the shared memory segment
//
size_t CEntity::GetData( void* data, size_t len )
{   
  PRINT_DEBUG2( "S: getting type %d data at %p - ", 
          m_player_type, m_data_io );
  fflush( stdout );

  Lock();

  // the data must be the right size!
  //if( len <= m_info_io->data_avail )
  
  // RTV - the data must be EXACTLY the right size!
  if( len == m_info_io->data_avail )
    memcpy( data, m_data_io, len); // copy the data
  else
  {
    PRINT_DEBUG2( "error: requested %d data (%d bytes available)\n", 
		 len, m_info_io->data_avail );
    
    len = 0; // set the return value to indicate failure 
  }
  
  Unlock();
  
  return len;
}



///////////////////////////////////////////////////////////////////////////
// Copy a command from the shared memory segment
//
size_t CEntity::GetCommand( void* cmd, size_t len )
{   
  PRINT_DEBUG2( "S: getting type %d cmd at %p - ", 
	       m_player_type, m_command_io );
  
  Lock();

  // the command must be EXACTLY the right size!
  if( len == m_info_io->command_len && len == m_info_io->command_avail )
    memcpy( cmd, m_command_io, len); // import device-specific data
  else
  {
    PRINT_DEBUG1( "no command found (%d bytes available)\n", 
		 m_info_io->command_avail );
    
    len = 0; // set the return value to indicate failure 
  }
  
  Unlock();
  
  return len;
}


///////////////////////////////////////////////////////////////////////////
// Read a configuration from the shared memory
size_t CEntity::GetConfig( void* config, size_t len )
{  
  Lock();

//    if (m_info_io->config_avail > 0)
//    {
//      // Copy data out of shared memory,
//      // but check for overflow.
//      if (len >= m_info_io->config_avail)
//        memcpy(config, m_config_io, len);
//      else
//      {
//        PRINT_WARN("config buffer overflow; discarding configuration request");
//        len = 0;
//      }
//      // Consume the configuration request
//      m_info_io->config_avail = 0;
//    }

  // made the config logic the same as command and data 
  // this seems to fix the config bug - why was it different?

  // the config MUST be the right size
  if (len == m_info_io->config_avail && len == m_info_io->config_len  )
    {
      memcpy(config, m_config_io, len);
      // Consume the configuration request
      m_info_io->config_avail = 0;
    }
  else
    {
      PRINT_DEBUG1( "no config found (%d bytes available)\n", 
		    m_info_io->config_avail ); 
      len = 0;
    }
  
  Unlock();
  return len;    
}


///////////////////////////////////////////////////////////////////////////
// See if the PlayerDevice is subscribed
int CEntity::Subscribed() 
{
  //return true;

  Lock();
  int subscribed = m_info_io->subscribed;
  Unlock();
  
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

  memcpy( &truth->hostaddr, &m_hostaddr, sizeof(truth->hostaddr) );

  truth->id.port = m_player_port;
  truth->id.type = m_player_type;
  truth->id.index = m_player_index;

  truth->stage_type = m_stage_type;

  // we don't want an echo
  truth->echo_request = false;

  truth->red   = (uint16_t)this->color.red;
  truth->green = (uint16_t)this->color.green;
  truth->blue  = (uint16_t)this->color.blue;

  double x, y, th;
  GetGlobalPose( x,y,th );
  
  // position and extents
  truth->x = (uint32_t)( x * 1000.0 );
  truth->y = (uint32_t)( y * 1000.0 );
  truth->w = (uint16_t)( this->size_x * 1000.0 );
  truth->h = (uint16_t)( this->size_y * 1000.0 );
 
  // center of rotation offsets
  truth->rotdx = (int16_t)( this->origin_x * 1000.0 );
  truth->rotdy = (int16_t)( this->origin_y * 1000.0 );

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
     
    //PRINT_DEBUG( "dirty!" );
  }

  // store these quantized locations for next time
  m_last_pixel_x = x;
  m_last_pixel_y = y;
  m_last_degree = degree;
};


///////////////////////////////////////////////////////////////////////////
// lock the shared mem
// Returns true on success
//
bool CEntity::Lock( void )
{
  // POSIX RECORD LOCKING METHOD
  struct flock cmd;

  cmd.l_type = F_WRLCK; // request write lock
  cmd.l_whence = SEEK_SET; // count bytes from start of file
  cmd.l_start = this->lock_byte; // lock my unique byte
  cmd.l_len = 1; // lock 1 byte

  fcntl( m_world->m_locks_fd, F_SETLKW, &cmd );

  // DEBUG: write into the file to show which byte is locked
  // X = locked, '_' = unlocked
  //lseek( m_world->m_locks_fd, this->lock_byte, SEEK_SET );
  //write(  m_world->m_locks_fd, "X", 1 );

  //////////////////////////////////////////////////////////////////
  // BSD file locking method
  // block until we can get an exclusive lock on this file
  //if( flock( m_fd, LOCK_EX ) != 0 )
  //perror( "flock() LOCK failed" );

  ///////////////////////////////////////////////////////////////////
  // POSIX semaphore method
 //assert( m_lock );
  //PRINT_DEBUG1( "%p", m_lock );
  //if( sem_wait( m_lock ) < 0 )
  //{
  //PRINT_ERR( "sem_wait failed" );
  //return false;
  //}

  return true;
}

///////////////////////////////////////////////////////////////////////////
// unlock the shared mem
//
bool CEntity::Unlock( void )
{
  // POSIX RECORD LOCKING METHOD
  struct flock cmd;

  cmd.l_type = F_UNLCK; // request unlock
  cmd.l_whence = SEEK_SET; // count bytes from start of file
  cmd.l_start = this->lock_byte; // unlock my unique byte
  cmd.l_len = 1; // unlock 1 byte

  // DEBUG: write into the file to show which byte is locked
  // X = locked, '_' = unlocked
  //lseek( m_world->m_locks_fd, this->lock_byte, SEEK_SET );
  //write(  m_world->m_locks_fd, "_", 1 );

  fcntl( m_world->m_locks_fd, F_SETLKW, &cmd );

  ///////////////////////////////////////////////////////////////////
  // BSD file locking method
  // block until we can get an exclusive lock on this file
  //if( flock( m_fd, LOCK_UN ) != 0 )
  //perror( "flock() UNLOCK failed" );

  ////////////////////////////////////////////////////////////////
  // POSIX semaphore method
 //assert( m_lock );
  //PRINT_DEBUG1( "%p", m_lock );
  //if( sem_post( m_lock ) < 0 )
  //{
  //PRINT_ERR( "sem_post failed" );
  //return false;
  //}

  return true;
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CEntity::RtkStartup()
{
  // Create a figure representing this object
  this->fig = rtk_fig_create(m_world->canvas, NULL, 50);

  // Set the color
  double r = this->color.red / 255.0;
  double g = this->color.green / 255.0;
  double b = this->color.blue / 255.0;
  rtk_fig_color(this->fig, r, g, b);

  // Compute geometry
  double qx = this->origin_x;
  double qy = this->origin_y;
  double sx = this->size_x;
  double sy = this->size_y;
  
  switch (this->shape)
  {
    case ShapeRect:
      rtk_fig_rectangle(this->fig, qx, qy, 0, sx, sy, false);
      break;
    case ShapeCircle:
      rtk_fig_ellipse(this->fig, qx, qy, 0, sx, sx, false);
      break;
  }

  // Create the label
  // By default, the label is not shown
  this->fig_label = rtk_fig_create(m_world->canvas, this->fig, 51);
  rtk_fig_show(this->fig_label, false);    
  rtk_fig_set_movemask(this->fig_label, 0);

  char label[1024];
  char tmp[1024];

  label[0] = 0;
  snprintf(tmp, sizeof(tmp), "name: %s", this->name);
  strncat(label, tmp, sizeof(label));
  snprintf(tmp, sizeof(tmp), "\ntype: %s", RtkGetStageType());
  strncat(label, tmp, sizeof(label));
  if (m_player_port > 0)
  {
    snprintf(tmp, sizeof(tmp), "\nplayer: %d:%d", m_player_port, m_player_index);
    strncat(label, tmp, sizeof(label));
  }

  qx = qx + 0.75 * sx;
  qy = qy + 0.75 * sy;
  rtk_fig_text(this->fig_label, qx, qy, 0, label);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CEntity::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->fig);
  rtk_fig_destroy(this->fig_label);
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CEntity::RtkUpdate()
{
  // We need to handle mouse dragging by the user.
  // We can only move top-level objects.
  // Do the test here, since some objects (eg pucks) may
  // change their parents.
  if (m_parent_object != NULL)
    rtk_fig_set_movemask(this->fig, 0);
  else
    rtk_fig_set_movemask(this->fig, this->movemask);

  // Make sure the entity and the figure have the same pose.
  // Either update the pose of the object in the world,
  // or update the pose of the figure in the GUI.
  if (rtk_fig_mouse_selected(this->fig))
  {
    double gx, gy, gth;
    rtk_fig_get_origin(this->fig, &gx, &gy, &gth);
    SetGlobalPose(gx, gy, gth);
  }
  else
  {
    double gx, gy, gth;
    GetGlobalPose(gx, gy, gth);
    rtk_fig_origin(this->fig, gx, gy, gth);
  }

  // Show the figure's label if it is selected
  if (rtk_fig_mouse_over(this->fig))
    rtk_fig_show(this->fig_label, true);
  else
    rtk_fig_show(this->fig_label, false);    
}


///////////////////////////////////////////////////////////////////////////
// Get a string describing the Stage type of the entity
const char *CEntity::RtkGetStageType()
{
  switch (m_stage_type)
  {
    case OmniPositionType:
      return "OmniPositionDev";
    case RectRobotType:
      return "RectRobotDev";
    default:
      return "Unknown";
  }
}

#endif





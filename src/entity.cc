/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Base class for every moveable entity.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: entity.cc,v 1.68.4.1 2002-07-08 02:36:48 rtv Exp $
 */

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
//#undef DEBUG
//#undef VERBOSE

#include "entity.hh"
#include "raytrace.hh"
#include "world.hh"
#include "worldfile.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
// Requires a pointer to the parent and a pointer to the world.
CEntity::CEntity(CWorld *world, CEntity *parent_entity )
{
  //PRINT_DEBUG( "CEntity::CEntity()" );
  this->lock_byte = world->GetEntityCount();
  
  m_world = world; 
  m_parent_entity = parent_entity;
  m_default_entity = this;

  // Set our default type (will be reset by subclass).
  this->stage_type = NullType;

  // Set the filename used for this device's mmap
  this->device_filename[0] = 0;

  // Set our default name
  this->name[0] = 0;

  // Set default pose
  this->local_px = this->local_py = this->local_pth = 0;

  // Set global velocity to stopped
  this->vx = this->vy = this->vth = 0;

  // unmoveably MASSIVE! by default
  this->mass = 1000.0;
    
  // Set the default shape and geometry
  this->shape = ShapeNone;
  this->size_x = this->size_y = 0;
  this->origin_x = this->origin_y = 0;

  // Set the default color
  this->color = 0xFF0000;

  m_local = false; 
  
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

  // by default, we are not a player device
  // player devices set this up sensibly in their constructor and ::Load() 
  // TODO - should create a player device subclass again :)
  // that would require changes in *many* device files. boo. rtv
  memset( &m_player, 0, sizeof(m_player) );
    
  // we start out NOT dirty - no one gets deltas unless they ask for 'em.
  SetDirty( 0 );
  
  m_last_pixel_x = m_last_pixel_y = m_last_degree = 0;

  m_interval = 0.1; // update interval in seconds 
  m_last_update = -MAXFLOAT; // initialized 

  m_info_len    = sizeof( player_stage_info_t ); // same size for all types
  m_data_len    = 0; // type specific - set in subclasses
  m_command_len = 0; // ditto
  m_config_len  = 0; // ditto
  m_reply_len   = 0; // ditto

  m_info_io     = NULL; // instance specific pointers into mmap
  m_data_io     = NULL; 
  m_command_io  = NULL;
  m_config_io   = NULL;
  m_reply_io    = NULL;

  m_reqqueue = NULL;
  m_repqueue = NULL;

#ifdef INCLUDE_RTK2
  // Default figures for drawing the entity.
  this->fig = NULL;
  this->fig_label = NULL;

  // By default, we can both translate and rotate the entity.
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
  strcpy( this->name, worldfile->ReadString(section, "name", "") );
      
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

  // Read the entity color
  this->color = worldfile->ReadColor(section, "color", this->color);

  const char *rvalue;
  
  // Obstacle return values
  if (this->obstacle_return)
    rvalue = "visible";
  else
    rvalue = "invisible";
  rvalue = worldfile->ReadString(section, "obstacle_return", rvalue);
  if (strcmp(rvalue, "visible") == 0)
    this->obstacle_return = 1;
  else
    this->obstacle_return = 0;

  // Sonar return values
  if (this->sonar_return)
    rvalue = "visible";
  else
    rvalue = "invisible";
  rvalue = worldfile->ReadString(section, "sonar_return", rvalue);
  if (strcmp(rvalue, "visible") == 0)
    this->sonar_return = 1;
  else
    this->sonar_return = 0;

  // Vision return values
  if (this->vision_return)
    rvalue = "visible";
  else
    rvalue = "invisible";
  rvalue = worldfile->ReadString(section, "vision_return", rvalue);
  if (strcmp(rvalue, "visible") == 0)
    this->vision_return = 1;
  else
    this->vision_return = 0;
  
  // Laser return values
  if (this->laser_return == LaserBright)
    rvalue = "bright";
  else if (this->laser_return == LaserVisible)
    rvalue = "visible";
  else
    rvalue = "invisible";
  rvalue = worldfile->ReadString(section, "laser_return", rvalue);
  if (strcmp(rvalue, "bright") == 0)
    this->laser_return = LaserBright;
  else if (strcmp(rvalue, "visible") == 0)
    this->laser_return = LaserVisible;
  else
    this->laser_return = LaserTransparent;
  
  // Read the player port (default 0)
  m_player.port = worldfile->ReadInt(section, "port", 0 );
  
  // if the port wasn't set, default to the parent's port
  // (assuming zero is not a valid port num)
  if (m_player.port == 0 && m_parent_entity )
    m_player.port = m_parent_entity->m_player.port;
  
  // Read the device index
  m_player.index = worldfile->ReadInt(section, "index", 0);
  
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
// A virtual function that lets entitys do some initialization after
// everything has been loaded.
bool CEntity::Startup( void )
{
#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  RtkStartup();
#endif

  // if this is not a player device, we're done. bail right here
  if( m_player.code == 0 )
    return true;

  // otherwise, we set up a mmapped file in the tmp filesystem to share
  // data with player

  player_stage_info_t* playerIO = 0;

  size_t mem = SharedMemorySize();
   
  snprintf( this->device_filename, sizeof(this->device_filename), "%s/%d.%d.%d", 
            m_world->m_device_dir, m_player.port, m_player.code, m_player.index );
  
  PRINT_DEBUG1("creating device %s", m_device_filename);

  int tfd;
  if( (tfd = open( this->device_filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR )) == -1 )
  {
      
    PRINT_DEBUG2( "failed to open file %s for device at %p\n"
                  "assuming this is a client device!\n", 
                  this->device_filename, this );
      
    // make some memory to store the data
    assert( playerIO = (player_stage_info_t*) new char[ mem ] );

    // remove the filename so we don't try to unlink it
    this->device_filename[0] = 0;

    PRINT_DEBUG("successfully created client IO buffers");

  }
  else
  {
    // make the file the right size
    if( ftruncate( tfd, mem ) < 0 )
    {
      PRINT_ERR1( "failed to set file size: %s", strerror(errno) );
      return false;
    }
      
    // it's a little larger than a player_stage_info_t, but that's the 
    // first part of the buffer, so we'll call it that
    playerIO = 
      (player_stage_info_t*)mmap( NULL, mem, PROT_READ | PROT_WRITE, 
                                  MAP_SHARED, tfd, (off_t) 0);
      
    if (playerIO == MAP_FAILED )
    {
      PRINT_ERR1( "Failed to map memory: %s", strerror(errno) );
      return false;
    }
      
    close( tfd ); // can close fd once mapped
      
    //PRINT_DEBUG("successfully mapped shared memory");
      
    //PRINT_DEBUG2( "S: mmapped %d bytes at %p\n", mem, playerIO );
  }
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
  
  // we'll do file locking instead (see ::Lock() and ::Unlock())
  
  //PRINT_DEBUG1( "Stage: device lock at %p\n", m_lock );
  
  // try a lock
  assert( Lock() );
  
  // Initialise entire space
  memset(playerIO, 0, mem);
  
  // set the pointers into the shared memory region
  m_info_io    = playerIO; // info is the first record
  m_data_io    = (uint8_t*)m_info_io + m_info_len;
  m_command_io = (uint8_t*)m_data_io + m_data_len; 
  m_config_io  = (uint8_t*)m_command_io + m_command_len;
  m_reply_io   = (uint8_t*)(m_config_io + 
                            (m_config_len * sizeof(playerqueue_elt_t)));
  
  m_info_io->len = SharedMemorySize(); // total size of all the shared data
  m_info_io->lockbyte = this->lock_byte; // record lock on this byte
  
  // set the lengths in the info structure
  m_info_io->data_len    =  (uint32_t)m_data_len;
  m_info_io->command_len =  (uint32_t)m_command_len;
  m_info_io->config_len  =  (uint32_t)m_config_len;
  m_info_io->reply_len   =  (uint32_t)m_reply_len;
  
  m_info_io->data_avail    =  0;
  m_info_io->command_avail =  0;
  m_info_io->config_avail  =  0;
  
  m_info_io->player_id.port = m_player.port;
  m_info_io->player_id.index = m_player.index;
  m_info_io->player_id.code = m_player.code;
  m_info_io->subscribed = 0;

  // create the PlayerQueue entitys that we'll use to access requests and
  // replies.  pass in the chunks of memory that are already mmap()ed
  assert(m_reqqueue = new PlayerQueue(m_config_io,m_config_len));
  assert(m_repqueue = new PlayerQueue(m_reply_io,m_reply_len));

#ifdef DEBUG
  printf( "\t\t(%p) (%d,%d,%d) IO at %p\n"
          "\t\ttotal: %d\tinfo: %d\tdata: %d (%d)\tcommand: %d (%d)\tconfig: %d (%d)\n ",
          this,
          m_info_io->player_id.port,
          m_info_io->player_id.code,
          m_info_io->player_id.index,
          m_info_io,
          m_info_len + m_data_len + m_command_len + m_config_len,
          m_info_len,
          m_info_io->data_len, m_data_len,
          m_info_io->command_len, m_command_len,
          m_info_io->config_len, m_config_len );
#endif  

  // try  an unlock
  assert( Unlock() );
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine
void CEntity::Shutdown()
{
  PRINT_DEBUG( "entity shutting down" );

#ifdef INCLUDE_RTK2
  // Clean up the figure we created
  rtk_fig_destroy(this->fig);
#endif

  // if this is not a player device, we bail right here
  if( m_player.code == 0 )
    return;

  if( this->device_filename[0] )
    // remove our name in the filesystem
    if( unlink( this->device_filename ) == -1 )
      PRINT_ERR1( "Device failed to unlink it's IO file:%s", strerror(errno) );
}


///////////////////////////////////////////////////////////////////////////
// Compute the total shared memory size for the device
int CEntity::SharedMemorySize( void )
{
  return( m_info_len + m_data_len + m_command_len + 
          (m_config_len * sizeof(playerqueue_elt_t)) +
          (m_reply_len * sizeof(playerqueue_elt_t)));
}


///////////////////////////////////////////////////////////////////////////
// Update the entity's representation
void CEntity::Update( double sim_time )
{
  //PRINT_DEBUG( "UPDATE" );
 
  //PRINT_DEBUG4( "(%d,%d,%d) at %.2f\n",  
  //	m_player.port, m_player.type, m_player.index, sim_time );

  //PRINT_DEBUG1( "subs: %d\n", m_info_io->subscribed );
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


///////////////////////////////////////////////////////////////////////////
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
// Set the entitys pose in the parent cs
void CEntity::SetPose(double px, double py, double pth)
{
  // if the new position is different, call SetProperty to make the change.
  // the -1 indicates that this change is dirty on all connections
  
  if( this->local_px != px ) 
    SetProperty( -1, PropPoseX, &px, sizeof(px) );
  
  if( this->local_py != py ) 
    SetProperty( -1, PropPoseY, &py, sizeof(py) );
  
  if( this->local_pth != pth ) 
    SetProperty( -1, PropPoseTh, &pth, sizeof(pth) );
}


///////////////////////////////////////////////////////////////////////////
// Get the entitys pose in the parent cs
void CEntity::GetPose(double &px, double &py, double &pth)
{
  px = this->local_px;
  py = this->local_py;
  pth = this->local_pth;
}


///////////////////////////////////////////////////////////////////////////
// Set the entitys pose in the global cs
void CEntity::SetGlobalPose(double px, double py, double pth)
{
  // Get the pose of our parent in the global cs
  double ox = 0;
  double oy = 0;
  double oth = 0;
  
  if (m_parent_entity) m_parent_entity->GetGlobalPose(ox, oy, oth);
  
  // Compute our pose in the local cs
  double new_x  =  (px - ox) * cos(oth) + (py - oy) * sin(oth);
  double new_y  = -(px - ox) * sin(oth) + (py - oy) * cos(oth);
  double new_th = pth - oth;
  
  SetPose( new_x, new_y, new_th );
}


///////////////////////////////////////////////////////////////////////////
// Get the entitys pose in the global cs
void CEntity::GetGlobalPose(double &px, double &py, double &pth)
{
  // Get the pose of our parent in the global cs
  double ox = 0;
  double oy = 0;
  double oth = 0;
  if (m_parent_entity)
    m_parent_entity->GetGlobalPose(ox, oy, oth);
    
  // Compute our pose in the global cs
  px = ox + this->local_px * cos(oth) - this->local_py * sin(oth);
  py = oy + this->local_px * sin(oth) + this->local_py * cos(oth);
  pth = oth + this->local_pth;
}


////////////////////////////////////////////////////////////////////////////
// Set the entitys velocity in the global cs
void CEntity::SetGlobalVel(double vx, double vy, double vth)
{
  this->vx = vx;
  this->vy = vy;
  this->vth = vth;
}


////////////////////////////////////////////////////////////////////////////
// Get the entitys velocity in the global cs
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
    entity = entity->m_parent_entity;
    if (entity == this)
      return true;
  }
  return false;
}


/////////////////////////////////////////////////////////////////////////////
// Package up some data with a header and send it via RTP
void CEntity::AnnounceDataViaRTP( void* data, size_t len )
{
  device_hdr_t header;
  
  memset( &header, 0, sizeof(header) );
  
  header.major_type = this->stage_type;
     
  double x, y, th;
  GetGlobalPose( x, y, th );

  header.id = 0;

  // find our world index to use as an id (yuk! - fix)
  for( int h=0; h<m_world->GetEntityCount(); h++ )
  {
    if( m_world->GetEntity(h) == this )
    {
      header.id = h; // unique id for this entity
      break;
    }
  }
  assert( header.id != 0 );
  
  header.x = x;
  header.y = y;
  header.w = size_y; // yes this is the correct way around
  header.h = size_x;
  header.th = th;

  // need this if we go to integer stuff
  // normalize degrees 0-360 (th is -/+PI)
  //int degrees = (int)RTOD( th );
  //if( degrees < 0 ) degrees += 360;
  //header.th = (uint32_t)degrees;  
  //printf( "sending %d degrees\n", header.th );

  // timestamp is in milliseconds
  /*
  uint32_t ms = 
    m_info_io->data_timestamp_sec * 1000 + 
    m_info_io->data_timestamp_usec / 1000;
    */
  
  header.len = len;
  
  // combine the header and data in a single buffer 
  int buflen = sizeof(header) + len;
  char* buf = new char[buflen];
  memcpy( buf, &header, sizeof(header) );
  memcpy( buf+sizeof(header), data, len );
  
  printf( "CEntity sending %d:%d (%d) bytes\n",
          sizeof( device_hdr_t ), 
          len, buflen );
  
  //m_world->rtp_player->SendData( 0, buf, buflen, ms );
  
  delete[] buf;
}


///////////////////////////////////////////////////////////////////////////
// Copy data from the shared memory segment
size_t CEntity::GetIOData( void* dest, size_t dest_len,  
			   void* src, uint32_t* avail )
{
  Lock();

  assert( dest ); // the caller must have somewhere to put this data
  
  if( src == 0 ) // this device doesn't have any data here
    return 0; // so bail right away

  // if there is enough space for the data 
  if( dest_len >= (size_t)*avail) 
    memcpy( dest, src, dest_len ); // copy the data
  else
  {
    //PRINT_ERR2( "requested %d data but %d bytes available\n", 
    //	    dest_len, *avail );  
      
    // indicate failure - caller might have to handle this, but
    // this usually isn't an error - there was just nothing to fetch.
    dest_len = 0; 
  }

  Unlock();

  return dest_len;
}

size_t CEntity::GetData( void* data, size_t len )
{
  //PRINT_DEBUG( "f" );
  return GetIOData( data, len, m_data_io, &m_info_io->data_avail );
}

size_t CEntity::GetCommand( void* data, size_t len )
{
  //PRINT_DEBUG( "f" );
  return GetIOData( data, len, m_command_io, &m_info_io->command_avail );
}


///////////////////////////////////////////////////////////////////////////
// Copy data into the shared memory segment
size_t CEntity::PutIOData( void* dest, size_t dest_len,  
			   void* src, size_t src_len,
			   uint32_t* ts_sec, uint32_t* ts_usec, 
			   uint32_t* avail )
{  
  Lock();

  assert( dest );
  assert( src );
  assert( src_len > 0 );
  assert( dest_len > 0 );
  
  if( src_len == 0 )
    printf( "WIERD! attempt to write no bytes into a buffer" );

  if( src_len <= dest_len ) // if there is room for the data
  {
    memcpy( dest, src, src_len); // export the data

    // update the availability and timestamps
    *avail    = src_len;
    *ts_sec   = m_world->m_sim_timeval.tv_sec;
    *ts_usec  = m_world->m_sim_timeval.tv_usec;
  }
  else
  {
    PRINT_ERR2( "failed trying to put %d bytes; io buffer is "
                "%d bytes.)\n", src_len, dest_len  );
      
    src_len = 0; // indicate failure
  }
  
  Unlock();
  
  return src_len;
}


////////////////////////////////////////////////////////////////////////////
// Copy the data into shared memory & update the info buffer
size_t CEntity::PutData( void* data, size_t len )
{
  //PRINT_DEBUG4( "Putting %d bytes of data into device (%d,%d,%d)\n",
  //	len, m_player.port, m_player.type, m_player.index );

  // tell the server to export this data to anyone that needs it
  this->SetDirty( PropData, 1 );
    
  // copy the data into the mmapped data buffer.
  // also set the timestamp and available fields for the data
  return PutIOData( (void*)m_data_io, m_data_len,
                    data, len,
                    &m_info_io->data_timestamp_sec,
                    &m_info_io->data_timestamp_usec,
                    &m_info_io->data_avail );

}

////////////////////////////////////////////////////////////////////////////
// Copy the command into shared memory & update the info buffer
size_t CEntity::PutCommand( void* data, size_t len )
{
  // tell the server to export this command to anyone that needs it
  this->SetDirty( PropCommand, 1 );
  
  // copy the data into the mmapped command buffer.
  // also set the timestamp and available fields for the command
  return PutIOData( m_command_io, m_command_len,
		    data, len,
		    &m_info_io->command_timestamp_sec,
		    &m_info_io->command_timestamp_usec,
		    &m_info_io->command_avail );
  
  
}


///////////////////////////////////////////////////////////////////////////
// Read a configuration from the shared memory
size_t CEntity::GetConfig(void** client, void* config, size_t len )
{
  int size;
  player_device_id_t id;

  Lock();

  if((size = m_reqqueue->Pop(&id, client, (unsigned char*)config, len)) < 0)
  {
    Unlock();
    return(0);
  }

  // tell the server to export this config to anyone that needs it
  this->SetDirty( PropConfig, 1 );

  Unlock();
  return(size);
}

size_t CEntity::PutReply(void* client, unsigned short type,
                         struct timeval* ts, void* reply, size_t len)
{
  double seconds;
  struct timeval curr;
  
  // tell the server to export this reply to anyone that needs it
  this->SetDirty( PropReply, 1 );

  if(ts)
    curr = *ts;
  else
  {
    seconds = m_world->GetTime();
    curr.tv_sec = (long)seconds;
    curr.tv_usec = (long)((seconds - curr.tv_sec)*1000000);
  }

  Lock();
  m_repqueue->Push(NULL, client, type, &curr, (unsigned char*)reply, len);
  Unlock();
  return(0);
}


///////////////////////////////////////////////////////////////////////////
// Put a reply back on the reply queue.
size_t CEntity::PutReply(void* client, unsigned short type)
{
  return(PutReply(client, type, NULL, NULL, 0));
} 


///////////////////////////////////////////////////////////////////////////
// See if the PlayerDevice is subscribed
int CEntity::Subscribed() 
{
  int subscribed = 0; 
  
  if( m_info_io ) // if we have player connection at all
    { 
      Lock();
      // see if a player client is connected to this entity
      subscribed = m_info_io->subscribed;
      Unlock();
    }
  
  return( subscribed );
}

///////////////////////////////////////////////////////////////////////////
//// subscribe to the device (used by other devices that depend on this one)
void CEntity::Subscribe() 
{
  Lock();
  m_info_io->subscribed++;
  Unlock();
}

///////////////////////////////////////////////////////////////////////////
// unsubscribe from the device (used by other devices that depend on this one)
void CEntity::Unsubscribe()
{ 
  Lock();
  m_info_io->subscribed--;
  Unlock();
} 


///////////////////////////////////////////////////////////////////////////
// lock the shared mem
// Returns true on success
//
bool CEntity::Lock( void )
{
  if( m_world->m_locks_fd > 0 ) // if the world has a locking file open
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
    lseek( m_world->m_locks_fd, this->lock_byte, SEEK_SET );
    write(  m_world->m_locks_fd, "X", 1 );

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
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////
// unlock the shared mem
//
bool CEntity::Unlock( void )
{
  if( m_world->m_locks_fd > 0 ) // if the world has a locking file open
  {
      // POSIX RECORD LOCKING METHOD
      struct flock cmd;
      
      cmd.l_type = F_UNLCK; // request unlock
      cmd.l_whence = SEEK_SET; // count bytes from start of file
      cmd.l_start = this->lock_byte; // unlock my unique byte
      cmd.l_len = 1; // unlock 1 byte
      
      // DEBUG: write into the file to show which byte is locked
      // X = locked, '_' = unlocked
      lseek( m_world->m_locks_fd, this->lock_byte, SEEK_SET );
      write(  m_world->m_locks_fd, "_", 1 );
      
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
  }

  return true;
}

void CEntity::SetDirty( int con, char v )
{
  for( int i=0; i< ENTITY_LAST_PROPERTY; i++ )
    SetDirty( con, (EntityProperty)i, v );
}

void CEntity::SetDirty( EntityProperty prop, char v )
{
  for( int i=0; i<MAX_POSE_CONNECTIONS; i++ )
    SetDirty( i, prop, v );
};

void CEntity::SetDirty( int con, EntityProperty prop, char v )
{
  m_dirty[con][prop] = v;
};

void CEntity::SetDirty( char v )
{
  memset( m_dirty, v, 
	  sizeof(char) * MAX_POSE_CONNECTIONS * ENTITY_LAST_PROPERTY );
};

int CEntity::SetProperty( int con, EntityProperty property, 
			  void* value, size_t len )
{
  assert( value );
  assert( len > 0 );
  assert( (int)len < MAX_PROPERTY_DATA_LEN );
  
  switch( property )
  {
    case PropPlayerSubscriptions:
      PRINT_DEBUG1( "PLAYER SUBSCRIPTIONS %d", *(int*) value);
      
      if( m_info_io )
      {
        Lock();
        m_info_io->subscribed = *(int*)value;
        Unlock();
      }      
      break;
      
    case PropParent:
      // get a pointer to the (*value)'th entity - that's our new parent
      this->m_parent_entity = m_world->GetEntity( *(int*)value );     
      break;
    case PropSizeX:
      memcpy( &size_x, (double*)value, sizeof(size_x) );
      // force the device to re-render itself
      RtkShutdown();
      RtkStartup();
      break;
    case PropSizeY:
      memcpy( &size_y, (double*)value, sizeof(size_y) );
      // force the device to re-render itself
      RtkShutdown();
      RtkStartup();
      break;
    case PropPoseX:
      memcpy( &local_px, (double*)value, sizeof(local_px) );
      break;
    case PropPoseY:
      memcpy( &local_py, (double*)value, sizeof(local_py) );
      break;
    case PropPoseTh:
      memcpy( &local_pth, (double*)value, sizeof(local_pth) );
      break;
    case PropOriginX:
      memcpy( &origin_x, (double*)value, sizeof(origin_x) );
      break;
    case PropOriginY:
      memcpy( &origin_y, (double*)value, sizeof(origin_y) );
      break;
    case PropName:
      strcpy( name, (char*)value );
      // force the device to re-render itself
      RtkShutdown();
      RtkStartup();
      break;
    case PropColor:
      memcpy( &color, (StageColor*)value, sizeof(color) );
      // force the device to re-render itself
      RtkShutdown();
      RtkStartup();
      break;
    case PropShape:
      memcpy( &shape, (StageShape*)value, sizeof(shape) );
      // force the device to re-render itself
      RtkShutdown();
      RtkStartup();
      break;
    case PropLaserReturn:
      memcpy( &laser_return, (LaserReturn*)value, sizeof(laser_return) );
      break;
    case PropIdarReturn:
      memcpy( &idar_return, (IDARReturn*)value, sizeof(idar_return) );
      break;
    case PropSonarReturn:
      memcpy( &sonar_return, (bool*)value, sizeof(sonar_return) );
      break;
    case PropObstacleReturn:
      memcpy( &obstacle_return, (bool*)value, sizeof(obstacle_return) );
      break;
    case PropVisionReturn:
      memcpy( &vision_return, (bool*)value, sizeof(vision_return));
      break;
    case PropPuckReturn:
      memcpy( &puck_return, (bool*)value, sizeof(puck_return) );
      break;
    case PropPlayerId:
      memcpy( &m_player, (player_device_id_t*)value, sizeof(m_player) );
      break;
      
      // these properties manipulate the player IO buffers
    case PropCommand:
      PutCommand( value, len );
      break;
    case PropData:
      PutData( value, len );
      break;
    case PropConfig: // copy the  playerqueue's external memory chunk
    { 
      Lock();
      size_t len = m_config_len * sizeof(playerqueue_elt_t);
      memcpy( m_config_io, value, len ); 
      Unlock();
    }
    break;
    case PropReply:
    { 
      Lock();
      size_t len = m_reply_len * sizeof(playerqueue_elt_t);
      memcpy( m_reply_io, value, len ); 
      Unlock();
    }
    break;

    default:
      printf( "Stage Warning: attempting to set unknown property %d\n", 
              property );
      break;
  }
  
  // indicate that the property is dirty on all _but_ the connection
  // it came from - that way it gets propogated onto to other clients
  // and everyone stays in sync. (assuming no recursive connections...)
  
  this->SetDirty( property, 1 ); // dirty on all cons
  
  if( con != -1 ) // unless this was a local change 
    this->SetDirty( con, property, 0 ); // clean on this con

  return 0;
}

int CEntity::GetProperty( EntityProperty property, void* value )
{
  assert( value );

  // indicate no data - this should be overridden below
  int retval = 0;

  switch( property )
  {
    case PropPlayerSubscriptions:
      PRINT_DEBUG( "GET SUBS PROPERTY");
      { int subs = Subscribed();
      memcpy( value, (void*)&subs, sizeof(subs) ); 
      retval = sizeof(subs); }
      break;
    case PropParent:
      // find the parent's position in the world's entity array
      // if parent pointer is null or otherwise invalid, index is -1 
    { int parent_index = m_world->GetEntityIndex( m_parent_entity );
    memcpy( value, &parent_index, sizeof(parent_index) );
    retval = sizeof(parent_index); }
    break;
    case PropSizeX:
      memcpy( value, &size_x, sizeof(size_x) );
      retval = sizeof(size_x);
      break;
    case PropSizeY:
      memcpy( value, &size_y, sizeof(size_y) );
      retval = sizeof(size_y);
      break;
    case PropPoseX:
      memcpy( value, &local_px, sizeof(local_px) );
      retval = sizeof(local_px);
      break;
    case PropPoseY:
      memcpy( value, &local_py, sizeof(local_py) );
      retval = sizeof(local_py);
      break;
    case PropPoseTh:
      memcpy( value, &local_pth, sizeof(local_pth) );
      retval = sizeof(local_pth);
      break;
    case PropOriginX:
      memcpy( value, &origin_x, sizeof(origin_x) );
      retval = sizeof(origin_x);
      break;
      break;
    case PropOriginY:
      memcpy( value, &origin_y, sizeof(origin_y) );
      retval = sizeof(origin_y);
      break;
    case PropName:
      strcpy( (char*)value, name );
      retval = strlen(name);
      break;
    case PropColor:
      memcpy( value, &color, sizeof(color) );
      retval = sizeof(color);
      break;
    case PropShape:
      memcpy( value, &shape, sizeof(shape) );
      retval = sizeof(shape);
      break;
    case PropLaserReturn:
      memcpy( value, &laser_return, sizeof(laser_return) );
      retval = sizeof(laser_return);
      break;
    case PropSonarReturn:
      memcpy( value, &sonar_return, sizeof(sonar_return) );
      retval = sizeof(sonar_return);
      break;
    case PropIdarReturn:
      memcpy( value, &idar_return, sizeof(idar_return) );
      retval = sizeof(idar_return);
      break;
    case PropObstacleReturn:
      memcpy( value, &obstacle_return, sizeof(obstacle_return) );
      retval = sizeof(obstacle_return);
      break;
    case PropVisionReturn:
      memcpy( value, &vision_return, sizeof(vision_return) );
      retval = sizeof(vision_return);
      break;
    case PropPuckReturn:
      memcpy( value, &puck_return, sizeof(puck_return) );
      retval = sizeof(puck_return);
      break;
    case PropPlayerId:
      memcpy( value, &m_player, sizeof(m_player) );
      retval = sizeof(m_player);
      break;

      // these properties manipulate the player IO buffers
    case PropCommand:
      retval = GetCommand( value, m_command_len );
      break;
    case PropData:
      retval = GetData( value, m_data_len );
      break;
    case PropConfig:
    { 
      Lock();
      size_t len = m_config_len * sizeof(playerqueue_elt_t);
      memcpy( value, m_config_io, len );
      retval = len; 
      Unlock();
    }
    break;
    case PropReply:
    { 
      Lock();
      size_t len = m_reply_len * sizeof(playerqueue_elt_t);
      memcpy( value, m_reply_io, len );
      retval = len; 
      Unlock();
    }
    break;

    default:
      //printf( "Stage Warning: attempting to get unknown property %d\n", 
      //      property );
      break;
  }
  
  return retval;
}

#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CEntity::RtkStartup()
{
  // Create a figure representing this entity
  this->fig = rtk_fig_create(m_world->canvas, NULL, 50);

  // Set the color
  rtk_fig_color_rgb32(this->fig, this->color);

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
  rtk_fig_movemask(this->fig_label, 0);
  
  char label[1024];
  char tmp[1024];

  label[0] = 0;
  snprintf(tmp, sizeof(tmp), "%s", this->name);
  strncat(label, tmp, sizeof(label));
  if (m_player.port > 0)
  {
    snprintf(tmp, sizeof(tmp), "\n%d:%d", m_player.port, m_player.index);
    strncat(label, tmp, sizeof(label));
  }

  qx = qx + 0.75 * sx;
  qy = qy + 0.75 * sy;
  rtk_fig_color_rgb32(this->fig, this->color);
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
  // if we're not looking at this device, hide it 
  if( !m_world->ShowDeviceBody( this->stage_type) )
    {
      rtk_fig_show(this->fig, false);
    }
  else
    {     
      rtk_fig_show( this->fig, true );

  // We need to handle mouse dragging by the user.
  // We can only move top-level entitys.
  // Do the test here, since some entitys (eg pucks) may
  // change their parents.
  if (m_parent_entity != NULL)
    rtk_fig_movemask(this->fig, 0);
  else
    rtk_fig_movemask(this->fig, this->movemask);

  // Make sure the entity and the figure have the same pose.
  // Either update the pose of the entity in the world,
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
}

#endif



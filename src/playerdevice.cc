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
 *
 * Desc: Add player interaction to basic entity class
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: playerdevice.cc,v 1.50.2.1 2003-05-24 01:11:19 inspectorg Exp $
 */

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <sys/types.h>
#include <math.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#if HAVE_VALUES_H
  #include <values.h>  // for MAXFLOAT
#endif
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <iomanip>
#include <iostream>

//#define DEBUG
//#define VERBOSE
//#undef DEBUG
//#undef VERBOSE

#include "playerdevice.hh"
#include "world.hh"
#include "worldfile.hh"
#include "gui.hh"

#include "library.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
// Requires a pointer to the parent and a pointer to the world.
CPlayerEntity::CPlayerEntity(LibraryItem *libit, CWorld *world, CEntity *parent_entity )
  : CEntity( libit, world, parent_entity )
{
  //PRINT_DEBUG( "CEntity::CEntity()" );
  this->lock_byte = world->GetEntityCount();
  
  // init the player ID structure - subclasses will set this properly
  memset( &m_player, 0, sizeof(m_player) );
  
  // init the player IO structure - subclasses will set these properly
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

  // Set the filename used for this device's mmap
  this->device_filename[0] = 0;
}


///////////////////////////////////////////////////////////////////////////
// Destructor
CPlayerEntity::~CPlayerEntity()
{
}


void CPlayerEntity::Update( double sim_time )
{
  //PRINT_DEBUG1( "subs: %d\n", this->subscribed );

  //int subs = this->Subscribed();

  // if our subscription state has changed, generate an property change event
  //if( this->last_sub != subs )
  // this->SetProperty( 
  
  CEntity::Update( sim_time );
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CPlayerEntity::Load(CWorldFile *worldfile, int section)
{
  CEntity::Load( worldfile, section );

  // Read the player port (default 0)
  m_player.port = worldfile->ReadInt(section, "port", 0 );
  
  // if the port wasn't set, and our parent is a player device,
  // default to the parent's port
  if (m_player.port == 0 && RTTI_ISPLAYERP(m_parent_entity) )
    {
      m_player.port = dynamic_cast<CPlayerEntity*>(m_parent_entity)->m_player.port;
    }

  // Read the device index
  m_player.index = worldfile->ReadInt(section, "index", 0);
  
  // m_player.code is uniquely set in a subclass's constructor

  //PRINT_DEBUG2( "port: %d index %d", m_player.port, m_player.index );

  if( m_player.port == 0 )
    printf( "\nWarning: Player device (%s:%d:%d) has zero port. Missing 'port' property in world file?\n",
	    this->lib_entry->token, m_player.port, m_player.index );

  return true;
}

bool CPlayerEntity::Save(CWorldFile *worldfile, int section)
{
  return CEntity::Save( worldfile, section );
  // can't currently change port / index dynamically
}


///////////////////////////////////////////////////////////////////////////
// Startup routine
// A virtual function that lets entitys do some initialization after
// everything has been loaded.
bool CPlayerEntity::Startup( void )
{
  CEntity::Startup();

  // we set up a mmapped file in the tmp filesystem to share
  // data with player
  
  player_stage_info_t* playerIO = 0;
  
  size_t mem = SharedMemorySize();
   
  snprintf( this->device_filename, sizeof(this->device_filename), "%s/%d.%d.%d", 
            m_world->m_device_dir, m_player.port, m_player.code, m_player.index );
  
  PRINT_DEBUG1("creating device %s", device_filename);

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
  
  // initialize the driver name with something sensible, in case the device
  // doesn't set it
  strncpy((char*)(this->m_info_io->drivername), "stage_device", 
          sizeof(this->m_info_io->drivername));

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
  
  // store our driver name in the player IO structure.
  // use the worldfile token as the driver name to avoid possible confusion
  this->SetDriverName( (char*)this->lib_entry->token );

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Shutdown routine
void CPlayerEntity::Shutdown()
{
  CEntity::Shutdown();

  if( this->device_filename[0] )
    // remove our name in the filesystem
    if( unlink( this->device_filename ) == -1 )
      PRINT_DEBUG2( "Device failed to unlink it's IO file:%s %s", 
		    this->device_filename, strerror(errno) );
}


///////////////////////////////////////////////////////////////////////////
// Compute the total shared memory size for the device
int CPlayerEntity::SharedMemorySize( void )
{
  return( m_info_len + m_data_len + m_command_len + 
          (m_config_len * sizeof(playerqueue_elt_t)) +
          (m_reply_len * sizeof(playerqueue_elt_t)));
}


// Copy data from the shared memory segment
size_t CPlayerEntity::GetIOData( void* dest, size_t dest_len,  
                                 void* src, uint32_t* avail )
{
  size_t len;
  
  assert( dest ); // the caller must have somewhere to put this data
  if( src == 0 )  // this device doesn't have any data here
    return 0;     // so bail right away

  Lock();

  len = 0;

  // If there is data available...
  if (*avail > 0)
  {
    // If the data fits...
    if ((size_t) *avail <= dest_len)
    {
      memcpy( dest, src, *avail);
      len = *avail;
    }
    else
    {
      PRINT_ERR2("command buffer overflow; (%d > %d)", *avail, dest_len);
      len = 0;
    }
  }

  Unlock();

  return len;
}

size_t CPlayerEntity::GetData( void* data, size_t len )
{
  //PRINT_DEBUG( "f" );
  return GetIOData( data, len, m_data_io, &m_info_io->data_avail );
}

size_t CPlayerEntity::GetCommand( void* data, size_t len )
{
  //PRINT_DEBUG( "f" );
  return GetIOData( data, len, m_command_io, &m_info_io->command_avail );
}


///////////////////////////////////////////////////////////////////////////
// Copy data into the shared memory segment
size_t CPlayerEntity::PutIOData( void* dest, size_t dest_len,  
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
// Write to the driver name segment of the IO buffer
//
// this is called at the end of CPlayerEntity::Startup() with the
// worldfile token for this device.
void 
CPlayerEntity::SetDriverName( char* name )
{
  Lock();

  assert(name);

  strncpy((char*)(this->m_info_io->drivername), name, 
          sizeof(this->m_info_io->drivername));
  // just to be sure
  this->m_info_io->drivername[sizeof(this->m_info_io->drivername)-1] = '\0';

  Unlock();
}

////////////////////////////////////////////////////////////////////////////
// Copy the data into shared memory & update the info buffer
size_t CPlayerEntity::PutData( void* data, size_t len )
{
  // tell the server to export this data to anyone that needs it
  this->SetDirty( PropData, 1 );
  
  //PRINT_DEBUG4( "Putting %d bytes of data into device (%d,%d,%d)\n",
  //	len, m_player.port, m_player.type, m_player.index );
  
  // copy the data into the mmapped data buffer.
  // also set the timestamp and available fields for the data
  size_t retval = PutIOData( (void*)m_data_io, m_data_len,
			     data, len,
			     &m_info_io->data_timestamp_sec,
			     &m_info_io->data_timestamp_usec,
			     &m_info_io->data_avail );
  
  // give the GUI a chance to display this data
  GuiEntityPropertyChange( this, PropData );

  return retval;
}

////////////////////////////////////////////////////////////////////////////
// Copy the command into shared memory & update the info buffer
size_t CPlayerEntity::PutCommand( void* data, size_t len )
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
size_t CPlayerEntity::GetConfig(void** client, void* config, size_t len )
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

size_t CPlayerEntity::PutReply(void* client, unsigned short type,
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
size_t CPlayerEntity::PutReply(void* client, unsigned short type)
{
  return(PutReply(client, type, NULL, NULL, 0));
} 


///////////////////////////////////////////////////////////////////////////
// See if the PlayerDevice is subscribed
int CPlayerEntity::Subscribed() 
{
  int subscribed = 0; 
  
  if( m_info_io ) // if we have player connection at all
    { 
      Lock();
      // see if a player client is connected to this entity
      subscribed = m_info_io->subscribed;
      Unlock();
    }
  
  //printf( "SUBS: %d\n", subscribed );


  return( subscribed );
}

void CPlayerEntity::FamilySubscribe()
{
  CHILDLOOP( ch )
    ch->FamilySubscribe();

  this->Subscribe();
}

void CPlayerEntity::FamilyUnsubscribe()
{
  CHILDLOOP( ch )
    ch->FamilyUnsubscribe();

  this->Unsubscribe();
}

///////////////////////////////////////////////////////////////////////////
//// subscribe to the device (used by other devices that depend on this one)
void CPlayerEntity::Subscribe() 
{
  Lock();
  m_info_io->subscribed++;  
  Unlock(); 
  
  //printf( "player %d.%d.%d subs %d\n", 
  //  this->m_player.port,  
  //  this->m_player.code,  
  //  this->m_player.index,
  //  m_info_io->subscribed );

  CEntity::Subscribe();
}

///////////////////////////////////////////////////////////////////////////
// unsubscribe from the device (used by other devices that depend on this one)
void CPlayerEntity::Unsubscribe()
{ 
  //puts( "PUNSUB" );
  
  Lock();
  if( m_info_io->subscribed > 0 )
    m_info_io->subscribed--;
  Unlock();

  CEntity::Unsubscribe();
} 


///////////////////////////////////////////////////////////////////////////
// lock the shared mem
// Returns true on success
//
bool CPlayerEntity::Lock( void )
{
  assert(m_world);
  return m_world->LockByte( this->lock_byte );
}

///////////////////////////////////////////////////////////////////////////
// unlock the shared mem
//
bool CPlayerEntity::Unlock( void )
{
  assert(m_world);
  return m_world->UnlockByte( this->lock_byte );
}

int CPlayerEntity::SetProperty( int con, EntityProperty property, 
			  void* value, size_t len )
{
  PRINT_DEBUG( "" );

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
      
    default: // assume it'll be handled by CEntity::SetProperty
      break; 
  }

  // even if we handled it, we still call the basic SetProperty in
  // order to set the dirty flags correctly
  return( CEntity::SetProperty( con, property, value, len ) ); 
}


int CPlayerEntity::GetProperty( EntityProperty property, void* value )
{ 
  PRINT_DEBUG3( "finding property %d for entity (%d %s)", 
                property, this->stage_id, this->lib_entry->token );

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
      // we didn't have that proprty code - pass request down to the CEntity and return
      return( CEntity::GetProperty( property, value ) );
  }

  return retval;
}

void CPlayerEntity::GetStatusString( char* buf, int buflen )
{
  double x, y, th;
  this->GetGlobalPose( x, y, th );
  
  // check for overflow
  assert( -1 !=
	  snprintf( buf, buflen, 
		    "Pose(%.2f,%.2f,%.2f) Player(%d:%d:%d) Stage(%d:%d(%s))",
		    x, 
		    y, 
		    th,
		    this->m_player.port, 
		    this->m_player.code, 
		    this->m_player.index, 
		    this->stage_id,
		    this->lib_entry->type_num,
		    this->lib_entry->token
		    ) );
 
}

#ifdef INCLUDE_RTK2

// Initialise the rtk gui
void CPlayerEntity::RtkStartup()
{
  CEntity::RtkStartup();

  // add this device to the world's data menu 
  this->m_world->AddToDataMenu( this, true); 

  // add the player ID string to the label figure
  char label[1024];
  char tmp[1024];
   
  label[0] = 0;
  snprintf(tmp, sizeof(tmp), "%s %s", 
	   this->name,
	   this->lib_entry->token );
  strncat(label, tmp, sizeof(label));
  if (m_player.port > 0)
  {
    snprintf(tmp, sizeof(tmp), "\n%d:%d", m_player.port, m_player.index);
    strncat(label, tmp, sizeof(label));
  }
   
  rtk_fig_clear(this->fig_label );
  rtk_fig_text(this->fig_label,  0.75 * size_x,  0.75 * size_y, 0, label);

  return;
}


///////////////////////////////////////////////////////////////////////////
// Process mouse events
void CPlayerEntity::RtkOnMouse(rtk_fig_t *fig, int event, int mode)
{
  switch (event)
  {
    case RTK_EVENT_PRESS:
    
      // if the CTRL key is held down, launch playerv
      // rtk doesn't support modifiers directly so we use this
      // backdoor route which may not sync perfectly with the click
      // itself over a delayed network, but it should work pretty well.
      int x, y;
      GdkModifierType modifiers;
      gdk_window_get_pointer(m_world->canvas->canvas->window, 
                             &x, &y, &modifiers);
    
      //if( modifiers & GDK_CONTROL_MASK ) SpawnPlayerv();
      break;

      // Handle case when mouse is moved over the figure
    case RTK_EVENT_MOUSE_OVER:
      this->FamilySubscribe();
      break;
      
      // Handle case when mouse is moved away from the figure
    case RTK_EVENT_MOUSE_NOT_OVER:
      this->FamilyUnsubscribe();
      break;
      
    default:
      break;
  }
  
  CEntity::RtkOnMouse(fig, event, mode);
  return;
}

#endif


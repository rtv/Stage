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
 * Desc: Base class for movable entities.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 04 Dec 2000
 * CVS info: $Id: playerdevice.hh,v 1.8 2002-10-10 02:45:25 gerkey Exp $
 */

#ifndef PLAYERENTITY_HH
#define PLAYERENTITY_HH

#include "entity.hh"

#if HAVE_STDINT_H
#include <stdint.h>
#endif

// (a little hacky - might engineer out of this one day - rtv)
class CPlayerEntity;
#include <typeinfo>

#define RTTI_ISTYPE(className, pObj) \
		(NULL != dynamic_cast<className> (pObj))

// macro is true if object X is a CPlayerEntity 
#define RTTI_ISPLAYER(obj) RTTI_ISTYPE( CPlayerEntity, obj)
// macro is true if object X is a CPlayerEntity*
#define RTTI_ISPLAYERP(pobj) RTTI_ISTYPE( CPlayerEntity*, pobj)


class CPlayerEntity : public CEntity
{
  // Minimal constructor
  // Requires a pointer to the parent and a pointer to the world.
  public: CPlayerEntity(CWorld *world, CEntity *parent_entity );

  // Destructor
  public: virtual ~CPlayerEntity();

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);
  
  // Save the entity to the worldfile
  public: virtual bool Save(CWorldFile *worldfile, int section);
  
  // Initialise entity
  public: virtual bool Startup( void ); 
  
  // Finalize entity
  public: virtual void Shutdown();

  // Get/set properties
  public: virtual int SetProperty( int con,
                                   EntityProperty property, void* value, size_t len );
  public: virtual int GetProperty( EntityProperty property, void* value );
  
public: virtual void Update( double sim_time );

  // writes a description of this device into the buffer
public: virtual void GetStatusString( char* buf, int buflen );

  ///////////////////////////////////////////////////////////////////////
  // RTP stuff
  CRTPPlayer* rtp_p;

  // Get the shared memory size
  private: int SharedMemorySize( void );

  //////////////////////////////////////////////////////////////////////
  // PLAYER IO STUFF
  
  // Port and index numbers for player
  // identify this device as belonging to the Player on port N at index M
  public: player_device_id_t m_player;

  // basic external IO functions

  // copies data from src to dest (if there is room), updates the
  // timestamp with the current time, sets the availability value to
  // the number of bytes copied
  size_t PutIOData( void* dest, size_t dest_len,  
                    void* src, size_t src_len,
                    uint32_t* ts_sec, uint32_t* ts_usec, uint32_t* avail );
  
  // copies data from src to dest if there is room and if there is the
  // right amount of data available
  size_t GetIOData( void* dest, size_t dest_len,  
                    void* src, uint32_t* avail );
    
  // specific external IO functions

  // Write to the driver name segment of the IO buffer
  //
  // every player device should invoke this method early, like in the
  // constructor.
  protected: void SetDriverName( char* name );
  
  // Write to the data buffer
  // Returns the number of bytes copied
  // timestamp should be the time the data was created/sensed. if timestamp
  //   is 0, then current time is used
  protected: size_t PutData( void* data, size_t len );

  // Read from the data buffer
  // Returns the number of bytes copied
  public: size_t GetData( void* data, size_t len );
   
  // gets a command from the mmapped command buffer
  // returns the number of bytes copied (on success, this is == len)
  protected: size_t GetCommand( void* command, size_t len);

  // returns number of bytes copied
  protected: size_t PutCommand( void* command, size_t len);

  // Read from the configuration buffer
  // Returns the number of bytes copied
  protected: size_t GetConfig(void** client, void* config, size_t len);

  // Push a configuration reply onto the outgoing queue.
  // Returns 0 on success, non-zero on error.
  protected: size_t PutReply(void* client, unsigned short type,
                             struct timeval* ts, void* reply, size_t len);

  // A short form for zero-length replies.
  // Returns 0 on success, non-zero on error.
  protected: size_t PutReply(void* client, unsigned short type);


  // See if the device is subscribed
  // returns the number of current subscriptions
  //private int player_subs;
   public: virtual int Subscribed();

  // subscribe to / unsubscribe from the device
  // these are used when one device (e.g., lbd) depends on another (e.g.,
  // laser)
  public: virtual void Subscribe();
  public: virtual void Unsubscribe();
  
  // these versions sub/unsub to this device and all its decendants
public: virtual void FamilySubscribe();
public: virtual void FamilyUnsubscribe();

  // packages and sends data via rtp
  protected: void AnnounceDataViaRTP( void* data, size_t len );

  // Pointers into shared mmap for the IO structures
  // the io buffer is allocated by the World 
  // after it has loaded all the Entities (so it knows how 
  // much to allocate). Then the world calls Startup() to allocate 
  // the local storage for each entity. 

  protected: player_stage_info_t *m_info_io;
  protected: uint8_t *m_data_io; 
  protected: uint8_t *m_command_io;
  protected: uint8_t *m_config_io;
  protected: uint8_t *m_reply_io;

  // the sizes of these buffers in bytes
  protected: size_t m_data_len;
  protected: size_t m_command_len;
  protected: size_t m_config_len;
  protected: size_t m_reply_len;
  protected: size_t m_info_len;

  // we'll use these entitys to access the configuration request/reply queues
  protected: PlayerQueue* m_reqqueue;
  protected: PlayerQueue* m_repqueue;

  // set and unset the semaphore that protects this entity's shared memory 
  protected: bool Lock( void );
  protected: bool Unlock( void );
  
  // pointer to the  semaphore in the shared memory
  //private: sem_t* m_lock; 

  // IO access to this device is controlled by an advisory lock 
  // on this byte in the shared lock file
  private: int lock_byte; 

  private: char device_filename[256];

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual void RtkStartup();
#endif

};

#endif




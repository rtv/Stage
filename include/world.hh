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
 * Desc: top level class that contains everything
 * Author: Richard Vaughan, Andrew Howard
 * Date: 7 Dec 2000
 * CVS info: $Id: world.hh,v 1.65 2002-08-21 21:54:48 gerkey Exp $
 */

#ifndef WORLD_HH
#define WORLD_HH

#include <stddef.h>
#include <netinet/in.h>
//#include <pthread.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/poll.h>
#include <limits.h>

#include "player.h" //from player
#include "image.hh"
#include "entity.hh"
#include "playercommon.h"
#include "matrix.hh"
#include "worldfile.hh"

#include "rtp.h"

#if INCLUDE_RTK2
#include "rtk.h"
#define MAX_DEVICE_MENU_ITEMS 32
#endif

// World class
class CWorld
{
  public: 
  
  // Default constructor
  CWorld( int argc, char** argv );
  
  // Destructor
  virtual ~CWorld();
  
public: CRTPPlayer* rtp_player;
  
  // the main world-model data structure
  public: CMatrix *matrix;
  
  // the locks file descriptor
  public: int m_locks_fd;

  public: char m_device_dir[PATH_MAX]; //device  directory name 
  
  // this should really be in the CStageServer, but the update is tricky there
  // so i'll leave it here for now - RTV
  // export the time in this buffer
  protected: stage_clock_t* m_clock; // a timeval and lock

  //private: CStageIO* m_io; // a client or server object

  // Properties of the underlying matrix representation
  // for the world.
  // The resolution is specified in pixels-per-meter.
  public: double ppm;

  // Entity representing the fixed environment
  protected: CEntity *wall;
  
  public:
  // timing
  //bool m_realtime_mode;
  double m_real_timestep; // the time between wake-up signals in msec
  double m_sim_timestep; // the sim time increment in seconds
  // change ratio of these to run stage faster or slower than real time.

  uint32_t m_step_num; // the number of cycles executed, from 0

  // when to shutdown (in seconds)
  private: int m_stoptime;
  
  // Enable flag -- world only updates while this is set
  protected: bool m_enable;
  
  // the hostname of this computer
  public: char m_hostname[ HOSTNAME_SIZE ];
  public: char m_hostname_short[ HOSTNAME_SIZE ];  // same thing, w/out domain

  // the IP address of this computer
  public: struct in_addr m_hostaddr;

  double Pause();
  void Output();
  void LogOutputHeader( void );

  void LogOutput( double freq,
                  //double sleep_duration,
                  //double avg_sleep_duration,
                  unsigned int bytes_in, unsigned int bytes_out,
                  unsigned int total_bytes_in, unsigned int total_bytes_out );

  void ConsoleOutput( double freq,
                      //double ratio, double avg_ratio,
                      unsigned int bytes_in, unsigned int bytes_out,
                      double avg_data );
    
  void StartTimer(double interval);

  //-----------------------------------------------------------------------
  // Timing
  // Real time at which simulation started.
  // The time according to the simulator (m_sim_time <= m_start_time).
  private: double m_start_time, m_last_time;
  private: double m_sim_time;
  // the same as m_sim_time but in timeval format
  public: struct timeval m_sim_timeval; 
  
  public: bool m_log_output, m_console_output;

  int m_log_fd; // logging file descriptor
  char m_log_filename[PATH_MAX]; // path to the log file
  char m_cmdline[512]; // a string copy of the command line that started stage

  //private: double m_max_timestep;
  
  // Update rate (just for diagnostics)
  private: double m_update_ratio;
  private: double m_update_rate;
  
  // color definitions
  private: char m_color_database_filename[PATH_MAX];

  // Entity list
  private: int m_entity_count;
  private: int m_entity_alloc;
  private: CEntity **m_entity;
  
  public: CEntity** Entities( void ){ return m_entity; };

  protected: int CountDirtyOnConnection( int con );

  public: void DirtyEntities( void )
    {
      for( int i=0; i<m_entity_count; i++ )
        m_entity[i]->SetDirty( 1 );
    }

  public: void CleanEntities( void )
    {
      for( int i=0; i<m_entity_count; i++ )
        m_entity[i]->SetDirty( 0 );
    }

  // dirty all entities for a particulat connection
  public: void DirtyEntities( int con )
    {
      for( int i=0; i<m_entity_count; i++ )
        m_entity[i]->SetDirty( con, 1 );
    }

  // clan all entities for a particulat connection
  public: void CleanEntities( int con )
    {
      for( int i=0; i<m_entity_count; i++ )
        m_entity[i]->SetDirty( con, 0 );
    }

  // Authentication key
  public: char m_auth_key[PLAYER_KEYLEN];

  // if the user is running more than one copy of stage, this number
  // identifies this instance uniquely
  int m_instance;


  ///////////////////////////////////////////////////////////////////////////
  // Configuration variables

  // REMOVE?
  // flags that control servers
  //public: bool m_env_server_ready;
  //private: bool m_run_environment_server;
  //private: bool m_run_pose_server;

  // Non-zero if we should run the gui (a command line setting)
  private: bool enable_gui;

  protected: bool m_external_sync_required;
  public: bool m_send_idar_packets;
  
  // the pose server port
  //public: int m_server_port;

  public: bool ParseCmdLine( int argv, char** argv );

  // Save the world file.  This is pure virtual since the actual
  // saving is implemented in the CServer or CClient subclasses.
  protected: virtual bool SaveFile( char* filename ) = 0;

  
  //////////////////////////////////////////////////////////////////////
  // main methods
  
  // Initialise the world
  public: virtual bool Startup();
  
  // Shutdown the world
  public: virtual void Shutdown();
  
  // Update everything
  public: virtual void Update();

  // Add an entity to the world
  public: void AddEntity(CEntity *entity);


  //////////////////////////////////////////////////////////////////////////
  // Time functions
    
  // Get the simulation time - Returns time in sec since simulation started
  public: double GetTime();

  // Get the real time - Returns time in sec since simulation started
  public: double GetRealTime();

  ///////////////////////////////////////////////////////////////////////////
  // matrix-based world functions
  
  // Initialise the matrix - loading the bitmap world description
  private: bool InitGrids(const char *env_file);

  // Update a rectangle in the matrix
  // Set add = true to add the entity, set add = false to remove it.
  public: void SetRectangle(double px, double py, double pth,
                            double dx, double dy, CEntity* ent, bool add );

  // Update a circle in the matrix
  // Set add = true to add the entity, set add = false to remove it.
  public: void SetCircle(double px, double py, double pr,
                         CEntity* ent, bool add );
  
  

  ////////////////////////////////////////////////////////////////////////
  // utility methods

  // return a string that names this type of entity
  public: char* CWorld::StringFromType( StageType t );
  public: StageType TypeFromString( const char* ent_string );

  public: CEntity* GetEntityByID( int port, int type, int index );
  
  // entities can be created by type number or by string description
  public: CEntity* CreateEntity(const char *type_str, CEntity *parent );
  public: CEntity* CreateEntity( StageType type, CEntity *parent );
  
  /////////////////////////////////////////////////////////////////////////////
  // access methods
  
  public:
  double GetWidth( void )
    { if( matrix ) return matrix->width; else return 0; };
  
  double GetHeight( void )
    { if( matrix ) return matrix->height; else return 0; };
  
  int GetEntityCount( void )
    { return m_entity_count; };
  
  CEntity* GetEntity( int i )
    { if( i>=0 && i<m_entity_count ) return (m_entity[i]); else return 0; }; 

  // get the array index of the entity (-1 if no match)
  int GetEntityIndex( CEntity* ent )
  { 
    if( ent )
      for( int e=0; e<m_entity_count; e++ ) 
	if( m_entity[e] == ent ) return e; 
    
    return -1;
  };
    
  // returns true if the given hostname matches our hostname, false otherwise
  //bool CheckHostname(char* host);


  // RTK STUFF ----------------------------------------------------------------
#ifdef INCLUDE_RTK2
  // Initialise the GUI
  protected: bool RtkLoad(CWorldFile *worldfile);
  
  // Save the GUI
  protected: bool RtkSave(CWorldFile *worldfile);
  
  // Start the GUI
  protected: bool RtkStartup();

  // Stop the GUI
  protected: void RtkShutdown();

  // Update the GUI
  protected: void RtkUpdate();

protected: void RtkMenuHandling();
  
  //protected: bool device_view_status[ NUMBER_OF_STAGE_TYPES ];
  //protected: bool data_view_status[ NUMBER_OF_STAGE_TYPES ];

  // Basic GUI elements
  public: rtk_app_t *app;
  public: rtk_canvas_t *canvas;

  // Figure for the grid
  private: rtk_fig_t *fig_grid;
  
  // The file menu
  private: rtk_menu_t *file_menu;
  private: rtk_menuitem_t *save_menuitem;
  private: rtk_menuitem_t *export_menuitem;
  private: rtk_menuitem_t *exit_menuitem;

  // The view menu
  public: rtk_menu_t *view_menu;
  private: rtk_menuitem_t *grid_item;
  private: rtk_menuitem_t *walls_item;

  // The action menu
public: rtk_menu_t* action_menu;
private: rtk_menuitem_t* subscribedonly_item;
private: rtk_menuitem_t* autosubscribe_item;
public: static int autosubscribe;


  // The view/device menu
public: rtk_menu_t *device_menu;
private: rtk_menuitem_t *device_menu_items[ NUMBER_OF_STAGE_TYPES ];

  // the view/data menu
public: rtk_menu_t *data_menu;
private: rtk_menuitem_t *data_menu_items[ NUMBER_OF_STAGE_TYPES ];
  
  // devices check this to see if they should display their data
public: bool ShowDeviceData( StageType devtype )
  { 
    rtk_menuitem_t* menu_item = data_menu_items[ devtype ];

    if( menu_item )
      return( rtk_menuitem_ischecked( data_menu_items[ devtype ]) );  
    else // if there's no option in the menu, display this data
      return true;
  }

public: bool ShowDeviceBody( StageType devtype )
  {
    rtk_menuitem_t* menu_item = device_menu_items[ devtype ];
    
    if( menu_item )
      return( rtk_menuitem_ischecked( device_menu_items[ devtype ]) );  
    else // if there's no option in the menu, display this data
      return true;    
}

  // Number of exported images
  private: int export_count;
#endif
  
};

#endif


















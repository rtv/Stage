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
 * CVS info: $Id: world.hh,v 1.20 2002-11-11 01:29:23 inspectorg Exp $
 */

#ifndef WORLD_HH
#define WORLD_HH

// NOTE: include <sys/types.h> FIRST, as it may have an effect on other headers
#include <sys/types.h> 
#include <stddef.h>
#include <netinet/in.h>
//#include <pthread.h>
//#include <sys/sem.h>
//#include <sys/ipc.h>
#include <limits.h>

#include "replace.h" // for poll(2)
#include "player.h" //from player
#include "entity.hh"
#include "playercommon.h"
#include "matrix.hh"
#include "worldfile.hh"

class Library;

#if INCLUDE_RTK2
#include "rtk.h"
#endif

// World class
class CWorld
{
  public: 
  
  // constructor
  CWorld( int argc, char** argv, Library* lib );
  
  // Destructor
  virtual ~CWorld();
  
  // the Library class knows how to create entities given a worldfile
  // token, and can find the token given a StageType number
  public: Library* lib;
  public: inline Library* GetLibrary( void ){ return lib; }

  // we store the app command line
  int argc;
  char** argv;

  // the main world-model data structure
  public: CMatrix *matrix;
  
  // these do nothing in the base CWorld class, but are overridden in
  // CServer to control access to shared memory
  virtual bool LockByte( int offset );
  virtual bool UnlockByte( int offset );

  // TODO - this should move to the server
  public: char m_device_dir[PATH_MAX]; //device  directory name 
  
  // Properties of the underlying matrix representation
  // for the world.
  // The resolution is specified in pixels-per-meter.
  public: double ppm;


  // Object encapsulating world description file (null if there is no file)
  public: CWorldFile* worldfile;

  // access func

  // timing
  //bool m_realtime_mode;
  public: double m_real_timestep; // the time between wake-up signals in msec
   public: double m_sim_timestep; // the sim time increment in seconds
  // change ratio of these to run stage faster or slower than real time.

public: uint32_t m_step_num; // the number of cycles executed, from 0
  
  // set the current time from the interval and step number
  // returning the current time in seconds
  virtual double SetClock( double interval, uint32_t step_num );
  

  // Enable flag -- world only updates while this is set
  protected: bool m_enable;
  
  // the hostname of this computer
  public: char m_hostname[ HOSTNAME_SIZE ];
  public: char m_hostname_short[ HOSTNAME_SIZE ];  // same thing, w/out domain

  // the IP address of this computer
  public: struct in_addr m_hostaddr;

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
    

  void Ticker( void )
  {
    putchar( ' ' );
    static int z = 0;
    char c = 0;
    
    switch( z % 4 )
      {
      case 0: c = '|'; break;
      case 1: c = '/'; break;
      case 2: c = '-'; break;
      case 3: c = '\\'; break;
      }
    
    putchar( c );
    putchar( '\b' );
    fflush( stdout );
    z++;
  };
  
  //-----------------------------------------------------------------------
  // Timing
  // Real time at which simulation started.
  // The time according to the simulator (m_sim_time <= m_start_time).
  protected: double m_start_time, m_last_time;
  protected: double m_sim_time;
  // the same as m_sim_time but in timeval format
  public: struct timeval m_sim_timeval; 
  
  int m_log_fd; // logging file descriptor
  char m_log_filename[PATH_MAX]; // path to the log file
  char m_cmdline[512]; // a string copy of the command line that started stage

  // Update rate (just for diagnostics)
  private: double m_update_ratio;
  private: double m_update_rate;
  
  // color definitions
  private: char m_color_database_filename[PATH_MAX];

  
  // this is the root of the entity tree - all entities are children
  // of root
  public: static CEntity* root;
  public: CEntity* GetRoot( void ){ return root; };

  // pointers to all entities are also stored in a flat array
  // be found by number without having to search the tree
  protected: CEntity** entities;
  // the number of entities stored in the array
  protected: int entity_count; 
  
  // the capacity of the currently allocated array
  private: int entities_size;

  // return a pointer to an entity with matching id
  public: CEntity* GetEntity( int i );
  
  public: int GetEntityCount( void ){ return this->entity_count; };
  
  // adds the pointer to the flat child_list and increments
  // entity_count, returning the entity's position in the list to use
  // as a unique id
   public: stage_id_t RegisterEntity( CEntity *entity);
  
  // Authentication key
  public: char m_auth_key[PLAYER_KEYLEN];

  // if the user is running more than one copy of stage, this number
  // identifies this instance uniquely (TODO - broken) 
  int m_instance;

  ///////////////////////////////////////////////////////////////////////////
  // Configuration variables

  // if true we  run the gui - a command line switch
  public: bool enable_gui;
  // if true  we log output to to file - a command line switch
  public: bool m_log_output;
  // if true we log output ot the console - a command line switch
  public: bool m_console_output;

  public: bool ParseCmdLine( int argv, char** argv );

  // Save the world file (or ask the server to save it)
  public: virtual bool Save( void );

  // Load the world file (or download it from a server)
  public: virtual bool Load( void );

  //////////////////////////////////////////////////////////////////////
  // main methods
  
  // Initialise the world
  public: virtual bool Startup();
  
  // Shutdown the world
  public: virtual bool Shutdown();
  
  // Update everything
  public: virtual void Update();

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

  // entities can be created by type number or by string description
  public: CEntity* CreateEntity(const char *type_str, CEntity *parent );
  //public: CEntity* CreateEntity( int StageType type, CEntity *parent );
  

  /////////////////////////////////////////////////////////////////////////////
  // access methods
  
  public:
  double GetWidth( void )
    { if( matrix ) return matrix->width; else return 0; };
  
  double GetHeight( void )
    { if( matrix ) return matrix->height; else return 0; };
  
  // returns true if the given hostname matches our hostname, false otherwise
  //bool CheckHostname(char* host);

  // return the entity nearest the specified point, but not more than range m away,
  // that has the specified parent 
  CEntity* GetNearestChildWithinRange( double x, double y, double range, 
				       CEntity* parent );
  
  // return the entity nearest the specified point, but not more than range m away
  CEntity* GetNearestEntityWithinRange( double x, double y, double range );

  
  // RTK STUFF ----------------------------------------------------------------
#ifdef INCLUDE_RTK2
  // Initialise the GUI
  public: bool RtkLoad(CWorldFile *worldfile);
  
  // Save the GUI
  public: bool RtkSave(CWorldFile *worldfile);
  
  // Start the GUI
  public: bool RtkStartup();

  // Stop the GUI
  public: void RtkShutdown();

  // Update the GUI
  public: void RtkUpdate();

  // Handle menu stuff
  protected: void RtkMenuHandling();

  // Handle the movie sub-menu.
  private: void RtkUpdateMovieMenu();
  
  // Basic GUI elements
  public: rtk_app_t *app;
  public: rtk_canvas_t *canvas;

  // Timing info for the gui.
  // [rtk_update_rate] is the update rate (Hz).
  // [rtk_update_time] is the last time the gui was update (simulation time).
  private: double rtk_update_rate;
  private: double rtk_update_time;

  // Figure for the grid
  private: rtk_fig_t *fig_grid;
  
  // The file menu
  private: rtk_menu_t *file_menu;
  private: rtk_menuitem_t *save_menuitem;
  private: rtk_menuitem_t *exit_menuitem;

  // The stills menu
  private: rtk_menu_t *stills_menu;
  private: rtk_menuitem_t *stills_jpeg_menuitem;
  private: rtk_menuitem_t *stills_ppm_menuitem;

  // Export stills info
  private: int stills_series;
  private: int stills_count;

  // The movie menu  
  private: rtk_menu_t *movie_menu;
  private: struct CMovieOption
  {
    rtk_menuitem_t *menuitem;
    int speed;
  };
  private: int movie_option_count;
  private: CMovieOption movie_options[10];

  // Export movie info
  private: int movie_count;

  // The view menu
  public: rtk_menu_t *view_menu;
  private: rtk_menuitem_t *grid_item;
  private: rtk_menuitem_t *walls_item;
  private: rtk_menuitem_t *matrix_item;

  // The action menu
  public: rtk_menu_t* action_menu;
  private: rtk_menuitem_t* subscribedonly_item;
  private: rtk_menuitem_t* autosubscribe_item;
  public: static int autosubscribe;


  typedef struct 
  {
    rtk_menu_t *menu;
    rtk_menuitem_t *items[ 1000 ]; // TODO - get rid of this fixed size buffer
  } stage_menu_t;
  
  // The view/device menu
  protected: stage_menu_t device_menu;
  
  // the view/data menu
  public: stage_menu_t data_menu;
  
  protected: void AddToMenu( stage_menu_t* menu, CEntity* ent, int check );
  public:  void AddToDataMenu( CEntity* ent, int check );
  public: void AddToDeviceMenu( CEntity* ent, int check );
  
  // devices check this to see if they should display their data
  public: bool ShowDeviceData( int devtype );
  public: bool ShowDeviceBody( int devtype );

  // Number of exported images
  private: int export_count;
#endif

  //public: void GuiStartup( void );

};

#endif


















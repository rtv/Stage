/*************************************************************************
 * world.h - most of the header action is here 
 * RTV
 * $Id: world.hh,v 1.1.2.4 2000-12-07 00:30:00 ahoward Exp $
 ************************************************************************/

#ifndef WORLD_HH
#define WORLD_HH

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iomanip.h>
#include <termios.h>
#include <strstream.h>
#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream.h>

#include "playerrobot.hh"
#include "image.h"

#ifndef INCLUDE_RTK
#include "win.h"
#else INCLUDE_RTK
#include "rtk-ui.hh"
#endif

// forward declaration
class CWorldWin;
class CObject;


// Enumerated type describing the layers in the world rep
//
enum EWorldLayer
{
    layer_obstacle = 0,
    layer_laser = 1,
    layer_vision = 2
};


// various c function declarations - an untidy mix of c and c++ - yuk!
int InitNetworking( void );


class CWorld : public CObject
{
    public:
  int width, height, depth; 
  int paused;

  unsigned char channels[256];
  char posFile[64];
  char bgFile[64];

  CPlayerRobot* bots;

    /* *** REMOVE ahoward
       float pioneerWidth, pioneerLength;
       float localizationNoise;
       float sonarNoise;

       float maxAngularError; // percent error on turning odometry
    */

    float ppm;

  int* hits;
  int population;

  double timeStep, timeNow, timeThen, timeBegan;


  // methods
  int LoadVars( char* initFile);
  void SavePos( void );
  //void LoadPos( void );

  void DumpSonar( void );
  void DumpOdometry( void );
  void GetUpdate( void );  

  CPlayerRobot* NearestRobot( float x, float y );

    ///////////////////////////////////////////////////////
    // Added new stuff here -- ahoward

    // Default constructor
    //
    public: CWorld();

    // Destructor
    //
    public: virtual ~CWorld();
    
    // Startup routine -- creates objects in the world
    //
    public: virtual bool Startup(RtkCfgFile *cfg);

    // Shutdown routine -- deletes objects in the world
    //
    public: virtual void Shutdown();

    // Update everything
    //
    public: virtual void Update();
    
    // Initialise the world grids
    //
    private: bool StartupGrids();
  
    // Get a cell from the world grid
    //
    public: BYTE GetCell(double px, double py, EWorldLayer layer);

    // Set a cell in the world grid
    //
    public: void SetCell(double px, double py, EWorldLayer layer, BYTE value);

    // Set a rectangle in the world grid
    //
    public: void SetRectangle(double px, double py, double pth,
                              double dx, double dy, EWorldLayer layer, BYTE value);

    // *** HACK -- make private
    // Obstacle data
    //
    public: Nimage* bimg; // background image 
    public: Nimage* img; //foreground img;

    // Laser image
    // Store laser data (obstacles and beacons
    //
    private: Nimage *m_laser_img;

    // Vision image
    // Stores vision data
    //
    private: Nimage *m_vision_img;

#ifndef INCLUDE_RTK

  public:
    void Draw( void );
    CWorldWin* win; 
    int refreshBackground;
    
#else

    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

    // Draw the background; i.e. things that dont move
    //
    private: void DrawBackground(RtkUiDrawData *pData);

    // Draw the laser layer
    //
    private: void DrawLayer(RtkUiDrawData *pData, EWorldLayer layer);
  
#endif
};

#endif





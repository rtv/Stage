/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: win.h,v 1.1.1.1 2000-11-29 00:16:53 ahoward Exp $
 ************************************************************************/

#ifndef WIN_H
#define WIN_H

#include <X11/Xlib.h>

#include "world.h"

class CWorldWin
{
public:
  // constructor
  CWorldWin( CWorld* wworld, char* initFile );
  
  Window win;
  GC gc, bgc, wgc;
  
  Display* display;
  int screen;

  unsigned long red, green, blue, yellow, magenta, cyan, white, black; 
  int grey;


  int width, height, x,y;
  int iwidth, iheight, panx, pany;
  int drawMode;
   
  int showSensors;

  XEvent reportEvent;

  double xscale, yscale;
  double dimx, dimy;

  unsigned int RGB( int r, int g, int b )
    {
      return (((r << 8) | g) << 8) | b;
    };

 // data
  CWorld* world;
  CRobot* dragging;

  // methods  

  //void CheckSubscriptions( CRobot* r );
  void BoundsCheck( void );
  int LoadVars( char* initFile );
  void HandleEvent( void );
  void Update( void );
  void DragRobot( void );
  void DragBox( void );
  void PrintCoords( void );
  void DrawSonar( CRobot* r );
  void DrawLaser( CRobot* r );
  void DrawCrumRadius( CRobot* r );
  void Draw( void );
  void DrawZones( void );
  //void DrawCrum( CCrum* aCrum );
  //void DrawCrums( void );
  void DrawRobotIfMoved( CRobot* r );
  void DrawRobot( CRobot* r );
  void DrawRobotWithScaling( CRobot* r );
  void DrawRobotsInRect( int xx, int yy, int ww, int hh );
  void DrawRobots( void );
  void DrawWalls( void );
  void BlackBackground( void );
  void BlackBackgroundInRect(int xx, int yy, int ww, int hh  );
  void DrawWallsInRect( int xx, int yy, int ww, int hh );
  void ScanBackground( void );
  void ScanBackgroundWithScaling( void );
  void MoveSize(void);
  unsigned long  DrawInRobotColor( CRobot* r );
};

#endif

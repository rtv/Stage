/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: win.h,v 1.3 2000-12-04 02:11:31 vaughan Exp $
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
  
  void PrintCoords( void );
  
  void Draw( void );
  void DrawRobotIfMoved( CRobot* r );
  void DrawRobot( CRobot* r );

  void DrawRobotsInRect( int xx, int yy, int ww, int hh );
  void DrawRobots( void );
  void DrawWalls( void );
  void BlackBackground( void );
  void BlackBackgroundInRect(int xx, int yy, int ww, int hh  );
  void DrawWallsInRect( int xx, int yy, int ww, int hh );
  void ScanBackground( void );

  void MoveSize(void);
  void DrawInRobotColor( CRobot* r );
  unsigned long  RobotDrawColor( CRobot* r );
  void SetForeground( unsigned long color );
  void DrawLines( XPoint* pts, int numPts );
  void DrawPoints( XPoint* pts, int numPts );
  //void DrawPolygon( XPoint* pts, int numPts );

  void SetDrawMode( int mode );
};

#endif

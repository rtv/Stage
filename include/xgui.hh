/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: xgui.hh,v 1.1.2.1 2001-05-21 23:25:47 vaughan Exp $
 ************************************************************************/

#ifndef WIN_H
#define WIN_H

#include <X11/Xlib.h>

//#include "world.hh"

// forward definition
class CWorld;
class CObject;

// for now i have a MAX_OBJECTS maximum to the number of objects
// that can be processed by the GUI. should make this dynamic one day
static int MAX_OBJECTS = 1000;

typedef struct  
{
  double x, y;
} DPoint;

typedef struct
{
  double toplx, toply, toprx, topry, botlx, botly, botrx, botry;
} DRect;

enum ObjectType 
{ 
  generic_o, 
  laserturret_o, 
  ptz_o, 
  pioneer_o, 
  uscpioneer_o,		  
  box_o, 
  laserbeacon_o 
};

// structure to hold an object's exportable data
// i.e. sufficient to render it in an external GUI
// each CObject (CThing) has one of these when XGui is enabled.
class ExportData
{
public:
  int objectType;
  //int objectId;
  CObject* objectId;
  double x, y, th;
  double width, height;
  int dataSize;
  unsigned char* data;
};

// for now the import type can be the same as the export type.
// might need to change this later, so i'll declare a type for it.
typedef  ExportData ImportData;

class CXGui
{
public:
  // constructor
  CXGui( CWorld* wworld );
  //CXGui( CWorld* wworld, char* initFile );
  
  Window win;
  GC gc, bgc, wgc;
  
  Display* display;
  int screen;

  ExportData* database;
  int numObjects;

  unsigned long red, green, blue, yellow, magenta, cyan, white, black; 
  int grey;

  int width, height, x,y;
  int iwidth, iheight, panx, pany;
  int drawMode;
   
  int showSensors;

  XEvent reportEvent;

  double xscale, yscale;
  double dimx, dimy;

 // data
  CWorld* world;
  CObject* dragging;
  CObject* near;

  // methods  

  unsigned int RGB( int r, int g, int b )
  {
    return (((r << 8) | g) << 8) | b;
  };

  ////void CheckSubscriptions( CRobot* r );
  void BoundsCheck( void );
  int LoadVars( char* initFile );
  void HandleEvent( void );
  void Update( void );
  void MoveObject( CObject* obj, double x, double y, double theta );
  
  void PrintCoords( void );
  
  void DrawBackground( void );
  void RefreshObjects( void );
  
  void DrawWalls( void );
  void BlackBackground( void );
  void BlackBackgroundInRect(int xx, int yy, int ww, int hh  );
  void DrawWallsInRect( int xx, int yy, int ww, int hh );
  void ScanBackground( void );

  void MoveSize(void);
  //void DrawInRobotColor( CRobot* r );
  //unsigned long  RobotDrawColor( CRobot* r );
  void SetForeground( unsigned long color );
  void DrawLines( DPoint* pts, int numPts );
  void DrawPoints( DPoint* pts, int numPts );
  void DrawPolygon( DPoint* pts, int numPts );
  void DrawCircle( double x, double y, double r );

  void SetDrawMode( int mode );

  void DrawBox( double px, double py, double boxdelta );

  void RenderGenericObject( ExportData* exp );
  void RenderLaserTurret( ExportData* exp );
  void RenderPioneer( ExportData* exp );
  void RenderUSCPioneer( ExportData* exp );
  void RenderPTZ( ExportData* exp );
  void RenderBox( ExportData* exp );
  void RenderLaserBeacon( ExportData* exp );

  void GetRect( double x, double y, double dx, double dy, 
	       double rotateAngle, DPoint* pts );

  void HighlightObject( CObject* obj );

  ExportData* GetLastMatchingExport( ExportData* exp );
  void RenderObject( ExportData* exp );
  void ImportExportData( ExportData* exp );
};

#endif

/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: xgui.hh,v 1.5 2001-07-11 02:09:14 gerkey Exp $
 ************************************************************************/

#ifndef WIN_H
#define WIN_H

#include <X11/Xlib.h>
#include <string.h>
#include <iostream.h>

#include "guiexport.hh"

// forward definition
class CWorld;
class CEntity;

class CXGui
{
public:
  // constructor
  CXGui( CWorld* wworld );
  
  // destructor
  CXGui::~CXGui( void );

  Window win;
  GC gc, bgc, wgc;
  
  Display* display;
  int screen;

  // an array of pointers to ExportData objects
  ExportData** database;
  int numObjects;

  unsigned int requestPointerMoveEvents;
  unsigned long red, green, blue, yellow, magenta, cyan, white, black; 
  int grey;

  int width, height, x,y;
  int iwidth, iheight, panx, pany;
  int drawMode;
   
  int showSensors;

  XEvent reportEvent;

  //double xscale, yscale;
  double dimx, dimy;

  double ppm; // scale between world and X coordinates
  
  // data
  CWorld* world;
  ExportData* dragging;
  ExportData* near;

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
  void MoveObject( ExportData* exp, double x, double y, double theta );
  
  void PrintCoords( void );
  
  void DrawBackground( void );
  void RefreshObjects( void );
  
  void DrawWalls( void );
  void BlackBackground( void );
  void BlackBackgroundInRect(int xx, int yy, int ww, int hh  );
  void DrawWallsInRect( int xx, int yy, int ww, int hh );
  void ScanBackground( void );

  void MoveSize(void);
  void SetForeground( unsigned long color );
  void DrawLines( DPoint* pts, int numPts );
  void DrawPoints( DPoint* pts, int numPts );
  void DrawPolygon( DPoint* pts, int numPts );
  void DrawCircle( double x, double y, double r );
  void DrawString( double x, double y, char* str, int len );

  void SetDrawMode( int mode );

  void DrawBox( double px, double py, double boxdelta );

  void RenderObjectLabel( ExportData* exp, char* str, int len );
  void RenderGenericObject( ExportData* exp );

  void RenderLaserTurret( ExportData* exp, bool extended  );
  void RenderPioneer( ExportData* exp, bool extended  );
  void RenderUSCPioneer( ExportData* exp, bool extended  );
  void RenderPTZ( ExportData* exp, bool extended  );
  void RenderBox( ExportData* exp, bool extended  );
  void RenderLaserBeacon( ExportData* exp, bool extended  );
  void RenderPuck( ExportData* exp, bool extended  );
  void RenderGripper( ExportData* exp, bool extended  );
  void RenderMisc( ExportData* exp, bool extended  );
  void RenderSonar( ExportData* exp, bool extended  );
  void RenderPlayer( ExportData* exp, bool extended  );
  void RenderLaserBeaconDetector( ExportData* exp, bool extended  );
  void RenderVision( ExportData* exp, bool extended  );

  void GetRect( double x, double y, double dx, double dy, 
	       double rotateAngle, DPoint* pts );

  void HighlightObject( ExportData* obj, bool undraw );

  ExportData* GetLastMatchingExport( ExportData* exp );
  ExportData*  NearestObject( double x, double y );
  void RenderObject( ExportData* exp, bool extended );
  void ImportExportData( ExportData* exp );
  size_t DataSize( ExportData* exp );
  bool GetObjectType( ExportData* exp, char* buf, size_t maxlen );
  bool IsDiffGenericExportData(ExportData* exp1, ExportData* exp2 );
  bool IsDiffSpecificExportData(ExportData* exp1, ExportData* exp2 );
  void CopyGenericExportData( ExportData* dest, ExportData* src );
  void CopySpecificExportData( ExportData* dest, ExportData* src );
 
};


#endif

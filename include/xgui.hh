/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: xgui.hh,v 1.1.2.4 2001-05-30 02:21:26 vaughan Exp $
 ************************************************************************/

#ifndef WIN_H
#define WIN_H

#include <X11/Xlib.h>

#include <stage.h> // for some player data types and sizes
#include <string.h>
#include <iostream.h>

// forward definition
class CWorld;
class CEntity;

#define LABELSIZE 32
#define SONARSAMPLES PLAYER_NUM_SONAR_SAMPLES 
#define LASERSAMPLES PLAYER_NUM_LASER_SAMPLES 

#define MAXBEACONS 100

#define MAXEXPORTOBJECTS 10000;

// for now i have a MAX_OBJECTS maximum to the number of objects
// that can be processed by the GUI. should make this dynamic one day
//static int MAX_OBJECTS = 1000;

enum ExportObjectType 
{ 
  player_o,
  sonar_o,
  misc_o,
  laserbeacondetector_o,
  vision_o,
  ptz_o, 
  generic_o, 
  laserturret_o, 
  pioneer_o, 
  uscpioneer_o,		  
  box_o, 
  laserbeacon_o
};

typedef struct  
{
  double x, y;
} DPoint;

typedef struct  
{
  double x, y, th;
} DTriple;

typedef struct
{
  double toplx, toply, toprx, topry, botlx, botly, botrx, botry;
} DRect;

typedef struct  
{
  double x, y, th;
  int id;
} BeaconData;


// structure to hold an object's exportable data
// i.e. sufficient to render it in an external GUI
// each CObject (CThing) has one of these when XGui is enabled.
typedef struct
{
  int objectType;
  CEntity* objectId;
  double x, y, th;
  double width, height;
  char* data;
  char label[LABELSIZE];
} ExportData;

typedef struct
{
  DPoint hitPts[LASERSAMPLES];
  int hitCount;
} ExportLaserData;

typedef struct
{
  DPoint hitPts[SONARSAMPLES];
  int hitCount;
} ExportSonarData;

typedef struct
{
  BeaconData beacons[MAXBEACONS];
  int beaconCount;
} ExportLaserBeaconDetectorData;

typedef struct
{
  double pan, tilt, zoom;
} ExportPtzData;


// for now the import type can be the same as the export type.
// might need to change this later, so i'll declare a type for it.
typedef  ExportData ImportData;

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
  //CEntity* near;

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

/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: xs.hh,v 1.2 2001-08-10 20:48:38 vaughan Exp $
 ************************************************************************/

#ifndef _WIN_H
#define _WIN_H

#include <X11/Xlib.h>
#include <string.h>
#include <iostream.h>

#include <messages.h> // player data types

typedef struct
{
  int stage_id;
  StageType stage_type;
  int channel;
  player_id_t id;
  player_id_t parent;
  double x, y, th, w, h; // pose and extents
  double rotdx, rotdy; // center of rotation offsets
  int subtype; 
} truth_t;


typedef struct  
{
  double x, y;
} DPoint;

//typedef struct  
//{
// double x, y, col;
//} DPixel;

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
  int width, height; // in pixels
  int ppm, num_pixels;
  XPoint* pixels; // coords in original image pixels
  XPoint* pixels_scaled; // coords in screen pixels
} environment_t;

class CXGui
{
public:
  // constructor
  CXGui( int argc, char** argv, environment_t* anenv );

  // destructor
  ~CXGui( void );

  char* window_title;

  environment_t* env;
  Window win;
  GC gc, bgc, wgc;
  
  Display* display;
  int screen;

  truth_t* dragging;

  unsigned int requestPointerMoveEvents;
  unsigned long channel_colors[ 32 ];
  unsigned long red, green, blue, yellow, magenta, cyan, white, black, grey; 

  int width, height, x,y;
  int iwidth, iheight, panx, pany;
  int drawMode;
   
  int showSensors;

  XEvent reportEvent;

  double ppm; // scale between world and X coordinates
  
  // methods  

  unsigned int RGB( int r, int g, int b )
  {
    return (((r << 8) | g) << 8) | b;
  };

  void BoundsCheck( void );
  int LoadVars( char* initFile );
  void HandleXEvent( void );
  void Update( void );
  void MoveObject( truth_t *obj, double x, double y, double th );

  void PrintCoords( void );
  char* StageNameOf( const truth_t& truth ); 
  char* PlayerNameOf( const player_id_t& ent ); 
  void PrintMetricTruth( int stage_id, truth_t &truth );
  void PrintMetricTruthVerbose( int stage_id, truth_t &truth );

  void DrawBackground( void );
  void RefreshObjects( void );
  
   void Move(void);
  void Size(void);
  void SetForeground( unsigned long color );
  void DrawLine( DPoint& a, DPoint& b );
  void DrawLines( DPoint* pts, int numPts );
  void DrawPoints( DPoint* pts, int numPts );
  void DrawPolygon( DPoint* pts, int numPts );
  void DrawCircle( double x, double y, double r );
  void DrawArc( double x, double y, double w, double h, 
		double th1, double th2 );
  void DrawString( double x, double y, char* str, int len );
  void DrawNoseBox( double x, double y, double w, double h, double th );
  void DrawNose( double x, double y, double l, double th );

  void SetDrawMode( int mode );

  void DrawBox( double px, double py, double boxdelta );


  void RenderObject( truth_t &truth ); 

  void RenderObjectLabel( truth_t* exp, char* str, int len );
  void RenderGenericObject( truth_t* exp );

  void RenderLaserTurret( truth_t* exp, bool extended  );
  void RenderRectRobot( truth_t* exp, bool extended  );
  void RenderRoundRobot( truth_t* exp, bool extended  );
  void RenderUSCPioneer( truth_t* exp, bool extended  );
  void RenderPTZ( truth_t* exp, bool extended  );
  void RenderBox( truth_t* exp, bool extended  );
  void RenderLaserBeacon( truth_t* exp, bool extended  );
  void RenderMisc( truth_t* exp, bool extended  );
  void RenderSonar( truth_t* exp, bool extended  );
  void RenderPlayer( truth_t* exp, bool extended  );
  void RenderLaserBeaconDetector( truth_t* exp, bool extended  );
  void RenderBroadcast( truth_t* exp, bool extended  );
  void RenderVision( truth_t* exp, bool extended  ); 
  void RenderVisionBeacon( truth_t* exp, bool extended  );
  void RenderTruth( truth_t* exp, bool extended  );
  void RenderOccupancy( truth_t* exp, bool extended );
  void RenderGripper( truth_t* exp, bool extended );
  void RenderGps( truth_t* exp, bool extended );
  void RenderPuck( truth_t* exp, bool extended );
  void RenderOccupancyGrid( void );
  truth_t* NearestEntity( double x, double y );

  void GetRect( double x, double y, double dx, double dy, 
	       double rotateAngle, DPoint* pts );

  void GetRect( truth_t* exp );

  void HighlightObject( truth_t* exp, bool undraw );

  void HandleIncomingQueue( void );

  void CalcPPM( void );
  void ScaleBackground( void );
  void HandleCursorKeys( KeySym &key );
  void Zoom( double ratio );
  void StopDragging( XEvent& reportEvent );
  void StartDragging( XEvent& reportEvent );

  void HandleExposeEvent( XEvent& reportEvent );
  void HandleConfigureEvent( XEvent& reportEvent );
  void HandleKeyPressEvent( XEvent& reportEvent );
  void HandleButtonPressEvent( XEvent& reportEvent );
  void HandleMotionEvent( XEvent& reportEvent );

  void SelectColor( truth_t* exp, unsigned long def );

  void SetupZoom( char* );
  void SetupPan( char* );
  void SetupGeometry( char* );
  void SetupChannels( char* );

  Colormap default_cmap;
};


#endif // _WIN_H

 


/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: xs.hh,v 1.7 2001-09-21 02:04:39 vaughan Exp $
 ************************************************************************/

#ifndef _WIN_H
#define _WIN_H

#include <X11/Xlib.h>
#include <string.h>
#include <iostream.h>

#include <messages.h> // player data types
#include <playermulticlient.h>

const int NUM_PROXIES = 64;

typedef struct
{
  int stage_id;
  StageType stage_type;
  //int channel;
  StageColor color;
  unsigned long pixel_color;
  player_id_t id;
  player_id_t parent;
  double x, y, th, w, h; // pose and extents
  double rotdx, rotdy; // center of rotation offsets
  int subtype; 
} xstruth_t;


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

  /* create a multiclient to poll my Player reading */
  PlayerMultiClient playerClients;
  
  ClientProxy* playerProxies[NUM_PROXIES];
  int num_proxies;

  environment_t* env;
  Window win;
  GC gc, bgc, wgc;
  
  Display* display;
  int screen;

  bool draw_all_devices;

  xstruth_t* dragging;

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

  void HandlePlayers( void );
  void AddClient( xstruth_t* ent );

  void BoundsCheck( void );
  int LoadVars( char* initFile );
  void HandleXEvent( void );
  void Update( void );
  void MoveObject( xstruth_t *obj, double x, double y, double th );

  void HeadingStick( xstruth_t* truth );

  void PrintCoords( void );
  char* StageNameOf( const xstruth_t& truth ); 
  char* PlayerNameOf( const player_id_t& ent ); 
  void PrintMetricTruth( int stage_id, xstruth_t &truth );
  void PrintMetricTruthVerbose( int stage_id, xstruth_t &truth );

  void DrawBackground( void );
  void RefreshObjects( void );
  
   void Move(void);
  void Size(void);
  void SetForeground( unsigned long color );
  void DrawLine( DPoint& a, DPoint& b );
  void DrawLines( DPoint* pts, int numPts );
  void DrawPoints( DPoint* pts, int numPts );
  void DrawPolygon( DPoint* pts, int numPts );
  void FillPolygon( DPoint* pts, int numPts );
  void FillCircle( double x, double y, double r );
  void DrawRect( double x, double y, double w, double h, double th );
  void DrawCircle( double x, double y, double r );
  void DrawArc( double x, double y, double w, double h, 
		double th1, double th2 );
  void FillArc( double x, double y, double w, double h, 
		double th1, double th2 );
  void DrawString( double x, double y, char* str, int len );
  void DrawNoseBox( double x, double y, double w, double h, double th );
  void DrawNose( double x, double y, double l, double th );

  void SetDrawMode( int mode );

  void DrawBox( double px, double py, double boxdelta );


  void RenderObject( xstruth_t &truth ); 

  void RenderObjectLabel( xstruth_t* exp, char* str, int len );
  void RenderGenericObject( xstruth_t* exp );

  void RenderLaserTurret( xstruth_t* exp, bool extended  );
  void RenderRectRobot( xstruth_t* exp, bool extended  );
  void RenderRoundRobot( xstruth_t* exp, bool extended  );
  void RenderUSCPioneer( xstruth_t* exp, bool extended  );
  void RenderPTZ( xstruth_t* exp, bool extended  );
  void RenderBox( xstruth_t* exp, bool extended  );
  void RenderLaserBeacon( xstruth_t* exp, bool extended  );
  void RenderMisc( xstruth_t* exp, bool extended  );
  void RenderSonar( xstruth_t* exp, bool extended  );
  void RenderPlayer( xstruth_t* exp, bool extended  );
  void RenderLaserBeaconDetector( xstruth_t* exp, bool extended  );
  void RenderBroadcast( xstruth_t* exp, bool extended  );
  void RenderVision( xstruth_t* exp, bool extended  ); 
  void RenderVisionBeacon( xstruth_t* exp, bool extended  );
  void RenderTruth( xstruth_t* exp, bool extended  );
  void RenderOccupancy( xstruth_t* exp, bool extended );
  void RenderGripper( xstruth_t* exp, bool extended );
  void RenderGps( xstruth_t* exp, bool extended );
  void RenderPuck( xstruth_t* exp, bool extended );
  void RenderOccupancyGrid( void );

  void RenderLaserProxy( LaserProxy* prox );
  void RenderSonarProxy( SonarProxy* prox );
  void RenderGpsProxy( GpsProxy* prox );
  void RenderPtzProxy( PtzProxy* prox );
  void RenderVisionProxy( VisionProxy* prox );

  xstruth_t* NearestEntity( double x, double y );

  void GetRect( double x, double y, double dx, double dy, 
	       double rotateAngle, DPoint* pts );

  void GetRect( xstruth_t* exp );

  void HighlightObject( xstruth_t* exp, bool undraw );

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

  void SelectColor( xstruth_t* exp );

  void SetupZoom( char* );
  void SetupPan( char* );
  void SetupGeometry( char* );
  void SetupChannels( char* );

  Colormap default_cmap;
};


#endif // _WIN_H

 


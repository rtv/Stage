/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: xs.hh,v 1.8 2001-09-22 01:51:50 vaughan Exp $
 ************************************************************************/

#ifndef _WIN_H
#define _WIN_H

#include <X11/Xlib.h>
#include <string.h>
#include <iostream.h>

#include <messages.h> // player data types
#include <playermulticlient.h>
#include <map>

#include "laserproxy.h"
#include "sonarproxy.h"
#include "gpsproxy.h"
#include "visionproxy.h"
#include "ptzproxy.h"
#include "laserbeaconproxy.h"

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

// a map template for storing them
typedef std::map< int, xstruth_t > TruthMap;

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

  // an associative array, indexed by player ID (neat!)
  TruthMap truth_map; 
  
  double ppm; // scale between world and X coordinates
  
  // methods  

  unsigned int RGB( int r, int g, int b )
  {
    return (((r << 8) | g) << 8) | b;
  };

  bool PoseFromId( int port, int device, int index, 
		   double& x, double& y, double& th, unsigned long& col );
    
  void HandlePlayers( void );
  void TogglePlayerClient( xstruth_t* ent );

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

  xstruth_t* NearestEntity( double x, double y );

  void GetSonarPose(int s, double &px, double &py, double &pth );  

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

class CGraphicLaserProxy : public LaserProxy
{
protected:
  DPoint pts[ PLAYER_NUM_LASER_SAMPLES ];
  unsigned long pixel;
  int samples;
  CXGui* win;
  
public:
  
  CGraphicLaserProxy( CXGui* w, PlayerClient* pc, unsigned short index, 
		      unsigned char access='c'):
    LaserProxy(pc,index,access) 
  {
    win = w;
    pixel = 0;
    samples = 0;
    memset( pts, 0, PLAYER_NUM_LASER_SAMPLES * sizeof( unsigned short ) );
  };

  ~CGraphicLaserProxy( void )
  {
    // undraw the old data
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    win->DrawPolygon( pts, samples );
  };

  void Render();
  
};

class CGraphicSonarProxy : public SonarProxy
{
protected:
  DPoint pts[ PLAYER_NUM_SONAR_SAMPLES ];
  unsigned long pixel;
  CXGui* win;
  
public:
  
  CGraphicSonarProxy( CXGui* w, PlayerClient* pc, unsigned short index, 
		      unsigned char access='c'):
    SonarProxy(pc,index,access) 
  {
    win = w;
    pixel = 0;
    memset( pts, 0, PLAYER_NUM_SONAR_SAMPLES * sizeof( unsigned short ) );
  };

  ~CGraphicSonarProxy( void )
  {
    // undraw the old data
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    win->DrawPolygon( pts, PLAYER_NUM_SONAR_SAMPLES );
  };
  
  void Render();
};

class CGraphicGpsProxy : public GpsProxy
{
protected:
  char buf[64];
  double drawx, drawy;
  
  unsigned long pixel;
  CXGui* win;
  
public:
  
  CGraphicGpsProxy( CXGui* w, PlayerClient* pc, unsigned short index, 
		    unsigned char access='c'):
    GpsProxy(pc,index,access) 
  {
    win = w;
    pixel = 0;
    drawx = drawy = -99999;
    memset( buf, 0, 64 );
  };

  ~CGraphicGpsProxy( void )
  {
    // undraw the old data
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    win->DrawString( drawx, drawy, buf, strlen(buf) );
  };
  
  void Render();
};

class CGraphicVisionProxy : public VisionProxy
{
protected:
  unsigned long pixel;
  CXGui* win;
  
public:
  
  CGraphicVisionProxy( CXGui* w, PlayerClient* pc, unsigned short index, 
		       unsigned char access='c'):
    VisionProxy(pc,index,access) 
  {
    win = w;
    pixel = 0;
  };
  
  ~CGraphicVisionProxy( void )
  {
    // undraw the old data
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
  };
  
  void Render()
  {};
  //  { Print(); };
  
};

class CGraphicPtzProxy : public PtzProxy
{
protected:
  unsigned long pixel;
  CXGui* win;
  DPoint pts[3];

public:
  
  CGraphicPtzProxy( CXGui* w, PlayerClient* pc, unsigned short index, 
		       unsigned char access='c'):
    PtzProxy(pc,index,access) 
  {
    win = w;
    pixel = 0;
  };
  
  ~CGraphicPtzProxy( void )
  {
    // undraw the old data
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    win->DrawPolygon( pts, 3 );
  };
  
  void Render();
};


class CGraphicLaserBeaconProxy : public LaserbeaconProxy
{
protected:
  unsigned long pixel;
  CXGui* win;
  
  DPoint origin;
  DPoint pts[ PLAYER_MAX_LASERBEACONS ];
  int ids[ PLAYER_MAX_LASERBEACONS ];
  int stored;
  
public:
  
  CGraphicLaserBeaconProxy( CXGui* w, PlayerClient* pc, unsigned short index, 
			    unsigned char access='c'):
    LaserbeaconProxy(pc,index,access) 
  {
    origin.x = origin.y = 0;
    win = w;
    pixel = 0;
    stored = 0;
  };
  
  ~CGraphicLaserBeaconProxy( void )
  {
    // undraw the old data
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    
    char buf[16];
    
    for( int b=0; b<stored; b++ )
      {
	win->DrawLine( origin, pts[b] );
	
	sprintf( buf, "%d", ids[b] );
	win->DrawString( pts[b].x + 0.1, pts[b].y, buf, strlen(buf) ); 
	//win->DrawString( origin.x + 0.1, origin.y, buf, strlen(buf) ); 
      };
  };
  
  void Render();
};


#endif // _WIN_H

 


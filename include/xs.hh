/*************************************************************************
 * win.h - all the X graphics stuff is here
 * RTV
 * $Id: xs.hh,v 1.19 2002-02-27 22:27:27 rtv Exp $
 ************************************************************************/

#ifndef _WIN_H
#define _WIN_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <string.h>
#include <iostream.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <messages.h> // player data types
#include <playermulticlient.h>
#include <map>

#include "laserproxy.h"
#include "sonarproxy.h"
#include "gpsproxy.h"
#include "visionproxy.h"
#include "ptzproxy.h"

#ifdef HRL_DEVICES
#include "laserbeaconproxy.h"
#include "idarproxy.h"
#endif

const int NUM_PROXIES = 64;

// forward declaration
class GraphicProxy;

typedef struct
{
  int stage_id, parent_id;
  StageType stage_type;
  StageColor color;
  unsigned long pixel_color;
  //char hostname[ HOSTNAME_SIZE ];
  struct in_addr hostaddr;
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

const int MAXMSGS = 10000;

class CXGui
{
public:
  void DrawIDARMessages( void );
  bool ReceiveMessage( DPoint& from, DPoint& to, unsigned long &col );
  void HandleIDARMessages( void );
  void SetupMessageServer( void );

  unsigned long idar_col[MAXMSGS];  
  DPoint idar_from[MAXMSGS];
  DPoint idar_to[MAXMSGS];
  int idar_count;

  // constructor
  CXGui( int argc, char** argv );

  // destructor
  ~CXGui( void );

  void Startup( int argc, char** argv );
  
  char* window_title;

  /* create a multiclient to poll my Player reading */
  PlayerMultiClient playerClients;

  GraphicProxy* graphicProxies[NUM_PROXIES];
  ClientProxy* playerProxies[NUM_PROXIES];
  int num_proxies;

  environment_t* env;
  Window win, infowin;
  GC gc, bgc, wgc;
  
  XTextProperty windowName; // we store the standard window title here

  Display* display;
  int screen;

  bool draw_all_devices;

  xstruth_t* dragging;

  bool scrolling;
  int scrollStartX, scrollStartY;
  
  bool enableLaser, enableSonar, enablePtz, enableVision, enableGps,
    enableLaserBeacon, enableIDAR;
  
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

  bool PoseFromId( int stage_id,  
		   double& x, double& y, double& th, unsigned long& col );
    
  bool PoseFromPlayerId( int port, int type, int index, 
			 double& x, double& y, double& th, unsigned long& col);
    

  void HandlePlayers( void );
  void TogglePlayerClient( xstruth_t* ent );

  bool DownloadEnvironment();
  bool DownloadObjects( int, int);

  void BoundsCheck( void );
  int LoadVars( char* initFile );
  void HandleXEvent( void );
  void Update( void );
  void MoveObject( xstruth_t *obj, double x, double y, double th );

  void HeadingStick( xstruth_t* truth );

  void GetGlobalPose( xstruth_t &truth, double &px, double &py, double &pth);
  
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
  // render an object and all its children
  void RenderFamily( xstruth_t &truth );

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
#ifdef HRL_DEVICES
  void RenderIDAR( xstruth_t* exp, bool extended );
#endif
  void RenderPuck( xstruth_t* exp, bool extended );
  void RenderOccupancyGrid( void );

  xstruth_t* NearestEntity( double x, double y );

  void GetSonarPose(int s, double &px, double &py, double &pth );  

  void GetRect( double x, double y, double dx, double dy, 
	       double rotateAngle, DPoint* pts );

  void GetRect( xstruth_t* exp );

  void HighlightObject( xstruth_t* exp, bool undraw );

  void HandleIncomingQueue( void );
  void ImportTruth( stage_truth_t &struth ); 

  void CalcPPM( void );
  void ScaleBackground( void );
  void HandleCursorKeys( KeySym &key );
  void Zoom( double ratio );
  void Zoom( double ratio, int centerx, int centery );
  void StopDragging( XEvent& reportEvent );
  void StartDragging( XEvent& reportEvent );
  void StopScrolling( XEvent& reportEvent );
  void StartScrolling( XEvent& reportEvent );

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


// a virtual class that holds some common functionality for proxies
class GraphicProxy
{
public:
  unsigned long pixel;
  CXGui* win;
  int stage_id; // the unique id for this device

  GraphicProxy( CXGui* w, int i )
  {
    win = w;
    pixel = 0;
    stage_id = i;
    redraw = true;
  }

  // subclasses must provide these functions
  virtual void Draw( void ) = 0;
  virtual void ProcessData( void ) = 0;

  bool redraw;

  void Render()
  {
    Draw();
    ProcessData();
    Draw();
  };
};

class CGraphicLaserProxy : public LaserProxy, public GraphicProxy
{
protected:
  DPoint pts[ PLAYER_NUM_LASER_SAMPLES ];
  int samples;
  
public:
  CGraphicLaserProxy( CXGui* w, int id, PlayerClient* pc, unsigned short index, 
		      unsigned char access='c'):
    LaserProxy(pc,index,access), GraphicProxy( w, id ) 
  {
    samples = 0;
    memset( pts, 0, PLAYER_NUM_LASER_SAMPLES * sizeof( unsigned short ) );
  };

  virtual void ProcessData();
  virtual void Draw( void )
  {
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    win->DrawPolygon( pts, samples );
  };
  
  ~CGraphicLaserProxy( void ){ Draw(); };

};

class CGraphicSonarProxy : public SonarProxy, public GraphicProxy
{
protected:
  DPoint endpts[ PLAYER_NUM_SONAR_SAMPLES ];
  DPoint startpts[ PLAYER_NUM_SONAR_SAMPLES ];
  
public:
  CGraphicSonarProxy( CXGui* w, int id, PlayerClient* pc, unsigned short index, 
		      unsigned char access='c'):
    SonarProxy(pc,index,access), GraphicProxy( w, id )
  {
    memset( startpts, 0, PLAYER_NUM_SONAR_SAMPLES * sizeof( unsigned short ) );
    memset( endpts, 0, PLAYER_NUM_SONAR_SAMPLES * sizeof( unsigned short ) );
  };

  virtual void ProcessData( void );
  virtual void Draw( void )
  {
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );

    for( int i=0; i<PLAYER_NUM_SONAR_SAMPLES; i++ )
      win->DrawLine( startpts[i], endpts[i] );
  };

  ~CGraphicSonarProxy( void ){Draw();};
  
};

#ifdef HRL_DEVICES

class CGraphicIDARProxy : public IDARProxy, public GraphicProxy
{
protected:
  DPoint intensitypts[ PLAYER_NUM_IDAR_SAMPLES ];
  DPoint rangepts[ PLAYER_NUM_IDAR_SAMPLES ][ RAYS_PER_SENSOR ];
  
  DPoint origin;

  char rx_buf[PLAYER_NUM_IDAR_SAMPLES][16]; // should be big enough

public:
  CGraphicIDARProxy( CXGui* w, int id, PlayerClient* pc, unsigned short index, 
		      unsigned char access='c'):
    IDARProxy(pc,index,access), GraphicProxy( w, id )
  {
    origin.x = origin.y = 0.0;

    memset( intensitypts, 0, PLAYER_NUM_IDAR_SAMPLES *  sizeof(DPoint) );
    memset( rangepts, 0, 
	    PLAYER_NUM_IDAR_SAMPLES * RAYS_PER_SENSOR * sizeof(DPoint) );

    // init the value string buffers
    for( int l=0; l < PLAYER_NUM_IDAR_SAMPLES; l++ )
      {
	sprintf( rx_buf[l], "%u", 0 );
      }
  };

  virtual void ProcessData( void );
  virtual void Draw( void )
  {
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );

    for( int i=0; i<PLAYER_NUM_IDAR_SAMPLES; i++ )
      {
	//win->DrawLine( origin, intensitypts[i] );
	win->DrawString( intensitypts[i].x, intensitypts[i].y, 
			 rx_buf[i], strlen(rx_buf[i] ) ); 
      }

    win->SetForeground( win->grey );
    win->SetDrawMode( GXxor );

    for( int i=0; i<PLAYER_NUM_IDAR_SAMPLES; i++ )
      {  //for( int f=0; f<RAYS_PER_SENSOR; f++ )
	//win->DrawLine( origin, rangepts[i][f] ); 
	win->DrawLines( rangepts[i], RAYS_PER_SENSOR ); 
	win->DrawLine( rangepts[i][0], origin );
	win->DrawLine( rangepts[i][RAYS_PER_SENSOR-1], 
		       rangepts[(i+1)%PLAYER_NUM_IDAR_SAMPLES][0] );
      }
    
  };

  ~CGraphicIDARProxy( void ){Draw();};
  
};

#endif

class CGraphicGpsProxy : public GpsProxy, public GraphicProxy
{
protected:
  char buf[64];
  double drawx, drawy;
    
public:
  
  CGraphicGpsProxy( CXGui* w, int id, PlayerClient* pc, unsigned short index, 
		    unsigned char access='c'):
    GpsProxy(pc,index,access), GraphicProxy( w, id )
  {
    drawx = drawy = -99999;
    memset( buf, 0, 64 );
  };

  virtual void ProcessData( void );
  virtual void Draw( void ) 
  {
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    win->DrawString( drawx, drawy, buf, strlen(buf) );
  };
  
  ~CGraphicGpsProxy( void ){ Draw(); };
};

class CGraphicVisionProxy : public VisionProxy, public GraphicProxy
{
public:
  
  CGraphicVisionProxy( CXGui* w, int id, PlayerClient* pc, unsigned short index, 
		       unsigned char access='c'):
    VisionProxy(pc,index,access), GraphicProxy( w, id ) 
  {
    int foo = 0;
    foo++;
    // nothing here
  };
  
  virtual void ProcessData( void )
  {
    // do nothing!;
  };
  
  virtual void Draw()
  {
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    // nothing to draw yet
  };
  
  ~CGraphicVisionProxy( void ){ Draw(); };  
};

class CGraphicPtzProxy : public PtzProxy, public GraphicProxy
{
protected:
  DPoint pts[3];

public:
  
  CGraphicPtzProxy( CXGui* w, int id, PlayerClient* pc, unsigned short index, 
		       unsigned char access='c'):
    PtzProxy(pc,index,access), GraphicProxy( w, id ) 
  {
    memset( pts, 0, 3 * sizeof( DPoint ) );
  };

  virtual void ProcessData( void );
  virtual void Draw( void )
  {
    win->SetForeground( pixel );
    win->SetDrawMode( GXxor );
    win->DrawPolygon( pts, 3 );
  };
  
  ~CGraphicPtzProxy( void ){ Draw(); };
};


class CGraphicLaserBeaconProxy : public LaserbeaconProxy, public GraphicProxy
{
protected:
  DPoint origin;
  DPoint pts[ PLAYER_MAX_LASERBEACONS ];
  int ids[ PLAYER_MAX_LASERBEACONS ];
  int stored;
  
public:
  
  CGraphicLaserBeaconProxy( CXGui* w, int id, 
			    PlayerClient* pc, unsigned short index, 
			    unsigned char access='c'):
    LaserbeaconProxy(pc,index,access) , GraphicProxy( w, id )
  {
    origin.x = origin.y = 0;
    stored = 0;
  };

  virtual void ProcessData( void );
  virtual void Draw( void )
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
      };
  };

  ~CGraphicLaserBeaconProxy( void ){ Draw(); };
};


#endif // _WIN_H

 







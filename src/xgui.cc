/*************************************************************************
 * win.cc - all the graphics and X management
 * RTV
 * $Id: xgui.cc,v 1.1.2.2 2001-05-21 23:34:17 vaughan Exp $
 ************************************************************************/

#include <stream.h>
#include <assert.h>
#include <math.h>
#include <iomanip.h>
#include <values.h>
#include <signal.h>
#include <fstream.h>
#include <iostream.h>

#include "xgui.hh"
#include "world.hh"

#include <X11/keysym.h> 
#include <X11/keysymdef.h> 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define DEBUG
#define VERBOSE

#define LUI long unsigned int

XPoint* backgroundPts;
int backgroundPtsCount;

// temporary hack to set these values
#define SONARSAMPLES 16
#define LASERSAMPLES 361

const double TWOPI = 6.283185307;
const int numPts = SONARSAMPLES;
const int laserSamples = LASERSAMPLES;

const float maxAngularError =  -0.1; // percent error on turning odometry

//extern int drawMode;

unsigned int RGB( int r, int g, int b );

int coords = false;



CXGui::CXGui( CWorld* wworld )//,  char* initFile )
{
#ifdef DEBUG
  cout << "Creating a window..." << flush;
#endif

  world = wworld;

  database = new ExportData[ MAX_OBJECTS ];
  numObjects = 0;

  char* title = "Stage";

  grey = 0x00FFFFFF;

  // provide some sensible defaults for the window parameters
  xscale = yscale = 1.0;
  width = 600;
  height = 400;
  panx = pany = x = y = 0;

  //LoadVars( initFile ); // read the window parameters from the initfile

  // init X globals 
  display = XOpenDisplay( (char*)NULL );
  screen = DefaultScreen( display );

  Colormap default_cmap = DefaultColormap( display, screen ); 

  XColor col, rcol;
  XAllocNamedColor( display, default_cmap, "red", &col, &rcol ); 
  red = col.pixel;
  XAllocNamedColor( display, default_cmap, "green", &col, &rcol ); 
  green= col.pixel;
  XAllocNamedColor( display, default_cmap, "blue", &col, &rcol ); 
  blue = col.pixel;
  XAllocNamedColor( display, default_cmap, "cyan", &col, &rcol ); 
  cyan = col.pixel;
  XAllocNamedColor( display, default_cmap, "magenta", &col, &rcol ); 
  magenta = col.pixel;
  XAllocNamedColor( display, default_cmap, "yellow", &col, &rcol ); 
  yellow = col.pixel;
  
  black = BlackPixel( display, screen );
  white = WhitePixel( display, screen );

  XTextProperty windowName;

#ifdef DEBUG
  cout << "Window coords: " << x << '+' << y << ':' << width << 'x' << height;
#endif

  win = XCreateSimpleWindow( display, 
			     RootWindow( display, screen ), 
			     x, y, width, height, 4, 
			     black, white );
   
  XSelectInput(display, win, ExposureMask | StructureNotifyMask | 
	       ButtonPressMask | ButtonReleaseMask | KeyPressMask | PointerMotionMask );
 
  XStringListToTextProperty( &title, 1, &windowName);

  iwidth = world->GetBackgroundImage()->width;
    iheight = world->GetBackgroundImage()->height;
  dragging = near = 0;
  //drawMode = false;
  showSensors = false;

  XSizeHints sz;

  sz.flags = PMaxSize | PMinSize;
  sz.min_width = 100; 
  sz.max_width = iwidth;
  sz.min_height = 100;
  sz.max_height = iheight;

  XSetWMProperties(display, win, &windowName, (XTextProperty*)NULL, 
		   (char**)NULL, 0, 
		   &sz, (XWMHints*)NULL, (XClassHint*)NULL );
  
  gc = XCreateGC(display, win, (unsigned long)0, NULL );

  const unsigned long int mask = 0xFFFFFFFF;
  
#ifdef DEBUG
  cout << "Screen Depth: " << DefaultDepth( display, screen ) 
       << " Visual: " << DefaultVisual( display, screen ) << endl;
#endif 

  XMapWindow(display, win);

  ScanBackground();

#ifdef DEBUG
  cout << "ok." << endl;
#endif
}

int CXGui::LoadVars( char* filename )
{
  
#ifdef DEBUG
  cout << "Loading window parameters from " << filename << "... " << flush;
#endif

  ifstream init( filename );
  char buffer[255];
  char c;

  if( !init )
    {
      cout << "FAILED" << endl;
      return -1;
    }
  
  char token[50];
  char value[200];
  
  while( !init.eof() )
    {
      init.get( buffer, 255, '\n' );
      init.get( c );
    
      sscanf( buffer, "%s = %s", token, value );
      
#ifdef DEBUG      
      printf( "token: %s value: %s.\n", token, value );
      fflush( stdout );
#endif
      
      if ( strcmp( token, "window_x_pos" ) == 0 )
	x = strtol( value, NULL, 10 );
      else if ( strcmp( token, "window_y_pos" ) == 0 )
	y = strtol( value, NULL, 10 );
      else if ( strcmp( token, "window_width" ) == 0 )
	width = strtol( value, NULL, 10 );
      else if ( strcmp( token, "window_height" ) == 0 )
	height = strtol( value, NULL, 10 );
      else if ( (strcmp( token, "window_xscroll" ) == 0) 
		|| (strcmp( token, "window_x_scroll" ) == 0) )
	panx = strtol( value, NULL, 10 );
      else if ( (strcmp( token, "window_yscroll" ) == 0 )
		|| (strcmp( token, "window_y_scroll" ) == 0) )
	pany = strtol( value, NULL, 10 );
    }

#ifdef DEBUG
  cout << "ok." << endl;
#endif
  return 1;
}


void CXGui::MoveObject( CObject* obj, double x, double y, double theta )
{    
  ImportData imp;
  
  imp.x = x; 
  imp.y = y; 
  imp.th = theta;
  
  // update the object through its pointer
  obj->ImportExportData( &imp );
}


void CXGui::DrawBackground( void ) 
{ 
  BlackBackground();
  DrawWalls();
}


void CXGui::PrintCoords( void )
{
  Window dummyroot, dummychild;
  int dummyx, dummyy, xpos, ypos;
  unsigned int dummykeys;
  
  XQueryPointer( display, win, &dummyroot, &dummychild, 
		 &dummyx, &dummyy, &xpos, &ypos, &dummykeys );
  
  
  cout << "Mouse: " << xpos << ',' << ypos << endl;;
}
//int firstDrag;

void CXGui::BoundsCheck( void )
{
  if( panx < 0 ) panx = 0;
  if( panx > ( iwidth - width ) ) panx =  iwidth - width;
  
  if( pany < 0 ) pany = 0;
  if( pany > ( iheight - height ) ) pany =  iheight - height;
  
  //hide for a bit
  float xminScale = (float)width / (float)(world->GetBackgroundImage()->width);
  float yminScale = (float)height / (float)(world->GetBackgroundImage()->height);
  
  if( xscale < xminScale ) xscale = xminScale;
  if( yscale < yminScale ) yscale = yminScale;
}


void CXGui::HandleEvent( void )
{    
  XEvent reportEvent;
  int r;
  KeySym key;
  int drawnRobots = false;
  static int sw = 0, sh = 0;

  // used for inverting Y between world and X coordinates
  int imgHeight= world->GetBackgroundImage()->height;
  double wppm = world->ppm; // scale between world and X coordinates
  ImportData imp; // data structure for updating world objects
 
  unsigned int motionMask;
  // we only want mouse motion messages when we're dragging
  //dragging ? motionMask = PointerMotionHintMask : motionMask = 0;

  motionMask = PointerMotionHintMask;

  while( XCheckWindowEvent( display, win,
			    StructureNotifyMask 
			    | ButtonPressMask | ButtonReleaseMask 
			    | ExposureMask | KeyPressMask 
			    | motionMask, 
			    &reportEvent ) )
  
    switch( reportEvent.type ) // there was an event waiting, so handle it...
      {
      case ConfigureNotify:	// window resized or modified
	if( width != reportEvent.xconfigure.width ||
	    height != reportEvent.xconfigure.height )
	  {
	    width = reportEvent.xconfigure.width;
	    height = reportEvent.xconfigure.height;
	    
	    BoundsCheck();

	    x = reportEvent.xconfigure.x;
	    y = reportEvent.xconfigure.y;
	  }

	break;

      case Expose:
	if( reportEvent.xexpose.count == 0 ) // on the last pending expose...
	{
	  ScanBackground();
	  
	  DrawBackground(); // black backround and draw walls
	  RefreshObjects();
	  //drawnRobots = true;
	}
	break;

      case MotionNotify:	
	//cout << "motion" << endl;
	if( dragging )// DragObject();
	  {
	    double dummy, theta;
	    dragging->GetGlobalPose( dummy, dummy, theta );

	    MoveObject( dragging, 
			reportEvent.xmotion.x / wppm + panx,
			(imgHeight-reportEvent.xmotion.y) / wppm - panx,
			theta );

	    HighlightObject( dragging );
	  }
	else // highlight the nearest object? dunno if i want this,
	  {  // but this is the place to do it.
	    double x2, y2, dummy;

	    double x1 = reportEvent.xbutton.x / wppm + panx;
	    double y1 = (imgHeight - reportEvent.xbutton.y) / wppm + pany;

	    near = world->NearestObject( x1, y1 ); 
	    
	    near->GetGlobalPose( x2, y2, dummy );

	    // if the object is close, highlight it
	    if( hypot( x1-x2, y1-y2 ) < 1.0 ) 
	      HighlightObject( near ); 
	    else // it's not close enough - don't grab it
	      near = 0;
	  }
	break;
	
      case ButtonRelease:
	if( near ) HighlightObject( near );
	break;
	
      case ButtonPress: 
	  switch( reportEvent.xbutton.button )
	    {	      
	    case Button1: 
	      if( dragging  ) 
  		{ // stopped dragging
  		  dragging = NULL;
		  
  		  //redraw the walls
  		  //world->Draw();
  		} 
  	      else if( near )
  		{  // find nearest robot and drag it
  		  dragging = near;
		  //world->NearestObject(reportEvent.xbutton.x 
		  //		 / wppm + panx, 
		  //		 (imgHeight-reportEvent.xbutton.y)
		  //		 / wppm + pany ); 
		  
		  double dummy, theta;
		  dragging->GetGlobalPose( dummy, dummy, theta );
		  
		  MoveObject( dragging, 
			      reportEvent.xbutton.x / wppm + panx,
			      (imgHeight-reportEvent.xbutton.y) / wppm - panx,
			      theta );
		}	      
	      break;
	      
	    case Button2: 
	      if( dragging )
		{	  
		  double x, y, theta;
		  dragging->GetGlobalPose( x, y, theta );
		  
		  MoveObject( dragging, 
			      x, y, theta + M_PI/10.0 );
		}	 
//  		  near->ToggleSensorDisplay();

#ifdef DEBUG	      
		  // this can be taken out at release
		  //useful debug stuff!
		  // dump the raw image onto the screen
//  		  for( int f = 0; f<world->img->width; f++ )
//  		    for( int g = 0; g<world->img->height; g++ )
//  		      {
//  			if( world->img->get_pixel( f,g ) == 0 )
//  			  XSetForeground( display, gc, white );
//  			else
//  			  XSetForeground( display, gc, black );

//  			XDrawPoint( display,win,gc, f, g );
//  		      }

//  		  getchar();
		  
//  		  DrawBackground();
//  		  DrawRobots();
//  		  drawnRobots = true;
#endif
		  //	    }
	      break;
	    case Button3: 
	      if( dragging )
		{	  
		  double x, y, theta;
		  dragging->GetGlobalPose( x, y, theta );
		  
		  MoveObject( dragging, 
			      x, y, theta - M_PI/10.0 );
		}	      
	      else
		{ // we're centering on robot[0] - redraw everything
		  //panx = (int)(world->bots[0].x - width/2.0);
		  //pany = (int)(world->bots[0].y - height/2.0);
		  
		  //BoundsCheck();		  
		  //ScanBackground();
		  //DrawBackground();		
		  //DrawRobots();
		  //drawnRobots = true;
		}
	      break;
	    }
	break;
	
      case KeyPress:
	// pan the window view
	key = XLookupKeysym( &reportEvent.xkey, 0 );

	int oldPanx = panx;
	int oldPany = pany;
	double oldXscale = xscale;

	if( key == XK_Left )
	  panx -= width/5;
	else if( key == XK_Right )
	  panx += width/5;
	else if( key == XK_Up )
	  pany -= height/5;
	else if( key == XK_Down )
	  pany += height/5;

	BoundsCheck();

	// if the window has been panned, refresh everything
	if( panx != oldPanx || pany != oldPany )
	  {
	    ScanBackground();
	    DrawBackground();
	    RefreshObjects();
	    //DrawRobots();
	    //drawnRobots = true;
	    
	  }
	break;
      }

  // if we haven't already refreshed the robots on screen, do it
  if( !drawnRobots )
    {

      //RefreshRobots();
      //DrawRobotIfMoved( r );
     
    }

  // try to sync the display to avoid flicker
  XSync( display, false );

}


void CXGui::ScanBackground( void )
{
#ifdef VERBOSE
  cout << "Scanning background (no scaling)... " << flush;
#endif

  if( backgroundPts ) delete[] backgroundPts;

  backgroundPts = new XPoint[width*height];
  backgroundPtsCount = 0;

  Nimage* img = world->GetBackgroundImage();

  int sx, sy, px, py;
  for( px=0; px < width; px++ )
    for( py=0; py < height; py++ )
      if( img->get_pixel( px+panx, py+pany ) != 0 )
      	{
            backgroundPts[backgroundPtsCount].x = px;
            backgroundPts[backgroundPtsCount].y = py;
            backgroundPtsCount++;
      }
#ifdef VERBOSE
  cout << "ok." << endl;
#endif
}


void CXGui::BlackBackground( void )
{
    XSetForeground( display, gc, black );
    XFillRectangle( display, win, gc, 0,0, width, height );
}

void CXGui::BlackBackgroundInRect( int xx, int yy, int ww, int hh )
{
    XSetForeground( display, gc, black );
    XFillRectangle( display, win, gc, xx,yy, ww, hh );
}
 
void CXGui::DrawWalls( void )
{
  XSetForeground( display, gc, white );
  XDrawPoints( display, win, gc, 
	       backgroundPts, backgroundPtsCount, CoordModeOrigin );
}

void CXGui::SetDrawMode( int mode )
{
  // the modes are defined in X11/Xlib.h
  XSetFunction( display, gc, mode );
}


void CXGui::DrawPolygon( DPoint* pts, int numPts )
{
  DPoint* pPts = new DPoint[numPts+1];

  memcpy( pPts, pts, numPts * sizeof( DPoint ) );

  pPts[numPts].x = pts[0].x;
  pPts[numPts].y = pts[0].y;

  DrawLines( pPts, numPts+1 );

  delete[] pPts;
}

void CXGui::DrawLines( DPoint* dpts, int numPts )
{
  //SetDrawMode( GXcopy );
  int imgHeight= world->GetBackgroundImage()->height;

  XPoint* xpts = new XPoint[numPts];
  
  // scale, pan and quantize the points
  for( int l=0; l<numPts; l++ )
    {
      xpts[l].x = ((int)( dpts[l].x * world->ppm )) - panx;
      xpts[l].y = (imgHeight-pany) - ((int)( dpts[l].y * world->ppm ));
    }
  
  // and draw them
  XDrawLines( display, win, gc, xpts, numPts, CoordModeOrigin );
   
  // try to sync the display to avoid flicker
  //XSync( display, false );

  delete [] xpts; 
}

void CXGui::DrawPoints( DPoint* dpts, int numPts )
{
 
  int imgHeight= world->GetBackgroundImage()->height;

  XPoint* xpts = new XPoint[ numPts ];
  
  // scale, pan and quantize the points
  for( int l=0; l<numPts; l++ )
    {
      xpts[l].x = ((int)( dpts[l].x * world->ppm )) - panx;
      xpts[l].y = (imgHeight-pany) - ((int)( dpts[l].y * world->ppm ));
    }
 
  // and draw them
  XDrawPoints( display, win, gc, xpts, numPts, CoordModeOrigin );
   
  delete [] xpts; 
}

void CXGui::DrawCircle( double x, double y, double r )
{
  int imgHeight= world->GetBackgroundImage()->height;

  XDrawArc( display, win, gc, 
	    (LUI)((x-r) * world->ppm -panx), 
	    (LUI)((imgHeight-pany)-((y+r) * world->ppm)), 
	    (LUI)(2.0*r*world->ppm), (LUI)(2.0*r*world->ppm), 0, 23040 );
}

void CXGui::DrawWallsInRect( int xx, int yy, int ww, int hh )
{
   XSetForeground( display, gc, white );

  // draw just the points that are in the defined rectangle
  for( int p=0; p<backgroundPtsCount; p++ )
    if( backgroundPts[p].x >= xx &&
	backgroundPts[p].y >= yy &&  
	backgroundPts[p].x <= xx+ww &&   
	backgroundPts[p].y <= yy+hh )
      XDrawPoint( display, win, gc,  backgroundPts[p].x, backgroundPts[p].y );
}

void CXGui::RefreshObjects( void )
{
  SetDrawMode( GXxor );
 
  for( int o=0; o<numObjects; o++ )     
    RenderObject( &database[o] );

 
  // hide the CRobot reference

  //  for( CRobot* r = world->bots; r; r = r->next ) 
  //  r->GUIDraw();
  
  SetDrawMode( GXcopy );
}

// hide the CRobot reference
//  unsigned long CXGui::RobotDrawColor( CRobot* r )
//  { 
//    unsigned long color;
//    switch( r->channel % 6 )
//      {
//      case 0: color = red; break;
//      case 1: color = green; break;
//      case 2: color = blue; break;
//      case 3: color = yellow; break;
//      case 4: color = magenta; break;
//      case 5: color = cyan; break;
//      }

//    return color;
//  }

void CXGui::SetForeground( unsigned long col )
{ 
  XSetForeground( display, gc, col );
}


// hide the CRobot reference
//  void CXGui::DrawInRobotColor( CRobot* r )
//  { 
//    XSetForeground( display, gc, RobotDrawColor( r ) );
//  }


void CXGui::MoveSize( void )
{
  XMoveResizeWindow( display, win, x,y, width, height );
}

istream& operator>>(istream& s, CXGui& w )
{
  s >> w.x >> w.y >> w.width >> w.height;
  w.MoveSize();

  return s;
}

void CXGui::DrawBox( double px, double py, double boxdelta )
{
    DPoint pts[5];
  
    pts[4].x = pts[0].x = px - boxdelta;
    pts[4].y = pts[0].y = py + boxdelta;
    pts[1].x = px + boxdelta;
    pts[1].y = py + boxdelta;
    pts[2].x = px + boxdelta;
    pts[2].y = py - boxdelta;
    pts[3].x = px - boxdelta;
    pts[3].y = py - boxdelta;
  
    DrawLines( pts, 5 );
} 

ExportData* CXGui::GetLastMatchingExport( ExportData* exp )
{
  for( int o=0; o<numObjects; o++ )     
    if( database[o].objectId == exp->objectId  )
      return &database[o]; // found it!

  return 0; // didn't find it
}

void CXGui::ImportExportData( ExportData* exp )
{
  if( exp == 0 ) return; // do nothing if the pointer is null

  // lookup the object in the database 

  ExportData* lastExp = GetLastMatchingExport( exp );

  SetDrawMode( GXxor );
  
  if( lastExp ) // we've drawn this before
    {
      if( memcmp( lastExp, exp, sizeof( ExportData ) ) != 0 )
      {
      // data has changed since the last time
	 RenderObject( lastExp ); // erase old image
	 memcpy( lastExp, exp, sizeof( ExportData ) ); // store this data
	 // do the new drawing
	 RenderObject( exp ); // draw new image
      }	  
    }
  else
    {
      cout << "\nadding a new object, type " << exp->objectType 
	   << " ID " << exp->objectId << endl;
      // add this new object
      memcpy( &database[numObjects], exp, sizeof( ExportData ) );
      numObjects++;     

      // do the new drawing
      RenderObject( exp ); // draw new image
    }
  
  SetDrawMode( GXcopy );
}

void CXGui::RenderObject( ExportData* exp )
{
  //cout << "Rendering " << exp->objectId << endl;

  // check the object type and call the appropriate render function 
  switch( exp->objectType )
    {
    case generic_o: RenderGenericObject( exp ); break;
    case pioneer_o: RenderPioneer( exp ); break;
    case uscpioneer_o: RenderUSCPioneer( exp ); break;
    case laserturret_o: RenderLaserTurret( exp ); break;
    case laserbeacon_o: RenderLaserBeacon( exp ); break;
    case box_o: RenderBox( exp ); break;
    default: printf( "XGui asked to render unknown object type\n" );
    }
}



void CXGui::RenderGenericObject( ExportData* exp )
{
  SetForeground( RGB(255,255,255) );
  DrawBox( exp->x, exp->y, 0.2 );
}

void CXGui::RenderLaserTurret( ExportData* exp )
{ 
  SetForeground( RGB(0,255,0) );
  DrawBox( exp->x, exp->y, 0.2 );

  DrawLines( (DPoint*)exp->data, 361 );
}

void CXGui::RenderPioneer( ExportData* exp )
{ 
  SetForeground( RGB(255,0,0) );

  DPoint pts[4];
  GetRect( exp->x, exp->y, exp->width/2.0, exp->height/2.0, exp->th, pts );

  DrawPolygon( pts, 4 );
}

void CXGui::RenderUSCPioneer( ExportData* exp )
{ 
  SetForeground( RGB(255,0,255) );

  DPoint pts[7];
  GetRect( exp->x, exp->y, -exp->width/2.0, exp->height/2.0, exp->th, pts );

  pts[4].x = pts[0].x;
  pts[4].y = pts[0].y;

  pts[5].x = exp->x;
  pts[5].y = exp->y;

  pts[6].x = pts[3].x;
  pts[6].y = pts[3].y;

  DrawLines( pts, 7 );
}

void CXGui::RenderPTZ( ExportData* exp )
{ 
  SetForeground( RGB(255,255,0) );
  DrawBox( exp->x, exp->y, 0.1 );
}

void CXGui::RenderBox( ExportData* exp )
{ 
  SetForeground( RGB(0,0,255) );

  DPoint pts[4];
  GetRect( exp->x, exp->y, exp->width/2.0, exp->height/2.0, exp->th, pts );

  DrawPolygon( pts, 4 );
}

void CXGui::RenderLaserBeacon( ExportData* exp )
{ 
  SetForeground( RGB(0,255,0) );
  //DrawBox( exp->x, exp->y, 0.1 );
  //DrawCircle( exp->x, exp->y, 0.2 );

  DPoint pts[4];
  GetRect( exp->x, exp->y, exp->width/2.0, exp->height/2.0, exp->th, pts );

  DrawPolygon( pts, 4 );
}

void CXGui::GetRect( double x, double y, double dx, double dy, 
		       double rotateAngle, DPoint* pts )
{ 
  double cosa = cos( rotateAngle );
  double sina = sin( rotateAngle );
  double cxcosa = dx * cosa;
  double cycosa = dy * cosa;
  double cxsina = dx * sina;
  double cysina = dy * sina;
  
  pts[0].x = x + (-cxcosa + cysina);
  pts[0].y = y + (-cxsina - cycosa);
  pts[1].x = x + (+cxcosa + cysina);
  pts[1].y = y + (+cxsina - cycosa);
  pts[3].x = x + (-cxcosa - cysina);
  pts[3].y = y + (-cxsina + cycosa);
  pts[2].x = x + (+cxcosa - cysina);
  pts[2].y = y + (+cxsina + cycosa);
}

void CXGui::HighlightObject( CObject* obj )
{
  static double x, y, th;
  //static double lx, ly, lth;

  static CObject* lastObj;

  //if( obj != lastObj )//|| lastObj == 0 )
    {
      SetForeground( white );
      XSetLineAttributes( display, gc, 0, LineOnOffDash, CapRound, JoinRound );
      SetDrawMode( GXxor );

      // undraw the last one
      if( lastObj != 0 ) 
	DrawCircle( x, y, 0.6 );
      
      obj->GetGlobalPose( x,y,th );

      DrawCircle( x,y, 0.6 );
      SetDrawMode( GXcopy );
      XSetLineAttributes( display, gc, 0, LineSolid, CapRound, JoinRound );
    }
  
  lastObj = obj;
  //  lx = x; ly = y; lth = th;
}


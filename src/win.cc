/*************************************************************************
 * win.cc - all the graphics and X management
 * RTV
 * $Id: win.cc,v 1.6.2.1 2000-12-05 23:17:34 ahoward Exp $
 ************************************************************************/

#include <stream.h>
#include <assert.h>
#include <math.h>
#include <iomanip.h>
#include <values.h>
#include <signal.h>

#include "win.h"

#include <X11/keysym.h> 
#include <X11/keysymdef.h> 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

//#define DEBUG
//#define VERBOSE

XPoint* backgroundPts;
int backgroundPtsCount;


const double TWOPI = 6.283185307;
//*** REMOVE ahoward const int numPts = SONARSAMPLES;
//*** REMOVE ahoward int laserSamples = LASERSAMPLES;

const float maxAngularError =  -0.1; // percent error on turning odometry

extern int drawMode;

unsigned int RGB( int r, int g, int b );

int coords = false;

CWorldWin::CWorldWin( CWorld* wworld, char* initFile )
{
#ifdef DEBUG
  cout << "Creating a window..." << flush;
#endif

  world = wworld;

  char* title = "Stage";

  grey = 0x00FFFFFF;

  // provide some sensible defaults for the window parameters
  xscale = yscale = 1.0;
  width = 400;
  height = 300;
  panx = pany = x = y = 0;

  LoadVars( initFile ); // read the window parameters from the initfile

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
	       ButtonPressMask | ButtonReleaseMask | KeyPressMask );
 
  XStringListToTextProperty( &title, 1, &windowName);

  iwidth = world->width;
  iheight = world->height;
  dragging = 0;
  drawMode = false;
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

int CWorldWin::LoadVars( char* filename )
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


void CWorldWin::DragRobot( void )
{  
  Window dummyroot, dummychild;
  int dummyx, dummyy, xpos, ypos;
  unsigned int dummykeys;
 
  XQueryPointer( display, win, &dummyroot, &dummychild, 
		 &dummyx, &dummyy, &xpos, &ypos, &dummykeys );
  
  dragging->oldx = dragging->x;
  dragging->oldy = dragging->y;

  dragging->x = panx + (float)xpos / xscale;
  dragging->y = pany + (float)ypos / yscale;

  dragging->MapUnDraw();
  dragging->MapDraw();
}


void CWorldWin::Draw( void ) 
{ 
  BlackBackground();
  DrawWalls();
}


void CWorldWin::PrintCoords( void )
{
  Window dummyroot, dummychild;
  int dummyx, dummyy, xpos, ypos;
  unsigned int dummykeys;
  
  XQueryPointer( display, win, &dummyroot, &dummychild, 
		 &dummyx, &dummyy, &xpos, &ypos, &dummykeys );
  
  
  cout << "Mouse: " << xpos << ',' << ypos << endl;;
}
//int firstDrag;

void CWorldWin::BoundsCheck( void )
{
  if( panx < 0 ) panx = 0;
  if( panx > ( iwidth - width ) ) panx =  iwidth - width;
  
  if( pany < 0 ) pany = 0;
  if( pany > ( iheight - height ) ) pany =  iheight - height;
  
  float xminScale = (float)width / (float)world->width;
  float yminScale = (float)height / (float)world->height;
  
  if( xscale < xminScale ) xscale = xminScale;
  if( yscale < yminScale ) yscale = yminScale;
}

void CWorldWin::HandleEvent( void )
{    
  XEvent reportEvent;
    
  int r;
  KeySym key;

  int drawnRobots = false;

  static int sw = 0, sh = 0;

  while( XCheckWindowEvent( display, win,
			    StructureNotifyMask 
			    | ButtonPressMask | ButtonReleaseMask 
			    | ExposureMask | KeyPressMask,
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
	if( reportEvent.xexpose.count == 0 )
	{
	  ScanBackground();
	  
	  BlackBackground();	  
	  DrawWalls();

	  //BlackBackgroundInRect( reportEvent.xexpose.x, 
	  // reportEvent.xexpose.y,
	  //reportEvent.xexpose.width, 
	  // reportEvent.xexpose.height );
	  
	  //DrawWallsInRect( reportEvent.xexpose.x, 
	  // reportEvent.xexpose.y,
	  // reportEvent.xexpose.width, 
	  //reportEvent.xexpose.height );
	  
	  //DrawRobotsInRect( reportEvent.xexpose.x, 
	  //	  reportEvent.xexpose.y,
	  //	  reportEvent.xexpose.width, 
	  //	  reportEvent.xexpose.height );
	  
	  // draw robots in XOR mode
	  SetDrawMode( GXxor );
	  
	  for( CRobot* r = world->bots; r; r = r->next )  r->GUIDraw();
	  
	  SetDrawMode( GXcopy );
	  
	  drawnRobots = true;
	}
	break;

      case ButtonRelease:
	break;
	
      case ButtonPress: 
	  switch( reportEvent.xbutton.button )
	    {	      
	    case Button1: 
	      if( dragging  ) 
		{ // stopped dragging
		  dragging = NULL;
		} 
	      else
		{  // find nearest robot and drag it
		  dragging = 
		    world->NearestRobot((float)reportEvent.xbutton.x / xscale 
					+ panx, 
					(float)reportEvent.xbutton.y / yscale 
					+ pany ); 
		}
	      break;
	      
	    case Button2: 
	      if( dragging )
		{
		  dragging->a -= M_PI/10.0;
		  dragging->a = fmod( dragging->a + TWOPI, TWOPI ); // normalize
		}
	      else
		{
		  CRobot* near = 
		    world->NearestRobot((float)reportEvent.xbutton.x / xscale 
					+ panx, 
					(float)reportEvent.xbutton.y / yscale 
					+ pany ); 

		  near->ToggleSensorDisplay();
		}
#ifdef DEBUG	      
	      else
		{ 

		  // this can be taken out at release
		  //useful debug stuff!
		  // dump the raw image onto the screen
		  for( int f = 0; f<world->img->width; f++ )
		    for( int g = 0; g<world->img->height; g++ )
		      {
			if( world->img->get_pixel( f,g ) == 0 )
			  XSetForeground( display, gc, white );
			else
			  XSetForeground( display, gc, black );

			XDrawPoint( display,win,gc, f, g );
		      }

		  getchar();
		  
		  Draw();

		  // draw robots in XOR mode
		  SetDrawMode( GXxor );
		  
		  for( CRobot* r = world->bots; r; r = r->next )  r->GUIDraw();
		  
		  SetDrawMode( GXcopy );
		  
		  drawnRobots = true;
		}
#endif
	      break;
	    case Button3: 
	      if( dragging )
		{
		  dragging->a += M_PI/10.0;  
		  dragging->a = fmod( dragging->a + TWOPI, TWOPI ); // normalize
		}
	      else
		{
		  panx = (int)(world->bots[0].x - width/2.0);
		  pany = (int)(world->bots[0].y - height/2.0);
		  
		  BoundsCheck();		  
		  ScanBackground();
		  Draw();
		  
		  // draw robots in XOR mode
		  SetDrawMode( GXxor );
		  
		  for( CRobot* r = world->bots; r; r = r->next )  r->GUIDraw();
		  
		  SetDrawMode( GXcopy );
		  
		  drawnRobots = true;
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

	if( panx != oldPanx || pany != oldPany )
	  {
	    ScanBackground();
	    Draw();

	    // draw robots in XOR mode
	    SetDrawMode( GXxor );

	    for( CRobot* r = world->bots; r; r = r->next )  r->GUIDraw();

	    SetDrawMode( GXcopy );
	    
	    drawnRobots = true;
	    
	  }
	break;
      }

  // if we haven't already refreshed the robots on screen, do it
  if( !drawnRobots )
    {
      // draw robots in XOR mode
      SetDrawMode( GXxor );
      
      // we've handled the X events; now we'll handle dragging modes
      if( dragging ) DragRobot();
      
      DrawRobots();
      //DrawRobotIfMoved( r );
      
      SetDrawMode( GXcopy ); // reset drawing mode
    }

  // try to sync the display to avoid flicker
  XSync( display, false );

}

void CWorldWin::ScanBackground( void )
{
#ifdef VERBOSE
  cout << "Scanning background (no scaling)... " << flush;
#endif

  if( backgroundPts ) delete[] backgroundPts;

  backgroundPts = new XPoint[width*height];
  backgroundPtsCount = 0;

  int sx, sy, px, py;
  for( px=0; px < width; px++ )
    for( py=0; py < height; py++ )
      if( world->bimg->get_pixel( px+panx, py+pany ) != 0 )
	{
	      backgroundPts[backgroundPtsCount].x = px;
	      backgroundPts[backgroundPtsCount].y = py;
	      backgroundPtsCount++;
	}
#ifdef VERBOSE
  cout << "ok." << endl;
#endif
}


void CWorldWin::BlackBackground( void )
{
    XSetForeground( display, gc, black );
    XFillRectangle( display, win, gc, 0,0, width, height );
}

void CWorldWin::BlackBackgroundInRect( int xx, int yy, int ww, int hh )
{
    XSetForeground( display, gc, black );
    XFillRectangle( display, win, gc, xx,yy, ww, hh );
}
 
void CWorldWin::DrawWalls( void )
{
  XSetForeground( display, gc, white );
  XDrawPoints( display, win, gc, 
	       backgroundPts, backgroundPtsCount, CoordModeOrigin );
}

void CWorldWin::SetDrawMode( int mode )
{
  // the modes are defined in X11/Xlib.h
  XSetFunction( display, gc, mode );
}

void CWorldWin::DrawLines( XPoint* pts, int numPts )
{
  
  // if the window isn't panned
  if( panx == 0 && pany == 0 ) 
    // draw the points as-is
    XDrawLines( display, win, gc, pts, numPts, CoordModeOrigin );
  else
    {
      // shift the points to allow for panning
      XPoint* panPts = new XPoint[ numPts ];
      
      for( int c=0; c<numPts; c++ )
	{
	  panPts[c].x = pts[c].x - panx;
	  panPts[c].y = pts[c].y - pany;
	}
      
      // and draw the shifted points
      XDrawLines( display, win, gc, panPts, numPts, CoordModeOrigin );

      delete [] panPts;
    }

}

void CWorldWin::DrawPoints( XPoint* pts, int numPts )
{
  
  // if the window isn't panned
  if( panx == 0 && pany == 0 ) 
    // draw the points as-is
    XDrawPoints( display, win, gc, pts, numPts, CoordModeOrigin );
  else
    {
      // shift the points to allow for panning
      XPoint* panPts = new XPoint[ numPts ];
      
      for( int c=0; c<numPts; c++ )
	{
	  panPts[c].x = pts[c].x - panx;
	  panPts[c].y = pts[c].y - pany;
	}
      
      // and draw the shifted points
      XDrawPoints( display, win, gc, panPts, numPts, CoordModeOrigin );

      delete [] panPts;
    }

}

/*void CWorldWin::DrawPolygon( XPoint* pts, int numPts )
{
  
  XPoint* panPts = new XPoint[ numPts ];


  // if the window isn't panned
  if( panx == 0 && pany == 0 ) 
  {  // draw the points as-is
    XDrawLines( display, win, gc, pts, numPts, CoordModeOrigin );
    XDrawLine( display, win, gc, pts[0].x, pts[0].y, 
	       pts[LASERSAMPLES-1].x, pts[LASERSAMPLES-1]. y );
  }
  else
    {
      // shift the points to allow for panning
      XPoint* panPts = new XPoint[ numPts ];
      
      for( int c=0; c<numPts; c++ )
	{
	  panPts[c].x = pts[c].x - panx;
	  panPts[c].y = pts[c].y - pany;
	}
      
      // and draw the shifted points
      XDrawPoints( display, win, gc, panPts, numPts, CoordModeOrigin );
      XDrawLine( display, win, gc, panPts[0].x, panPts[0].y, 
		 panPts[LASERSAMPLES-1].x, panPts[LASERSAMPLES-1].y );

      delete [] panPts;
    }

}*/



void CWorldWin::DrawWallsInRect( int xx, int yy, int ww, int hh )
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

void CWorldWin::DrawRobotsInRect( int xx, int yy, int ww, int hh )
{
  // do bounds checking here sometime - no big deal
  for( CRobot* r = world->bots; r; r = r->next )
    {
      DrawRobot( r );
    }
}

void CWorldWin::DrawRobot( CRobot* r )
{
  r->GUIUnDraw();
  r->GUIDraw();
}

void CWorldWin::DrawRobots( void )
{
  for( CRobot* r = world->bots; r; r = r->next )
    {
      DrawRobot( r );
    }
}

void CWorldWin::DrawRobotIfMoved( CRobot* r )
{
  if( r->HasMoved() ) DrawRobot( r );
}

unsigned long CWorldWin::RobotDrawColor( CRobot* r )
{ 
  unsigned long color;
  switch( r->channel % 6 )
    {
    case 0: color = red; break;
    case 1: color = green; break;
    case 2: color = blue; break;
    case 3: color = yellow; break;
    case 4: color = magenta; break;
    case 5: color = cyan; break;
    }

  return color;
}

void CWorldWin::SetForeground( unsigned long col )
{ 
  XSetForeground( display, gc, col );
}


void CWorldWin::DrawInRobotColor( CRobot* r )
{ 
  XSetForeground( display, gc, RobotDrawColor( r ) );
}


//  void CWorldWin::DrawSonar( CRobot* r )
//  {
//    XSetFunction( display, gc, GXxor );
//    XSetForeground( display, gc, white );
 
//    XDrawPoints( display, win, gc, r->oldHitPts, numPts, CoordModeOrigin   ); 
//    XDrawPoints( display, win, gc, r->hitPts, numPts, CoordModeOrigin ); 
  
//    memcpy( r->oldHitPts, r->hitPts, numPts * sizeof( XPoint ) );
  
//    XSetFunction( display, gc, GXcopy );
  
//    r->redrawSonar = false;  
//  }

//  void CWorldWin::DrawLaser( CRobot* r )
//  {
//    XSetFunction( display, gc, GXxor );
//    XSetForeground( display, gc, white );
  
//    XDrawPoints( display,win,gc, r->loldHitPts, laserSamples/2,CoordModeOrigin);
//    XDrawPoints( display,win,gc, r->lhitPts, laserSamples/2, CoordModeOrigin); 

//    memcpy( r->loldHitPts, r->lhitPts, laserSamples/2 * sizeof( XPoint ) );
  
//    XSetFunction( display, gc, GXcopy );
  
//    r->redrawLaser = false;
//  }


void CWorldWin::MoveSize( void )
{
  XMoveResizeWindow( display, win, x,y, width, height );
}

istream& operator>>(istream& s, CWorldWin& w )
{
  s >> w.x >> w.y >> w.width >> w.height;
  w.MoveSize();

  return s;
}









/*************************************************************************
 * win.cc - all the graphics and X management
 * RTV
 * $Id: win.cc,v 1.2 2000-11-29 22:44:49 vaughan Exp $
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
const int numPts = SONARSAMPLES;
const int laserSamples = LASERSAMPLES;

const float maxAngularError =  -0.1; // percent error on turning odometry

int boxX, boxY;

extern int drawMode;

unsigned int RGB( int r, int g, int b );

int dragBox = false;
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

  // move and redraw  
  dragging->StoreRect();
  dragging->UnDraw( world->img );
  dragging->CalculateRect();
  dragging->Draw( world->img );

  // just to make sure...
  dragging->turnRate = dragging->speed = 0.0;
  
  //cout << "col: " << (int)(dragging->color) 
  //   << " id: " << (int)(dragging->id)
  //   << " channel: " <<(int)( dragging->channel) << endl;
  
  // reset robot's odometry
  //dragging->xorigin = dragging->xodom = dragging->x;
  //dragging->yorigin = dragging->yodom = dragging->y;
  //dragging->aorigin = dragging->aodom = dragging->a;
}


void CWorldWin::Draw( void ) 
{ 
  BlackBackground();
  DrawWalls();
  
  //if( world->zc > 0 ) DrawZones();
}

//  void CWorldWin::DrawZones( void )
//  {
//   // draw the zones
//    for( int z = 0; z < world->zc; z++ )
//      {
//        XSetForeground( display, gc, red );
//        XDrawRectangle( display, win, gc, 
//  		      (int)(world->zones[z].x*world->ppm*xscale) - panx,
//  		      (int)(world->zones[z].y*world->ppm*yscale) - pany,
//  		      (int)(world->zones[z].w*world->ppm*xscale),
//  		      (int)(world->zones[z].h*world->ppm*yscale) );
//      }
//  }



void CWorldWin::PrintCoords( void )
{
  Window dummyroot, dummychild;
  int dummyx, dummyy, xpos, ypos;
  unsigned int dummykeys;
  
  XQueryPointer( display, win, &dummyroot, &dummychild, 
		 &dummyx, &dummyy, &xpos, &ypos, &dummykeys );
  
  
  cout << "Mouse: " << xpos << ',' << ypos << endl;;
}
int firstDrag;

void CWorldWin::DragBox( void )
{
  Window dummyroot, dummychild;
  int dummyx, dummyy, xpos, ypos;
  unsigned int dummykeys;

  static int oldx = 0, oldy = 0;
 
  XSetFunction( display, gc, GXxor );

  XQueryPointer( display, win, &dummyroot, &dummychild, 
		 &dummyx, &dummyy, &xpos, &ypos, &dummykeys );
 
  if( !firstDrag )
    {
      XDrawLine( display, win, gc, boxX, boxY, boxX, oldy );
      XDrawLine( display, win, gc, boxX, boxY, oldx, boxY );
      XDrawLine( display, win, gc, boxX, oldy, oldx, oldy );
      XDrawLine( display, win, gc, oldx, oldy, oldx, boxY );
    }
 
  XSetForeground( display,gc, white );

  XDrawLine( display, win, gc, boxX, boxY, boxX, ypos );
  XDrawLine( display, win, gc, boxX, boxY, xpos, boxY );
  XDrawLine( display, win, gc, boxX, ypos, xpos, ypos );
  XDrawLine( display, win, gc, xpos, ypos, xpos, boxY );

  XSetFunction( display, gc, GXcopy );

  //cout << xpos << ' ' << ypos << endl;
	
  firstDrag = false;
  oldx = xpos;
  oldy = ypos;
}

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
	ScanBackground();
	
	BlackBackgroundInRect( reportEvent.xexpose.x, 
			      reportEvent.xexpose.y,
			      reportEvent.xexpose.width, 
			       reportEvent.xexpose.height );

	DrawWallsInRect( reportEvent.xexpose.x, 
			      reportEvent.xexpose.y,
			      reportEvent.xexpose.width, 
			      reportEvent.xexpose.height );
	
	DrawRobotsInRect( reportEvent.xexpose.x, 
			  reportEvent.xexpose.y,
			  reportEvent.xexpose.width, 
			  reportEvent.xexpose.height );

	if( reportEvent.xexpose.count == 0 )
	  {
	    //DrawZones();    
	    for( CRobot* r = world->bots; r; r = r->next ) DrawRobot( r );
	  }
	break;

      case ButtonRelease:
	//  if( drawMode && (world->zc < PROXIMITY_ZONES) )
//  	  switch( reportEvent.xbutton.button )
//  	    {
//  	    case Button1:
//  	      cout << "end draw" << endl;
//  	      dragBox = false;
	      
//  	      int xpos =  (int)(reportEvent.xbutton.x / xscale) + panx;
//  	      int ypos =  (int)(reportEvent.xbutton.y / yscale) + pany;
	      
//  	      boxX += panx;
//  	      boxY += pany;

//  	      // make sure we go from top left + width + height
//  	      if( boxX > xpos )
//  		{
//  		  int swap = boxX;
//  		  boxX = xpos;
//  		  xpos = swap;
//  		}
//  	      if( boxY > ypos )
//  		{
//  		  int swap = boxY;
//  		  boxY = ypos;
//  		  ypos = swap;
//  		}
	      
//  	      //cout << boxX << ' ' << boxY << ' ' 
//  	      //<< xpos << ' ' << ypos << endl;
	      
//  	      // add the box to the zones
//  	      cout << "zc pre: " << world->zc << endl;
//  	      int z = world->zc;
	    
//  	      world->zones[z].x = (float)boxX / world->ppm;
//  	      world->zones[z].y = (float)boxY / world->ppm;
//  	      world->zones[z].w = (float)(xpos-boxX) / world->ppm;
//  	      world->zones[z].h = (float)(ypos-boxY) / world->ppm;
	      	      
//  	      world->refreshBackground = true;
	      
//  	      Draw();
	      
//  	      // increment the number of zones
//  	      world->zc++;  

//  	      cout << "zc post: " << world->zc << endl;

//  	      break;
//  	    }
	break;
	
      case ButtonPress: 
	//  if( drawMode && (world->zc < PROXIMITY_ZONES) )
//  	  {
//  	    switch( reportEvent.xbutton.button )
//  	      {
//  	      case Button1:
//  		cout << "Start draw" << endl;
//  		dragBox = true;
//  		firstDrag = true;
//  		boxX = (int)((float)reportEvent.xbutton.x / xscale);
//  		boxY = (int)((float)reportEvent.xbutton.y / yscale);
//  		break;
//  	      }
//  	  }
//  	else
	  switch( reportEvent.xbutton.button )
	    {	      
	    case Button1: 
	      if( dragging  )
		{ // stopped dragging
		  // draw the robot back into the world bitmap
		  //dragging->Draw( world->img );
		  dragging->stall = 0;

		  if( showSensors ) DrawLaser( dragging );
		  if( showSensors ) DrawSonar( dragging );

		  // move and redraw to tidy up
		  dragging->StoreRect();
		  dragging->UnDraw( world->img );
		  dragging->CalculateRect();
		  dragging->Draw( world->img );

		  dragging = NULL;

		  world->refreshBackground = true;
		  world->Draw();

		  // refresh the graphics
		  DrawWalls();
		  DrawRobots();
		} 
	      else
		{  // find nearest robot and drag it
		  dragging = 
		    world->NearestRobot((float)reportEvent.xbutton.x / xscale 
					+ panx, 
					(float)reportEvent.xbutton.y / yscale 
					+ pany ); 
		  // stop the robot moving
		  dragging->speed = 0;
		  dragging->turnRate = 0;
		  dragging->stall = 1;
		  
		  // insert a line break into the robot's trail
		  // ROBOT LOGGING
		  if( dragging->log ) *(dragging->log) << endl; 

		  // remove the robot from the world bitmap
		  dragging->UnDraw( world->img );
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
		  sleep( 3 );
		  Draw();
		  DrawRobots();
		
		}
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
		  DrawRobots();
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
	// image scaling code removed for now
	//  else if( key == XK_Prior )
//  	  {
//  	    xscale *= 0.8;
//  	    yscale *= 0.8;
//  	  }
//  	else if( key == XK_Next )
//  	  {
//  	    xscale *= 1.2;
//  	    yscale *= 1.2;
//  	  }

	BoundsCheck();

	if( panx != oldPanx || pany != oldPany )//|| xscale != oldXscale )
	  {
	    ScanBackground();
	    Draw();

	    DrawRobots();
	  }
	break;
      }

  // we've handled the X events; now we'll handle dragging modes
  if( dragging ) DragRobot();
  else if( dragBox ) DragBox();

  //now we'll redraw anything that needs it
  for( CRobot* r = world->bots; r; r = r->next )
    {
      DrawRobotIfMoved( r );

      if( showSensors )
	{
	  if( r->redrawSonar ) DrawSonar( r );
	  if( r->redrawLaser ) DrawLaser( r );
	}
    }

  // some things only need done every now and again - saves some time.
  //static int sometimes = 0;
  //if( ++sometimes > 25 )
  //{
  //  sometimes = 0;
      ////DrawZones();
  // }
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


void CWorldWin::ScanBackgroundWithScaling( void )
{
#ifdef VERBOSE
  cout << "Scanning background (with scaling)... " << flush;
#endif

  if( backgroundPts ) delete[] backgroundPts;

  backgroundPts = new XPoint[width*height];
  backgroundPtsCount = 0;

  int sx, sy, px, py;
  for( px=0; px < world->width; px++ )
    for( py=0; py < world->height; py++ )
      if( world->bimg->get_pixel( px, py ) != 0 )
	{
	  //find the pixel that must be set
	  sx = (int)((float)px * xscale) -panx;
	  sy = (int)((float)py * yscale) -pany;
	  // add this point if i've not painted it already
	    if(  !(  backgroundPts[backgroundPtsCount - 1].x == sx 
		&& backgroundPts[backgroundPtsCount -1 ].y == sy) )
	    {
	      backgroundPts[backgroundPtsCount].x = sx;
	      backgroundPts[backgroundPtsCount].y = sy;
	      backgroundPtsCount++;
	    } 
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

unsigned long CWorldWin::DrawInRobotColor( CRobot* r )
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
  XSetForeground( display, gc, color );

  return 0; // 
}


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
  for( CRobot* r = world->bots; r; r = r->next )
    {
      DrawRobot( r );
    }
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
  if( r->HasMoved() )    
    // then move it on the screen, too.     
    DrawRobot( r );
}

void CWorldWin::DrawRobot( CRobot* r )
{
  XPoint pts[7];

  // undraw the old position
  pts[4].x = pts[0].x = (short)r->oldRect.toprx -panx;
  pts[4].y = pts[0].y = (short)r->oldRect.topry -pany;
  pts[1].x = (short)r->oldRect.toplx -panx;
  pts[1].y = (short)r->oldRect.toply -pany;
  pts[6].x = pts[3].x = (short)r->oldRect.botlx -panx;
  pts[6].y = pts[3].y = (short)r->oldRect.botly -pany;
  pts[2].x = (short)r->oldRect.botrx -panx;
  pts[2].y = (short)r->oldRect.botry -pany;
  pts[5].x = (short)r->oldCenterx -panx;
  pts[5].y = (short)r->oldCentery -pany;
  
  XSetForeground( display, gc, black );
  XDrawLines( display, win, gc, pts, 7, CoordModeOrigin );
  
  DrawInRobotColor( r );

  if( r->leaveTrail ) // redraw the bottom edge for a trail
    XDrawLine( display,win,gc, pts[1].x, pts[1].y, pts[2].x, pts[2].y );
  
  // draw the new position
  pts[4].x = pts[0].x = (short)r->rect.toprx -panx;
  pts[4].y = pts[0].y = (short)r->rect.topry -pany;
  pts[1].x = (short)r->rect.toplx -panx;
  pts[1].y = (short)r->rect.toply -pany;
  pts[6].x = pts[3].x = (short)r->rect.botlx -panx;
  pts[6].y = pts[3].y = (short)r->rect.botly -pany;
  pts[2].x = (short)r->rect.botrx -panx;
  pts[2].y = (short)r->rect.botry -pany;
  pts[5].x = (short)r->centerx -panx;
  pts[5].y = (short)r->centery -pany;
      
  XDrawLines( display, win, gc, pts, 7, CoordModeOrigin );

  //if( leaveTrail )
  //XDrawLine( display, win, gc, pts[5].x, pts[5].y, 
}

void CWorldWin::DrawRobotWithScaling( CRobot* r )
{
  XPoint pts[7];

  // undraw the old position
  pts[4].x = pts[0].x = (short)(xscale * r->oldRect.toprx) -panx;
  pts[4].y = pts[0].y = (short)(yscale * r->oldRect.topry) -pany;
  pts[1].x = (short)(xscale * r->oldRect.toplx) -panx;
  pts[1].y = (short)(yscale * r->oldRect.toply) -pany;
  pts[6].x = pts[3].x = (short)(xscale * r->oldRect.botlx) -panx;
  pts[6].y = pts[3].y = (short)(yscale * r->oldRect.botly) -pany;
  pts[2].x = (short)(xscale * r->oldRect.botrx) -panx;
  pts[2].y = (short)(yscale * r->oldRect.botry) -pany;
  pts[5].x = (short)(xscale * r->oldCenterx ) -panx;
  pts[5].y = (short)(yscale * r->oldCentery ) -pany;
  
  XSetForeground( display, gc, black );
  XDrawLines( display, win, gc, pts, 7, CoordModeOrigin );
  
  // draw the new position
  pts[4].x = pts[0].x = (short)(xscale * r->rect.toprx) -panx;
  pts[4].y = pts[0].y = (short)(yscale * r->rect.topry) -pany;
  pts[1].x = (short)(xscale * r->rect.toplx) -panx;
  pts[1].y = (short)(yscale * r->rect.toply) -pany;
  pts[6].x = pts[3].x = (short)(xscale * r->rect.botlx) -panx;
  pts[6].y = pts[3].y = (short)(yscale * r->rect.botly) -pany;
  pts[2].x = (short)(xscale * r->rect.botrx) -panx;
  pts[2].y = (short)(yscale * r->rect.botry) -pany;
  pts[5].x = (short)(xscale * r->centerx ) -panx;
  pts[5].y = (short)(yscale * r->centery ) -pany;
      
  DrawInRobotColor( r );
  XDrawLines( display, win, gc, pts, 7, CoordModeOrigin );
}


void CWorldWin::DrawSonar( CRobot* r )
{
  XSetFunction( display, gc, GXxor );
  XSetForeground( display, gc, white );
 
  XDrawPoints( display, win, gc, r->oldHitPts, numPts, CoordModeOrigin   ); 
  XDrawPoints( display, win, gc, r->hitPts, numPts, CoordModeOrigin ); 
  
  memcpy( r->oldHitPts, r->hitPts, numPts * sizeof( XPoint ) );
  
  XSetFunction( display, gc, GXcopy );
  
  r->redrawSonar = false;  
}

void CWorldWin::DrawLaser( CRobot* r )
{
  XSetFunction( display, gc, GXxor );
  XSetForeground( display, gc, white );
  
  XDrawPoints( display,win,gc, r->loldHitPts, laserSamples/2,CoordModeOrigin);
  XDrawPoints( display,win,gc, r->lhitPts, laserSamples/2, CoordModeOrigin); 

  memcpy( r->loldHitPts, r->lhitPts, laserSamples/2 * sizeof( XPoint ) );
  
  XSetFunction( display, gc, GXcopy );
  
  r->redrawLaser = false;
}


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









/*************************************************************************
 * xgui.cc - all the graphics and X management
 * RTV
 * $Id: xgui.cc,v 1.5 2001-07-11 02:09:15 gerkey Exp $
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
#include "pioneermobiledevice.hh" // for pioneer_shape_t

#include <X11/keysym.h> 
#include <X11/keysymdef.h> 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

//#define DEBUG
//#define VERBOSE
//#define VERYVERBOSE
//#define LABELS

#define ULI unsigned long int
#define USI unsigned short int

// size of a static array - yuk! will make dynamic sooner or later
#define MAX_OBJECTS 10000

XPoint* backgroundPts;
int backgroundPtsCount;

bool renderSensorData = false;

//int exposed = false;

// forward declaration
unsigned int RGB( int r, int g, int b );

CXGui::CXGui( CWorld* wworld )//,  char* initFile )
{
#ifdef DEBUG
  cout << "Creating a window..." << flush;
#endif

  world = wworld;
  ppm = world->ppm; // scale between world and X coordinates

  database = new ExportData*[ MAX_OBJECTS ];
 
  numObjects = 0;

  char* title = "Stage";

  grey = 0x00FFFFFF;

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

  iwidth = world->GetBackgroundImage()->width;
  iheight = world->GetBackgroundImage()->height;
  // provide some sensible defaults for the window parameters
  //xscale = yscale = 1.0;
  width = iwidth;
  height = iheight;
 
  // limit opening size to something reasonable
  if( height > 600 ) height = 600;
  if( width > 900 ) width = 900;

  panx = 0;
  pany = height; // pan to have the origin in the bottom left

  win = XCreateSimpleWindow( display, 
			     RootWindow( display, screen ), 
			     x, y, width, height, 4, 
			     black, white );
  
  XSelectInput(display, win, ExposureMask | StructureNotifyMask | 
	       ButtonPressMask | ButtonReleaseMask | KeyPressMask );
  
  XStringListToTextProperty( &title, 1, &windowName);


  dragging = near = (ExportData*) NULL;
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
  
  gc = XCreateGC(display, win, (ULI)0, NULL );
  

  BoundsCheck();


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

// destructor
CXGui::~CXGui( void )
{
  delete [] database;
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


void CXGui::MoveObject( ExportData* exp, double x, double y, double theta )
{    
  ImportData imp;
  
  imp.x = x; 
  imp.y = y; 
  imp.th = theta;
  
  // update the object through its pointer
  (CEntity*)(exp->objectId)->ImportExportData( &imp );
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



void CXGui::BoundsCheck( void )
{
  if( panx < 0 ) panx = 0;
  if( panx > ( iwidth - width ) ) panx =  iwidth - width;
  
  if( pany < 0 ) pany = 0;
  if( pany > ( iheight - height ) ) pany =  iheight - height;
}


void CXGui::HandleEvent( void )
{    
  XEvent reportEvent;
  KeySym key;
  
  // used for inverting Y between world and X coordinates
  int imgHeight= world->GetBackgroundImage()->height;
  
  while( XCheckWindowEvent( display, win,
			    StructureNotifyMask 
			    | ButtonPressMask | ButtonReleaseMask 
			    | ExposureMask | KeyPressMask 
			    | PointerMotionHintMask, 
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
	}
	break;

      case MotionNotify:	
	if( dragging )
	  {
	    MoveObject( dragging, 
			(reportEvent.xmotion.x + panx ) / ppm,
			((imgHeight-reportEvent.xmotion.y) - pany ) / ppm,
			dragging->th );
	    
	    HighlightObject( dragging, true );
	  }
	break;
	
      case ButtonRelease:
	//if( near ) HighlightObject( near );
	//if( near ) near = 0;
	break;
	
      case ButtonPress: 
	  switch( reportEvent.xbutton.button )
	    {	      
	    case Button1: 
	      if( dragging  ) 
  		{ // stopped dragging
  		  dragging = NULL;
		  near = NULL;

		  HighlightObject( NULL, true ); // erases the highlighting
	
		  // disable the pointer motion events
		  XSelectInput(display, win, 
			       ExposureMask | StructureNotifyMask | 
			       ButtonPressMask | ButtonReleaseMask | 
			       KeyPressMask );
  		} 
  	      else 
  		{  
		  if( near )
		    dragging = near;
		  else
		    {
		      // find nearest robot and drag it
		      dragging = 
			NearestObject((reportEvent.xmotion.x+panx )/ppm,
				      ((imgHeight-reportEvent.xmotion.y)
				       -pany)/ppm );
		      
		      // unless i was holding down the shift key

		      // but only if it is moveable - ie. it is 
		      // a parentless object		
		      CEntity* ent = (CEntity*)dragging->objectId;
		      while( ent->m_parent_object )
			ent = ent->m_parent_object;
		      
		      dragging = 0;
		      
		      // get the export structure for this entity
		      for( int o=0; o<numObjects; o++ )
			if( database[o]->objectId == ent )
			  {
			    dragging = database[o];
			    break;
			  }
		    }
		  
		  if( !dragging ) 
		    cout << "Warning! dragging ptr: parent not found!" << endl;
		  else
		      HighlightObject( dragging, true );

		  // enable generation of pointer motion events
		  XSelectInput(display, win, 
			       ExposureMask | StructureNotifyMask | 
			       ButtonPressMask | ButtonReleaseMask | 
			       KeyPressMask | PointerMotionMask );
		}	      
	      break;
	      
	    case Button2: 
	      if( dragging )
		{	  
		  MoveObject( dragging, dragging->x, dragging->y, 
			      dragging->th + M_PI/10.0 );
		}
	      else
		{
		  //  CEntity* ent = 
//  		    world->NearestObject((reportEvent.xmotion.x+panx )/ppm,
//  					 ((imgHeight-reportEvent.xmotion.y)
//  					  -pany)/ppm );
		  
//  		  ent->ToggleGuiExport();

		  RefreshObjects();
		  renderSensorData = !renderSensorData;
		  RefreshObjects();
		}  
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
		  MoveObject( dragging, dragging->x, dragging->y, 
			      dragging->th - M_PI/10.0 );
		}	      
	      else
		{ 
		  near = 
		    NearestObject((reportEvent.xmotion.x+panx )/ppm,
				  ((imgHeight-reportEvent.xmotion.y)
				   -pany)/ppm );
		  
		  HighlightObject( near, true );
		  
		  // enable generation of pointer motion events
		  XSelectInput(display, win, 
			       ExposureMask | StructureNotifyMask | 
			       ButtonPressMask | ButtonReleaseMask | 
			       KeyPressMask | PointerMotionMask );
		  // we're centering on robot[0] - redraw everything
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
	//double oldXscale = xscale;

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
	  }

	// check for LOAD and SAVE key commands
	if( key == XK_l || key == XK_L )
	{
	  cout << "LOAD not implemented!" << endl;
	  //world->Load();
	  //RefreshObjects();
	}

	if( key == XK_s || key == XK_S )
	{
	  //cout << "SAVE" << endl;
	  world->Save();
	}

  
	break;
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

  //int sx, sy,
    int px, py;
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


void CXGui::DrawString( double x, double y, char* str, int len )
{ 
  if( str )
    {
      int imgHeight= world->GetBackgroundImage()->height;
      
      XDrawString( display,win,gc, 
		   (ULI)(x * ppm - panx),
		   (ULI)( (imgHeight-pany) - ((int)( y*ppm ))),
		   str, len );
    }
  else
    cout << "Warning: XGUI::DrawString was passed a null pointer" << endl;
}

void CXGui::DrawLines( DPoint* dpts, int numPts )
{
  //SetDrawMode( GXcopy );
  int imgHeight= world->GetBackgroundImage()->height;

  XPoint* xpts = new XPoint[numPts];
  
  // scale, pan and quantize the points
  for( int l=0; l<numPts; l++ )
    {
      xpts[l].x = ((int)( dpts[l].x * ppm )) - panx;
      xpts[l].y = (imgHeight-pany) - ((int)( dpts[l].y * ppm ));
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
      xpts[l].x = ((int)( dpts[l].x * ppm )) - panx;
      xpts[l].y = (imgHeight-pany) - ((int)( dpts[l].y * ppm ));
    }
 
  // and draw them
  XDrawPoints( display, win, gc, xpts, numPts, CoordModeOrigin );
   
  delete [] xpts; 
}

void CXGui::DrawCircle( double x, double y, double r )
{
  int imgHeight= world->GetBackgroundImage()->height;

  XDrawArc( display, win, gc, 
	    (ULI)((x-r) * ppm -panx), 
	    (ULI)((imgHeight-pany)-((y+r) * ppm)), 
	    (ULI)(2.0*r*ppm), (ULI)(2.0*r*ppm), 0, 23040 );
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
    RenderObject( database[o], renderSensorData );
  
  HighlightObject( dragging, false );
  HighlightObject( near, false );

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
    if( database[o]->objectId == exp->objectId  )
      return database[o]; // found it!

  return 0; // didn't find it
}

size_t CXGui::DataSize( ExportData* exp )
{
  size_t sz = 0;

  // check the object type to find the size of the type-specific data
  switch( exp->objectType )
    {
    case laserturret_o: sz = sizeof( ExportLaserData );
      break;
    case misc_o: sz = sizeof( player_misc_data_t );
      break;
    case sonar_o: sz = sizeof( ExportSonarData );
      break;
    case laserbeacondetector_o:  
      sz = sizeof( ExportLaserBeaconDetectorData );
      break;
    case ptz_o: sz = sizeof( ExportPtzData );
      break;  
    case vision_o: break;
    case uscpioneer_o: break;
    case pioneer_o: break;
    case laserbeacon_o: break;
    case box_o: break;
    case player_o: break;
    case puck_o: break;
    }	
  
  return sz;
}


void CXGui::ImportExportData( ExportData* exp )
{
  if( exp == 0 ) return; // do nothing if the pointer is null
  
  // lookup the object in the database 
  
  ExportData* lastExp = GetLastMatchingExport( exp );
  
  SetDrawMode( GXxor );

  if( lastExp ) // we've drawn this before
    {	  
      bool renderExtended = renderSensorData;
      bool genericChanged = false;
      bool specificChanged = false;
      
      // see if the generic object data has changed
      // i.e. the object has moved or changed type
      if( IsDiffGenericExportData( lastExp, exp ) )
	genericChanged = true;
       
      // see if the type-specific data has changed
      // i.e. the sensor data has been updated
      if( IsDiffSpecificExportData( lastExp, exp ) )
	specificChanged = true;
      else
	// we don't need to render the type-specific data
	renderExtended = false;
      
      if( genericChanged || specificChanged )
	{
	  RenderObject( lastExp, renderExtended ); // undraw
	  
	  // update the database entry with imported data
	  if( specificChanged ) CopySpecificExportData( lastExp, exp );
	  if( genericChanged )  CopyGenericExportData( lastExp, exp );	  
	  
	  RenderObject( lastExp, renderExtended ); // draw
	  if( lastExp == dragging ) HighlightObject( dragging, true );
	  if( lastExp == near ) HighlightObject( near, true );
	}
    }
  else // we haven't seen this before - add it to the database
    {

#ifdef VERYVERBOSE
      char buf[30];
      GetObjectType( exp, buf, 29 );
      cout << "Adding " << exp->objectId << " " << buf <<  endl;
#endif

      database[numObjects] = new ExportData();
      memcpy( database[numObjects], exp, sizeof( ExportData ) );
      
      // check the object type and create the type-specific data
      switch( exp->objectType )
	{
	case laserturret_o: 
	  database[numObjects]->data = (char*)new ExportLaserData();
	  CopySpecificExportData( database[numObjects], exp );
	  break;
	case misc_o:	
	  database[numObjects]->data = (char*)new player_misc_data_t();
	  CopySpecificExportData( database[numObjects], exp );
	  break;
	case sonar_o: 	
	  database[numObjects]->data = (char*)new ExportSonarData();
	  CopySpecificExportData( database[numObjects], exp );
	  break;
	case laserbeacondetector_o: 
	  database[numObjects]->data = 
	    (char*)new ExportLaserBeaconDetectorData();
	  CopySpecificExportData( database[numObjects], exp );
	  break;
	case vision_o: break;
	case ptz_o: 	 
	  database[numObjects]->data = (char*)new ExportPtzData();
	  CopySpecificExportData( database[numObjects], exp );
	  break;  
	case uscpioneer_o: break;
	case pioneer_o: break;
	case laserbeacon_o: break;
	case puck_o: break;
	case box_o: break;
	case player_o: break;
	  
	}
      numObjects++; 
      
      // do the new drawing
      RenderObject( exp, true ); // draw new image
    }

  SetDrawMode( GXcopy );
}

void CXGui::RenderObject( ExportData* exp, bool extended )
{
#ifdef VERYVERBOSE
  char buf[30];
  GetObjectType( exp, buf, 29 );
  cout << "Rendering " << exp->objectId << " " << buf <<  endl;
#endif

  // check the object type and call the appropriate render function 
  switch( exp->objectType )
    {
     case pioneer_o: RenderPioneer( exp, extended ); break;
      case uscpioneer_o: RenderUSCPioneer( exp, extended ); break;
      case laserturret_o: RenderLaserTurret( exp, extended ); break;
      case laserbeacon_o: RenderLaserBeacon( exp, extended ); break;
      case puck_o: RenderPuck( exp, extended ); break;
      case box_o: RenderBox( exp, extended ); break;
      case player_o: RenderPlayer( exp, extended ); break;
      case misc_o: RenderMisc( exp , extended); break;
      case sonar_o: RenderSonar( exp, extended ); break;
      case laserbeacondetector_o: 
        RenderLaserBeaconDetector( exp, extended ); break;
      case vision_o: RenderVision( exp, extended ); break;
      case ptz_o: RenderPTZ( exp, extended ); break;  
      case gripper_o: RenderGripper( exp, extended ); break;  
      
      case generic_o: RenderGenericObject( exp ); 
        cout << "Warning: GUI asked to render a GENERIC object" << endl;
        break;
    default: cout << "XGui: unknown object type " << exp->objectType << endl;
    }
}

void CXGui::RenderObjectLabel( ExportData* exp, char* str, int len )
{
  SetForeground( RGB(255,255,255) );
  DrawString( exp->x + 0.6, exp->y - (8.0/ppm + exp->objectType * 11.0/ppm) , str, len );
}

void CXGui::RenderGenericObject( ExportData* exp )
{
#ifdef LABELS
  RenderObjectLabel( exp, "Generic object", 14 );
#endif

  SetForeground( RGB(255,255,255) );
  DrawBox( exp->x, exp->y, 0.9 );

  char buf[64];

  sprintf( buf, "Generic @ %d", (int)(exp->objectId) );
  RenderObjectLabel( exp, buf, strlen(buf) );
}

//void CXGui::RenderPlayer

void CXGui::RenderLaserTurret( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "SICK LMS200", 11 );
#endif

  SetForeground( RGB(0,0,255) );

  DPoint pts[4];
  GetRect( exp->x, exp->y, 0.1, 0.1, exp->th, pts );
  DrawPolygon( pts, 4 );

  //cout << "trying to render " << exp->hitCount << " points" << endl;

  if( extended && exp->data )
    {
      SetForeground( RGB(70,70,70) );
      ExportLaserData* ld = (ExportLaserData*) exp->data;
      DrawPolygon( ld->hitPts, ld->hitCount );
    }
}

void CXGui::RenderLaserBeaconDetector( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Detector", 8 );
#endif
  
  if( extended && exp->data )
    {
      DPoint pts[4];
      char buf[20];
      
      SetForeground( RGB( 255, 0, 255 ) );
      
      ExportLaserBeaconDetectorData* p = 
	(ExportLaserBeaconDetectorData*)exp->data;
      
      if( p->beaconCount > 0 ) for( int b=0; b < p->beaconCount; b ++ )
	{
	  GetRect( p->beacons[ b ].x, p->beacons[ b ].y, 
		   (4.0/ppm), 0.12 + (2.0/ppm), p->beacons[ b ].th, pts );
	  
	  // don't close the rectangle 
	  // so the open box visually indicates heading of the beacon
	  DrawLines( pts, 4 ); 
	  
	  //DrawLines( &(pts[1]), 2 ); 

	  // render the beacon id as text
	  sprintf( buf, "%d", p->beacons[ b ].id );
	  DrawString( p->beacons[ b ].x + 0.2 + (5.0/ppm), 
		      p->beacons[ b ].y - (4.0/ppm), 
		      buf, strlen( buf ) );
	}
    }
}

void CXGui::RenderPTZ( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "PTZ Camera", 10 );
#endif

  SetForeground( RGB(200,200,200 ) );

  DPoint pts[4];
  GetRect( exp->x, exp->y, 0.05, 0.05, exp->th, pts );
  DrawPolygon( pts, 4 );
  
  
  if( 0 )
  //if( extended && exp->data ) 
    {

      ExportPtzData* p = (ExportPtzData*)exp->data;
      
      //DPoint pts[4];
      //GetRect( exp->x, exp->y, 0.4, 0.4, exp->th, pts );
      //DrawPolygon( pts, 4 );
      
      double len = 0.2; // meters
      // have to figure out the zoom with Andrew
      // for now i have a fixed width triangle
      //double startAngle =  exp->th + p->pan - p->zoom/2.0;
      //double stopAngle  =  startAngle + p->zoom;
      double startAngle =  exp->th + p->pan - DTOR( 30 );
      double stopAngle  =  startAngle + DTOR( 60 );
      
      pts[0].x = exp->x;
      pts[0].y = exp->y;
      
      pts[1].x = exp->x + len * cos( startAngle );  
      pts[1].y = exp->y + len * sin( startAngle );  
      
      pts[2].x = exp->x + len * cos( stopAngle );  
      pts[2].y = exp->y + len * sin( stopAngle );  
      
      DrawPolygon( pts, 3 );
    }
}

void CXGui::RenderSonar( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Sonar", 5 );
#endif
  if( extended && exp->data )
    {
      ExportSonarData* p = (ExportSonarData*)exp->data;
      SetForeground( RGB(70,70,70) );
      DrawPolygon( p->hitPts, p->hitCount );
    }
}

void CXGui::RenderVision( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Vision", 6 );
#endif
}

void CXGui::RenderPlayer( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Player", 6 );
#endif

  SetForeground( RGB(255,0,0) );
  DrawString( exp->x, exp->y, "P", 1 );
}

void CXGui::RenderMisc( ExportData* exp, bool extended )
{
#ifdef LABELS
  RenderObjectLabel( exp, "Misc", 4 );
#endif
 
  if( 0 ) // disable for now
  //if( extended && exp->data )
    {
      SetForeground( RGB(255,0,0) ); // red to match Pioneer base
      
      char buf[20];
      
      player_misc_data_t* p = (player_misc_data_t*)exp->data;
      
      // support bumpers eventually
      //p->rearbumpers;
      //p->frontbumpers;
      
      float v = p->voltage / 10.0;
      sprintf( buf, "%.1fV", v );
      DrawString( exp->x + 1.0, exp->y + 1.0, buf, strlen( buf ) ); 
    }
}


void CXGui::RenderPioneer( ExportData* exp, bool extended )
{
#ifdef LABELS
  RenderObjectLabel( exp, "Pioneer base", 12 );
#endif
  
  //SetForeground( RGB(255,0,0) ); // red Pioneer base color
  SetForeground( RGB(255,100,100) ); // red Pioneer base color

  //CPioneerMobileDevice* pioneer = (CPioneerMobileDevice*)(exp->objectId);
  ExportPositionData* pioneer_data = (ExportPositionData*)(exp->data);
  pioneer_shape_t shape = (pioneer_shape_t)(pioneer_data->shape);
  
  if(shape == rectangle)
  {
    DPoint pts[7];
    GetRect( exp->x, exp->y, exp->width/2.0, exp->height/2.0, exp->th, pts );

    pts[4].x = pts[0].x;
    pts[4].y = pts[0].y;

    pts[5].x = exp->x;
    pts[5].y = exp->y;

    pts[6].x = pts[3].x;
    pts[6].y = pts[3].y;

    DrawLines( pts, 7 );
  }
  else if(shape == circle)
  {
    DPoint pts[3];

    double radius = pioneer_data->radius;
    
    // draw the chassis
    DrawCircle(exp->x, exp->y, radius);
    
    // Draw the direction indicator
    //

    pts[0].x = exp->x + radius * cos(exp->th + M_PI/4.0);
    pts[0].y = exp->y + radius * sin(exp->th + M_PI/4.0);

    pts[1].x = exp->x;
    pts[1].y = exp->y;

    pts[2].x = exp->x + radius * cos(exp->th - M_PI/4.0);
    pts[2].y = exp->y + radius * sin(exp->th - M_PI/4.0);

    DrawLines(pts,3);
  }
  else
    PRINT_MSG("CXGui::RenderPioneer(): unknown shape!");
}

void CXGui::RenderUSCPioneer( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "USC Pioneer", 11 );
#endif
}

void CXGui::RenderBox( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Box", 3 );
#endif  
  SetForeground( RGB(200,200,200) );

  DPoint pts[4];
  GetRect( exp->x, exp->y, exp->width/2.0, exp->height/2.0, exp->th, pts );

  DrawPolygon( pts, 4 );
}

void CXGui::RenderLaserBeacon( ExportData* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Beacon", 6 );
#endif
  
  SetForeground( RGB(0,255,0) );
  //DrawBox( exp->x, exp->y, 0.1 );
  //DrawCircle( exp->x, exp->y, 0.2 );

  DPoint pts[4];
  GetRect( exp->x, exp->y, exp->width/2.0, exp->height/2.0, exp->th, pts );

  DrawPolygon( pts, 4 );
}

void CXGui::RenderGripper( ExportData* exp, bool extended )
{
  /*
  ExportGripperData* expGripper = (ExportGripperData*)(exp->data);
  SetForeground(RGB(0, 255, 0));
  
  // Draw the gripper
  DPoint pts[4];
  GetRect( exp->x, exp->y, exp->width/2.0, exp->height/2.0, exp->th, pts );
  DrawPolygon( pts, 4 );
  
  printf("paddles_open: %d\n", expGripper->paddles_open);
  // Draw the paddles
  if(expGripper->paddles_open)
  {
    GetRect( exp->x+(exp->width/2.0)+(expGripper->paddle_width/2.0), 
             exp->y+(exp->height/2.0)-(expGripper->paddle_height/2.0), 
             expGripper->paddle_width/2.0, 
             expGripper->paddle_height/2.0, 
             exp->th, pts );
    DrawPolygon( pts, 4 );

    GetRect( exp->x+(exp->width/2.0)+(expGripper->paddle_width/2.0), 
             exp->y-(exp->height/2.0)+(expGripper->paddle_height/2.0), 
             expGripper->paddle_width/2.0, 
             expGripper->paddle_height/2.0, 
             exp->th, pts );
    DrawPolygon( pts, 4 );
  }
  else
  {
    GetRect( exp->x+(exp->width/2.0)+(expGripper->paddle_width/2.0), 
             exp->y+(expGripper->paddle_height/2.0), 
             expGripper->paddle_width/2.0, 
             expGripper->paddle_height/2.0, 
             exp->th, pts );
    DrawPolygon( pts, 4 );

    GetRect( exp->x+(exp->width/2.0)+(expGripper->paddle_width/2.0), 
             exp->y-(expGripper->paddle_height/2.0), 
             expGripper->paddle_width/2.0, 
             expGripper->paddle_height/2.0, 
             exp->th, pts );
    DrawPolygon( pts, 4 );
  }
  */
}

void CXGui::RenderPuck( ExportData* exp, bool extended )
{
#ifdef LABELS
  RenderObjectLabel( exp, "Puck", 4 );
#endif
  
  SetForeground( RGB(0,0,255) );
  //DrawBox( exp->x, exp->y, 0.1 );
  DrawCircle( exp->x, exp->y, (exp->width/2.0));

  //DPoint pts[4];
  //GetRect( exp->x, exp->y, exp->width/2.0, exp->height/2.0, exp->th, pts );

  //DrawPolygon( pts, 4 );
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
  
  pts[2].x = x + (-cxcosa + cysina);
  pts[2].y = y + (-cxsina - cycosa);
  pts[3].x = x + (+cxcosa + cysina);
  pts[3].y = y + (+cxsina - cycosa);
  pts[1].x = x + (-cxcosa - cysina);
  pts[1].y = y + (-cxsina + cycosa);
  pts[0].x = x + (+cxcosa - cysina);
  pts[0].y = y + (+cxsina + cycosa);
}

void CXGui::HighlightObject( ExportData* exp, bool undraw )
{
  static double x = -1000.0, y = -1000.0, r = -1000.0, th = -1000.0;
  static char label[LABELSIZE];
  static char info[50];
  
  // setup the GC 
  XSetLineAttributes( display, gc, 0, LineOnOffDash, CapRound, JoinRound );
  SetForeground( white );
  SetDrawMode( GXxor );
  
  double xoffset = 0.5 + ( 5.0 / ppm );
  double yoffset = 0.0 / ppm;
  double yincrement = -12.0 / ppm;

  // undraw the last stuff if there was any
  if( x != -1000.0 && y != -1000.0 && undraw )
    {
      DrawCircle( x, y, r );
      if( label ) 
	DrawString( x + xoffset, y + yoffset, label, strlen( label ) );
      if( info[0] ) 	  
	DrawString( x + xoffset, y + (yoffset + yincrement), info, strlen( info ) );

    }
  if( exp ) // if this is a valid object
    {
      x = exp->x;
      y = exp->y;
      r = 0.5;
      th = exp->th;
    
      DrawCircle( x,y, r ); // and highlight it

      if( exp->label )
      	{
	  strcpy( label, exp->label );
	  DrawString( x + xoffset, y + yoffset, label, strlen( label ) );
      	}

      if( renderSensorData )
	{
	  sprintf( info, "(%.2f, %.2f, %.2f)", exp->x, exp->y, exp->th );
	  DrawString( x + xoffset, y + (yoffset + yincrement), info, strlen( info ) );
	}
      else
	info[0] = 0; //null string
    }
  else
    x = y = th = -1000.0; //reset the bouding box
  
  // reset the GC
  SetDrawMode( GXcopy );
  XSetLineAttributes( display, gc, 0, LineSolid, CapRound, JoinRound );
}

bool CXGui::GetObjectType( ExportData* exp, char* buf, size_t maxlen )
{
  char* str = 0;

  switch( exp->objectType )
    {
    case laserturret_o: str = "laser turret"; break;
    case misc_o:  str = "misc"; break;
    case sonar_o:  str = "sonar"; break;
    case laserbeacondetector_o:  str = "laser beacon detector"; break;
    case ptz_o:  str = "PTZ camera"; break;
    case vision_o:  str = "vision"; break;
    case uscpioneer_o:  str = "USC pioneer"; break;
    case pioneer_o:  str = "pioneer base"; break;
    case laserbeacon_o: str = "laser beacon"; break;
    case box_o:  str = "box"; break;
    case player_o:  str = "Player"; break;
    }	
  
  if( str == 0 ) return( false ); // didn't find the type
  if( strlen( str ) > maxlen ) return( false ); // not enough space

  strcpy( buf, str ); // fill in the string
  
  return( true );
}

bool CXGui::IsDiffGenericExportData( ExportData* exp1, ExportData* exp2 )
{
  if( exp1->objectType != exp2->objectType ) return true;
  if( exp1->objectId != exp2->objectId ) return true;
  if( exp1->x != exp2->x ) return true;
  if( exp1->y != exp2->y ) return true;
  if( exp1->th != exp2->th ) return true;
  if( exp1->width != exp2->width ) return true;
  if( exp1->height != exp2->height ) return true;

  // else
  return false;
}

bool CXGui::IsDiffSpecificExportData( ExportData* exp1, ExportData* exp2 )
{
  if( exp1 == exp2 ) // same data! 
    {
      cout << "same ptr!" << endl;
      return false; 
    }
  if( exp1 == 0 || exp2 == 0 ) // invalid pointer!
    {
      cout << "invalid ptr!" << endl;
      return true;
    }

  if( memcmp( exp1->data, exp2->data, DataSize( exp2 ) ) != 0) 
    return true; // different data!
  // else
  return false; // same data!
}

void CXGui::CopyGenericExportData( ExportData* dest, ExportData* src )
{
  dest->objectType = src->objectType;
  dest->objectId = src->objectId;
  dest->x = src->x;
  dest->y = src->y;
  dest->th = src->th;
  dest->width = src->width;
  dest->height = src->height;
}

void CXGui::CopySpecificExportData( ExportData* dest, ExportData* src )
{
  if( src && dest ) // if the src and dest are good
    {
      if( src->data ) // if there is data 
	memcpy( dest->data, src->data, DataSize( src ) ); // copy the data
      else
	bzero( dest->data, DataSize( src ) ); // zero the destination data
    }
  else
    cout << "warning: attempt to import data passed null pointer" << endl;
}


ExportData* CXGui::NearestObject( double x, double y )
{ 
  double dist, nearDist = 9999999.9;
  ExportData* nearest = 0;

  for( int o=0; o<numObjects; o++ )
    {
      dist = hypot( x - database[o]->x, y - database[o]->y );

      if( dist < nearDist ) 
	{
	  nearDist = dist;
	  nearest = database[o];
	}
    }
  return nearest; // didn't find it
}

  



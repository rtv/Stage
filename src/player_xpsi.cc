/*************************************************************************
 * xgui.cc - all the graphics and X management
 * RTV
 * $Id: player_xpsi.cc,v 1.1 2001-08-09 08:00:10 vaughan Exp $
 ************************************************************************/

#include <X11/keysym.h> 
#include <X11/keysymdef.h> 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <sys/time.h>
#include <iostream.h>
#include <values.h>
#include <stream.h>
#include <assert.h>
#include <math.h>
#include <iomanip.h>
#include <values.h>
#include <signal.h>
#include <fstream.h>
#include <iostream.h>

#include "xpsi.hh"

#define MIN_WINDOW_WIDTH 200
#define MIN_WINDOW_HEIGHT 100

#define MAX_WINDOW_WIDTH 1000
#define MAX_WINDOW_HEIGHT 700


#define DEBUG
//ndef DEBUG

//#define LABELS

#include <stdio.h>
#include <stdlib.h>  // for atoi(3)
#include <playerclient.h>  // for player client stuff
//#include <truthproxy.h>

#include <string.h> /* for strcpy() */

//typedef void (*callback_t)(truth_t*);

#define USAGE  "USAGE: xpsi [-h <host>] [-p <port>]\n       -h <host>: connect to Player on this host\n       -p <port>: connect to Player on this TCP port\n" 

char host[256] = "localhost";
int port = PLAYER_PORTNUM;

CXGui *win = 0; // global pointer to the CXGui object created in main()
                // we need this in the callback function below 

void RenderObjectWrapper( truth_t* truth )
{
  if( win ) win->RenderObject( truth );
}

void RenderOccupancyGridWrapper( void )
{
  if( win ) win->RenderOccupancyGrid();
}

/* easy little command line argument parser */
void parse_args(int argc, char** argv)
{
  int i;

  i=1;
  while(i<argc)
  {
    if(!strcmp(argv[i],"-h"))
    {
      if(++i<argc)
        strcpy(host,argv[i]);
      else
      {
        puts(USAGE);
        exit(1);
      }
    }
    else if(!strcmp(argv[i],"-p"))
    {
      if(++i<argc)
        port = atoi(argv[i]);
      else
      {
        puts(USAGE);
        exit(1);
      }
    }
    else
    {
      puts(USAGE);
      exit(1);
    }
    i++;
  }
}


int main(int argc, char **argv)
{
  // parse out the required port and host 
  parse_args(argc,argv); 
  
  // connect to Player
  PlayerClient pclient(host,port);

  // create the truth and occupancy proxies
  TruthProxy tp( &pclient, 0, 'a' ); // read/write
  OccupancyProxy op( &pclient, 0, 'r' ); // read-only

  // establish callback functions 
 
  // these functions are called immediately before and after the proxy
  // receives new data.  they test the window pointer, so they don't
  // actually do anything until a window is created
  tp.AttachPreUpdateCallback( &RenderObjectWrapper ); //undraw in window
  tp.AttachPostUpdateCallback( &RenderObjectWrapper ); //draw in window

  // this callback draws any updated background pixels
  op.AttachPostUpdateCallback( &RenderOccupancyGridWrapper ); //draw in window

  if( pclient.Read()) // make sure the read works before we create the window
    {
      printf( "%s: Failed to communicate with Player on port %d. Quitting.\n",
	      argv[0], port );
      exit(1);
    }

  // now we've done a read, we make a window that inspects the proxies
  // to find out the size of the world and all that.
  win = new CXGui( &tp, &op ); 

  assert( win ); // make sure the window was created

  // we're set up, so go into the read/handle loop
  for(;;)
  {
    /* this blocks until new data comes; 10Hz by default */
    if( pclient.Read() ) 
      exit(1);

    win->HandleEvent();
  }
}


//#define LABELS

  #define ULI unsigned long int
  #define USI unsigned short int

//  XPoint* backgroundPts;
//  int backgroundPtsCount;

//  bool renderSensorData = false;

//  //int exposed = false;

//  // forward declaration
//  unsigned int RGB( int r, int g, int b );


int display, screen;

CXGui::CXGui( TruthProxy* atp, OccupancyProxy* aop )
{
#ifdef DEBUG
  cout << "Creating a window..." << flush;
#endif

  dragging = 0;
  
  tp = atp; // record the TruthProxy ptr
  op = aop;

  ppm = op->ppm; 

  char* title = "X Player/Stage Interface v0.1";

  grey = 0x00FFFFFF;

//    //LoadVars( initFile ); // read the window parameters from the initfile

//    // init X globals 
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

    // provide some sensible defaults for the window parameters
    x = y = 0; 
    // the size of the whole world
    width = iwidth = op->width;
    height = iheight = op->height;
    
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

    XSizeHints sz;

    sz.flags = PMaxSize | PMinSize;// | PAspect;
    sz.min_width = MIN_WINDOW_WIDTH; 
    sz.max_width = iwidth;
    sz.min_height = MIN_WINDOW_HEIGHT;
    sz.max_height = iheight;
    //sz.min_aspect.x = iwidth;
    //sz.min_aspect.y = iheight;
    //sz.max_aspect.x = iwidth;
    //sz.max_aspect.y = iheight;

    XSetWMProperties(display, win, &windowName, (XTextProperty*)NULL, 
  		   (char**)NULL, 0, 
  		   &sz, (XWMHints*)NULL, (XClassHint*)NULL );
  
    gc = XCreateGC(display, win, (ULI)0, NULL );
  

    BoundsCheck();

    XMapWindow(display, win);
    //Size();

    printf( "post-map ppm: %g\n", ppm );

    // where did the WM put us?
    XWindowAttributes attrib;
    XGetWindowAttributes( display, win, &attrib );
    
    x = attrib.x;
    y = attrib.y;
    width = attrib.width;
    height = attrib.height;

    if( attrib.depth != 32 )
      printf( "Warning: color depth is %d bits. Colors will not be "
	      "drawn correctly\n", attrib.depth );

    
#ifdef DEBUG
    cout << " Window coords: " << x << '+' << y << ':' 
	 << width << 'x' << height << " ok." << endl;
#endif
  }


CXGui::~CXGui( void )
{
  // empty destructor
}


//  int CXGui::LoadVars( char* filename )
//  {
  
//  #ifdef DEBUG
//    cout << "Loading window parameters from " << filename << "... " << flush;
//  #endif

//    ifstream init( filename );
//    char buffer[255];
//    char c;

//    if( !init )
//      {
//        cout << "FAILED" << endl;
//        return -1;
//      }
  
//    char token[50];
//    char value[200];
  
//    while( !init.eof() )
//      {
//        init.get( buffer, 255, '\n' );
//        init.get( c );
    
//        sscanf( buffer, "%s = %s", token, value );
      
//  #ifdef DEBUG      
//        printf( "token: %s value: %s.\n", token, value );
//        fflush( stdout );
//  #endif
      
//        if ( strcmp( token, "window_x_pos" ) == 0 )
//  	x = strtol( value, NULL, 10 );
//        else if ( strcmp( token, "window_y_pos" ) == 0 )
//  	y = strtol( value, NULL, 10 );
//        else if ( strcmp( token, "window_width" ) == 0 )
//  	width = strtol( value, NULL, 10 );
//        else if ( strcmp( token, "window_height" ) == 0 )
//  	height = strtol( value, NULL, 10 );
//        else if ( (strcmp( token, "window_xscroll" ) == 0) 
//  		|| (strcmp( token, "window_x_scroll" ) == 0) )
//  	panx = strtol( value, NULL, 10 );
//        else if ( (strcmp( token, "window_yscroll" ) == 0 )
//  		|| (strcmp( token, "window_y_scroll" ) == 0) )
//  	pany = strtol( value, NULL, 10 );
//      }

//  #ifdef DEBUG
//    cout << "ok." << endl;
//  #endif
//    return 1;
//  }


void CXGui::MoveObject( truth_t *obj, double x, double y, double th )
{    
  obj->x = x;    
  obj->y = y;
  obj->th = th;
  
  tp->SendSingleTruthToStage( *obj );

//  #ifdef DEBUG
//    printf( "XPSI: Moving object "
//  	  "(%d,%d,%d) [%.2f,%.2f,%.2f]\n", 
//  	  obj->id.port, 
//  	  obj->id.type, 
//  	  obj->id.index,
//  	  obj->x, obj->y, obj->th );
  
//    int num = 
//  #endif
}


void CXGui::DrawBackground( void ) 
{ 
  BlackBackground();
  DrawWalls();
}


//  void CXGui::PrintCoords( void )
//  {
//    Window dummyroot, dummychild;
//    int dummyx, dummyy, xpos, ypos;
//    unsigned int dummykeys;
  
//    XQueryPointer( display, win, &dummyroot, &dummychild, 
//  		 &dummyx, &dummyy, &xpos, &ypos, &dummykeys );
  
  
//    cout << "Mouse: " << xpos << ',' << ypos << endl;;
//  }



void CXGui::BoundsCheck( void )
{
  if( panx < 0 ) panx = 0;
  if( panx > ( iwidth - width ) ) panx =  iwidth - width;
  
  if( pany < 0 ) pany = 0;
  if( pany > ( iheight - height ) ) pany =  iheight - height;
}


void CXGui::HandleEvent()
{    
  XEvent reportEvent;
  KeySym key;
  
  //    // used for inverting Y between world and X coordinates
//    int iheight= world->GetBackgroundImage()->height;
  
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
	    cout << "CONFIGURE NOTIFY" << endl;
  	    x = reportEvent.xconfigure.x;
  	    y = reportEvent.xconfigure.y;
  	    width = reportEvent.xconfigure.width;
  	    height = reportEvent.xconfigure.height;
	    
  	    BoundsCheck();
  	  }

  	break;
 
        case Expose:
  	if( reportEvent.xexpose.count == 0 ) // on the last pending expose...
  	{
	  //cout << "EXPOSE" << endl;
  	  DrawBackground(); // black backround and draw walls
  	  RefreshObjects();

  	}
  	break;

        case MotionNotify:	
  	if( dragging )
  	  {
	    MoveObject( dragging, 
			(reportEvent.xmotion.x + panx ) / ppm,
			((iheight-reportEvent.xmotion.y) - pany ) / ppm,
			dragging->th );
	    
  	    HighlightObject( dragging, true );
  	  }
  	break;
	
       case ButtonRelease:
  	break;
	
        case ButtonPress: 
  	  switch( reportEvent.xbutton.button )
  	    {	      
  	    case Button1:  cout << "BUTTON 1" << endl;
  	      if( dragging  ) 
    		{ // stopped dragging
		  
		  // we're done with the dragging object now
    		  delete dragging;
		  dragging = NULL; // we test the value elsewhere

  		  HighlightObject( NULL, true ); // erases the highlighting
	
  		  // disable the pointer motion events
  		  XSelectInput(display, win, 
  			       ExposureMask | StructureNotifyMask | 
  			       ButtonPressMask | ButtonReleaseMask | 
  			       KeyPressMask );
    		} 
    	      else 
    		{  
		  // make a new local device for dragging around
		  dragging = new truth_t();

		  memset( dragging, 0, sizeof( truth_t ) );
		  
		  // copy the details of the nearest device
		  memcpy( dragging, 
			  tp->NearestDevice((reportEvent.xmotion.x
					     +panx )/ppm,
					    ((iheight-
					      reportEvent.xmotion.y)
					     -pany)/ppm ), 
			  sizeof( truth_t ) );

		  assert( dragging );

#ifdef DEBUG
		  printf( "XPSI: Dragging object "
			  "(%d,%d,%d) parent (%d,%d,%d) [%.2f,%.2f,%.2f]\n", 
			  dragging->id.port, 
			  dragging->id.type, 
			  dragging->id.index,
			  dragging->parent.port, 
			  dragging->parent.type, 
			  dragging->parent.index,
			  dragging->x, dragging->y, dragging->th );
#endif		      

		  // don't drag things with parents
		  while( dragging->parent.port != 0 )
		  {
		    // grab the parent't data instead
		    truth_t* parent = tp->GetDeviceByID( &(dragging->parent) );

		    assert( parent );

		    memcpy( dragging, parent, sizeof(truth_t) );
		  }

		  assert( dragging );


		  HighlightObject( dragging, false );
		  
		  // enable generation of pointer motion events
		  XSelectInput(display, win, 
			       ExposureMask | StructureNotifyMask | 
			       ButtonPressMask | ButtonReleaseMask | 
			       KeyPressMask | PointerMotionMask );
		}	      
	      break;
	      
	case Button2: cout << "BUTTON 2" << endl;
	  if( dragging )
  		{	  
  		  MoveObject( dragging, dragging->x, dragging->y, 
  			      NORMALIZE(dragging->th + M_PI/10.0) );
  		}
	  
//  #ifdef DEBUG	      
//  		  // this can be taken out at release
//  		  //useful debug stuff!
//  		  // dump the raw image onto the screen
//  //  		  for( int f = 0; f<world->img->width; f++ )
//  //  		    for( int g = 0; g<world->img->height; g++ )
//  //  		      {
//  //  			if( world->img->get_pixel( f,g ) == 0 )
//  //  			  XSetForeground( display, gc, white );
//  //  			else
//  //  			  XSetForeground( display, gc, black );

//  //  			XDrawPoint( display,win,gc, f, g );
//  //  		      }

//  //  		  getchar();
		  
//  //  		  DrawBackground();
//  //  		  DrawRobots();
//  //  		  drawnRobots = true;
//  #endif//  		  //	    }
  	     break;
	    case Button3: cout << "BUTTON 3" << endl;
  	     if( dragging )
	       {  			  
  		 MoveObject( dragging, dragging->x, dragging->y, 
  			      NORMALIZE(dragging->th - M_PI/10.0) );
  		}	      
//  	      else
//  		{ 
//  		  // we're centering on robot[0] - redraw everything
//  		  //panx = (int)(world->bots[0].x - width/2.0);
//  		  //pany = (int)(world->bots[0].y - height/2.0);
		  
//  		  //BoundsCheck();		  
//  		  //ScanBackground();
//  		  //DrawBackground();		
//  		  //DrawRobots();
//  		  //drawnRobots = true;
//  		}
	      break;
	    }
	  break;	
	  
	    case KeyPress: cout << "KEY" << endl;
//  	// pan te window view
  	key = XLookupKeysym( &reportEvent.xkey, 0 );

	printf( "key ppm: %g\n", ppm );

  	int oldPanx = panx;
  	int oldPany = pany;

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
  	    //ScanBackground();
  	    DrawBackground();
  	    RefreshObjects();
  	  }

  	// check for LOAD and SAVE key commands
  	if( key == XK_l || key == XK_L )
  	{
  	  cout << "LOAD" << endl;
//  	  //world->Load();
//  	  //RefreshObjects();
  	}

  	if( key == XK_s || key == XK_S )
  	{
  	  cout << "SAVE" << endl;
//  	  world->Save();
  	}

  	if( key == XK_q || key == XK_Q )
  	{
  	  cout << "QUIT" << endl;
	  exit( 0 );
  	}

  	if( key == XK_z || key == XK_Z )
  	{
	  // if the window isn't too huge, zoom out
	  if( width * 1.2 <= MAX_WINDOW_WIDTH )
	    {
	      cout << "ZOOM IN" << endl;
	      ppm *= 1.2;
	      iwidth *= 1.2;
	      iheight *= 1.2;
	      width *= 1.2;
	      height *= 1.2;
	      
	      Size();
	      DrawBackground(); // black backround and draw walls
	      RefreshObjects();
	    }
  	}

  	if( key == XK_x || key == XK_X )
  	{
  	  cout << "ZOOM OUT" << endl;

	  // if the window isn't too teeny, zoom in
	  if( width * 0.8 >= MIN_WINDOW_WIDTH )
	  {
	    ppm *= 0.8;
	    iwidth *= 0.8;
	    iheight *= 0.8;
	    width *= 0.8;
	    height *= 0.8;

	    Size();
	    DrawBackground(); // black backround and draw walls
	    RefreshObjects();
	  }
  	}

	// PREVENT US GROWING THE WINDOW BEYOND THE SIZE OF THE WORLD
	  XSizeHints sz;

	  sz.flags = PMaxSize | PMinSize;// | PAspect;
	  sz.min_width = MIN_WINDOW_WIDTH; 
	  sz.max_width = iwidth;
	  sz.min_height = MIN_WINDOW_HEIGHT;
	  sz.max_height = iheight;
 
	  XSetWMProperties(display, win, 
			   (XTextProperty*)NULL,
			   (XTextProperty*)NULL, 
			   (char**)NULL, 0, 
			   &sz, (XWMHints*)NULL, (XClassHint*)NULL );

  
  	break;
        }

//    // try to sync the display to avoid flicker
    XSync( display, false );
}


//  void CXGui::ScanBackground( void )
//  {
//  #ifdef VERBOSE
//    cout << "Scanning background (no scaling)... " << flush;
//  #endif

//    if( backgroundPts ) delete[] backgroundPts;

//    backgroundPts = new XPoint[width*height];
//    backgroundPtsCount = 0;

//    Nimage* img = world->GetBackgroundImage();

//    //int sx, sy,
//      int px, py;
//    for( px=0; px < width; px++ )
//      for( py=0; py < height; py++ )
//        if( img->get_pixel( px+panx, py+pany ) != 0 )
//        	{
//              backgroundPts[backgroundPtsCount].x = px;
//              backgroundPts[backgroundPtsCount].y = py;
//              backgroundPtsCount++;
//        }
//  #ifdef VERBOSE
//    cout << "ok." << endl;
//  #endif
//  }


void CXGui::BlackBackground( void )
{
  XSetForeground( display, gc, black );
  XFillRectangle( display, win, gc, 0,0, width, height );
}

//  void CXGui::BlackBackgroundInRect( int xx, int yy, int ww, int hh )
//  {
//      XSetForeground( display, gc, black );
//      XFillRectangle( display, win, gc, xx,yy, ww, hh );
//  }
 
void CXGui::DrawWalls( void )
{
  SetDrawMode( GXcopy );

  DPoint pts[ op->num_pixels ];

  SetForeground( RGB(255,255,255) );
 
  for( int i=0; i<op->num_pixels; i++ )
    {
      pixel_t *p = &(op->pixels[i]);

      //GetRect( p->x / ppm, p->y / ppm, 
      //       (1/width) / ppm, (1/height) / ppm, 
      //       0, pts );
      //DrawPolygon( pts, 4 );

      pts[i].x = p->x / (double)(op->ppm);
      pts[i].y = iheight/(double)ppm - p->y / (double)(op->ppm);
    }

  //DrawPoints( pts, op->og.num_pixels );
  DrawPoints( pts, op->num_pixels );
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
  if( str && strlen( str ) > 0 )
    {
      XDrawString( display,win,gc, 
  		   (ULI)(x * ppm - panx),
  		   (ULI)( (iheight-pany) - ((int)( y*ppm ))),
  		   str, len );
    }
  else
    cout << "Warning: XGUI::DrawString was passed a null pointer" << endl;
}

void CXGui::DrawLines( DPoint* dpts, int numPts )
{
  XPoint* xpts = new XPoint[numPts];
  
  // scale, pan and quantize the points
  for( int l=0; l<numPts; l++ )
    {
      xpts[l].x = ((int)( dpts[l].x * ppm )) - panx;
      xpts[l].y = (iheight-pany) - ((int)( dpts[l].y * ppm ));
    }
  
  // and draw them
  XDrawLines( display, win, gc, xpts, numPts, CoordModeOrigin );
   
  // try to sync the display to avoid flicker
  //XSync( display, false );
  
  delete [] xpts; 
}

  void CXGui::DrawPoints( DPoint* dpts, int numPts )
  {
    XPoint* xpts = new XPoint[ numPts ];
  
    // scale, pan and quantize the points
    for( int l=0; l<numPts; l++ )
      {
        xpts[l].x = ((int)( dpts[l].x * ppm )) - panx;
        xpts[l].y = (iheight-pany) - ((int)( dpts[l].y * ppm ));
      }
 
    // and draw them
    XDrawPoints( display, win, gc, xpts, numPts, CoordModeOrigin );
 
    delete [] xpts; 
  }

  void CXGui::DrawCircle( double x, double y, double r )
  {
    XDrawArc( display, win, gc, 
  	    (ULI)((x-r) * ppm -panx), 
  	    (ULI)((iheight-pany)-((y+r) * ppm)), 
  	    (ULI)(2.0*r*ppm), (ULI)(2.0*r*ppm), 0, 23040 );
  }

//  void CXGui::DrawWallsInRect( int xx, int yy, int ww, int hh )
//  {
//     XSetForeground( display, gc, white );

//    // draw just the points that are in the defined rectangle
//    for( int p=0; p<backgroundPtsCount; p++ )
//      if( backgroundPts[p].x >= xx &&
//  	backgroundPts[p].y >= yy &&  
//  	backgroundPts[p].x <= xx+ww &&   
//  	backgroundPts[p].y <= yy+hh )
//        XDrawPoint( display, win, gc,  backgroundPts[p].x, backgroundPts[p].y );
//  }

void CXGui::RefreshObjects( void )
{
  SetDrawMode( GXxor );

  truth_t *truth;

  tp->FreshenAll();

  while( ( truth= tp->GetNextFreshDevice()) )
    {
      RenderObject( truth );
    }

  HighlightObject( dragging, false );
//    HighlightObject( near, false );

    SetDrawMode( GXcopy );
}

//  // hide the CRobot reference
//  //  unsigned long CXGui::RobotDrawColor( CRobot* r )
//  //  { 
//  //    unsigned long color;
//  //    switch( r->channel % 6 )
//  //      {
//  //      case 0: color = red; break;
//  //      case 1: color = green; break;
//  //      case 2: color = blue; break;
//  //      case 3: color = yellow; break;
//  //      case 4: color = magenta; break;
//  //      case 5: color = cyan; break;
//  //      }

//  //    return color;
//  //  }

void CXGui::SetForeground( unsigned long col )
{ 
  XSetForeground( display, gc, col );
}


//  // hide the CRobot reference
//  //  void CXGui::DrawInRobotColor( CRobot* r )
//  //  { 
//  //    XSetForeground( display, gc, RobotDrawColor( r ) );
//  //  }


void CXGui::Move( void )
{
  XMoveWindow( display, win, x,y );
}

void CXGui::Size( void )
{
  XResizeWindow( display, win, width, height );
}

istream& operator>>(istream& s, CXGui& w )
{
  s >> w.x >> w.y >> w.width >> w.height;
  w.Move();
  w.Size();

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

void CXGui::RenderObject( truth_t* truth )
  {
#ifdef DEBUG
    //    char buf[30]
    //GetObjectType( exp, buf, 29 );
    //cout << "Rendering " << exp->objectId << " " << buf <<  endl;

    //printf( "R(%d,%d,%d)\n", truth->id.port, truth->id.type, truth->id.index );
    //fflush( stdout );
#endif

    bool extended = false;

    SetDrawMode( GXxor );


    // check the object type and call the appropriate render function 
    switch( truth->id.type )
      {
      case PLAYER_POSITION_CODE: RenderPioneer( truth, extended ); break; 
      case PLAYER_LASER_CODE: RenderLaserTurret( truth, extended ); break;
	//case PLAYER_BEACON_CODE: RenderLaserBeacon( &truth, extended ); break;
	//case box_o: RenderBox( &truth, extended ); break;
	case PLAYER_PLAYER_CODE: RenderPlayer( truth, extended ); break;
	case PLAYER_MISC_CODE: RenderMisc( truth , extended); break;
	case PLAYER_SONAR_CODE: RenderSonar( truth, extended ); break;
	//case laserbeacondetector_o: 
	//RenderLaserBeaconDetector( &truth, extended ); break;
	//case PLAYER_VISION_CODE: RenderVision( &truth, extended ); break;
	case PLAYER_PTZ_CODE: RenderPTZ( truth, extended ); break;  
	case PLAYER_TRUTH_CODE: RenderTruth( truth, extended ); break;  
	case PLAYER_OCCUPANCY_CODE: RenderOccupancy( truth, extended ); break; 
	
	//        case generic_o: RenderGenericObject( &truth ); 
	//          cout << "Warning: GUI asked to render a GENERIC object" << endl;
	//          break;
      default: cout << "XGui: unknown object type " 
		    << truth->id.type << endl;
      }

    SetDrawMode( GXcopy );
    
  }

void CXGui::RenderObjectLabel( truth_t* exp, char* str, int len )
{
  if( str && strlen(str) > 0 )
    {
      SetForeground( RGB(255,255,255) );
      DrawString( exp->x + 0.6, exp->y - (8.0/ppm + exp->id.type * 11.0/ppm) , 
		  str, len );
    }
}

//  void CXGui::RenderGenericObject( truth_t* exp )
//  {
//  #ifdef LABELS
//    RenderObjectLabel( exp, "Generic object", 14 );
//  #endif

//    SetForeground( RGB(255,255,255) );
//    DrawBox( exp->x, exp->y, 0.9 );

//    char buf[64];

//    sprintf( buf, "Generic @ %d", (int)(exp->objectId) );
//    RenderObjectLabel( exp, buf, strlen(buf) );
//  }

//  //void CXGui::RenderPlayer

void CXGui::RenderLaserTurret( truth_t* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "SICK LMS200", 11 );
#endif
  
  SetForeground( RGB(0,0,255) );
  
  DPoint pts[4];
  GetRect( exp->x, exp->y, 0.1, 0.1, exp->th, pts );
  DrawPolygon( pts, 4 );
  
  //cout << "trying to render " << exp->hitCount << " points" << endl;
  
  //if( extended && exp->data )
  //      {
  //        SetForeground( RGB(70,70,70) );
  //        ExportLaserData* ld = (ExportLaserData*) exp->data;
  //        DrawPolygon( ld->hitPts, ld->hitCount );
  //      }
}

//  void CXGui::RenderLaserBeaconDetector( truth_t* exp, bool extended )
//  { 
//  #ifdef LABELS
//    RenderObjectLabel( exp, "Detector", 8 );
//  #endif
  
//    if( extended && exp->data )
//      {
//        DPoint pts[4];
//        char buf[20];
      
//        SetForeground( RGB( 255, 0, 255 ) );
      
//        ExportLaserBeaconDetectorData* p = 
//  	(ExportLaserBeaconDetectorData*)exp->data;
      
//        if( p->beaconCount > 0 ) for( int b=0; b < p->beaconCount; b ++ )
//  	{
//  	  GetRect( p->beacons[ b ].x, p->beacons[ b ].y, 
//  		   (4.0/ppm), 0.12 + (2.0/ppm), p->beacons[ b ].th, pts );
	  
//  	  // don't close the rectangle 
//  	  // so the open box visually indicates heading of the beacon
//  	  DrawLines( pts, 4 ); 
	  
//  	  //DrawLines( &(pts[1]), 2 ); 

//  	  // render the beacon id as text
//  	  sprintf( buf, "%d", p->beacons[ b ].id );
//  	  DrawString( p->beacons[ b ].x + 0.2 + (5.0/ppm), 
//  		      p->beacons[ b ].y - (4.0/ppm), 
//  		      buf, strlen( buf ) );
//  	}
//      }
//  }

void CXGui::RenderTruth( truth_t* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Truth", 12 );
#endif 

  SetForeground( RGB(255,255,0) );
  DrawString( exp->x, exp->y, "Truth", 5 );

}

void CXGui::RenderOccupancy( truth_t* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Occupancy", 10 );
#endif 

  SetForeground( RGB(0,255,0) );
  DrawString( exp->x, exp->y, "Occupancy", 9 ); 
}

void CXGui::RenderOccupancyGrid( void )
{
  
  SetDrawMode( GXcopy );

  DPoint pts[ op->num_pixels ];
  
  SetForeground( RGB(255,255,255) );
 
  for( int i=0; i<op->num_pixels; i++ )
    {
      pixel_t *p = &(op->pixels[i]);
      
      //printf( " (%d %d %d)", p->x, p->y, p->color );
      //fflush( stdout );

      //GetRect( p->x / ppm, p->y / ppm, 
      //       (1/width) / ppm, (1/height) / ppm, 
      //       0, pts );
      //DrawPolygon( pts, 4 );

      pts[i].x = p->x / (double)(op->ppm);
      pts[i].y = iheight/(double)ppm - p->y / (double)(op->ppm);
    }

  printf( "DRAWING %d POINTS\n", op->num_pixels );

  //DrawPoints( pts, op->og.num_pixels );
  DrawPoints( pts, op->num_pixels );

    // try to sync the display to avoid flicker
  //XSync( display, false );

}

void CXGui::RenderPTZ( truth_t* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "PTZ Camera", 10 );
#endif
  
  SetForeground( RGB(200,200,200 ) );
  
  DPoint pts[4];
  GetRect( exp->x, exp->y, 0.05, 0.05, exp->th, pts );
  DrawPolygon( pts, 4 );
  
  
//    if( 0 )
//    //if( extended && exp->data ) 
//      {

//        ExportPtzData* p = (ExportPtzData*)exp->data;
      
//        //DPoint pts[4];
//        //GetRect( exp->x, exp->y, 0.4, 0.4, exp->th, pts );
//        //DrawPolygon( pts, 4 );
      
//        double len = 0.2; // meters
//        // have to figure out the zoom with Andrew
//        // for now i have a fixed width triangle
//        //double startAngle =  exp->th + p->pan - p->zoom/2.0;
//        //double stopAngle  =  startAngle + p->zoom;
//        double startAngle =  exp->th + p->pan - DTOR( 30 );
//        double stopAngle  =  startAngle + DTOR( 60 );
      
//        pts[0].x = exp->x;
//        pts[0].y = exp->y;
      
//        pts[1].x = exp->x + len * cos( startAngle );  
//        pts[1].y = exp->y + len * sin( startAngle );  
      
//        pts[2].x = exp->x + len * cos( stopAngle );  
//        pts[2].y = exp->y + len * sin( stopAngle );  
      
//        DrawPolygon( pts, 3 );
//      }
 }

void CXGui::RenderSonar( truth_t* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Sonar", 5 );
#endif
  //    if( extended && exp->data )
  //      {
  //        ExportSonarData* p = (ExportSonarData*)exp->data;
  //        SetForeground( RGB(70,70,70) );
  //        DrawPolygon( p->hitPts, p->hitCount );
  //      }
  //  }

  SetForeground( RGB(255,0,0) );
  DrawString( exp->x, exp->y, "Sonar", 1 );
}

void CXGui::RenderVision( truth_t* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Vision", 6 );
#endif
}

void CXGui::RenderPlayer( truth_t* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Player", 6 );
#endif
  
  SetForeground( RGB(255,255,0) );
  DrawString( exp->x, exp->y, "P", 1 );
}

void CXGui::RenderMisc( truth_t* exp, bool extended )
{
#ifdef LABELS
  RenderObjectLabel( exp, "Misc", 4 );
#endif
 
  //    if( 0 ) // disable for now
//    //if( extended && exp->data )
//      {
  //        SetForeground( RGB(255,0,0) ); // red to match Pioneer base
      
  char buf[20];
      
//        player_misc_data_t* p = (player_misc_data_t*)exp->data;
      
        // support bumpers eventually
        //p->rearbumpers;
//        //p->frontbumpers;
      
  float v = 11.9;//p->voltage / 10.0;
  sprintf( buf, "%.1fV", v );
  DrawString( exp->x + 1.0, exp->y + 1.0, buf, strlen( buf ) ); 
//      }
}


void CXGui::RenderPioneer( truth_t* exp, bool extended )
 { 
   //  cout << "render position" << endl;

#ifdef LABELS
   RenderObjectLabel( exp, "Pioneer base", 12 );
#endif
  
//    //SetForeground( RGB(255,0,0) ); // red Pioneer base color
    SetForeground( RGB(255,100,100) ); // red Pioneer base color

    DPoint pts[7];
    GetRect( exp->x, exp->y, exp->w/2.0, exp->h/2.0, exp->th, pts );

    pts[4].x = pts[0].x;
    pts[4].y = pts[0].y;

    pts[5].x = exp->x;
    pts[5].y = exp->y;

    pts[6].x = pts[3].x;
    pts[6].y = pts[3].y;

//    //DrawCircle( pts[1].x, pts[1].y, 0.3 );

    DrawLines( pts, 7 );
}

void CXGui::RenderLaserBeacon( truth_t* exp, bool extended )
{ 
#ifdef LABELS
  RenderObjectLabel( exp, "Beacon", 6 );
#endif
  
  SetForeground( RGB(0,255,0) );
  //DrawBox( exp->x, exp->y, 0.1 );
//DrawCircle( exp->x, exp->y, 0.2 );
  
  DPoint pts[4];
  GetRect( exp->x, exp->y, exp->w/2.0, exp->h/2.0, exp->th, pts );
  
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
  
    pts[2].x = x + (-cxcosa + cysina);
    pts[2].y = y + (-cxsina - cycosa);
    pts[3].x = x + (+cxcosa + cysina);
    pts[3].y = y + (+cxsina - cycosa);
    pts[1].x = x + (-cxcosa - cysina);
    pts[1].y = y + (-cxsina + cycosa);
    pts[0].x = x + (+cxcosa - cysina);
    pts[0].y = y + (+cxsina + cycosa);
  }

  void CXGui::HighlightObject( truth_t* exp, bool undraw )
  {
    static double x = -1000.0, y = -1000.0, r = -1000.0, th = -1000.0;
    //    static char label[LABELSIZE];
    //static char info[50];
  
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
        //if( label ) 
  	//DrawString( x + xoffset, y + yoffset, label, strlen( label ) );
        //if( info[0] ) 	  
  	//DrawString( x + xoffset, y + (yoffset + yincrement), info, strlen( info ) );

      }

    if( exp ) // if this is a valid object
      {
	x = exp->x;
        y = exp->y;
        r = 0.5;
        th = exp->th;
    
        DrawCircle( x,y, r ); // and highlight it

//          if( exp->label )
//  	  {
//  	    strcpy( label, exp->label );
//  	    DrawString( x + xoffset, y + yoffset, label, strlen( label ) );
//  	  }
	
//        if( renderSensorData )
//  	{
//  	  sprintf( info, "(%.2f, %.2f, %.2f)", exp->x, exp->y, exp->th );
//  	  DrawString( x + xoffset, y + (yoffset + yincrement), info, strlen( info ) );
//  	}
	//        else
//  	info[0] = 0; //null string
      }
//    else
//      x = y = th = -1000.0; //reset the bouding box
  
    // reset the GC
    SetDrawMode( GXcopy );
    XSetLineAttributes( display, gc, 0, LineSolid, CapRound, JoinRound );
}



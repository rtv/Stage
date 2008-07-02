/** canvas.cc
    Implement the main world viewing area in FLTK and OpenGL. 
    Author: Richard Vaughan (vaughan@sfu.ca)
    $Id: canvas.cc,v 1.12 2008-03-03 07:01:12 rtv Exp $
*/

#include "stage_internal.hh"
#include "texture_manager.hh"
#include "replace.h"

#include <string>
#include <png.h>

#include "file_manager.hh"
#include "options_dlg.hh"

using namespace Stg;

static  const int checkImageWidth = 2;
static  const int	checkImageHeight = 2;
static  GLubyte checkImage[checkImageHeight][checkImageWidth][4];
static  GLuint texName;

void StgCanvas::TimerCallback( StgCanvas* c )
{
  c->redraw();
  
  Fl::repeat_timeout(((double)c->interval/1000),
		     (Fl_Timeout_Handler)StgCanvas::TimerCallback, 
		     c);
}


StgCanvas::StgCanvas( StgWorldGui* world, int x, int y, int w, int h) :
  Fl_Gl_Window(x,y,w,h),
  // initialize Option objects
  showBlinken( "Blinkenlights", "show_blinkenlights", "", true ),
  showBlocks( "Blocks", "show_blocks", "b", true  ),
  showClock( "Clock", "show_clock", "c", true ),
  showData( "Data", "show_data", "d", false ),
  showFlags( "Flags", "show_flags", "f",  true ),
  showFollow( "Follow", "show_follow", "F", false ),
  showFootprints( "Footprints", "show_footprints", "f", false ),
  showGrid( "Grid", "show_grid", "g", true ),
  showOccupancy( "Debug/Occupancy", "show_occupancy", "#O", false ),
  showScreenshots( "Save screenshots", "screenshots", "", false ),
  showStatus( "Status", "show_status", "", true ),
  showTrailArrows( "Trails/Rising Arrows", "show_trailarrows", "#a", false ),
  showTrailRise( "Trails/Rising blocks", "show_trailrise", "#r", false ),
  showTrails( "Trails/Fast", "show_trailfast", "t", false ),
  showTree( "Debug/Tree", "show_tree", "#T", false )
{
  end();

  //show(); // must do this so that the GL context is created before configuring GL
  // but that line causes a segfault in Linux/X11! TODO: test in OS X

  this->world = world;
  selected_models = NULL;
  last_selection = NULL;

  use_perspective_camera = false;
  perspective_camera.setPose( -3.0, 0.0, 1.0 );
  perspective_camera.setPitch( 70.0 ); //look down
	
  startx = starty = 0;
  //panx = pany = stheta = sphi = 0.0;
  //scale = 15.0;
  interval = 50; //msec between redraws

  graphics = true;
  dragging = false;
  rotating = false;

  // // start the timer that causes regular redraws
  Fl::add_timeout( ((double)interval/1000), 
		   (Fl_Timeout_Handler)StgCanvas::TimerCallback, 
		   this);
}

StgCanvas::~StgCanvas()
{
}

StgModel* StgCanvas::Select( int x, int y )
{
  // TODO XX
  //return NULL;

	
  // render all models in a unique color
  make_current(); // make sure the GL context is current
  glClearColor ( 1,1,1,1 ); // white
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


  glDisable(GL_DITHER);
  glDisable(GL_BLEND); // turns off alpha blending, so we read back
  // exactly what we write to a pixel

  // render all top-level, draggable models in a color that is their
  // id 
  for( GList* it= world->StgWorld::children; it; it=it->next )
    {
      StgModel* mod = (StgModel*)it->data;
		
      if( mod->GuiMask() & (STG_MOVE_TRANS | STG_MOVE_ROT ))
	{
	  uint8_t rByte, gByte, bByte, aByte;
	  uint32_t modelId = mod->id;
	  rByte = modelId;
	  gByte = modelId >> 8;
	  bByte = modelId >> 16;
	  aByte = modelId >> 24;

	  //printf("mod->Id(): 0x%X, rByte: 0x%X, gByte: 0x%X, bByte: 0x%X, aByte: 0x%X\n", modelId, rByte, gByte, bByte, aByte);
			
	  glColor4ub( rByte, gByte, bByte, aByte );
	  mod->DrawPicker();
	}
    }
	
  // read the color of the pixel in the back buffer under the mouse
  // pointer
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);

  uint8_t rByte, gByte, bByte, aByte;
  uint32_t modelId;
	
  glReadPixels( x,viewport[3]-y,1,1,
		GL_RED,GL_UNSIGNED_BYTE,(void*)&rByte );
  glReadPixels( x,viewport[3]-y,1,1,
		GL_GREEN,GL_UNSIGNED_BYTE,(void*)&gByte );
  glReadPixels( x,viewport[3]-y,1,1,
		GL_BLUE,GL_UNSIGNED_BYTE,(void*)&bByte );
  glReadPixels( x,viewport[3]-y,1,1,
		GL_ALPHA,GL_UNSIGNED_BYTE,(void*)&aByte );
	
  modelId = rByte;
  modelId |= gByte << 8;
  modelId |= bByte << 16;
  //modelId |= aByte << 24;
	
  //	printf("Clicked rByte: 0x%X, gByte: 0x%X, bByte: 0x%X, aByte: 0x%X\n", rByte, gByte, bByte, aByte);
  //	printf("-->model Id = 0x%X\n", modelId);
	
  StgModel* mod = StgModel::LookupId( modelId );

  //printf("%p %s %d %x\n", mod, mod ? mod->Token() : "(none)", id, id );

  // put things back the way we found them
  glEnable(GL_DITHER);
  glEnable(GL_BLEND);
  glClearColor ( 0.7, 0.7, 0.8, 1.0);

  if( mod ) // we clicked on a root model
    {
      // if it's already selected
      if( GList* link = g_list_find( selected_models, mod ) )
	{
	  // remove it from the selected list
	  selected_models = 
	    g_list_remove_link( selected_models, link );
	  mod->Disable();
	}			  
      else      
	{
	  last_selection = mod;
	  selected_models = g_list_prepend( selected_models, mod );
	  mod->Enable();
	}

      invalidate();
    }

  return mod;
}

// convert from 2d window pixel to 3d world coordinates
void StgCanvas::CanvasToWorld( int px, int py, 
			       double *wx, double *wy, double* wz )
{
  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

  GLdouble projection[16];	
  glGetDoublev(GL_PROJECTION_MATRIX, projection);	

  GLfloat pz;
  glReadPixels( px, h()-py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pz );
  gluUnProject( px, w()-py, pz, modelview, projection, viewport, wx,wy,wz );
}

int StgCanvas::handle(int event) 
{

  switch(event) 
    {
    case FL_MOUSEWHEEL:
      if( selected_models )
	{
	  // rotate all selected models
	  for( GList* it = selected_models; it; it=it->next )
	    {
	      StgModel* mod = (StgModel*)it->data;
	      mod->AddToPose( 0,0,0, 0.1*(double)Fl::event_dy() );
	    }
	  redraw();
	}
      else
	{
	  if( use_perspective_camera == true ) {
	    perspective_camera.scroll( Fl::event_dy() / 10.0 );
	  } else {
	    camera.scale( Fl::event_dy(),  Fl::event_x(), w(), Fl::event_y(), h() );
	  }
	  invalidate();
	  redraw();
	}
      return 1;

    case FL_MOVE: // moused moved while no button was pressed
      if( Fl::event_state( FL_CTRL ) )
	{          
	  int dx = Fl::event_x() - startx;
	  int dy = Fl::event_y() - starty;

	  if( use_perspective_camera == true ) {
	    perspective_camera.addYaw( -dx );
	    perspective_camera.addPitch( -dy );
	  } else {
	    camera.pitch( 0.5 * static_cast<double>( dy ) );
	    camera.yaw( 0.5 * static_cast<double>( dx ) );
	  }

	  invalidate();
	  redraw();
	}
      else if( Fl::event_state( FL_ALT ) )
	{   
	  int dx = Fl::event_x() - startx;
	  int dy = Fl::event_y() - starty;

	  if( use_perspective_camera == true ) {
	    perspective_camera.move( -dx, dy, 0.0 );
	  } else {
	    camera.move( -dx, dy );

	  }
	  invalidate();

	}

      startx = Fl::event_x();
      starty = Fl::event_y();
			
      return 1;

    case FL_PUSH: // button pressed
      switch( Fl::event_button() )
	{
	case 1:
	  startx = Fl::event_x();
	  starty = Fl::event_y();
	  if( Select( startx, starty ) )
	    dragging = true;	  
	  return 1;
	case 3:
	  {
	    startx = Fl::event_x();
	    starty = Fl::event_y();
	    if( Select( startx, starty ) )
	      rotating = true;	  
	    return 1;
	  }
	default:
	  return 0;
	}    

    case FL_DRAG: // mouse moved while button was pressed
      {
	int dx = Fl::event_x() - startx;
	int dy = Fl::event_y() - starty;

	switch( Fl::event_button() )
	  {
	  case 1:
	    if( dragging )
	      {
		assert(selected_models);

		double sx,sy,sz;
		CanvasToWorld( startx, starty,
			       &sx, &sy, &sz );
		double x,y,z;
		CanvasToWorld( Fl::event_x(), Fl::event_y(),
			       &x, &y, &z );

		// move all selected models to the mouse pointer
		for( GList* it = selected_models; it; it=it->next )
		  {
		    StgModel* mod = (StgModel*)it->data;
		    mod->AddToPose( x-sx, y-sy, 0, 0 );
		  }
	      }
	    else
	      {
		camera.move( -dx, dy );
		invalidate(); // so the projection gets updated
	      }
	    break;	    
	  case 3: // right button
	    if( rotating )
	      {
		// move all selected models to the mouse pointer
		for( GList* it = selected_models; it; it=it->next )
		  {
		    StgModel* mod = (StgModel*)it->data;
		    mod->AddToPose( 0,0,0, 0.05*dx );
		  }
	      }
	    break;
	  }
      }
      startx = Fl::event_x();
      starty = Fl::event_y();

      redraw();
      return 1; // end case FL_DRAG

    case FL_RELEASE:   // mouse button released
      // unselect everyone unless shift is pressed
      if( ! Fl::event_state( FL_SHIFT ) )
	{
	  for( GList* it=selected_models; it; it=it->next )
	    ((StgModel*)it->data)->Enable();

	  g_list_free( selected_models );
	  selected_models = NULL;
	  dragging = false;
	  rotating = false;
	  redraw();
	}
      return 1;

    case FL_FOCUS :
    case FL_UNFOCUS :
      //.... Return 1 if you want keyboard events, 0 otherwise
      return 1;
    case FL_KEYBOARD:
      switch( Fl::event_key() )
	{
	case 'p': // pause
	  world->TogglePause();
	  break;
	case ' ': // space bar
	  camera.resetAngle();
	  //invalidate();
	  if( Fl::event_state( FL_CTRL ) ) {
	    //do a complete reset
	    camera.setPose( 0.0, 0.0 );
	    camera.setScale( 10.0 );
	  }
	  redraw();
	  break;
	case FL_Left:
	  if( use_perspective_camera == false ) { camera.move( -10, 0 ); } 
	  else { perspective_camera.strafe( -0.5 ); } break;
	case FL_Right: 
	  if( use_perspective_camera == false ) {camera.move( 10, 0 ); } 
	  else { perspective_camera.strafe( 0.5 ); } break;
	case FL_Down:  
	  if( use_perspective_camera == false ) {camera.move( 0, -10 ); } 
	  else { perspective_camera.forward( -0.5 ); } break;
	case FL_Up:  
	  if( use_perspective_camera == false ) {camera.move( 0, 10 ); } 
	  else { perspective_camera.forward( 0.5 ); } break;
	default:
	  return 0; // keypress unhandled
	}
      invalidate(); // update projection
      return 1;
      //case FL_SHORTCUT:
      ///... shortcut, key is in Fl::event_key(), ascii in Fl::event_text()
      // ... Return 1 if you understand/use the shortcut event, 0 otherwise...
      //return 1;
    default:
      // pass other events to the base class...
      //printf( "EVENT %d\n", event );
      return Fl_Gl_Window::handle(event);
    }
}

void StgCanvas::FixViewport(int W,int H) 
{
  glLoadIdentity();
  glViewport(0,0,W,H);
}




void StgCanvas::DrawGlobalGrid()
{

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  stg_bounds3d_t bounds = world->GetExtent();

  char str[16];	
  PushColor( 0.15, 0.15, 0.15, 1.0 ); // pale gray
  for( double i = floor(bounds.x.min); i < bounds.x.max; i++)
    {
      snprintf( str, 16, "%d", (int)i );
      gl_draw_string(  i, 0, 0.00, str );
    }
	
  for( double i = floor(bounds.y.min); i < bounds.y.max; i++)
    {
      snprintf( str, 16, "%d", (int)i );
      gl_draw_string(  0, i, 0.00, str );
    }
  PopColor();
	
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(2.0, 2.0);
  glDisable(GL_BLEND);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texName);

  glBegin(GL_QUADS);

  glTexCoord2f( bounds.x.min/2.0, bounds.y.min/2.0 ); 
  glVertex3f( bounds.x.min, bounds.y.min, 0 );
  glTexCoord2f( bounds.x.max/2.0, bounds.y.min/2.0); 
  glVertex3f(  bounds.x.max, bounds.y.min, 0 );
  glTexCoord2f( bounds.x.max/2.0, bounds.y.max/2.0 ); 
  glVertex3f(  bounds.x.max, bounds.y.max, 0 );
  glTexCoord2f( bounds.x.min/2.0, bounds.y.max/2.0 ); 
  glVertex3f( bounds.x.min, bounds.y.max, 0 );

  glEnd();

  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
	

  glDisable(GL_POLYGON_OFFSET_FILL );
}

//draw the floor without any grid ( for robot's perspective camera model )
void StgCanvas::DrawFloor()
{
  stg_bounds3d_t bounds = world->GetExtent();
  float z = 0;
  glColor3f( 0.6, 0.6, 1.0 );
  glBegin(GL_QUADS);
  glVertex3f( bounds.x.min, bounds.y.min, z );
  glVertex3f(  bounds.x.max, bounds.y.min, z );
  glVertex3f(  bounds.x.max, bounds.y.max, z );
  glVertex3f( bounds.x.min, bounds.y.max, z );
  glEnd();
	
  glEnd();
}

void StgCanvas::DrawBlocks() 
{
  LISTMETHOD( world->StgWorld::children, StgModel*, DrawBlocksTree );
}

void StgCanvas::renderFrame()
{
  if( ! showTrails )
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  if( showGrid )
    DrawGlobalGrid();
  
  if( showTree || showOccupancy )
    {
      glPushMatrix();	  
      glScalef( 1.0/world->Resolution(), 1.0/world->Resolution(), 0 );
		
      glLineWidth( 1 );
      glPolygonMode( GL_FRONT, GL_LINE );
      colorstack.Push(1,0,0);
		
      if( showOccupancy )
	((StgWorldGui*)world)->DrawTree( false );
		
      if( showTree )
	((StgWorldGui*)world)->DrawTree( true );
		
      colorstack.Pop();
      glPopMatrix();
    }
  
  for( GList* it=selected_models; it; it=it->next )
    ((StgModel*)it->data)->DrawSelected();
  
  if( showFootprints )
    {
      glDisable( GL_DEPTH_TEST );
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
		
      for( GList* it=world->StgWorld::children; it; it=it->next )
	{
	  ((StgModel*)it->data)->DrawTrailFootprint();
	}
      glEnable( GL_DEPTH_TEST );
    }
  
  if( showTrailRise )
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
		
      for( GList* it=world->StgWorld::children; it; it=it->next )
	{
	  ((StgModel*)it->data)->DrawTrailBlocks();
	}
    }
  
  if( showTrailArrows )
    {
      glEnable( GL_DEPTH_TEST );
      for( GList* it=world->StgWorld::children; it; it=it->next )
	((StgModel*)it->data)->DrawTrailArrows();
    }

  if( showBlocks )
    DrawBlocks();
  
  // draw the model-specific visualizations
  if( showData ) 
    for( GList* it=world->StgWorld::children; it; it=it->next ) 
      ((StgModel*)it->data)->DataVisualizeTree();  
  
  if( showGrid ) 
    for( GList* it=world->StgWorld::children; it; it=it->next )
      ((StgModel*)it->data)->DrawGrid();
		
  if( showFlags ) 
    for( GList* it=world->StgWorld::children; it; it=it->next )
      ((StgModel*)it->data)->DrawFlagList();
		
  if( showBlinken ) 
    for( GList* it=world->StgWorld::children; it; it=it->next )
      ((StgModel*)it->data)->DrawBlinkenlights();	
	
  if ( showStatus )
    for( GList* it=world->StgWorld::children; it; it=it->next )
      ((StgModel*)it->data)->DrawStatusTree( this );
	
  if( world->GetRayList() )
    {
      glDisable( GL_DEPTH_TEST );
      PushColor( 0,0,0,0.5 );
      for( GList* it = world->GetRayList(); it; it=it->next )
	{
	  float* pts = (float*)it->data;
	  glBegin( GL_LINES );
	  glVertex2f( pts[0], pts[1] );
	  glVertex2f( pts[2], pts[3] );
	  glEnd();
	}  
      PopColor();
      glEnable( GL_DEPTH_TEST );
		 
      world->ClearRays();
    } 
	
  if( showClock )
    {
      //use orthogonal projeciton without any zoom
      glMatrixMode (GL_PROJECTION);
      glPushMatrix(); //save old projection
      glLoadIdentity ();
      glOrtho( -w()/2.0, w()/2.0, -h()/2.0, h()/2.0, -100, 100 );	
      glMatrixMode (GL_MODELVIEW);

      glPushMatrix();
      glLoadIdentity();
      glDisable( GL_DEPTH_TEST );

      // if trails are on, we need to clear the clock background

      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

      colorstack.Push( 0.8,0.8,1.0 ); // pale blue
      glRectf( -w()/2, -h()/2, -w()/2 +120, -h()/2+20 );
      colorstack.Pop();

      char clockstr[50];
      world->ClockString( clockstr, 50 );

      colorstack.Push( 0,0,0 ); // black
      gl_draw_string( -w()/2+4, -h()/2+4, 5, clockstr );
      colorstack.Pop();

      glEnable( GL_DEPTH_TEST );
      glPopMatrix();

      //restore camera projection
      glMatrixMode (GL_PROJECTION);
      glPopMatrix();
      glMatrixMode (GL_MODELVIEW);
    }

  
  if( showScreenshots )
    Screenshot();
}


void StgCanvas::Screenshot()
{

  int width = w();
  int height = h();
  int depth = 4; // RGBA

  // we use RGBA throughout, though we only need RGB, as the 4-byte
  // pixels avoid a nasty word-alignment problem when indexing into
  // the pixel array.

  // might save a bit of time as the image size rarely changes
  static uint8_t* pixels = NULL;  
  pixels = g_renew( uint8_t, pixels, width * height * depth );
  
  glFlush(); // make sure the drawing is done
  // read the pixels from the screen
  glReadPixels( 0,0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels );			 
  
  static uint32_t count = 0;		 
  char filename[64];
  snprintf( filename, 63, "stage-%d.png", count++ );
  
  FILE *fp = fopen( filename, "wb" );
  if( fp == NULL ) 
    {
      PRINT_ERR1( "Unable to open %s", filename );
    }
  
  // create PNG data
  png_structp pp = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  assert(pp);
  png_infop info = png_create_info_struct(pp);
  assert(info);

  // setup the output file
  png_init_io(pp, fp);

  // need to invert the image as GL and PNG disagree on the row order
  png_bytep rowpointers[height];
  for( int i=0; i<height; i++ )
    rowpointers[i] = &pixels[ (height-1-i) * width * depth ];

  png_set_rows( pp, info, rowpointers ); 

  //png_set_compression_level(pp, Z_DEFAULT_COMPRESSION);
  png_set_IHDR( pp, info, 
		width, height, 8, 
		PNG_COLOR_TYPE_RGBA, 
		PNG_INTERLACE_NONE, 
		PNG_COMPRESSION_TYPE_DEFAULT, 
		PNG_FILTER_TYPE_DEFAULT);

  png_write_png( pp, info, PNG_TRANSFORM_IDENTITY, NULL );

  // free the PNG data - we reuse the pixel data next time.
  png_destroy_write_struct(&pp, &info);
  
  fclose(fp);
  
  printf( "Saved %s\n", filename );
}


void StgCanvas::CreateMenuItems( Fl_Menu_Bar* menu, std::string path )
{
  showData.CreateMenuItem( menu, path );
  showBlocks.CreateMenuItem( menu, path );
  showFlags.CreateMenuItem( menu, path );
  showClock.CreateMenuItem( menu, path );
  showFlags.CreateMenuItem( menu, path );
  showFollow.CreateMenuItem( menu, path );
  showFootprints.CreateMenuItem( menu, path );
  showGrid.CreateMenuItem( menu, path );
  showStatus.CreateMenuItem( menu, path );
  showOccupancy.CreateMenuItem( menu, path );
  showTrailArrows.CreateMenuItem( menu, path );
  showTrails.CreateMenuItem( menu, path );
  showTrailRise.CreateMenuItem( menu, path );  
  showTree.CreateMenuItem( menu, path );  
  showScreenshots.CreateMenuItem( menu, path );  
}


void StgCanvas::Load( Worldfile* wf, int sec )
{
  float x = wf->ReadTupleFloat(sec, "center", 0, 0 );
  float y = wf->ReadTupleFloat(sec, "center", 1, 0 );
  camera.setPose( x, y );
  
  camera.setPitch( wf->ReadTupleFloat( sec, "rotate", 0, 0 ) );
  camera.setYaw( wf->ReadTupleFloat( sec, "rotate", 1, 0 ) );

  camera.setScale( wf->ReadFloat(sec, "scale", camera.getScale() ) );
  interval = wf->ReadInt(sec, "interval", interval );
  
  showData.Load( wf, sec );
  showFlags.Load( wf, sec );
  showBlocks.Load( wf, sec );
  showClock.Load( wf, sec );
  showFollow.Load( wf, sec );
  showFootprints.Load( wf, sec );
  showGrid.Load( wf, sec );
  showOccupancy.Load( wf, sec );
  showTrailArrows.Load( wf, sec );
  showTrailRise.Load( wf, sec );
  showTrails.Load( wf, sec );
  showTree.Load( wf, sec );
  showScreenshots.Load( wf, sec );

  invalidate(); // we probably changed something
}

void StgCanvas::Save( Worldfile* wf, int sec )
{
  printf( "canvas save section %d\n", sec  );
  wf->WriteFloat( sec, "scale", camera.getScale() );
  
  wf->WriteTupleFloat( sec, "center", 0, camera.getX() );
  wf->WriteTupleFloat( sec, "center", 1, camera.getY() );
  
  wf->WriteTupleFloat( sec, "rotate", 0, camera.getPitch()  );
  wf->WriteTupleFloat( sec, "rotate", 1, camera.getYaw()  );
  
  wf->WriteFloat(sec, "scale", camera.getScale() );
  wf->WriteInt(sec, "interval", interval );

  showData.Save( wf, sec );
  showBlocks.Save( wf, sec );
  showClock.Save( wf, sec );
  showFlags.Save( wf, sec );
  showFollow.Save( wf, sec );
  showFootprints.Save( wf, sec );
  showGrid.Save( wf, sec );
  showOccupancy.Save( wf, sec );
  showTrailArrows.Save( wf, sec );
  showTrailRise.Save( wf, sec );
  showTrails.Save( wf, sec );
  showTree.Save( wf, sec );
  showScreenshots.Save( wf, sec );
}


void StgCanvas::draw()
{
  static bool loaded_texture = false;

  //Enable the following to debug camera model
  //	if( loaded_texture == true && use_perspective_camera == true )
  //		return;

  if (!valid() ) 
    { 
      valid(1);
      FixViewport(w(), h());

      // set gl state that won't change every redraw
      glClearColor ( 0.7, 0.7, 0.8, 1.0);
      //glClearColor ( 1,1,1,1 );
      glDisable( GL_LIGHTING );
      glEnable( GL_DEPTH_TEST );
      glDepthFunc( GL_LESS );
      glCullFace( GL_BACK );
      glEnable (GL_CULL_FACE);
      glEnable( GL_BLEND );
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      glEnable( GL_LINE_SMOOTH );
      glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
      glDepthMask( GL_TRUE );
      glEnable( GL_TEXTURE_2D );


      //TODO find a better home for loading textures
      if( loaded_texture == false ) {
	std::string fullpath = world->fileMan->fullPath( "stall.png" );
	if ( fullpath == "" ) {
	  PRINT_DEBUG( "Unable to load texture.\n" );
	}
			
	GLuint stall_id = TextureManager::getInstance().loadTexture( fullpath.c_str() );
	TextureManager::getInstance()._stall_texture_id = stall_id;

	//create floor texture
	{
	  //TODO merge this code into the textureManager
	  int i, j;
	  for (i = 0; i < checkImageHeight; i++) 
	    for (j = 0; j < checkImageWidth; j++) 
	      {			
		int even = (i+j)%2;
		checkImage[i][j][0] = (GLubyte) 255 - 10*even;
		checkImage[i][j][1] = (GLubyte) 255 - 10*even;
		checkImage[i][j][2] = (GLubyte) 255;// - 5*even;
		checkImage[i][j][3] = 255;
	      }
				
				
	  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	  glGenTextures(1, &texName);		 
	  glBindTexture(GL_TEXTURE_2D, texName);
	  glEnable(GL_TEXTURE_2D);
				
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 
		       0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
				
	  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	}
			
	loaded_texture = true;
      }

      // install a font
      gl_font( FL_HELVETICA, 12 );

      if( use_perspective_camera == true ) {
	perspective_camera.setAspect( static_cast< float >( w() ) / static_cast< float >( h() ) );
	perspective_camera.SetProjection();
      } else {
	stg_bounds3d_t extent = world->GetExtent();
	camera.SetProjection( w(), h(), extent.y.min, extent.y.max );
	camera.Draw();
      }

      // enable vertex arrays
      glEnableClientState( GL_VERTEX_ARRAY );
      //glEnableClientState( GL_COLOR_ARRAY );

      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }            

	
  if( use_perspective_camera == true ) {
    if( showFollow  && last_selection ) {
      //Follow the selected robot
      stg_pose_t gpose = last_selection->GetGlobalPose();
      perspective_camera.setPose( gpose.x, gpose.y, 0.2 );
      perspective_camera.setYaw( rtod( gpose.a ) - 90.0 );
			
    }
    perspective_camera.Draw();
  } else if( showFollow  && last_selection ) {
    //Follow the selected robot
    stg_pose_t gpose = last_selection->GetGlobalPose();
    camera.setPose( gpose.x, gpose.y );
		
    stg_bounds3d_t extent = world->GetExtent();
    camera.SetProjection( w(), h(), extent.y.min, extent.y.max );
    camera.Draw();
  }
	
  renderFrame();
}

void StgCanvas::resize(int X,int Y,int W,int H) 
{
  Fl_Gl_Window::resize(X,Y,W,H);
  FixViewport(W,H);
  invalidate();
}

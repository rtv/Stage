/** canvas.cc
    Implement the main world viewing area in FLTK and OpenGL. 
    Authors: Richard Vaughan (vaughan@sfu.ca)
             Alex Couture-Beil (asc17@sfu.ca)
             Jeremy Asher (jra11@sfu.ca)
    $Id$
*/

#include "stage_internal.hh"
#include "texture_manager.hh"
#include "replace.h"

#include <string>
#include <map>
#include <sstream>
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
  showFlags( "Flags", "show_flags", "l",  true ),
  showFollow( "Follow", "show_follow", "f", false ),
  showFootprints( "Footprints", "show_footprints", "o", false ),
  showGrid( "Grid", "show_grid", "g", true ),
  showOccupancy( "Debug/Occupancy", "show_occupancy", "^o", false ),
  showScreenshots( "Save screenshots", "screenshots", "", false ),
  showStatus( "Status", "show_status", "s", true ),
  showTrailArrows( "Trails/Rising Arrows", "show_trailarrows", "^a", false ),
  showTrailRise( "Trails/Rising blocks", "show_trailrise", "^r", false ),
  showTrails( "Trails/Fast", "show_trailfast", "^f", false ),
  showTree( "Debug/Tree", "show_tree", "^t", false ),
  pCamOn( "Perspective camera", "pcam_on", "r", false ),
  visualizeAll( "Visualize All", "vis_all", "^v", true ) 
{
  end();

  //show(); // must do this so that the GL context is created before configuring GL
  // but that line causes a segfault in Linux/X11! TODO: test in OS X

  this->world = world;
  selected_models = NULL;
  last_selection = NULL;
  wf = NULL;

  perspective_camera.setPose( 0.0, -4.0, 3.0 );
  current_camera = &camera;
  setDirtyBuffer();
	
  startx = starty = -1;
  //panx = pany = stheta = sphi = 0.0;
  //scale = 15.0;
  interval = 50; //msec between redraws

  graphics = true;

  // // start the timer that causes regular redraws
  Fl::add_timeout( ((double)interval/1000), 
		   (Fl_Timeout_Handler)StgCanvas::TimerCallback, 
		   this);
}

StgCanvas::~StgCanvas()
{
}

StgModel* StgCanvas::getModel( int x, int y )
{
  // render all models in a unique color
  make_current(); // make sure the GL context is current
  glClearColor ( 1,1,1,1 ); // white
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glLoadIdentity();
	current_camera->SetProjection();
	current_camera->Draw();

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
  
  // useful for debugging the picker
  //Screenshot();
	
  return mod;
}

bool StgCanvas::selected( StgModel* mod ) {
	if( g_list_find( selected_models, mod ) )
		return true;
	else
		return false;
}

void StgCanvas::select( StgModel* mod ) {
	if( mod )
    {
		last_selection = mod;
		selected_models = g_list_prepend( selected_models, mod );
//		mod->Disable();
    }
}

void StgCanvas::unSelect( StgModel* mod ) {
	if( mod )
	{
		if ( GList* link = g_list_find( selected_models, mod ) ) 
		{
			// remove it from the selected list
			selected_models = 
			g_list_remove_link( selected_models, link );
//			mod->Enable();
		}
	}  
}

void StgCanvas::unSelectAll() { 
//	for( GList* it=selected_models; it; it=it->next )
//		((StgModel*)it->data)->Enable();
	
	g_list_free( selected_models );
	selected_models = NULL;
}

// convert from 2d window pixel to 3d world coordinates
void StgCanvas::CanvasToWorld( int px, int py, 
			       double *wx, double *wy, double* wz )
{
	if( px <= 0 )
		px = 1;
	else if( px >= w() )
		px = w() - 1;
	if( py <= 0 )
		py = 1;
	else if( py >= h() )
		py = h() - 1;
	
	//redraw the screen only if the camera model isn't active.
	//TODO new selection technique will simply use drawfloor to result in z = 0 always and prevent strange behaviours near walls
	//TODO refactor, so glReadPixels reads (then caches) the whole screen only when the camera changes.
	if( true || dirtyBuffer() ) {
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		current_camera->SetProjection();
		current_camera->Draw();
		DrawFloor(); //call this rather than renderFrame for speed - this won't give correct z values
		dirty_buffer = false;
	}
	
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
		if( pCamOn == true ) {
			perspective_camera.scroll( Fl::event_dy() / 10.0 );
		}
		else {
			camera.scale( Fl::event_dy(),  Fl::event_x(), w(), Fl::event_y(), h() );
		}
		invalidate();
		redraw();
		return 1;
			
	case FL_MOVE: // moused moved while no button was pressed
		if ( startx >=0 ) {
			// mouse pointing to valid value
			
			if( Fl::event_state( FL_CTRL ) )
			{
				int dx = Fl::event_x() - startx;
				int dy = Fl::event_y() - starty;

				if( pCamOn == true ) {
					perspective_camera.addYaw( -dx );
					perspective_camera.addPitch( -dy );
				} 
				else {
					camera.addPitch( - 0.5 * static_cast<double>( dy ) );
					camera.addYaw( - 0.5 * static_cast<double>( dx ) );
				}
				invalidate();
				redraw();
			}
			else if( Fl::event_state( FL_ALT ) )
			{   
				int dx = Fl::event_x() - startx;
				int dy = Fl::event_y() - starty;

				if( pCamOn == true ) {
					perspective_camera.move( -dx, dy, 0.0 );
				} 
				else {
					camera.move( -dx, dy );
				}
				invalidate();
			}
		}
		startx = Fl::event_x();
		starty = Fl::event_y();
		return 1;

	case FL_PUSH: // button pressed
	  {
	    StgModel* mod = getModel( startx, starty );
	    startx = Fl::event_x();
	    starty = Fl::event_y();
	    selectedModel = false;
	    switch( Fl::event_button() )
	      {
	      case 1:
        clicked_empty_space = ( mod == NULL );
				  empty_space_startx = startx;
				  empty_space_starty = starty;
		if( mod ) { 
		  // clicked a model
		  if ( Fl::event_state( FL_SHIFT ) ) {
		    // holding shift, toggle selection
		    if ( selected( mod ) ) 
		      unSelect( mod );
		    else {
		      select( mod );
		      selectedModel = true; // selected a model
		    }
		  }
		  else {
		    if ( !selected( mod ) ) {
		      // clicked on an unselected model while
		      //  not holding shift, this is the new
		      //  selection
		      unSelectAll();
		      select( mod );
		    }
		    selectedModel = true; // selected a model
		  }
		}
		
		return 1;
	      case 3:
		{
		  // leave selections alone
		  // rotating handled within FL_DRAG
		  return 1;
		}
	      default:
		return 0;
	      }    
	  }
	  
	case FL_DRAG: // mouse moved while button was pressed
	{
		int dx = Fl::event_x() - startx;
		int dy = Fl::event_y() - starty;

		if ( Fl::event_state( FL_BUTTON1 ) && Fl::event_state( FL_CTRL ) == false ) {
			// Left mouse button drag
			if ( selectedModel ) {
				// started dragging on a selected model
				
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
			else {
				// started dragging on empty space or an
				//  unselected model, move the canvas
				if( pCamOn == true ) {
					perspective_camera.move( -dx, dy, 0.0 );
				} 
				else {
					camera.move( -dx, dy );
				}
				invalidate(); // so the projection gets updated
			}
		}
		else if ( Fl::event_state( FL_BUTTON3 ) || ( Fl::event_state( FL_BUTTON1 ) &&  Fl::event_state( FL_CTRL )  ) ) {
				// rotate all selected models
				for( GList* it = selected_models; it; it=it->next )
				{
					StgModel* mod = (StgModel*)it->data;
					mod->AddToPose( 0,0,0, 0.05*(dx+dy) );
				}
		}
		
		startx = Fl::event_x();
		starty = Fl::event_y();

		redraw();
		return 1;
	} // end case FL_DRAG

	case FL_RELEASE:   // mouse button released
			if( empty_space_startx == Fl::event_x() && empty_space_starty == Fl::event_y() && clicked_empty_space == true ) {
			  // clicked on empty space, unselect all
			  unSelectAll();
			}
		return 1;

	case FL_FOCUS:
	case FL_UNFOCUS:
		//.... Return 1 if you want keyboard events, 0 otherwise
		return 1;

	case FL_KEYBOARD:
		switch( Fl::event_key() )
		{
		case 'p': // pause
			world->TogglePause();
			break;
		case ' ': // space bar
			//current_camera->reset();
			if ( wf )
				current_camera->Load( wf, wf->LookupEntity( "window" ) );
			else
				current_camera->reset();
					
			//invalidate();
			if( Fl::event_state( FL_CTRL ) ) {
				resetCamera();
			}
			redraw();
			break;			
		case FL_Left:
			if( pCamOn == false ) { camera.move( -10, 0 ); } 
			else { perspective_camera.strafe( -0.5 ); } break;
		case FL_Right: 
			if( pCamOn == false ) {camera.move( 10, 0 ); } 
			else { perspective_camera.strafe( 0.5 ); } break;
		case FL_Down:  
			if( pCamOn == false ) {camera.move( 0, -10 ); } 
			else { perspective_camera.forward( -0.5 ); } break;
		case FL_Up:  
			if( pCamOn == false ) {camera.move( 0, 10 ); } 
			else { perspective_camera.forward( 0.5 ); } break;
		default:
			return 0; // keypress unhandled
		}
		
		invalidate(); // update projection
		return 1;
			
//	case FL_SHORTCUT:
//		//... shortcut, key is in Fl::event_key(), ascii in Fl::event_text()
//		//... Return 1 if you understand/use the shortcut event, 0 otherwise...
//		return 1;
	default:
		// pass other events to the base class...
		//printf( "EVENT %d\n", event );
		return Fl_Gl_Window::handle(event);
			
    } // end switch( event )
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
  glColor3f( 1.0, 1.0, 1.0 );

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
	
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(2.0, 2.0);
	
  glColor4f( 1.0, 1.0, 1.0, 1.0 );
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

void StgCanvas::resetCamera()
{
	float max_x = 0, max_y = 0, min_x = 0, min_y = 0;
	
	//TODO take orrientation ( `a' ) and geom.pose offset into consideration
	for( GList* it=world->StgWorld::children; it; it=it->next ) {
		StgModel* ptr = (StgModel*) it->data;
		stg_pose_t pose = ptr->GetPose();
		stg_geom_t geom = ptr->GetGeom();
		
		float tmp_min_x = pose.x - geom.size.x / 2.0;
		float tmp_max_x = pose.x + geom.size.x / 2.0;
		float tmp_min_y = pose.y - geom.size.y / 2.0;
		float tmp_max_y = pose.y + geom.size.y / 2.0;
		
		if( tmp_min_x < min_x ) min_x = tmp_min_x;
		if( tmp_max_x > max_x ) max_x = tmp_max_x;
		if( tmp_min_y < min_y ) min_y = tmp_min_y;
		if( tmp_max_y > max_y ) max_y = tmp_max_y;
	}
	
	//do a complete reset
	float x = ( min_x + max_x ) / 2.0;
	float y = ( min_y + max_y ) / 2.0;
	camera.setPose( x, y );
	float scale_x = w() / (max_x - min_x) * 0.9;
	float scale_y = h() / (max_y - min_y) * 0.9;
	camera.setScale( scale_x < scale_y ? scale_x : scale_y );
	
	//TODO reset perspective cam
}

void StgCanvas::renderFrame()
{
	//before drawing, order all models based on distance from camera
	float x = current_camera->x();
	float y = current_camera->y();
	float sphi = -dtor( current_camera->yaw() );
	
	//estimate point of camera location - hard to do with orthogonal mode
	x += -sin( sphi ) * 100;
	y += -cos( sphi ) * 100;
	
	// TODO - keep this map around in between frames, because the order
	// changes slowly, and sorting an already-sorted list is
	// usually very fast (rtv)

	//store all models in a sorted multimap
	std::multimap< float, StgModel* > ordered;
	for( GList* it=world->StgWorld::children; it; it=it->next ) {
		StgModel* ptr = (StgModel*) it->data;
		stg_pose_t pose = ptr->GetPose();
		
		float dist = sqrt( ( x - pose.x ) * ( x - pose.x ) + ( y - pose.y ) * ( y - pose.y ) );
		ordered.insert( std::pair< float, StgModel* >( dist, (StgModel*)it->data ) );
	}
	
	//now the models can be iterated over with:
	// for( std::multimap< float, StgModel* >::reverse_iterator i = ordered.rbegin(); i != ordered.rend(); i++ )
	
  if( ! showTrails )
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  if( showGrid )
    DrawGlobalGrid();
  else
    DrawFloor();
  
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
		
		for( std::multimap< float, StgModel* >::reverse_iterator i = ordered.rbegin(); i != ordered.rend(); i++ ) {
			i->second->DrawTrailFootprint();
		}
      glEnable( GL_DEPTH_TEST );
    }
  
  if( showTrailRise )
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
		
		for( std::multimap< float, StgModel* >::reverse_iterator i = ordered.rbegin(); i != ordered.rend(); i++ ) {
			i->second->DrawTrailBlocks();
		}
		
    }
  
  if( showTrailArrows )
    {
      glEnable( GL_DEPTH_TEST );
		for( std::multimap< float, StgModel* >::reverse_iterator i = ordered.rbegin(); i != ordered.rend(); i++ ) {
			i->second->DrawTrailArrows();
		}
		
    }

  if( showBlocks )
    DrawBlocks();
  
  // draw the model-specific visualizations
	if( showData ) {
		if ( visualizeAll ) {
			for( GList* it = world->StgWorld::children; it; it=it->next ) 
				((StgModel*)it->data)->DataVisualizeTree();
		}
		else if ( selected_models ) {
			for( GList* it = selected_models; it; it=it->next ) 
				((StgModel*)it->data)->DataVisualizeTree();
		}
		else if ( last_selection ) {
			last_selection->DataVisualizeTree();
		}
	}
  
  if( showGrid ) 
	  for( std::multimap< float, StgModel* >::reverse_iterator i = ordered.rbegin(); i != ordered.rend(); i++ ) {
		  i->second->DrawGrid();
	  }
	
		
  if( showFlags ) 
	  for( std::multimap< float, StgModel* >::reverse_iterator i = ordered.rbegin(); i != ordered.rend(); i++ ) {
		  i->second->DrawFlagList();
	  }
	
		
  if( showBlinken ) 
	for( std::multimap< float, StgModel* >::reverse_iterator i = ordered.rbegin(); i != ordered.rend(); i++ ) {
		i->second->DrawBlinkenlights();
	}
	
	
	if ( showStatus ) {
		glPushMatrix();
		for( std::multimap< float, StgModel* >::reverse_iterator i = ordered.rbegin(); i != ordered.rend(); i++ ) {
			//ensure two icons can't be in the exact same plane
			if( camera.pitch() == 0 )
				glTranslatef( 0, 0, 0.1 );
			i->second->DrawStatusTree( this );
		}
		glPopMatrix();
	}
	
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
      glOrtho( 0, w(), 0, h(), -100, 100 );	
      glMatrixMode (GL_MODELVIEW);

      glPushMatrix();
      glLoadIdentity();
      glDisable( GL_DEPTH_TEST );

      // if trails are on, we need to clear the clock background

      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		std::string clockstr = world->ClockString();
		if( showFollow == true && last_selection )
			clockstr.append( " [ FOLLOW MODE ]" );

		int margin = 3;
		int width = gl_width( clockstr.c_str() ) + 2 * margin;
		int height = gl_height() + 2 * margin;
		
		colorstack.Push( 0.8,0.8,1.0 ); // pale blue
		glRectf( 0, 0, width, height );
		colorstack.Pop();
		
      colorstack.Push( 0,0,0 ); // black
      gl_draw_string( margin, margin, 5, clockstr.c_str() );
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

void StgCanvas::perspectiveCb( Fl_Widget* w, void* p ) 
{
	StgCanvas* canvas = static_cast<StgCanvas*>( w );
	Option* opt = static_cast<Option*>( p ); // pCamOn
	if ( opt ) {
		// Perspective mode is on, change camera
		canvas->current_camera = &canvas->perspective_camera;
	}
	else {
		canvas->current_camera = &canvas->camera;
	}
	
	canvas->invalidate();
}

void StgCanvas::createMenuItems( Fl_Menu_Bar* menu, std::string path )
{
  showData.createMenuItem( menu, path );
//  visualizeAll.createMenuItem( menu, path );
  showBlocks.createMenuItem( menu, path );
  showFlags.createMenuItem( menu, path );
  showClock.createMenuItem( menu, path );
  showFlags.createMenuItem( menu, path );
  showFollow.createMenuItem( menu, path );
  showFootprints.createMenuItem( menu, path );
  showGrid.createMenuItem( menu, path );
  showStatus.createMenuItem( menu, path );
  pCamOn.createMenuItem( menu, path );
  pCamOn.menuCallback( perspectiveCb, this );
  showOccupancy.createMenuItem( menu, path );
  showTrailArrows.createMenuItem( menu, path );
  showTrails.createMenuItem( menu, path );
  showTrailRise.createMenuItem( menu, path );  
  showTree.createMenuItem( menu, path );  
  showScreenshots.createMenuItem( menu, path );  
}


void StgCanvas::Load( Worldfile* wf, int sec )
{
  this->wf = wf;
  camera.Load( wf, sec );
  perspective_camera.Load( wf, sec );		
	
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
  pCamOn.Load( wf, sec );

  invalidate(); // we probably changed something
}

void StgCanvas::Save( Worldfile* wf, int sec )
{
  camera.Save( wf, sec );
  perspective_camera.Save( wf, sec );	
	
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
  pCamOn.Save( wf, sec );
}


void StgCanvas::draw()
{
  static bool loaded_texture = false;

  //Enable the following to debug camera model
  //	if( loaded_texture == true && pCamOn == true )
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
		glEnableClientState( GL_VERTEX_ARRAY );

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
				
	  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
			
	loaded_texture = true;
      }

      // install a font
      gl_font( FL_HELVETICA, 12 );

      if( pCamOn == true ) {
	perspective_camera.setAspect( static_cast< float >( w() ) / static_cast< float >( h() ) );
	perspective_camera.SetProjection();
		  current_camera = &perspective_camera;
      } else {
	stg_bounds3d_t extent = world->GetExtent();
	camera.SetProjection( w(), h(), extent.y.min, extent.y.max );
		  current_camera = &camera;
      }

      // enable vertex arrays
      glEnableClientState( GL_VERTEX_ARRAY );
      //glEnableClientState( GL_COLOR_ARRAY );

      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }            

  //Follow the selected robot	
  if( showFollow  && last_selection ) {
    stg_pose_t gpose = last_selection->GetGlobalPose();
    if( pCamOn == true ) {
      perspective_camera.setPose( gpose.x, gpose.y, 0.2 );
      perspective_camera.setYaw( rtod( gpose.a ) - 90.0 );
    } else {
      camera.setPose( gpose.x, gpose.y );
    }
  }

  current_camera->Draw();	
  renderFrame();
}

void StgCanvas::resize(int X,int Y,int W,int H) 
{
  Fl_Gl_Window::resize(X,Y,W,H);
  FixViewport(W,H);
  invalidate();
}

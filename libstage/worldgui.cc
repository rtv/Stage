
#include "stage.hh"
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Button.H>

extern void glPolygonOffsetEXT (GLfloat, GLfloat);

void dummy_cb(Fl_Widget*, void* v) 
{
}

void view_toggle_cb(Fl_Widget* w, bool* view ) 
{
  assert(view);
  *view = !(*view);
  
  Fl_Menu_Item* item = (Fl_Menu_Item*)((Fl_Menu_*)w)->mvalue();
  *view ?  item->check() : item->clear();
}


void timer_cb( void* v)
{
  StgWorldGui* w = (StgWorldGui*)v;
  w->redraw();
  
  if( w->show_clock )  
    w->redraw_overlay();
  
  Fl::repeat_timeout(0.1, timer_cb, v);
}

StgWorldGui::StgWorldGui(int W,int H,const char*L) 
  : Fl_Gl_Window(0,0,W,H,L)
{
  //Fl_Menu_Bar* menubar = new Fl_Menu_Bar(0,0, W, 30);// 640, 30);
  //menubar->menu(menuitems);  
  //menubar->show();

  selected_models = NULL;
  graphics = true;
  dragging = false;

  startx = starty = 0;
  panx = pany = stheta = sphi = 0.0;
  scale = 15.0;

  fg = 1.0;
  bg = 0.0;
  resizable(this);
  end();

  follow_selection = false;
  show_clock = true;
  show_quadtree = false;  
  show_occupancy = false;  
  show_geom = false;
  show_boxes = true;
  show_grid = false;
  show_data = true;
  show_cfg = false;
  show_cmd = false;
  show_thumbnail = false;
  show_trails = false;


  // build the menus
  menu = new Fl_Menu_Button( 0,0,0,0 );
  menu->type( Fl_Menu_Button::POPUP3 );

  //menu->add( "&File", 0, 0, 0, FL_SUBMENU );
  //menu->add( "File/&Save File", FL_CTRL + 's', (Fl_Callback *)dummy_cb );
  //menu->add( "File/Save File &As...", FL_CTRL + FL_SHIFT + 's', (Fl_Callback *)dummy_cb, 0, FL_MENU_DIVIDER );
  //menu->add( "File/E&xit", FL_CTRL + 'q', (Fl_Callback *)dummy_cb, 0 );
  
  //menu->add( "&View", 0, 0, 0, FL_SUBMENU );
  menu->add( "Data", FL_CTRL+'z', (Fl_Callback *)view_toggle_cb, &show_data, FL_MENU_TOGGLE|FL_MENU_VALUE );  
  menu->add( "Blocks", FL_CTRL+'b', (Fl_Callback *)view_toggle_cb, &show_boxes, FL_MENU_TOGGLE|FL_MENU_VALUE );  
  //menu->add( "View/Clock", FL_CTRL+'c', (Fl_Callback *)view_toggle_cb, &show_clock, FL_MENU_TOGGLE|FL_MENU_VALUE );  
  menu->add( "Grid",      FL_CTRL + 'c', (Fl_Callback *)view_toggle_cb, &show_grid, FL_MENU_TOGGLE|FL_MENU_VALUE );
  menu->add( "Occupancy", FL_CTRL + 'o', (Fl_Callback *)view_toggle_cb, &show_occupancy, FL_MENU_TOGGLE|FL_MENU_VALUE );
  menu->add( "Tree",      FL_CTRL + 't', (Fl_Callback *)view_toggle_cb, &show_quadtree, FL_MENU_TOGGLE|FL_MENU_VALUE );

  //menu->add( "Help", 0, 0, 0, FL_SUBMENU );
  //menu->add( "Help/About Stage...", FL_CTRL + 'f', (Fl_Callback *)dummy_cb );
  //menu->add( "Help/HTML Documentation", FL_CTRL + 'g', (Fl_Callback *)dummy_cb );
 
  show(); // must do this so that the GL context is created before configuring GL
 
  // set gl state that won't change every redraw
  glClearColor ( 0.7, 0.7, 0.8, 1.0);
  glDisable(GL_LIGHTING);
  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  glCullFace( GL_BACK );
  glEnable (GL_CULL_FACE);  
  glEnable( GL_BLEND );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_LINE_SMOOTH );
  glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);
  glDepthMask(GL_TRUE);

  // install a font
  gl_font( FL_HELVETICA, 12 );
  
  // start the timer that causes regular redraws
  Fl::add_timeout( 0.1, timer_cb, this);
 }

bool StgWorldGui::RealTimeUpdate()
{
  //  puts( "RTU" );
  bool res = StgWorld::RealTimeUpdate();
  Fl::check();
  return res;
}

bool StgWorldGui::Update()
{
  //  puts( "RTU" );
  bool res = StgWorld::Update();
  Fl::check();
  return res;
}


void StgWorldGui::AddModel( StgModel*  mod  )
{
  mod->InitGraphics();
  StgWorld::AddModel( mod );
}

StgModel* StgWorldGui::Select( int x, int y )
{
  // render all models in a unique color
  make_current(); // make sure the GL context is current
  glClearColor ( 0,0,0,1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glDisable(GL_DITHER);
  
  // render all top-level, draggable models in a color that is their
  // id + a 100% alpha value
  for( GList* it=StgWorld::children; it; it=it->next )
    {
      StgModel* mod = (StgModel*)it->data;      

      if( mod->gui_mask & STG_MOVE_TRANS )
	{
	  uint32_t col = (mod->id | 0xFF000000);
	  glColor4ubv( (GLubyte*)&col );     
	  mod->DrawPicker();
	}
    }
  
  // read the color of the pixel in the back buffer under the mouse
  // pointer
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);
  
  uint32_t id;
  glReadPixels( x,viewport[3]-y,1,1,
		GL_RGBA,GL_UNSIGNED_BYTE,(void*)&id );
  
  // strip off the alpha channel byte to retrieve the model id
  id &= 0x00FFFFFF;
  
  StgModel* mod = (StgModel*)
    g_hash_table_lookup( models_by_id, &id );
  
  printf("%p %s %d\n", mod, mod ? mod->Token() : "", id );
  
  glEnable(GL_DITHER);
  glClearColor ( 0.7, 0.7, 0.8, 1.0);
  
  if( mod ) // we clicked on a root model
    {
      // if it's already selected
      if( GList* link = g_list_find( selected_models, mod ) )
	{
	  // remove it from the selected list
	  selected_models = 
	    g_list_remove_link( selected_models, link );

	  mod->disabled = false;
	}			  
      else      
	{
	  selected_models = g_list_prepend( selected_models, mod );
	  mod->disabled = true;
	}
   
      invalidate();
    }

  return mod;
}

void StgWorldGui::CanvasToWorld( int px, int py, 
				 double *wx, double *wy, double* wz )
{
  // convert from window pixel to world coordinates
  //int width = widget->allocation.width;

  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview); 

  GLdouble projection[16];	
  glGetDoublev(GL_PROJECTION_MATRIX, projection);	

  GLfloat pz;
 
  glReadPixels( px, h()-py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pz );
  gluUnProject( px, w()-py, pz, modelview, projection, viewport, wx,wy,wz );

  //printf( "Z: %.2f\n", pz );
  //printf( "FINAL: x:%.2f y:%.2f z:%.2f\n",
  //  *wx,*wy,zz );
}

int StgWorldGui::handle(int event) 
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
	  scale -= 0.5 * (double)Fl::event_dy();
	  if( scale < 1 )
	    scale = 1;    
	  invalidate();
	}
      return 1;

    case FL_MOVE: // moused moved while no button was pressed
      //printf( "mouse moved %d %d\n", Fl::event_x(), Fl::event_y() );
      //printf( "mouse delta %d %d\n", Fl::event_dx(), Fl::event_dy() );
      
      if( Fl::event_state( FL_CTRL ) )
	{          
	  int dx = Fl::event_x() - startx; 
	  int dy = Fl::event_y() - starty;
	    
	  stheta -= 0.01 * (double)dy;
	  sphi += 0.01 * (double)dx;
	  stheta = constrain( stheta, 0, M_PI/2.0 );
	  invalidate();
	}

      if( Fl::event_state( FL_ALT ) )
	{          
	  int dx = Fl::event_x() - startx; 
	  int dy = Fl::event_y() - starty;
	    
	  panx -= dx;
	  pany += dy; 
	  invalidate();
	}
      
      startx = Fl::event_x();
      starty = Fl::event_y();
      
      return 1;

    case FL_PUSH: // button pressed
      puts( "PUSH" );
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
	    puts( "POPUP" );
	    menu->popup();
	    return 1;
	  }
	default:
	  return 0;
	}    
      
    case FL_DRAG: // mouse moved while button was pressed
      {
	int dx = Fl::event_x() - startx; 
	int dy = Fl::event_y() - starty;
	
	if( Fl::event_button() == 1 )
	  {
	    if( selected_models && dragging )
	      {
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
		panx -= dx;
		pany += dy; 
	      }
	  }
	
	startx = Fl::event_x();
	starty = Fl::event_y();
	invalidate();
      }
      return 1;
	
    case FL_RELEASE:   // mouse button released
      // unselect everyone unless shift is pressed
      if( ! Fl::event_state( FL_SHIFT ) )
	{
	  for( GList* it=selected_models; it; it=it->next )
	    ((StgModel*)it->data)->disabled = false;
	  
	  g_list_free( selected_models );
	  selected_models = NULL;
	  dragging = false;
	  invalidate();
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
	  paused = !paused;
	  break;
	case ' ': // space bar
	  sphi = stheta = 0.0;
	  invalidate();
	  break;
	case FL_Left:  panx += 10; break;
	case FL_Right: panx -= 10; break;
	case FL_Down:    pany += 10; break;
	case FL_Up:  pany -= 10; break;
	default:
	  return 0; // keypress unhandled
	}
      invalidate();
      return 1;
      //case FL_SHORTCUT:
      ///... shortcut, key is in Fl::event_key(), ascii in Fl::event_text()
      // ... Return 1 if you understand/use the shortcut event, 0 otherwise...
      //return 1;
    default:
      // pass other events to the base class...
      printf( "EVENT %d\n", event );
      return Fl_Gl_Window::handle(event);
    }
}

void StgWorldGui::FixViewport(int W,int H) 
{
  glLoadIdentity();
  glViewport(0,0,W,H);
}

void StgWorldGui::draw() 
{
  if (!valid()) 
    { 
      valid(1); 
      FixViewport(w(), h()); 
      
      double zclip = hypot(width, height) * scale;
      double pixels_width =  w();//canvas->allocation.width;
      double pixels_height = h();//canvas->allocation.height ;
      
      //double width = w() / scale;
      //double height = h() / scale;
      
      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      
      // map the viewport to pixel units by scaling it the same as the window
      //  glOrtho( 0, pixels_width,
      //   0, pixels_height,
      
      glOrtho( -pixels_width/2.0, pixels_width/2.0,
	       -pixels_height/2.0, pixels_height/2.0,
	       0, zclip );
      
      /*   glFrustum( 0, pixels_width, */
      /* 	     0, pixels_height, */
      /* 	     0.1, zclip ); */
      
      glMatrixMode (GL_MODELVIEW);
      
      glLoadIdentity ();
      glTranslatef(  -panx, 
		     -pany, 
		     -zclip / 2.0 );
      
      glRotatef( RTOD(-stheta), 1,0,0);  
      
      //glTranslatef(  -panx * cos(sphi) - pany*sin(sphi), 
      //	 panx*sin(sphi) - pany*cos(sphi), 0 );
      
      glRotatef( RTOD(sphi), 0.0, 0.0, 1.0);   // rotate about z - pitch
      
      // meter scale
      glScalef ( scale, scale, scale ); // zoom
    }      

  // Clear screen to bg color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  // Draw 'X' in fg color
  //glColor3f(fg, fg, fg);
  //glBegin(GL_LINE_STRIP); glVertex2f(w(), h()); glVertex2f(-w(),-h()); glEnd();
  //glBegin(GL_LINE_STRIP); glVertex2f(w(),-h()); glVertex2f(-w(), h()); glEnd();

  //gluLookAt( 0,0,10,
  //   0,0,0,
  //   0,1,0 );

/*   gluLookAt( win->panx,  */
/* 	     win->pany + win->stheta,  */
/* 	     10, */
/*     	     win->panx,  */
/* 	     win->pany,   */
/* 	     0, */
/* 	     sin(win->sphi), */
/* 	     cos(win->sphi),  */
/* 	     0 ); */

//   if( follow_selection && selected_models )
//     {      
//       glTranslatef(  0,0,-zclip/2.0 );

//       // meter scale
//       glScalef ( scale, scale, scale ); // zoom

//       // get the pose of the model at the head of the selected models
//       // list
//       stg_pose_t gpose;
//       ((StgModel*)selected_models->data)->GetGlobalPose( &gpose );

//       //glTranslatef( -gpose.x + (pixels_width/2.0)/scale, 
//       //	    -gpose.y + (pixels_height/2.0)/scale,
//       //	    -zclip/2.0 );      

//       // pixel scale  
//       glTranslatef(  -gpose.x, 
// 		     -gpose.y, 
// 		     0 );

//     }
//   else
//     {
      // pixel scale  
      //glTranslatef( 0,0, -MAX(width,height) );

      // TODO - rotate around the mouse pointer or window center, not the
      //origin  - the commented code is close, but not quite right
      //glRotatef( RTOD(sphi), 0.0, 0.0, 1.0);   // rotate about z - pitch
      //glTranslatef( -click_point.x, -click_point.y, 0 ); // shift back            //printf( "panx %f pany %f scale %.2f stheta %.2f sphi %.2f\n",
      //  panx, pany, scale, stheta, sphi );
      //}
  
//   // draw the world size rectangle in white
//   // draw the floor a little pushed into the distance so it doesn't z-fight with the
  // models
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  glColor3f( 1,1,1 );
  glRectf( -width/2.0, -height/2.0,
	   width/2.0, height/2.0 ); 
  glDisable(GL_POLYGON_OFFSET_FILL);

  // draw the model bounding boxes
  //g_hash_table_foreach( models, (GHFunc)model_draw_bbox, NULL);
  
  //PRINT_DEBUG1( "Colorstack depth %d",
  //	colorstack.Length() );

  if( show_quadtree || show_occupancy )
     {
      glDisable( GL_LINE_SMOOTH );
      glLineWidth( 1 );

      glPushMatrix();
      glTranslatef( -width/2.0, -height/2.0, 1 );
      glScalef( 1.0/ppm, 1.0/ppm, 0 );
      glPolygonMode( GL_FRONT, GL_LINE );
      colorstack.Push(1,0,0);
      
      if( show_occupancy )
	this->bgrid->Draw( false );

      if( show_quadtree )
	this->bgrid->Draw( true );
      
      colorstack.Pop();
      glPopMatrix();  
      
      glEnable( GL_LINE_SMOOTH );
     }

  colorstack.Push( 1, 0, 0 );
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth( 4 );
  
  for( GList* it=selected_models; it; it=it->next )
    {
      ((StgModel*)it->data)->DrawSelected();
      //((StgModel*)it->data)->DrawPicker();
    }  
  colorstack.Pop();
  
  glLineWidth( 1 );
  
  // draw the models
  GList* it;
  if( show_boxes )
    for( it=StgWorld::children; it; it=it->next )
      ((StgModel*)it->data)->Draw();
  
  // draw the models' data
  if( show_data )
    for( it=StgWorld::children; it; it=it->next )
      ((StgModel*)it->data)->DrawData();
  
  // draw anything in the assorted displaylist list
//   for( it = dl_list; it; it=it->next )
//     {
//       int dl = (int)it->data;
//       printf( "Calling dl %d\n", dl );
//       glCallList( dl );
//    }
}

void StgWorldGui::draw_overlay() 
{
  if( show_clock )
    {
      char clockstr[50];
      ClockString( clockstr, 50 );
      
      glPushMatrix();
      glLoadIdentity();
      
      colorstack.Push( 0,0,0 ); // black
      gl_draw( clockstr, -w()/2.0 + 5, -h()/2 + 5 );
      colorstack.Pop();
      glPopMatrix();
    }
}


void StgWorldGui::resize(int X,int Y,int W,int H) 
{
  Fl_Gl_Window::resize(X,Y,W,H);
  FixViewport(W,H);
  invalidate();
}


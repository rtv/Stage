
#include "stage.hh"
#include <FL/fl_draw.H>
#include <FL/fl_Box.H>
#include <FL/Fl_Menu_Button.H>

void StgCanvas::TimerCallback( StgCanvas* c )
{
  c->redraw();
  
  Fl::repeat_timeout(((double)c->interval/1000),
		     (Fl_Timeout_Handler)StgCanvas::TimerCallback, 
		     c);
}


StgCanvas::StgCanvas( StgWorld* world, int x, int y, int w, int h) 
  : Fl_Gl_Window(x,y,w,h)
{
  end();
  show(); // must do this so that the GL context is created before configuring GL

  this->world = world;
  selected_models = NULL;
  last_selection = NULL;

  startx = starty = 0;
  panx = pany = stheta = sphi = 0.0;
  scale = 15.0;
  interval = 100; //msec between redraws

  graphics = true;
  dragging = false;
  rotating = false;

  showflags = STG_SHOW_CLOCK | STG_SHOW_DATA | STG_SHOW_BLOCKS | STG_SHOW_GRID;
 
  // start the timer that causes regular redraws
  Fl::add_timeout( ((double)interval/1000), 
		   (Fl_Timeout_Handler)StgCanvas::TimerCallback, 
		   this);
 }

StgCanvas::~StgCanvas()
{
}

StgModel* StgCanvas::Select( int x, int y )
{
  // render all models in a unique color
  make_current(); // make sure the GL context is current
  glClearColor ( 0,0,0,1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glDisable(GL_DITHER);
  
  // render all top-level, draggable models in a color that is their
  // id + a 100% alpha value
  for( GList* it=world->children; it; it=it->next )
    {
      StgModel* mod = (StgModel*)it->data;      

      if( mod->gui_mask & (STG_MOVE_TRANS | STG_MOVE_ROT ))
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
    g_hash_table_lookup( world->models_by_id, &id );
  
  //printf("%p %s %d\n", mod, mod ? mod->Token() : "", id );
  
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
	  last_selection = mod;
	  selected_models = g_list_prepend( selected_models, mod );
	  mod->disabled = true;
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
	  scale -= 0.5 * (double)Fl::event_dy();
	  if( scale < 1 )
	    scale = 1;    

	  //if( width

	  invalidate();
	}
      return 1;

    case FL_MOVE: // moused moved while no button was pressed
      if( Fl::event_state( FL_CTRL ) )
	{          
	  int dx = Fl::event_x() - startx; 
	  int dy = Fl::event_y() - starty;
	    
	  stheta -= 0.01 * (double)dy;
	  sphi += 0.01 * (double)dx;
	  stheta = constrain( stheta, 0, M_PI/2.0 );
	  invalidate();
	}
      else if( Fl::event_state( FL_ALT ) )
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
	    puts( "button 3" );
	  startx = Fl::event_x();
	  starty = Fl::event_y();
	  if( Select( startx, starty ) )
	    {
	      printf( "rotating" );
	      rotating = true;	  
	    }	    
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
		panx -= dx;
		pany += dy; 
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
      return 1; // end case FL_DRAG
      
    case FL_RELEASE:   // mouse button released
      // unselect everyone unless shift is pressed
      if( ! Fl::event_state( FL_SHIFT ) )
	{
	  for( GList* it=selected_models; it; it=it->next )
	    ((StgModel*)it->data)->disabled = false;
	  
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
	  world->paused = !world->paused;
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

void StgCanvas::draw() 
{
  if (!valid()) 
    { 
      valid(1); 
      FixViewport(w(), h()); 

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
      
//   glClearColor ( 0.7, 0.7, 0.8, 1.0);
//   glDisable(GL_LIGHTING);
//   glEnable (GL_DEPTH_TEST);
//   glDepthFunc (GL_LESS);
//   glCullFace( GL_BACK );
//   glEnable (GL_CULL_FACE);  
//   glEnable( GL_BLEND );
//   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//   glEnable( GL_LINE_SMOOTH );
//   glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);
//   glDepthMask(GL_TRUE);

  // install a font
      //  gl_font( FL_HELVETICA, 12 );

      double zclip = hypot(world->width, world->height) * scale;
      double pixels_width =  w();
      double pixels_height = h();
      
      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      
      // map the viewport to pixel units by scaling it the same as the window
      glOrtho( -pixels_width/2.0, pixels_width/2.0,
	       -pixels_height/2.0, pixels_height/2.0,
	       0, zclip );
      
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
  
  if( (showflags & STG_SHOW_FOLLOW)  && last_selection )
    {      
      glLoadIdentity ();
      double zclip = hypot(world->width, world->height) * scale;
      glTranslatef(  0,0,
      	     -zclip / 2.0 );
      
      // meter scale
      glScalef ( scale, scale, scale ); // zoom
      
      stg_pose_t gpose;
      last_selection->GetGlobalPose( &gpose );
      
      // and put it in the center of the window
      glTranslatef(  -gpose.x, -gpose.y, 0 );
     }
   
   // draw the world size rectangle in white, using the polygon offset
   // so it doesn't z-fight with the models
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(1.0, 1.0);
   glColor3f( 1,1,1 );
   glRectf( -world->width/2.0, -world->height/2.0,
	    world->width/2.0, world->height/2.0 ); 
   glDisable(GL_POLYGON_OFFSET_FILL);

   if( (showflags & STG_SHOW_QUADTREE) || (showflags & STG_SHOW_OCCUPANCY) )
     {
       glDisable( GL_LINE_SMOOTH );
       glLineWidth( 1 );
       
       glPushMatrix();
       glTranslatef( -world->width/2.0, -world->height/2.0, 1 );
       glScalef( 1.0/world->ppm, 1.0/world->ppm, 0 );
       glPolygonMode( GL_FRONT, GL_LINE );
       colorstack.Push(1,0,0);
       
      if( showflags & STG_SHOW_OCCUPANCY )
	world->bgrid->Draw( false );

      if( showflags & STG_SHOW_QUADTREE )
	world->bgrid->Draw( true );
      
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
      //      ((StgModel*)it->data)->DrawPicker();
    }  
  colorstack.Pop();
  
  glLineWidth( 1 );
  
  
  // draw the models
  GList* it;
  if( showflags ) // if any bits are set there's something to draw
    for( it=world->children; it; it=it->next )
    ((StgModel*)it->data)->Draw( showflags );
    
  if( showflags && STG_SHOW_CLOCK )  
    redraw_overlay();
}

void StgCanvas::draw_overlay() 
{
  char clockstr[50];
  world->ClockString( clockstr, 50 );
  
  glPushMatrix();
  glLoadIdentity();
  
  colorstack.Push( 0,0,0 ); // black
  gl_draw( clockstr, -w()/2.0 + 5, -h()/2 + 5 );
  colorstack.Pop();
  glPopMatrix();
}

void StgCanvas::resize(int X,int Y,int W,int H) 
{
  Fl_Gl_Window::resize(X,Y,W,H);
  FixViewport(W,H);
  invalidate();
}



/**************************************************************************
 * Header file inclusions.
 **************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkglglext.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
//#include <GLUT/glut.h>
#include <GL/glx.h> // for XFont for drawing text from X fonts

#include "model.hh"
 
static GdkGLConfig *glconfig = NULL;
//static GdkGLConfig *gllaser = NULL;

GList* dl_list = NULL;
int dl_debug; // a displaylist used for debugging stuff

#include "stage_internal.h"
#include "gui.h"

typedef GLfloat point3[3];

int pointer_list = 0;

/**************************************************************************
 * The following section contains all the macro definitions.
 **************************************************************************/

//#define SCALE_MIN 0.01

#define DEFAULT_WIDTH  300
#define DEFAULT_HEIGHT 200

#define THICKNESS 0.25

#define THUMBNAILHEIGHT 100
//#define LIST_BODY "_gl_body"
#define LIST_DATA "_gl_data"
#define LIST_GRID "_gl_grid"
#define LIST_SELECTED "_gl_selected"

/**************************************************************************
 * Global variable declarations.
 **************************************************************************/

static gboolean animate = TRUE;

static int beginX, beginY;

static float lightPosition[4] = {0.0, 0.0, 1.0, 1.0};

static GQueue* colorstack = NULL;

/**************************************************************************
 * The following section contains the function prototype declarations.
 **************************************************************************/

static GtkWidget   *create_popup_menu (GtkWidget   *drawing_area);

StgModel* stg_world_nearest_root_model( stg_world_t* world, double wx, double wy );

static int recticle_list = 0;

GLuint  base=0; /* Base Display List For The Font Set */

/* function to recover memory form our list of characters */
GLvoid KillFont( GLvoid )
{
    glDeleteLists( base, 96 );

    return;
}

/* function to build our font list */
GLvoid buildFont( GLvoid )
{
    Display *dpy;          /* Our current X display */
    XFontStruct *fontInfo; /* Our font info */

    /* Storage for 96 characters */
    base = glGenLists( 96 );
    assert( base > 0 );
   
    /* Get our current display long enough to get the fonts */
    dpy = XOpenDisplay( NULL );
    
    /* Get the font information */
    if( (fontInfo = XLoadQueryFont( dpy, "fixed" )) ==  NULL )
      {   
	/* If that font doesn't exist, something is wrong */
	PRINT_ERR( "no X font \"fixed\" available. Giving up." );
	exit( 1 );
      }
    
    /* generate the list */
    glXUseXFont( fontInfo->fid, 32, 96, base );

    /* Recover some memory */
    XFreeFont( dpy, fontInfo );

    /* close the display now that we're done with it */
    XCloseDisplay( dpy );

    //puts( "FINISHED BUILDING FONT" );

    return;
}


/* Print our GL text to the screen */
GLvoid glPrint( const char *fmt, ... )
{
  // first call, we generate a bitmap for each character
  if( base == 0 )
    // generate font bitmap drawing lists
    buildFont();

    char text[256]; /* Holds our string */
    va_list ap;     /* Pointer to our list of elements */

    /* If there's no text, do nothing */
    if ( fmt == NULL )
        return;

    /* Parses The String For Variables */
    va_start( ap, fmt );
      /* Converts Symbols To Actual Numbers */
      vsprintf( text, fmt, ap );
    va_end( ap );

    // use this method in case the glPushAttriv() version below is too
    // slow on OS X
    
 /*        int c; */
/*         int len = strlen(text); */
/*         for( c=0; c<len; c++ ) */
/*           glCallList( base - 32 + text[c] ); */
    
    // More elegant version, faster on some machines
    glPushAttrib( GL_LIST_BIT ); // push/pop attrib is slow on OS X
    glListBase( base - 32 );
    glCallLists( strlen( text ), GL_UNSIGNED_BYTE, text );
    glPopAttrib( );
}


// transform the current coordinate frame by the given pose
void gl_coord_shift( double x, double y, double z, double a  )
{
  glTranslatef( x,y,z );
  glRotatef( RTOD(a), 0,0,1 );
}

// transform the current coordinate frame by the given pose
void gl_pose_shift( stg_pose_t* pose )
{
  gl_coord_shift( pose->x, pose->y, pose->z, pose->a );
}

void print_color_stack( char* msg )
{
  if(msg==NULL)
    msg = "";

  printf( "color stack (%s):\n", msg );

  size_t c;
  for(c=0; c<colorstack->length; c++ )
    {      
      GLdouble* color = (GLdouble*)g_queue_peek_nth( colorstack, c );
      
      printf( "   %.2f %.2f %.2f %.2f\n",
	      color[0],
	      color[1],
	      color[2],
	      color[3] );
    }
}


void stg_color_to_glcolor4dv( stg_color_t scol, double gcol[4] )
{
  gcol[0] = ((scol & 0x00FF0000) >> 16) / 256.0;
  gcol[1] = ((scol & 0x0000FF00) >> 8)  / 256.0;
  gcol[2] = ((scol & 0x000000FF) >> 0)  / 256.0;
  gcol[3] = 1.0 - (((scol & 0xFF000000) >> 24) / 256.0);
}

// stash this color 
void push_color( GLdouble col[4] )
{
  // create the stack before first use
  if( colorstack == NULL )
    colorstack = g_queue_new();

  size_t sz =  4 * sizeof(col[0]);  
  GLdouble *keep = (GLdouble*)malloc( sz );
  memcpy( keep, col, sz );

  g_queue_push_head( colorstack, keep );

  // and select this color in GL
  glColor4dv( col );

  //print_color_stack( "push" );
}


// a convenience wrapper around push_color(), with a=1
void push_color_rgb( double r, double g, double b )
{
  GLdouble col[4];
  col[0] = r;
  col[1] = g;
  col[2] = b;
  col[3] = 1.0;
  
  push_color( col );
}

// a convenience wrapper around push_color()
void push_color_rgba( double r, double g, double b, double a )
{
  GLdouble col[4];
  col[0] = r;
  col[1] = g;
  col[2] = b;
  col[3] = a;
  
  push_color( col );
}

void push_color_stgcolor( stg_color_t col )
{
  GLdouble d[4];

  d[0] = ((col & 0x00FF0000) >> 16) / 256.0;
  d[1] = ((col & 0x0000FF00) >> 8)  / 256.0;
  d[2] = ((col & 0x000000FF) >> 0)  / 256.0;
  d[3] = 1.0 - (((col & 0xFF000000) >> 24) / 256.0);

  push_color( d );
}


// reset GL to the color we stored using store_color()
void pop_color( void )
{
  assert( colorstack != NULL );
  assert( ! g_queue_is_empty( colorstack ) );

  GLdouble *col = (GLdouble*)g_queue_pop_head( colorstack );
  glColor4dv( col ); 
  free( col );

  //print_color_stack( "pop" );
}


void gui_invalidate_window( stg_world_t* world )
{
  gdk_window_invalidate_rect( world->win->canvas->window, 
			      &world->win->canvas->allocation, FALSE);
}


void gui_enable_alpha( stg_world_t* world, int enable )
{
  if( enable )
    { 
      glEnable(GL_LINE_SMOOTH); 
      glEnable(GL_BLEND);
      world->win->show_alpha = TRUE;
    }
  else
    {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH); 
      world->win->show_alpha = FALSE;
    }

  world->win->dirty = true;
}

void widget_to_world( GtkWidget* widget, int px, int py, 
		      stg_world_t* world, double *wx, double *wy, double* wz )
{
  // convert from window pixel to world coordinates
  int width = widget->allocation.width;
  int height = widget->allocation.height;

  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview); 

  GLdouble projection[16];	
  glGetDoublev(GL_PROJECTION_MATRIX, projection);	

  GLfloat pz;
 
  glReadPixels( px, height-py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pz );
  gluUnProject( px, height-py, pz, modelview, projection, viewport, wx,wy,wz );

  //printf( "Z: %.2f\n", pz );
  //printf( "FINAL: x:%.2f y:%.2f z:%.2f\n",
  //  *wx,*wy,zz );
}

void box3d_wireframe( stg_bbox3d_t* bbox )
{
  // TODO - this would be more efficient with a point array
  
  // bottom rectangle
  glBegin(GL_LINE_LOOP );
  glVertex3f( bbox->x.min, bbox->y.min, bbox->z.min );
  glVertex3f( bbox->x.min, bbox->y.max, bbox->z.min );
  glVertex3f( bbox->x.max, bbox->y.max, bbox->z.min );
  glVertex3f( bbox->x.max, bbox->y.min, bbox->z.min );
  glEnd();

  // top rectangle
  glBegin(GL_LINE_LOOP );
  glVertex3f( bbox->x.min, bbox->y.min, bbox->z.max );
  glVertex3f( bbox->x.min, bbox->y.max, bbox->z.max );
  glVertex3f( bbox->x.max, bbox->y.max, bbox->z.max );
  glVertex3f( bbox->x.max, bbox->y.min, bbox->z.max );
  glEnd();

  // verticals
  glBegin( GL_LINES );
  glVertex3f( bbox->x.min, bbox->y.min, bbox->z.min );
  glVertex3f( bbox->x.min, bbox->y.min, bbox->z.max );
  glVertex3f( bbox->x.max, bbox->y.min, bbox->z.min );
  glVertex3f( bbox->x.max, bbox->y.min, bbox->z.max );
  glVertex3f( bbox->x.min, bbox->y.max, bbox->z.min );
  glVertex3f( bbox->x.min, bbox->y.max, bbox->z.max );
  glVertex3f( bbox->x.max, bbox->y.max, bbox->z.min );
  glVertex3f( bbox->x.max, bbox->y.max, bbox->z.max );
  glEnd();
}


void make_dirty( StgModel* mod )
{
  //((gui_window_t*)mod->world->win)->dirty = TRUE;
}




/* int gl_ranger_render_data( stg_model_t* mod, void* enabled ) */
/* { */
/*   //puts ("GL ranger data" ); */
    
/*   int list = gui_model_get_displaylist( mod, LIST_DATA ); */

/*   // replaces any existing list with this ID */
/*   glNewList( list, GL_COMPILE ); */

/*   stg_ranger_sample_t* samples = (stg_ranger_sample_t*)mod->data;  */
/*   size_t sample_count = mod->data_len / sizeof(stg_ranger_sample_t); */
/*   stg_ranger_config_t *cfg = (stg_ranger_config_t*)mod->cfg; */
  
/*   if( (samples == NULL) | (sample_count < 1) ) */
/*     {  */
/*       glEndList(); */
/*       return 0; */
/*     } */

/*   assert( cfg ); */
      
/*   // now get on with rendering the laser data */

/*   glPushMatrix(); */

/*   // go into model coordinates */
/*   stg_pose_t gpose; */
/*   stg_model_get_global_pose( mod, &gpose ); */
  
/*   gl_pose_shift( &gpose ); */
/*   //glTranslatef( gpose.x, gpose.y, gpose.z ); */
/*   //glRotatef(RTOD(gpose.a), 0,0,1); */
      
/*   // local offset coords */
/*   //stg_geom_t geom; */
/*   //stg_model_get_geom( mod, &geom ); */
      
/*   glDepthMask( GL_FALSE ); */
  
/*   // respect the local offsets */
/*   gl_pose_shift( &mod->geom.pose ); */
/*   //glTranslatef( mod->geom.pose.x, mod->geom.pose.y, mod->geom.pose.z ); */
/*   //glRotatef(RTOD(mod->geom.pose.a), 0,0,1); */
  
/*   push_color_rgba( 1, 0, 0, 0.1 ); // transparent red */
  
/*   // draw the range  beams */
/*   int s; */
/*   for( s=0; s<sample_count; s++ ) */
/*     { */
/*       if( samples[s].range > 0.0 ) */
/* 	{ */
/* 	  stg_ranger_config_t* rngr = &cfg[s]; */
	  
/* 	  // sensor FOV */
/* 	  double sidelen = samples[s].range; */
/* 	  double da = rngr->fov/2.0; */
	  
/* 	  double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - da ); */
/* 	  double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - da ); */
/* 	  double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + da ); */
/* 	  double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + da ); */
	  
/* 	  if( mod->world->win->show_alpha ) */
/* 	    glBegin(GL_POLYGON); */
/* 	  else */
/* 	    glBegin(GL_LINE_LOOP); */
	    
/*  	    glVertex2f( rngr->pose.x, rngr->pose.y ); */
/* 	    glVertex2f( x1, y1 ); */
/* 	    glVertex2f( x2, y2 ); */
/* 	  glEnd();	   */
/* 	} */
/*     } */
  
/*   glPopMatrix(); */

/*   // restore state */
/*   glDepthMask( GL_TRUE ); */
/*   pop_color(); */

/*   glEndList(); */

/*   make_dirty(mod); */

/*   return 0; // callback runs until removed */
/* } */



/* void gl_model_select( stg_model_t* mod, void* user ) */
/* {   */
/*   // get the display list associated with this model */
/*   int list = gui_model_get_displaylist( mod, LIST_SELECTED ); */
  
/*   glNewList( list, GL_COMPILE ); */

/*   glPushMatrix(); */
/*   //glPushAttrib( GL_ALL_ATTRIB_BITS ); */

/*   push_color_rgba( 1, 0, 0, 1 ); */
  
/*   double dx = mod->geom.size.x / 2.0 * 1.3; */
/*   double dy = mod->geom.size.y / 2.0 * 1.3; */

/*   //glTranslatef( 0,0,0.1 ); */

/*   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE ); */
/*   glRectf( -dx, -dy, dx, dy ); */
/*   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL ); */

/*   pop_color(); */

/*   glPopMatrix(); */
/*   //glPopAttrib(); */
/*   glEndList(); */
/* } */

void gl_model_selected( StgModel* mod, void* user )
{
  glPushMatrix();

  stg_pose_t pose;
  mod->GetPose( &pose );
  stg_geom_t geom;
  mod->GetGeom( &geom );

  gl_pose_shift( &pose );
  gl_pose_shift( &geom.pose );

  double dx = geom.size.x / 2.0 * 1.3;
  double dy = geom.size.y / 2.0 * 1.3;

  //glTranslatef( 0,0,0.1 );

  glRectf( -dx, -dy, dx, dy );

  pop_color();
  //pop_color();

  glPopMatrix();
}


/* void gl_model_grid_axes( stg_model_t* mod ) */
/* { */
/*   double dx = mod->geom.size.x; */
/*   double dy = mod->geom.size.y; */
/*   double sp = 1.0; */
  
/*   int nx = (int) ceil((dx/2.0) / sp); */
/*   int ny = (int) ceil((dy/2.0) / sp); */
  
/*   if( nx == 0 ) nx = 1.0; */
/*   if( ny == 0 ) ny = 1.0; */
  
/*   int i; */
/*   for (i = -nx+1; i < nx; i++) */
/*     { */
/*       glRasterPos2f( -0.2 + (i*sp), -0.2 ); */
/*       glPrint( "%d", (int)i );       */
/*     } */
  
/*   for (i = -ny+1; i < ny; i++) */
/*     { */
/*       glRasterPos2f( -0.2, -0.2 + (i * sp) ); */
/*       glPrint( "%d", (int)i );       */
      
/*       //snprintf( str, 64, "%d", (int)i ); */
/*       //stg_rtk_fig_text( fig, -0.2, -0.2 + (oy + i * sp) , 0, str ); */
/*     } */
/* } */




void stg_d_render( stg_d_draw_t* d )
{
  //printf( "STG_D DRAW COMMAND %d\n", d->type );
  
  switch( d->type )
    {
    case STG_D_DRAW_POINTS:
      glPointSize( 3 );
      glBegin( GL_POINTS );
      break;	      
    case STG_D_DRAW_LINES:
      glBegin( GL_LINES );
      break;
    case STG_D_DRAW_LINE_STRIP:
      glBegin( GL_LINE_STRIP );
      break;
    case STG_D_DRAW_LINE_LOOP:
      glBegin( GL_LINE_LOOP );
      break;
    case STG_D_DRAW_TRIANGLES:
      glBegin( GL_TRIANGLES );
      break;
    case STG_D_DRAW_TRIANGLE_STRIP:
      glBegin( GL_TRIANGLE_STRIP );
      break;
    case STG_D_DRAW_TRIANGLE_FAN:
      glBegin( GL_TRIANGLE_FAN );
      break;
    case STG_D_DRAW_QUADS:
      glBegin( GL_QUADS );
      break;
    case STG_D_DRAW_QUAD_STRIP:
      glBegin( GL_QUAD_STRIP );
      break;
    case STG_D_DRAW_POLYGON:
      glBegin( GL_POLYGON );
      break;
      
    default:
      PRINT_WARN1( "drawing mode %d not handled", 
		   d->type );	 
    }
  
  unsigned int c;
  for( c=0; c<d->vert_count; c++ )
    {
      glVertex3f( d->verts[c].x,
		  d->verts[c].y,
		  d->verts[c].z );
    }
  glEnd();
}




/**************************************************************************
 * The following section contains all the callback function definitions.
 **************************************************************************/

/***
 *** The "realize" signal handler. All the OpenGL initialization
 *** should be performed here, such as default background colour,
 *** certain states etc.
 ***/
void 
realize (GtkWidget *widget,
	 gpointer   data)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  GdkGLProc proc = NULL;

  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return;

  /* glPolygonOffsetEXT */
  proc = gdk_gl_get_glPolygonOffsetEXT ();
  if (proc == NULL)
    {
      /* glPolygonOffset */
      proc = gdk_gl_get_proc_address ("glPolygonOffset");
      if (proc == NULL)
	{
	  g_print ("Sorry, glPolygonOffset() is not supported by this renderer.\n");
	  exit (1);
	}
    }

  glDisable(GL_LIGHTING);
 
  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  glClearColor ( 0.7, 0.7, 0.8, 1.0);

  gdk_gl_glPolygonOffsetEXT (proc, 1.0, 1.0);

  glCullFace( GL_BACK );
  glEnable (GL_CULL_FACE);

  //glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  //glEnable( GL_POLYGON_SMOOTH );
  
  // set gl state that won't change every redraw
  glEnable( GL_BLEND );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_LINE_SMOOTH );
  glHint (GL_LINE_SMOOTH_HINT, GL_FASTEST);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  gdk_gl_drawable_gl_end (gldrawable);

  return;
}

/***
 *** The "configure_event" signal handler. Any processing required when
 *** the OpenGL-capable drawing area is re-configured should be done here.
 *** Almost always it will be used to resize the OpenGL viewport when
 *** the window is resized.
 ***/
gboolean
configure_event (GtkWidget         *widget,
		 GdkEventConfigure *event,
		 gpointer           data)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  GLfloat w = widget->allocation.width;
  GLfloat h = widget->allocation.height;

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;

  //aspect = (float)w/(float)h;
  glViewport (0, 0, w, h);

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  return TRUE;
}


/* void draw_thumbnail( stg_world_t* world ) */
/* { */
/*   //double boxwidth = THUMBNAILWIDTH; // pixels */
/*   double boxheight = THUMBNAILHEIGHT;//boxwidth / (world->width/(double)world->height); */
/*   double boxwidth = boxheight / (world->height/(double)world->width); */

/*   glPushAttrib( GL_ALL_ATTRIB_BITS ); */

/*   glViewport(0,0,boxwidth, boxheight ); */

/*   glMatrixMode (GL_PROJECTION); */
/*   glPushMatrix();   */
/*   glLoadIdentity(); */

/*   glOrtho( 0,1,0,1,-1,1); */

/*   glMatrixMode (GL_MODELVIEW); */
/*   glPushMatrix(); */
/*   glLoadIdentity (); */

/*   glTranslatef( 0,0,-0.9 ); */




/*   glEnable(GL_POLYGON_OFFSET_FILL); */
/*   glPolygonOffset(1.0, 1.0); */
/*   glColor3f( 1,1,1 );       */
/*   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); */
/*   glRecti( 0,0,1,1 ); // fill the viewport */
/*   glDisable(GL_POLYGON_OFFSET_FILL); */
  
/*   glColor3f( 0,0,0 );       */
/*   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); */
/*   glRecti( 0,0,1,1 ); // fill the viewport */
  
/*   glTranslatef( 0.5,0.5,0 ); */
/*   glScalef( 1.0/world->width, 1.0/world->height, 1.0); */
  
/*   // don't render grids in the thumbnail */
/*   int keep_grid  = world->win->show_grid; */
/*   int keep_data = world->win->show_data; */
/*   world->win->show_data = 0; */
/*   world->win->show_grid = 0; */

/*   g_list_foreach( world->children, (GFunc)model_draw_cb, NULL ); */

/*   world->win->show_grid = keep_grid; */
/*   world->win->show_data = keep_data; */
  
  
/* /\*   // if we're in 2d mode - show the visble region *\/ */
  
/* /\*   double left = -seex/2.0 * world->width - world->win->panx; *\/ */
/* /\*   double top = -seey/2.0 * world->height - world->win->pany; *\/ */
/* /\*   double right = seex/2.0 * world->width - world->win->panx; *\/ */
/* /\*   double bottom = seey/2.0 * world->height - world->win->pany; *\/ */
  
  
/* /\*   if( world->win->show_alpha ) *\/ */
/* /\*     { *\/ */
/* /\*       glColor4f( 0,0,0,0.15 );       *\/ */
/* /\*       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); *\/ */
      
/* /\*       glRectf( -world->width/2.0,  *\/ */
/* /\* 	       -world->height/2.0,  *\/ */
/* /\* 	       left, *\/ */
/* /\* 	       world->height/2.0 ); *\/ */
      
/* /\*       glRectf( left, *\/ */
/* /\* 	       -world->height/2.0,  *\/ */
/* /\* 	       world->width/2.0, *\/ */
/* /\* 	       top ); *\/ */
      
/* /\*       glRectf( left, *\/ */
/* /\* 	       bottom, *\/ */
/* /\* 	       world->width/2.0, *\/ */
/* /\* 	       world->height/2.0 ); *\/ */
      
/* /\*       glRectf( right, *\/ */
/* /\* 	       top, *\/ */
/* /\* 	       world->width/2.0, *\/ */
/* /\* 	       bottom); *\/ */
/* /\*     } *\/ */

/* /\*   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); *\/ */
/* /\*   glColor3f( 1.0,0,0.1 ); *\/ */
/* /\*   glRectf( left, top, right, bottom ); *\/ */
  
  
/*   glPopMatrix(); // restore GL_MODELVIEW */

/*   glMatrixMode(GL_PROJECTION); */
/*   glPopMatrix(); // restore GL_PROJECTION */


/*   glPopAttrib();       */
/* } */


/* static GLuint VertShader; */
/* static char * VertSrc =  */
/* "!!ARBvp1.0 \n\ */
/* PARAM mvp[4]={state.matrix.mvp}; \n\ */
/* TEMP r0,tmp; \n\ */
/* DP4  r0.x, mvp[0], vertex.position;	\n\ */
/* DP4  r0.y, mvp[1], vertex.position;	\n\ */
/* DP4  r0.z, mvp[2], vertex.position;	\n\ */
/* DP4  r0.w, mvp[3], vertex.position;	\n\ */
/* #RCP  tmp.w, r0.w; \n\ */
/* RCP tmp, 8; \n\ */
/* MUL tmp.w, r0.w, tmp.w; \n\ */
/* #MUL  r0.z, r0.z, r0.w; \n\  */
/* MUL  r0.z, r0.z, tmp.w; \n\  */
/* MOV  result.position, r0;    \n\ */
/* MOV  result.color, vertex.color;  \n\ */
/* END\n "; */

void draw_world(  stg_world_t* world )
{
  //puts( "draw_world" );

  gui_window_t* win = (gui_window_t*)world->win;
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable( win->canvas );

  // clear the offscreen buffer
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  double zclip = hypot(world->width, world->height) * win->scale;
  double pixels_width =  world->win->canvas->allocation.width;
  double pixels_height = world->win->canvas->allocation.height ;

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
/* 	     //0,1,0 ); */


  if( world->win->follow_selection && world->win->selected_models )
    {      
      glTranslatef(  0,0,-zclip/2.0 );

      // meter scale
      glScalef ( win->scale, win->scale, win->scale ); // zoom

      // get the pose of the model at the head of the selected models
      // list
      stg_pose_t gpose;
      ((StgModel*)world->win->selected_models->data)->GetGlobalPose( &gpose );

      //glTranslatef( -gpose.x + (pixels_width/2.0)/win->scale, 
      //	    -gpose.y + (pixels_height/2.0)/win->scale,
      //	    -zclip/2.0 );      

      // pixel scale  
      glTranslatef(  -gpose.x, 
		     -gpose.y, 
		     0 );

    }
  else
    {
      // pixel scale  
      glTranslatef(  -win->panx, 
		     -win->pany, 
		     -zclip / 2.0 );
      
      glRotatef( RTOD(-win->stheta), 1,0,0);  
      
      //glTranslatef(  -win->panx * cos(win->sphi) - win->pany*sin(win->sphi), 
      //	 win->panx*sin(win->sphi) - win->pany*cos(win->sphi), 0 );
      
      glRotatef( RTOD(win->sphi), 0.0, 0.0, 1.0);   // rotate about z - pitch
      
      // meter scale
      glScalef ( win->scale, win->scale, win->scale ); // zoom
      //glTranslatef( 0,0, -MAX(world->width,world->height) );

      // TODO - rotate around the mouse pointer or window center, not the
      //origin  - the commented code is close, but not quite right
      //glRotatef( RTOD(win->sphi), 0.0, 0.0, 1.0);   // rotate about z - pitch
      //glTranslatef( -win->click_point.x, -win->click_point.y, 0 ); // shift back            //printf( "panx %f pany %f scale %.2f stheta %.2f sphi %.2f\n",
      //  panx, pany, scale, stheta, sphi );
    }
  
  // draw the world size rectangle in white
  // draw the floor a little pushed into the distance so it doesn't z-fight with the
  // models
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  glColor3f( 1,1,1 );
  glRectf( -world->width/2.0, -world->height/2.0,
	   world->width/2.0, world->height/2.0 ); 
  glDisable(GL_POLYGON_OFFSET_FILL);

  // draw the model bounding boxes
  //g_hash_table_foreach( world->models, (GHFunc)model_draw_bbox, NULL);


  if( win->show_matrix )
    {
      glDisable( GL_LINE_SMOOTH );
      glLineWidth( 1 );
      glTranslatef( 0,0,1 );
      glPolygonMode( GL_FRONT, GL_LINE );
      push_color_rgb( 0,1,0 );
      stg_cell_render_tree( world->matrix->root );
      glTranslatef( 0,0,-1 );
      pop_color();
      glEnable( GL_LINE_SMOOTH );
    }

  push_color_rgb( 1, 0, 0 );
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth( 4 );
  g_list_foreach( world->win->selected_models, (GFunc)gl_model_selected, NULL );
  glLineWidth( 1 );

  // draw the models
  GList* it;
  for( it=world->children; it; it=it->next )
    ((StgModel*)it->data)->Draw();

  for( it=world->children; it; it=it->next )
    ((StgModel*)it->data)->DrawData();


  // draw anything in the assorted displaylist list
  for( it = dl_list; it; it=it->next )
    {
      int dl = (int)it->data;
      printf( "Calling dl %d\n", dl );
      glCallList( dl );
    }

  glCallList( dl_debug );

  //gl_coord_shift( 0,0,-2,0 );

  //if( win->show_thumbnail ) // if drawing the whole world mini-view
      //draw_thumbnail( world );

  //glEndList();

  /* Swap buffers */
  if (gdk_gl_drawable_is_double_buffered (gldrawable))
    gdk_gl_drawable_swap_buffers (gldrawable);
  else
    glFlush ();
}

gboolean
expose_event (GtkWidget      *widget,
	      GdkEventExpose *event,
	      stg_world_t* world )
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    return FALSE;

  draw_world( world );

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  return TRUE;
}



gboolean
scroll_event (GtkWidget      *widget,
	      GdkEventScroll *event,
	      stg_world_t* world )
{
  switch( event->direction ) 
    {
     case GDK_SCROLL_UP:
       //puts( "SCROLL UP" );
       world->win->scale *= 1.05;
       break;
     case GDK_SCROLL_DOWN:
       //puts( "SCROLL DOWN" );
       world->win->scale *= 0.95;
       break;
     default:
       break;
    }
  
   return TRUE;
}

// return a value based on val, but limited minval <= val >= maxval  
double constrain( double val, double minval, double maxval )
{
  if( val < minval )
    return minval;

  if( val > maxval )
    return maxval;
  
  return val;
}



/***
 *** The "motion_notify_event" signal handler. Any processing required when
 *** the OpenGL-capable drawing area is under drag motion should be done here.
 ***/
gboolean
motion_notify_event (GtkWidget      *widget,
		     GdkEventMotion *event,
		     stg_world_t* world )
{
  //printf( "mouse pointer %d %d\n",(int)event->x, (int)event->y );

  // only change the view angle if CTRL is pressed
  guint modifiers = gtk_accelerator_get_default_mod_mask ();

  if( (event->state & modifiers) == GDK_CONTROL_MASK)
    {
      if (event->state & GDK_BUTTON1_MASK)
	{
	  world->win->sphi += (float)(event->x - beginX) / 100.0;
	  world->win->stheta += (float)(beginY - event->y) / 100.0;
	  
	  // stop us looking underneath the world
	  world->win->stheta = constrain( world->win->stheta, 0, M_PI/2.0 );
	  world->win->dirty = TRUE; 
	}
    }
  else if( event->state & GDK_BUTTON1_MASK && world->win->dragging )
    {
      //printf( "dragging an object\n" );

      if( world->win->selected_models )
	{
	  // convert from window pixel to world coordinates
	  double obx, oby, obz;
	  widget_to_world( widget, event->x, event->y,
			   world, &obx, &oby, &obz );
	  
	  double dx = obx - world->win->selection_pointer_start.x; 
	  double dy = oby - world->win->selection_pointer_start.y; 

	  world->win->selection_pointer_start.x = obx;
	  world->win->selection_pointer_start.y = oby;

	  //printf( "moved mouse %.3f, %.3f meters\n", dx, dy );

	  // move all selected models by dx,dy 
	  for( GList* it = world->win->selected_models;
	       it;
	       it = it->next )
	    ((StgModel*)it->data)->AddToPose( dx, dy, 0, 0 );

	  world->win->dirty = TRUE; 
	}
    }  
  else if( event->state & GDK_BUTTON3_MASK && world->win->dragging ) 
    {
      //printf( "rotating an object\n" );
      
      if( world->win->selected_models )
	{
	  // convert from window pixel to world coordinates
	  double obx, oby, obz;
	  widget_to_world( widget, event->x, event->y,
			   world, &obx, &oby, &obz );
	  
	  //printf( "rotating model to %f %f\n", obx, oby );
	  
	  for( GList* it = world->win->selected_models;
	       it;
	       it = it->next )
	    {
	      double da = (event->x < beginX ) ? -0.15 : 0.15;
	      
	      ((StgModel*)it->data)->AddToPose( 0,0,0, da );
	    }
	  world->win->dirty = TRUE; 
	}
    }
  else
    {
      if( event->state & GDK_BUTTON1_MASK ) // PAN
	{	  
	  world->win->panx -= (event->x - beginX);
	  world->win->pany += (event->y - beginY);
	  world->win->dirty = TRUE; 

	}
      
      if (event->state & GDK_BUTTON3_MASK)
	{
	  if( event->x > beginX ) world->win->scale *= 1.04;
	  if( event->x < beginX ) world->win->scale *= 0.96;
	  //printf( "ZOOM scale %f\n", world->win->scale );
	  world->win->dirty = TRUE; 
	}
    }
  
  beginX = event->x;
  beginY = event->y;  

  return true;
}


gboolean
button_release_event (GtkWidget      *widget,
		      GdkEventButton *event,
		      stg_world_t*  world )
{
  //puts( "RELEASE" );

  // stop dragging models when mouse 1 is released
  if( event->button == 1 || event->button == 3 )
    {
      // only change the view angle if CTRL is pressed
      guint modifiers = gtk_accelerator_get_default_mod_mask ();
      
      if( ((event->state & modifiers) != GDK_CONTROL_MASK) &&
	  ((event->state & modifiers) != GDK_SHIFT_MASK) )
	{
	  world->win->dragging = FALSE;
	  
	  // clicked away from a model - clear selection
	  g_list_free( world->win->selected_models );
	  world->win->selected_models = NULL;
	  world->win->dirty = true;
	}
    }
  
  return TRUE;
}




/***
 *** The "button_press_event" signal handler. Any processing required when
 *** mouse buttons are pressed on the OpenGL-
 *** capable drawing area should be done here.
 ***/
gboolean
button_press_event (GtkWidget      *widget,
		    GdkEventButton *event,
		    stg_world_t*  world )
{
  // convert from window pixel to world coordinates
  double obx, oby, obz;
  widget_to_world( widget, event->x, event->y,
		   world, &obx, &oby, &obz );
  
  // ctrl is pressed - choose this point as the center of rotation
  //printf( "mouse click at (%.2f %.2f %.2f)\n", obx, oby, obz );
  
  world->win->click_point.x = obx;
  world->win->click_point.y = oby;
  
  beginX = event->x;
  beginY = event->y;

  if( event->button == 1 || event->button == 3 ) // select object
    {
      // only change the view angle if CTRL is pressed
      guint modifiers = gtk_accelerator_get_default_mod_mask ();
      
      StgModel* sel = stg_world_nearest_root_model( world, obx, oby );
      
      if( sel )
	{
	  // if model is already selected, unselect it
	  if( GList* link = g_list_find( world->win->selected_models, sel ) )
	    {
	      world->win->selected_models = 
		g_list_remove_link( world->win->selected_models, link );

	      world->win->dirty = true;
	      return true;
	    }			  
	    
	  //}	  

	  printf( "selected model %s\n", sel->Token() );
	  
	  //world->win->selection_active = sel;
	  
	  // store the pose of the selected object at the moment of
	  // selection
	  sel->GetPose( &world->win->selection_pose_start );
	  
	  // and store the 3d pose of the pointer
	  world->win->selection_pointer_start.x = obx;
	  world->win->selection_pointer_start.y = oby;
	  world->win->selection_pointer_start.z = obz;
	  
	  world->win->dragging = TRUE;
	  
	  // need to redraw to see the selection visualization
	  world->win->dirty = true;
	  
	  // unless shift is held, we empty the list of selected objects
	  if( (event->state & modifiers) != GDK_SHIFT_MASK)
	    {
	      g_list_free( world->win->selected_models );
	      world->win->selected_models = NULL;
	    }
	  
	  world->win->selected_models = 
	    g_list_prepend( world->win->selected_models, sel );
	}
    }
  
  return TRUE;
}

/* For popup menu. */
gboolean
button_press_event_popup_menu (GtkWidget      *widget,
			       GdkEventButton *event,
			       gpointer        data)
{
  if (event->button == 3)
    {
      /* Popup menu. */
      gtk_menu_popup (GTK_MENU(widget), NULL, NULL, NULL, NULL,
		      event->button, event->time);
      return TRUE;
    }

  return FALSE;
}

/***
 *** The "key_press_event" signal handler. Any processing required when key
 *** presses occur should be done here.
 ***/
gboolean
key_press_event (GtkWidget   *widget,
		 GdkEventKey *event,
		 stg_world_t* world )
{
  switch (event->keyval)
    {
    case GDK_plus:
      world->win->scale *= 0.9;
      break;

    case GDK_minus:
      world->win->scale *= 1.1;
      break;

    default:
      return FALSE;
    }

  return TRUE;
}

/***
 *** The "unrealize" signal handler. Any processing required when
 *** the OpenGL-capable window is unrealized should be done here.
 ***/
void
unrealize (GtkWidget *widget,
	   gpointer   data)
{
  // TODO
  puts( "Window unrealize" );
}



/**************************************************************************
 * The following section contains the GUI building function definitions.
 **************************************************************************/

/* /\*** */
/*  *** Creates the popup menu to be displayed. */
/*  ***\/ */
/*  GtkWidget * */
/* create_popup_menu (GtkWidget *drawing_area) */
/* { */
/*   GtkWidget *menu; */
/*   GtkWidget *menu_item; */

/*   menu = gtk_menu_new (); */

/*   /\* Toggle animation *\/ */
/*   menu_item = gtk_menu_item_new_with_label ("Toggle Animation"); */
/*   gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item); */

/*   g_signal_connect_swapped (G_OBJECT (menu_item), "activate", */
/* 			    G_CALLBACK (toggle_animation), drawing_area); */
/*   gtk_widget_show (menu_item); */

/*   /\* Init wireframe model *\/ */
/*   menu_item = gtk_menu_item_new_with_label ("Initialize"); */
/*   gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item); */
/*   g_signal_connect_swapped (G_OBJECT (menu_item), "activate", */
/* 			    G_CALLBACK (init_wireframe), drawing_area); */
/*   gtk_widget_show (menu_item); */

/*   /\* Quit *\/ */
/*   menu_item = gtk_menu_item_new_with_label ("Quit"); */
/*   gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item); */
/*   g_signal_connect (G_OBJECT (menu_item), "activate", */
/* 		    G_CALLBACK (gtk_main_quit), NULL); */
/*   gtk_widget_show (menu_item); */
	
/*   return menu; */
/* } */

GdkGLConfig* gl_init( void )
{
  gtk_gl_init (NULL,NULL);

  /* Configure OpenGL framebuffer. */
  /* Try double-buffered visual */
  glconfig = 
    gdk_gl_config_new_by_mode( (GdkGLConfigMode)
			       (GDK_GL_MODE_RGBA |
				GDK_GL_MODE_DEPTH |
				GDK_GL_MODE_DOUBLE) );
  
  if (glconfig == NULL)
    {
      g_print ("\n*** Cannot find the double-buffered visual.\n");
      g_print ("\n*** Trying single-buffered visual.\n");
      
      /* Try single-buffered visual */
      glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)
					   (GDK_GL_MODE_RGBA |
					    GDK_GL_MODE_DEPTH));
      if (glconfig == NULL)
	{
	  g_print ("*** No appropriate OpenGL-capable visual found.\n");
	  exit (1);
	}
    }

  // experimental
/*   assert( gllaser = gdk_gl_config_new_by_mode( (GdkGLConfigMode) */
/* 					       (GDK_GL_MODE_RGBA | */
/* 					       GDK_GL_MODE_DEPTH | */
/* 						GDK_GL_MODE_DOUBLE))); */
  
  return glconfig;
}


GtkWidget * gui_create_canvas( stg_world_t* world )
{
  if( glconfig == NULL )
    gl_init(); // sets up glconfig static variable used in this
	       // function

  GtkWidget *drawing_area;

  // Drawing area to draw OpenGL scene.
  drawing_area = gtk_drawing_area_new ();
  gtk_widget_set_size_request (drawing_area, DEFAULT_WIDTH, DEFAULT_HEIGHT);

  /* Set OpenGL-capability to the widget */
  gtk_widget_set_gl_capability (drawing_area,
				glconfig,
				NULL,
				TRUE,
				GDK_GL_RGBA_TYPE);

  gtk_widget_add_events (drawing_area,
			 GDK_POINTER_MOTION_MASK      |
			 GDK_BUTTON1_MOTION_MASK      |
			 GDK_BUTTON3_MOTION_MASK      |
			 GDK_BUTTON_PRESS_MASK      |
			 GDK_BUTTON_RELEASE_MASK    |
			 GDK_SCROLL_MASK            |
			 GDK_VISIBILITY_NOTIFY_MASK);

  /* Connect signal handlers to the drawing area */
  g_signal_connect_after (G_OBJECT (drawing_area), "realize",
                          G_CALLBACK (realize), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "configure_event",
		    G_CALLBACK (configure_event), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "expose_event",
		    G_CALLBACK (expose_event), world );

  g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event",
		    G_CALLBACK (motion_notify_event), world );
  g_signal_connect (G_OBJECT (drawing_area), "button_press_event",
		    G_CALLBACK (button_press_event), world );
  g_signal_connect (G_OBJECT (drawing_area), "button_release_event",
		    G_CALLBACK (button_release_event), world );
  g_signal_connect (G_OBJECT (drawing_area), "unrealize",
		    G_CALLBACK (unrealize), NULL);

  g_signal_connect (G_OBJECT (drawing_area), "scroll-event",
		    G_CALLBACK (scroll_event), world );

  /* key_press_event handler for top-level window */
  g_signal_connect_swapped (G_OBJECT (drawing_area), "key_press_event",
			    G_CALLBACK (key_press_event), world );

  gtk_widget_show (drawing_area);

  // set up a reasonable default scaling, so that the world fits
  // neatly into the window.
  world->win->scale = DEFAULT_WIDTH / (double)world->width;

  memset( &world->win->click_point, 0, sizeof(world->win->click_point));

  // allocate a drawing list for everything in the world
  //world->win->draw_list = glGenLists(1);

  // allocate a drawing list for debug info
  //world->win->debug_list = glGenLists(1);

  dl_debug = glGenLists(1);

  return drawing_area;
}


/**************************************************************************
 * The following section contains utility function definitions.
 **************************************************************************/




/* void gui_ranger_init( stg_model_t* mod ) */
/* { */
/*   stg_model_add_property_toggles( mod,  */
/* 				  &mod->data, */
/* 				  gl_ranger_render_data, // called when toggled on */
/* 				  NULL, */
/* 				  gl_unrender_list_cb, // called when toggled off */
/* 				  (void*)LIST_DATA, */
/* 				  "rangerdata", */
/* 				  "ranger data", */
/* 				  TRUE );  // initial state */
/* } */

/* static int init = 0; */
/* static stg_color_t laser_color=0, bright_color=0, fill_color=0, cfg_color=0, geom_color=0; */

/* void gui_laser_init( stg_model_t* mod ) */
/* { */
/*   // we do this just the first tie a laser is created */
/*   if( init == 0 ) */
/*     { */
/*       laser_color =   stg_lookup_color(STG_LASER_COLOR); */
/*       bright_color = stg_lookup_color(STG_LASER_BRIGHT_COLOR); */
/*       fill_color = stg_lookup_color(STG_LASER_FILL_COLOR); */
/*       geom_color = stg_lookup_color(STG_LASER_GEOM_COLOR); */
/*       cfg_color = stg_lookup_color(STG_LASER_CFG_COLOR); */
/*       init = 1; */
/*     } */

/*   // adds a menu item and associated on-and-off callbacks */
/*   stg_model_add_property_toggles( mod,  */
/* 				  &mod->data, */
/* 				  gl_laser_render_data, // called when toggled on */
/* 				  NULL, */
/* 				  gl_unrender_list_cb, // called when toggled off */
/* 				  (void*)LIST_DATA, */
/* 				  "laserdata", */
/* 				  "laser data", */
/* 				  TRUE );   */
/* } */

/* // TODO */

/* void gui_position_init( stg_model_t* mod ) */
/* {} */

/* void gui_gripper_init( stg_model_t* mod ) */
/* {} */

/* void gui_bumper_init( stg_model_t* mod ) */
/* {} */

/* void gui_ptz_init( stg_model_t* mod ) */
/* {} */

/* void gui_puck_init( stg_model_t* mod ) */
/* {} */

/* void gui_fiducial_init( stg_model_t* mod ) */
/* { */
/*   // adds a menu item and associated on-and-off callbacks */
/*   stg_model_add_property_toggles( mod, */
/* 				  &mod->data, */
/* 				  gl_fiducial_render_data, // called when toggled on */
/* 				  NULL, */
/* 				  gl_unrender_list_cb, // called when toggled off */
/* 				  (void*)LIST_DATA, */
/* 				  "fiducialdata", */
/* 				  "fiducial data", */
/* 				  TRUE ); */
/* } */

/* void gui_blobfinder_init( stg_model_t* mod ) */
/* {} */

/* void gui_speech_init( stg_model_t* mod ) */
/* {} */




/* int gl_fiducial_render_data( stg_model_t* mod, void* enabled ) */
/* { */
/*   int list = gui_model_get_displaylist( mod, LIST_DATA ); */
  
/*   glNewList( list, GL_COMPILE ); */

/*   stg_fiducial_t* fids = (stg_fiducial_t*)mod->data;  */
/*   size_t fids_count = mod->data_len / sizeof(stg_fiducial_t); */
/*   //stg_fidulaser_config_t *cfg = (stg_laser_config_t*)mod->cfg; */
/*   //assert( cfg ); */
  
/*   if( fids && fids_count ) */
/*     {     */
/*       stg_pose_t gpose; */
/*       stg_model_get_global_pose( mod, &gpose ); */
      
/*       // do alpha properly */
/*       glDepthMask( GL_FALSE ); */
      
/*       //glPushMatrix(); */
/*       //glTranslatef( 0,0, gpose.z + mod->geom.size.z/2.0 ); // shoot the laser beam out at the right height */
      
/*       push_color_rgba( 0.8, 0, 0.8, 0.5 );  */
      
/*       // indicate the fiducial */
/*       int f; */
/*       for( f=0; f<fids_count; f++ ) */
/* 	{	  */
/* 	  // first draw a box on the fiducial's position */
/* 	  glPushMatrix(); */
	  
/* 	  glTranslatef( fids[f].pose.x, fids[f].pose.y, 0 ); */
/* 	  glRotatef( RTOD(gpose.a+fids[f].geom.a), 0,0,1); */
	  
/* 	  double dx = fids[f].geom.x / 2.0;  */
/* 	  double dy = fids[f].geom.y / 2.0;  */
/* 	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE ); */
/* 	  glRectf( -dx, -dy, dx, dy ); */
/* 	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL ); */

/* 	  // now draw a little nose line to show heading */
/* 	  glBegin( GL_LINES ); */
/* 	  glVertex2d( 0,0 ); */
/* 	  glVertex2d( dx+0.1,0); */
/* 	  glEnd();	   */
	  
/* 	  glPopMatrix(); */
	  
/* 	  // now draw a line from the sensor to the target	   */
/* 	  glBegin( GL_LINES ); */
/* 	  glVertex2d( gpose.x, gpose.y ); */
/* 	  glVertex2d( fids[f].pose.x, fids[f].pose.y ); */
/* 	  glEnd(); */
	  
/* 	  // now print the fiducial ID, if available */
/* 	  if( fids[f].id )	   */
/* 	    {	 */
/* 	      glRasterPos2f( fids[f].pose.x + 0.4,  */
/* 			     fids[f].pose.y ); */
/* 	      glPrint( "%d", fids[f].id ); */
/* 	    }	   */
/* 	} */
      
/*       pop_color(); */
/*       glDepthMask( GL_TRUE );  */
/*     } */
  
/*   glEndList();   */
/*   make_dirty(mod); */
/*   return 0; // callback runs until removed */
/* } */


/**************************************************************************
 * End of file.
 **************************************************************************/



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
 
static GdkGLConfig *glconfig = NULL;
static GdkGLConfig *gllaser = NULL;

GList* dl_list = NULL;

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
#define LIST_BODY "_gl_body"
#define LIST_DATA "_gl_data"
#define LIST_POLYS_2D "_gl_polys_2d"
#define LIST_POLYS_3D "_gl_polys_3d"
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

stg_model_t* stg_world_nearest_root_model( stg_world_t* world, double wx, double wy );

// foward-declare some callbacks
void model_draw_cb( stg_model_t* mod, void* user );
void model_bbox_cb( gpointer key, stg_model_t* mod, void* user );


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
	Quit( 1 );
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


void print_color_stack( char* msg )
{
  if(msg==NULL)
    msg = "";

  printf( "color stack (%s):\n", msg );

  int c;
  for(c=0; c<colorstack->length; c++ )
    {      
      GLdouble* color = g_queue_peek_nth( colorstack, c );
      
      printf( "   %.2f %.2f %.2f %.2f\n",
	      color[0],
	      color[1],
	      color[2],
	      color[3] );
    }
}


void stg_color_to_glcolor4dv( stg_color_t scol, GLdouble gcol[4] )
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
  GLdouble *keep = malloc( sz );
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

  GLdouble *col = g_queue_pop_head( colorstack );
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

static void polygon3d( double* pts, size_t pt_count, 
		       double base, double height, int top  )
{ 
  if( pt_count < 3 )
    {
      PRINT_WARN1( "failed to render a polygon with %d points\n", (int)pt_count );
      return;
    }
  
  if( ! top )
    glDisable (GL_CULL_FACE);
  
  // construct a strip that wraps around the polygon

  glBegin(GL_QUAD_STRIP);
  int p;
  for( p=0; p<pt_count; p++)
    {
      glVertex3f( pts[p*2], pts[p*2+1], base + height );
      glVertex3f( pts[p*2], pts[p*2+1], base );
    }

  // close the strip
  glVertex3f( pts[0], pts[1], base+height );
  glVertex3f( pts[0], pts[1], base );

  glEnd();

  if( top )
    {
      // and draw a top that fits over the stripa
      glBegin(GL_POLYGON);
      for( p=0; p<pt_count; p++)
	glVertex3f( pts[p*2], pts[p*2+1], base+height );
      glEnd();
    }
  else
    glEnable(GL_CULL_FACE);
}



static void polygon2d( double* pts, size_t pt_count, stg_color_t color  )
{ 
  glEnableClientState( GL_VERTEX_ARRAY );
  glVertexPointer( 2, GL_DOUBLE, 0, pts );  
  glDrawArrays( GL_POLYGON, 0, pt_count );
}


void make_dirty( stg_model_t* mod )
{
  ((gui_window_t*)mod->world->win)->dirty = TRUE;
}



int gl_unrender_list_cb( stg_model_t* mod, void* list_name )
{
  gui_model_free_displaylist( mod, (char*)list_name );
  return 1; // prevents this callback being called again
}


int gl_laser_render_data( stg_model_t* mod, void* enabled )
{
  //puts ("GL laser data" );
  int list = gui_model_get_displaylist( mod, LIST_DATA );
  
  glNewList( list, GL_COMPILE );



  // end exp

  stg_laser_sample_t* samples = (stg_laser_sample_t*)mod->data; 
  size_t sample_count = mod->data_len / sizeof(stg_laser_sample_t);
  stg_laser_config_t *cfg = (stg_laser_config_t*)mod->cfg;
  assert( cfg );
  
  if( samples && sample_count )
    {    
      stg_pose_t gpose;
      stg_model_get_global_pose( mod, &gpose );

  // do alpha properly
      glDepthMask( GL_FALSE );
      
      glPushMatrix();
      glTranslatef( 0,0, gpose.z + mod->geom.size.z/2.0 ); // shoot the laser beam out at the right height
      
      // pack the laser hit points into a vertex array for fast rendering

      static float* pts = NULL;
      pts = g_realloc( pts, 2 * (sample_count+1) * sizeof(float));
      
      pts[0] = (float)gpose.x;
      pts[1] = (float)gpose.y;
      
      int s;
      for( s=0; s<sample_count; s++ )
	{
	  double ray_angle = gpose.a + (s * (cfg->fov / (sample_count-1))) - cfg->fov/2.0;

	  //pts[2*s+2] = (float)samples[s].hitpoint.x;
	  //pts[2*s+3] = (float)samples[s].hitpoint.y;
	  pts[2*s+2] = (float)(samples[s].range * cos(ray_angle) + gpose.x);
	  pts[2*s+3] = (float)(samples[s].range * sin(ray_angle) + gpose.y);
	}
      
      glEnableClientState( GL_VERTEX_ARRAY );
      glVertexPointer( 2, GL_FLOAT, 0, pts );
      
      if( mod->world->win->show_alpha )
	{
	  glColor4f( 0, 0, 1, 0.1 );
	  glDrawArrays( GL_POLYGON, 0, sample_count+1 );
	}
      else
	{
	  glColor3f( 0.5, 0.5, 1.0 );
	  glDrawArrays( GL_LINE_LOOP, 0, sample_count+1 );
	}
      
      //free(pts);
      glPopMatrix();
      glDepthMask( GL_TRUE ); 
    }
  
/*   // outline the polgons that the selected robot intersects with */
/*   push_color_stgcolor( stg_lookup_color( "orange" )); */
  
/*   GList *b; */
/*   for( b = mod->sense_poly->intersectors; b; b=b->next ) */
/*     { */
/*       stg_polygon_t* p = (stg_polygon_t*)b->data; */
      
/*       glPushMatrix(); */
      
/*       stg_pose_t pose; */
/*       stg_model_get_global_pose( p->mod, &pose ); */
      
/*       // move into this model's coordinate frame */
/*       glTranslatef( pose.x, pose.y, pose.z ); */
/*       glRotatef( RTOD(pose.a), 0,0,1 ); */
/*       gl_draw_polygon3d( p ); */
      
/*       glPopMatrix(); */
/*     } */
  
/*   pop_color(); */

  glEndList();
  
  make_dirty(mod);

/*   // loop through again, drawing bright boxes on top of the polygon */
/*   for( s=0; s<sample_count; s++ ) */
/*     {       */
/*       // if this hit point is bright, we draw a little box */
/*       if( samples[s].reflectance > 0 ) */
/* 	{ */
/* 	  stg_rtk_fig_color_rgb32( fg, bright_color ); */
/* 	  stg_rtk_fig_rectangle( fg,  */
/* 				 points[1+s].x, points[1+s].y, 0, */
/* 				 0.04, 0.04, 1 ); */
/* 	  stg_rtk_fig_color_rgb32( fg, laser_color ); */
/* 	} */
/*     } */
      
  return 0; // callback runs until removed
}

int gl_fiducial_render_data( stg_model_t* mod, void* enabled )
{
  int list = gui_model_get_displaylist( mod, LIST_DATA );
  
  glNewList( list, GL_COMPILE );

  stg_fiducial_t* fids = (stg_fiducial_t*)mod->data; 
  size_t fids_count = mod->data_len / sizeof(stg_fiducial_t);
  //stg_fidulaser_config_t *cfg = (stg_laser_config_t*)mod->cfg;
  //assert( cfg );
  
  if( fids && fids_count )
    {    
      stg_pose_t gpose;
      stg_model_get_global_pose( mod, &gpose );
      
      // do alpha properly
      glDepthMask( GL_FALSE );
      
      //glPushMatrix();
      //glTranslatef( 0,0, gpose.z + mod->geom.size.z/2.0 ); // shoot the laser beam out at the right height
      
      push_color_rgba( 0.8, 0, 0.8, 0.5 ); 
      
      // indicate the fiducial
      int f;
      for( f=0; f<fids_count; f++ )
	{	 
	  // first draw a box on the fiducial's position
	  glPushMatrix();
	  
	  glTranslatef( fids[f].pose.x, fids[f].pose.y, 0 );
	  glRotatef( RTOD(gpose.a+fids[f].geom.a), 0,0,1);
	  
	  double dx = fids[f].geom.x / 2.0; 
	  double dy = fids[f].geom.y / 2.0; 
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
	  glRectf( -dx, -dy, dx, dy );
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );

	  // now draw a little nose line to show heading
	  glBegin( GL_LINES );
	  glVertex2d( 0,0 );
	  glVertex2d( dx+0.1,0);
	  glEnd();	  
	  
	  glPopMatrix();
	  
	  // now draw a line from the sensor to the target	  
	  glBegin( GL_LINES );
	  glVertex2d( gpose.x, gpose.y );
	  glVertex2d( fids[f].pose.x, fids[f].pose.y );
	  glEnd();
	  
	  // now print the fiducial ID, if available
	  if( fids[f].id )	  
	    {	
	      glRasterPos2f( fids[f].pose.x + 0.4, 
			     fids[f].pose.y );
	      glPrint( "%d", fids[f].id );
	    }	  
	}
      
      pop_color();
      glDepthMask( GL_TRUE ); 
    }
  
  glEndList();  
  make_dirty(mod);
  return 0; // callback runs until removed
}


int gui_model_get_displaylist( stg_model_t* mod, char* list_name )
{
  // get the display list associated with this model
  int list = (int)stg_model_get_property( mod, list_name );
  
  if( list == 0 ) // no list, get a valid list ID and remember it
    {
      list = glGenLists(1);
      stg_model_set_property( mod, list_name, (void*)list );
    }
  
  return list;
}

void gui_model_free_displaylist( stg_model_t* mod, char* list_name )
{
  // get the display list associated with this model
  int list = (int)stg_model_get_property( mod, list_name );
  
  if( list )
    glDeleteLists( list, 1 );
}

int gl_ranger_render_data( stg_model_t* mod, void* enabled )
{
  //puts ("GL ranger data" );
    
  int list = gui_model_get_displaylist( mod, LIST_DATA );

  // replaces any existing list with this ID
  glNewList( list, GL_COMPILE );

  stg_ranger_sample_t* samples = (stg_ranger_sample_t*)mod->data; 
  size_t sample_count = mod->data_len / sizeof(stg_ranger_sample_t);
  stg_ranger_config_t *cfg = (stg_ranger_config_t*)mod->cfg;
  
  if( (samples == NULL) | (sample_count < 1) )
    { 
      glEndList();
      return 0;
    }

  assert( cfg );
      
  // now get on with rendering the laser data

  glPushMatrix();

  // go into model coordinates
  stg_pose_t gpose;
  stg_model_get_global_pose( mod, &gpose );
  
  glTranslatef( gpose.x, gpose.y, gpose.z );
  glRotatef(RTOD(gpose.a), 0,0,1);
      
  // local offset coords
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );
      
  glDepthMask( GL_FALSE );
  
  // respect the local offsets
  glTranslatef( mod->geom.pose.x, mod->geom.pose.y, mod->geom.pose.z );
  glRotatef(RTOD(mod->geom.pose.a), 0,0,1);
  
  push_color_rgba( 1, 0, 0, 0.1 ); // transparent red
  
  // draw the range  beams
  int s;
  for( s=0; s<sample_count; s++ )
    {
      if( samples[s].range > 0.0 )
	{
	  stg_ranger_config_t* rngr = &cfg[s];
	  
	  // sensor FOV
	  double sidelen = samples[s].range;
	  double da = rngr->fov/2.0;
	  
	  double x1= rngr->pose.x + sidelen*cos(rngr->pose.a - da );
	  double y1= rngr->pose.y + sidelen*sin(rngr->pose.a - da );
	  double x2= rngr->pose.x + sidelen*cos(rngr->pose.a + da );
	  double y2= rngr->pose.y + sidelen*sin(rngr->pose.a + da );
	  
	  if( mod->world->win->show_alpha )
	    glBegin(GL_POLYGON);
	  else
	    glBegin(GL_LINE_LOOP);
	    
 	    glVertex2f( rngr->pose.x, rngr->pose.y );
	    glVertex2f( x1, y1 );
	    glVertex2f( x2, y2 );
	  glEnd();	  
	}
    }
  
  glPopMatrix();

  // restore state
  glDepthMask( GL_TRUE );
  pop_color();

  glEndList();

  make_dirty(mod);

  return 0; // callback runs until removed
}


int gl_model_grid( stg_model_t* mod, void* user )
{  
  // get the display list associated with this model
  int list = gui_model_get_displaylist( mod, LIST_GRID );
  
  glNewList( list, GL_COMPILE );

  push_color_rgba( 0.8,0.8,0.8,0.8 ); 

  double dx = mod->geom.size.x;
  double dy = mod->geom.size.y;
  double sp = 1.0;
 
  int nx = (int) ceil((dx/2.0) / sp);
  int ny = (int) ceil((dy/2.0) / sp);
  
  if( nx == 0 ) nx = 1.0;
  if( ny == 0 ) ny = 1.0;
  
  glBegin(GL_LINES);

  // draw the bounding box first
  glVertex2f( -nx, -ny );
  glVertex2f(  nx, -ny );

  glVertex2f( -nx, ny );
  glVertex2f(  nx, ny );

  glVertex2f( nx, -ny );
  glVertex2f( nx,  ny );

  glVertex2f( -nx,-ny );
  glVertex2f( -nx, ny );

  int i;
  for (i = -nx+1; i < nx; i++)
    {
      glVertex2f(  i * sp,  - dy/2 );
      glVertex2f(  i * sp,  + dy/2 );
      //snprintf( str, 64, "%d", (int)i );
      //stg_rtk_fig_text( fig, -0.2 + (ox + i * sp), -0.2 , 0, str );
    }
  
  for (i = -ny+1; i < ny; i++)
    {
      glVertex2f( - dx/2, i * sp );
      glVertex2f( + dx/2,  i * sp );      
      //snprintf( str, 64, "%d", (int)i );
      //stg_rtk_fig_text( fig, -0.2, -0.2 + (oy + i * sp) , 0, str );
    }
  
  glEnd();
  
  pop_color();
  glEndList();

  return 0; // callback runs until removed
}

void gl_model_select( stg_model_t* mod, void* user )
{  
  // get the display list associated with this model
  int list = gui_model_get_displaylist( mod, LIST_SELECTED );
  
  glNewList( list, GL_COMPILE );

  glPushMatrix();
  //glPushAttrib( GL_ALL_ATTRIB_BITS );

  push_color_rgba( 1, 0, 0, 1 );
  
  double dx = mod->geom.size.x / 2.0 * 1.3;
  double dy = mod->geom.size.y / 2.0 * 1.3;

  //glTranslatef( 0,0,0.1 );

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
  glRectf( -dx, -dy, dx, dy );
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );

  pop_color();

  glPopMatrix();
  //glPopAttrib();
  glEndList();
}

void gl_model_selected( stg_model_t* mod, void* user )
{  
  // get the display list associated with this model
  int list = gui_model_get_displaylist( mod, LIST_SELECTED );
  
  glNewList( list, GL_COMPILE );

  //glPushMatrix();
  //glPushAttrib( GL_ALL_ATTRIB_BITS );

  push_color_rgba( 1, 0, 0, 1 );
  
  double dx = mod->geom.size.x / 2.0 * 1.3;
  double dy = mod->geom.size.y / 2.0 * 1.3;

  //glTranslatef( 0,0,0.1 );

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glRectf( -dx, -dy, dx, dy );
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  pop_color();

  //glPopMatrix();
  //glPopAttrib();
  glEndList();
}

void gl_draw_polygon_bbox( stg_polygon_t* p )
{
  // no need to optimize this, it's debug visualization mainly.

  //  stg_color_t col = stg_lookup_color( "purple" );
  //push_color_stgcolor( col );

  // bottom plane
  glBegin(GL_LINE_LOOP );
  glVertex3f( p->epts[0].value, p->epts[2].value, p->epts[4].value );
  glVertex3f( p->epts[1].value, p->epts[2].value, p->epts[4].value );
  glVertex3f( p->epts[1].value, p->epts[3].value, p->epts[4].value );
  glVertex3f( p->epts[0].value, p->epts[3].value, p->epts[4].value );
  glEnd();

  // top plane
  glBegin(GL_LINE_LOOP );
  glVertex3f( p->epts[0].value, p->epts[2].value, p->epts[5].value );
  glVertex3f( p->epts[1].value, p->epts[2].value, p->epts[5].value );
  glVertex3f( p->epts[1].value, p->epts[3].value, p->epts[5].value );
  glVertex3f( p->epts[0].value, p->epts[3].value, p->epts[5].value );
  glEnd();

  // verticals joining corners
  glBegin(GL_LINES );
  glVertex3f( p->epts[0].value, p->epts[2].value, p->epts[4].value );
  glVertex3f( p->epts[0].value, p->epts[2].value, p->epts[5].value );
  glVertex3f( p->epts[1].value, p->epts[2].value, p->epts[4].value );
  glVertex3f( p->epts[1].value, p->epts[2].value, p->epts[5].value );
  glVertex3f( p->epts[1].value, p->epts[3].value, p->epts[4].value );
  glVertex3f( p->epts[1].value, p->epts[3].value, p->epts[5].value );
  glVertex3f( p->epts[0].value, p->epts[3].value, p->epts[4].value );
  glVertex3f( p->epts[0].value, p->epts[3].value, p->epts[5].value );
  glEnd();

  //pop_color();
}

void gl_draw_polygon_bbox_cb( stg_polygon_t* p, gpointer unused )
{
  gl_draw_polygon_bbox( p );
}


void gl_draw_polygon2d( stg_polygon_t* p )
{
  polygon2d( (double*)p->points->data, p->points->len, p->color );

  // bbox in local coords
/*   push_color_rgba( 0, 1, 0, 1 ); */

/*   glBegin(GL_LINE_LOOP ); */
/*   glVertex2f( p->minx, p->miny ); */
/*   glVertex2f( p->maxx, p->miny ); */
/*   glVertex2f( p->maxx, p->maxy ); */
/*   glVertex2f( p->minx, p->maxy ); */
/*   glEnd(); */

/*   printf( "drawing polygon local bbox (%.2f,%.2f)(%.2f,%.2f)\n", */
/* 	  p->minx, p->maxx,  */
/* 	  p->miny, p->maxy ); */

/*   pop_color(); */
}

void gl_draw_polygon2d_cb( stg_polygon_t* p, gpointer unused )
{
  gl_draw_polygon2d( p );
}

void gl_draw_polygon3d( stg_polygon_t* p )
{
  polygon3d( (double*)p->points->data, p->points->len,
	     0, p->mod->geom.size.z, 
	     TRUE );
}

void gl_draw_polygon3d_cb( stg_polygon_t* p, gpointer unused )
{
  gl_draw_polygon3d( p );
}


int gl_model_polygons( stg_model_t* mod, void* userp )
{
  //puts( "model render polygons" );

  gui_window_t* win = (gui_window_t*)mod->world->win;

  // 2D RENDERING
  // get the display list associated with this model
  int list = gui_model_get_displaylist( mod, LIST_POLYS_2D );
  
  glNewList( list, GL_COMPILE );
  
  glPushMatrix();
  
  // go into model's geometry coordframe
  glTranslatef( mod->geom.pose.x, mod->geom.pose.y, mod->geom.pose.z );
  glRotatef( RTOD(mod->geom.pose.a), 0,0,1 );

  // draw each poly
  g_list_foreach( mod->polys, (GFunc)gl_draw_polygon2d_cb, NULL );

  //if( 1 )// mod->boundary ) 
  //  box3d_wireframe( &mod->bbox );

  glPopMatrix(); // drop out of local coords
  glEndList();

  // 3D RENDERING
  // get the display list associated with this model
  list = gui_model_get_displaylist( mod, LIST_POLYS_3D );
  
  glNewList( list, GL_COMPILE );

  glPushMatrix();
  
  // go into model's geometry coordframe
  glTranslatef( mod->geom.pose.x, mod->geom.pose.y, mod->geom.pose.z );
  glRotatef( RTOD(mod->geom.pose.a), 0,0,1 );

  // draw each poly
  g_list_foreach( mod->polys, (GFunc)gl_draw_polygon3d_cb, NULL );

  glPopMatrix(); // drop out of local coords
  glEndList();

  make_dirty(mod);
  
  return 0;
}



int gl_model_redraw( stg_model_t* mod, void* userp )
{
  // need to redraw the whole model tree
  make_dirty(mod);
  return 0;
}

void gl_model_grid_axes( stg_model_t* mod )
{
  double dx = mod->geom.size.x;
  double dy = mod->geom.size.y;
  double sp = 1.0;
  
  int nx = (int) ceil((dx/2.0) / sp);
  int ny = (int) ceil((dy/2.0) / sp);
  
  if( nx == 0 ) nx = 1.0;
  if( ny == 0 ) ny = 1.0;
  
  int i;
  for (i = -nx+1; i < nx; i++)
    {
      glRasterPos2f( -0.2 + (i*sp), -0.2 );
      glPrint( "%d", (int)i );      
    }
  
  for (i = -ny+1; i < ny; i++)
    {
      glRasterPos2f( -0.2, -0.2 + (i * sp) );
      glPrint( "%d", (int)i );      
      
      //snprintf( str, 64, "%d", (int)i );
      //stg_rtk_fig_text( fig, -0.2, -0.2 + (oy + i * sp) , 0, str );
    }
}

void gl_model_draw( stg_model_t* mod )
{
  //puts( "model render polygons" );

  gui_window_t* win = (gui_window_t*)mod->world->win;

  // get the display list associated with this model
  int list = gui_model_get_displaylist( mod, LIST_BODY );  

  glNewList( list, GL_COMPILE_AND_EXECUTE );
  
  //stg_pose_t gp;
  //stg_model_get_global_pose( mod, &gp );
  
  glPushMatrix();
  
  // move into this model's coordinate frame
  
  //if( mod->parent )
  //   glTranslatef( 0,0, mod->parent->geom.pose.a );

  glTranslatef( mod->pose.x, mod->pose.y, mod->pose.z );
  glRotatef( RTOD(mod->pose.a), 0,0,1 );
  
  //draw blocks
  GList* it;
  for( it=mod->blocks; it; it=it->next )
    stg_block_render( (stg_block_t*)it->data );

  // player color elements are uint8_t type
  glColor4ub( 0,2,0,255 );

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );      

  // draw the stg_d list
  for( it=mod->d_list; it; it=it->next )
    {
      stg_d_draw_t* d = (stg_d_draw_t*)it->data;
      
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

	  //printf( "vertex [%.2f, %.2f, %.2f]\n",
	  //  d->verts[c].x,
	  //  d->verts[c].y,
	  //  d->verts[c].z );

	}

      //puts("");

      glEnd();
    }
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );      

  // go into model's geometry coordframe
  //glTranslatef( mod->geom.pose.x, mod->geom.pose.y, mod->geom.pose.z );
  //glRotatef( RTOD(mod->geom.pose.a), 0,0,1 );

  if( win->show_grid && mod->gui_grid )
    {
      int gridlist = (int)stg_model_get_property( mod, LIST_GRID );
      glCallList( gridlist );
      
      // for some reason, glprint() doesn't work in the grid display
      // list, so we do it here
      gl_model_grid_axes( mod );
    }
  
  // model's body color
  push_color_stgcolor( mod->color );

  if( win->show_polygons )
    {
      // choose 2d or 2d drawing
      char* idstr;
      if( win->sphi || win->stheta )
	idstr = LIST_POLYS_3D;
      else
	idstr = LIST_POLYS_2D;

      int polylist = (int)stg_model_get_property( mod, idstr );
      
      if( win->fill_polygons )
	{
	  glEnable(GL_POLYGON_OFFSET_FILL);
	  glPolygonOffset(1.0, 1.0);
	  glCallList( polylist );
	  glDisable(GL_POLYGON_OFFSET_FILL);
	}

      if( mod->gui_outline && win->fill_polygons ) 
	{
	  double gcol[4];	  
	  stg_color_to_glcolor4dv( mod->color, gcol );
	  
	  // make a darker version of the same color
	  gcol[0]/=2.0;
	  gcol[1]/=2.0;
	  gcol[2]/=2.0;

	  push_color( gcol );

	}
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );      
      glCallList( polylist );
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );      

      pop_color();
    }

  if( mod->world->win->selection_active == mod )
    {
      int selectedlist = (int)stg_model_get_property( mod, LIST_SELECTED );
      glCallList( selectedlist );
    }
  
  
  g_list_foreach( mod->children, (GFunc)model_draw_cb, NULL );

/*       // recursively draw the tree below this model */
/*       GList *it;; */
/*       for( it=mod->children; it; it=it->next ) */
/* 	  gl_model_draw(  (stg_model_t*)it->data ); */
/*     } */
  
  glPopMatrix(); // drop out of local coords

  pop_color();
  glEndList();
  
  // opengl.org recommends not using compile-and-execute, but explitly
  // compiling and calling the display list seperately - need to think
  // about how to do this, as it messes up the logic/geometry of
  // drawing above.  
  //glCallList( list );

  make_dirty(mod);
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

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LESS);
  glClearColor ( 0.7, 0.7, 0.8, 1.0);
  //glClearColor ( 0,0,0, 1.0);
  gdk_gl_glPolygonOffsetEXT (proc, 1.0, 1.0);

  glEnable (GL_CULL_FACE);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glDisable (GL_LIGHTING);
  
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

void model_draw_data_cb( gpointer key, stg_model_t* mod, void* user )
{
  if( ((gui_window_t*)mod->world->win)->show_data )
    {
      int datalist = (int)stg_model_get_property( mod, LIST_DATA );      
      glCallList( datalist );
    }
}

void model_draw_cb( stg_model_t* mod, void* user )
{
  gl_model_draw( mod );
}

void model_bbox_cb( gpointer key, stg_model_t* mod, void* user )
{
  push_color_stgcolor( mod->color );
  g_list_foreach( mod->polys, (GFunc)gl_draw_polygon_bbox_cb, NULL );
  pop_color();
}




void draw_thumbnail( stg_world_t* world )
{
  //double boxwidth = THUMBNAILWIDTH; // pixels
  double boxheight = THUMBNAILHEIGHT;//boxwidth / (world->width/(double)world->height);
  double boxwidth = boxheight / (world->height/(double)world->width);

  glPushAttrib( GL_ALL_ATTRIB_BITS );

  glViewport(0,0,boxwidth, boxheight );

  glMatrixMode (GL_PROJECTION);
  glPushMatrix();  
  glLoadIdentity();

  glOrtho( 0,1,0,1,-1,1);

  glMatrixMode (GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity ();

  glTranslatef( 0,0,-0.9 );




  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  glColor3f( 1,1,1 );      
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glRecti( 0,0,1,1 ); // fill the viewport
  glDisable(GL_POLYGON_OFFSET_FILL);
  
  glColor3f( 0,0,0 );      
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glRecti( 0,0,1,1 ); // fill the viewport
  
  glTranslatef( 0.5,0.5,0 );
  glScalef( 1.0/world->width, 1.0/world->height, 1.0);
  
  // don't render grids in the thumbnail
  int keep_grid  = world->win->show_grid;
  int keep_data = world->win->show_data;
  world->win->show_data = 0;
  world->win->show_grid = 0;

  g_list_foreach( world->children, (GFunc)model_draw_cb, NULL );

  world->win->show_grid = keep_grid;
  world->win->show_data = keep_data;
  
  
/*   // if we're in 2d mode - show the visble region */
  
/*   double left = -seex/2.0 * world->width - world->win->panx; */
/*   double top = -seey/2.0 * world->height - world->win->pany; */
/*   double right = seex/2.0 * world->width - world->win->panx; */
/*   double bottom = seey/2.0 * world->height - world->win->pany; */
  
  
/*   if( world->win->show_alpha ) */
/*     { */
/*       glColor4f( 0,0,0,0.15 );       */
/*       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); */
      
/*       glRectf( -world->width/2.0,  */
/* 	       -world->height/2.0,  */
/* 	       left, */
/* 	       world->height/2.0 ); */
      
/*       glRectf( left, */
/* 	       -world->height/2.0,  */
/* 	       world->width/2.0, */
/* 	       top ); */
      
/*       glRectf( left, */
/* 	       bottom, */
/* 	       world->width/2.0, */
/* 	       world->height/2.0 ); */
      
/*       glRectf( right, */
/* 	       top, */
/* 	       world->width/2.0, */
/* 	       bottom); */
/*     } */

/*   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); */
/*   glColor3f( 1.0,0,0.1 ); */
/*   glRectf( left, top, right, bottom ); */
  
  
  glPopMatrix(); // restore GL_MODELVIEW

  glMatrixMode(GL_PROJECTION);
  glPopMatrix(); // restore GL_PROJECTION


  glPopAttrib();      
}

void draw_endpoints( stg_world_t* world )
{
  double epsilon = 0.2;
  double hepsilon = epsilon / 2.0;

  int i=0;
  stg_endpoint_t* ep;

/*   print_endpoint_list( world->endpts.x, "X LIST" ); */
/*   print_endpoint_list( world->endpts.y, "Y LIST" ); */
/*   print_endpoint_list( world->endpts.z, "Z LIST" ); */

  for( ep = world->endpts.x; ep; ep=ep->next )
    {
      assert( ep );
      assert( ep->polygon);
      assert( ep->polygon->mod );

      push_color_stgcolor( ep->polygon->mod->color );

      glBegin( GL_POLYGON );
      glVertex3f( ep->value, 0, 0 );

      if( ep->type == 0 )
	{	
	  glVertex3f( ep->value, -epsilon, 0 );
	  glVertex3f( ep->value+hepsilon, -hepsilon, 0 );
	}
      else
	{	
	  glVertex3f( ep->value-hepsilon, -hepsilon, 0 );
	  glVertex3f( ep->value, -epsilon, 0 );
	}
	
      glEnd();

      glRasterPos2f( ep->value, -0.5 );
      glPrint( "%d", (int)i++ );      

      glRasterPos2f( ep->value, -0.8 );
      glPrint( "%.2f", ep->value );      
      pop_color();
    }

  i=0;
  for( ep = world->endpts.y; ep; ep=ep->next )
    {
      push_color_stgcolor( ep->polygon->mod->color );

      glBegin( GL_POLYGON );
      glVertex3f( 0, ep->value, 0 );

      if( ep->type == 0 )
	{	
	  glVertex3f( -hepsilon, ep->value+hepsilon,  0 );
	  glVertex3f( -epsilon, ep->value, 0 );
	}
      else
	{	
	  glVertex3f( -epsilon, ep->value, 0 );
	  glVertex3f( -hepsilon, ep->value-hepsilon,  0 );
	}
	
      glEnd();

      glRasterPos2f( -0.5, ep->value );
      glPrint( "%d", (int)i++ );      

      glRasterPos2f( -0.8, ep->value - 0.3 );
      glPrint( "%.2f", ep->value );      
      pop_color();
    }

/*   i=0; */
/*   for( ep = world->endpts.z; ep; ep=ep->next ) */
/*     { */
/*       push_color_stgcolor( ep->polygon->color ); */

/*       glBegin( GL_POLYGON ); */
/*       glVertex3f( 0, 0, ep->value ); */

/*       if( ep->type == 0 ) */
/* 	{	 */
/* 	  glVertex3f( -hepsilon, 0, ep->value+hepsilon ); */
/* 	  glVertex3f( -epsilon, 0, ep->value  ); */
/* 	} */
/*       else */
/* 	{	 */
/* 	  glVertex3f(  -epsilon, 0, ep->value ); */
/* 	  glVertex3f( -hepsilon, 0,  ep->value-hepsilon ); */
/* 	} */
	
/*       glEnd(); */

/*       glRasterPos3f( -0.5, 0, ep->value ); */
/*       glPrint( "%d", (int)i++ );       */

/*       glRasterPos3f( -0.8, 0, ep->value ); */
/*       glPrint( "%.2f", ep->value );       */

/*       pop_color();    */
/*     } */

  //puts( "\n" );
}

static GLuint VertShader;
static char * VertSrc = 
"!!ARBvp1.0 \n\
PARAM mvp[4]={state.matrix.mvp}; \n\
TEMP r0,tmp; \n\
DP4  r0.x, mvp[0], vertex.position;	\n\
DP4  r0.y, mvp[1], vertex.position;	\n\
DP4  r0.z, mvp[2], vertex.position;	\n\
DP4  r0.w, mvp[3], vertex.position;	\n\
#RCP  tmp.w, r0.w; \n\
RCP tmp, 8; \n\
MUL tmp.w, r0.w, tmp.w; \n\
#MUL  r0.z, r0.z, r0.w; \n\ 
MUL  r0.z, r0.z, tmp.w; \n\ 
MOV  result.position, r0;    \n\
MOV  result.color, vertex.color;  \n\
END\n ";

void draw_world(  stg_world_t* world )
{
  gui_window_t* win = (gui_window_t*)world->win;
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable( win->canvas );

  // clear the offscreen buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  if( world->win->sphi || world->win->stheta )
    {
      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
    }
  else // zero rotation allows a speed-up
    {
      glDisable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
    }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  if( win->show_alpha )
    { 
      glEnable(GL_LINE_SMOOTH); 
      glEnable(GL_BLEND);
    }
  else
    {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
    }

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



 //glClear (GL_COLOR_BUFFER_BIT);


  if( world->win->follow_selection && world->win->selection_last )
    {      
      glTranslatef(  0,0,-zclip/2.0 );

      // meter scale
      glScalef ( win->scale, win->scale, win->scale ); // zoom

      stg_pose_t gpose;
      stg_model_get_global_pose( world->win->selection_last, &gpose );

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
  // draw the floor a little offset so it doesn't z-fight with the
  // models
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0, 1.0);
  glColor3f( 1,1,1 );
  glRectf( -world->width/2.0, -world->height/2.0,
	   world->width/2.0, world->height/2.0 ); 
  glDisable(GL_POLYGON_OFFSET_FILL);

  // draw the model bounding boxes
  //g_hash_table_foreach( world->models, (GHFunc)model_draw_bbox, NULL);

  // draw the models
  //g_hash_table_foreach( world->models, (GHFunc)model_draw_cb, NULL );

  g_list_foreach( world->children, (GFunc)model_draw_cb, NULL );

  if( win->show_bboxes )   // draw bounding boxes
    {
      g_hash_table_foreach( world->models, (GHFunc)model_bbox_cb, NULL );
      // draw the bbox lists
      draw_endpoints( world );
    }
  
  // get the display list associated with this model
  //int slist = gui_model_get_displaylist( mod, LIST_SELECTED );
  if( recticle_list )
    glCallList( recticle_list );

  // draw the model's data
  g_hash_table_foreach( world->models, (GHFunc)model_draw_data_cb, NULL );

  // draw anything in the assorted displaylist list
  GList* it;
  for( it = dl_list; it; it=it->next )
    {
      int dl = (int)it->data;
      //printf( "Calling dl %d\n", dl );
      glCallList( (int)it->data );
    }


  if( world->win->selection_active
      &&  world->win->selection_active->sense_poly )
    {
      stg_model_t* mod = world->win->selection_active;

      push_color_stgcolor( stg_lookup_color( "green" ));

      gl_draw_polygon_bbox( mod->sense_poly );
      pop_color();
  //}
 /*      // outline the polgons that the selected robot intersects with */
/*       push_color_stgcolor( stg_lookup_color( "blue" )); */
      
/*       GList *b; */
/*       for( b = mod->sense_poly->intersectors; b; b=b->next ) */
/* 	{ */
/* 	  stg_polygon_t* p = (stg_polygon_t*)b->data; */
	  
/* 	  glPushMatrix(); */
	  
/* 	  stg_pose_t pose; */
/* 	  stg_model_get_global_pose( p->mod, &pose ); */
	  
/* 	  // move into this model's coordinate frame */
/* 	  glTranslatef( pose.x, pose.y, pose.z ); */
/* 	  glRotatef( RTOD(pose.a), 0,0,1 ); */
/* 	  gl_draw_polygon3d( p ); */
	  
/* 	  glPopMatrix(); */
/* 	} */

/*       pop_color(); */
    }     

/*       int err; */
      
/*       //if (! glutExtensionSupported("GL_ARB_vertex_program")) */
/*       //Fail("GL_ARB_vertex_program not available on this machine"); */
      
/*       /\* Vertex shader *\/ */
/*       glEnable(GL_VERTEX_PROGRAM_ARB); */
/*       glGenProgramsARB(1, &VertShader); */
/*       glBindProgramARB(GL_VERTEX_PROGRAM_ARB, VertShader); */
/*       glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, */
/* 			 strlen(VertSrc), VertSrc); */

/*       glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &err); */
/*       if (err >= 0) */
/* 	printf("Error in vertex shader %s\n", */
/* 	       glGetString(GL_PROGRAM_ERROR_STRING_ARB)); */
      
 
      
/*       glPushAttrib( GL_ALL_ATTRIB_BITS ); */
/*       // these need to be set */
/*       glEnable(GL_DEPTH_TEST); */
/*       glDepthMask(GL_TRUE); */

/*       glViewport( 0,0, 361, 100 );  */


/*       glMatrixMode (GL_PROJECTION);       */
/*       glPushMatrix();  */
/*       glLoadIdentity (); */
/*       //glScalef( win->scale, win->scale, win->scale ); */

/*       double far = 8.0; */
/*       double near = 0.1; */

/*       //glScalef( 1,1,1.0/far ); */
/*       gluPerspective( 45, 361/100, near, far ); */


      
/*       glMatrixMode( GL_MODELVIEW ); */
/*       glPushMatrix();  */
/*       glLoadIdentity(); */
      
      
/*       stg_pose_t gpose;  */
/*       stg_model_get_global_pose( world->win->selection_last, &gpose );  */

/*       gluLookAt( gpose.x, gpose.y, 0.4, // eye point */
/* 		 gpose.x + cos( gpose.a), // view direction */
/* 		 gpose.y + sin( gpose.a), */
/* 		 0.4, */
/* 		 0,0,1 );// Z is up */
		 

/*       push_color_stgcolor( stg_lookup_color( "gray" )); */

/*       glTranslatef( gpose.x, gpose.y, 0 ); */
/*       GLUquadricObj* q = gluNewQuadric(); */
/*       gluQuadricDrawStyle( q, GLU_LINE ); */
/*       gluSphere( q, 8.0, 20, 20 ); */
/*       gluDeleteQuadric( q ); */
/*       glTranslatef( -gpose.x, -gpose.y, 0 ); */

/*       //glRotatef( RTOD( -gpose.a ), 0,0,1 ); */
      


/*       for( GList* b = mod->sense_poly->intersectors; b; b=b->next )       */
/* 	{ */
/* 	  stg_polygon_t* p = (stg_polygon_t*)b->data; */

/* 	  if( p->mod == world->win->selection_last ) */
/* 	    continue; */
	  
/* 	  push_color_stgcolor( p->mod->color ); */
	  
/* 	  glPushMatrix(); */
	  
/* 	  stg_pose_t pose; */
/* 	  stg_model_get_global_pose( p->mod, &pose ); */
	  
/* 	  // move into this model's coordinate frame */
/* 	  glTranslatef( pose.x, pose.y, pose.z ); */
/* 	  glRotatef( RTOD(pose.a), 0,0,1 ); */
	  
/* 	  //gl_draw_polygon_bbox( p ); */
/* 	  gl_draw_polygon3d( p ); */
	  
/* 	  glPopMatrix();        */

/* 	  pop_color(); */
/* 	} */

/*       //glFlush(); */
      

/*       pop_color(); */
/*       //pop_color(); */
      
/*       glPopMatrix();   */

/*       glPopAttrib(); */

/*       glMatrixMode( GL_PROJECTION ); */
/*       glPopMatrix();   */

      
/*       GLfloat* depths = calloc( sizeof(GLfloat), 361 ); */
/*       glReadPixels( 0,50, 361, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depths ); */
   
/* /\*       puts(""); *\/ */
/* /\*       for( int c=0; c<361; c++ ) *\/ */
/* /\* 	printf( "%.3f ", depths[c] ); *\/ */
/* /\*       puts("");             *\/ */

/*       // HACK alert. woop woop. */
/*       puts(""); */
/*       for( int c=0; c<361; c++ ) */
/* 	printf( "%.2f ", 2 * ((depths[c] * (far-near)) -far/2.0) ); */
/*       puts(""); */

/*     } */


  
/*       glFrustum( 0, 361, */
/* 		 0, 100, */
/* 		 zclip, 2*zclip ); */
      
/*       glScalef ( 1, 1, 1.0 ); // zoom */


      // 1 pixel-per unit

 /*  else */
/*     { */
/*       push_color_rgb( 1,0,0 ); */
/*       glRecti( 0,0,1,1 ); */
/*       pop_color(); */

/*       for( GList* b = mod->sense_poly->intersectors; b; b=b->next )       */
/* 	{ */
/* 	  stg_polygon_t* p = (stg_polygon_t*)b->data; */
	  
/* 	  glPushMatrix(); */
	  

/* 	  //glTranslatef( -gpose.x + (pixels_width/2.0)/win->scale,  */
/* 	  //    -gpose.y + (pixels_height/2.0)/win->scale, */
/* 	  //    -zclip/2.0 );       */


/* 	  /\* stg_pose_t pose; *\/ */
/* /\* 	  stg_model_get_global_pose( p->mod, &pose ); *\/ */
	  
/* /\* 	  // move into this model's coordinate frame *\/ */
/* /\* 	  glTranslatef( pose.x, pose.y, pose.z ); *\/ */
/* /\* 	  glRotatef( RTOD(pose.a), 0,0,1 ); *\/ */
	  
/* /\* 	  //gl_draw_polygon_bbox( p ); *\/ */
/* /\* 	  gl_draw_rectpolygon3d( p ); *\/ */
	  
/* 	  push_color_rgba( 1,0,0,1 );  */
/* 	  glRectf( 0, 0, 1, 1 ); */
/* 	  pop_color(); */

/* 	  glPopMatrix(); */
/* 	  //glMatrixMode( GL_MODELVIEW ); */
/* 	} */
/*       //      glPopMatrix(); */


/*       pop_color();	    */
/*     } */


  if( win->show_thumbnail ) // if drawing the whole world mini-view
    draw_thumbnail( world );

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
  gboolean redraw = FALSE;

  //printf( "mouse pointer %d %d\n",(int)event->x, (int)event->y );

  // convert from window pixel to world coordinates
  double obx, oby, obz;
  widget_to_world( widget, event->x, event->y,
		   world, &obx, &oby, &obz );

  //printf( "mouse pointer (%d %d) world (%.2f %.2f %.2f)\n",
  //  (int)event->x, (int)event->y,
  //  obx, oby, obz );

  if( ! world->win->selection_active )
    {
      // if anything's close, draw a selection recticle
      stg_model_t* near = stg_world_nearest_root_model( world, obx, oby );
      
      if( near )
	{
	  if( recticle_list == 0 ) // no list, get a valid list ID and remember it
	    recticle_list = glGenLists(1);
	  
	  // draw a recticle around the nearby robot
	  glNewList( recticle_list, GL_COMPILE );
	  
	  //glPushMatrix();
	  //glPushAttrib( GL_ALL_ATTRIB_BITS );
	  
	  
	  push_color_rgba( 1, 0, 1, 0 );
	  
	  double dx = near->geom.size.x / 2.0 * 1.3;
	  double dy = near->geom.size.y / 2.0 * 1.3;
	  
	  //glTranslatef( 0,0,0.1 );
	  
	  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	  glRectf( obx-dx, oby-dy, obx+dx, obx+dy );
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	  
	  pop_color();

	  //glPopMatrix();
	  //glPopAttrib();
	  glEndList();
	}
    }

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

	  redraw = TRUE;
	}
    }
  else if( event->state & GDK_BUTTON1_MASK && world->win->dragging )
    {
      //printf( "dragging an object\n" );

      if( world->win->selection_active )
	{
	  //printf( "moving model to %f %f\n", obx, oby );
	  
	  double dx = obx - world->win->selection_pointer_start.x; 
	  double dy = oby - world->win->selection_pointer_start.y; 

	  // move the model by the x,y distance moved since we selected it
	  stg_pose_t pose;
	  stg_model_get_pose( world->win->selection_active, &pose );
	  pose.x = world->win->selection_pose_start.x + dx ;
	  pose.y = world->win->selection_pose_start.y + dy;
	  pose.a = world->win->selection_active->pose.a;
	  stg_model_set_pose(  world->win->selection_active, &pose );
	}
    }  
  else if( event->state & GDK_BUTTON3_MASK && world->win->dragging ) 
    {
      //printf( "rotating an object\n" );
      
      if( world->win->selection_active )
	{
	  //printf( "rotating model to %f %f\n", obx, oby );
	  
	  stg_pose_t pose;
	  stg_model_get_pose(  world->win->selection_active, &pose );

	  if(event->x > beginX )
	    pose.a += 0.15;

	  if(event->x < beginX )
	    pose.a -= 0.15;

	  stg_model_set_pose(  world->win->selection_active, &pose );
	}
    }
  else
    {

      //glMatrixMode (GL_MODELVIEW);
  
      if( event->state & GDK_BUTTON1_MASK ) // PAN
	{	  
	  // find how far the mouse has moved in world coordinates
	  //double lobx, loby, lobz;
	  //widget_to_world( widget, beginX, beginY,
	  //	   world, &lobx, &loby, &lobz );

	  world->win->panx -= (event->x - beginX) ;//(obx - lobx);
	  world->win->pany += (event->y - beginY); //(oby - loby);

	  redraw = TRUE;
	}
      
      if (event->state & GDK_BUTTON3_MASK)
	{
	  if( event->x > beginX ) world->win->scale *= 1.04;
	  if( event->x < beginX ) world->win->scale *= 0.96;
	  //printf( "ZOOM scale %f\n", world->win->scale );
	  redraw = TRUE;
	}

    }
  
  beginX = event->x;
  beginY = event->y;  

  if (redraw)
    world->win->dirty = TRUE; 

  return TRUE;
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
      world->win->dragging = FALSE;
      world->win->selection_active = NULL;
    }

  return TRUE;
}


void select_model_if_close( stg_world_t* world, 
			    double x, double y, double z )
{
  // find nearest object
  
  stg_model_t* sel = stg_world_nearest_root_model( world, x, y );
  
  if( sel )
    {
      // add a selection visualization to the model
      gl_model_selected( sel, NULL );
      
      world->win->selection_last = world->win->selection_active = sel;
      world->selected_models = 
	g_list_append( world->selected_models, sel );
      
      // store the pose of the selected object at the moment of
      // selection
      stg_model_get_pose( sel, &world->win->selection_pose_start );
      
      // and store the 3d pose of the pointer
      world->win->selection_pointer_start.x = x;
      world->win->selection_pointer_start.y = y;
      world->win->selection_pointer_start.z = z;
      
      world->win->dragging = TRUE;
    }  
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

	  
      if( (event->state & modifiers) == GDK_CONTROL_MASK)
	{
	  // ctrl is pressed 
	  //printf( "CTRL-click\n" );
	}
      else
	{
	  //puts( "SELECT" );
	  select_model_if_close( world, obx, oby, obz ); 
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
  glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA   |
					GDK_GL_MODE_DEPTH |
					GDK_GL_MODE_DOUBLE);

  if (glconfig == NULL)
    {
      g_print ("\n*** Cannot find the double-buffered visual.\n");
      g_print ("\n*** Trying single-buffered visual.\n");
      
      /* Try single-buffered visual */
      glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA   |
					    GDK_GL_MODE_DEPTH);
      if (glconfig == NULL)
	{
	  g_print ("*** No appropriate OpenGL-capable visual found.\n");
	  exit (1);
	}
    }

  // experimental
  assert( gllaser = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA   |
					       GDK_GL_MODE_DEPTH |
					       GDK_GL_MODE_DOUBLE));
  
  return glconfig;
}


/***
 *** Creates the simple application window with one
 *** drawing area that has an OpenGL-capable visual.
 ***/
GtkWidget *
gui_create_canvas( stg_world_t* world )
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
  world->win->scale = DEFAULT_WIDTH / (double)world->width; //, world->height );;

  memset( &world->win->click_point, 0, sizeof(world->win->click_point));

  // allocate a drawing list for everything in the world
  world->win->draw_list = glGenLists(1);

  // allocate a drawing list for debug info
  world->win->debug_list = glGenLists(1);

  return drawing_area;
}


/**************************************************************************
 * The following section contains utility function definitions.
 **************************************************************************/



void gui_model_init( stg_model_t* mod )
{
  GList* it;
  for( it=mod->children; it; it=it->next )
      gui_model_init( (stg_model_t*)it->data );

  // GL CALLBACKS

  // recompile display lists when these properties change
  stg_model_add_callback( mod, &mod->polys, gl_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->gui_outline, gl_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->boundary, gl_model_polygons, NULL );
  stg_model_add_callback( mod, &mod->gui_grid, gl_model_grid, NULL );

  // complete redraw when these change
  stg_model_add_callback( mod, &mod->pose, gl_model_redraw, NULL );
  stg_model_add_callback( mod, &mod->color, gl_model_redraw, NULL );
  stg_model_add_callback( mod, &mod->parent, gl_model_redraw, NULL );
}


void gui_ranger_init( stg_model_t* mod )
{
  stg_model_add_property_toggles( mod, 
				  &mod->data,
				  gl_ranger_render_data, // called when toggled on
				  NULL,
				  gl_unrender_list_cb, // called when toggled off
				  (void*)LIST_DATA,
				  "rangerdata",
				  "ranger data",
				  TRUE );  // initial state
}

static int init = 0;
static stg_color_t laser_color=0, bright_color=0, fill_color=0, cfg_color=0, geom_color=0;

void gui_laser_init( stg_model_t* mod )
{
  // we do this just the first tie a laser is created
  if( init == 0 )
    {
      laser_color =   stg_lookup_color(STG_LASER_COLOR);
      bright_color = stg_lookup_color(STG_LASER_BRIGHT_COLOR);
      fill_color = stg_lookup_color(STG_LASER_FILL_COLOR);
      geom_color = stg_lookup_color(STG_LASER_GEOM_COLOR);
      cfg_color = stg_lookup_color(STG_LASER_CFG_COLOR);
      init = 1;
    }

  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, 
				  &mod->data,
				  gl_laser_render_data, // called when toggled on
				  NULL,
				  gl_unrender_list_cb, // called when toggled off
				  (void*)LIST_DATA,
				  "laserdata",
				  "laser data",
				  TRUE );  
}

// TODO

void gui_position_init( stg_model_t* mod )
{}

void gui_gripper_init( stg_model_t* mod )
{}

void gui_bumper_init( stg_model_t* mod )
{}

void gui_ptz_init( stg_model_t* mod )
{}

void gui_puck_init( stg_model_t* mod )
{}

void gui_fiducial_init( stg_model_t* mod )
{
  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod,
				  &mod->data,
				  gl_fiducial_render_data, // called when toggled on
				  NULL,
				  gl_unrender_list_cb, // called when toggled off
				  (void*)LIST_DATA,
				  "fiducialdata",
				  "fiducial data",
				  TRUE );
}

void gui_blobfinder_init( stg_model_t* mod )
{}

void gui_speech_init( stg_model_t* mod )
{}



/**************************************************************************
 * End of file.
 **************************************************************************/


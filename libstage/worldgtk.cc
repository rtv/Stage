/***
 File: gui_gtk.cc: 
 Desc: Implements GTK-based GUI for Stage
 Author: Richard Vaughan

 CVS: $Id: worldgtk.cc,v 1.2 2008-01-15 01:16:49 rtv Exp $
***/


/** @addtogroup stage 
    @{
*/

/** @defgroup window Window

The Stage window consists of a menu bar, a view of the simulated
world, and a status bar.

The world view shows part of the simulated world. You can zoom the
view in and out, and scroll it to see more of the world. Simulated
robot devices, obstacles, etc., are rendered as colored polygons. The
world view can also show visualizations of the data and configuration
of various sensor and actuator models. The View menu has options to
control which data and configurations are rendered.

<h2>Worldfile Properties</h2>

@par Summary and default values

@verbatim
window
(
  # gui properties
  center [0 0]
  size [700 740]
  scale 1.0

  # model properties do not apply to the gui window
)
@endverbatim

@par Details
- size [width:int width:int]
  - size of the window in pixels
- center [x:float y:float]
  - location of the center of the window in world coordinates (meters)
  - scale [?:double]
  - ratio of world to pixel coordinates (window zoom)


<h2>Using the Stage window</h2>


<h3>Scrolling the view</h3>

<p>Left-click and drag on the background to move your view of the world.

<h3>Zooming the view</h3>

<p>Right-click and drag on the background to zoom your view of the
world. When you press the right mouse button, a circle appears. Moving
the mouse adjusts the size of the circle; the current view is scaled
with the circle.

<h3>Saving the world</h3>

<P>You can save the current pose of everything in the world, using the
File/Save menu item. <b>Warning: the saved poses overwrite the current
world file.</b> Make a copy of your world file before saving if you
want to keep the old poses.


<h3>Saving a screenshot</h3>

<p> The File/Export menu allows you to export a screenshot of the
current world view in JPEG or PNG format. The frame is saved in the
current directory with filename in the format "stage-(frame
number).(jpg/png)". 

 You can also save sequences of screen shots. To start saving a
sequence, select the desired time interval from the same menu, then
select File/Export/Sequence of frames. The frames are saved in the
current directory with filenames in the format "stage-(sequence
number)-(frame number).(jpg/png)".

The frame and sequence numbers are reset to zero every time you run
Stage, so be careful to rename important frames before they are
overwritten.

<h3>Pausing and resuming the clock</h3>

<p>The Clock/Pause menu item allows you to stop the simulation clock,
freezing the world. Selecting this item again re-starts the clock.


<h3>View options</h3>

<p>The View menu allows you to toggle rendering of a 1m grid, to help
you line up objects (View/Grid). You can control whether polygons are
filled (View/Fill polygons); turning this off slightly improves
graphics performance. The rest of the view menu contains options for
rendering of data and configuration for each type of model, and a
debug menu that enables visualization of some of the innards of Stage.

*/

/** @} */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkglglext.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#define DEBUG 

#include "stage.hh"
#include "config.h" 

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION 42.0
#endif

#ifndef VERSION
#define VERSION 42.0
#endif

static const stg_msec_t STG_DEFAULT_REDRAW_INTERVAL = 100;

// only models that have fewer rectangles than this get matrix
// rendered when dragged
//#define STG_POLY_THRESHOLD 10

// CALLBACKS - most are simple wrappers for methods

static void cbAbout( GtkAction *action, StgWorldGtk* world )
{
  world->AboutBox();
}

static void cbRealize( GtkWidget *widget, StgWorldGtk* world )
{
  world->Realize( widget );
}

static void cbDestroy( GtkObject *object, StgWorldGtk* world )
{
  PRINT_MSG( "Window destroyed." );
  world->Quit();
}

static gboolean cbDelete( GtkWidget *widget, GdkEvent *event, StgWorldGtk* world )
{
  PRINT_MSG( "Signal delete" );
  return world->DialogQuit( (GtkWindow*)widget );
}

static gboolean cbButtonPress( GtkWidget* widget, GdkEventButton* event, StgWorldGtk* world )
{
  assert( widget == world->GetCanvas() );
  world->CanvasButtonPress( event );
  return TRUE;
}

static gboolean cbButtonRelease( GtkWidget* widget, GdkEventButton* event, StgWorldGtk* world )
{ 
  assert( widget == world->GetCanvas() );
  world->CanvasButtonRelease( event ); 
  return TRUE;
}

static gboolean cbMotionNotify(GtkWidget *widget, GdkEventMotion *event, StgWorldGtk* world )
{
  assert( widget == world->GetCanvas() );
  world->CanvasMotionNotify( event );
  return TRUE;
}

static void cbExportFrameAction( GtkAction* action, StgWorldGtk* world ) 
{ 
  world->ExportWindow();
}

static gboolean cbExportFrame( StgWorldGtk* world ) 
{ 
  world->ExportWindow();
  return TRUE;
}

static void cbDerotate( GtkAction* action, StgWorldGtk* world ) 
{ 
  world->Derotate();
}

static void cbPolygons( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Disable polygons menu item" );
  world->SetShowPolygons( gtk_toggle_action_get_active( action ) );
}

static void cbPause( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Pause menu item" );
  
  if( gtk_toggle_action_get_active( action ) )
    world->Stop();
  else
    world->Start();
}

static void cbQuadtree( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "QuadTree menu item" );
  world->SetShowQuadtree(  gtk_toggle_action_get_active( action ) );
}

static void cbOccupancy( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Occupancy menu item" );
  world->SetShowOccupancy(  gtk_toggle_action_get_active( action ) );
}

static void cbGrid( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Grid menu item" );  
  world->SetShowGrid( gtk_toggle_action_get_active( action ) );
}

static void cbFollow( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Follow menu item" );  
  world->SetFollowSelection( gtk_toggle_action_get_active( action ) );
}

static void cbAlpha( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Alpha menu item" );  
  world->SetShowAlpha( gtk_toggle_action_get_active( action ) );
}

static void cbTrails( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Trails menu item" );  
  world->SetShowTrails( gtk_toggle_action_get_active( action ) );
}

static void cbFill( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Fill menu item" );  
  world->SetShowFill( gtk_toggle_action_get_active( action ) );
}

static void cbThumbnail(  GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Thumbnail menu item" );  
  world->SetShowThumbnail( gtk_toggle_action_get_active( action ) );
}

static void cbBBoxes( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "BBox menu item" );
  world->SetShowBBoxes( gtk_toggle_action_get_active( action ) );
}

static gboolean cbExpose( GtkWidget* widget, GdkEventExpose *event, StgWorldGtk* world )
{
  PRINT_DEBUG( "Expose" );
  world->Draw();
  return TRUE;
}

static gboolean cbScroll( GtkWidget *widget, GdkEventScroll *event, StgWorldGtk* world )
{
  switch( event->direction ) 
    {
    case GDK_SCROLL_UP:
      world->Zoom( 1.05 );
      break;
    case GDK_SCROLL_DOWN:
      world->Zoom( 0.95 );
      break;
    default:
      break;
    }
  
  return TRUE;
}

static gboolean cbKeyPress( GtkWidget  *widget, GdkEventKey *event, StgWorldGtk* world )
{
  switch (event->keyval)
    {
    case GDK_plus:
      world->Zoom( 0.9 );
      break;
      
    case GDK_minus:
      world->Zoom( 1.1 );
      break;

    case GDK_space:
      world->Derotate();
      break;

    default:
      return FALSE;
    }

  return TRUE;
}

void cbSave( GtkAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Save menu item" );
  world->Save();
}

void cbReload( GtkAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Reload menu item" );
  world->Reload();
}

void cbExit( GtkAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Exit menu item" );
  world->DialogQuit( NULL );
}

void cbExportSequence( GtkToggleAction* action, StgWorldGtk* world )
{
  PRINT_DEBUG( "Export sequence menu item" );
  if( gtk_toggle_action_get_active(action) )
    world->ExportSequenceStart();
  else
    world->ExportSequenceStop();    
}

static gboolean cbConfigure( GtkWidget* widget, GdkEventConfigure* event, StgWorldGtk* world )
{
  PRINT_DEBUG( "Configure event" );
  
  if( widget == world->GetCanvas() )
    world->SetWindowSize( widget->allocation.width, 
			  widget->allocation.height );
  else
    PRINT_WARN1( "received configure event from unexpected widget %p", widget );
  return TRUE;
}


static void cbExportInterval( GtkRadioAction* action, 
			      GtkRadioAction *current, 
			      StgWorldGtk* world )
{
  PRINT_DEBUG( "ExportInterval menu item" );
  world->SetExportInterval( gtk_radio_action_get_current_value(action) );
}

static void cbExportFormat( GtkRadioAction* action, 
			    GtkRadioAction *current, 
			    StgWorldGtk* world )
{
  PRINT_DEBUG( "ExportFormat menu item" );
  world->SetExportFormat( (stg_image_format_t)gtk_radio_action_get_current_value(action) );
}


// END CALLBACKS


#define TOGGLE_PATH "/Main/View"

extern int _stg_quit;

static GtkUIManager *ui_manager = NULL;

/* Normal items */
static GtkActionEntry entries[] = {
  { "File", NULL, "_File" },
  { "View", NULL, "_View" },
  { "Debug", NULL, "_Debug" },
  { "Edit", NULL, "_Edit" },
  { "Clock", NULL, "_Clock" },
  { "Help", NULL, "_Help" },
  { "Model", NULL, "_Models" },
  { "Screenshot", NULL, "Screenshot" },
  { "ExportInterval", NULL, "Screenshot interval" },
  { "ExportFormat", NULL, "Screenshot bitmap format" },
  { "ExportFrame", NULL, "Single frame", "<ctrl>f", "Export a screenshot to disk", G_CALLBACK(cbExportFrameAction) },
  { "About", NULL, "About Stage", NULL, NULL, G_CALLBACK(cbAbout) },
  //  { "Preferences", NULL, "Preferences...", NULL, NULL, G_CALLBACK(gui_action_prefs) },
  { "Save", GTK_STOCK_SAVE, "_Save", "<control>S", "Save world state", G_CALLBACK(cbSave) },
  { "Reset", GTK_STOCK_OPEN, "_Reset", "<control>R", "Reset to last saved world state", G_CALLBACK(cbReload) },
  { "Derotate", NULL, "Zero rotation", NULL, "Reset the 3D view rotation to zero", G_CALLBACK(cbDerotate) },
  { "Exit", GTK_STOCK_QUIT, "E_xit", "<control>Q", "Exit the program", G_CALLBACK(cbExit) },
};

/* Toggle items */
static GtkToggleActionEntry toggle_entries[] = {
  { "Pause", NULL, "_Pause", "P", "Pause the simulation clock", G_CALLBACK(cbPause), 0 },
  { "Fill", NULL, "Fill blocks", NULL, "Toggle drawing of filled or outlined polygons", G_CALLBACK(cbFill), 1 },
  { "ExportSequence", NULL, "Sequence of frames", "<control>G", "Export a sequence of screenshots at regular intervals", G_CALLBACK(cbExportSequence), 0 },

  { "Polygons", NULL, "Show blocks", "B", NULL, G_CALLBACK(cbPolygons), 1 },
  { "Alpha", NULL, "Alpha channel", "A", NULL, G_CALLBACK(cbAlpha), 1 },
  { "Thumbnail", NULL, "Thumbnail", "T", NULL, G_CALLBACK(cbThumbnail), 0 },
  { "Bboxes", NULL, "Bounding _boxes", "B", NULL, G_CALLBACK(cbBBoxes), 1 },
    { "Trails", NULL, "Trails", NULL, NULL, G_CALLBACK(cbTrails), 0 },
    { "Follow", NULL, "Follow selection", NULL, NULL, G_CALLBACK(cbFollow), 0 },
  { "Grid", NULL, "_Grid", "G", "Toggle drawing of 1-metre grid", G_CALLBACK(cbGrid), 1 },

  /*   { "DebugRays", NULL, "_Raytrace", "<alt>R", "Draw sensor rays", G_CALLBACK(gui_action_raytrace), 0 }, */
  /*   { "DebugGeom", NULL, "_Geometry", "<alt>G", "Draw model geometry", G_CALLBACK(gui_action_geom), 0 }, */
  { "DebugMatrixTree", NULL, "Tree", NULL, "Show occupancy tree", G_CALLBACK(cbQuadtree), 0 }, 
   { "DebugMatrixOccupancy", NULL, "Leaves", NULL, "Show occupancy grid", G_CALLBACK(cbOccupancy), 0 }, 
/*   { "DebugMatrixDelta", NULL, "Matrix _Delta", "<alt>D", "Show changes to quadtree", G_CALLBACK(gui_action_matrixdelta), 0 }, */
};

/* Radio items */
static GtkRadioActionEntry export_format_entries[] = {
  { "ExportFormatPNG", NULL, "PNG", NULL, "Portable Network Graphics format", STG_IMAGE_FORMAT_PNG },
  { "ExportFormatJPEG", NULL, "JPEG", NULL, "JPEG format", STG_IMAGE_FORMAT_JPG },
};
 
static GtkRadioActionEntry export_freq_entries[] = {
  { "ExportInterval1", NULL, "0.1 seconds", NULL, NULL, 100 },
  { "ExportInterval2", NULL, "0.2 seconds", NULL, NULL, 200 },
  { "ExportInterval5", NULL, "0.5 seconds", NULL, NULL, 500 },
  { "ExportInterval10", NULL, "1 second", NULL, NULL, 1000 },
  { "ExportInterval50", NULL, "5 seconds", NULL, NULL, 5000 },
  { "ExportInterval100", NULL, "10 seconds", NULL, NULL, 10000 },
};

static const char *ui_description =
"<ui>"
"  <menubar name='Main'>"
"    <menu action='File'>"
"      <menuitem action='Save'/>"
"      <menuitem action='Reset'/>"
"      <separator/>"
"        <menu action='Screenshot'>"
"          <menuitem action='ExportFrame'/>" 
"          <menuitem action='ExportSequence'/>" 
"          <separator/>" 
"          <menuitem action='ExportFormat'/>"
"          <menuitem action='ExportFormatPNG'/>"
"          <menuitem action='ExportFormatJPEG'/>"
"          <separator/>"
"          <menuitem action='ExportInterval'/>"
"          <menuitem action='ExportInterval1'/>"
"          <menuitem action='ExportInterval2'/>"
"          <menuitem action='ExportInterval5'/>"
"          <menuitem action='ExportInterval10'/>"
"          <menuitem action='ExportInterval50'/>"
"          <menuitem action='ExportInterval100'/>"
"        </menu>"
"      <separator/>"
"      <menuitem action='Exit'/>"
"    </menu>"
"    <menu action='Edit'>"
//"      <menuitem action='Preferences'/>"
"    </menu>"
"    <menu action='View'>"
"      <menuitem action='Derotate'/>"
"      <separator/>"
"      <menuitem action='Fill'/>"
"      <menuitem action='Polygons'/>"
"      <menuitem action='Grid'/>"
"      <menuitem action='Bboxes'/>"
"      <menuitem action='Follow'/>"
"      <menuitem action='Alpha'/>"
"      <menuitem action='Trails'/>"
"      <menuitem action='Thumbnail'/>"
"      <separator/>"
/* "      <separator/>" */
/* "        <menu action='Debug'>" */
/* "          <menuitem action='DebugRays'/>" */
/* "          <menuitem action='DebugGeom'/>" */
/* "          <menuitem action='DebugMatrixDelta'/>" */
/* "          <separator/>" */
/* "            <menu action='Model'>" */
/* "              <separator/>" */
/* "            </menu>" */
/* "        </menu>" */
"      <separator/>"
"    </menu>"
"    <menu action='Debug'>"
"      <menuitem action='DebugMatrixTree'/>" 
"      <menuitem action='DebugMatrixOccupancy'/>" 
"    </menu>"
"    <menu action='Clock'>"
"      <menuitem action='Pause'/>"
"    </menu>"
"    <menu action='Help'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";


void StgWorldGtk::Init( int* argc, char** argv[] )
{
  PRINT_DEBUG( "Initializing GUI libraries" );
  
  // Initialise the gtk libs. These calls will remove any arguments
  // they process from the arg list
  gtk_init(argc, argv);
  gtk_gl_init (argc, argv);

  g_set_application_name( PACKAGE_NAME );

  // pass the remaining arguments to the world
  StgWorld::Init( argc, argv );
}


StgWorldGtk::StgWorldGtk()
  : StgWorld()
{
  if( ! init_done )
    {
      PRINT_WARN( "StgWorldGtk::Init() must be called before a StgWorldGtk is created." );
      exit(-1);
    }

  PRINT_DEBUG( "Constructing StgWorldGtk" );
  
  this->graphics = true;

  // Create a top-level window
  frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  assert( frame );

  gtk_window_set_default_size( GTK_WINDOW(frame), 
			       STG_DEFAULT_WINDOW_WIDTH,
			       STG_DEFAULT_WINDOW_HEIGHT  );

  layout = gtk_vbox_new(FALSE, 0);
  assert(layout);

  GtkHBox* hbox = GTK_HBOX(gtk_hbox_new( FALSE, 3 ));

  status_bar = GTK_STATUSBAR(gtk_statusbar_new());
  gtk_statusbar_set_has_resize_grip( status_bar, FALSE );
  clock_label = GTK_LABEL(gtk_label_new( "clock" ));

  gtk_container_add(GTK_CONTAINER(frame), layout);
  
  g_signal_connect(GTK_OBJECT(frame),
		   "destroy",
		   GTK_SIGNAL_FUNC(cbDestroy),
		   this );
  
  g_signal_connect(GTK_OBJECT(frame),
		   "delete-event",
		   GTK_SIGNAL_FUNC(cbDelete),
		   this );

  gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(clock_label), FALSE, FALSE, 5);

  gtk_box_pack_start(GTK_BOX(hbox), 
  	   GTK_WIDGET(status_bar), TRUE, TRUE, 5);

 
  // we'll add these backwards so we can stick the menu in later
  gtk_box_pack_end(GTK_BOX(layout), 
		   GTK_WIDGET(hbox), FALSE, TRUE, 0);
  
  /* Configure OpenGL framebuffer. */
  /* Try double-buffered visual */
  glconfig = 
    gdk_gl_config_new_by_mode( (GdkGLConfigMode)
			       (GDK_GL_MODE_RGBA |
				GDK_GL_MODE_DEPTH |
				GDK_GL_MODE_DOUBLE) );
  assert( glconfig );

  // Drawing area to draw OpenGL scene.
  canvas = gtk_drawing_area_new ();
  gtk_widget_set_size_request (canvas, 400, 400 );

  /* Set OpenGL-capability to the widget */
  gtk_widget_set_gl_capability (canvas,
				glconfig,
				NULL,
				TRUE,
				GDK_GL_RGBA_TYPE);

  gtk_widget_add_events (canvas,
			 GDK_POINTER_MOTION_MASK      |
			 GDK_BUTTON1_MOTION_MASK      |
			 GDK_BUTTON3_MOTION_MASK      |
			 GDK_BUTTON_PRESS_MASK      |
			 GDK_BUTTON_RELEASE_MASK    |
			 GDK_SCROLL_MASK            |
			 GDK_KEY_PRESS_MASK         |
			 GDK_VISIBILITY_NOTIFY_MASK);

  /* Connect signal handlers to the drawing area */
  g_signal_connect_after (G_OBJECT (canvas), "realize",
                          G_CALLBACK(cbRealize), this );
  g_signal_connect (G_OBJECT (canvas), "configure_event",
		    G_CALLBACK(cbConfigure), this );
  g_signal_connect (G_OBJECT (canvas), "expose_event",
		    G_CALLBACK(cbExpose), this );
  g_signal_connect (G_OBJECT (canvas), "motion_notify_event",
		    G_CALLBACK(cbMotionNotify), this );
  g_signal_connect (G_OBJECT (canvas), "button_press_event",
		    G_CALLBACK(cbButtonPress), this );
  g_signal_connect (G_OBJECT (canvas), "button_release_event",
		    G_CALLBACK(cbButtonRelease), this );  
  g_signal_connect (G_OBJECT (canvas), "scroll-event",
		    G_CALLBACK (cbScroll), this );
  
  // this doesn't work attached to the canvas, so we use the frame
  g_signal_connect( G_OBJECT (frame), "key_press_event",
		    G_CALLBACK(cbKeyPress), this );
  
  gtk_widget_show (canvas);

  // set up a reasonable default scaling, so that the world fits
  // neatly into the window.
  scale = width; //(1 to 1)

  memset( &click_point, 0, sizeof(click_point));

  gtk_box_pack_end( GTK_BOX(layout),
		    canvas, TRUE, TRUE, 0);

  dirty = true; // redraw requested

  selected_models = NULL;

  char txt[256];
  snprintf( txt, 256, " %s v%s", PACKAGE, VERSION );
  gtk_statusbar_push( status_bar, 0, txt ); 

  snprintf( txt, 256, " %s: %s", PACKAGE, token );
  gtk_window_set_title( GTK_WINDOW(frame), txt );

  follow_selection = false;
  show_quadtree = false;
  show_occupancy = false;
  show_cfg = true;
  show_data = true;
  show_cmd = false;
  show_geom = true;
  show_alpha = true;
  show_fill = true;
  show_polygons = true;
  show_thumbnail = false;
  show_bboxes = true;
  show_trails = false;
  show_grid = false;

  frame_interval = 500; // ms
  frame_format = STG_IMAGE_FORMAT_PNG;
  frame_series = 0;
  frame_index = 0;
  
  sphi = 0.0;
  stheta = 0.0;

  this->timer_handle = 0;
  this->redraw_interval = STG_DEFAULT_REDRAW_INTERVAL; //msec

  // CREATE THE MENUS
  ui_manager = gtk_ui_manager_new ();
  
  // actions
  action_group = gtk_action_group_new ("MenuActions");
  
  gtk_action_group_add_actions( action_group, entries, 
				G_N_ELEMENTS (entries),
				this );
  
  gtk_action_group_add_toggle_actions( action_group,toggle_entries, 
				       G_N_ELEMENTS(toggle_entries), 
				       this );
  
  gtk_action_group_add_radio_actions( action_group, export_format_entries, 
				      G_N_ELEMENTS(export_format_entries), 
				      frame_format, 
				      G_CALLBACK(cbExportFormat),
				      this );
  
  gtk_action_group_add_radio_actions( action_group, export_freq_entries, 
				      G_N_ELEMENTS(export_freq_entries),
				      frame_interval, 
				      G_CALLBACK(cbExportInterval), 
				      this );
  
  gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

  // grey-out the radio list labels
  
  // gtk+-2.6 code
  //gtk_action_set_sensitive(gtk_action_group_get_action( action_group, "ExportFormat" ), FALSE );
  //gtk_action_set_sensitive(gtk_action_group_get_action( action_group, "ExportInterval" ), FALSE );

  // gtk+-2.4 code
  int sensi = FALSE;
  g_object_set( gtk_action_group_get_action( action_group, "ExportFormat" ), "sensitive", sensi, NULL );
  g_object_set( gtk_action_group_get_action( action_group, "ExportInterval" ), "sensitive", sensi, NULL);

  // accels
  //GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (ui_manager);
  //gtk_window_add_accel_group(GTK_WINDOW (canvas->frame), accel_group);
  
  // menus
  gtk_ui_manager_set_add_tearoffs( ui_manager, TRUE );
  
  GError* error = NULL;
  if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error))
    {
      g_message ("building menus failed: %s", error->message);
      g_error_free (error);
      exit (EXIT_FAILURE);
    }
  
  // MENUS DONE

  // install the widget
  GtkWidget *menubar = gtk_ui_manager_get_widget (ui_manager, "/Main");
  gtk_box_pack_start (GTK_BOX(layout), menubar, FALSE, FALSE, 0);
  //gtk_layout_put(GTK_LAYOUT(canvas->layout), menubar, 0,300 );
  
 
  toggle_list = NULL;

  // show the window
  gtk_widget_show_all(frame);
}


gboolean timed_redraw( StgWorldGtk* world )
{
  //puts( "TIMED REDRAW" );
  world->RenderClock();
  world->Draw();
  return TRUE;
}

/***
 *** The "realize" signal handler. All the OpenGL initialization
 *** should be performed here, such as default background colour,
 *** certain states etc.
 ***/

void  StgWorldGtk::Realize( GtkWidget *widget )
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

  
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  double pixels_width =  canvas->allocation.width;
  double pixels_height = canvas->allocation.height ;
  double zclip = hypot(width, height) * scale;

  glOrtho( -pixels_width/2.0, pixels_width/2.0,
	   -pixels_height/2.0, pixels_height/2.0,
	   0, zclip );
  
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  glTranslatef(  -panx, 
		 -pany, 
		 -zclip / 2.0 );
  
  glRotatef( RTOD(-stheta), 1,0,0);      
  glRotatef( RTOD(sphi), 0.0, 0.0, 1.0);   // rotate about z - pitch
      
  // meter scale
  glScalef ( scale, scale, scale ); // zoom


  gdk_gl_drawable_gl_end (gldrawable);


  // start a timer to redraw the window periodically
  timer_handle = g_timeout_add( redraw_interval, (GSourceFunc)timed_redraw, this );
}

void StgWorldGtk::CanvasToWorld( int px, int py, 
				 double *wx, double *wy, double* wz )
{
  // convert from window pixel to world coordinates
  //int width = widget->allocation.width;
  int height = canvas->allocation.height;

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

/***
 *** The "motion_notify_event" signal handler. Any processing required when
 *** the OpenGL-capable drawing area is under drag motion should be done here.
 ***/
void StgWorldGtk::CanvasMotionNotify( GdkEventMotion *event )
{
  //printf( "mouse pointer %d %d\n",(int)event->x, (int)event->y );

  // only change the view angle if CTRL is pressed
  guint modifiers = gtk_accelerator_get_default_mod_mask ();

  if( (event->state & modifiers) == GDK_CONTROL_MASK)
    {
      if (event->state & GDK_BUTTON1_MASK)
	{
	  sphi += (float)(event->x - beginX) / 100.0;
	  stheta += (float)(beginY - event->y) / 100.0;
	  
	  // stop us looking underneath the world
	  stheta = constrain( stheta, 0, M_PI/2.0 );
	  dirty = true; 
	}
    }
  else if( event->state & GDK_BUTTON1_MASK && dragging )
    {
      //printf( "dragging an object\n" );

      if( selected_models )
	{
	  // convert from window pixel to world coordinates
	  double obx, oby, obz;
	  CanvasToWorld( event->x, event->y,
			 &obx, &oby, &obz );
	  
	  double dx = obx - selection_pointer_start.x; 
	  double dy = oby - selection_pointer_start.y; 

	  selection_pointer_start.x = obx;
	  selection_pointer_start.y = oby;

	  //printf( "moved mouse %.3f, %.3f meters\n", dx, dy );

	  // move all selected models by dx,dy 
	  for( GList* it = selected_models;
	       it;
	       it = it->next )
	    ((StgModel*)it->data)->AddToPose( dx, dy, 0, 0 );

	  dirty = true; 
	}
    }  
  else if( event->state & GDK_BUTTON3_MASK && dragging ) 
    {
      //printf( "rotating an object\n" );
      
      if( selected_models )
	{
	  // convert from window pixel to world coordinates
	  double obx, oby, obz;
	  CanvasToWorld( event->x, event->y,
			 &obx, &oby, &obz );
	  
	  //printf( "rotating model to %f %f\n", obx, oby );
	  
	  for( GList* it = selected_models;
	       it;
	       it = it->next )
	    {
	      double da = (event->x < beginX ) ? -0.15 : 0.15;
	      
	      ((StgModel*)it->data)->AddToPose( 0,0,0, da );
	    }
	  dirty = true; 
	}
    }
  else
    {
      if( event->state & GDK_BUTTON1_MASK ) // PAN
	{	  
	  panx -= (event->x - beginX);
	  pany += (event->y - beginY);
	  dirty = true; 

	}
      
      if (event->state & GDK_BUTTON3_MASK)
	{
	  if( event->x > beginX ) scale *= 1.04;
	  if( event->x < beginX ) scale *= 0.96;
	  //printf( "ZOOM scale %f\n", scale );
	  dirty = true; 
	}
    }
  
  beginX = event->x;
  beginY = event->y;  
}



void StgWorldGtk::CanvasButtonRelease( GdkEventButton* event )
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
	  dragging = FALSE;
	  
	  // clicked away from a model - clear selection
	  g_list_free( selected_models );
	  selected_models = NULL;
	  dirty = true;
	}
    }
}




/***
 *** The "button_press_event" signal handler. Any processing required when
 *** mouse buttons are pressed on the OpenGL-
 *** capable drawing area should be done here.
 ***/
void StgWorldGtk::CanvasButtonPress( GdkEventButton* event )
{
  
  // convert from window pixel to world coordinates
  double obx, oby, obz;
  CanvasToWorld( event->x, event->y,
		 &obx, &oby, &obz );
  
  // ctrl is pressed - choose this point as the center of rotation
  //printf( "mouse click at (%.2f %.2f %.2f)\n", obx, oby, obz );
  
  click_point.x = obx;
  click_point.y = oby;
  
  beginX = event->x;
  beginY = event->y;

  if( event->button == 1 || event->button == 3 ) // select object
    {
      // only change the view angle if CTRL is pressed
      guint modifiers = gtk_accelerator_get_default_mod_mask ();
      
      StgModel* sel = NearestRootModel( obx, oby );
      
      if( sel )
	{
	  // if model is already selected, unselect it
	  if( GList* link = g_list_find( selected_models, sel ) )
	    {
	      selected_models = 
		g_list_remove_link( selected_models, link );

	      dirty = true;
	      return;
	    }			  
	    
	  //}	  

	  printf( "selected model %s\n", sel->Token() );
	  
	  //selection_active = sel;
	  
	  // store the pose of the selected object at the moment of
	  // selection
	  sel->GetPose( &selection_pose_start );
	  
	  // and store the 3d pose of the pointer
	  selection_pointer_start.x = obx;
	  selection_pointer_start.y = oby;
	  selection_pointer_start.z = obz;
	  
	  dragging = true;
	  
	  // need to redraw to see the selection visualization
	  dirty = true;
	  
	  // unless shift is held, we empty the list of selected objects
	  if( (event->state & modifiers) != GDK_SHIFT_MASK)
	    {
	      g_list_free( selected_models );
	      selected_models = NULL;
	    }
	  
	  selected_models = 
	    g_list_prepend( selected_models, sel );	 
	}
    }
}

StgWorldGtk::~StgWorldGtk()
{
  PRINT_DEBUG( "StgWorldGtk destructor" );

  // Process remaining events (might catch something important, such
  // as a "save" menu click)
  while (gtk_events_pending())
    gtk_main_iteration();

  // do something?
}


extern const GdkPixdata logo_pixbuf;

void StgWorldGtk::AboutBox()
{
  //const char* program_name = "Stage";
  
  const GdkPixbuf* logo = 
    gdk_pixbuf_from_pixdata( &logo_pixbuf, true, NULL );
  assert(logo);
  
  const char* authors[] = STG_STRING_AUTHORS;
  const char* copyright = STG_STRING_COPYRIGHT;
  const char* website = STG_STRING_WEBSITE;
  const char* comments = STG_STRING_DESCRIPTION;
  const char* license = STG_STRING_LICENSE;

  /* GTK+2 >= version 2.10 */
  gtk_show_about_dialog( NULL,
			 //"program-name",program_name, // GTK bug?
			 "version", PACKAGE_VERSION,
			 "authors", authors,
			 "logo", logo,
			 "copyright", copyright,
			 "license", license,
			 "website", website,
			 "comments", comments,
			 NULL );
  
  /* GTK+2 < version 2.10 */
//   const char* str =  "Stage\nVersion %s\n"
//     "Part of the Player/Stage Project\n"
//     "[http://playerstage.sourceforge.net]\n\n"
//     "Copyright Richard Vaughan, Brian Gerkey, Andrew Howard\n"
//     " and contributors 2000-2007.\n\n"
//     "Released under the GNU General Public License v2.";
  
//   GtkMessageDialog* about = (GtkMessageDialog*)
//     gtk_message_dialog_new( NULL, 
// 			    (GtkDialogFlags)0, 
// 			    (GtkMessageType)GTK_MESSAGE_INFO,
// 			    GTK_BUTTONS_CLOSE, 
// 			    str,
// 			    PACKAGE_VERSION);
    
//   g_signal_connect (about, "destroy",
// 		    G_CALLBACK(gtk_widget_destroyed), &about);
  
//   /* Destroy the dialog when the user responds to it (e.g. clicks a button) */
//   g_signal_connect_swapped( about, "response",
// 			    G_CALLBACK (gtk_widget_destroy),
// 			    about );

//   gtk_widget_show(GTK_WIDGET(about));
}


// defined in gui_prefs.c
//GtkWidget* create_prefsdialog( void );

//void gui_action_prefs( GtkAction *action, gpointer user_data)
//{
  //GtkWidget* dlg = create_prefsdialog();
  //gtk_widget_show(dlg);
//}



// set the frame interval in milliseconds
void StgWorldGtk::SetExportInterval( int frame_interval )
{
  this->frame_interval = frame_interval;
}


// set the graphics file format for window dumps
void StgWorldGtk::SetExportFormat( stg_image_format_t frame_format )
{
  this->frame_format = frame_format;
}

void StgWorldGtk::Derotate()
{
  PRINT_DEBUG1( "World %s Resetting view rotation.", this->token );
  this->stheta = 0.0;
  this->sphi = 0.0;
  dirty = true;
}

// void StgWorldGtk::AddTreeItem( StgModel* mod )
// {
//   GtkWidget* m = 
//     gtk_ui_manager_get_widget( ui_manager, "/Main/View/Debug/Model" );
  
//   if( m == NULL )
//     {
//       puts( "Warning: debug model menu not found" );
//       return;
//     }
  
//   GtkWidget* menu = GTK_WIDGET(gtk_menu_item_get_submenu(GTK_MENU_ITEM(m)));

//   GtkWidget* item = gtk_menu_item_new_with_label(  mod->Token() );
//   assert(item);

//   gtk_widget_set_sensitive( item, FALSE );

//   //GtkWidget* submenu = gtk_menu_new();
//   //gtk_menu_item_set_submenu( item, submenu );  
//   //g_signal_connect( item, "activate", test, (void*)mod );
 
//   gtk_menu_shell_append( (GtkMenuShell*)menu, item );

//   gtk_widget_show(item);
// }


// void StgWorldGtk::TogglePropertyCallback( GtkToggleAction* action, 
// 					  stg_property_toggle_args_t* args )
// {
//   //puts( "TOGGLE!" );

//   int active = gtk_toggle_action_get_active( action );
//   if( active )
//     {
//       if( args->callback_off )
// 	{
// 	  //printf( "removing OFF callback\n" );
// 	  args->mod->RemoveCallback( args->member, 
// 				     args->callback_off );
// 	}
      
//       if( args->callback_on )
// 	{
// 	  //printf( "adding ON callback with args %s\n", (char*)args->arg_on );
// 	  args->mod->AddCallback( args->member,
// 				  args->callback_on, 
// 				  args->arg_on );

// 	  // trigger the callback
// 	  //model_change( args->mod, args->member );
// 	  args->mod->CallCallbacks( args->member );
// 	}
//     }
//   else
//     {
//       if( args->callback_on )
// 	{
// 	  //printf( "removing ON callback\n" );
// 	  args->mod->RemoveCallback( args->member,  
// 				     args->callback_on );
// 	}
      
//       if( args->callback_off )
// 	{
// 	  //printf( "adding OFF callback with args %s\n", (char*)args->arg_off );
// 	  args->mod->AddCallback( args->member,
// 				  args->callback_off, 
// 				  args->arg_off );

// 	  // trigger the callback
// 	  //model_change( args->mod, args->member );
// 	  args->mod->CallCallbacks( args->member );
// 	}
      
//     }
// }



// void StgModel::AddPropertyToggles( void* member, 
// 				   stg_model_callback_t callback_on,
// 				   void* arg_on,
// 				   stg_model_callback_t callback_off,
// 				   void* arg_off,
// 				   char* name,
// 				   char* label,
// 				   gboolean enabled )
// {
//   StgModel* mod = this;
  
//   gui_window_t* win = (gui_window_t*)mod->world->win;

//   stg_property_toggle_args_t* args = (stg_property_toggle_args_t*)
//     calloc(sizeof(stg_property_toggle_args_t),1);
  
//   args->mod = mod;
//   args->name = name;
//   args->member = member; 
//   args->callback_on = callback_on;
//   args->callback_off = callback_off;
//   args->arg_on = arg_on;
//   args->arg_off = arg_off;

//   static GtkActionGroup* grp = NULL;  
//   GtkAction* act = NULL;
  
//   if( ! grp )
//     {
//       grp = gtk_action_group_new( "DynamicDataActions" );
//       gtk_ui_manager_insert_action_group(ui_manager, grp, 0);
//     }      
//   else
//     // find the action associated with this label
//     act = gtk_action_group_get_action( grp, name );
  
//   if( act == NULL )
//     {
//       //printf( "creating new action/item for prop %s\n", propname );
      
//       GtkToggleActionEntry entry;
//       memset( &entry, 0, sizeof(entry));
//       entry.name = name;
//       entry.label = label;
//       entry.tooltip = NULL;
//       entry.callback = G_CALLBACK(toggle_property_callback);
      
//       //override setting with value in worldfile, if one exists
//       if( win->world->wf->PropertyExists(  win->wf_section, name ))
// 	enabled = win->world->wf->ReadInt( win->wf_section, 
// 					   name,
// 					   enabled );
      
//       entry.is_active = enabled;
            
//       gtk_action_group_add_toggle_actions( grp, &entry, 1, args  );        
//       guint merge = gtk_ui_manager_new_merge_id( ui_manager );
//       gtk_ui_manager_add_ui( ui_manager, 
// 			     merge,
// 			     TOGGLE_PATH, 
// 			     name, 
// 			     name, 
// 			     GTK_UI_MANAGER_AUTO, 
// 			     FALSE );
      
//       act = gtk_action_group_get_action( grp, name );
      
//       // store the action in the toggle structure for recall
//       args->action = act;
//       args->path = strdup(name);

      
//       // stash this structure in the window's pointer list
//       win->toggle_list = 
// 	g_list_append( win->toggle_list, args );
      
//     }
//   else
//     g_signal_connect( act, "activate",  G_CALLBACK(toggle_property_callback), args );
  
//   // if we start enabled, attach the 'on' callback
//   if( enabled )
//     {
//       //printf( "adding ON callback with args %s\n", (char*)args->arg_on );
//       args->mod->AddCallback( args->member,
// 			      args->callback_on, 
// 			      args->arg_on );
      
//       // trigger the callback
//       //model_change( args->mod, args->member );
//       args->mod->CallCallbacks( args->member );
//     }  
// }

/* gboolean timeout( stg_world_t* world ) */
/* { */
/*   puts( "TIMEOUT" ); */
  
/*   char* clock = stg_world_clockstring( world ); */
/*   gtk_label_set_text( clock_label, clock ); */
/*   free(clock); */
  
/*   if( dirty ) */
/*     { */
/*       /\* Invalidate the whole window to trigger a redraw *\/ */
/*       gdk_window_invalidate_rect( canvas->window, &canvas->allocation, FALSE); */
/*       dirty = FALSE; */
/*     } */
  
/*   return TRUE; */
/* } */


// void gui_load_toggle( gui_window_t* win, 
// 		      int section,
// 		      int* var, 
// 		      char* action_name, 
// 		      char* keyword )
// {
//   if( win->world->wf->PropertyExists( section, keyword ) )
//     {
//       *var = win->world->wf->ReadInt( section, keyword, *var );
      
//       GtkToggleAction* action = (GtkToggleAction*)
// 	gtk_action_group_get_action( win->action_group, action_name );
      
//       if( action )      
// 	gtk_toggle_action_set_active( action, *var );
//       else
// 	PRINT_ERR1( "Failed to get menu action name \"%s\"", action_name );
//     }
// }


void StgWorldGtk::Load( const char* filename )
{
  PRINT_DEBUG1( "%s.Load()", token );

  StgWorld::Load( filename);
  
  wf_section = wf->LookupEntity( "window" );

  if( wf_section < 1) // no section defined
    return;

  panx = wf->ReadTupleFloat(wf_section, "center", 0, panx );
  pany = wf->ReadTupleFloat(wf_section, "center", 1, pany );
  stheta = wf->ReadTupleFloat( wf_section, "rotate", 0, stheta );
  sphi = wf->ReadTupleFloat( wf_section, "rotate", 1, sphi );
  scale = wf->ReadFloat(wf_section, "scale", scale );

  /*  fill_polygons = wf_read_int(wf_section, "fill_polygons", fill_polygons ); */
/*   show_grid = wf_read_int(wf_section, "show_grid", show_grid ); */
/*   show_alpha = wf_read_int(wf_section, "show_alpha", show_alpha ); */
/*   show_data = wf_read_int(wf_section, "show_data", show_data ); */
/*   show_thumbnail = wf_read_int(wf_section, "show_thumbnail", win ->show_thumbnail ); */

  // gui_load_toggle( win, wf_section, &show_bboxes, "Bboxes", "show_bboxes" );
//   gui_load_toggle( win, wf_section, &show_alpha, "Alpha", "show_alpha" );
//   gui_load_toggle( win, wf_section, &show_grid, "Grid", "show_grid" );
//   gui_load_toggle( win, wf_section, &show_bboxes, "Thumbnail", "show_thumbnail" );
//   gui_load_toggle( win, wf_section, &show_data, "Data", "show_data" );
//   gui_load_toggle( win, wf_section, &follow_selection, "Follow", "show_follow" );
//   gui_load_toggle( win, wf_section, &fill_polygons, "Fill Polygons", "show_fill" );

/*   if( wf_property_exists( wf_section, "show_bboxes" ) ) */
/*     { */
/*       show_bboxes = wf_read_int(wf_section, "show_bboxes", show_bboxes ); */
      
/*       GtkToggleAction* action = gtk_action_group_get_action( action_group, "Bboxes" ); */
/*       gtk_toggle_action_set_active( action, show_bboxes ); */
/*     } */

  int width, height;
  gtk_window_get_size(  GTK_WINDOW(frame), &width, &height );
  width = (int)wf->ReadTupleFloat(wf_section, "size", 0, width );
  height = (int)wf->ReadTupleFloat(wf_section, "size", 1, height );
  gtk_window_resize( GTK_WINDOW(frame), width, height );

  if( wf->PropertyExists( wf_section, "redraw_interval" ) )
    {
      redraw_interval = 
	wf->ReadInt(wf_section, "redraw_interval", redraw_interval );
      
      const stg_msec_t lowbound = 10;
      if( redraw_interval < lowbound )
	{
	  PRINT_WARN2( "redraw_interval of %lu is probably too low "
		       "to be practical. Setting to %lu msec instead.",
		       redraw_interval, lowbound );

	  redraw_interval = lowbound;
	}
      
      // there's no point redrawing faster than the world updates
      //redraw_interval = MAX( redraw_interval, interval_real );

      if( timer_handle > 0 )
	g_source_remove( timer_handle );
      
      timer_handle = 
	g_timeout_add( redraw_interval, (GSourceFunc)timed_redraw, this );
    }
}

void StgWorldGtk::SetTitle( char* txt )
{
  assert(txt);
  assert(frame);
  gtk_window_set_title( GTK_WINDOW(frame), txt );
}

void StgWorldGtk::Save( void )
{
  PRINT_DEBUG1( "%s.Save()", token );
  
  int width, height;
  gtk_window_get_size(  GTK_WINDOW(frame), &width, &height );
  
  wf->WriteTupleFloat( wf_section, "size", 0, width );
  wf->WriteTupleFloat( wf_section, "size", 1, height );

  wf->WriteFloat( wf_section, "scale", scale );

  wf->WriteTupleFloat( wf_section, "center", 0, panx );
  wf->WriteTupleFloat( wf_section, "center", 1, pany );

  wf->WriteTupleFloat( wf_section, "rotate", 0, stheta  );
  wf->WriteTupleFloat( wf_section, "rotate", 1, sphi  );

  wf->WriteInt( wf_section, "fill_polygons", show_fill );
  wf->WriteInt( wf_section, "show_grid", show_grid );
  wf->WriteInt( wf_section, "show_bboxes", show_bboxes );

  // save the state of the property toggles
  for( GList* list = toggle_list; list; list = g_list_next( list ) )
    {
      stg_property_toggle_args_t* tog = 
	(stg_property_toggle_args_t*)list->data;
      assert( tog );
      printf( "toggle item path: %s\n", tog->path );

      wf->WriteInt( wf_section, tog->path, 
		   gtk_toggle_action_get_active(  GTK_TOGGLE_ACTION(tog->action) ));
    }

  StgWorld::Save();
}



gboolean StgWorldGtk::DialogQuit( GtkWindow* parent )
{
  GtkMessageDialog *dlg = (GtkMessageDialog*)
    gtk_message_dialog_new( parent,
			    GTK_DIALOG_DESTROY_WITH_PARENT,
			    GTK_MESSAGE_QUESTION,
			    GTK_BUTTONS_YES_NO,
			    "Are you sure you want to quit Stage?" );
    
  gint result = gtk_dialog_run( GTK_DIALOG(dlg));
  gtk_widget_destroy( (GtkWidget*)dlg);
  
  // return TRUE if use clicked YES
  //return( result == GTK_RESPONSE_YES );

  if( result == GTK_RESPONSE_YES )
    {
      PRINT_MSG( "Confirmed" );
      this->quit = true;
    }
  else
    PRINT_MSG( "Cancelled" );

  return true;
}

void StgWorldGtk::AddModel( StgModel*  mod  )
{
  mod->InitGraphics();
  StgWorld::AddModel( mod );
}


typedef struct 
{
  double x,y;
  StgModel* closest_model;
  stg_meters_t closest_range;
} find_close_t;

void find_close_func( gpointer key,
		      gpointer value,
		      gpointer user_data )
{
  StgModel* mod = (StgModel*)value;
  find_close_t* f = (find_close_t*)user_data;

  if( mod->Parent() == NULL && mod->GuiMask() )
    {
      stg_pose_t pose;
      mod->GetPose( &pose );
      
      double range = hypot( f->y - pose.y, f->x - pose.x );
      
      if( range < f->closest_range )
	{
	  f->closest_range = range;
	  f->closest_model = mod;
	}
    }
}

StgModel* StgWorldGtk::NearestRootModel( double wx, double wy )
{
  find_close_t f;
  f.closest_range = 0.5;
  f.closest_model = NULL;
  f.x = wx;
  f.y = wy;
  
  g_hash_table_foreach( models_by_id, find_close_func, &f );
  
  //printf( "closest model %s\n", f.closest_model ? f.closest_model->token : "none found" );

  return f.closest_model;
}


void StgWorldGtk::PopModelStatus()
{
  guint cid = gtk_statusbar_get_context_id( status_bar, "on_mouse" );
  gtk_statusbar_pop( status_bar, cid ); 
}

void StgWorldGtk::PushModelStatus( StgModel* mod, char* verb )
{
  char txt[256];

  // display the pose
  snprintf(txt, sizeof(txt), "%s %s", 
	   verb, mod->PrintWithPose()); 
  guint cid = gtk_statusbar_get_context_id( status_bar, "on_mouse" );
  gtk_statusbar_pop( status_bar, cid ); 
  gtk_statusbar_push( status_bar, cid, txt ); 
  //printf( "STATUSBAR: %s\n", txt );
}


/// save a frame as a jpeg, with incremental index numbers. if series
/// is greater than 0, its value is incorporated into the filename
void StgWorldGtk::ExportWindow()
{
  char filename[128];

  frame_index++;

  char* suffix;
  switch( frame_format )
    {
    case STG_IMAGE_FORMAT_JPG: suffix = "jpg"; break;
    case STG_IMAGE_FORMAT_PNG: suffix = "png"; break;
    default:
      suffix = ".png";
    }

  if( frame_series > 0 )
    snprintf(filename, sizeof(filename), "stage-%03d-%04d.%s",
	     frame_series, frame_index, suffix);
  else
    snprintf(filename, sizeof(filename), "stage-%03d.%s", frame_index, suffix);

  printf("Stage: saving [%s]\n", filename);
  
  
  // TODO - look into why this doesn't work - it just makes empty
  // bitmaps right now

  GdkWindow* drawable = gdk_gl_window_get_window(gtk_widget_get_gl_window( canvas ));

  GdkPixbuf* buf = gdk_pixbuf_get_from_drawable( NULL, 
						 drawable,
						 NULL,
						 0,0,0,0,
						 200, 200 );//canvas->sizex,
  //canvas->sizey );
  if( buf )
    { 
      switch( frame_format )
	{
	case STG_IMAGE_FORMAT_JPG:
	  gdk_pixbuf_save( buf, filename, "jpeg", NULL,
			   "quality", "100", NULL );
	  break;
	  
	case STG_IMAGE_FORMAT_PNG:
	  gdk_pixbuf_save( buf, filename, "png", NULL, NULL );
	  break;
	  
	default: 
	  puts( "unrecognized image format" );
	  break;
	}

      gdk_pixbuf_unref( buf );
    }
}


void StgWorldGtk::ExportSequenceStart()
{
  PRINT_DEBUG1( "Saving frame sequence with interval %d", 
		frame_interval );
  
  // advance the sequence counter - the first sequence is 1.
  frame_series++;
  frame_index = 0;
  
  frame_callback_tag =
    g_timeout_add( frame_interval,
		   (GSourceFunc)cbExportFrame,
		   this );
}

void StgWorldGtk::ExportSequenceStop()
{
  PRINT_DEBUG( "sequence stop" );
  // stop the frame callback
  g_source_remove(frame_callback_tag); 
}

void StgWorldGtk::RenderClock()
{
  ClockString( clock, 256 );
  gtk_label_set_text( clock_label, clock );
}

// returns 1 if the world should be destroyed
bool StgWorldGtk::Update()
{
  //PRINT_DEBUG( "StgWorldGtk::Update()" );
  
  StgWorld::Update(); // ignore return value
  
  //if( dirty )
  //Draw();
  
  while( gtk_events_pending() )
    gtk_main_iteration(); 
  
  return !( quit || quit_all );
}


bool StgWorldGtk::RealTimeUpdate()
 
{
  //PRINT_DEBUG( "StageWorldGtk::RealTimeUpdate()" );
  
  //StgWorld::Update(); // ignore return value
    
  this->Update();

//   // handle GUI events and possibly sleep until it's time to update  
//   stg_msec_t timenow = 0;
//   while( 1 )    
//     {    
//       while( gtk_events_pending() )
// 	gtk_main_iteration(); 
            

  PauseUntilNextUpdateTime();
  
  return !( quit || quit_all );
}


void StgWorldGtk::Zoom( double multiplier )
{
  scale *= multiplier;
  dirty = true;
}

void StgWorldGtk::SetScale( double scale )
{
  this->scale = scale;
}

void StgWorldGtk::SetWindowSize( unsigned int w, unsigned int h )
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context( canvas );
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable( canvas );

  assert( gdk_gl_drawable_gl_begin (gldrawable, glcontext) );  
  /*** OpenGL BEGIN ***/
  
  //aspect = (float)w/(float)h;
  glViewport (0, 0, (GLfloat)w, (GLfloat)h);
  
  // clear the offscreen buffer - new drawing will accumulate in here
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  dirty = true; // window needs redrawn ASAP

  /*** OpenGL END ***/
  gdk_gl_drawable_gl_end (gldrawable);
}


void StgWorldGtk::Draw()
{
  //PRINT_DEBUG1( "Drawing world %s", token );
  
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable( canvas );
  GdkGLContext *glcontext = gtk_widget_get_gl_context( canvas);
  
  /*** OpenGL BEGIN ***/
  assert( gdk_gl_drawable_gl_begin(gldrawable, glcontext) );

  // clear the offscreen buffer
  //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  double zclip = hypot(width, height) * scale;
  double pixels_width =  canvas->allocation.width;
  double pixels_height = canvas->allocation.height ;

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


  if( follow_selection && selected_models )
    {      
      glTranslatef(  0,0,-zclip/2.0 );

      // meter scale
      glScalef ( scale, scale, scale ); // zoom

      // get the pose of the model at the head of the selected models
      // list
      stg_pose_t gpose;
      ((StgModel*)selected_models->data)->GetGlobalPose( &gpose );

      //glTranslatef( -gpose.x + (pixels_width/2.0)/scale, 
      //	    -gpose.y + (pixels_height/2.0)/scale,
      //	    -zclip/2.0 );      

      // pixel scale  
      glTranslatef(  -gpose.x, 
		     -gpose.y, 
		     0 );

    }
  else
    {
      // pixel scale  
      glTranslatef(  -panx, 
		     -pany, 
		     -zclip / 2.0 );
      
      glRotatef( RTOD(-stheta), 1,0,0);  
      
      //glTranslatef(  -panx * cos(sphi) - pany*sin(sphi), 
      //	 panx*sin(sphi) - pany*cos(sphi), 0 );
      
      glRotatef( RTOD(sphi), 0.0, 0.0, 1.0);   // rotate about z - pitch
      
      // meter scale
      glScalef ( scale, scale, scale ); // zoom
      //glTranslatef( 0,0, -MAX(width,height) );

      // TODO - rotate around the mouse pointer or window center, not the
      //origin  - the commented code is close, but not quite right
      //glRotatef( RTOD(sphi), 0.0, 0.0, 1.0);   // rotate about z - pitch
      //glTranslatef( -click_point.x, -click_point.y, 0 ); // shift back            //printf( "panx %f pany %f scale %.2f stheta %.2f sphi %.2f\n",
      //  panx, pany, scale, stheta, sphi );
    }
  
  // draw the world size rectangle in white
  // draw the floor a little pushed into the distance so it doesn't z-fight with the
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
    ((StgModel*)it->data)->DrawSelected();
  
  colorstack.Pop();

  glLineWidth( 1 );
  
  // draw the models
  //puts( "WORLD DRAW" );
  GList* it;
  for( it=children; it; it=it->next )
    ((StgModel*)it->data)->Draw();

  //for( it=children; it; it=it->next )
  //((StgModel*)it->data)->DrawData();

  // draw anything in the assorted displaylist list
//   for( it = dl_list; it; it=it->next )
//     {
//       int dl = (int)it->data;
//       printf( "Calling dl %d\n", dl );
//       glCallList( dl );
//    }

/*   glNewList( dl_debug, GL_COMPILE ); */
/*   colorstack.Push( 1,0,1 ); */
/*   glRectf( 0,0,1,1 ); */
/*   colorstack.Pop(); */
/*   glEndList(); */
  
//  glCallList( dl_debug );

  //gl_coord_shift( 0,0,-2,0 );

  //if( show_thumbnail ) // if drawing the whole world mini-view
      //draw_thumbnail( world );

  //glEndList();

  /* Swap buffers */
  if (gdk_gl_drawable_is_double_buffered (gldrawable))
    gdk_gl_drawable_swap_buffers (gldrawable);
  else
  {
    PRINT_WARN( "not double buffered" );
    glFlush ();
  }    

  // clear the offscreen buffer - new drawing will accumulate in here
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  if( selected_models )
    PushModelStatus( (StgModel*)selected_models->data, "" );
  else
    PopModelStatus();

  dirty = false; // we don't redraw until someone unsets this
}

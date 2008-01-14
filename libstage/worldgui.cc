/** @defgroup worldgui World with Graphical User Interface

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

#include "stage_internal.hh"

// some utilities

static const char* MITEM_VIEW_DATA =      "View/Data";
static const char* MITEM_VIEW_BLOCKS =    "View/Blocks";
static const char* MITEM_VIEW_GRID =      "View/Grid";
static const char* MITEM_VIEW_OCCUPANCY = "View/Occupancy";
static const char* MITEM_VIEW_QUADTREE =  "View/Tree";
static const char* MITEM_VIEW_FOLLOW =    "View/Follow";
static const char* MITEM_VIEW_CLOCK =     "View/Clock";
static const char* MITEM_VIEW_FOOTPRINTS = "View/Trails/Footprints";
static const char* MITEM_VIEW_TRAILS =     "View/Trails/Blocks";
static const char* MITEM_VIEW_ARROWS =     "View/Trails/Arrows";

// transform the current coordinate frame by the given pose
void Stg::gl_coord_shift( double x, double y, double z, double a  )
{
  glTranslatef( x,y,z );
  glRotatef( rtod(a), 0,0,1 );
}

// transform the current coordinate frame by the given pose
void Stg::gl_pose_shift( stg_pose_t* pose )
{
  gl_coord_shift( pose->x, pose->y, pose->z, pose->a );
}

// TODO - this could be faster, but we don't draw a lot of text
void Stg::gl_draw_string( float x, float y, float z, char *str ) 
{  
  char *c;
  glRasterPos3f(x, y,z);
  for (c=str; *c != '\0'; c++) 
    glutBitmapCharacter( GLUT_BITMAP_HELVETICA_12, *c);
}

/////

void dummy_cb(Fl_Widget*, void* v) 
{
}

void StgWorldGui::SaveCallback( Fl_Widget* wid, StgWorldGui* world )
{
  world->Save();
}

void view_toggle_cb(Fl_Menu_Bar* menubar, StgCanvas* canvas ) 
{
  char picked[128];
  menubar->item_pathname(picked, sizeof(picked)-1);

  //printf("CALLBACK: You picked '%s'\n", picked);

  // this is slow and a little ugly, but it's the least hacky approach I think
       if( strcmp(picked, MITEM_VIEW_DATA ) == 0 ) canvas->InvertView( STG_SHOW_DATA );
  else if( strcmp(picked, MITEM_VIEW_BLOCKS ) == 0 ) canvas->InvertView( STG_SHOW_BLOCKS );
  else if( strcmp(picked, MITEM_VIEW_GRID ) == 0 ) canvas->InvertView( STG_SHOW_GRID );
  else if( strcmp(picked, MITEM_VIEW_FOLLOW ) == 0 ) canvas->InvertView( STG_SHOW_FOLLOW );
  else if( strcmp(picked, MITEM_VIEW_QUADTREE ) == 0 ) canvas->InvertView( STG_SHOW_QUADTREE );
  else if( strcmp(picked, MITEM_VIEW_OCCUPANCY ) == 0 ) canvas->InvertView( STG_SHOW_OCCUPANCY );
  else if( strcmp(picked, MITEM_VIEW_CLOCK ) == 0 ) canvas->InvertView( STG_SHOW_CLOCK );
  else if( strcmp(picked, MITEM_VIEW_FOOTPRINTS ) == 0 ) canvas->InvertView( STG_SHOW_FOOTPRINT );
  else if( strcmp(picked, MITEM_VIEW_ARROWS ) == 0 ) canvas->InvertView( STG_SHOW_ARROWS );
  else if( strcmp(picked, MITEM_VIEW_TRAILS ) == 0 ) canvas->InvertView( STG_SHOW_TRAILS );
  else PRINT_ERR1( "Unrecognized menu item \"%s\" not handled", picked );
  
  //printf( "value: %d\n", item->value() );
}


StgWorldGui::StgWorldGui(int W,int H,const char*L) 
  : Fl_Window(0,0,W,H,L)
{
  graphics = true;

  // build the menus
  mbar = new Fl_Menu_Bar(0,0, W, 30);// 640, 30);
  mbar->textsize(12);
  
  canvas = new StgCanvas( this,0,30,W,H-30 );
  resizable(canvas);
  end();
  
  mbar->add( "File", 0, 0, 0, FL_SUBMENU );
  mbar->add( "File/Save File", FL_CTRL + 's', (Fl_Callback *)SaveCallback, this );
  //mbar->add( "File/Save File &As...", FL_CTRL + FL_SHIFT + 's', (Fl_Callback *)dummy_cb, 0, FL_MENU_DIVIDER );
  mbar->add( "File/Exit", FL_CTRL+'q', (Fl_Callback *)dummy_cb, 0 );
 
  mbar->add( "View", 0, 0, 0, FL_SUBMENU );
  mbar->add( MITEM_VIEW_DATA,      'd', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_DATA ? FL_MENU_VALUE : 0 ));  
  mbar->add( MITEM_VIEW_BLOCKS,    'b', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_BLOCKS ? FL_MENU_VALUE : 0 ));  
  mbar->add( MITEM_VIEW_GRID,      'g', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_GRID ? FL_MENU_VALUE : 0 ));  
  mbar->add( MITEM_VIEW_OCCUPANCY, FL_ALT+'o', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_OCCUPANCY ? FL_MENU_VALUE : 0 ));  
  mbar->add( MITEM_VIEW_QUADTREE,  FL_ALT+'t', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_QUADTREE ? FL_MENU_VALUE : 0 ));  
  mbar->add( MITEM_VIEW_FOLLOW,    'f', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_FOLLOW ? FL_MENU_VALUE : 0 ));  
  mbar->add( MITEM_VIEW_CLOCK,    'c', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_CLOCK ? FL_MENU_VALUE : 0 ));  

  mbar->add( MITEM_VIEW_FOOTPRINTS,  FL_CTRL+'f', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_FOOTPRINT ? FL_MENU_VALUE : 0 ));  
  mbar->add( MITEM_VIEW_ARROWS,    FL_CTRL+'a', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_ARROWS ? FL_MENU_VALUE : 0 ));  
  mbar->add( MITEM_VIEW_TRAILS,    FL_CTRL+'t', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
	     FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_TRAILS ? FL_MENU_VALUE : 0 ));  
  
  mbar->add( "Help", 0, 0, 0, FL_SUBMENU );
  mbar->add( "Help/About Stage...", FL_CTRL + 'f', (Fl_Callback *)dummy_cb );
  mbar->add( "Help/HTML Documentation", FL_CTRL + 'g', (Fl_Callback *)dummy_cb );
  show();
}

StgWorldGui::~StgWorldGui()
{
}

void StgWorldGui::Load( const char* filename )
{
  PRINT_DEBUG1( "%s.Load()", token );

  StgWorld::Load( filename);
  
  wf_section = wf->LookupEntity( "window" );
  if( wf_section < 1) // no section defined
    return;
  
  int width =  (int)wf->ReadTupleFloat(wf_section, "size", 0, w() );
  int height = (int)wf->ReadTupleFloat(wf_section, "size", 1, h() );
  size( width,height ); // resize this window
  
  canvas->panx = wf->ReadTupleFloat(wf_section, "center", 0, canvas->panx );
  canvas->pany = wf->ReadTupleFloat(wf_section, "center", 1, canvas->pany );
  canvas->stheta = wf->ReadTupleFloat( wf_section, "rotate", 0, canvas->stheta );
  canvas->sphi = wf->ReadTupleFloat( wf_section, "rotate", 1, canvas->sphi );
  canvas->scale = wf->ReadFloat(wf_section, "scale", canvas->scale );
  canvas->interval = wf->ReadInt(wf_section, "interval", canvas->interval );

  // set the canvas visibilty flags   
  uint32_t flags = canvas->GetShowFlags();
  uint32_t grid = wf->ReadInt(wf_section, "show_grid", flags & STG_SHOW_GRID ) ? STG_SHOW_GRID : 0;
  uint32_t data = wf->ReadInt(wf_section, "show_data", flags & STG_SHOW_DATA ) ? STG_SHOW_DATA : 0;
  uint32_t follow = wf->ReadInt(wf_section, "show_follow", flags & STG_SHOW_FOLLOW ) ? STG_SHOW_FOLLOW : 0;
  uint32_t blocks = wf->ReadInt(wf_section, "show_blocks", flags & STG_SHOW_BLOCKS ) ? STG_SHOW_BLOCKS : 0;
  uint32_t quadtree = wf->ReadInt(wf_section, "show_tree", flags & STG_SHOW_QUADTREE ) ? STG_SHOW_QUADTREE : 0;
  uint32_t clock = wf->ReadInt(wf_section, "show_clock", flags & STG_SHOW_CLOCK ) ? STG_SHOW_CLOCK : 0;
  uint32_t trails = wf->ReadInt(wf_section, "show_trails", flags & STG_SHOW_TRAILS ) ? STG_SHOW_TRAILS : 0;
  uint32_t arrows = wf->ReadInt(wf_section, "show_arrows", flags & STG_SHOW_ARROWS ) ? STG_SHOW_ARROWS : 0;
  uint32_t footprints = wf->ReadInt(wf_section, "show_footprints", flags & STG_SHOW_FOOTPRINT ) ? STG_SHOW_FOOTPRINT : 0;
  
  canvas->SetShowFlags( grid | data | follow | blocks | quadtree | clock
			| trails | arrows | footprints );  
  canvas->invalidate(); // we probably changed something

  // fix the GUI menu checkboxes to match
  flags = canvas->GetShowFlags();
  
  Fl_Menu_Item* item = NULL;
  
  item = (Fl_Menu_Item*)mbar->find_item( MITEM_VIEW_DATA );
  (flags & STG_SHOW_DATA) ? item->check() : item->clear();

  item = (Fl_Menu_Item*)mbar->find_item( MITEM_VIEW_GRID );
  (flags & STG_SHOW_GRID) ? item->check() : item->clear();

  item = (Fl_Menu_Item*)mbar->find_item( MITEM_VIEW_BLOCKS );
  (flags & STG_SHOW_BLOCKS) ? item->check() : item->clear();

  item = (Fl_Menu_Item*)mbar->find_item( MITEM_VIEW_FOLLOW );
  (flags & STG_SHOW_FOLLOW) ? item->check() : item->clear();

  item = (Fl_Menu_Item*)mbar->find_item( MITEM_VIEW_OCCUPANCY );
  (flags & STG_SHOW_OCCUPANCY) ? item->check() : item->clear();

  item = (Fl_Menu_Item*)mbar->find_item( MITEM_VIEW_QUADTREE );
  (flags & STG_SHOW_QUADTREE) ? item->check() : item->clear();

  // TODO - per model visualizations load
}

void StgWorldGui::Save( void )
{
  PRINT_DEBUG1( "%s.Save()", token );
  
  wf->WriteTupleFloat( wf_section, "size", 0, w() );
  wf->WriteTupleFloat( wf_section, "size", 1, h() );

  wf->WriteFloat( wf_section, "scale", canvas->scale );

  wf->WriteTupleFloat( wf_section, "center", 0, canvas->panx );
  wf->WriteTupleFloat( wf_section, "center", 1, canvas->pany );

  wf->WriteTupleFloat( wf_section, "rotate", 0, canvas->stheta  );
  wf->WriteTupleFloat( wf_section, "rotate", 1, canvas->sphi  );

  uint32_t flags = canvas->GetShowFlags();
  wf->WriteInt( wf_section, "show_blocks", flags & STG_SHOW_BLOCKS );
  wf->WriteInt( wf_section, "show_grid", flags & STG_SHOW_GRID );
  wf->WriteInt( wf_section, "show_follow", flags & STG_SHOW_FOLLOW );
  wf->WriteInt( wf_section, "show_data", flags & STG_SHOW_DATA );
  wf->WriteInt( wf_section, "show_occupancy", flags & STG_SHOW_OCCUPANCY );
  wf->WriteInt( wf_section, "show_tree", flags & STG_SHOW_QUADTREE );
  wf->WriteInt( wf_section, "show_clock", flags & STG_SHOW_CLOCK );
  
  // TODO - per model visualizations save 

  StgWorld::Save();
}

bool StgWorldGui::RealTimeUpdate()
{
  //  puts( "RTU" );
  bool updated = StgWorld::RealTimeUpdate();
  Fl::check(); // may redraw the window
  return updated;
}

bool StgWorldGui::Update()
{
  //  puts( "RTU" );
  bool updated = StgWorld::Update();
  Fl::check(); // may redraw the window
  return updated;
}


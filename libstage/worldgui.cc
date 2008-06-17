/** @defgroup worldgui World with Graphical User Interface

  The Stage window consists of a menu bar, a view of the simulated
  world, and a status bar.

  The world view shows part of the simulated world. You can zoom the
  view in and out, and scroll it to see more of the world. Simulated
  robot devices, obstacles, etc., are rendered as colored polygons. The
  world view can also show visualizations of the data and configuration
  of various sensor and actuator models. The View menu has options to
  control which data and configurations are rendered.

API: Stg::StgWorldGui

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
- title [string]
- the string displayed in the window title bar. Defaults to the worldfile file name.
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
#include "region.hh"

#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_File_Chooser.H>

#include "file_manager.hh"

static const char* MITEM_VIEW_DATA =      "&View/&Data";
static const char* MITEM_VIEW_BLOCKS =    "&View/&Blocks";
static const char* MITEM_VIEW_GRID =      "&View/&Grid";
static const char* MITEM_VIEW_OCCUPANCY = "&View/&Occupancy";
static const char* MITEM_VIEW_QUADTREE =  "&View/&Tree";
static const char* MITEM_VIEW_FOLLOW =    "&View/&Follow";
static const char* MITEM_VIEW_CLOCK =     "&View/&Clock";
static const char* MITEM_VIEW_FOOTPRINTS = "&View/T&rails/&Footprints";
static const char* MITEM_VIEW_BLOCKSRISING =  "&View/T&rails/&Blocks rising";
static const char* MITEM_VIEW_ARROWS =     "&View/T&rails/&Arrows rising";
static const char* MITEM_VIEW_TRAILS =     "&View/&Trail";
static const char* MITEM_VIEW_STATUS =     "&View/&Status";
static const char* MITEM_VIEW_PERSPECTIVE = "&View/Perspective camera";

// this should be set by CMake
#ifndef PACKAGE_STRING
#define PACKAGE_STRING "Stage-3.dev"
#endif




	StgWorldGui::StgWorldGui(int W,int H,const char* L) 
: Fl_Window(0,0,W,H,L)
{
	//size_range( 100,100 ); // set minimum window size

	graphics = true;
	paused = false;

	interval_real = (stg_usec_t)thousand * DEFAULT_INTERVAL_REAL;
	
	for( unsigned int i=0; i<INTERVAL_LOG_LEN; i++ )
		this->interval_log[i] = this->interval_real;

	// build the menus
	mbar = new Fl_Menu_Bar(0,0, W, 30);// 640, 30);
	mbar->textsize(12);

	canvas = new StgCanvas( this,0,30,W,H-30 );
	resizable(canvas);
	end();

	mbar->add( "&File", 0, 0, 0, FL_SUBMENU );
	mbar->add( "File/&Load World...", FL_CTRL + 'l', (Fl_Callback *)LoadCallback, this, FL_MENU_DIVIDER );
	mbar->add( "File/Save World", FL_CTRL + 's', (Fl_Callback *)SaveCallback, this );
	mbar->add( "File/Save World &As...", FL_CTRL + FL_SHIFT + 's', (Fl_Callback *)SaveAsCallback, this, FL_MENU_DIVIDER );
	mbar->add( "File/Exit", FL_CTRL+'q', (Fl_Callback *)QuitCallback, this );

	mbar->add( "&View", 0, 0, 0, FL_SUBMENU );
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
	mbar->add( MITEM_VIEW_PERSPECTIVE,   'r', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
			  FL_MENU_TOGGLE| (canvas->use_perspective_camera ));

	mbar->add( MITEM_VIEW_TRAILS,    't', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
			FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_TRAILS ? FL_MENU_VALUE : 0 ));
	
	mbar->add( MITEM_VIEW_STATUS,    's', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
			  FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_STATUS ? FL_MENU_VALUE : 0 ));

	mbar->add( MITEM_VIEW_FOOTPRINTS,  FL_CTRL+'f', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
			FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_FOOTPRINT ? FL_MENU_VALUE : 0 ));
	mbar->add( MITEM_VIEW_ARROWS,    FL_CTRL+'a', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
			FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_ARROWS ? FL_MENU_VALUE : 0 ));
	mbar->add( MITEM_VIEW_BLOCKSRISING,    FL_CTRL+'t', (Fl_Callback*)view_toggle_cb, (void*)canvas, 
			FL_MENU_TOGGLE| (canvas->showflags & STG_SHOW_TRAILRISE ? FL_MENU_VALUE : 0 ));

	mbar->add( "&Help", 0, 0, 0, FL_SUBMENU );
	mbar->add( "Help/&About Stage...", 0, (Fl_Callback *)About_cb, this );
	//mbar->add( "Help/HTML Documentation", FL_CTRL + 'g', (Fl_Callback *)dummy_cb );

	callback( (Fl_Callback*)WindowCallback, this );
	show();
}

StgWorldGui::~StgWorldGui()
{
	delete mbar;
	delete canvas;
}


void StgWorldGui::ClockString( char* str, size_t maxlen )
{
	const uint32_t usec_per_hour = 360000000;
	const uint32_t usec_per_minute = 60000000;
	const uint32_t usec_per_second = 1000000;
	const uint32_t usec_per_msec = 1000;

	uint32_t hours   = sim_time / usec_per_hour;
	uint32_t minutes = (sim_time % usec_per_hour) / usec_per_minute;
	uint32_t seconds = (sim_time % usec_per_minute) / usec_per_second;
	uint32_t msec    = (sim_time % usec_per_second) / usec_per_msec;

	// find the average length of the last few realtime intervals;
	stg_usec_t average_real_interval = 0;
	for( uint32_t i=0; i<INTERVAL_LOG_LEN; i++ )
		average_real_interval += interval_log[i];
	average_real_interval /= INTERVAL_LOG_LEN;

	double localratio = (double)interval_sim / (double)average_real_interval;

#ifdef DEBUG
	if( hours > 0 )
		snprintf( str, maxlen, "Time: %uh%02um%02u.%03us\t[%.6f]\tsubs: %d  %s",
				hours, minutes, seconds, msec,
				localratio,
				total_subs,
				paused ? "--PAUSED--" : "" );
	else
		snprintf( str, maxlen, "Time: %02um%02u.%03us\t[%.6f]\tsubs: %d  %s",
				minutes, seconds, msec,
				localratio,
				total_subs,
				paused ? "--PAUSED--" : "" );
#else
	if( hours > 0 )
		snprintf( str, maxlen, "%uh%02um%02u.%03us\t[%.2f] %s",
				hours, minutes, seconds, msec,
				localratio,
				paused ? "--PAUSED--" : "" );
	else
		snprintf( str, maxlen, "%02um%02u.%03us\t[%.2f] %s",
				minutes, seconds, msec,
				localratio,
				paused ? "--PAUSED--" : "" );
#endif
}


void StgWorldGui::Load( const char* filename )
{
	PRINT_DEBUG1( "%s.Load()", token );
	
	fileMan.newWorld( filename );

	StgWorld::Load( filename );

// 	wf_section = wf->LookupEntity( "window" );
// 	if( wf_section < 1) // no section defined
// 		return;
	
	int wf_section = 0; // root section

	this->paused = 
	  wf->ReadInt( wf_section, "paused", this->paused );

	this->interval_real = (stg_usec_t)thousand *  
	  wf->ReadInt( wf_section, "interval_real", (int)(this->interval_real/thousand) );

	int width =  (int)wf->ReadTupleFloat(wf_section, "size", 0, w() );
	int height = (int)wf->ReadTupleFloat(wf_section, "size", 1, h() );
	// on OS X this behaves badly - prevents the Window manager resizing
	//larger than this size.
	size( width,height );

	float x = wf->ReadTupleFloat(wf_section, "center", 0, 0 );
	float y = wf->ReadTupleFloat(wf_section, "center", 1, 0 );
	canvas->camera.setPose( x, y );

	canvas->camera.setPitch( wf->ReadTupleFloat( wf_section, "rotate", 0, 0 ) );
	canvas->camera.setYaw( wf->ReadTupleFloat( wf_section, "rotate", 1, 0 ) );
	canvas->camera.setScale( wf->ReadFloat(wf_section, "scale", canvas->camera.getScale() ) );
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
	uint32_t trailsrising = wf->ReadInt(wf_section, "show_trails_rising", flags & STG_SHOW_TRAILRISE ) ? STG_SHOW_TRAILRISE : 0;
	uint32_t arrows = wf->ReadInt(wf_section, "show_arrows", flags & STG_SHOW_ARROWS ) ? STG_SHOW_ARROWS : 0;
	uint32_t footprints = wf->ReadInt(wf_section, "show_footprints", flags & STG_SHOW_FOOTPRINT ) ? STG_SHOW_FOOTPRINT : 0;
	uint32_t status = wf->ReadInt(wf_section, "show_status", flags & STG_SHOW_STATUS ) ? STG_SHOW_STATUS : 0;

	canvas->SetShowFlags( grid | data | follow | blocks | quadtree | clock
			| trails | arrows | footprints | trailsrising | status );
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

	item = (Fl_Menu_Item*)mbar->find_item( MITEM_VIEW_STATUS );
	(flags & STG_SHOW_STATUS) ? item->check() : item->clear();

	
	// TODO - per model visualizations load
}

void StgWorldGui::UnLoad() 
{
	StgWorld::UnLoad();
//	canvas->camera.setPose( 0, 0 );
}

void StgWorldGui::LoadCallback( Fl_Widget* wid, StgWorldGui* world )
{
	const char* filename;
	const char* worldsPath;
	//bool success;
	const char* pattern = "World Files (*.world)";
	
	worldsPath = world->fileMan.worldsRoot().c_str();
	Fl_File_Chooser fc( worldsPath, pattern, Fl_File_Chooser::CREATE, "Load World File..." );
	fc.ok_label( "Load" );
	
	fc.show();
	while (fc.shown())
		Fl::wait();
	
	filename = fc.value();
	
	if (filename != NULL) { // chose something
		if ( world->fileMan.readable( filename ) ) {
			// file is readable, clear and load

			// if (initialized) {
			world->Stop();
			world->UnLoad();
			// }
			
			// todo: make sure loading is successful
			world->Load( filename );
			world->Start(); // if (stopped)
		}
		else {
			fl_alert( "Unable to read selected world file." );
		}
		

	}
}

void StgWorldGui::SaveCallback( Fl_Widget* wid, StgWorldGui* world )
{
	// save to current file
	bool success =  world->Save( NULL );
	if ( !success ) {
		fl_alert( "Error saving world file." );
	}
}

void StgWorldGui::SaveAsCallback( Fl_Widget* wid, StgWorldGui* world )
{
	world->SaveAsDialog();
}

bool StgWorldGui::SaveAsDialog()
{
	const char* newFilename;
	bool success = false;
	const char* pattern = "World Files (*.world)";

	Fl_File_Chooser fc( wf->filename, pattern, Fl_File_Chooser::CREATE, "Save File As..." );
	fc.ok_label( "Save" );

	fc.show();
	while (fc.shown())
		Fl::wait();

	newFilename = fc.value();

	if (newFilename != NULL) {
		// todo: make sure file ends in .world
		success = Save( newFilename );
		if ( !success ) {
			fl_alert( "Error saving world file." );
		}
	}

	return success;
}

void StgWorldGui::QuitCallback( Fl_Widget* wid, StgWorldGui* world ) 
{
	bool done = world->CloseWindowQuery();
	if (done) {
		exit(0);
	}
}

bool StgWorldGui::CloseWindowQuery()
{
	int choice;
	
	if ( wf ) {
		// worldfile loaded, ask to save
		choice = fl_choice("Do you want to save?",
						   "&Cancel", // ->0: defaults to ESC
						   "&Yes", // ->1
						   "&No" // ->2
						   );
		
		switch (choice) {
			case 1: // Yes
				if ( SaveAsDialog() ) {
					return true;
				}
				else {
					return false;
				}
			case 2: // No
				return true;
		}
		
		// Cancel
		return false;
	}
	else {
		// nothing is loaded, just quit
		return true;
	}
}

void StgWorldGui::WindowCallback( Fl_Widget* wid, StgWorldGui* world )
{
	switch ( Fl::event() ) {
		case FL_SHORTCUT:
			if ( Fl::event_key() == FL_Escape )
				return;
		case FL_CLOSE: // clicked close button
			bool done = world->CloseWindowQuery();
			if ( !done )
				return;
	}

	exit(0);
}

void StgWorldGui::view_toggle_cb( Fl_Menu_Bar* menubar, StgCanvas* canvas ) 
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
	else if( strcmp(picked, MITEM_VIEW_BLOCKSRISING ) == 0 ) canvas->InvertView( STG_SHOW_TRAILRISE );
	else if( strcmp(picked, MITEM_VIEW_STATUS ) == 0 ) canvas->InvertView( STG_SHOW_STATUS );
	else if( strcmp(picked, MITEM_VIEW_PERSPECTIVE ) == 0 ) { canvas->use_perspective_camera = ! canvas->use_perspective_camera; canvas->invalidate(); }
	else PRINT_ERR1( "Unrecognized menu item \"%s\" not handled", picked );

	//printf( "value: %d\n", item->value() );
}

void StgWorldGui::About_cb( Fl_Widget*, StgWorldGui* world ) 
{
	fl_register_images();
	
	const int Width = 400;
	const int Height = 220;
	const int Spc = 10;
	const int ButtonH = 25;
	const int ButtonW = 60;
	const int pngH = 82;
	//const int pngW = 264;
	
	Fl_Window win( Width, Height ); // make a window

	Fl_Box box( Spc, Spc, 
			   Width-2*Spc, pngH ); // widget that will contain image

	
	std::string fullpath;
	fullpath = world->fileMan.fullPath( "stagelogo.png" );
	Fl_PNG_Image png( fullpath.c_str() ); // load image into ram
	box.image(png); // attach image to box
	
	Fl_Text_Display* textDisplay;
	textDisplay = new Fl_Text_Display( Spc, pngH+2*Spc,
						  Width-2*Spc, Height-pngH-ButtonH-4*Spc );
	textDisplay->box( FL_NO_BOX );
	textDisplay->color(win.color());
	
	const char* AboutText = 
		"\n" 
		"Part of the Player Project\n"
		"http://playerstage.sourceforge.net\n"
		"Copyright 2000-2008 Richard Vaughan and contributors";
	
	Fl_Text_Buffer* tbuf = new Fl_Text_Buffer;
	tbuf->text( PACKAGE_STRING );
	tbuf->append( AboutText );
	textDisplay->buffer( tbuf );
	
	Fl_Return_Button button( (Width - ButtonW)/2, Height-Spc-ButtonH,
					  ButtonW, ButtonH,
					  "&OK" );
	button.callback( (Fl_Callback*)HelpAboutCallback );
	
	win.show();
	while (win.shown())
		Fl::wait();
	
	delete textDisplay;
	delete tbuf;
}

void StgWorldGui::HelpAboutCallback( Fl_Widget* wid ) {
	wid->window()->hide();
}

bool StgWorldGui::Save( const char* filename )
{
	PRINT_DEBUG1( "%s.Save()", token );

	int wf_section = 0;

	wf->WriteTupleFloat( wf_section, "size", 0, w() );
	wf->WriteTupleFloat( wf_section, "size", 1, h() );

	wf->WriteFloat( wf_section, "scale", canvas->camera.getScale() );
	
	wf->WriteTupleFloat( wf_section, "center", 0, canvas->camera.getX() );
	wf->WriteTupleFloat( wf_section, "center", 1, canvas->camera.getY() );
	
	wf->WriteTupleFloat( wf_section, "rotate", 0, canvas->camera.getPitch()  );
	wf->WriteTupleFloat( wf_section, "rotate", 1, canvas->camera.getYaw()  );

	uint32_t flags = canvas->GetShowFlags();
	wf->WriteInt( wf_section, "show_blocks", flags & STG_SHOW_BLOCKS );
	wf->WriteInt( wf_section, "show_grid", flags & STG_SHOW_GRID );
	wf->WriteInt( wf_section, "show_follow", flags & STG_SHOW_FOLLOW );
	wf->WriteInt( wf_section, "show_data", flags & STG_SHOW_DATA );
	wf->WriteInt( wf_section, "show_occupancy", flags & STG_SHOW_OCCUPANCY );
	wf->WriteInt( wf_section, "show_tree", flags & STG_SHOW_QUADTREE );
	wf->WriteInt( wf_section, "show_clock", flags & STG_SHOW_CLOCK );

	// TODO - per model visualizations save 

	return StgWorld::Save( filename );
}


bool StgWorldGui::Update()
{
  if( real_time_of_last_update == 0 )
	 real_time_of_last_update = RealTimeNow();

  bool val = paused ? true : StgWorld::Update();
  

  stg_usec_t interval;
  stg_usec_t timenow;
  
  do { // we loop over updating the GUI, sleeping if there's any spare
		 // time
	 Fl::check();

	 timenow = RealTimeNow();
	 interval = timenow - real_time_of_last_update; // guaranteed to be >= 0
	 
	 double sleeptime = (double)interval_real - (double)interval;
	 
	 if( sleeptime > 0  ) 
	   usleep( (stg_usec_t)MIN(sleeptime,100000) ); // check the GUI at 10Hz min
	 
  } while( interval < interval_real );
  
  
  real_time_of_last_update = timenow;
  
  
  interval_log[updates%INTERVAL_LOG_LEN] = interval_real;//timenow - real_time_now;

  return val;
}

void StgWorldGui::DrawTree( bool drawall )
{  
	g_hash_table_foreach( superregions, (GHFunc)SuperRegion::Draw_cb, NULL );
}

void StgWorldGui::DrawFloor()
{
	PushColor( 1,1,1,1 );
	g_hash_table_foreach( superregions, (GHFunc)SuperRegion::Floor_cb, NULL );
	PopColor();
}

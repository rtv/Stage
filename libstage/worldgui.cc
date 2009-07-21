/* worldgui.cc
    Implements a subclass of World that has an FLTK and OpenGL GUI
    Authors: Richard Vaughan (vaughan@sfu.ca)
             Alex Couture-Beil (asc17@sfu.ca)
             Jeremy Asher (jra11@sfu.ca)
    SVN: $Id$
*/

/** @defgroup worldgui World with Graphical User Interface

The Stage window consists of a menu bar and a view of the simulated
world.

The world view shows part of the simulated world. You can zoom the
view in and out, and scroll it to see more of the world. Simulated
robot devices, obstacles, etc., are rendered as colored polygons. The
world view can also show visualizations of the data and configuration
of various sensor and actuator models. The View menu has options to
control which data and configurations are rendered.

API: Stg::WorldGui

<h2>Worldfile Properties</h2>

@par Summary and default values

@verbatim
window
(
  size [ 400 300 ]
  
  # camera options
  center [ 0 0 ]
  rotate [ 0 0 ]
  scale 1.0

  # perspective camera options
  pcam_loc [ 0 -4 2 ]
  pcam_angle [ 70 0 ]

  # GUI options
  show_data 0
  show_flags 1
  show_blocks 1
  show_clock 1
  show_footprints 0
  show_grid 1
  show_trailarrows 0
  show_trailrise 0
  show_trailfast 0
  show_occupancy 0
  show_tree 0
  pcam_on 0
  screenshots 0
)
@endverbatim

@par Details
- size [ <width:int> <height:int> ]\n
size of the window in pixels
- center [ <x:float> <y:float> ]\n
location of the center of the window in world coordinates (meters)
- rotate [ <pitch:float> <yaw:float> ]\n
angle relative to straight up, angle of rotation (degrees)
- scale <float>\n
ratio of world to pixel coordinates (window zoom)
- pcam_loc [ <x:int> <y:int> <z:int> ]\n
location of the perspective camera (meters)
- pcam_angle [ <pitch:float> <yaw:float> ]\n
verticle and horizontal angle of the perspective camera
- pcam_on <int>\n
whether to start with the perspective camera enabled (0/1)


<h2>Using the Stage window</h2>


<h3>Scrolling the view</h3>
<p>Left-click and drag on the background to move your view of the world.

<h3>Zooming the view</h3>
<p>Scroll the mouse wheel to zoom in or out on the mouse cursor.

<h3>Saving the world</h3>
<P>You can save the current pose of everything in the world, using the
File/Save menu item. <b>Warning: the saved poses overwrite the currentworld file.</b> Make a copy of your world file before saving if you
want to keep the old poses.  Alternatively the File/Save As menu item
can be used to save to a new world file.

<h3>Pausing and resuming the clock</h3> <p>The simulation can be
paused or resumed by pressing the 'p' key. Run one simulation step at
a time by pressing the '.' (period) key. Hold down the '.' key to step
repeatedly. Stepping leaves the simulation paused, so press 'p' to
resume running. The initial paused/unpaused state can be set in the
worldfile using the "paused" property. 

<h3>Selecting models</h3> <p>Models can be selected by clicking on
them with the left mouse button.  It is possible to select multiple
models by holding the shift key and clicking on multiple models.
Selected models can be moved by dragging or rotated by holding the
right mouse button and moving the mouse. Selections can be cleared by
clicking on an empty location in the world.  After clearing the
selection, the last single model selected will be saved as the target
for several view options described below which affect a particular
model.

<h3>View options</h3>
<p>The View menu provides access to a number of features affecting how
the world is rendered.  To the right of each option there is usually
a keyboard hotkey which can be pressed to quickly toggle the relevant
option.

<p>Sensor data visualizations can be toggled by the "Data" option
(shortcut 'd').  The filter data option (shortcut shift-'d') opens a
dialog which enables turning on and off visualizations of particular
sensors.  The "Visualize All" option in the dialog toggles whether
sensor visualizations are enabled for all models or only the currently
selected ones.

<p>The "Follow" option keeps the view centered on the last selected model.

<p>The "Perspective camera" option switches from orthogonal viewing to perspective viewing. 

<h3>Saving a screenshot</h3>
<p> To save a sequence of screenshots of the world, select the "Save
screenshots" option from the view menu to start recording images and
then select the option from the menu again to stop.

*/

#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_File_Chooser.H>

#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm> // for std::sort

#include "canvas.hh"
#include "region.hh"
#include "worldfile.hh"
#include "file_manager.hh"
#include "options_dlg.hh"
#include "config.h" // build options from CMake

using namespace Stg;

static const char* AboutText = 
	"\n" 
	"Part of the Player Project\n"
	"http://playerstage.org\n"
	"\n"
	"Copyright 2000-2008 Richard Vaughan,\n"
	"Brian Gerkey, Andrew Howard, Reed Hedges, \n"
	"Toby Collett, Alex Couture-Beil, Jeremy Asher \n"
	"and contributors\n" 
	"\n"
	"Distributed under the terms of the \n"
	"GNU General Public License v2";

static const char* MoreHelpText = 
  "http://playerstage.org\n"
  "\n"
  "has these resources to help you:\n"
  "\n"
  "\t* A user manual including API documentation\n"
  "\t* A bug and feature request tracking system\n"
  "\t* Mailing lists for users and developers\n"
  "\t* A Wiki"
  "\n\n"
  "The user manual is included with the Stage source code but\n"
  "is not built by default. To build the manual, run \"make\"\n"
  "in the directory \"docsrc\" to produce \"docsrc/stage/index.html\" .\n"
  "(requires Doxygen and supporting programs to be installed first).\n";
 
WorldGui::WorldGui(int W,int H,const char* L) : 
  Fl_Window(W,H,L ),
  canvas( new Canvas( this,0,30,W,H-30 ) ),
  drawOptions(),
  fileMan( new FileManager() ),
  interval_log(),
  interval_real( (stg_usec_t)1e5 ),
  mbar( new Fl_Menu_Bar(0,0, W, 30)),
  oDlg( NULL ),
  pause_time( false ),	
  real_time_of_last_update( RealTimeNow() )
{
  for( unsigned int i=0; i<INTERVAL_LOG_LEN; i++ )
	 interval_log[i] = interval_real;
  
  Fl::scheme( "gtk+" );
  resizable(canvas);
  
  end();
  
  label( PROJECT );
  
  // make this menu's shortcuts work whoever has focus
  mbar->global();
  mbar->textsize(12);
  
  mbar->add( "&File", 0, 0, 0, FL_SUBMENU );
  mbar->add( "File/&Load World...", FL_CTRL + 'l', fileLoadCb, this, FL_MENU_DIVIDER );
  mbar->add( "File/&Save World", FL_CTRL + 's', fileSaveCb, this );
  mbar->add( "File/Save World &As...", FL_CTRL + FL_SHIFT + 's', WorldGui::fileSaveAsCb, this, FL_MENU_DIVIDER );
  
  //mbar->add( "File/Screenshots", 0,0,0, FL_SUBMENU );
  //mbar->add( "File/Screenshots/Save Frames, fileScreenshotSaveCb, this,FL_MENU_TOGGLE );
  
  mbar->add( "File/E&xit", FL_CTRL+'q', WorldGui::fileExitCb, this );
  
  mbar->add( "&View", 0, 0, 0, FL_SUBMENU );

  mbar->add( "View/Reset", ' ', (Fl_Callback*) WorldGui::resetViewCb, this );

  mbar->add( "View/Filter data...", FL_SHIFT + 'd', (Fl_Callback*)WorldGui::viewOptionsCb, this );
  canvas->createMenuItems( mbar, "View" );

  mbar->add( "Run", 0,0,0, FL_SUBMENU );
  mbar->add( "Run/Pause", 'p', (Fl_Callback*) WorldGui::pauseCb, this );
  mbar->add( "Run/One step", '.', (Fl_Callback*) WorldGui::onceCb, this, FL_MENU_DIVIDER );
  mbar->add( "Run/Faster", ']', (Fl_Callback*) WorldGui::fasterCb, this );
  mbar->add( "Run/Slower", '[', (Fl_Callback*) WorldGui::slowerCb, this, FL_MENU_DIVIDER  );
  mbar->add( "Run/Realtime", '{', (Fl_Callback*) WorldGui::realtimeCb, this );
  mbar->add( "Run/Fast", '}', (Fl_Callback*) WorldGui::fasttimeCb, this );
  
  mbar->add( "&Help", 0, 0, 0, FL_SUBMENU );
  mbar->add( "Help/Getting help...", 0,  (Fl_Callback*)WorldGui::moreHelptCb, this, FL_MENU_DIVIDER );
  mbar->add( "Help/&About Stage...", 0, (Fl_Callback*) WorldGui::helpAboutCb, this );
  
  callback( WorldGui::windowCb, this );	 

	show();	
}

WorldGui::~WorldGui()
{
  delete mbar;
  if ( oDlg )
    delete oDlg;
  delete canvas;
}


void WorldGui::Show()
{
  show(); // fltk
}

void WorldGui::Load( const char* filename )
{
  PRINT_DEBUG1( "%s.Load()", token );
	
  // needs to happen before StgWorld load, or we segfault with GL calls on some graphics cards
  Fl::check();

  fileMan->newWorld( filename );

  World::Load( filename );
	
  int world_section = 0; // use the top-level section for some parms
  // that traditionally live there

  this->interval_real = (stg_usec_t)thousand *  
    wf->ReadInt( world_section, "interval_real", (int)(this->interval_real/thousand) );

  // this is a world property that's only load()ed in worldgui
  this->paused = 
    wf->ReadInt( world_section, "paused", this->paused );

  // use the window section for the rest
  int window_section = wf->LookupEntity( "window" );

  if( window_section < 1) // no section defined
    return;
	

  int width =  (int)wf->ReadTupleFloat(window_section, "size", 0, w() );
  int height = (int)wf->ReadTupleFloat(window_section, "size", 1, h() );
  size( width,height );
  size_range( 100, 100 ); // set min size to 100/100, max size to screen size

  // configure the canvas
  canvas->Load(  wf, window_section );
	
  std::string title = PROJECT;
  if ( wf->filename ) {
    // improve the title bar to say "Stage: <worldfile name>"
    title += ": ";		
    title += wf->filename;
  }
  label( title.c_str() );
	
  show();
}

void WorldGui::UnLoad() 
{
  World::UnLoad();
  //	canvas->camera.setPose( 0, 0 );
}

bool WorldGui::Save( const char* filename )
{
  PRINT_DEBUG1( "%s.Save()", token );
	
  // use the window section for the rest
  int window_section = wf->LookupEntity( "window" );
	
  if( window_section > 0 ) // section defined
    {
      wf->WriteTupleFloat( window_section, "size", 0, w() );
      wf->WriteTupleFloat( window_section, "size", 1, h() );
	    
      canvas->Save( wf, window_section );
	    
      // TODO - per model visualizations save 
    }
	
	World::Save( filename );
	
  // TODO - error checking
  return true;
}

bool WorldGui::Update()
{
  //pause the simulation if quit time is set
  if( PastQuitTime() && pause_time == false ) {
	  TogglePause();
	  pause_time = true;
  }
  
  // if we're paused and counting down, we need to redraw the window
  // because it's not drawn on a timer when paused.
  bool need_redraw = paused && (steps > 0);
  
  bool val = World::Update(); 
  
  if( need_redraw )
	 canvas->redraw();
 	 
  stg_usec_t interval;
  stg_usec_t timenow;
  
  do { // we loop over updating the GUI, sleeping if there's any spare
    // time
    Fl::check();

    timenow = RealTimeNow();
	 
	 // do
	 {
		interval = timenow - real_time_of_last_update; // guaranteed to be >= 0
		
		double sleeptime = (double)interval_real - (double)interval;
		
		if( paused ) sleeptime = 50000; // spare the CPU if we're paused
		
		// printf( "real %.2f interval %.2f sleeptime %.2f\n", 
		//	 (double)interval_real,
		//	 (double)interval,
		//	 sleeptime );
	  
		if( (sleeptime > 0) || paused ) 
		  usleep( (stg_usec_t)MIN(sleeptime,20000) ); // check the GUI at 10Hz min
	 }
  } while( interval < interval_real );
  
  interval_log[updates%INTERVAL_LOG_LEN] =  timenow - real_time_of_last_update;

  real_time_of_last_update = timenow;
  
  //puts( "FINSHED UPDATE" );

  return val;
}

std::string WorldGui::ClockString()
{
  std::string str = World::ClockString();
  
  // find the average length of the last few realtime intervals;
  stg_usec_t average_real_interval = 0;
  for( uint32_t i=0; i<INTERVAL_LOG_LEN; i++ )
    average_real_interval += interval_log[i];
  average_real_interval /= INTERVAL_LOG_LEN;
  
  double localratio = (double)interval_sim / (double)average_real_interval;
  
  char buf[32];
  snprintf( buf, 32, " [%.2f]", localratio );
  str += buf;
  
  if( paused == true )
	 str += " [ PAUSED ]";
  
  return str;
}


void WorldGui::AddModel( Model*  mod  )
{
  if( mod->parent == NULL )
	 canvas->AddModel( mod );
  
  World::AddModel( mod );
}


void WorldGui::RemoveChild( Model* mod )
{
  canvas->RemoveModel( mod );

  World::RemoveChild( mod );
}


std::string WorldGui::EnergyString()
{	
  char str[512];
  
  snprintf( str, 255, "Energy\n  stored:   %.0f / %.0f KJ\n  input:    %.0f KJ\n  output:   %.0f KJ at %.2f KW\n",
				PowerPack::global_stored / 1e3,
				PowerPack::global_capacity /1e3,
				PowerPack::global_input / 1e3,
				PowerPack::global_dissipated / 1e3,
				(PowerPack::global_dissipated / (sim_time / 1e6)) / 1e3 );
  
  std::string s( str );
  return s;
}

void WorldGui::DrawTree( bool drawall )
{  
  //g_hash_table_foreach( superregions, (GHFunc)Draw_cb, (void*)drawall );
  
  for( std::map<stg_point_int_t,SuperRegion*>::iterator it = superregions.begin();
		 it != superregions.end();
		 it++ )
	 (*it).second->Draw( drawall );
}

void WorldGui::DrawFloor()
{
  PushColor( 1,1,1,1 );
  //g_hash_table_foreach( superregions, (GHFunc)Floor_cb, NULL );

  for( std::map<stg_point_int_t,SuperRegion*>::iterator it = superregions.begin();
		 it != superregions.end();
		 it++ )
	 (*it).second->Floor();

  PopColor();
}

void WorldGui::windowCb( Fl_Widget* w, void* p )
{
  WorldGui* worldGui = static_cast<WorldGui*>( p );

  switch ( Fl::event() ) {
  case FL_SHORTCUT:
    if ( Fl::event_key() == FL_Escape )
      return;
  case FL_CLOSE: // clicked close button
    bool done = worldGui->closeWindowQuery();
    if ( !done )
      return;
  }

  puts( "User closed window" );
  exit(0);
}

void WorldGui::fileLoadCb( Fl_Widget* w, void* p )
{
  WorldGui* worldGui = static_cast<WorldGui*>( p );

  const char* filename;
  //bool success;
  const char* pattern = "World Files (*.world)";
	
	std::string worldsPath = worldGui->fileMan->worldsRoot();
	worldsPath.append( "/" );
  Fl_File_Chooser fc( worldsPath.c_str(), pattern, Fl_File_Chooser::CREATE, "Load World File..." );
  fc.ok_label( "Load" );
	
  fc.show();
  while (fc.shown())
    Fl::wait();
	
  filename = fc.value();
	
  if (filename != NULL) { // chose something
    if ( FileManager::readable( filename ) ) {
      // file is readable, clear and load

      // if (initialized) {
      worldGui->Stop();
      worldGui->UnLoad();
      // }
			
      // todo: make sure loading is successful
      worldGui->Load( filename );
      worldGui->Start(); // if (stopped)
    }
    else {
      fl_alert( "Unable to read selected world file." );
    }
		

  }
}

void WorldGui::fileSaveCb( Fl_Widget* w, void* p )
{
  WorldGui* worldGui = static_cast<WorldGui*>( p );

  // save to current file
  bool success =  worldGui->Save( NULL );
  if ( !success ) {
    fl_alert( "Error saving world file." );
  }
}

void WorldGui::fileSaveAsCb( Fl_Widget* w, void* p )
{
  WorldGui* worldGui = static_cast<WorldGui*>( p );

  worldGui->saveAsDialog();
}

void WorldGui::fileExitCb( Fl_Widget* w, void* p ) 
{
  WorldGui* worldGui = static_cast<WorldGui*>( p );

  bool done = worldGui->closeWindowQuery();
  if (done) {
	 puts( "User exited via menu" );
    exit(0);
  }
}

// static void append_option( char* name, Option* opt, std::vector<Option*>* optv )
// {
//   //printf( "adding option %s @ %p\n", name, opt );
//   optv->push_back( opt );
// }

// static bool sort_option_pointer( const Option* a, 
// 											const Option* b ) const
// {
//   // Option class overloads operator<. Nasty nasty C++ makes code less
//   // readable IMHO.
//   return (*a) < (*b);
// }

void WorldGui::resetViewCb( Fl_Widget* w, WorldGui* worldGui )
{
  worldGui->canvas->current_camera->reset();
  
  if( Fl::event_state( FL_CTRL ) ) 
	 {
		worldGui->canvas->resetCamera();
	 }
  worldGui->canvas->redraw();
}

void WorldGui::slowerCb( Fl_Widget* w, WorldGui* wg )
{
  if( wg->interval_real == 0 )
	 wg->interval_real = 10;
  else
	 {
		wg->interval_real *= 1.2;
	 }
}

void WorldGui::fasterCb( Fl_Widget* w, WorldGui* wg )
{
  if( wg->interval_real == 0 )
	 putchar( 7 ); // bell!
  else
	 {
		wg->interval_real *= 0.8;
		if( wg->interval_real < 10 )
		  wg->interval_real = 0;
	 }
}

void WorldGui::realtimeCb( Fl_Widget* w, WorldGui* wg )
{
  wg->interval_real = wg->interval_sim;
}

void WorldGui::fasttimeCb( Fl_Widget* w, WorldGui* wg )
{
  wg->interval_real = 0;
}

void WorldGui::Start()
{
  World::Start();
  
  // start the timer that causes regular redraws
  Fl::add_timeout( ((double)canvas->interval/1000), 
						 (Fl_Timeout_Handler)Canvas::TimerCallback, 
						 canvas );
}

void WorldGui::Stop()
{
  World::Stop();
  
  // remove the redraw timeout
  Fl::remove_timeout( (Fl_Timeout_Handler)Canvas::TimerCallback );	

  // drawn 'cos we cancelled the timeout
  canvas->redraw(); // in case something happened that will never be
}  

void WorldGui::pauseCb( Fl_Widget* w, WorldGui* worldGui )
{
  worldGui->TogglePause();
}

void WorldGui::onceCb( Fl_Widget* w, WorldGui* worldGui )
{
  worldGui->paused = true;
  worldGui->steps = 1; // number of steps to run  
}

void WorldGui::viewOptionsCb( OptionsDlg* oDlg, WorldGui* worldGui ) 
{
  // the options dialog expects a std::vector of options (annoyingly)
  // std::vector<Option*> optvec;
  // adds each option to the vector
  //g_hash_table_foreach( worldGui->option_table, 
  //						(GHFunc)append_option, 
  //						(void*)&optvec );  
  
  // sort the vector by option label alphabetically
  //std::sort();// worldGui->option_table.begin(), worldGui->option_table.end() );//, sort_option_pointer );
  //std::sort();// worldGui->option_table.begin(), worldGui->option_table.end() );//, sort_option_pointer );

  if ( !worldGui->oDlg ) 
	 {
		int x = worldGui->w()+worldGui->x() + 10;
		int y = worldGui->y();
		OptionsDlg* oDlg = new OptionsDlg( x,y, 180,250 );
		oDlg->callback( (Fl_Callback*)optionsDlgCb, worldGui );
		
		oDlg->setOptions( worldGui->option_table );
		oDlg->showAllOpt( &worldGui->canvas->visualizeAll );
		worldGui->oDlg = oDlg;
		oDlg->show();
	 }
  else 
	 {
		worldGui->oDlg->hide();
		delete worldGui->oDlg;
		worldGui->oDlg = NULL;
	 }
 
}

void WorldGui::optionsDlgCb( OptionsDlg* oDlg, WorldGui* worldGui ) 
{
  // get event from dialog
  OptionsDlg::event_t event;
  event = oDlg->event();
	
  // Check FLTK events first
  switch ( Fl::event() ) {
  case FL_SHORTCUT:
    if ( Fl::event_key() != FL_Escape ) 
      break; //return
    // otherwise, ESC pressed-> do as below
  case FL_CLOSE: // clicked close button
    // override event to close
    event = OptionsDlg::CLOSE;
    break;
  }
	
  switch ( event ) {
  case OptionsDlg::CHANGE: 
    {
      //Option* o = oDlg->changed();
      //printf( "\"%s\" changed to %d!\n", o->name().c_str(), o->val() );			
      break;
    }			
  case OptionsDlg::CLOSE:
    // invalidate the oDlg pointer from the WorldGui
    //   instance before the dialog is destroyed
    worldGui->oDlg = NULL; 
    oDlg->hide();
    Fl::delete_widget( oDlg );
    return;	
  case OptionsDlg::NO_EVENT:
  case OptionsDlg::CHANGE_ALL:
    break;
  }
}

void aboutOKBtnCb( Fl_Widget* w, void* p ) {
  Fl_Return_Button* btn;
  btn = static_cast<Fl_Return_Button*>( w );

  btn->window()->do_callback();
}

void aboutCloseCb( Fl_Widget* w, void* p ) {
  Fl_Window* win;
  win = static_cast<Fl_Window*>( w );
  Fl_Text_Display* textDisplay;
  textDisplay = static_cast<Fl_Text_Display*>( p );
	
  Fl_Text_Buffer* tbuf = textDisplay->buffer();
  textDisplay->buffer( NULL );
  delete tbuf;
  Fl::delete_widget( win );
}

void WorldGui::helpAboutCb( Fl_Widget* w, void* p ) 
{
  fl_register_images();
	
  const int Width = 420;
  const int Height = 330;
  const int Spc = 10;
  const int ButtonH = 25;
  const int ButtonW = 60;
  const int pngH = 82;
  //const int pngW = 264;
	
  Fl_Window* win = new Fl_Window( Width, Height ); // make a window

  Fl_Box* box = new Fl_Box( Spc, Spc, 
			    Width-2*Spc, pngH ); // widget that will contain image
	
  std::string fullpath;
  fullpath = FileManager::findFile( "assets/stagelogo.png" );
  Fl_PNG_Image* png = new Fl_PNG_Image( fullpath.c_str() ); // load image into ram
  box->image( png ); // attach image to box
	
  Fl_Text_Display* textDisplay;
  textDisplay = new Fl_Text_Display( Spc, pngH+2*Spc,
				     Width-2*Spc, Height-pngH-ButtonH-4*Spc );
  textDisplay->box( FL_NO_BOX );
  textDisplay->color( win->color() );
  win->callback( aboutCloseCb, textDisplay );
		
  Fl_Text_Buffer* tbuf = new Fl_Text_Buffer;
  tbuf->text( PROJECT );
  tbuf->append( "-" );
  tbuf->append( VERSION );
  tbuf->append( AboutText );
  //textDisplay->wrap_mode( true, 50 );
  textDisplay->buffer( tbuf );
	
  Fl_Return_Button* button;
  button = new Fl_Return_Button( (Width - ButtonW)/2, Height-Spc-ButtonH,
				 ButtonW, ButtonH,
				 "&OK" );
  button->callback( aboutOKBtnCb );
	
  win->show();
}

void WorldGui::moreHelptCb( Fl_Widget* w, WorldGui* wg ) 
{
  const int Width =  500;
  const int Height = 250;
  const int Spc = 10;
	
  Fl_Window* win = new Fl_Window( Width, Height ); // make a window
	  win->label( "Getting help with Stage" );

  Fl_Text_Display* textDisplay;
  textDisplay = new Fl_Text_Display( Spc, Spc,
												 Width-2*Spc, Height-2*Spc );

  win->resizable( textDisplay );
  textDisplay->box( FL_NO_BOX );
  textDisplay->color( win->color() );
		
  Fl_Text_Buffer* tbuf = new Fl_Text_Buffer;
  tbuf->append( MoreHelpText );
  // textDisplay->wrap_mode( true, 50 );
  textDisplay->buffer( tbuf );
	
  win->show();
}


bool WorldGui::saveAsDialog()
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

bool WorldGui::closeWindowQuery()
{
  int choice;
	
  if ( wf ) {
    // worldfile loaded, ask to save
    choice = fl_choice("Quitting Stage",
		       "&Cancel", // ->0: defaults to ESC
		       "&Save, then quit", // ->1
		       "&Quit without saving" // ->2
		       );
		
    switch (choice) {
    case 1: // Save before quitting
      if ( saveAsDialog() ) 
	return true;      
      else 
	return false;
      
    case 2: // Quit without saving
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

void WorldGui::DrawBoundingBoxTree()
{
  FOR_EACH( it, World::children )
	 (*it)->DrawBoundingBoxTree();
}

void WorldGui::PushColor( Color col )
{ canvas->PushColor( col ); } 

void WorldGui::PushColor( double r, double g, double b, double a )
{ canvas->PushColor( r,g,b,a ); }

void WorldGui::PopColor()
{ canvas->PopColor(); }

Model* WorldGui::RecentlySelectedModel()
{ return canvas->last_selection; }


#include "stage.hh"
#include <FL/fl_draw.H>
#include <FL/fl_Box.H>
#include <FL/Fl_Menu_Button.H>

extern void glPolygonOffsetEXT (GLfloat, GLfloat);

void dummy_cb(Fl_Widget*, void* v) 
{
}

void StgWorldGui::SaveCallback( Fl_Widget* wid, StgWorldGui* world )
{
  world->Save();
}

void view_toggle_cb(Fl_Widget* w, bool* view ) 
{
  assert(view);
  *view = !(*view);
  
  Fl_Menu_Item* item = (Fl_Menu_Item*)((Fl_Menu_*)w)->mvalue();
  *view ?  item->check() : item->clear();
}


StgWorldGui::StgWorldGui(int W,int H,const char*L) 
  : Fl_Window(0,0,W,H,L)
{
  graphics = true;

  // build the menus
  Fl_Menu_Bar* mbar = new Fl_Menu_Bar(0,0, W, 30);// 640, 30);
  mbar->textsize(12);
  
  canvas = new StgCanvas( this,0,30,W,H-30 );
  resizable(canvas);
  end();
  
  mbar->add( "&File", 0, 0, 0, FL_SUBMENU );
  mbar->add( "File/&Save File", FL_CTRL + 's', (Fl_Callback *)SaveCallback, this );
  //mbar->add( "File/Save File &As...", FL_CTRL + FL_SHIFT + 's', (Fl_Callback *)dummy_cb, 0, FL_MENU_DIVIDER );
  mbar->add( "File/E&xit", FL_CTRL + 'q', (Fl_Callback *)dummy_cb, 0 );
 
  mbar->add( "&View", 0, 0, 0, FL_SUBMENU );

//    mbar->add( "View/Data", FL_CTRL+'z', (Fl_Callback *)view_toggle_cb, &canvas->show_data, FL_MENU_TOGGLE|FL_MENU_VALUE );  
//    mbar->add( "View/Blocks", FL_CTRL+'b', (Fl_Callback *)view_toggle_cb, &canvas->show_boxes, FL_MENU_TOGGLE|FL_MENU_VALUE );  
//    //mbar->add( "View/Clock", FL_CTRL+'c', (Fl_Callback *)view_toggle_cb, &show_clock, FL_MENU_TOGGLE|FL_MENU_VALUE );  
//    mbar->add( "View/Grid",      FL_CTRL + 'c', (Fl_Callback *)view_toggle_cb, &canvas->show_grid, FL_MENU_TOGGLE|FL_MENU_VALUE );
//    mbar->add( "View/Occupancy", FL_CTRL + 'o', (Fl_Callback *)view_toggle_cb, &canvas->show_occupancy, FL_MENU_TOGGLE|FL_MENU_VALUE );
//    mbar->add( "View/Tree",      FL_CTRL + 't', (Fl_Callback *)view_toggle_cb, &canvas->show_quadtree, FL_MENU_TOGGLE|FL_MENU_VALUE|FL_MENU_DIVIDER );
//    mbar->add( "View/Follow selection", FL_CTRL + 'f', (Fl_Callback *)view_toggle_cb, &canvas->follow_selection, FL_MENU_TOGGLE|FL_MENU_VALUE );

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
  canvas->interval = wf->ReadInt(wf_section, "redraw_interval", canvas->interval );


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

  canvas->invalidate(); // we probably changed something
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

  //wf->WriteInt( wf_section, "fill_polygons", show_fill );
  //wf->WriteInt( wf_section, "show_grid", show_grid );
  //wf->WriteInt( wf_section, "show_bboxes", show_bboxes );

//   // save the state of the property toggles
//   for( GList* list = toggle_list; list; list = g_list_next( list ) )
//     {
//       stg_property_toggle_args_t* tog = 
// 	(stg_property_toggle_args_t*)list->data;
//       assert( tog );
//       printf( "toggle item path: %s\n", tog->path );

//       wf->WriteInt( wf_section, tog->path, 
// 		   gtk_toggle_action_get_active(  GTK_TOGGLE_ACTION(tog->action) ));
//     }

  StgWorld::Save();
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


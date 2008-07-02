#include "stage_internal.hh"

using namespace Stg;


Option::Option( std::string n, std::string tok, std::string key, bool v ) : 
optName( n ), 
value( v ), 
wf_token( tok ), 
shortcut( key ), 
menu_item( NULL )
{ }

Option::Option( const Option& o ) : 
optName( o.optName ), 
value( o.value ), 
wf_token( o.wf_token ), 
shortcut( o.shortcut ), 
menu_item( o.menu_item )
{ }


void Option::Load( Worldfile* wf, int section )
{
  Set( (bool)wf->ReadInt(section, wf_token.c_str(), value ));
}

void Option::Save( Worldfile* wf, int section )
{
  wf->WriteInt(section, wf_token.c_str(), value );
}

void Option::CreateMenuItem( Fl_Menu_Bar* menu, std::string path )
{
  path = path + "/" + optName;
  menu->add( path.c_str(), shortcut.c_str(), 
				 (Fl_Callback*)ToggleCb, this, 
				 FL_MENU_TOGGLE | (value ? FL_MENU_VALUE : 0 ) );
  
  // find the menu item we just created and store it for later access
  menu_item = (Fl_Menu_Item*)menu->find_item( path.c_str() );
}		

void Option::ToggleCb( Fl_Menu_Bar* menubar, Option* opt ) 
{
  opt->Invert();
}


void Option::Invert()
{
  Set( !value );
}

void Option::Set( bool val )
{
  value = val;

  if( menu_item )
	 value ? menu_item->set() : menu_item->clear();
}

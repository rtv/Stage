#include "stage_internal.hh"

using namespace Stg;


Option::Option( std::string n, std::string tok, std::string key, bool v ) : 
optName( n ), 
value( v ), 
wf_token( tok ), 
shortcut( key ), 
menu( NULL ),
menuIndex( -1 )
{ }

Option::Option( const Option& o ) : 
optName( o.optName ), 
value( o.value ), 
wf_token( o.wf_token ), 
shortcut( o.shortcut ), 
menu( o.menu ),
menuIndex( o.menuIndex )
{ }


void Option::Load( Worldfile* wf, int section )
{
	set( (bool)wf->ReadInt( section, wf_token.c_str(), value ));
}

void Option::Save( Worldfile* wf, int section )
{
  wf->WriteInt(section, wf_token.c_str(), value );
}

void toggleCb( Fl_Widget* w, void* p ) 
{
	//Fl_Menu_* menu = static_cast<Fl_Menu_*>( w );
	Option* opt = static_cast<Option*>( p );
	opt->invert();
}

void Option::createMenuItem( Fl_Menu_Bar* m, std::string path )
{
	menu = m;
	path = path + "/" + optName;
	// create a menu item and save its index
	menuIndex = menu->add( path.c_str(), shortcut.c_str(), 
				 toggleCb, this, 
				 FL_MENU_TOGGLE | (value ? FL_MENU_VALUE : 0 ) );
}		

void Option::set( bool val )
{
	value = val;

	if( menu ) {
		const Fl_Menu_Item* mArr = menu->menu();
		Fl_Menu_Item* item = const_cast<Fl_Menu_Item*>( &mArr[ menuIndex ] );
		value ? item->set() : item->clear();
	}
}

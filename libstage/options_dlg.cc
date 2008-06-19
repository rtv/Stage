#include "options_dlg.hh"
#include <FL/Fl.H>


OptionsDlg::OptionsDlg( const std::vector<Option>& opts, Fl_Callback* cb, int w, int h ) :
Fl_Window( w, h, "Options" ),
options( opts ),
changedCb( cb ) {
	const int hm = w/6;
	const int vm = 2;
	const int btnH = 25;
	
	scroll = new Fl_Scroll( 0, 0, w, h-btnH-2*vm );
	scroll->type( Fl_Scroll::VERTICAL );
	
	Fl_Check_Button* check;
	for ( unsigned int i=0; i<options.size(); i++ ) {
		check = new Fl_Check_Button( 0,30*i, w, 30, options[ i ].name().c_str() );
		if ( options[ i ].val() )
			check->set();
		check->callback( checkChanged, this );
	}
	scroll->end();
	
	button = new Fl_Button( hm, h-btnH-vm, w-2*hm, btnH, "Apply to all" );
	button->callback( applyAllPress, this );
	this->end();
}

OptionsDlg::~OptionsDlg() {
	delete button;
	delete scroll; // deletes members
}

void OptionsDlg::display() {
	Fl_Window::show();
	while ( shown() )
		Fl::wait();
}

void OptionsDlg::checkChanged( Fl_Widget* w, void* p ) {
	Fl_Check_Button* check = static_cast<Fl_Check_Button*>( w );
	OptionsDlg* oDlg = static_cast<OptionsDlg*>( p );
	
	int item = oDlg->scroll->find( check );
	oDlg->options[ item ].set( check->value() );
	oDlg->changedItem = oDlg->options[ item ];
	oDlg->changedCb( oDlg, NULL );
}

void OptionsDlg::applyAllPress( Fl_Widget* w, void* p ) {
	OptionsDlg* oDlg = static_cast<OptionsDlg*>( p );
	
	oDlg->changedItem = Option( -1, "", false );
	oDlg->changedCb( oDlg, NULL );
}

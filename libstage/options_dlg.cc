#include "options_dlg.hh"
#include <FL/Fl.H>

OptionsDlg::OptionsDlg( std::vector<Option> options ) : Fl_Window( 180, 250, "Options" ) {
	end();
}

void OptionsDlg::display() {
	Fl_Window::show();
	while ( shown() )
		Fl::wait();
}

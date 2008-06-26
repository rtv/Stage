#include "options_dlg.hh"
#include <FL/Fl.H>

namespace Stg {

	OptionsDlg::OptionsDlg( int x, int y, int w, int h ) :
	Fl_Window( x,y, w,h, "Model Options" ),
	changedItem( NULL ),
	showAll( NULL ),
	status( NO_EVENT ),
	hm( w/6 ) {
		showAllCheck = new Fl_Check_Button( 0,0, w,boxH );
		showAllCheck->callback( checkChanged, this );
		showAllCheck->deactivate();
		
		scroll = new Fl_Scroll( 0,boxH+vm, w,h-boxH-btnH-3*vm );
		scroll->type( Fl_Scroll::VERTICAL );
		scroll->end();

		
		button = new Fl_Button( hm, h-btnH-vm, w-2*hm, btnH, "&Close" );
		button->callback( closePress, this );
		this->end();
	}

	OptionsDlg::~OptionsDlg() {
		delete button;
		delete scroll; // deletes members
		delete showAllCheck;
	}


	void OptionsDlg::checkChanged( Fl_Widget* w, void* p ) {
		Fl_Check_Button* check = static_cast<Fl_Check_Button*>( w );
		OptionsDlg* oDlg = static_cast<OptionsDlg*>( p );
		
		if ( check == oDlg->showAllCheck && oDlg->showAll ) {
			oDlg->status = CHANGE_ALL;
			oDlg->showAll->set( check->value() );
			oDlg->do_callback();
			oDlg->status = NO_EVENT;
		}
		else {
			int item = oDlg->scroll->find( check );
			oDlg->options[ item ]->set( check->value() );
			oDlg->changedItem = oDlg->options[ item ];
			oDlg->status = CHANGE;
			oDlg->do_callback();
			oDlg->changedItem = NULL;
			oDlg->status = NO_EVENT;
		}
	}

	void OptionsDlg::closePress( Fl_Widget* w, void* p ) {
		OptionsDlg* oDlg = static_cast<OptionsDlg*>( p );
		
		oDlg->status = CLOSE;
		oDlg->do_callback();
		oDlg->status = NO_EVENT;
	}

	int OptionsDlg::handle( int event ) {
	//	switch ( event ) {
	//		
	//	}
		
		return Fl_Window::handle( event );
	}


	void OptionsDlg::updateChecks() {
		scroll->clear();
		scroll->begin();
		Fl_Check_Button* check;
		for ( unsigned int i=0; i<options.size(); i++ ) {
			check = new Fl_Check_Button( 0,boxH*i, scroll->w(), boxH, options[ i ]->name().c_str() );
			if ( options[ i ]->val() )
				check->set();
			check->callback( checkChanged, this );
		}
		scroll->end();
	}

	void OptionsDlg::setOptions( const std::vector<Option*>& opts ) {
		options.clear();
		options.insert( options.begin(), opts.begin(), opts.end() );
		updateChecks();
	}
	
	void OptionsDlg::setOptions( const std::set<Option*, optComp>& opts ) {
		options.clear();
		options.insert( options.begin(), opts.begin(), opts.end() );
		updateChecks();
	}
	
	void OptionsDlg::showAllOpt( Option* opt ) {
		showAll = opt;
		showAllCheck->label( opt->name().c_str() );
		showAllCheck->value( opt->val() );
	}

} // namespace Stg 
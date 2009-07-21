#include "options_dlg.hh"
#include <FL/Fl.H>

namespace Stg {
  
  OptionsDlg::OptionsDlg( int x, int y, int w, int h ) :
	 Fl_Window( x,y, w,h, "Visualize" ),
	 changedItem( NULL ),
	 showAll( NULL ),
	 status( NO_EVENT ),
	 hm( w/6 ) 
  {
	 set_non_modal(); // keep on top but do not grab all events

	 showAllCheck = new Fl_Check_Button( 0,0, w,boxH );
	 showAllCheck->callback( checkChanged, this );
	 showAllCheck->box( FL_UP_FRAME );

 	 scroll = new Fl_Scroll( 0,boxH+vm, w,h-boxH-3*vm );
	 resizable( scroll );
 	 scroll->type( Fl_Scroll::VERTICAL );
	 scroll->end();
	 
	 this->end();
  }
  
	OptionsDlg::~OptionsDlg() {
	  delete scroll; // deletes members
	  delete showAllCheck;
	}
  
  
  void OptionsDlg::checkChanged( Fl_Widget* w, void* p ) {
	 Fl_Check_Button* check = static_cast<Fl_Check_Button*>( w );
	 OptionsDlg* oDlg = static_cast<OptionsDlg*>( p );
	 
	 if ( check == oDlg->showAllCheck && oDlg->showAll ) 
		{
		  oDlg->status = CHANGE_ALL;
		  oDlg->showAll->set( check->value() );
		  oDlg->do_callback();
		  oDlg->status = NO_EVENT;
		}
	 else 
		{
		  int item = oDlg->scroll->find( check );
		  oDlg->options[ item ]->set( check->value() );
		  oDlg->changedItem = oDlg->options[ item ];
		  oDlg->status = CHANGE;
		  oDlg->do_callback();
		  oDlg->changedItem = NULL;
		  oDlg->status = NO_EVENT;
		}
  }
  
	void OptionsDlg::updateChecks() {
		if (scroll->children())
			scroll->clear();
		scroll->begin();
		Fl_Check_Button* check;
		for ( unsigned int i=0; i<options.size(); i++ ) {
			check = new Fl_Check_Button( 0,boxH*(i+1)+vm, scroll->w(), boxH, options[ i ]->name().c_str() );
			if ( options[ i ]->val() )
				check->set();
			check->callback( checkChanged, this );
		}
		scroll->end();
		this->redraw();
	}

	void OptionsDlg::setOptions( const std::set<Option*>& opts ) {
	  options.clear();
		options.assign( opts.begin(), opts.end() );
		updateChecks();
	}
	
	
	void OptionsDlg::showAllOpt( Option* opt ) {
		showAll = opt;
		showAllCheck->label( opt->name().c_str() );
		showAllCheck->value( opt->val() );
	}

} // namespace Stg 

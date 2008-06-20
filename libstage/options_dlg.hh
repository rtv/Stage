#ifndef _OPTIONS_DLG_H_
#define _OPTIONS_DLG_H_

#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>

#include <string>
#include <vector>


class Option {
private:
	int index;
	std::string optName;
	bool value;
public:
	Option() { }
	Option( int i, std::string n, bool v ) : index( i ), optName( n ), value( v ) { }
	Option( const Option& o ) : index( o.index ), optName( o.optName ), value( o.value ) { }
	const std::string name() const { return optName; }
	const int id() const { return index; }
	const bool val() const { return value; }
	void set( bool val ) { value = val; }
};


class OptionsDlg : protected Fl_Window {
private:
	std::vector<Option> options;
	Fl_Scroll* scroll;
	Fl_Button* button;
	virtual int OptionsDlg::handle( int event );
	static void checkChanged( Fl_Widget* w, void* p );
	static void applyAllPress( Fl_Widget* w, void* p );
	Option changedItem;
public:
	OptionsDlg( const std::vector<Option>& opts, int w, int h );
	virtual ~OptionsDlg();
	void display();	
	void callback( Fl_Callback* cb, void* p ) { Fl_Window::callback( cb, p ); }
	//void setChangeCb( Fl_Callback* cb, void* p );
	const Option changed() const { return changedItem; }
	void hide() { Fl_Window::hide(); }
};

#endif

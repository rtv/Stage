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
	std::string optName;
	bool value;
	int index;
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
	Fl_Callback* changedCb;
	static void checkChanged( Fl_Widget* w, void* p );
	static void applyAllPress( Fl_Widget* w, void* p );
	Option changedItem;
public:
	OptionsDlg( const std::vector<Option>& opts, Fl_Callback* cb, int w, int h );
	//void setChangeCb( Fl_Callback* cb ) { changedCb=cb; }
	virtual ~OptionsDlg();
	void display();	

	const Option changed() const { return changedItem; }
};

#endif

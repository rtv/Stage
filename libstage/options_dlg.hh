#ifndef _OPTIONS_DLG_H_
#define _OPTIONS_DLG_H_

#include <FL/Fl_Window.H>

#include <string>
#include <vector>


class Option {
private:
	std::string name;
	bool value;
public:
	Option( std::string n, bool v ) : name(n), value(v) { }
	inline const bool get() const { return value; }
	inline void set( bool val ) { value = val; }
};


class OptionsDlg : private Fl_Window {
private:
public:
	OptionsDlg( std::vector<Option> options );
	
	void display();
};

#endif
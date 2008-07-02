/** option.hh
 Class that encapsulates a boolean and pairs it with a string description
 Used to pass settings between the GUI and the drawing classes
 
 Author: Jeremy Asher, Richard Vaughan
 SVN: $Id$
*/


#ifndef _OPTION_H_
#define _OPTION_H_

#include <string>
#include "worldfile.hh"

#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>

namespace Stg {

	class Option {
	private:
	  friend bool compare( const Option* lhs, const Option* rhs );

	  std::string optName;
	  bool value;
	  /** worldfile entry string for loading and saving this value */
	  std::string wf_token; 
	  std::string shortcut;
	  Fl_Menu_Item* menu_item;
	  
	public:
		Option( std::string n, std::string tok, std::string key, bool v );	  
		Option( const Option& o );

	  const std::string name() const { return optName; }
		inline bool val() const { return value; }
		inline operator bool() { return val(); }
		inline bool operator<( const Option& rhs ) const
			{ return optName<rhs.optName; } 
	  void Set( bool val );
	  void Invert();

	  // Comparator to dereference Option pointers and compare their strings
	  struct optComp {
		 inline bool operator()( const Option* lhs, const Option* rhs ) const
		 { return lhs->operator<(*rhs); } 
	  };
	  
	  static void ToggleCb( Fl_Menu_Bar* menubar, Option* opt );

	  void Load( Worldfile* wf, int section );
	  void Save( Worldfile* wf, int section );	  
	  void CreateMenuItem( Fl_Menu_Bar* menu, std::string path );
	};
}
	
#endif

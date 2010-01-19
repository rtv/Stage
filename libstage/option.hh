#ifndef _OPTION_H_
#define _OPTION_H_

#include <string>
#include "worldfile.hh"

#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>

namespace Stg {
	class World;
	/** option.hh
	 Class that encapsulates a boolean and pairs it with a string description
	 Used to pass settings between the GUI and the drawing classes
	 
	 Author: Jeremy Asher, Richard Vaughan
	 SVN: $Id: option.hh 8061 2009-07-21 01:49:26Z rtv $
	 */	
	class Option {
	private:
		friend bool compare( const Option* lhs, const Option* rhs );

		std::string optName;
		bool value;
		/** worldfile entry string for loading and saving this value */
		std::string wf_token; 
		std::string shortcut;
		Fl_Menu_* menu;
		int menuIndex;
		Fl_Callback* menuCb;
		Fl_Widget* menuCbWidget;
		World* _world;
	  
	public:
		Option( std::string n, std::string tok, std::string key, bool v, World *world );	  

		const std::string name() const { return optName; }
		inline bool isEnabled() const { return value; }
		inline bool val() const { return value; }
		inline operator bool() { return val(); }
		inline bool operator<( const Option& rhs ) const
			{
			  puts( "comparing" );
			  return optName<rhs.optName; 
			} 
		void set( bool val );
		void invert() { set( !value ); }

// 		// Comparator to dereference Option pointers and compare their strings
// 		struct optComp {
// 			inline bool operator()( const Option* a, const Option* b ) const
// 			//{ return lhs->operator<(*rhs); } 
// 				{ return a->optName < b->optName; } 
// 		};


		void createMenuItem( Fl_Menu_Bar* menu, std::string path );
		void menuCallback( Fl_Callback* cb, Fl_Widget* w );
		static void toggleCb( Fl_Widget* w, void* p );
		void Load( Worldfile* wf, int section );
		void Save( Worldfile* wf, int section );	  

	  const char* htname;
	};
}
	
#endif

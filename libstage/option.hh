#ifndef _OPTION_H_
#define _OPTION_H_

#include "worldfile.hh"
#include <string>

#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>

namespace Stg {
class World;
/** option.hh
   Class that encapsulates a boolean and pairs it with a string description
   Used to pass settings between the GUI and the drawing classes

   Author: Jeremy Asher, Richard Vaughan
   SVN: $Id$
   */
class Option {
private:
  friend bool compare(const Option *lhs, const Option *rhs);

  std::string optName;
  bool value;
  /** worldfile entry string for loading and saving this value */
  std::string wf_token;
  std::string shortcut;
  Fl_Menu_ *menu;
  int menuIndex;
  /** Callback type for menu events*/
  typedef void (*Callback)(Option *opt, void * data);
  Callback menuCb;
  void *callbackData;
  World *_world;

public:
  Option(const std::string &n, const std::string &tok, const std::string &key, bool v,
         World *world);

  const std::string name() const { return optName; }
  inline bool isEnabled() const { return value; }
  inline bool val() const { return value; }
  inline operator bool() { return val(); }
  inline bool operator<(const Option &rhs) const
  {
    puts("comparing");
    return optName < rhs.optName;
  }
  void set(bool val);
  void invert() { set(!value); }
  // 		// Comparator to dereference Option pointers and compare their strings
  // 		struct optComp {
  // 			inline bool operator()( const Option* a, const Option* b ) const
  // 			//{ return lhs->operator<(*rhs); }
  // 				{ return a->optName < b->optName; }
  // 		};

  void setOptionCallback(Callback cb, void *w);
  void Load(Worldfile *wf, int section);
  void Save(Worldfile *wf, int section);

  std::string htname;
};
}

#endif

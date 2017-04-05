#ifndef _OPTIONS_DLG_H_
#define _OPTIONS_DLG_H_

#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Window.H>

#include <set>
#include <string>
#include <vector>

#include "option.hh"
#include "stage.hh"

namespace Stg {

// class Option;

class OptionsDlg : public Fl_Window {
public:
  enum event_t { NO_EVENT, CHANGE, CHANGE_ALL, CLOSE };

private:
  std::vector<Option *> options;
  Option *changedItem;
  Option *showAll;
  event_t status;
  Fl_Scroll *scroll;
  Fl_Check_Button *showAllCheck;
  void updateChecks();

  static void checkChanged(Fl_Widget *w, void *p);

  // constants
  static const int vm = 4;
  // const int hm;
  static const int boxH = 30;

public:
  OptionsDlg(int x, int y, int w, int h);
  virtual ~OptionsDlg();

  void setOptions(const std::set<Option *> &opts);
  void clearOptions() { options.clear(); }
  void showAllOpt(Option *opt);
  event_t event() const { return status; }
  Option *changed() { return changedItem; }
};
}

#endif

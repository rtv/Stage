/*
 * stage_gui.hh
 *
 *  Created on: Nov 10, 2017
 *      Author: vrobot
 */

#ifndef STAGE_LIBSTAGE_STAGE_GUI_HH_
#define STAGE_LIBSTAGE_STAGE_GUI_HH_

#include "stage.hh"
#include "canvas_fltk.hh"

// FLTK Gui includes
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/gl.h> // FLTK takes care of platform-specific GL stuff

namespace Stg {
/** Extends World to implement an FLTK / OpenGL graphical user
      interface.
  */
class CanvasFLTK;

class WorldGui : public World, public Fl_Window {
  friend class Canvas;
  friend class ModelCamera;
  friend class Model;
  friend class Option;

private:
  CanvasFLTK *canvas;
  std::vector<Option *> drawOptions;
  FileManager *fileMan; ///< Used to load and save worldfiles
  std::vector<usec_t> interval_log;

  /** Stage attempts to run this many times faster than real
time. If -1, Stage runs as fast as possible. */
  double speedup;

  bool confirm_on_quit; ///< if true, show save dialog on (GUI) exit (default)

  Fl_Menu_Bar *mbar;
  OptionsDlg *oDlg;
  bool pause_time;
  std::string caption_prefix; //!< prefix of window caption (default PROJECT name constant)

  /** The amount of real time elapsed between $timing_interval
timesteps. */
  usec_t real_time_interval;

  /** The current real time in microseconds. */
  usec_t real_time_now;

  /** The last recorded real time, sampled every $timing_interval
updates. */
  usec_t real_time_recorded;

  /** Number of updates between measuring elapsed real time. */
  uint64_t timing_interval;

  // static callback functions
  static void windowCb(Fl_Widget *w, WorldGui *wg);
  static void fileLoadCb(Fl_Widget *w, WorldGui *wg);
  static void fileSaveCb(Fl_Widget *w, WorldGui *wg);
  static void fileSaveAsCb(Fl_Widget *w, WorldGui *wg);
  static void fileExitCb(Fl_Widget *w, WorldGui *wg);
  static void viewOptionsCb(OptionsDlg *oDlg, WorldGui *wg);
  static void optionsDlgCb(OptionsDlg *oDlg, WorldGui *wg);
  static void helpAboutCb(Fl_Widget *w, WorldGui *wg);
  static void pauseCb(Fl_Widget *w, WorldGui *wg);
  static void onceCb(Fl_Widget *w, WorldGui *wg);
  static void fasterCb(Fl_Widget *w, WorldGui *wg);
  static void slowerCb(Fl_Widget *w, WorldGui *wg);
  static void realtimeCb(Fl_Widget *w, WorldGui *wg);
  static void fasttimeCb(Fl_Widget *w, WorldGui *wg);
  static void resetViewCb(Fl_Widget *w, WorldGui *wg);
  static void moreHelptCb(Fl_Widget *w, WorldGui *wg);

  // GUI functions
  bool saveAsDialog();
  bool closeWindowQuery();

  virtual void AddModel(Model *mod);

  void SetTimeouts();

  /// Defines what all WorldGUI::Load(*) in methods have in common. Called after initial setup.
  void LoadWorldGuiPostHook(usec_t load_start_time);

protected:
  virtual void PushColor(Color col);
  virtual void PushColor(double r, double g, double b, double a);
  virtual void PopColor();
public:
  WorldGui(int width, int height, const char *caption = NULL);
  ~WorldGui();

  /** Forces the window to be redrawn, even if paused.*/
  virtual void Redraw(void);

  virtual std::string ClockString() const;
  virtual bool Update();
  virtual bool Load(const std::string &worldfile_path);
  virtual bool Load(std::istream &world_content, const std::string &worldfile_path = std::string());

  virtual void UnLoad();
  virtual bool Save(const char *filename);
  virtual bool IsGUI() const { return true; }
  virtual Model *RecentlySelectedModel() const;

  virtual void Start();
  virtual void Stop();

  usec_t RealTimeNow(void) const;

  Canvas *GetCanvas(void);
  /** show the window - need to call this if you don't Load(). */
  void Show();

  /** Get human readable string that describes the current global energy state. */
  std::string EnergyString(void) const;
  virtual void RemoveChild(Model *mod);

  bool IsTopView();
};

} //namespace Stg

#endif /* STAGE_LIBSTAGE_STAGE_GUI_HH_ */

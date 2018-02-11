#ifndef _CANVAS_FLTK_HH_
#define _CANVAS_FLTK_HH_

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Menu_Bar.H>

#include "canvas.hh"
#include "world_gui.hh"

namespace Stg {

class MenuManagerFLTK;

class CanvasFLTK : public Canvas, public Fl_Gl_Window {
  friend class WorldGui; // allow access to private members
  friend class Model;
protected:
  /** State flag for drag&drop*/
  bool selectedModel;
  int startx, starty;
  int empty_space_startx, empty_space_starty;

  Option showScreenshots;

  MenuManagerFLTK * menu_manager;
public:
  CanvasFLTK(WorldGui *world, int x, int y, int width, int height);
  ~CanvasFLTK();

  bool graphics;
  int screenshot_frame_skip;

  void Screenshot();
  void createMenuItems(Fl_Menu_Bar *menu, std::string path);

  virtual void renderFrame() override;
  virtual int handle(int event) override;
  void resize(int X, int Y, int W, int H) override;

  virtual bool isValid() override;
	virtual void doRedraw() override;
	virtual int getWidth() const override;
	virtual int getHeight() const override;
	virtual void setInvalidate() override;

	double fontWidth(const char * str) const override;
	double fontHeight() const override;

	void draw_string(float x, float y, float z, const char *str) override;
	void draw_string_multiline(float x, float y, float w, float h, const char *str, int align) override;

  void InvertView(uint32_t invertflags);

  static void TimerCallback(Canvas *canvas);
  static void perspectiveCb(Option * opt, void *p);

  void Load(Worldfile *wf, int sec) override;
  void Save(Worldfile *wf, int section) override;

  /** Overriding draw method from Fl_Gl_Window*/
  virtual void draw() override;
};
} //namespace Stg

#endif

#ifndef _CANVAS_FLTK_HH_
#define _CANVAS_FLTK_HH_

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Menu_Bar.H>

#include "canvas.hh"
#include "world_gui.hh"

namespace Stg {

class CanvasFLTK;

class MenuManagerFLTK : public MenuManager
{
public:
	MenuManagerFLTK(CanvasFLTK * canvas, Fl_Menu_Bar *menu);
	virtual void syncMenuState(Option * opt);
	/** Creates menu item for an option */
	void createMenuItem(Option & opt, std::string path);
protected:
	/** Toggle callback from FLTK */
	static void toggleCb(Fl_Widget *w, void *p);
	CanvasFLTK * GetCanvas();

	Fl_Menu_ *menu;
	CanvasFLTK * canvas;
};

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
  void InitTextures();
  void createMenuItems(Fl_Menu_Bar *menu, std::string path);

  virtual void renderFrame();
  virtual int handle(int event);
  void resize(int X, int Y, int W, int H);

  virtual bool isValid() {return valid();}
	virtual void doRedraw() {redraw();}
	virtual int getWidth() const {return w();}
	virtual int getHeight() const {return h();}
	virtual void setInvalidate() { invalidate(); }

	double fontWidth(const char * str) const;
	double fontHeight() const;

	void draw_string(float x, float y, float z, const char *str);
	void draw_string_multiline(float x, float y, float w, float h, const char *str, int align);

  void InvertView(uint32_t invertflags);

  static void TimerCallback(Canvas *canvas);
  static void perspectiveCb(Option * opt, void *p);

  void Load(Worldfile *wf, int sec);
  void Save(Worldfile *wf, int section);
};
} //namespace Stg

#endif

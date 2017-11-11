#ifndef _CANVAS_FLTK_HH_
#define _CANVAS_FLTK_HH_

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Menu_Bar.H>

#include "canvas.hh"

namespace Stg {
class CanvasFLTK : public Canvas, public Fl_Gl_Window {
  friend class WorldGui; // allow access to private members
  friend class Model;

protected:
  void createMenuItem(Option * opt, Fl_Menu_Bar *menu, std::string path);
  static void toggleCb(Fl_Widget *w, void *p);

public:
  CanvasFLTK(WorldGui *world, int x, int y, int width, int height);
  ~CanvasFLTK();

  bool graphics;
  int screenshot_frame_skip;

  void Screenshot();
  void InitGl();
  void InitTextures();
  void createMenuItems(Fl_Menu_Bar *menu, std::string path);

  void FixViewport(int W, int H);
  void resetCamera();
  virtual void renderFrame();
  virtual void draw();
  virtual int handle(int event);
  void resize(int X, int Y, int W, int H);

  virtual bool isValid() {return valid();}
	virtual void doRedraw() {redraw();}
	virtual int getWidth() const {return w();}
	virtual int getHeight() const {return h();}
	virtual void setInvalidate() { invalidate(); }

  void CanvasToWorld(int px, int py, double *wx, double *wy, double *wz);

  void InvertView(uint32_t invertflags);

  static void TimerCallback(Canvas *canvas);
  static void perspectiveCb(Option * opt, void *p);
};
} //namespace Stg

#endif

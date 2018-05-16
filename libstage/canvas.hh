#ifndef _CANVAS_HH_
#define _CANVAS_HH_

#include "option.hh"
#include "stage.hh"

#include <map>
#include <stack>

namespace Stg {

class Canvas {
  friend class WorldGui; // allow access to private members
  friend class Model;

protected:
  class GlColorStack {
  public:
    GlColorStack() : colorstack() {}
    ~GlColorStack() {}
    void Push(double r, double g, double b, double a = 1.0) { Push(Color(r, g, b, a)); }
    void Push(Color col)
    {
      colorstack.push(col);
      glColor4f(col.r, col.g, col.b, col.a);
    }

    void Pop()
    {
      if (colorstack.size() < 1)
        PRINT_WARN1("Attempted to ColorStack.Pop() but ColorStack %p is empty",
                    static_cast<void *>(this));
      else {
        Color &old = colorstack.top();
        colorstack.pop();
        glColor4f(old.r, old.g, old.b, old.a);
      }
    }

    unsigned int Length() { return colorstack.size(); }
  private:
    std::stack<Color> colorstack;
  } colorstack;

  std::list<Model *> models_sorted;

  Camera *current_camera;
  OrthoCamera camera;
  PerspectiveCamera perspective_camera;
  bool dirty_buffer;
  Worldfile *wf;
  bool clicked_empty_space;
  std::list<Model *> selected_models;
  Model *last_selection; ///< the most recently selected model
  ///(even if it is now unselected).

  msec_t interval; // window refresh interval in ms
  bool init_done;
  bool texture_load_done;


  void RecordRay(double x1, double y1, double x2, double y2);
  void DrawRays();
  void ClearRays();
  void DrawGlobalGrid();

  void AddModel(Model *mod);
  void RemoveModel(Model *mod);

  Option // showBlinken,
      showBBoxes,
      showBlocks, showBlur, showClock, showData, showFlags, showFollow, showFootprints, showGrid,
      showOccupancy, showStatus, showTrailArrows, showTrailRise, showTrails,
      showVoxels, pCamOn, visualizeAll;

public:
  Canvas(World *world, int x, int y, int width, int height);
  virtual ~Canvas();

  bool graphics;
  World *world;
  unsigned long frames_rendered_count;

  std::map<std::string, Option *> _custom_options;

  void InitGl();
  void InitTextures();

  void FixViewport(int W, int H);
  void DrawFloor(); // simpler floor compared to grid
  void DrawBlocks();
  void DrawBoundingBoxes();
  void resetCamera();
  virtual void renderFrame();
  /** Switch between camera modes
   * @param mode - camera mode:
   *  - true: perspective camera
   *  - false: default camera
   */
  void switchCameraMode(bool mode);

  virtual bool isValid() {return false;}
  virtual void doRedraw() {}
  virtual int getWidth() const {return 1;}
  virtual int getHeight() const {return 1;}
  virtual void setInvalidate() {}

  msec_t getRedrawInterval() const {return interval; }

  /// Wraps interaction with current font geometry
  virtual double fontWidth(const char * str) const {return 0;};
  virtual double fontHeight() const {return 0;};

  virtual void draw_array(float x, float y, float w, float h, float *data, size_t len, size_t offset, float min, float max);
  virtual void draw_array(float x, float y, float w, float h, float *data, size_t len, size_t offset);

  virtual void draw_string(float x, float y, float z, const char *string);
  virtual void draw_string_multiline(float x, float y, float w, float h, const char *string, int align);

  void draw_speech_bubble(float x, float y, float z, const char *str);
  void draw_grid(bounds3d_t vol);

  void DrawPose(Pose pose);

  void CanvasToWorld(int px, int py, double *wx, double *wy, double *wz);

  // Get model under screen coordinates {x,y}
  Model *getModel(int x, int y);
  bool selected(Model *mod);
  void select(Model *mod);
  void unSelect(Model *mod);
  void unSelectAll();

  inline void setDirtyBuffer(void) { dirty_buffer = true; }
  inline bool dirtyBuffer(void) const { return dirty_buffer; }
  void PushColor(Color col) { colorstack.Push(col); }
  void PushColor(double r, double g, double b, double a) { colorstack.Push(r, g, b, a); }
  void PopColor() { colorstack.Pop(); }
  void InvertView(uint32_t invertflags);

  bool VisualizeAll() { return !visualizeAll; }
  //static void TimerCallback(Canvas *canvas);
  //static void perspectiveCb(Fl_Widget *w, void *p);

  void EnterScreenCS();
  void LeaveScreenCS();

  virtual void Load(Worldfile *wf, int section);
  virtual void Save(Worldfile *wf, int section);

  bool IsTopView() { return ((fabs(camera.yaw()) < 0.1) && (fabs(camera.pitch()) < 0.1)); }
};

} // namespace Stg

#endif

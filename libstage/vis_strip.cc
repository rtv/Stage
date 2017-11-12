/*  Strip plot visualizer
 *  Richard Vaughan 30 March 2009
 */

#include "canvas.hh"
#include "stage.hh"
using namespace Stg;

StripPlotVis::StripPlotVis(float x, float y, float w, float h, size_t len, Color fgcolor,
                           Color bgcolor, const char *name, const char *wfname)
    : Visualizer(name, wfname), data(new float[len]), len(len), count(0), x(x), y(y), w(w), h(h),
      min(1e32), max(-1e32), fgcolor(fgcolor), bgcolor(bgcolor)
{
  // zero the data
  memset(data, 0, len * sizeof(float));
}

StripPlotVis::~StripPlotVis()
{
  if (data)
    free(data);
}

void StripPlotVis::Visualize(Model *mod, Camera *camera, Canvas * canvas)
{
  if (!canvas->selected(mod)) // == canvas->SelectedVisualizeAll() )
    return;

  canvas->EnterScreenCS();

  mod->PushColor(bgcolor);
  glRectf(x, y, w, h);
  mod->PopColor();

  mod->PushColor(fgcolor);
  canvas->draw_array(x, y, w, h, data, len, count % len, min, max);
  mod->PopColor();

  canvas->LeaveScreenCS();
}

void StripPlotVis::AppendValue(float value)
{
  data[count % len] = value;
  count++;

  min = std::min(value, min);
  max = std::max(value, max);
}

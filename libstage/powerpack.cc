/** powerpack.cc
         Simple model of energy storage
         Richard Vaughan
         Created 2009.1.15
    SVN: $Id$
*/

#include "stage.hh"
#include "texture_manager.hh"
using namespace Stg;

joules_t PowerPack::global_stored = 0.0;
joules_t PowerPack::global_input = 0.0;
joules_t PowerPack::global_capacity = 0.0;
joules_t PowerPack::global_dissipated = 0.0;

PowerPack::PowerPack(Model *mod)
    : event_vis(2.0 * std::max(fabs(ceil(mod->GetWorld()->GetExtent().x.max)),
                               fabs(floor(mod->GetWorld()->GetExtent().x.min))),
                2.0 * std::max(fabs(ceil(mod->GetWorld()->GetExtent().y.max)),
                               fabs(floor(mod->GetWorld()->GetExtent().y.min))),
                1.0),
      output_vis(0, 100, 200, 40, 1200, Color(1, 0, 0), Color(0, 0, 0, 0.5), "energy output",
                 "energy_input"),
      stored_vis(0, 142, 200, 40, 1200, Color(0, 1, 0), Color(0, 0, 0, 0.5), "energy stored",
                 "energy_stored"),
      mod(mod), stored(0.0), capacity(0.0), charging(false), dissipated(0.0), last_time(0),
      last_joules(0.0), last_watts(0.0)
{
  // tell the world about this new pp
  mod->world->AddPowerPack(this);

  mod->AddVisualizer(&event_vis, false);
  mod->AddVisualizer(&output_vis, false);
  mod->AddVisualizer(&stored_vis, false);
}

PowerPack::~PowerPack()
{
  mod->world->RemovePowerPack(this);
  mod->RemoveVisualizer(&event_vis);
  mod->RemoveVisualizer(&output_vis);
  mod->RemoveVisualizer(&stored_vis);
}

/** OpenGL visualization of the powerpack state */
void PowerPack::Visualize(Camera *cam)
{
  (void)cam; // avoid warning about unused var

  const double height = 0.5;
  const double width = 0.2;

  double percent = stored / capacity * 100.0;

  const double alpha = 0.5;

  if (percent > 50)
    glColor4f(0, 1, 0, alpha); // green
  else if (percent > 25)
    glColor4f(1, 0, 1, alpha); // magenta
  else
    glColor4f(1, 0, 0, alpha); // red

  //  snprintf( buf, 32, "%.0f", percent );

  glTranslatef(-width, 0.0, 0.0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  GLfloat fullness = height * (percent * 0.01);
  glRectf(0, 0, width, fullness);

  // outline the charge-o-meter
  glTranslatef(0, 0, 0.1);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glColor4f(0, 0, 0, 0.7);

  glRectf(0, 0, width, height);

  glBegin(GL_LINES);
  glVertex2f(0, fullness);
  glVertex2f(width, fullness);
  glEnd();

  if (stored < 0.0) // inifinite supply!
  {
    // draw an arrow toward the top
    glBegin(GL_LINES);
    glVertex2f(width / 3.0, height / 3.0);
    glVertex2f(2.0 * width / 3, height / 3.0);

    glVertex2f(width / 3.0, height / 3.0);
    glVertex2f(width / 3.0, height - height / 5.0);

    glVertex2f(width / 3.0, height - height / 5.0);
    glVertex2f(0, height - height / 5.0);

    glVertex2f(0, height - height / 5.0);
    glVertex2f(width / 2.0, height);

    glVertex2f(width / 2.0, height);
    glVertex2f(width, height - height / 5.0);

    glVertex2f(width, height - height / 5.0);
    glVertex2f(2.0 * width / 3.0, height - height / 5.0);

    glVertex2f(2.0 * width / 3.0, height - height / 5.0);
    glVertex2f(2.0 * width / 3, height / 3.0);

    glEnd();
  }

  if (charging) {
    glLineWidth(6.0);
    glColor4f(1, 0, 0, 0.7);

    glRectf(0, 0, width, height);

    glLineWidth(1.0);
  }

  // compute the instantaneous power output
  usec_t time_now = mod->world->SimTimeNow();
  usec_t delta_t = time_now - last_time;
  watts_t watts = last_watts;

  if (delta_t > 0) // some sim time elapsed
  {
    joules_t delta_j = stored - last_joules;
    watts_t watts = (-1e6 * delta_j) / (double)delta_t;

    last_joules = stored;
    last_time = time_now;
    last_watts = watts;
  }

  if (fabs(watts) > 1e-5) // any current
  {
    glColor4f(1, 0, 0, 0.8); // red
    char buf[32];
    snprintf(buf, 32, "%.1fW", watts);
    Gl::draw_string(-0.05, height + 0.05, 0, buf);
  }
}

joules_t PowerPack::RemainingCapacity() const
{
  return (capacity - stored);
}

void PowerPack::Add(joules_t j)
{
  joules_t amount = std::min(RemainingCapacity(), j);
  stored += amount;
  global_stored += amount;

  if (amount > 0)
    charging = true;
}

void PowerPack::Subtract(joules_t j)
{
  if (stored < 0) // infinte supply!
  {
    global_input += j; // record energy entering the system
    return;
  }

  joules_t amount = std::min(stored, j);

  stored -= amount;
  global_stored -= amount;
}

void PowerPack::TransferTo(PowerPack *dest, joules_t amount)
{
  // printf( "amount %.2f stored %.2f dest capacity %.2f\n",
  //	 amount, stored, dest->RemainingCapacity() );

  // if stored is non-negative we can't transfer more than the stored
  // amount. If it is negative, we have infinite energy stored
  if (stored >= 0.0)
    amount = std::min(stored, amount);

  // we can't transfer more than he can take
  amount = std::min(amount, dest->RemainingCapacity());

  // printf( "%s gives %.3f J to %s\n",
  //	 mod->Token(), amount, dest->mod->Token() );

  Subtract(amount);
  dest->Add(amount);

  mod->NeedRedraw();
}

void PowerPack::SetCapacity(joules_t cap)
{
  global_capacity -= capacity;
  capacity = cap;
  global_capacity += capacity;

  if (stored > cap) {
    global_stored -= stored;
    stored = cap;
    global_stored += stored;
  }
}

joules_t PowerPack::GetCapacity() const
{
  return capacity;
}

joules_t PowerPack::GetStored() const
{
  return stored;
}

joules_t PowerPack::GetDissipated() const
{
  return dissipated;
}

void PowerPack::SetStored(joules_t j)
{
  global_stored -= stored;
  stored = j;
  global_stored += stored;
}

void PowerPack::Dissipate(joules_t j)
{
  joules_t amount = (stored < 0) ? j : std::min(stored, j);

  Subtract(amount);
  dissipated += amount;
  global_dissipated += amount;

  output_vis.AppendValue(amount);
  stored_vis.AppendValue(stored);
}

void PowerPack::Dissipate(joules_t j, const Pose &p)
{
  Dissipate(j);
  event_vis.Accumulate(p.x, p.y, j);
}

//------------------------------------------------------------------------------
// Dissipation Visualizer class

joules_t PowerPack::DissipationVis::global_peak_value = 0.0;

PowerPack::DissipationVis::DissipationVis(meters_t width, meters_t height, meters_t cellsize)
    : Visualizer("energy dissipation", "energy_dissipation"), columns(width / cellsize),
      rows(height / cellsize), width(width), height(height), cells(columns * rows), peak_value(0),
      cellsize(cellsize)
{ /* nothing to do */
}

PowerPack::DissipationVis::~DissipationVis()
{
}

void PowerPack::DissipationVis::Visualize(Model *mod, Camera *cam)
{
  (void)cam; // avoid warning about unused var

  // go into world coordinates

  glPushMatrix();

  Gl::pose_inverse_shift(mod->GetGlobalPose());

  glTranslatef(-width / 2.0, -height / 2.0, 0.01);
  glScalef(cellsize, cellsize, 1);

  for (unsigned int y = 0; y < rows; y++)
    for (unsigned int x = 0; x < columns; x++) {
      joules_t j = cells[y * columns + x];

      // printf( "%d %d %.2f\n", x, y, j );

      if (j > 0) {
        glColor4f(1.0, 0, 0, j / global_peak_value);
        glRectf(x, y, x + 1, y + 1);
      }
    }

  glPopMatrix();
}

void PowerPack::DissipationVis::Accumulate(meters_t x, meters_t y, joules_t amount)
{
  // printf( "accumulate %.2f %.2f %.2f\n", x, y, amount );

  int ix = (x + width / 2.0) / cellsize;
  int iy = (y + height / 2.0) / cellsize;

  // don't accumulate if we're outside the grid
  if (ix < 0 || ix >= int(columns) || iy < 0 || iy >= int(rows))
    return;

  joules_t &j = cells[ix + (iy * columns)];

  j += amount;
  if (j > peak_value) {
    peak_value = j;

    if (peak_value > global_peak_value)
      global_peak_value = peak_value;
  }
}

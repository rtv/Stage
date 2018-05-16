#include "stage.hh"

using namespace Stg;

// transform the current coordinate frame by the given pose
void Stg::Gl::coord_shift(double x, double y, double z, double a)
{
  glTranslatef(x, y, z);
  glRotatef(rtod(a), 0, 0, 1);
}

// transform the current coordinate frame by the given pose
void Stg::Gl::pose_shift(const Pose &pose)
{
  coord_shift(pose.x, pose.y, pose.z, pose.a);
}

void Stg::Gl::pose_inverse_shift(const Pose &pose)
{
  coord_shift(0, 0, 0, -pose.a);
  coord_shift(-pose.x, -pose.y, -pose.z, 0);
}

// draw an octagon with center rectangle dimensions w/h
//   and outside margin m
void Stg::Gl::draw_octagon(float w, float h, float m)
{
  glBegin(GL_POLYGON);
  glVertex2f(m + w, 0);
  glVertex2f(w + 2 * m, m);
  glVertex2f(w + 2 * m, h + m);
  glVertex2f(m + w, h + 2 * m);
  glVertex2f(m, h + 2 * m);
  glVertex2f(0, h + m);
  glVertex2f(0, m);
  glVertex2f(m, 0);
  glEnd();
}

// draw an octagon with center rectangle dimensions w/h
//   and outside margin m
void Stg::Gl::draw_octagon(float x, float y, float w, float h, float m)
{
  glBegin(GL_POLYGON);
  glVertex2f(x + m + w, y);
  glVertex2f(x + w + 2 * m, y + m);
  glVertex2f(x + w + 2 * m, y + h + m);
  glVertex2f(x + m + w, y + h + 2 * m);
  glVertex2f(x + m, y + h + 2 * m);
  glVertex2f(x, y + h + m);
  glVertex2f(x, y + m);
  glVertex2f(x + m, y);
  glEnd();
}

void Stg::Gl::draw_centered_rect(float x, float y, float dx, float dy)
{
  glRectf(x - 0.5 * dx, y - 0.5 * dy, x + 0.5 * dx, y + 0.5 * dy);
}

void Stg::Gl::draw_vector(double x, double y, double z)
{
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(x, y, z);
  glEnd();
}

void Stg::Gl::draw_origin(double len)
{
  draw_vector(len, 0, 0);
  draw_vector(0, len, 0);
  draw_vector(0, 0, len);
}

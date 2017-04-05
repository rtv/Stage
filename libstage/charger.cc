/** charger.cc
         Simple model of energy storage
         Richard Vaughan
         Created 2009.1.15
    SVN: $Id$
*/

#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

Charger::Charger(World *world) : world(world), watts(1000.0)
{
  // printf( "Charger constructed" );
}

void Charger::ChargeIfContained(PowerPack *pp, Pose pose)
{
  if (Contains(pose))
    Charge(pp);
}

bool Charger::Contains(Pose pose)
{
  return (pose.x >= volume.x.min && pose.x < volume.x.max && pose.y >= volume.y.min
          && pose.y < volume.y.max && pose.z >= volume.z.min && pose.z < volume.z.max);
}

void Charger::Charge(PowerPack *pp)
{
  double given = watts * world->interval_sim * 1e-6;

  pp->stored += given;

  // do not exceed capacity
  pp->stored = MIN(pp->stored, pp->capacity);
  pp->charging = true;
  /*
       printf( "charger %p  at [%.2f %.2f] [%.2f %.2f] [%.2f %.2f] gave pack %p
%.3f joules\n",
       this,
       volume.x.min,
       volume.x.max,
       volume.y.min,
       volume.y.max,
       volume.z.min,
       volume.z.max,
       pp, given );

pp->Print( "just charged" );
*/
}

void Charger::Visualize()
{
  glPushMatrix();
  glPolygonMode(GL_FRONT, GL_FILL);
  glColor4f(1, 0.5, 0, 0.4);
  glTranslatef(0, 0, volume.z.min);
  glRectf(volume.x.min, volume.y.min, volume.x.max, volume.y.max);

  glTranslatef(0, 0, volume.z.max);
  glRectf(volume.x.min, volume.y.min, volume.x.max, volume.y.max);
  glPopMatrix();

  glPushMatrix();
  glPolygonMode(GL_FRONT, GL_LINE);
  glColor4f(1, 0.5, 0, 0.8);
  glTranslatef(0, 0, volume.z.min);
  glRectf(volume.x.min, volume.y.min, volume.x.max, volume.y.max);

  glTranslatef(0, 0, volume.z.max);
  glRectf(volume.x.min, volume.y.min, volume.x.max, volume.y.max);
  glPopMatrix();

  // ?
  // glPolygonMode( GL_FRONT, GL_FILL );
}

void swap(double &a, double &b)
{
  double keep = a;
  a = b;
  b = keep;
}

void Charger::Load(Worldfile *wf, int entity)
{
  if (wf->PropertyExists(entity, "volume")) {
    volume.x.min = wf->ReadTupleLength(entity, "volume", 0, volume.x.min);
    volume.x.max = wf->ReadTupleLength(entity, "volume", 1, volume.x.max);
    volume.y.min = wf->ReadTupleLength(entity, "volume", 2, volume.y.min);
    volume.y.max = wf->ReadTupleLength(entity, "volume", 3, volume.y.max);
    volume.z.min = wf->ReadTupleLength(entity, "volume", 4, volume.z.min);
    volume.z.max = wf->ReadTupleLength(entity, "volume", 5, volume.z.max);

    // force the windings for GL's sake
    if (volume.x.min > volume.x.max)
      swap(volume.x.min, volume.x.max);

    if (volume.y.min > volume.y.max)
      swap(volume.y.min, volume.y.max);

    if (volume.z.min > volume.z.max)
      swap(volume.z.min, volume.z.max);
  }

  watts = wf->ReadFloat(entity, "watts", watts);
}

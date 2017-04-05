#include "region.hh"
#include "worldfile.hh"

using namespace Stg;
using std::vector;

static void canonicalize_winding(vector<point_t> &pts);

/** Create a new block. A model's body is a list of these
    blocks. The point data is copied, so pts can safely be freed
    after calling this.*/
Block::Block(BlockGroup *group, const std::vector<point_t> &pts, const Bounds &zrange)
    : group(group), pts(pts), local_z(zrange), global_z(), rendered_cells()
{
  assert(group);
  // canonicalize_winding(this->pts);
}

/** A from-file  constructor */
Block::Block(BlockGroup *group, Worldfile *wf, int entity)
    : group(group), pts(), local_z(), global_z(), rendered_cells()
{
  assert(group);
  assert(wf);
  assert(entity);

  Load(wf, entity);
}

Block::~Block()
{
  UnMap(0);
  UnMap(1);
}

void Block::Translate(double x, double y)
{
  FOR_EACH (it, pts) {
    it->x += x;
    it->y += y;
  }

  group->BuildDisplayList();
}

/** Return the value half way between the min and max Y position of
    the polygon points
 */
double Block::CenterY()
{
  // assume the polygon fits in a square a billion m a side
  double min = billion;
  double max = -billion;

  FOR_EACH (it, pts) {
    if (it->y > max)
      max = it->y;
    if (it->y < min)
      min = it->y;
  }

  // return the value half way between max and min
  return (min + (max - min) / 2.0);
}

/** Return the value half way between the min and max X position of
    the polygon points
 */
double Block::CenterX()
{
  // assume the polygon fits in a square a billion m a side
  double min = billion;
  double max = -billion;

  FOR_EACH (it, pts) {
    if (it->x > max)
      max = it->x;
    if (it->x < min)
      min = it->x;
  }

  // return the value half way between maxx and min
  return (min + (max - min) / 2.0);
}

/** move the block by the distance required to bring its center to
    the requested position */
void Block::SetCenter(double x, double y)
{
  Translate(x - CenterX(), y - CenterY());
}

/**  move the block by the distance required to bring its center to
     the requested position */
void Block::SetCenterY(double y)
{
  Translate(0, y - CenterY());
}

/** move the block by the distance required to bring its center to
   the requested position */
void Block::SetCenterX(double x)
{
  Translate(x - CenterX(), 0);
}

void Block::SetZ(double min, double max)
{
  local_z.min = min;
  local_z.max = max;

  // force redraw
  group->BuildDisplayList();
}

void Block::AppendTouchingModels(std::set<Model *> &touchers)
{
  unsigned int layer = group->mod.world->updates % 2;

  // for every cell we are rendered into
  FOR_EACH (cell_it, rendered_cells[layer])
    // for every block rendered into that cell
    FOR_EACH (block_it, (*cell_it)->GetBlocks(layer)) {
      if (!group->mod.IsRelated(&(*block_it)->group->mod))
        touchers.insert(&(*block_it)->group->mod);
    }
}

Model *Block::TestCollision()
{
  // printf( "model %s block %p test collision...\n", mod->Token(), this );

  // find the set of cells we would render into given the current global pose
  // GenerateCandidateCells();

  if (group->mod.vis.obstacle_return) {
    if (global_z.min < 0)
      return group->mod.world->GetGround();

    unsigned int layer = group->mod.world->updates % 2;

    // for every cell we may be rendered into
    FOR_EACH (cell_it, rendered_cells[layer]) {
      // for every block rendered into that cell
      FOR_EACH (block_it, (*cell_it)->GetBlocks(layer)) {
        Block *testblock = *block_it;
        Model *testmod = &testblock->group->mod;

        // printf( "   testing block %p of model %s\n", testblock,
        // testmod->Token() );

        // if the tested model is an obstacle and it's not attached to this
        // model
        if ((testmod != &group->mod) && testmod->vis.obstacle_return
            && (!group->mod.IsRelated(testmod)) &&
            // also must intersect in the Z range
            testblock->global_z.min <= global_z.max && testblock->global_z.max >= global_z.min) {
          // puts( "HIT");
          return testmod; // bail immediately with the bad news
        }
      }
    }
  }

  // printf( "model %s block %p collision done. no hits.\n", mod->Token(), this
  // );
  return NULL; // no hit
}

void Block::Map(unsigned int layer)
{
  // calculate the global pixel coords of the block vertices
  // and render this block's polygon into the world
  group->mod.world->MapPoly(group->mod.LocalToPixels(pts), this, layer);

  // update the block's absolute z bounds at this rendering
  Pose gpose(group->mod.GetGlobalPose());
  gpose.z += group->mod.geom.pose.z;
  global_z.min = local_z.min + gpose.z;
  global_z.max = local_z.max + gpose.z;
}

void Block::UnMap(unsigned int layer)
{
  FOR_EACH (it, rendered_cells[layer])
    (*it)->RemoveBlock(this, layer);

  rendered_cells[layer].clear();
}

void swap(int &a, int &b)
{
  int tmp = a;
  a = b;
  b = tmp;
}

void Block::Rasterize(uint8_t *data, unsigned int width, unsigned int height, meters_t cellwidth,
                      meters_t cellheight)
{
  // printf( "rasterize block %p : w: %u h: %u  scale %.2f %.2f  offset %.2f
  // %.2f\n",
  //	 this, width, height, scalex, scaley, offsetx, offsety );

  const size_t pt_count = pts.size();
  for (size_t i = 0; i < pt_count; ++i) {
    // convert points from local to model coords
    point_t mpt1 = pts[i]; // BlockPointToModelMeters( pts[i] );
    point_t mpt2 = pts[(i + 1) % pt_count]; // BlockPointToModelMeters( pts[(i+1)%pt_count] );

    // record for debug visualization
    group->mod.rastervis.AddPoint(mpt1.x, mpt1.y);

    // shift to the bottom left of the model
    mpt1.x += group->mod.geom.size.x / 2.0;
    mpt1.y += group->mod.geom.size.y / 2.0;
    mpt2.x += group->mod.geom.size.x / 2.0;
    mpt2.y += group->mod.geom.size.y / 2.0;

    // convert from meters to cells
    point_int_t a(floor(mpt1.x / cellwidth), floor(mpt1.y / cellheight));
    point_int_t b(floor(mpt2.x / cellwidth), floor(mpt2.y / cellheight));

    // render a line in the output bitmap for this edge, from mpt1
    // to mpt2
    bool steep = abs(b.y - a.y) > abs(b.x - a.x);
    if (steep) {
      swap(a.x, a.y);
      swap(b.x, b.y);
    }

    if (a.x > b.x) {
      swap(a.x, b.x);
      swap(a.y, b.y);
    }

    double dydx = (double)(b.y - a.y) / (double)(b.x - a.x);
    double y = a.y;
    for (int x = a.x; x <= b.x; ++x) {
      if (steep) {
        if (!(floor(y) >= 0))
          continue;
        if (!(floor(y) < (int)width))
          continue;
        if (!(x >= 0))
          continue;
        if (!(x < (int)height))
          continue;
      } else {
        if (!(x >= 0))
          continue;
        if (!(x < (int)width))
          continue;
        if (!(floor(y) >= 0))
          continue;
        if (!(floor(y) < (int)height))
          continue;
      }

      if (steep)
        data[(int)floor(y) + (x * width)] = 1;
      else
        data[x + ((int)floor(y) * width)] = 1;
      y += dydx;
    }
  }
}

void Block::DrawTop()
{
  // draw the top of the block - a polygon at the highest vertical
  // extent

  glBegin(GL_POLYGON);
  FOR_EACH (it, pts)
    glVertex3f(it->x, it->y, local_z.max);
  glEnd();
}

void Block::DrawSides()
{
  // construct a strip that wraps around the polygon
  glBegin(GL_QUAD_STRIP);

  FOR_EACH (it, pts) {
    glVertex3f(it->x, it->y, local_z.max);
    glVertex3f(it->x, it->y, local_z.min);
  }
  // close the strip
  glVertex3f(pts[0].x, pts[0].y, local_z.max);
  glVertex3f(pts[0].x, pts[0].y, local_z.min);
  glEnd();
}

void Block::DrawFootPrint()
{
  glBegin(GL_POLYGON);
  FOR_EACH (it, pts)
    glVertex2f(it->x, it->y);
  glEnd();
}

void Block::DrawSolid(bool)
{
  DrawSides();
  DrawTop();
}

void Block::Load(Worldfile *wf, int entity)
{
  const size_t pt_count = wf->ReadInt(entity, "points", 0);

  char key[256];
  for (size_t p = 0; p < pt_count; ++p) {
    snprintf(key, sizeof(key), "point[%d]", (int)p);

    point_t pt(0, 0);
    wf->ReadTuple(entity, key, 0, 2, "ll", &pt.x, &pt.y);
    pts.push_back(pt);
  }

  canonicalize_winding(pts);

  wf->ReadTuple(entity, "z", 0, 2, "ll", &local_z.min, &local_z.max);
}

/////////////////////////////////////////////////////////////////////////////////////////
// utility functions to ensure block winding is consistent and matches OpenGL's
// default

static
    /// util; puts angle into [0, 2pi)
    void
    positivize(radians_t &angle)
{
  while (angle < 0)
    angle += 2 * M_PI;
}

static
    /// util; puts angle into -pi/2, pi/2
    void
    pi_ize(radians_t &angle)
{
  while (angle < -M_PI)
    angle += 2 * M_PI;
  while (M_PI < angle)
    angle -= 2 * M_PI;
}

static
    /// util; How much was v1 rotated to get to v2?
    radians_t
    angle_change(point_t v1, point_t v2)
{
  radians_t a1 = atan2(v1.y, v1.x);
  positivize(a1);

  radians_t a2 = atan2(v2.y, v2.x);
  positivize(a2);

  radians_t angle_change = a2 - a1;
  pi_ize(angle_change);

  return angle_change;
}

static
    /// util; find vectors between adjacent points, pts[next] - pts[cur]
    vector<point_t>
    find_vectors(vector<point_t> const &pts)
{
  vector<point_t> vs;
  assert(2 <= pts.size());
  for (unsigned i = 0, n = pts.size(); i < n; ++i) {
    unsigned j = (i + 1) % n;
    vs.push_back(point_t(pts[j].x - pts[i].x, pts[j].y - pts[i].y));
  }
  assert(vs.size() == pts.size());
  return vs;
}

static
    /// util; finds sum of angle changes, from each vertex to the
    /// next one (in current ordering), wrapping around.
    radians_t
    angles_sum(vector<point_t> const &vs)
{
  radians_t angle_sum = 0;
  for (unsigned i = 0, n = vs.size(); i < n; ++i) {
    unsigned j = (i + 1) % n;
    angle_sum += angle_change(vs[i], vs[j]);
  }
  return angle_sum;
}

static
    /// Util
    bool
    is_canonical_winding(vector<point_t> const &ps)
{
  return (0 < angles_sum(find_vectors(ps)));
}

static
    /// util; sums angle changes to see whether it's 2pi or -2pi.
    /// 2pi is counter-clockwise winding (which OpenGL requires),
    /// -2pi is clockwise. Reverses <pts> when winding is clockwise.
    // Note that a simple line that doubles back on itself has an
    // angle sum of 0, but that's intrinsic to a line - its winding could
    // be either way.
    void
    canonicalize_winding(vector<point_t> &ps)
{
  if (not is_canonical_winding(ps)) {
    std::reverse(ps.begin(), ps.end());
  }
}

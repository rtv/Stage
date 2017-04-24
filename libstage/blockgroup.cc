
#include "stage.hh"
#include "worldfile.hh"

#include <cmath>
#include <libgen.h> // for dirname(3)
#include <limits.h> // for _POSIX_PATH_MAX
#include <limits>

using namespace Stg;
using namespace std;

BlockGroup::BlockGroup(Model &mod) : blocks(), displaylist(0), mod(mod)
{ /* empty */
}

BlockGroup::~BlockGroup()
{
  Clear();
}

void BlockGroup::AppendBlock(const Block &block)
{
  blocks.push_back(block);
}

void BlockGroup::Clear()
{
  // FOR_EACH( it, blocks )
  // delete *it;

  blocks.clear();
}

void BlockGroup::AppendTouchingModels(std::set<Model *> &v)
{
  FOR_EACH (it, blocks)
    it->AppendTouchingModels(v);
}

Model *BlockGroup::TestCollision()
{
  Model *hitmod = NULL;

  FOR_EACH (it, blocks)
    if ((hitmod = it->TestCollision()))
      break; // bail on the earliest collision

  return hitmod; // NULL if no collision
}

/** find the 3d bounding box of all the blocks in the group */
bounds3d_t BlockGroup::BoundingBox() const
{
  // assuming the blocks currently fit in a square +/- one billion units
  double minx, miny, maxx, maxy, minz, maxz;
  minx = miny = minz = billion;
  maxx = maxy = maxz = -billion;

  FOR_EACH (it, blocks) {
    // examine all the points in the polygon
    FOR_EACH (pit, it->pts) {
      if (pit->x < minx)
        minx = pit->x;
      if (pit->y < miny)
        miny = pit->y;
      if (pit->x > maxx)
        maxx = pit->x;
      if (pit->y > maxy)
        maxy = pit->y;
    }

    if (it->local_z.min < minz)
      minz = it->local_z.min;
    if (it->local_z.max > maxz)
      maxz = it->local_z.max;
  }

  return bounds3d_t(Bounds(minx, maxx), Bounds(miny, maxy), Bounds(minz, maxz));
}

// scale all blocks to fit into the bounding box of this group's model
void BlockGroup::CalcSize()
{
  const bounds3d_t b = BoundingBox();

// Prevents creating (-)NaNs when dividing by size.{x,y,z}:
#define ENSURE_NOT_ZERO(x)                                                                         \
  ((std::fabs(x) >= std::numeric_limits<double>::epsilon())                                        \
       ? (x)                                                                                       \
       : std::numeric_limits<double>::epsilon())
  const Size size(ENSURE_NOT_ZERO(b.x.max - b.x.min), ENSURE_NOT_ZERO(b.y.max - b.y.min),
                  ENSURE_NOT_ZERO(b.z.max - b.z.min));

  const Size offset(b.x.min + size.x / 2.0, b.y.min + size.y / 2.0, 0);

  // now scale the blocks to fit in the model's 3d bounding box, so
  // that the original points are now in model coordinates
  const Size modsize = mod.geom.size;

  FOR_EACH (it, blocks) {
    // polygon edges
    FOR_EACH (pit, it->pts) {
      pit->x = (pit->x - offset.x) * (modsize.x / size.x);
      pit->y = (pit->y - offset.y) * (modsize.y / size.y);
    }

    // vertical bounds
    it->local_z.min = (it->local_z.min - offset.z) * (modsize.z / size.z);
    it->local_z.max = (it->local_z.max - offset.z) * (modsize.z / size.z);
  }
}

void BlockGroup::Map(unsigned int layer)
{
  //static size_t count = 0;
 //printf( "BlockGroup::Map %lu (%lu)\n", ++count, blocks.size() );
    
  FOR_EACH (it, blocks)
      it->Map(layer);
}

void BlockGroup::UnMap(unsigned int layer)
{
  //static size_t count = 0;
  //printf( "BlockGroup::UnMap %lu (%lu)\n", ++count, blocks.size() );
  
  FOR_EACH (it, blocks)
    it->UnMap(layer);


}

void BlockGroup::DrawSolid(const Geom &geom)
{
  glPushMatrix();

  Gl::pose_shift(geom.pose);

  FOR_EACH (it, blocks)
    it->DrawSolid(false);

  glPopMatrix();
}

void BlockGroup::DrawFootPrint(const Geom &)
{
  FOR_EACH (it, blocks)
    it->DrawFootPrint();
}

// tesselation callbacks used in BlockGroup::BuildDisplayListTess()------------

static void errorCallback(GLenum errorCode)
{
  const GLubyte *estring;

  estring = gluErrorString(errorCode);
  fprintf(stderr, "Tessellation Error: %s\n", estring);
  exit(0);
}

static void combineCallback(GLdouble coords[3],
                            GLdouble **, // GLdouble *vertex_data[4],
                            GLfloat *, // GLfloat weight[4],
                            GLdouble **new_vertex)
{
  *new_vertex = new GLdouble[3];
  memcpy(*new_vertex, coords, 3 * sizeof(GLdouble));

  // @todo: fix the leak of this vertex buffer. it's not a lot of
  // data, and doesn't happen much, but it would be tidy.
}

// render each block as a polygon extruded into Z
void BlockGroup::BuildDisplayList()
{
  static GLUtesselator *tobj = NULL;

  if (!mod.world->IsGUI())
    return;

  if (displaylist == 0) {
    CalcSize(); // todo: is this redundant? count calls per model to figure this
    // out.

    displaylist = glGenLists(1);
    assert(displaylist != 0);

    // Stage polygons need not be convex, so we have to tesselate them for
    // rendering in OpenGL.
    tobj = gluNewTess();
    assert(tobj != NULL);

    // these use the standard GL calls
    gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid(*)()) & glVertex3dv);
    gluTessCallback(tobj, GLU_TESS_EDGE_FLAG, (GLvoid(*)()) & glEdgeFlag);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid(*)()) & glBegin);
    gluTessCallback(tobj, GLU_TESS_END, (GLvoid(*)()) & glEnd);

    // these are custom, defined above.
    gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid(*)()) & errorCallback);
    gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid(*)()) & combineCallback);
  }

  std::vector<std::vector<GLdouble> > contours;

  FOR_EACH (blk, blocks) {
    std::vector<GLdouble> verts;
    FOR_EACH (it, blk->pts) {
      verts.push_back(it->x);
      verts.push_back(it->y);
      verts.push_back(blk->local_z.max);
    }
    contours.push_back(verts);
  }

  glNewList(displaylist, GL_COMPILE);

  Gl::pose_shift(mod.GetGeom().pose);

  // draw filled polys
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(0.5, 0.5);

  mod.PushColor(mod.color);

  gluTessBeginPolygon(tobj, NULL);

  FOR_EACH (contour, contours) {
    gluTessBeginContour(tobj);
    for (size_t v = 0; v < contour->size(); v += 3)
      gluTessVertex(tobj, &(*contour)[v], &(*contour)[v]);
    gluTessEndContour(tobj);
  }

  gluTessEndPolygon(tobj);

  FOR_EACH (blk, blocks)
    blk->DrawSides();

  mod.PopColor();

  // now outline the polys
  glDisable(GL_POLYGON_OFFSET_FILL);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDepthMask(GL_FALSE);

  Color c = mod.color;
  c.r /= 2.0;
  c.g /= 2.0;
  c.b /= 2.0;
  mod.PushColor(c);

  gluTessBeginPolygon(tobj, NULL);

  FOR_EACH (contour, contours) {
    gluTessBeginContour(tobj);
    for (size_t v = 0; v < contour->size(); v += 3)
      gluTessVertex(tobj, &(*contour)[v], &(*contour)[v]);
    gluTessEndContour(tobj);
  }

  gluTessEndPolygon(tobj);

  FOR_EACH (blk, blocks)
    blk->DrawSides();

  glDepthMask(GL_TRUE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  mod.PopColor();

  glEndList();
}

void BlockGroup::CallDisplayList()
{
  if (displaylist == 0 || mod.rebuild_displaylist) {
    BuildDisplayList();
    mod.rebuild_displaylist = 0;
  }

  glCallList(displaylist);
}

void BlockGroup::LoadBlock(Worldfile *wf, int entity)
{
  AppendBlock(Block(this, wf, entity));
  // CalcSize(); // adjust the blocks so they fit in our bounding box
}

void BlockGroup::LoadBitmap(const std::string &bitmapfile, Worldfile *wf)
{
  PRINT_DEBUG1("attempting to load bitmap \"%s\n", bitmapfile.c_str());

  std::string full;

  if (bitmapfile[0] == '/')
    full = bitmapfile;
  else {
    char *workaround_const = strdup(wf->filename.c_str());
    full = std::string(dirname(workaround_const)) + "/" + bitmapfile;
    free(workaround_const);
  }

  char buf[512];
  snprintf(buf, 512, "[Image \"%s\"", bitmapfile.c_str());
  fputs(buf, stdout);
  fflush(stdout);

  PRINT_DEBUG1("attempting to load image %s", full.c_str());

  Color col(1.0, 0.0, 1.0, 1.0);

  std::vector<std::vector<point_t> > polys;

  if (polys_from_image_file(full, polys)) {
    PRINT_ERR1("failed to load polys from image file \"%s\"", full.c_str());
    return;
  }

  FOR_EACH (it, polys)
    AppendBlock(Block(this, *it, Bounds(0, 1)));

  CalcSize();

  fputs("]", stdout);
}

void BlockGroup::Rasterize(uint8_t *data, unsigned int width, unsigned int height,
                           meters_t cellwidth, meters_t cellheight)
{
  FOR_EACH (it, blocks)
    it->Rasterize(data, width, height, cellwidth, cellheight);
}

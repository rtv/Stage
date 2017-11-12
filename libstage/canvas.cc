
/** canvas.cc
    Implement the main world viewing area in FLTK and OpenGL.

    Authors:
    Richard Vaughan (vaughan@sfu.ca)
    Alex Couture-Beil (asc17@sfu.ca)
    Jeremy Asher (jra11@sfu.ca)

    $Id$
*/

#include "canvas.hh"
#include "replace.h"
#include "stage.hh"
#include "texture_manager.hh"
#include "worldfile.hh"

#include <algorithm>
#include <functional> // For greater<int>( )
#include <sstream>
#include <string>

#include "file_manager.hh"
#include "options_dlg.hh"
#include "region.hh"

using namespace Stg;

static const int checkImageWidth = 2;
static const int checkImageHeight = 2;
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];
static bool blur = true;

// GLuint glowTex;
GLuint checkTex;

Canvas::Canvas(World *world, int x, int y, int width, int height)
    : colorstack(), models_sorted(), current_camera(NULL),
      camera(), perspective_camera(), dirty_buffer(false), wf(NULL),
      selected_models(), last_selection(NULL), interval(40), // msec between redraws
      init_done(false), texture_load_done(false),
      // initialize Option objects
      //  showBlinken( "Blinkenlights", "show_blinkenlights", "", true, world ),
      showBBoxes("Debug/Bounding boxes", "show_boundingboxes", "^b", false),
      showBlocks("Blocks", "show_blocks", "b", true),
      showBlur("Trails/Blur", "show_trailblur", "^d", false),
      showClock("Clock", "show_clock", "c", true),
      showData("Data", "show_data", "d", false),
      showFlags("Flags", "show_flags", "l", true),
      showFollow("Follow", "show_follow", "f", false),
      showFootprints("Footprints", "show_footprints", "o", false),
      showGrid("Grid", "show_grid", "g", true),
      showOccupancy("Debug/Occupancy", "show_occupancy", "^o", false),
      showStatus("Status", "show_status", "s", true),
      showTrailArrows("Trails/Rising Arrows", "show_trailarrows", "^a", false),
      showTrailRise("Trails/Rising blocks", "show_trailrise", "^r", false),
      showTrails("Trails/Fast", "show_trailfast", "^f", false),
      showVoxels("Debug/Voxels", "show_voxels", "^v", false),
      pCamOn("Perspective camera", "pcam_on", "r", false),
      visualizeAll("Selected only", "vis_all", "v", false),
      // and the rest
      graphics(true), world(world), frames_rendered_count(0)
{
  perspective_camera.setPose(0.0, -4.0, 3.0);
  current_camera = &camera;
  setDirtyBuffer();

  // enable accumulation buffer
  // mode( mode() | FL_ACCUM );
  // assert( can_do( FL_ACCUM ) );
}

void Canvas::InitGl()
{
  //valid(1);
  FixViewport(getWidth(), getHeight());

  // set gl state that won't change every redraw
  glClearColor(0.7, 0.7, 0.8, 1.0);
  glDisable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  // culling disables text drawing on OS X, so I've disabled it until I
  // understand why
  // glCullFace( GL_BACK );
  // glEnable (GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
  glDepthMask(GL_TRUE);
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_VERTEX_ARRAY);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  blur = false;

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Canvas::InitTextures()
{
  // load textures
  std::string fullpath = FileManager::findFile("assets/stall.png");
  if (fullpath == "") {
    PRINT_DEBUG("Unable to load stall texture.\n");
  }

  // printf( "stall icon %s\n", fullpath.c_str() );

  GLuint stall_id = TextureManager::getInstance().loadTexture(fullpath.c_str());
  TextureManager::getInstance()._stall_texture_id = stall_id;

  fullpath = FileManager::findFile("assets/mainspower.png");
  if (fullpath == "") {
    PRINT_DEBUG("Unable to load mains texture.\n");
  }

  //	printf( "mains icon %s\n", fullpath.c_str() );

  GLuint mains_id = TextureManager::getInstance().loadTexture(fullpath.c_str());
  TextureManager::getInstance()._mains_texture_id = mains_id;

  //   // generate a small glow texture
  //   GLubyte* pixels = new GLubyte[ 4 * 128 * 128 ];

  //   for( int x=0; x<128; x++ )
  //  	 for( int y=0; y<128; y++ )
  //  		{
  //  		  GLubyte* p = &pixels[ 4 * (128*y + x)];
  //  		  p[0] = (GLubyte)255; // red
  //  		  p[1] = (GLubyte)0; // green
  //  		  p[2] = (GLubyte)0; // blue
  //  		  p[3] = (GLubyte)128; // alpha
  //  		}

  //   glGenTextures(1, &glowTex );
  //   glBindTexture( GL_TEXTURE_2D, glowTex );

  //   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0,
  // 					 GL_RGBA, GL_UNSIGNED_BYTE, pixels );

  //   delete[] pixels;

  // draw a check into a bitmap, then load that into a texture
  int i, j;
  for (i = 0; i < checkImageHeight; i++)
    for (j = 0; j < checkImageWidth; j++) {
      int even = (i + j) % 2;
      checkImage[i][j][0] = (GLubyte)255 - 10 * even;
      checkImage[i][j][1] = (GLubyte)255 - 10 * even;
      checkImage[i][j][2] = (GLubyte)255; // - 5*even;
      checkImage[i][j][3] = 255;
    }

  glGenTextures(1, &checkTex);
  glBindTexture(GL_TEXTURE_2D, checkTex);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, checkImage);
}

Canvas::~Canvas()
{
  // nothing to do
}

Model *Canvas::getModel(int x, int y)
{
  // render all models in a unique color
	// Why we are calling make_current here? This should be done on upper level
  //make_current(); // make sure the GL context is current
  glClearColor(1, 1, 1, 1); // white
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  current_camera->SetProjection();
  current_camera->Draw();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_DITHER);
  glDisable(GL_BLEND); // turns off alpha blending, so we read back
  // exactly what we write to a pixel

  // render all top-level, draggable models in a color that is their
  // id
  FOR_EACH (it, world->World::children) {
    Model *mod = (*it);

    if (mod->gui.move) {
      uint8_t rByte, gByte, bByte, aByte;
      uint32_t modelId = mod->id;
      rByte = modelId;
      gByte = modelId >> 8;
      bByte = modelId >> 16;
      aByte = modelId >> 24;

      // printf("mod->Id(): 0x%X, rByte: 0x%X, gByte: 0x%X, bByte: 0x%X, aByte:
      // 0x%X\n", modelId, rByte, gByte, bByte, aByte);

      glColor4ub(rByte, gByte, bByte, aByte);
      mod->DrawPicker();
    }
  }

  glFlush(); // make sure the drawing is done

  // read the color of the pixel in the back buffer under the mouse
  // pointer
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  uint8_t rgbaByte[4];
  uint32_t modelId;

  glReadPixels(x, viewport[3] - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &rgbaByte[0]);

  modelId = rgbaByte[0];
  modelId |= rgbaByte[1] << 8;
  modelId |= rgbaByte[2] << 16;
  // modelId |= rgbaByte[3] << 24;

  //	printf("Clicked rByte: 0x%X, gByte: 0x%X, bByte: 0x%X, aByte: 0x%X\n",
  // rgbaByte[0], rgbaByte[1], rgbaByte[2], rgbaByte[3]);
  //	printf("-->model Id = 0x%X\n", modelId);

  Model *mod = Model::LookupId(modelId);

  // printf("%p %s %d %x\n", mod, mod ? mod->Token() : "(none)", modelId,
  // modelId );

  // put things back the way we found them
  glEnable(GL_DITHER);
  glEnable(GL_BLEND);
  glClearColor(0.7, 0.7, 0.8, 1.0);

  // useful for debugging the picker
  // Screenshot();

  return mod;
}

bool Canvas::selected(Model *mod)
{
  return (std::find(selected_models.begin(), selected_models.end(), mod) != selected_models.end());
}

void Canvas::select(Model *mod)
{
  if (mod) {
    last_selection = mod;
    selected_models.push_front(mod);

    //		mod->Disable();
    doRedraw();
  }
}

void Canvas::unSelect(Model *mod)
{
  if (mod) {
    EraseAll(mod, selected_models);
    doRedraw();
  }
}

void Canvas::unSelectAll()
{
  selected_models.clear();
}

// convert from 2d window pixel to 3d world coordinates
void Canvas::CanvasToWorld(int px, int py, double *wx, double *wy, double *wz)
{
  if (px <= 0)
    px = 1;
  else if (px >= getWidth())
    px = getWidth() - 1;
  if (py <= 0)
    py = 1;
  else if (py >= getHeight())
    py = getHeight() - 1;

  // redraw the screen only if the camera model isn't active.
  // TODO new selection technique will simply use drawfloor to result in z = 0
  // always and prevent strange behaviours near walls
  // TODO refactor, so glReadPixels reads (then caches) the whole screen only
  // when the camera changes.
  if (true || dirtyBuffer()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    current_camera->SetProjection();
    current_camera->Draw();
    DrawFloor(); // call this rather than renderFrame for speed - this won't
    // give correct z values
    dirty_buffer = false;
  }

  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

  GLdouble projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  GLfloat pz;
  glReadPixels(px, getHeight() - py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pz);
  gluUnProject(px, getWidth() - py, pz, modelview, projection, viewport, wx, wy, wz);
}

void Canvas::FixViewport(int W, int H)
{
  glLoadIdentity();
  glViewport(0, 0, W, H);
}

void Canvas::AddModel(Model *mod)
{
  models_sorted.push_back(mod);
  doRedraw();
}

void Canvas::RemoveModel(Model *mod)
{
  printf("removing model %s from canvas list\n", mod->Token());
  EraseAll(mod, models_sorted);
}

void Canvas::DrawGlobalGrid()
{
  bounds3d_t bounds = world->GetExtent();

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(2.0, 2.0);
  glDisable(GL_BLEND);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, checkTex);
  glColor3f(1.0, 1.0, 1.0);

  glBegin(GL_QUADS);
  glTexCoord2f(bounds.x.min / 2.0, bounds.y.min / 2.0);
  glVertex2f(bounds.x.min, bounds.y.min);
  glTexCoord2f(bounds.x.max / 2.0, bounds.y.min / 2.0);
  glVertex2f(bounds.x.max, bounds.y.min);
  glTexCoord2f(bounds.x.max / 2.0, bounds.y.max / 2.0);
  glVertex2f(bounds.x.max, bounds.y.max);
  glTexCoord2f(bounds.x.min / 2.0, bounds.y.max / 2.0);
  glVertex2f(bounds.x.min, bounds.y.max);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);

  glDisable(GL_POLYGON_OFFSET_FILL);

  /*   printf( "bounds [%.2f %.2f] [%.2f %.2f] [%.2f %.2f]\n",
     bounds.x.min, bounds.x.max,
     bounds.y.min, bounds.y.max,
     bounds.z.min, bounds.z.max );

*/

  /* simple scaling of axis labels - could be better */
  int skip = (int)(50 / camera.scale());
  if (skip < 1)
    skip = 1;
  if (skip > 2 && skip % 2)
    skip += 1;

  // printf( "scale %.4f\n", camera.scale() );

  char str[64];
  PushColor(0.2, 0.2, 0.2, 1.0); // pale gray

  for (double i = 0; i < bounds.x.max; i += skip) {
    snprintf(str, 16, "%d", (int)i);
    this->draw_string(i, 0, 0, str);
  }

  for (double i = 0; i >= bounds.x.min; i -= skip) {
    snprintf(str, 16, "%d", (int)i);
    this->draw_string(i, 0, 0, str);
  }

  for (double i = 0; i < bounds.y.max; i += skip) {
    snprintf(str, 16, "%d", (int)i);
    this->draw_string(0, i, 0, str);
  }

  for (double i = 0; i >= bounds.y.min; i -= skip) {
    snprintf(str, 16, "%d", (int)i);
    this->draw_string(0, i, 0, str);
  }

  PopColor();
}

// draw the floor without any grid ( for robot's perspective camera model )
void Canvas::DrawFloor()
{
  bounds3d_t bounds = world->GetExtent();

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(2.0, 2.0);

  glColor4f(1.0, 1.0, 1.0, 1.0);

  glBegin(GL_QUADS);
  glVertex2f(bounds.x.min, bounds.y.min);
  glVertex2f(bounds.x.max, bounds.y.min);
  glVertex2f(bounds.x.max, bounds.y.max);
  glVertex2f(bounds.x.min, bounds.y.max);
  glEnd();
}

void Canvas::DrawBlocks()
{
  FOR_EACH (it, models_sorted)
    (*it)->DrawBlocksTree();
}

void Canvas::DrawBoundingBoxes()
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth(2.0);
  glPointSize(5.0);
  //glDisable(GL_CULL_FACE);
  const std::vector<Model *> & children = world->GetChildren();
  FOR_EACH (it, children)
      (*it)->DrawBoundingBoxTree();

  //glEnable(GL_CULL_FACE);
  glLineWidth(1.0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Canvas::resetCamera()
{
  float max_x = 0, max_y = 0, min_x = 0, min_y = 0;

  // TODO take orrientation ( `a' ) and geom.pose offset into consideration
  FOR_EACH (it, world->World::children) {
    Model *ptr = (*it);
    Pose pose = ptr->GetPose();
    Geom geom = ptr->GetGeom();

    float tmp_min_x = pose.x - geom.size.x / 2.0;
    float tmp_max_x = pose.x + geom.size.x / 2.0;
    float tmp_min_y = pose.y - geom.size.y / 2.0;
    float tmp_max_y = pose.y + geom.size.y / 2.0;

    if (tmp_min_x < min_x)
      min_x = tmp_min_x;
    if (tmp_max_x > max_x)
      max_x = tmp_max_x;
    if (tmp_min_y < min_y)
      min_y = tmp_min_y;
    if (tmp_max_y > max_y)
      max_y = tmp_max_y;
  }

  // do a complete reset
  float x = (min_x + max_x) / 2.0;
  float y = (min_y + max_y) / 2.0;
  camera.setPose(x, y);
  float scale_x = getWidth() / (max_x - min_x) * 0.9;
  float scale_y = getHeight() / (max_y - min_y) * 0.9;
  camera.setScale(scale_x < scale_y ? scale_x : scale_y);

  // TODO reset perspective cam
}

class DistFuncObj {
  meters_t x, y;
  DistFuncObj(meters_t x, meters_t y) : x(x), y(y) {}
  bool operator()(const Model *a, const Model *b) const
  {
    const Pose a_pose = a->GetGlobalPose();
    const Pose b_pose = b->GetGlobalPose();

    const meters_t a_dist = hypot(y - a_pose.y, x - a_pose.x);
    const meters_t b_dist = hypot(y - b_pose.y, x - b_pose.x);

    return (a_dist < b_dist);
  }
};

void Canvas::renderFrame()
{
  // before drawing, order all models based on distance from camera
  // float x = current_camera->x();
  // float y = current_camera->y();
  // float sphi = -dtor( current_camera->yaw() );

  // estimate point of camera location - hard to do with orthogonal mode
  // x += -sin( sphi ) * 100;
  // y += -cos( sphi ) * 100;

  // double coords[2];
  // coords[0] = x;
  // coords[1] = y;

  // sort the list of models by inverse distance from the camera -
  // probably doesn't change too much between frames so this is
  // usually fast
  // TODO
  // models_sorted = g_list_sort_with_data( models_sorted,
  // (GCompareDataFunc)compare_distance, coords );

  // TODO: understand why this doesn't work and fix it - cosmetic but important!
  // std::sort( models_sorted.begin(), models_sorted.end(), DistFuncObj(x,y) );

  glEnable(GL_DEPTH_TEST);

  if (!showTrails)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (showOccupancy)
    world->DrawOccupancy();

  if (showVoxels)
    world->DrawVoxels();

  if (!world->rt_cells.empty()) {
    glPushMatrix();
    GLfloat scale = 1.0 / world->Resolution();
    glScalef(scale, scale, 1.0); // XX TODO - this seems slightly

    world->PushColor(Color(0, 0, 1, 0.5));

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glPointSize(2);
    glBegin(GL_POINTS);

    for (unsigned int i = 0; i < world->rt_cells.size(); i++) {
      char str[128];
      snprintf(str, 128, "(%d,%d)", world->rt_cells[i].x, world->rt_cells[i].y);

      this->draw_string(world->rt_cells[i].x + 1, world->rt_cells[i].y + 1, 0.1, str);

      // printf( "x: %d y: %d\n", world->rt_regions[i].x, world->rt_regions[i].y
      // );
      // glRectf( world->rt_cells[i].x+0.3, world->rt_cells[i].y+0.3,
      //	 world->rt_cells[i].x+0.7, world->rt_cells[i].y+0.7 );

      glVertex2f(world->rt_cells[i].x, world->rt_cells[i].y);
    }

    glEnd();

#if 1
    world->PushColor(Color(0, 1, 0, 0.2));
    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i < world->rt_cells.size(); i++) {
      glVertex2f(world->rt_cells[i].x + 0.5, world->rt_cells[i].y + 0.5);
    }
    glEnd();
    world->PopColor();
#endif

    glPopMatrix();
    world->PopColor();
  }

  if (!world->rt_candidate_cells.empty()) {
    glPushMatrix();
    GLfloat scale = 1.0 / world->Resolution();
    glScalef(scale, scale, 1.0); // XX TODO - this seems slightly

    world->PushColor(Color(1, 0, 0, 0.5));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (unsigned int i = 0; i < world->rt_candidate_cells.size(); i++) {
      // 			 char str[128];
      // 			 snprintf( str, 128, "(%d,%d)",
      // 						  world->rt_candidate_cells[i].x,
      // 						  world->rt_candidate_cells[i].y
      // );

      // 			 Gl::draw_string(
      // world->rt_candidate_cells[i].x+1,
      // 									 world->rt_candidate_cells[i].y+1,
      // 0.1, str );

      // printf( "x: %d y: %d\n", world->rt_regions[i].x, world->rt_regions[i].y
      // );
      glRectf(world->rt_candidate_cells[i].x, world->rt_candidate_cells[i].y,
              world->rt_candidate_cells[i].x + 1, world->rt_candidate_cells[i].y + 1);
    }

    world->PushColor(Color(0, 1, 0, 0.2));
    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i < world->rt_candidate_cells.size(); i++) {
      glVertex2f(world->rt_candidate_cells[i].x + 0.5, world->rt_candidate_cells[i].y + 0.5);
    }
    glEnd();
    world->PopColor();

    glPopMatrix();
    world->PopColor();

    // world->rt_cells.clear();
  }

  if (showGrid)
    DrawGlobalGrid();
  else
    DrawFloor();

  if (showFootprints) {
    glDisable(GL_DEPTH_TEST); // using alpha blending

    FOR_EACH (it, models_sorted)
      (*it)->DrawTrailFootprint();

    glEnable(GL_DEPTH_TEST);
  }

  if (showFlags) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);

    FOR_EACH (it, models_sorted)
      (*it)->DrawFlagList();

    glEnd();
  }

  if (showTrailArrows)
    FOR_EACH (it, models_sorted)
      (*it)->DrawTrailArrows();

  if (showTrailRise)
    FOR_EACH (it, models_sorted)
      (*it)->DrawTrailBlocks();

  if (showBlocks)
    DrawBlocks();

  if (showBBoxes)
    DrawBoundingBoxes();

  // TODO - finish this properly
  // LISTMETHOD( models_sorted, Model*, DrawWaypoints );

  FOR_EACH (it, selected_models)
    (*it)->DrawSelected(this);

  // useful debug - puts a point at the origin of each model
  // for( GList* it = world->World::children; it; it=it->next )
  // ((Model*)it->data)->DrawOriginTree();

  // draw the model-specific visualizations
  if (world->sim_time > 0) {
    if (showData) {
      if (!visualizeAll) {
        FOR_EACH (it, world->World::children)
          (*it)->DataVisualizeTree(current_camera, this);
      } else if (selected_models.size() > 0) {
        FOR_EACH (it, selected_models)
          (*it)->DataVisualizeTree(current_camera, this);
      } else if (last_selection) {
        last_selection->DataVisualizeTree(current_camera, this);
      }
    }
  }

  if (showGrid)
    FOR_EACH (it, models_sorted)
      (*it)->DrawGrid(this);

  if (showStatus) {
    glPushMatrix();
    // ensure two icons can't be in the exact same plane
    if (camera.pitch() == 0 && !pCamOn)
      glTranslatef(0, 0, 0.1);

    FOR_EACH (it, models_sorted)
      (*it)->DrawStatusTree(&camera, this);

    glPopMatrix();
  }

  if (world->ray_list.size() > 0) {
    glDisable(GL_DEPTH_TEST);
    PushColor(0, 0, 0, 0.5);
    FOR_EACH (it, world->ray_list) {
      float *pts = *it;
      glBegin(GL_LINES);
      glVertex2f(pts[0], pts[1]);
      glVertex2f(pts[2], pts[3]);
      glEnd();
    }
    PopColor();
    glEnable(GL_DEPTH_TEST);

    world->ClearRays();
  }

  if (showClock) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // use orthogonal projeciton without any zoom
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); // save old projection
    glLoadIdentity();
    glOrtho(0, getWidth(), 0, getHeight(), -100, 100);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    std::string clockstr = world->ClockString();
    if (showFollow == true && last_selection)
      clockstr.append(" [FOLLOW MODE]");

    float txtWidth = this->fontWidth(clockstr.c_str());
    if (txtWidth < 200)
      txtWidth = 200;
    int txtHeight = this->fontHeight();

    const int margin = 5;
    int width, height;
    width = txtWidth + 2 * margin;
    height = txtHeight + 2 * margin;

    // TIME BOX
    colorstack.Push(0.8, 0.8, 1.0); // pale blue
    glRectf(0, 0, width, height);
    colorstack.Push(0, 0, 0); // black

    //char buf[ clockstr.size() ];
    //strcpy( buf, clockstr.c_str() );

    this->draw_string(margin, margin, 0, clockstr.c_str() );
    colorstack.Pop();
    colorstack.Pop();

    // ENERGY BOX
    // if (PowerPack::global_capacity > 0) {
    //   colorstack.Push(0.8, 1.0, 0.8, 0.85); // pale green
    //   glRectf(0, height, width, 90);
    //   colorstack.Push(0, 0, 0); // black
    //   Gl::draw_string_multiline(margin, height + margin, width, 50, world->EnergyString().c_str(),
    //                             (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM));
    //   colorstack.Pop();
    //   colorstack.Pop();
    // }

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();

    // restore camera projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }

  frames_rendered_count++;
}

void Canvas::EnterScreenCS()
{
  // use orthogonal projection without any zoom
  glMatrixMode(GL_PROJECTION);
  glPushMatrix(); // save old projection
  glLoadIdentity();
  glOrtho(0, getWidth(), 0, getHeight(), -100, 100);
  glMatrixMode(GL_MODELVIEW);

  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
}

void Canvas::LeaveScreenCS()
{
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void Canvas::switchCameraMode(bool mode)
{
	if (mode)
	{
		// Perspective mode is on, change camera
		this->current_camera = &this->perspective_camera;
	} else {
		this->current_camera = &this->camera;
	}
	setInvalidate();
}

void Canvas::Load(Worldfile *wf, int sec)
{
  this->wf = wf;
  camera.Load(wf, sec);
  perspective_camera.Load(wf, sec);

  interval = wf->ReadInt(sec, "interval", interval);

  showData.Load(wf, sec);
  showFlags.Load(wf, sec);
  showBlocks.Load(wf, sec);
  showBBoxes.Load(wf, sec);
  showBlur.Load(wf, sec);
  showClock.Load(wf, sec);
  showFollow.Load(wf, sec);
  showFootprints.Load(wf, sec);
  showGrid.Load(wf, sec);
  showOccupancy.Load(wf, sec);
  showTrailArrows.Load(wf, sec);
  showTrailRise.Load(wf, sec);
  showTrails.Load(wf, sec);
  // showVoxels.Load( wf, sec );
  pCamOn.Load(wf, sec);
}

void Canvas::Save(Worldfile *wf, int sec)
{
  camera.Save(wf, sec);
  perspective_camera.Save(wf, sec);

  wf->WriteInt(sec, "interval", interval);

  showData.Save(wf, sec);
  showBlocks.Save(wf, sec);
  showBBoxes.Save(wf, sec);
  showBlur.Save(wf, sec);
  showClock.Save(wf, sec);
  showFlags.Save(wf, sec);
  showFollow.Save(wf, sec);
  showFootprints.Save(wf, sec);
  showGrid.Save(wf, sec);
  showOccupancy.Save(wf, sec);
  showTrailArrows.Save(wf, sec);
  showTrailRise.Save(wf, sec);
  showTrails.Save(wf, sec);
  showVoxels.Save(wf, sec);
  pCamOn.Save(wf, sec);
}

void Canvas::draw_array(float x, float y, float w, float h, float *data, size_t len, size_t offset,
                         float min, float max)
{
  float sample_spacing = w / (float)len;
  float yscale = h / (max - min);

  // printf( "min %.2f max %.2f\n", min, max );

  glBegin(GL_LINE_STRIP);

  for (unsigned int i = 0; i < len; i++)
    glVertex3f(x + (float)i * sample_spacing, y + (data[(i + offset) % len] - min) * yscale, 0.01);

  glEnd();

  glColor3f(0, 0, 0);
  char buf[64];
  snprintf(buf, 63, "%.2f", min);
  this->draw_string(x, y, 0, buf);
  snprintf(buf, 63, "%.2f", max);
  this->draw_string(x, y + h - this->getHeight(), 0, buf);
}

void Canvas::draw_array(float x, float y, float w, float h, float *data, size_t len, size_t offset)
{
  // wild initial bounds
  float smallest = 1e16;
  float largest = -1e16;

  for (size_t i = 0; i < len; i++) {
    smallest = std::min(smallest, data[i]);
    largest = std::max(largest, data[i]);
  }

  draw_array(x, y, w, h, data, len, offset, smallest, largest);
}

void Canvas::draw_speech_bubble(float x, float y, float z, const char *str)
{
  draw_string(x, y, z, str);
}

void Canvas::draw_string(float x, float y, float z, const char *string)
{
	// TODO: To be implemented in child class
}

void Canvas::draw_string_multiline(float x, float y, float w, float h, const char *string, int align)
{
	// TODO: To be implemented in child class
}

void Canvas::draw_grid(bounds3d_t vol)
{
  glBegin(GL_LINES);

  for (double i = floor(vol.x.min); i < vol.x.max; i++) {
    glVertex2f(i, vol.y.min);
    glVertex2f(i, vol.y.max);
  }

  for (double i = floor(vol.y.min); i < vol.y.max; i++) {
    glVertex2f(vol.x.min, i);
    glVertex2f(vol.x.max, i);
  }

  glEnd();

  char str[16];

  for (double i = floor(vol.x.min); i < vol.x.max; i++) {
    snprintf(str, 16, "%d", (int)i);
    draw_string(i, 0, 0.00, str);
  }

  for (double i = floor(vol.y.min); i < vol.y.max; i++) {
    snprintf(str, 16, "%d", (int)i);
    draw_string(0, i, 0.00, str);
  }
}

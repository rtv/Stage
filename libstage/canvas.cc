
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
#include <png.h>
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

static bool init_done = false;
static bool texture_load_done = false;

// GLuint glowTex;
GLuint checkTex;

void Canvas::TimerCallback(Canvas *c)
{
  if (c->world->dirty) {
    // puts( "timer redraw" );
    c->redraw();
    c->world->dirty = false;
  }

  Fl::repeat_timeout(c->interval / 1000.0, (Fl_Timeout_Handler)Canvas::TimerCallback, c);
}

Canvas::Canvas(WorldGui *world, int x, int y, int width, int height)
    : Fl_Gl_Window(x, y, width, height), colorstack(), models_sorted(), current_camera(NULL),
      camera(), perspective_camera(), dirty_buffer(false), wf(NULL), startx(-1), starty(-1),
      selected_models(), last_selection(NULL), interval(40), // msec between redraws
      // initialize Option objects
      //  showBlinken( "Blinkenlights", "show_blinkenlights", "", true, world ),
      showBBoxes("Debug/Bounding boxes", "show_boundingboxes", "^b", false, world),
      showBlocks("Blocks", "show_blocks", "b", true, world),
      showBlur("Trails/Blur", "show_trailblur", "^d", false, world),
      showClock("Clock", "show_clock", "c", true, world),
      showData("Data", "show_data", "d", false, world),
      showFlags("Flags", "show_flags", "l", true, world),
      showFollow("Follow", "show_follow", "f", false, world),
      showFootprints("Footprints", "show_footprints", "o", false, world),
      showGrid("Grid", "show_grid", "g", true, world),
      showOccupancy("Debug/Occupancy", "show_occupancy", "^o", false, world),
      showScreenshots("Save screenshots", "screenshots", "", false, world),
      showStatus("Status", "show_status", "s", true, world),
      showTrailArrows("Trails/Rising Arrows", "show_trailarrows", "^a", false, world),
      showTrailRise("Trails/Rising blocks", "show_trailrise", "^r", false, world),
      showTrails("Trails/Fast", "show_trailfast", "^f", false, world),
      showVoxels("Debug/Voxels", "show_voxels", "^v", false, world),
      pCamOn("Perspective camera", "pcam_on", "r", false, world),
      visualizeAll("Selected only", "vis_all", "v", false, world),
      // and the rest
      graphics(true), world(world), frames_rendered_count(0), screenshot_frame_skip(1)
{
  end();
  // show(); // must do this so that the GL context is created before
  // configuring GL
  // but that line causes a segfault in Linux/X11! TODO: test in OS X

  mode( FL_RGB |FL_DOUBLE|FL_DEPTH| FL_MULTISAMPLE | FL_ALPHA );
  
  perspective_camera.setPose(0.0, -4.0, 3.0);
  current_camera = &camera;
  setDirtyBuffer();

  // enable accumulation buffer
  // mode( mode() | FL_ACCUM );
  // assert( can_do( FL_ACCUM ) );
}

void Canvas::InitGl()
{
  valid(1);
  FixViewport(w(), h());

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

  // install a font
  gl_font(FL_HELVETICA, 12);

  blur = false;

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  init_done = true;
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

  texture_load_done = true;
}

Canvas::~Canvas()
{
  // nothing to do
}

Model *Canvas::getModel(int x, int y)
{
  // render all models in a unique color
  make_current(); // make sure the GL context is current
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
    redraw();
  }
}

void Canvas::unSelect(Model *mod)
{
  if (mod) {
    EraseAll(mod, selected_models);
    redraw();
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
  else if (px >= w())
    px = w() - 1;
  if (py <= 0)
    py = 1;
  else if (py >= h())
    py = h() - 1;

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
  glReadPixels(px, h() - py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pz);
  gluUnProject(px, w() - py, pz, modelview, projection, viewport, wx, wy, wz);
}

int Canvas::handle(int event)
{
  // printf( "cam %.2f %.2f\n", camera.yaw(), camera.pitch() );

  switch (event) {
  case FL_MOUSEWHEEL:
    if (pCamOn == true) {
      perspective_camera.scroll(Fl::event_dy() / 10.0);
    } else {
      camera.scale(Fl::event_dy(), Fl::event_x(), w(), Fl::event_y(), h());
    }
    invalidate();
    redraw();
    return 1;

  case FL_MOVE: // moused moved while no button was pressed
    if (Fl::event_state(FL_META)) {
      puts("TODO: HANDLE HISTORY");
      // world->paused = ! world->paused;
      return 1;
    }

    if (startx >= 0) {
      // mouse pointing to valid value

      if (Fl::event_state(FL_CTRL)) // rotate the camera view
      {
        int dx = Fl::event_x() - startx;
        int dy = Fl::event_y() - starty;

        if (pCamOn == true) {
          perspective_camera.addYaw(-dx);
          perspective_camera.addPitch(-dy);
        } else {
          camera.addPitch(-0.5 * static_cast<double>(dy));
          camera.addYaw(-0.5 * static_cast<double>(dx));
        }
        invalidate();
        redraw();
      } else if (Fl::event_state(FL_ALT)) {
        int dx = Fl::event_x() - startx;
        int dy = Fl::event_y() - starty;

        if (pCamOn == true) {
          perspective_camera.move(-dx, dy, 0.0);
        } else {
          camera.move(-dx, dy);
        }
        invalidate();
      }
    }
    startx = Fl::event_x();
    starty = Fl::event_y();
    return 1;
  case FL_PUSH: // button pressed
  {
    // else
    {
      Model *mod = getModel(startx, starty);
      startx = Fl::event_x();
      starty = Fl::event_y();
      selectedModel = false;
      switch (Fl::event_button()) {
      case 1:
        clicked_empty_space = (mod == NULL);
        empty_space_startx = startx;
        empty_space_starty = starty;
        if (mod) {
          // clicked a model
          if (Fl::event_state(FL_SHIFT)) {
            // holding shift, toggle selection
            if (selected(mod))
              unSelect(mod);
            else {
              select(mod);
              selectedModel = true; // selected a model
            }
          } else {
            if (!selected(mod)) {
              // clicked on an unselected model while
              //  not holding shift, this is the new
              //  selection
              unSelectAll();
              select(mod);
            }
            selectedModel = true; // selected a model
          }
        }

        redraw(); // probably required
        return 1;
      case 3: {
        // leave selections alone
        // rotating handled within FL_DRAG
        return 1;
      }
      default: return 0;
      }
    }
  }

  case FL_DRAG: // mouse moved while button was pressed
  {
    int dx = Fl::event_x() - startx;
    int dy = Fl::event_y() - starty;

    if (Fl::event_state(FL_BUTTON1) && Fl::event_state(FL_CTRL) == false) {
      // Left mouse button drag
      if (selectedModel) {
        // started dragging on a selected model

        double sx, sy, sz;
        CanvasToWorld(startx, starty, &sx, &sy, &sz);
        double x, y, z;
        CanvasToWorld(Fl::event_x(), Fl::event_y(), &x, &y, &z);
        // move all selected models to the mouse pointer
        FOR_EACH (it, selected_models) {
          Model *mod = *it;
          mod->AddToPose(x - sx, y - sy, 0, 0);
        }
      } else {
        // started dragging on empty space or an
        //  unselected model, move the canvas
        if (pCamOn == true) {
          perspective_camera.move(-dx, dy, 0.0);
        } else {
          camera.move(-dx, dy);
        }
        invalidate(); // so the projection gets updated
      }
    } else if (Fl::event_state(FL_BUTTON3)
               || (Fl::event_state(FL_BUTTON1) && Fl::event_state(FL_CTRL))) {
      // rotate all selected models

      if (selected_models.size()) {
        FOR_EACH (it, selected_models) {
          Model *mod = *it;
          mod->AddToPose(0, 0, 0, 0.05 * (dx + dy));
        }
      } else {
        // printf( "button 2\n" );

        int dx = Fl::event_x() - startx;
        int dy = Fl::event_y() - starty;

        if (pCamOn == true) {
          perspective_camera.addYaw(-dx);
          perspective_camera.addPitch(-dy);
        } else {
          camera.addPitch(-0.5 * static_cast<double>(dy));
          camera.addYaw(-0.5 * static_cast<double>(dx));
        }
      }
      invalidate();
      redraw();
    }

    startx = Fl::event_x();
    starty = Fl::event_y();

    redraw();
    return 1;
  } // end case FL_DRAG

  case FL_RELEASE: // mouse button released
    if (empty_space_startx == Fl::event_x() && empty_space_starty == Fl::event_y()
        && clicked_empty_space == true) {
      // clicked on empty space, unselect all
      unSelectAll();
      redraw();
    }
    return 1;

  case FL_FOCUS:
  case FL_UNFOCUS:
    //.... Return 1 if you want keyboard events, 0 otherwise
    return 1;

  case FL_KEYBOARD:
    switch (Fl::event_key()) {
    case FL_Left:
      if (pCamOn == false) {
        camera.move(-10, 0);
      } else {
        perspective_camera.strafe(-0.5);
      }
      break;
    case FL_Right:
      if (pCamOn == false) {
        camera.move(10, 0);
      } else {
        perspective_camera.strafe(0.5);
      }
      break;
    case FL_Down:
      if (pCamOn == false) {
        camera.move(0, -10);
      } else {
        perspective_camera.forward(-0.5);
      }
      break;
    case FL_Up:
      if (pCamOn == false) {
        camera.move(0, 10);
      } else {
        perspective_camera.forward(0.5);
      }
      break;
    default:
      redraw(); // we probably set a display config - so need this
      return 0; // keypress unhandled
    }

    invalidate(); // update projection
    return 1;

  //	case FL_SHORTCUT:
  //		//... shortcut, key is in Fl::event_key(), ascii in
  // Fl::event_text()
  //		//... Return 1 if you understand/use the shortcut event, 0
  // otherwise...
  //		return 1;
  default:
    // pass other events to the base class...
    // printf( "EVENT %d\n", event );
    return Fl_Gl_Window::handle(event);

  } // end switch( event )

  return 0;
}

void Canvas::FixViewport(int W, int H)
{
  glLoadIdentity();
  glViewport(0, 0, W, H);
}

void Canvas::AddModel(Model *mod)
{
  models_sorted.push_back(mod);
  redraw();
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
    Gl::draw_string(i, 0, 0, str);
  }

  for (double i = 0; i >= bounds.x.min; i -= skip) {
    snprintf(str, 16, "%d", (int)i);
    Gl::draw_string(i, 0, 0, str);
  }

  for (double i = 0; i < bounds.y.max; i += skip) {
    snprintf(str, 16, "%d", (int)i);
    Gl::draw_string(0, i, 0, str);
  }

  for (double i = 0; i >= bounds.y.min; i -= skip) {
    snprintf(str, 16, "%d", (int)i);
    Gl::draw_string(0, i, 0, str);
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

  world->DrawBoundingBoxTree();

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
  float scale_x = w() / (max_x - min_x) * 0.9;
  float scale_y = h() / (max_y - min_y) * 0.9;
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

      Gl::draw_string(world->rt_cells[i].x + 1, world->rt_cells[i].y + 1, 0.1, str);

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
    (*it)->DrawSelected();

  // useful debug - puts a point at the origin of each model
  // for( GList* it = world->World::children; it; it=it->next )
  // ((Model*)it->data)->DrawOriginTree();

  // draw the model-specific visualizations
  if (world->sim_time > 0) {
    if (showData) {
      if (!visualizeAll) {
        FOR_EACH (it, world->World::children)
          (*it)->DataVisualizeTree(current_camera);
      } else if (selected_models.size() > 0) {
        FOR_EACH (it, selected_models)
          (*it)->DataVisualizeTree(current_camera);
      } else if (last_selection) {
        last_selection->DataVisualizeTree(current_camera);
      }
    }
  }

  if (showGrid)
    FOR_EACH (it, models_sorted)
      (*it)->DrawGrid();

  if (showStatus) {
    glPushMatrix();
    // ensure two icons can't be in the exact same plane
    if (camera.pitch() == 0 && !pCamOn)
      glTranslatef(0, 0, 0.1);

    FOR_EACH (it, models_sorted)
      (*it)->DrawStatusTree(&camera);

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
    glOrtho(0, w(), 0, h(), -100, 100);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    std::string clockstr = world->ClockString();
    if (showFollow == true && last_selection)
      clockstr.append(" [FOLLOW MODE]");

    float txtWidth = gl_width(clockstr.c_str());
    if (txtWidth < 200)
      txtWidth = 200;
    int txtHeight = gl_height();

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

    Gl::draw_string(margin, margin, 0, clockstr.c_str() );
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

  if (showScreenshots && (frames_rendered_count % screenshot_frame_skip == 0))
    Screenshot();

  frames_rendered_count++;
}

void Canvas::EnterScreenCS()
{
  // use orthogonal projection without any zoom
  glMatrixMode(GL_PROJECTION);
  glPushMatrix(); // save old projection
  glLoadIdentity();
  glOrtho(0, w(), 0, h(), -100, 100);
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

void Canvas::Screenshot()
{
  int width = w();
  int height = h();
  int depth = 4; // RGBA

  // we use RGBA throughout, though we only need RGB, as the 4-byte
  // pixels avoid a nasty word-alignment problem when indexing into
  // the pixel array.

  // might save a bit of time with a static var as the image size rarely changes
  static std::vector<uint8_t> pixels;
  pixels.resize(width * height * depth);

  glFlush(); // make sure the drawing is done
  // read the pixels from the screen
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

  static uint32_t count = 0;
  char filename[64];
  snprintf(filename, 63, "stage-%06d.png", count++);

  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    PRINT_ERR1("Unable to open %s", filename);
  }

  // create PNG data
  png_structp pp = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  assert(pp);
  png_infop info = png_create_info_struct(pp);
  assert(info);

  // setup the output file
  png_init_io(pp, fp);

  // need to invert the image as GL and PNG disagree on the row order
  png_bytep *rowpointers = new png_bytep[height];
  for (int i = 0; i < height; i++)
    rowpointers[i] = &pixels[(height - 1 - i) * width * depth];

  png_set_rows(pp, info, rowpointers);

  png_set_IHDR(pp, info, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_write_png(pp, info, PNG_TRANSFORM_IDENTITY, NULL);

  // free the PNG data - we reuse the pixel array next call.
  png_destroy_write_struct(&pp, &info);

  fclose(fp);

  printf("Saved %s\n", filename);
  delete[] rowpointers;
}

void Canvas::perspectiveCb(Fl_Widget *w, void *p)
{
  Canvas *canvas = static_cast<Canvas *>(w);
  Option *opt = static_cast<Option *>(p); // pCamOn
  if (opt) {
    // Perspective mode is on, change camera
    canvas->current_camera = &canvas->perspective_camera;
  } else {
    canvas->current_camera = &canvas->camera;
  }

  canvas->invalidate();
}

void Canvas::createMenuItems(Fl_Menu_Bar *menu, std::string path)
{
  showData.createMenuItem(menu, path);
  //  visualizeAll.createMenuItem( menu, path );
  showBlocks.createMenuItem(menu, path);
  showFlags.createMenuItem(menu, path);
  showClock.createMenuItem(menu, path);
  showFlags.createMenuItem(menu, path);
  showFollow.createMenuItem(menu, path);
  showFootprints.createMenuItem(menu, path);
  showGrid.createMenuItem(menu, path);
  showStatus.createMenuItem(menu, path);
  pCamOn.createMenuItem(menu, path);
  pCamOn.menuCallback(perspectiveCb, this);
  showOccupancy.createMenuItem(menu, path);
  showTrailArrows.createMenuItem(menu, path);
  showTrails.createMenuItem(menu, path);
  showTrailRise.createMenuItem(menu, path); // broken
  showBBoxes.createMenuItem(menu, path);
  // showVoxels.createMenuItem( menu, path );
  showScreenshots.createMenuItem(menu, path);
}

void Canvas::Load(Worldfile *wf, int sec)
{
  this->wf = wf;
  camera.Load(wf, sec);
  perspective_camera.Load(wf, sec);

  interval = wf->ReadInt(sec, "interval", interval);

  screenshot_frame_skip = wf->ReadInt(sec, "screenshot_skip", screenshot_frame_skip);
  if (screenshot_frame_skip < 1)
    screenshot_frame_skip = 1; // avoids div-by-zero if poorly set

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
  showScreenshots.Load(wf, sec);
  pCamOn.Load(wf, sec);

  if (!world->paused)
    // // start the timer that causes regular redraws
    Fl::add_timeout(((double)interval / 1000), (Fl_Timeout_Handler)Canvas::TimerCallback, this);

  invalidate(); // we probably changed something
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
  showScreenshots.Save(wf, sec);
  pCamOn.Save(wf, sec);
}

void Canvas::draw()
{
  // Enable the following to debug camera model
  //	if( loaded_texture == true && pCamOn == true )
  //		return;

  if (!valid()) {
    if (!init_done)
      InitGl();
    if (!texture_load_done)
      InitTextures();

    if (pCamOn == true) {
      perspective_camera.setAspect(static_cast<float>(w()) / static_cast<float>(h()));
      perspective_camera.SetProjection();
      current_camera = &perspective_camera;
    } else {
      bounds3d_t extent = world->GetExtent();
      camera.SetProjection(w(), h(), extent.y.min, extent.y.max);
      current_camera = &camera;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Follow the selected robot
  if (showFollow && last_selection) {
    Pose gpose = last_selection->GetGlobalPose();
    if (pCamOn == true) {
      perspective_camera.setPose(gpose.x, gpose.y, 0.2);
      perspective_camera.setYaw(rtod(gpose.a) - 90.0);
    } else {
      camera.setPose(gpose.x, gpose.y);
    }
  }

  current_camera->Draw();
  renderFrame();
}

void Canvas::resize(int X, int Y, int W, int H)
{
  Fl_Gl_Window::resize(X, Y, W, H);

  if (!init_done) // hack - should capture a create event to do this rather
    // thann have it in multiple places
    InitGl();

  FixViewport(W, H);
  invalidate();
}

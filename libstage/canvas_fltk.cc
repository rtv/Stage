#include "canvas_fltk.hh"
#include "menu_manager_fltk.hh"
#include <FL/Fl.H>
#include <FL/gl.h>
#include "png.h"

namespace Stg
{

CanvasFLTK::CanvasFLTK(WorldGui *world, int x, int y, int width, int height)
:Canvas(world, x, y, width, height)
,Fl_Gl_Window(x, y, width, height), selectedModel(false), startx(-1), starty(-1),
 empty_space_startx(0), empty_space_starty(0),
 showScreenshots("Save screenshots", "screenshots", "", false),
 menu_manager(NULL),
 graphics(true), screenshot_frame_skip(1)
{
  end();
  // show(); // must do this so that the GL context is created before
  // configuring GL
  // but that line causes a segfault in Linux/X11! TODO: test in OS X
  mode( FL_RGB |FL_DOUBLE|FL_DEPTH| FL_MULTISAMPLE | FL_ALPHA );
}

CanvasFLTK::~CanvasFLTK()
{

}

bool CanvasFLTK::isValid()
{
	return valid();
}

void CanvasFLTK::doRedraw()
{
	redraw();
}

int CanvasFLTK::getWidth() const
{
	return w();
}

int CanvasFLTK::getHeight() const
{
	return h();
}

void CanvasFLTK::setInvalidate()
{
	invalidate();
}

void CanvasFLTK::perspectiveCb(Option *opt, void *p)
{
  Canvas *canvas = static_cast<Canvas *>(p);
  canvas->switchCameraMode(opt->val());
}

void CanvasFLTK::createMenuItems(Fl_Menu_Bar *menu, std::string path)
{
	assert(this->menu_manager == NULL);
	MenuManagerFLTK * mm = new MenuManagerFLTK(this, menu);
	menu_manager = mm;
	mm->createMenuItem(showData, path);
  // mm->createMenuItem(visualizeAll, path );
	mm->createMenuItem(showBlocks, path);
	mm->createMenuItem(showFlags, path);
	mm->createMenuItem(showClock, path);
	mm->createMenuItem(showFollow, path);
	mm->createMenuItem(showFootprints, path);
	mm->createMenuItem(showGrid, path);
	mm->createMenuItem(showStatus, path);
	mm->createMenuItem(pCamOn, path);
	pCamOn.setOptionCallback(perspectiveCb, this);
	mm->createMenuItem(showOccupancy, path);
	mm->createMenuItem(showTrailArrows, path);
	mm->createMenuItem(showTrails, path);
	mm->createMenuItem(showTrailRise, path); // broken
	mm->createMenuItem(showBBoxes, path);
	// mm->createMenuItem(showVoxels, path );
	mm->createMenuItem(showScreenshots, path);
}

double CanvasFLTK::fontWidth(const char * str) const
{
	return gl_width(str);
}

double CanvasFLTK::fontHeight() const
{
	return gl_height();
}

void CanvasFLTK::Screenshot()
{
  int width = getWidth();
  int height = getHeight();
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


void CanvasFLTK::renderFrame()
{
	Canvas::renderFrame();

	if (showScreenshots && (frames_rendered_count % screenshot_frame_skip == 0))
	    Screenshot();
}

int CanvasFLTK::handle(int event)
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

void CanvasFLTK::resize(int X, int Y, int W, int H)
{
  Fl_Gl_Window::resize(X, Y, W, H);

  if (!init_done) // hack - should capture a create event to do this rather
    // thann have it in multiple places
    InitGl();

  FixViewport(W, H);
  invalidate();
}

void CanvasFLTK::Load(Worldfile *wf, int sec)
{
	Canvas::Load(wf, sec);

	showScreenshots.Load(wf, sec);

	screenshot_frame_skip = wf->ReadInt(sec, "screenshot_skip", screenshot_frame_skip);
	if (screenshot_frame_skip < 1)
		screenshot_frame_skip = 1; // avoids div-by-zero if poorly set

  if (!world->paused)
    // // start the timer that causes regular redraws
    Fl::add_timeout(((double)interval / 1000), (Fl_Timeout_Handler)CanvasFLTK::TimerCallback, this);

  invalidate(); // we probably changed something
}

void CanvasFLTK::Save(Worldfile *wf, int sec)
{
	wf->WriteInt(sec, "screenshot_skip", screenshot_frame_skip);
	showScreenshots.Save(wf, sec);
	Canvas::Save(wf, sec);
}

void CanvasFLTK::TimerCallback(Canvas *c)
{
  if (c->world->IsDirty())
  {
    // puts( "timer redraw" );
    c->doRedraw();
    c->world->SetDirty(false);
  }
  msec_t interval = c->getRedrawInterval();
  Fl::repeat_timeout(interval / 1000.0, (Fl_Timeout_Handler)CanvasFLTK::TimerCallback, c);
}

void CanvasFLTK::draw_string(float x, float y, float z, const char *str)
{
  glRasterPos3f(x, y, z);

  GLboolean b;
  glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &b);

  // printf( "[%.2f %.2f %.2f] %d string %u %s\n", x,y,z, (int)b, (unsigned
  // int)strlen(str), str );

  if (b)
    gl_draw(str); // fltk function
}

void CanvasFLTK::draw_string_multiline(float x, float y, float w, float h, const char *str, int align)
{
  // printf( "[%.2f %.2f %.2f] string %u %s\n", x,y,z,(unsigned int)strlen(str),
  // str );
  gl_draw(str, x, y, w, h, (Fl_Align)align); // fltk function
}

void CanvasFLTK::draw()
{
  // Enable the following to debug camera model
  //	if( loaded_texture == true && pCamOn == true )
  //		return;

  if (!isValid()) {
    if (!init_done)
    {
      InitGl();
      // install a font
      gl_font(FL_HELVETICA, 12);
      init_done = true;
    }
    if (!texture_load_done)
    {
      InitTextures();
      texture_load_done = true;
    }

    if (pCamOn == true) {
      perspective_camera.setAspect(static_cast<float>(getWidth()) / static_cast<float>(getHeight()));
      perspective_camera.SetProjection();
      current_camera = &perspective_camera;
    } else {
      bounds3d_t extent = world->GetExtent();
      camera.SetProjection(getWidth(), getHeight(), extent.y.min, extent.y.max);
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


}

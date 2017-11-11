#include "canvas_fltk.hh"

namespace Stg
{

//Fl_Gl_Window(x, y, width, height),

CanvasFLTK::CanvasFLTK(WorldGui *world, int x, int y, int width, int height)
:Canvas(world, x, y, width, height), screenshot_frame_skip(1)
{
  end();
  // show(); // must do this so that the GL context is created before
  // configuring GL
  // but that line causes a segfault in Linux/X11! TODO: test in OS X
  mode( FL_RGB |FL_DOUBLE|FL_DEPTH| FL_MULTISAMPLE | FL_ALPHA );
}

void CanvasFLTK::perspectiveCb(Option *opt, void *p)
{
  Canvas *canvas = static_cast<Canvas *>(p);
  if (opt) {
    // Perspective mode is on, change camera
    canvas->current_camera = &canvas->perspective_camera;
  } else {
    canvas->current_camera = &canvas->camera;
  }

  canvas->invalidate();
}

void CanvasFLTK::createMenuItems(Fl_Menu_Bar *menu, std::string path)
{
	createMenuItem(showData.menu, path);
  // createMenuItem(visualizeAll, menu, path );
	createMenuItem(showBlocks,menu, path);
	createMenuItem(showFlags, menu, path);
	createMenuItem(showClock, menu, path);
	createMenuItem(showFlags, menu, path);
	createMenuItem(showFollow, menu, path);
	createMenuItem(showFootprints, menu, path);
	createMenuItem(showGrid, menu, path);
	createMenuItem(showStatus, menu, path);
	createMenuItem(pCamOn, menu, path);
	pCamOn.setOptionCallback(perspectiveCb, this);
	createMenuItem(showOccupancy, menu, path);
	createMenuItem(showTrailArrows, menu, path);
	createMenuItem(showTrails, menu, path);
	createMenuItem(showTrailRise, menu, path); // broken
	createMenuItem(showBBoxes, menu, path);
	// createMenuItem(showVoxels, menu, path );
	createMenuItem(showScreenshots, menu, path);
}

void CanvasFLTK::toggleCb(Fl_Widget *, void *p)
{
  // Fl_Menu_* menu = static_cast<Fl_Menu_*>( w );
  Option *opt = static_cast<Option *>(p);
  opt->invert();
  if (opt->menuCb)
    opt->menuCb(opt, opt->callbackData);
}

void CanvasFLTK::createMenuItem(Option * opt, Fl_Menu_Bar *m, std::string path)
{
  opt->menu = m;
  path = path + "/" + optName;
  // create a menu item and save its index
  opt->menuIndex = menu->add(path.c_str(), shortcut.c_str(), toggleCb, opt, FL_MENU_TOGGLE | (value ? FL_MENU_VALUE : 0));
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
  if (!world->paused)
    // // start the timer that causes regular redraws
    Fl::add_timeout(((double)interval / 1000), (Fl_Timeout_Handler)Canvas::TimerCallback, this);

  invalidate(); // we probably changed something
}

void CanvasFLTK::TimerCallback(Canvas *c)
{
  if (c->world->dirty) {
    // puts( "timer redraw" );
    c->doRedraw();
    c->world->dirty = false;
  }
  Fl::repeat_timeout(c->interval / 1000.0, (Fl_Timeout_Handler)Canvas::TimerCallback, c);
}

}

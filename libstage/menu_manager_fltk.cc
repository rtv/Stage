/*
 * menu_manager_ftlk.cc
 *
 *  Created on: Nov 13, 2017
 *      Author: vrobot
 */

#include "menu_manager_fltk.hh"
#include "canvas_fltk.hh"


namespace Stg
{

Fl_Menu_Item *getMenuItem(Fl_Menu_ *menu, int i)
{
  const Fl_Menu_Item *mArr = menu->menu();
  return const_cast<Fl_Menu_Item *>(&mArr[i]);
}

MenuManagerFLTK::MenuManagerFLTK(CanvasFLTK * canvas, Fl_Menu_Bar *menu)
	:canvas(canvas), menu(menu)
{

}

CanvasFLTK * MenuManagerFLTK::GetCanvas()
{
	return canvas;
}

void MenuManagerFLTK::createMenuItem(Option & opt, std::string path)
{
  path = path + "/" + opt.name();
  std::string key = opt.key();
  // create a menu item and save its index
  size_t index = menu->add(path.c_str(), key.c_str(), toggleCb, &opt, FL_MENU_TOGGLE | (opt.val() ? FL_MENU_VALUE : 0));
  opt.onAttachGUI(this, reinterpret_cast<void*>(index));
}

void MenuManagerFLTK::syncMenuState(Option * opt)
{
	size_t index = reinterpret_cast<size_t>(opt->getWidgetData());

  if (menu != NULL) {
    Fl_Menu_Item *item = getMenuItem(menu, index);
    if(item != NULL) {
    	opt->val() ? item->set() : item->clear();
    }
  }

  CanvasFLTK *canvas = GetCanvas();
  if (canvas != NULL)
  {
    canvas->setInvalidate();
    canvas->redraw();
  }
}

void MenuManagerFLTK::toggleCb(Fl_Widget *, void *p)
{
  // Fl_Menu_* menu = static_cast<Fl_Menu_*>( w );
  Option *opt = static_cast<Option *>(p);
  opt->invert();
  opt->callValueChanged();
}

}



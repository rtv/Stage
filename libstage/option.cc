
#include "option.hh"
#include "canvas.hh"
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

Option::Option(const std::string &n, const std::string &tok, const std::string &key, bool v,
               World *world)
    : optName(n), value(v), wf_token(tok), shortcut(key), menu(NULL), menuIndex(0), menuCb(NULL),
      menuCbWidget(NULL), _world(world), htname(n)
{
  /* do nothing */
}

Fl_Menu_Item *getMenuItem(Fl_Menu_ *menu, int i)
{
  const Fl_Menu_Item *mArr = menu->menu();
  return const_cast<Fl_Menu_Item *>(&mArr[i]);
}

void Option::Load(Worldfile *wf, int section)
{
  // printf( "loading wf key %s\n", wf_token.c_str() );
  set((bool)wf->ReadInt(section, wf_token.c_str(), value));
}

void Option::Save(Worldfile *wf, int section)
{
  wf->WriteInt(section, wf_token.c_str(), value);
}

void Option::toggleCb(Fl_Widget *, void *p)
{
  // Fl_Menu_* menu = static_cast<Fl_Menu_*>( w );
  Option *opt = static_cast<Option *>(p);
  opt->invert();
  if (opt->menuCb)
    opt->menuCb(opt->menuCbWidget, opt);
}

void Option::menuCallback(Fl_Callback *cb, Fl_Widget *w)
{
  menuCb = cb;
  menuCbWidget = w;
}

void Option::createMenuItem(Fl_Menu_Bar *m, std::string path)
{
  menu = m;
  path = path + "/" + optName;
  // create a menu item and save its index
  menuIndex = menu->add(path.c_str(), shortcut.c_str(), toggleCb, this,
                        FL_MENU_TOGGLE | (value ? FL_MENU_VALUE : 0));
}

void Option::set(bool val)
{
  value = val;

  if (menu) {
    Fl_Menu_Item *item = getMenuItem(menu, menuIndex);
    value ? item->set() : item->clear();
  }

  if (_world) {
    WorldGui *wg = dynamic_cast<WorldGui *>(_world);
    if (wg == NULL)
      return;
    Canvas *canvas = wg->GetCanvas();
    canvas->invalidate();
    canvas->redraw();
  }
}


#include "option.hh"
#include "canvas.hh"
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

Option::Option(const std::string &n, const std::string &tok, const std::string &key, bool v,
               World *world)
    : optName(n), value(v), wf_token(tok), shortcut(key), menu(NULL), menuIndex(0), menuCb(NULL),
			callbackData(NULL), _world(world), htname(n)
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

void Option::setOptionCallback(Option::Callback cb, void *data)
{
  menuCb = cb;
  this->callbackData = data;
}

void Option::set(bool val)
{
  value = val;

  if (menu) {
    Fl_Menu_Item *item = getMenuItem(menu, menuIndex);
    value ? item->set() : item->clear();
  }

  if (_world) {
#ifdef TEST_LATER
    WorldGui *wg = dynamic_cast<WorldGui *>(_world);
    if (wg == NULL)
      return;
    Canvas *canvas = wg->GetCanvas();
    canvas->setInvalidate();
    canvas->redraw();
#endif
  }
}


#include "option.hh"
#include "canvas.hh"
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

void MenuManager::callOptionCallback(Option * opt)
{
	if (opt->menuCb)
			opt->menuCb(opt, opt->callbackData);
}

Option::Option(const std::string &n, const std::string &tok, const std::string &key, bool v)
    : optName(n), value(v), wf_token(tok), shortcut(key), menuCb(NULL),
			callbackData(NULL), manager(NULL), menu_widget(NULL), htname(n)
{
  /* do nothing */
}

Option::~Option()
{

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

  if(manager != NULL)
  {
  	manager->syncMenuState(this);
  }
}

void Option::onAttachGUI(MenuManager * manager, void * widget)
{
	assert(this->manager == NULL);
	this->manager = manager;
	this->menu_widget = widget;
}

void * Option::getWidgetData()
{
	return menu_widget;
}

void Option::callValueChanged()
{
	if (menuCb != NULL)
		menuCb(this, callbackData);
}

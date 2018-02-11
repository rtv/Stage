/*
 * menu_manager_fltk.hh
 *
 *  Created on: Nov 13, 2017
 *      Author: vrobot
 */

#ifndef LIBSTAGE_MENU_MANAGER_FLTK_HH_
#define LIBSTAGE_MENU_MANAGER_FLTK_HH_

#include "option.hh"
#include <FL/Fl_Menu_Bar.H>

namespace Stg {

class CanvasFLTK;

class MenuManagerFLTK : public MenuManager
{
public:
	MenuManagerFLTK(CanvasFLTK * canvas, Fl_Menu_Bar *menu);
	virtual void syncMenuState(Option * opt) override;
	/** Creates menu item for an option */
	void createMenuItem(Option & opt, std::string path);
protected:
	/** Toggle callback from FLTK */
	static void toggleCb(Fl_Widget *w, void *p);
	CanvasFLTK * GetCanvas();
private:
	CanvasFLTK * canvas;
	Fl_Menu_ *menu;
};

}


#endif /* LIBSTAGE_MENU_MANAGER_FLTK_HH_ */

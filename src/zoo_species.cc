/*
 *  Zoo driver for Player
 *  Copyright (C) 2005 Adam Lein
 *                      
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 */

/**
 * \addtogroup zoo
 * @{
 *  \defgroup ZooSpecies
 *  @{
 * Description: An addon to the Stage plugin for Zoo functionality.
 * Author: Adam Lein
 * Date: July 14, 2005
 */

#include "zoo_driver.h"
#include "zoo_species.h"

#include <player/devicetable.h>
#include <player/error.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <glib.h>
#include <wordexp.h>
#include <stdio.h>
#include <dlfcn.h>
#include <gui.h>

/********* ZooSpecies *********/

/**
 * ZooSpecies constructor.
 * @param _zoo the ZooDriver object that created me.
 */
ZooSpecies::ZooSpecies( ConfigFile *cf, int section, ZooDriver *_zoo )
{
	/* remember the ZooDriver for later use */
	zoo = _zoo;

	/* get the name; this is a const char * and it's not my problem
	 * to free it. */
	name = cf->ReadString(section, "name", ZOO_DEFAULT_SPECIES);

	/* get the list of models (and therefore ports) which are members of this
	 * species */
	population_size = cf->GetTupleCount(section, "population");
	port_list = new int[population_size];
	model_list = new char*[population_size];
	for (int i=0; i < population_size; ++i) {
		const char *mname;
		mname = cf->ReadTupleString(section, "population", i, "");
		if (!mname || !*mname) break;
		model_list[i] = strdup(mname);
		port_list[i] = zoo->GetModelPortByName(mname);
		if (port_list[i] == 0)
			zoo_err("Member \"%s\" of species \"%s\" doesn't exist.\n",
				model_list[i], name);
	}

	/* find all controllers belonging to me */
	controller.clear();
	for (int i=0; i < cf->GetSectionCount(); ++i) {
		int pi;

		/* only care about controllers */
		if (strcmp(cf->GetSectionType(i), ZOO_CONTROLLER_SECTYPE))
			continue;

		/* this should be a child section of my section */
		pi = cf->GetSectionParent(i);
		if (pi < 0
		 || strcmp(cf->GetSectionType(pi), "species")
		 || strcmp(cf->ReadString(pi, "name", ""), name))
			continue;

		ZooController zc(cf, i, zoo, this);
		controller.push_back(zc);
	}

	/* initialize controller selection */
	next_controller = 0;
	controller_instance = 0;

	/* default score display setup */
	score_draw_cb = NULL;
	score_draw_user_data = NULL;

	ZOO_DEBUGMSG("Zoo: Added species %s\n", name, 0);
}

/**
 * ZooSpecies destructor.  Destroys model list and port list.
 */
ZooSpecies::~ZooSpecies()
{
	ZOO_DEBUGMSG("Zoo: Cleaning up species %s", name, 0);
	delete port_list;
	delete model_list;
	ZOO_DEBUGMSG("Zoo: Done cleaning up %s\n", name, 0);
}

static int
clear_cb( stg_model_t *mod )
{
	stg_rtk_fig_t *fig;

	fig = stg_model_get_fig(mod, ZOOREF_SCORE_FIG_NAME);
	if (fig)
		stg_rtk_fig_clear(fig);

	return 0;
}

/**
 * Set a callback to be called when a robot's score changes.
 * FIXME: This means that if the score doesn't change, the display doesn't
 * move with the robot.  Oops.
 * @param draw_cb the callback.
 * @param userdata arbitrary data to be passed along.
 */
void
ZooSpecies::SetScoreDrawCB( zooref_score_draw_t draw_cb, void *userdata )
{
	score_draw_cb = draw_cb;
	score_draw_user_data = userdata;

	int i, n=zoo->GetModelCount();
	for (i=0; i<n; ++i) {
		stg_model_t *mod = zoo->GetModelByIndex(i);
		if (!mod) continue;
		stg_model_add_property_toggles(mod, ZOO_SCORE_PROPERTY_NAME,
			(stg_property_callback_t)draw_cb, userdata,
			(stg_property_callback_t)clear_cb, NULL,
			ZOO_SCORE_LABEL,
			0);
	}
}

/**
 * Run a controller on port a given port.
 * @param p the port to run on
 * @return a pointer to the ZooController that was executed.
 */
ZooController *
ZooSpecies::Run( int p )
{
	ZooController *zc = SelectController();
	if (!zc) {
		zoo_err("No controllers available for species %s\n", name);
		return NULL;
	}
	zc->Run(p);
	return zc;
}

/**
 * Run a controller for every member of my population.
 */
void
ZooSpecies::RunAll( void )
{
	int p;

	for (p = 0; p < population_size; ++p) {
		if (port_list[p] <= 0) continue;

		rmap_t *rp = zoo->FindRobot(port_list[p]);
		rp->controller = Run(port_list[p]);
	}
}

/**
 * Kill the controller on every member of my population.
 */
void
ZooSpecies::KillAll( void )
{
	int p;

	for (p = 0; p < population_size; ++p) {
		rmap_t *rp = zoo->FindRobot(port_list[p]);
		if (!rp || !rp->controller) continue;
		rp->controller->Kill();
		rp->controller = NULL;
	}
}

/**
 * Pick a controller from the pool.
 * Deterministic; uses a round-robin scheme with priority.
 */
ZooController *
ZooSpecies::SelectController( void )
{
	int n;
	ZooController *zc;

	n = controller.size();

	if (n == 0) return NULL;

	zc = &controller[next_controller];
	if (controller_instance++ == zc->GetFrequency()) {
		controller_instance = 0;
		next_controller = ++next_controller % n;
	}
	
	return zc;
}

/**
 * Determine whether the robot on this port is in the population of this
 * species.
 */
bool
ZooSpecies::Hosts( int p )
{
	for (int i=0; i < population_size; ++i)
		if (port_list[i] == p)
			return true;

	return false;
}

/**
 * Dump out details about this species.
 */
void
ZooSpecies::print( void )
{
	printf("Zoo species %s = { ", name);
	for (int i = 0; i < population_size; ++i)
		printf("%s(%d) ", model_list[i], port_list[i]);
	printf("}\n");
}

/**
 *  @}
 * @}
 */

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
 *  \defgroup ZooReferee
 *  @{
 * Description: An addon to the Stage plugin for Zoo functionality.
 * Author: Adam Lein
 * Date: July 14, 2005
 */

#include "zoo_referee.h"
#include "zoo_driver.h"

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

#define ZOOREF_CREATE "zooref_create"
#define ZOOREF_SCORE_FIG_NAME "zooref_score_fig"
typedef ZooReferee *(*zooref_create_t)(ConfigFile *, int, ZooDriver *);

/************ Referee *************/

ZooDriver *ZooReferee::zoo = NULL;

/**
 * ZooReferee constructor.
 * @param zd the ZooDriver I will be refereeing for.
 */
ZooReferee::ZooReferee( ConfigFile *cf, int section, ZooDriver *zd )
{
	zoo = zd;
}

/**
 * Called by Zoo when everything is ready to start.  I should use this
 * opportunity to start controllers for the initial batch of robots.
 *
 * I also like to draw little green circles around robots when a controller
 * is connected; attach a callback for this purpose here.
 */
void
ZooReferee::Startup( void )
{
	int i;
	for (i = 0; i < zoo->GetModelCount(); ++i) {
		stg_model_t *mod = zoo->GetModelByIndex(i);
		printf("Zoo: model %s has id %d\n", mod->token, mod->id);

		static stg_msec_t zero=0;
		stg_model_set_property(mod, ZOO_RUN_IND_PROP,
			&zero, sizeof(stg_msec_t));
		stg_model_add_property_callback(mod, "pose",
			(stg_property_callback_t)draw_run_ind_cb, NULL);
	}
	zoo->RunAll();
}

/**
 * Stock callback for drawing (arrays of) ints.
 * @param siz (number of elements) * sizeof(int)
 */
int
ZooReferee::draw_int_cb( stg_model_t *mod, const char *propname, 
                         const int *data, size_t siz )
{
	stg_rtk_fig_t *fig;

	/* Get an object to draw the score on, or create one if it doesn't
	 * exist. */
	fig = stg_model_get_fig(mod, ZOOREF_SCORE_FIG_NAME);
	if (!fig) {
		fig = stg_model_fig_create(mod, ZOOREF_SCORE_FIG_NAME, NULL,
			STG_LAYER_USER);
		stg_color_t *color = (stg_color_t *)
			stg_model_get_property_fixed(mod, "color", sizeof(stg_color_t));
		stg_rtk_fig_color_rgb32(fig, *color);
	}

	/* if the robot is uncontrolled, don't print the score */
	if (!mod->subs) return 0;

	/* what to draw */
	static char *buf=NULL, *cp;
	if (buf) free(buf); /* attempt to avoid leaks */
	buf = (char *)calloc(16*siz/sizeof(int), sizeof(char));
	strcpy(buf, "[ ");
	cp = buf+2;
	for (unsigned int i=0; i < siz/sizeof(int); ++i) {
		cp += snprintf(cp, 32, "%d", data[i]);
		if (i == siz/sizeof(int)-1)
			strcpy(cp, " ]");
		else
			strcpy(cp, ",\n  ");
		cp += 4;
	}

	/* where to print it */
	stg_pose_t pose;
	stg_model_get_global_pose(mod, &pose);
	stg_rtk_fig_origin(fig, pose.x, pose.y, 0);

	/* draw it, under and to the left of the position model */
	stg_rtk_fig_text(fig, -0.75, -0.75, 0.0, buf);

	/* keep marshalling callbacks */
	return 0;
}

/**
 * Stock callback for drawing (arrays of) doubles.
 * @param siz (number of elements) * sizeof(double)
 */
int
ZooReferee::draw_double_cb( stg_model_t *mod, const char *propname, 
                         const double *data, size_t siz )
{
	stg_rtk_fig_t *fig;

	/* Get an object to draw the score on, or create one if it doesn't
	 * exist. */
	fig = stg_model_get_fig(mod, ZOOREF_SCORE_FIG_NAME);
	if (!fig) {
		fig = stg_model_fig_create(mod, ZOOREF_SCORE_FIG_NAME, NULL,
			STG_LAYER_USER);
		stg_color_t *color = (stg_color_t *)
			stg_model_get_property_fixed(mod, "color", sizeof(stg_color_t));
		stg_rtk_fig_color_rgb32(fig, *color);
	}

	/* if the robot is uncontrolled, don't print the score */
	if (!mod->subs) return 0;

	/* what to draw */
	static char *buf=NULL, *cp;
	if (buf) free(buf); /* attempt to avoid leaks */
	buf = (char *)calloc(16*siz/sizeof(double), sizeof(char));
	strcpy(buf, "[ ");
	cp = buf+2;
	for (unsigned int i=0; i < siz/sizeof(double); ++i) {
		cp += snprintf(cp, 32, "%g", data[i]);
		if (i == siz/sizeof(double)-1)
			strcpy(cp, " ]");
		else
			strcpy(cp, ",\n  ");
		cp += 4;
	}

	/* where to print it */
	stg_pose_t pose;
	stg_model_get_global_pose(mod, &pose);
	stg_rtk_fig_origin(fig, pose.x, pose.y, 0);

	/* draw it, under and to the left of the position model */
	stg_rtk_fig_clear(fig);
	stg_rtk_fig_text(fig, -0.75, -0.75, 0.0, buf);

	/* keep marshalling callbacks */
	return 0;
}

/**
 * Callback to draw an indicator that a robot's controller has been run or
 * stopped.
 */
int
ZooReferee::draw_run_ind_cb( stg_model_t *mod, const char *propname,
                             const stg_msec_t *datum )
{
	stg_rtk_fig_t *fig;

	/* Get something to draw the score on, or create one if it doesn't
	 * exist. */
	fig = stg_model_get_fig(mod, "zooref_run_ind_fig");
	if (!fig) {
		fig = stg_model_fig_create(mod, "zooref_run_ind_fig", NULL,
			STG_LAYER_USER-1);

		/* properties */
		stg_rtk_fig_linewidth(fig, 2);
	}

	int *killed =
		(int *)stg_model_get_property_fixed(mod, "zoo_killed", sizeof(int));
	if (*killed)
		stg_rtk_fig_color(fig, 1, 0, 0); /* red */
	else
		stg_rtk_fig_color(fig, 0, 1, 0); /* green */
	stg_rtk_fig_clear(fig);

	/* if the robot is uncontrolled, don't draw */
	if (!*killed && !mod->subs) return 0;

	/* if the robot was started/stopped too long ago, don't draw */
	stg_msec_t *starttime = (stg_msec_t *)stg_model_get_property_fixed(mod,
		ZOO_RUN_IND_PROP, sizeof(stg_msec_t));
	if (!starttime || (stg_timenow() - *starttime > ZOO_RUN_IND_TIME))
		return 0;

	/* where to draw */
	stg_pose_t pose;
	stg_model_get_global_pose(mod, &pose);

	/* size of robot */
	stg_geom_t *geom = (stg_geom_t *)stg_model_get_property_fixed(mod,
		"geom", sizeof(stg_geom_t));

	/* draw */
	double m = geom->size.x > geom->size.y ? geom->size.x : geom->size.y;
	stg_rtk_fig_ellipse(fig,
		pose.x, pose.y, 0.0, /* center */
		1.2 * m, 1.2 * m,    /* bounding rectangle */
		0);                  /* (not) filled */

	return 0;
}

/**
 *  @}
 * @}
 */

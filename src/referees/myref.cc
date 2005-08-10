/*
 *  A sample referee for use with Zoo.
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

#include <zoo_driver.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <math.h>
#include <gui.h>

#define MYREF_SCORE_FIG_NAME "myref_score_fig"

/* simply subclass the referee.  I'm going to do some special things in the
 * constructor, and it feels nice to have the pose callback be a static class
 * member. */
class MyRef : public ZooReferee {
public:
	MyRef(ConfigFile *, int, ZooDriver *);
private:
	static int pose_cb(stg_model_t *, const char *, double *, size_t);
	static int score_draw_cb(stg_model_t *, const char *, const double *);
	static ZooDriver *zoo;
};

ZooDriver *MyRef::zoo = NULL; /* just initially; we'll set it to something
                               * appropriate in the constructor. */

/**
 * MyRef::MyRef:  Read a line from the ConfigFile to see that it works, and
 * print it out.  Then add the pose_cb callback to the pose property of every
 * model.
 */
MyRef::MyRef( ConfigFile *cf, int section, ZooDriver *_zoo )
	: ZooReferee(cf, section, _zoo)
{
	/* store a pointer to the driver for later use, such as in loading or
	 * killing controllers, or changing scores. */
	zoo = _zoo;

	/* print a message for demonstration purposes */
	printf("myref_message = %s\n",
		cf->ReadString(section, "myref_message", ""));

	/* attach the pose_cb callback to the pose property.  The NULL is "user
	 * data" that MyRef doesn't use.  Note that Stage automatically attaches
	 * the callback to *every* model in the world; we don't need to do it for
	 * each individual.  We have to case pose_cb because the parameter list
	 * doens't exactly match that of a stg_property_callback_t. */
	stg_world_add_property_callback(StgDriver::world, "pose",
		(stg_property_callback_t)pose_cb, NULL);

	/* set the default score-drawing callback.  Maybe this should be done in
	 * Prepare? */
/* FIXME: goddamned name mangling... */
#if 0
	zoo->SetScoreDrawCB((zooref_score_draw_t)score_draw_cb, NULL);
#else
	int x=0;
	for (int i=0; i < zoo->GetModelCount(); ++i) {
		stg_model_t *mod = zoo->GetModelByIndex(i);
		if (!mod) continue;
		stg_model_set_property(mod, ZOO_SCORE_PROPERTY_NAME, &x, sizeof(int));
		stg_model_add_property_toggles(mod, ZOO_SCORE_PROPERTY_NAME,
			(stg_property_callback_t)score_draw_cb, (char *)"foo",
			NULL, NULL, ZOO_SCORE_LABEL, 0);
	}
#endif
}

/**
 * MyRef::pose_cb: The pose callback.  This sets the robot's score, in this
 * case, calculated in a toy fashion as an example.
 */
int
MyRef::pose_cb( stg_model_t *mod, const char *prop,
                    double *data, size_t len )
{
	/* set the score of the robot to be the distance from the absolute
	 * origin. */
	double s = sqrt(data[0]*data[0] + data[1]*data[1]);
	double olds;

	zoo->SetScore(mod->token, &s, sizeof(double));

	/* continue marshalling callbacks. */
	return 0;
}

/**
 * MyRef::score_draw_cb: Draw the score on the GUI.  Note the other
 * parameters aren't there because we don't need them.  That's okay, because
 * when we attach the callback, we can just cast the function pointer.
 *
 * Note: the framework for this subroutine was taken from model_position.c,
 * the position_render_data() function, and model_laser.c, the
 * laser_render_data() function.  The latter was for figuring out where to
 * draw the data.
 */
int
MyRef::score_draw_cb( stg_model_t *mod, const char *prop, const double *score )
{
	stg_rtk_fig_t *fig = stg_model_get_fig(mod, MYREF_SCORE_FIG_NAME);

	if (!fig) {
		fig = stg_model_fig_create(mod, MYREF_SCORE_FIG_NAME, NULL,
			STG_LAYER_USER);
		stg_color_t *color = (stg_color_t *)stg_model_get_property_fixed(mod,
			"color", sizeof(stg_color_t));
		stg_rtk_fig_color_rgb32(fig, *color);
	}

	/* if the robot is uncontrolled, don't print the score */
	stg_rtk_fig_clear(fig);
	if (!mod->subs) return 0;

	/* what to draw */
	char buf[256];
	snprintf(buf, sizeof(buf), "<%.2f>", *score);

	/* where to print it */
	stg_pose_t pose;
	stg_model_get_global_pose(mod, &pose);
	stg_rtk_fig_origin(fig, pose.x, pose.y, 0);

	/* draw it, over and to the left of the position model */
	stg_rtk_fig_text(fig, -0.75, -0.75, 0.0, buf);

	/* keep marshalling callbacks */
	return 0;
}

/** 
 * This function will be loaded and called by the ZooDriver when the time is
 * ripe.  It should return a pointer to a ZooReferee object.
 */
extern "C" {
	void *
	zooref_create( ConfigFile *cf, int section, ZooDriver *_zoo )
	{
		/* just pass the arguments along. */
		return new MyRef(cf, section, _zoo);
	}
}

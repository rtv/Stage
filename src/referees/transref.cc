
/*
 *  A sample referee for use with Zoo.
 *  Copyright (C) 2005 Jeremy Holman
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
#include <playerclient.h>

#define REACH 1 /* meters */
#define TRANS_HAS_PUCK "trans_has_puck"
#define TRANS_NEAR_SITE "trans_near_site"

inline bool
closeTo(double x1, double y1, double x2, double y2, double dist)
{
	return ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) < (dist * dist);
}

/* simply subclass the referee.  I'm going to do some special things in the
 * constructor, and it feels nice to have the pose callback be a static class
 * member. */
class TransRef : public ZooReferee {
public:
	TransRef(ConfigFile *, int, ZooDriver *);
	void Startup(void); /* called by Player */
private:
	static int      pose_cb(stg_model_t *, const char *, double *, size_t,
		void *);
	//static ZooDriver      *zoo;

	typedef enum { b_dummy, b_source, b_sink } beacon_type;

	static int      source_count, source_mincap, source_maxcap;
	static int      sink_count, sink_mincap, sink_maxcap;
	static int      total_count;
	static int      site_index[256];
	static int      capacity[256];

	stg_fiducial_return_t
	          makeSiteFID(beacon_type sitemode, int value, int capacity);
	bool            setSiteFID(int siteindex, stg_fiducial_return_t fid);
};

int TransRef::source_count = 0;
int TransRef::source_mincap = 0;
int TransRef::source_maxcap = 0;
int TransRef::sink_count = 0;
int TransRef::sink_mincap = 0;
int TransRef::sink_maxcap = 0;
int TransRef::total_count = 0;
int TransRef::site_index[] = { 0 };
int TransRef::capacity[] = { 0 };

/**
 * TransRef::TransRef:  Read a line from the ConfigFile to see that it works, and
 * print it out.  Then add the pose_cb callback to the pose property of every
 * model.
 */
TransRef::TransRef(ConfigFile * cf, int section, ZooDriver * _zoo)
	: ZooReferee(cf, section, _zoo)
{
	/* store a pointer to the driver for later use, such as in loading or
	 * killing controllers, or changing scores. */
	zoo = _zoo;

	/* print a message for demonstration purposes */
	printf("although this isn't Adam's dummy ref, myref_message = %s\n",
		cf->ReadString(section, "myref_message", "omg, no message found"));

	/* get the parameters for updating sources and sinks, and 
	 * perfunctorily report having done so. */
	source_count = cf->ReadInt(section, "transref_source_count", 1);
	source_mincap = cf->ReadInt(section, "transref_source_mincap", 1);
	source_maxcap = cf->ReadInt(section, "transref_source_maxcap", 10);
	printf("Source parameters: %d/%d/%d\n", source_count, source_mincap,
		source_maxcap);
	sink_count = cf->ReadInt(section, "transref_sink_count", 1);
	sink_mincap = cf->ReadInt(section, "transref_sink_mincap", 1);
	sink_maxcap = cf->ReadInt(section, "transref_sink_maxcap", 10);
	printf("Sink parameters: %d/%d/%d\n", sink_count, sink_mincap,
		sink_maxcap);

	/* randomize a list of the first N integers, where N is the number of
 	 * sources and sinks.  The first source_count will be sources; the
 	 * remaining will be sinks. */
	total_count = source_count + sink_count;
	int i, a;
	for (i=0; i < 256; ++i) site_index[i] = i;
	for (i=0; i < total_count; ++i) {
		int j = random() % (total_count-i);
		a = site_index[i];
		site_index[i] = site_index[i+j];
		site_index[i+j] = a;
	}
	printf("\nTransRef: Sources are: ");
	for (i=0; i < source_count; ++i) {
		setSiteFID(site_index[i], makeSiteFID(b_source, i, source_maxcap));
		capacity[site_index[i]] = source_maxcap;
		printf("site%02d ", site_index[i]);
	}
	printf("\nTransRef: Sinks are: ");
	for (; i < total_count; ++i) {
		setSiteFID(site_index[i], makeSiteFID(b_sink, 0, 0));
		printf("site%02d ", site_index[i]);
	}
	for (; i < zoo->GetModelCount(); ++i)
		setSiteFID(site_index[i], makeSiteFID(b_dummy, 0, 0));
	putchar('\n');

	/* attach the pose_cb callback to the pose property.  The NULL is "user
	 * data" that TransRef doesn't use.  Note that Stage automatically attaches
	 * the callback to *every* model in the world; we don't need to do it for
	 * each individual.  We have to case pose_cb because the parameter list
	 * doens't exactly match that of a stg_property_callback_t. */
	stg_world_add_property_callback(StgDriver::world, "pose",
		(stg_property_callback_t) pose_cb, NULL);
}

void
TransRef::Startup( void )
{
	int zero=0;
	bool near_site[256];

	for (int i=0; i < 256; ++i)
		near_site[i] = false;

	for (int i=0; i < zoo->GetModelCount(); ++i) {
		stg_model_t *mod = zoo->GetModelByIndex(i);
		stg_model_set_property(mod, ZOO_SCORE_PROPERTY_NAME,
			&zero, sizeof(int));
		stg_model_set_property(mod, TRANS_HAS_PUCK,
			&zero, sizeof(bool));
		stg_model_set_property(mod, TRANS_NEAR_SITE,
			near_site, total_count*sizeof(bool));
	}

#if 0
	if (!fork()) {
		PlayerClient pc("localhost", 6670);
		FiducialProxy **ff = new FiducialProxy*[total_count];
		for (int i=0; i < total_count; ++i)
			ff[i] = new FiducialProxy(&pc, i, 'r');
		while(1) sleep(1);
		exit(0);
	}
#endif

	zoo->SetScoreDrawCB((zooref_score_draw_t)draw_int_cb, NULL);
	zoo->RunAll();
}

/**
 * TransRef::pose_cb: The pose callback.  Just print the new value.  Note that
 * the formal parameter list doesn't exactly match stg_property_callback_t
 * in that the user_data pointer is absent.  Doesn't matter, as long as the
 * formal parameter list is a prefix (w.r.t. types) of that of
 * stg_property_callback_t.
 */
int
TransRef::pose_cb(stg_model_t * mod, const char *prop,
	double *data, size_t len, void *userdata)
{
	/* current score */
	int *sp = (int *)stg_model_get_property_fixed(mod,
		ZOO_SCORE_PROPERTY_NAME, sizeof(int));
	if (!sp) /* not a robot */
		return 0;
	int score = *sp;

	/* determine whether this robot has a puck */
	bool *hpp = (bool *)stg_model_get_property_fixed(mod,
		TRANS_HAS_PUCK, sizeof(bool));
	if (!hpp) /* not a robot */
		return 0;
	bool has_puck = *hpp;

	/* determine whether this robot *was* near each non-dummy site */
	bool *near_site = (bool *)stg_model_get_property_fixed(mod,
		TRANS_NEAR_SITE, total_count*sizeof(bool));

	/* determine if the nearness changed for any non-dummy site */
	int i, j;
	double d;
	stg_pose_t *pos=(stg_pose_t *)data, *sitepos;
	char sitename[32];
	for (i=0; i < total_count; ++i) {
		/* some situations are not interesting */
		if (has_puck && i < source_count /* near a useless source */
		 || !has_puck && i >= source_count) /* near a useless sink */
			continue;

		j = site_index[i];

		/* otherwise, determine how close we are */
		sprintf(sitename, "site%02d", j);
		stg_model_t *site = zoo->GetModelByName(sitename);
		if (!site) continue;
		sitepos = (stg_pose_t *)stg_model_get_property_fixed(site,
			"pose", sizeof(stg_pose_t));
		if (!sitepos) continue;

		/* if we're entering a radius, either pick up or drop off a puck */
		d = hypot(sitepos->x-pos->x, sitepos->y-pos->y);
		if (d < REACH && !near_site[i]) {
			near_site[i] = true;
			if (i < source_count && capacity[j] != 0) {
				has_puck = true;
				capacity[j]--;
				printf("TransRef: %s picked up a puck from %s\n",
					mod->token, sitename);
			} else if (i >= source_count) {
				has_puck = false;
				++score;
				printf("TransRef: %s dropped off a puck at %s\n",
					mod->token, sitename);
			}
		} else if (d > REACH)
			near_site[i] = false;
	}

	/* update state */
	stg_model_set_property(mod, TRANS_HAS_PUCK,
		&has_puck, sizeof(bool));
	stg_model_set_property(mod, TRANS_NEAR_SITE,
		near_site, total_count*sizeof(bool));

	/* update score */
	stg_model_set_property(mod, ZOO_SCORE_PROPERTY_NAME,
		&score, sizeof(int));

	/* continue marshalling callbacks. */
	return 0;
}

union site {
	struct {
		unsigned int mode:2, value:15, capacity:15;
	} site_struct;
	unsigned int site_packed;
};

stg_fiducial_return_t
TransRef::makeSiteFID(beacon_type sitemode, int value, int capacity)
{
#if 0
	switch(sitemode) {
	case b_dummy:
		return 0x40;
	case b_source: case b_sink:
		return (0x80 + (sitemode==b_sink)*0x40
		             + value*16+capacity);
	}
#endif
	union site s = { sitemode, value, capacity };
	return s.site_packed;
}

bool
TransRef::setSiteFID(int siteindex, stg_fiducial_return_t fid)
{
	char sitename[32];
	rmap_t *rp;
	stg_model_t *mod;

	sprintf(sitename, "site%02d", siteindex);
	rp = zoo->FindRobot(sitename);
	mod = zoo->GetModelByName(sitename);
	//if (!rp || !rp->model) {
	if (!mod) {
		zoo_err("Something went wrong; seems %s isn't valid.\n", sitename);
		return false;
	}

	//stg_model_set_property(rp->model, "fiducial_return",
	stg_model_set_property(mod, "fiducial_return",
		&fid, sizeof(stg_fiducial_return_t));

	return true;
}

/** 
 * This function will be loaded and called by the ZooDriver when the time is
 * ripe.  It should return a pointer to a ZooReferee object.
 */
extern "C" {
	void *zooref_create(ConfigFile * cf, int section, ZooDriver * _zoo)
	{
		return new TransRef(cf, section, _zoo);
			/* just pass the arguments along. */
	}
}

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

/* simply subclass the referee.  I'm going to do some special things in the
 * constructor, and it feels nice to have the pose callback be a static class
 * member. */
class MyRef : public ZooReferee {
public:
	MyRef(ConfigFile *, int, ZooDriver *);
private:
	static int pose_cb(stg_model_t *, const char *, double *, size_t);
	ZooDriver *zoo;
};

/**
 * MyRef::MyRef:  Read a line from the ConfigFile to see that it works, and
 * print it out.  Then add the pose_cb callback to the pose property of every
 * model.
 */
MyRef::MyRef( ConfigFile *cf, int section, ZooDriver *_zoo )
	: ZooReferee(cf, section, _zoo)
{
	printf("myref_message = %s\n",
		cf->ReadString(section, "myref_message", ""));
	zoo = _zoo;

	stg_world_add_property_callback(StgDriver::world, "pose",
		(stg_property_callback_t)pose_cb, NULL);
}

/**
 * MyRef::pose_cb: The pose callback.  Just print the new value.
 */
int
MyRef::pose_cb( stg_model_t *mod, const char *prop,
                    double *data, size_t len )
{
	printf("Zoo: Model %s now has pose [%.2f, %.2f, %.3f]\n",
		mod->token, data[0], data[1], data[2]);
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
		return new MyRef(cf, section, _zoo);
	}
}

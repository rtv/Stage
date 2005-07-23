#include <zoo_driver.h>
#include <stdio.h>

class MyRef : public ZooReferee {
public:
	MyRef(ConfigFile *, int, ZooDriver *);
private:
	static int pose_cb(stg_model_t *, const char *, double *, size_t);
	ZooDriver *zoo;
};

MyRef::MyRef( ConfigFile *cf, int section, ZooDriver *_zoo )
	: ZooReferee(cf, section, _zoo)
{
	printf("myref_message = %s\n",
		cf->ReadString(section, "myref_message", ""));
	zoo = _zoo;

	stg_world_add_property_callback(StgDriver::world, "pose",
		(stg_property_callback_t)pose_cb, NULL);
}

int
MyRef::pose_cb( stg_model_t *mod, const char *prop,
                    double *data, size_t len )
{
	printf("Zoo: Model %s now has pose [%.2f, %.2f, %.3f]\n",
		mod->token, data[0], data[1], data[2]);
	return 0;
}

extern "C" {
	void *
	zooref_create( ConfigFile *cf, int section, ZooDriver *_zoo )
	{
		return new MyRef(cf, section, _zoo);
	}
}

#include <zoo_driver.h>
#include <stdio.h>

class MyRef : public ZooReferee {
public:
	MyRef(ConfigFile *, int, ZooDriver *);
};

MyRef::MyRef( ConfigFile *cf, int section, ZooDriver *_zoo )
	: ZooReferee(cf, section, _zoo)
{
	printf("myref_message = %s\n", cf->ReadString(section, "myref_message", ""));
	//stg_err(cf->ReadString(section, "myref_message", ""));
	fputc('\n', stderr);
}

extern "C" {
	void *
	zooref_create( ConfigFile *cf, int section, ZooDriver *_zoo )
	{
		return new MyRef(cf, section, _zoo);
	}
}

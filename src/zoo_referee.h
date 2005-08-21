#ifndef _STAGE_ZOO_REFEREE_H
#define _STAGE_ZOO_REFEREE_H

#include "p_driver.h"
#include <limits.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <stdio.h>
#include "zoo.h"

/* some forward declarations here */
class ZooSpecies;
class ZooController;
class ZooDriver;
class ZooReferee;

class ZooReferee
{
public:
	ZooReferee(ConfigFile *, int, ZooDriver *);
	virtual void Startup(void);
	static int draw_int_cb(stg_model_t *, const char *pname,
		const int *sdata, size_t siz );
	static int draw_double_cb(stg_model_t *, const char *pname,
		const double *sdata, size_t siz );
	static int draw_run_ind_cb(stg_model_t *, const char *pname,
		const stg_msec_t *data);
protected:
	static ZooDriver *zoo;
};

#endif

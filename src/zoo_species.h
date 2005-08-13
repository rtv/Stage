#ifndef _STAGE_ZOO_SPECIES_H
#define _STAGE_ZOO_SPECIES_H

#include "p_driver.h"
#include <limits.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <stdio.h>

#include "zoo.h"

#define ZOO_SCORE_BUFFER_SIZE	256
#define ZOO_SCORE_PROPERTY_NAME "zoo_score"
#define ZOO_SCORE_LABEL "score from Zoo"

/* some forward declarations here */
class ZooSpecies;
class ZooController;
class ZooDriver;
class ZooReferee;

class ZooSpecies
{
public:
	ZooSpecies(void);
	ZooSpecies(ConfigFile *cf, int section, ZooDriver *);
	~ZooSpecies();
	ZooController *Run(int);
	void RunAll(void);
#if 0
	void Kill(int);
#endif
	void KillAll(void);
	ZooController *SelectController(void);
	bool Hosts(int);
	void print(void);

	void SetScoreDrawCB(zooref_score_draw_t, void *userdata);
	zooref_score_draw_t score_draw_cb;
	void *score_draw_user_data;

	const char *name;
private:
	int population_size;
	int *port_list;
	char **model_list;
	std::vector<ZooController> controller;

	/* used by SelectController */
	int next_controller;
	int controller_instance; // for frequency > 1

	ZooDriver *zoo;
};

#endif

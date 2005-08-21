#ifndef _STAGE_ZOO_DRIVER_H
#define _STAGE_ZOO_DRIVER_H

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

/* for registering the driver */
void ZooDriver_Register(DriverTable *);

class ZooDriver : public Driver
{
public:
	ZooDriver(ConfigFile *cf, int section);
	~ZooDriver();

	ZooSpecies *GetSpeciesByName(const char *);

	/* Player calls these */
	int Setup(void);
	int Shutdown(void);
	void Prepare(void);

	/* Functions for starting and stopping controllers */
	void Run(int);
	void Run(const char *);   // accepts a model name
	void RunAll(void);
	void Kill(int);
	void Kill(const char *);  // accepts a model name
	void KillAll(void);

	/* Model access functions.  Provide a variety of ways of getting a
	 * variety of representations of a robot: a stg_model_t object, a string
	 * holding the name, the index in a list kept by Zoo, or port. */
	int GetModelCount(void);
	const char *GetModelNameByIndex(int);
	const char *GetModelNameByPort(int);
	stg_model_t *GetModelByName(const char *);
	stg_model_t *GetModelByIndex(int);
	stg_model_t *GetModelByPort(int);
	int GetModelPortByName(const char *);

	/* Functions for storing and retrieving arbitrary score data */
	int GetScore(const char *, void *);
	int GetScoreSize(const char *);
	int SetScore(const char *, void *, size_t);
	int ClearScore(const char *);
	void SetScoreDrawCB(zooref_score_draw_t, void *userdata);
		// for every species

	/* New interface for looking up robots */
	rmap_t *FindRobot(const char *modelName);
	rmap_t *FindRobot(int port);
	rmap_t *FindRobot(ZooController *);
	rmap_t *FindRobot(stg_model_t *);

private:
	int species_count;
	ZooSpecies **species;

	int robot_count; /* could be model_count or port_count */
	rmap_t *robotMap; // port --> ZooController
	std::vector<const char *> modelList;

	void *zooref_handle; // for dynamically linked ref
	ZooReferee *referee;
};

#endif

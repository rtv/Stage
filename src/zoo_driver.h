#ifndef _STAGE_ZOO_DRIVER_H
#define _STAGE_ZOO_DRIVER_H

#include "p_driver.h"
#include <limits.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <stdio.h>

#define ZOO_SCORE_BUFFER_SIZE	256
#define ZOO_SCORE_PROPERTY_NAME "zoo_score"
#define ZOO_SCORE_LABEL "score from Zoo"

/* some forward declarations here */
class ZooSpecies;
class ZooController;
class ZooDriver;
class ZooReferee;

/* referee stuff */
typedef int (*zooref_init_t)(ConfigFile *, int, ZooDriver *);
typedef int (*zooref_score_draw_t)(stg_model_t *, const char *propname,
                         const void *sdata, size_t, void *, int mindex);

/* for registering the driver */
void ZooDriver_Register(DriverTable *);

/* general stuff */
int zoo_err(const char *, int ...);

/* maps */
typedef struct {
	const char *model_name;
	int port;
	ZooController *controller;
} rmap_t;

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

	rmap_t *FindRobot(const char *modelName);
	rmap_t *FindRobot(int port);
	rmap_t *FindRobot(ZooController *);

private:
	int species_count;
	ZooSpecies **species;

	int robot_count; /* could be model_count or port_count */
	rmap_t *robotMap; // port --> ZooController
	std::vector<const char *> modelList;

	void *zooref_handle; // for dynamically linked ref
	ZooReferee *referee;
};

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

class ZooController
{
public:
	ZooController(ConfigFile *cf, int section, ZooSpecies *sp);
	~ZooController();

	void Run(int port);
	void Kill(void);
	inline int GetFrequency(void) { return frequency; }
	inline const char *GetCommand(void) { return command; }

#if 0
	/* A controller can be a member of more than one species, and each
	 * species may have its own way of keeping score, so each controller
	 * should maintain a map from species names to scores */
	std::map< const char *, void * > scoreMap;
	std::map< const char *, size_t > scoreSizeMap;
#endif
	void *score_data;
	int score_size;

	static const char *path;
	ZooSpecies *species;
private:
	pid_t pid;
	int frequency;
	const char *command;
};

class ZooReferee
{
public:
	ZooReferee(ConfigFile *, int, ZooDriver *);
	void Startup(void);
	static int draw_int_cb(stg_model_t *, const char *pname,
		const int *sdata, size_t siz );
	static int draw_double_cb(stg_model_t *, const char *pname,
		const double *sdata, size_t siz );
private:
	ZooDriver *zoo;
};

#endif

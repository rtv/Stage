#ifndef _STAGE_ZOO_DRIVER_H
#define _STAGE_ZOO_DRIVER_H

#include "p_driver.h"
#include <limits.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <stdio.h>

#define ZOO_SCORE_BUFFER_SIZE	256

/* some forward declarations here */
class ZooSpecies;
class ZooController;
class ZooDriver;
class ZooReferee;

/* referee stuff */
typedef int (*zooref_init_t)(ConfigFile *, int, ZooDriver *);
typedef int (*zooref_score_printer_t)(char *, const void *, size_t, void *);

/* so I don't have to keep typing it */
typedef std::map< int, ZooController * > cmap_t;

/* for registering the driver */
void ZooDriver_Register(DriverTable *);

/* general stuff */
int zoo_err(const char *, int ...);

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

private:
	int species_count;
	ZooSpecies **species;

	cmap_t controllerMap; // port --> ZooController
	std::map<const char *, int> portMap; // stg_model_t.token --> port
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
	void RunAll(cmap_t &cMap);
#if 0
	void Kill(int);
#endif
	void KillAll(cmap_t &cMap);
	ZooController *SelectController(void);
	bool Hosts(int);
	void print(void);

#if 0
	void SetScorePrintFunction(zooref_score_printer_t, void *);
	void PrintScore(FILE *fp); /* Print the score for every member of this
	                            * species. */
#endif
	const char *name;
private:
	int population_size;
	int *port_list;
	char **model_list;
	std::vector<ZooController> controller;

	/* used by SelectController */
	int next_controller;
	int controller_instance; // for frequency > 1

#if 0
	zooref_score_printer_t score_printer;
	void *score_printer_user_data;
#endif
};

class ZooController
{
public:
	ZooController(ConfigFile *cf, int section);
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
private:
	ZooDriver *zoo;
};

#endif

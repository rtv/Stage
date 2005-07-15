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

class ZooDriver : public Driver
{
public:
	ZooDriver(ConfigFile *cf, int section);
	~ZooDriver();

	ZooSpecies *GetSpeciesByName(const char *);

	int Setup(void);
	int Shutdown(void);
	void Prepare(void);

	void Run(int);
	void Run(const char *);   // accepts a model name
	void RunAll(void);
	void Kill(int);
	void Kill(const char *);  // accepts a model name
	void KillAll(void);

	int GetModelCount(void);
	const char *GetModelName(int);
	stg_model_t *GetModel(const char *);
	stg_model_t *GetModel(int);

	int GetScore(const char *, const char *, void *, size_t *siz=NULL);
	void SetScore(const char *, const char *, void *, size_t);

	void PrintScore(const char *filename);
	void SetScorePrintFunction(const char *, zooref_score_printer_t, void *);
private:
	int species_count;
	ZooSpecies **species;

	cmap_t controllerMap;
	std::map<const char *, int> portMap;
	std::vector<const char *> modelList;

	ZooReferee *referee;
};

class ZooSpecies
{
public:
	ZooSpecies(void);
	ZooSpecies(ConfigFile *cf, int section);
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

	void SetScorePrintFunction(zooref_score_printer_t, void *);
	void PrintScore(FILE *fp); /* Print the score for every member of this
	                            * species. */
	const char *name;
private:
	int range_count;
	int *min_port;
	int *max_port;
	std::vector<ZooController> controller;

	zooref_score_printer_t score_printer;
	void *score_printer_user_data;
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

	/* A controller can be a member of more than one species, and each
	 * species may have its own way of keeping score, so each controller
	 * should maintain a map from species names to scores */
	std::map< const char *, void * > scoreMap;
	std::map< const char *, size_t > scoreSizeMap;

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

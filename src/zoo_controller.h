#ifndef _STAGE_ZOO_CONTROLLER_H
#define _STAGE_ZOO_CONTROLLER_H

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

class ZooController
{
public:
	ZooController(ConfigFile *cf, int section, ZooDriver *zd, ZooSpecies *sp);
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
	ZooDriver *zoo;
private:
	pid_t pid;
	int frequency;
	const char *command;
	const char *outfilename, *outfilemode;
	const char *errfilename, *errfilemode;
	void cpathprintf(char *, const char *fmt, const rmap_t *);
};

#endif

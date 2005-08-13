#ifndef _STAGE_ZOO_H
#define _STAGE_ZOO_H

#include <player/configfile.h>
#include <stage.h>

class ZooDriver;
class ZooSpecies;
class ZooController;
class ZooReferee;

#define ZOO_DRIVER_NAME "zoo"
#define ZOO_SPECIES_SECTYPE "species"
#define ZOO_CONTROLLER_SECTYPE "controller"

#define ZOO_DEFAULT_SPECIES "planktron"
#define ZOO_DEFAULT_FREQUENCY 1

#define ZOO_DIAGNOSTIC_LEVEL 5
#define ZOO_DEBUGMSG(f, a, b) PLAYER_MSG2(ZOO_DIAGNOSTIC_LEVEL, f, a, b)

/* general stuff */
extern "C" { int zoo_err(const char *, ...); }

extern bool quiet_startup;

/* maps */
typedef struct {
	const char *model_name;
	int port;
	ZooController *controller;
} rmap_t;

/* referee stuff */
#define ZOOREF_CREATE "zooref_create"
#define ZOOREF_SCORE_FIG_NAME "zooref_score_fig"
typedef int (*zooref_init_t)(ConfigFile *, int, ZooDriver *);
typedef int (*zooref_score_draw_t)(stg_model_t *, const char *propname,
                         const void *sdata, size_t, void *, int mindex);
typedef ZooReferee *(*zooref_create_t)(ConfigFile *, int, ZooDriver *);

#include "zoo_driver.h"
#include "zoo_species.h"
#include "zoo_controller.h"
#include "zoo_referee.h"

#endif

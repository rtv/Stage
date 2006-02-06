#ifndef _STAGE_ZOO_H
#define _STAGE_ZOO_H

#include <libplayercore/playercore.h>
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

#define ZOO_RUN_IND_PROP "zoo_run"
#define ZOO_RUN_IND_TIME 5000 /* milliseconds */

/* general stuff */
extern "C" { int zoo_err(const char *, ...); }

extern bool player_quiet_startup;

/* maps */
typedef struct {
	const char *model_name;
	int port;
	ZooController *controller;
	stg_model_t *model;
} rmap_t;

/* referee stuff */
#define ZOOREF_CREATE "zooref_create"
#define ZOOREF_SCORE_FIG_NAME "zooref_score_fig"
#define ZOO_SCORE_BUFFER_SIZE	256
#define ZOO_SCORE_PROPERTY_NAME "zoo_score"
#define ZOO_SCORE_LABEL "score from Zoo"
typedef int (*zooref_init_t)(ConfigFile *, int, ZooDriver *);
typedef int (*zooref_score_draw_t)(stg_model_t *, const char *propname,
                         const void *sdata, size_t, void *, int mindex);
typedef ZooReferee *(*zooref_create_t)(ConfigFile *, int, ZooDriver *);

#include "zoo_driver.h"
#include "zoo_species.h"
#include "zoo_controller.h"
#include "zoo_referee.h"

/* FIXME: what happened to properties? */
#define stg_model_set_property(a,b,c,d)
#define stg_model_get_property_fixed(a,b,c) (0)
#define stg_model_add_property_toggles(a,b,c,d,e,f,g,h)
#define stg_model_add_property_callback(a,b,c,d)
typedef stg_model_callback_t stg_property_callback_t;

#endif

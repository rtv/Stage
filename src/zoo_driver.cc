/*
 *  Zoo driver for Player
 *  Copyright (C) 2005 Adam Lein
 *                      
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 */

/// \defgroup zoo Zoo

/**
 * @{
 * Description: An addon to the Stage plugin for Zoo functionality.
 * Author: Adam Lein
 * Date: July 14, 2005
 */

#define _GNU_SOURCE

#include "zoo_driver.h"

#include <player/devicetable.h>
#include <player/error.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <glib.h>
#include <wordexp.h>
#include <stdio.h>
#include <dlfcn.h>
#include <gui.h>

#define ZOO_DRIVER_NAME "zoo"
#define ZOO_SPECIES_SECTYPE "species"
#define ZOO_CONTROLLER_SECTYPE "controller"

#define ZOOREF_CREATE "zooref_create"
#define ZOOREF_SCORE_FIG_NAME "zooref_score_fig"
typedef ZooReferee *(*zooref_create_t)(ConfigFile *, int, ZooDriver *);

#define ZOO_DEFAULT_SPECIES "planktron"
#define ZOO_DEFAULT_FREQUENCY 1

#define ZOO_DIAGNOSTIC_LEVEL 5
#define ZOO_DEBUGMSG(f, a, b) PLAYER_MSG2(ZOO_DIAGNOSTIC_LEVEL, f, a, b)

extern bool quiet_startup;
extern DeviceTable *deviceTable;

/********* Driver ********/

/**
 * Called by player_driver_init, defined in p_driver.cc.  Just creates a new
 * ZooDriver object.
 * @param cf the current Player config file
 * @param section the section that ended up causing this Zoo to be created.
 * @return a pointer to the Player driver object.
 */
Driver *
ZooDriver_Init( ConfigFile *cf, int section )
{
	return (Driver *)(new ZooDriver(cf, section));
}

/**
 * Register the name of this driver, along with the function that creates
 * it, with Player's driver table.;
 * @param table Player's driver table
 */
void
ZooDriver_Register( DriverTable *table )
{
	printf("\n ** Zoo plugin v%s **", PACKAGE_VERSION);

	if (!quiet_startup) {
		printf("\n * Copyright 2005 Adam Lein *\n");
	}

	table->AddDriver(ZOO_DRIVER_NAME, ZooDriver_Init);
}

/********* General *********/

extern "C" {
/**
 * A variable-argument function that Zoo routines should call to report
 * errors that wouldn't stop the simulation (use stg_err for ones that
 * would).  The parameters work just like printf.
 */
	int
	zoo_err( const char *fmt, ... )
	{
		va_list va;
		char fmt2[1024];
		int r;

		sprintf(fmt2, "Zoo: %s", fmt);
		va_start(va, fmt);
		r = vfprintf(stderr, fmt2, va);
		va_end(va);

		return r;
	}
}

/********* ZooDriver *********/

/**
 * ZooDriver constructor.  Load all the species.
 * TODO: The Player config-file reader should really have a way of getting
 * all child sections of a given section.  Then the species and controller
 * sections could be included directly in the zoo section, and this would be
 * syntactically meaningful.
 */
ZooDriver::ZooDriver( ConfigFile *cf, int section )
	: Driver(cf, section)
{
	/* set me up as a valid player interface... or something */
	player_device_id_t devid;
	if (cf->ReadDeviceId(&devid, section, "provides", 0, 0, NULL))
		zoo_err("Zoo is confused\n");
	AddInterface(devid, PLAYER_ALL_MODE, 0, 0, 0, 0);

	/* count the robots */
	robot_count = 0;
	for (int i=0; i < cf->GetSectionCount(); ++i) {
		/* must be a model provided by the stage driver */
		if (strcmp(cf->GetSectionType(i), "driver")
		 || strcmp(cf->ReadString(i, "name", ""), "stage"))
			continue;

		/* must have a name */
		const char *name = cf->ReadString(i, "model", "");
		if (!name[0]) continue;

		/* must be included in the population of some species known to
		 * this Zoo. */
		for (int j=0; j < cf->GetSectionCount(); ++j) {
			if ((cf->GetSectionParent(j) != section)
			 || strcmp(cf->GetSectionType(j), ZOO_SPECIES_SECTYPE))
				continue;
			for (int k=0; k < cf->GetTupleCount(j, "population"); ++k)
				if (!strcmp(cf->ReadTupleString(j, "population", k, ""),
				            name))
					++robot_count;
			} /* for(all sections) - search for robot in pop'ns */
	} /* for(all sections) -- count the robots */
	robotMap = (rmap_t *)calloc(robot_count, sizeof(rmap_t));

	/* build a model-to-port mapping */
	int k=0;
	for (int i = 0; i < cf->GetSectionCount(); ++i) {
		/* only care about devices provided by stage */
		if (strcmp(cf->GetSectionType(i), "driver")
		 || strcmp(cf->ReadString(i, "name", ""), "stage"))
			continue;

		/* assuming all devices are on the same port, just get the port of
		 * the first device */
		int port = strtol(cf->ReadTupleString(i, "provides", 0, ""), NULL, 0);

		/* get the model name */
		const char *modelname = cf->ReadString(i, "model", NULL);
		if (!modelname) continue;

		/* must be included in the population of some species known to
		 * this Zoo. */
		int is_in_a_pop=0;
		for (int j=0; j < cf->GetSectionCount(); ++j) {
			if ((cf->GetSectionParent(j) != section)
			 || strcmp(cf->GetSectionType(j), "species"))
				continue;
			for (int k=0; k < cf->GetTupleCount(j, "population"); ++k)
				if (!strcmp(cf->ReadTupleString(j, "population", k, ""),
				            modelname))
					is_in_a_pop = 1;
		}
		if (!is_in_a_pop)
			continue;

		/* add the mapping */
		printf("Zoo: MODELMAP %s --> %d\n", modelname, port);
		robotMap[k].port = port;
		robotMap[k++].model_name = strdup(modelname);
	} /* for(all sections) */

	/* find all species in the config file: requires a model-to-port mapping
	 * already exists */
	species_count = 0;
	for (int i=0; i < cf->GetSectionCount(); ++i)
		if (!strcmp(cf->GetSectionType(i), ZOO_SPECIES_SECTYPE))
			++species_count; /* count the species */
	species = new ZooSpecies*[species_count];
	int si=0;
	for (int i=0; i < cf->GetSectionCount(); ++i) {
		int pi;

		/* only care about species sections */
		if (strcmp(cf->GetSectionType(i), ZOO_SPECIES_SECTYPE))
			continue;

		/* only care about children of a zoo driver section */
		pi = cf->GetSectionParent(i);
		if (pi < 0
		 || strcmp(cf->GetSectionType(pi), "driver")
		 || strcmp(cf->ReadString(pi, "name", ""), ZOO_DRIVER_NAME))
			continue;

		ZooSpecies *zs = new ZooSpecies(cf, i, this);
		species[si++] = zs;
	}
	/* FIXME: Something very bad happens here.  The port ranges on species
	 * are being overwritten somehow.
	 * Temporary fix: using entirely dynamic memory (i.e.
	 * "ZooSpecies **species" instead of "ZooSpecies *species")works... why?
	 */

	/* TODO: Grab sigint? */
	//signal(SIGINT, sigint_handler);

	/* get the path to find controllers in */
	ZooController::path = cf->ReadString(section, "controllerpath", "");

	/* see if we should load a referee other than the default */
	const char *referee_path;
	referee_path = cf->ReadString(section, "referee", "");
	if (strcmp(referee_path, "")) {
		zooref_create_t zooref_create;

		/* load the dynamic library */
		zooref_handle = dlopen(referee_path, RTLD_LAZY);
		if (!zooref_handle) {
			zoo_err("cannot load referee %s: %s\n",
				referee_path, dlerror());
			goto default_referee;
		}

		/* get the zooref_create function */
		zooref_create = (zooref_create_t)
			dlsym(zooref_handle, ZOOREF_CREATE);
		if (!zooref_create) {
			zoo_err("referee must define %s\n", ZOOREF_CREATE);
			dlclose(zooref_handle);
			goto default_referee;
		}

		/* create the referee */
		referee = zooref_create(cf, section, this);
	} else /* no referee_path */
default_referee:
		referee = new ZooReferee(cf, section, this);
}

/**
 * ZooDriver destructor.  Deletes the species list and robot map entries.
 */
ZooDriver::~ZooDriver()
{
	for (int i = 0; i < species_count; ++i)
		delete species[i];
	delete species;

	for (int i=0; i < robot_count; ++i)
		free((char *)robotMap[i].model_name);
	free(robotMap);
}

/**
 * Find a robot in the robot map, searching by model name.
 */
rmap_t *
ZooDriver::FindRobot( const char *modelName )
{
	for (int i=0; i < robot_count; ++i)
		if (!strcmp(robotMap[i].model_name, modelName))
			return robotMap+i;

	return NULL;
}

/**
 * Find a robot in the robot map, searching by port number.
 */
rmap_t *
ZooDriver::FindRobot( int port )
{
	for (int i=0; i < robot_count; ++i)
		if (robotMap[i].port == port)
			return robotMap+i;

	return NULL;
}

/**
 * Find a robot in the robot map, searching by Controller.
 */
rmap_t *
ZooDriver::FindRobot( ZooController *zc )
{
	for (int i=0; i < robot_count; ++i)
		if (robotMap[i].controller == zc)
			return robotMap+i;

	return NULL;
}

/**
 * Player wants Setup and Shutdown functions; NOOP for now.
 */
int
ZooDriver::Setup( void )
{
	printf("ZooDriver::Setup()\n");
	return 0;
}

/**
 * Player wants Setup and Shutdown functions; NOOP for now.
 */
int
ZooDriver::Shutdown( void )
{
	printf("ZooDriver::Shutdown()\n");
	return 0;
}

/**
 * Player calls this after everything else is ready; Zoo uses it to tell the
 * referee to start.
 */
void
ZooDriver::Prepare( void )
{
	referee->Startup();
}

/**
 * Run a controller on a specific port.  If more than one species has a
 * robot on this port in its population, then more than one controller will
 * be started.
 * @param p the port to connect the controller to.
 */
void
ZooDriver::Run( int p )
{
	int i, n;

	n = species_count;
	for (i = 0; i < n; ++i)
		if (species[i]->Hosts(p)) {
			rmap_t *rp = FindRobot(p);
			rp->controller = species[i]->Run(p);
		}
}

/**
 * Run a controller to control a robot with a given model name.
 * NOTE: currently uses GetModelPortByName, which I want to deprecate in
 * favour of FindRobot.
 * @param model the name of the top-level model of the robot.
 */
void
ZooDriver::Run( const char *model )
{
	Run(GetModelPortByName(model));
}

/** 
 * Run a controller for every member of the populations of every species.
 */
void
ZooDriver::RunAll( void )
{
	int i, n;

	n = species_count;
	for (i = 0; i < n; ++i)
		species[i]->RunAll();
}

/**
 * Kill the controller for a robot on a given port.
 * @param p the port whose controller should be killed.
 */
void
ZooDriver::Kill( int p )
{
	rmap_t *rp = FindRobot(p);
	if (!rp || !rp->controller) return;
	rp->controller->Kill();
	rp->controller = NULL;
}

/**
 * Kill the controller controlling a robot with a given top-level model
 * name.
 * @param model the name of the top-level model
 */
void
ZooDriver::Kill( const char *model )
{
	rmap_t *rp = FindRobot(model);
	if (!rp || !rp->controller) return;
	rp->controller->Kill();
	rp->controller = NULL;
}

/**
 * Kill all active controllers.
 */
void
ZooDriver::KillAll( void )
{
	int i;

	for (i = 0; i < robot_count; ++i)
		if (robotMap[i].controller) {
			robotMap[i].controller->Kill();
			robotMap[i].controller = NULL;
		}
}

/**
 * DEPRECATED: Use ZooDriver::FindRobot instead.
 * @param k index of a robot in the robot map.
 * @return the name of the top-level model of the robot.
 */
const char *
ZooDriver::GetModelNameByIndex( int k )
{
	return robotMap[k].model_name;
}

/**
 * Get the number of top-level models.  FIXME: This should be the number of
 * top-level models that Zoo cares about; i.e., the ones in the population
 * of some species.
 * @return the number of top-level models.
 */
int
ZooDriver::GetModelCount( void )
{
	return robot_count;
}

/**
 * Get a Stage stg_model_t object for the model with this name (the
 * stg_model_t.token entry).
 * @param name the name (token) of the model.
 * @return a pointer to the model; this is Stage's copy and should not be
 * freed.
 */
stg_model_t *
ZooDriver::GetModelByName( const char *name )
{
	return stg_world_model_name_lookup(StgDriver::world, name);
}

/**
 * Get a Stage stg_model_t object for the kth model in the robot map.
 * @param k the index to the robot map.
 * @return a pointer to the model; this is Stage's copy and should not be
 * freed.
 */
stg_model_t *
ZooDriver::GetModelByIndex( int k )
{
	return GetModelByName(robotMap[k].model_name);
}

/**
 * Get a Stage stg_model_t object for the model on the given port.
 * @param p the port on which there's a model we're interested in.
 * @return a pointer to the model; this is Stage's copy and should not be
 * freed.
 */
stg_model_t *
ZooDriver::GetModelByPort( int p )
{
	const char *mname = GetModelNameByPort(p);
	return mname ? GetModelByName(mname) : NULL;
}

/**
 * DEPRECATED: Use ZooDriver::FindRobot instead.
 * @param p the port
 * @return the name of the top-level model on this port.
 */
const char *
ZooDriver::GetModelNameByPort( int p )
{
	rmap_t *rp = FindRobot(p);

	return rp ? rp->model_name : NULL;
}

/**
 * DEPRECATED: Use ZooDriver::FindRobot instead.
 * @param m the name of a top-level model.
 * @return the port that this model is on.
 */
int
ZooDriver::GetModelPortByName( const char *m )
{
	rmap_t *rp = FindRobot(m);
	return rp ? rp->port : NULL;
}

/**
 * Get the score data for this model.  If no score data
 * has been set yet (using ZooDriver::SetScore), return 0.
 * The caller must provide
 * enough space to store the data, which is in a format unknown to Zoo.
 * @param model a string for the name of the model (the "token" field of a
 * stg_model_t struct).
 * @param data pointer to memory to copy the data to.
 * @return the number of bytes copied, or -1 if the model is not found.
 */
int
ZooDriver::GetScore( const char *model, void *data )
{
	rmap_t *rp = FindRobot(model);

	if (!rp || !rp->controller) return -1;
	if (!rp->controller->score_data || rp->controller->score_size <= 0)
		return 0;
	memcpy(data, rp->controller->score_data, rp->controller->score_size);

	return rp->controller->score_size;
}

/**
 * Get the size of the score data for this model.
 * 0 could mean that no score data has been set yet.
 * @param model a string for the name of the model (the "token" field of a
 * stg_model_t struct).
 * @return the size of the score data, or -1 if the model is not found.
 */
int
ZooDriver::GetScoreSize( const char *model )
{
	rmap_t *rp = FindRobot(model);
	return (rp && rp->controller) ? rp->controller->score_size : NULL;
}

/** 
 * Set the score data for this model.  Reallocates
 * storage, stored locally, each time the function is called.
 * @param model a string for the name of the model (the "token" field of a
 * stg_model_t struct).
 * @param score a pointer to the data to be copied into the local score
 * buffer.
 * @param siz the number of bytes to copy.
 */
int
ZooDriver::SetScore( const char *model, void *score, size_t siz )
{
	/* set persistent score on controller */
	rmap_t *rp = FindRobot(model);
	if (!rp || !rp->controller) return -1;
	if (rp->controller->score_data)
		free(rp->controller->score_data);
	rp->controller->score_data = malloc(siz);
	memcpy(rp->controller->score_data, score, siz);
	rp->controller->score_size = siz;

	/* set transient score on robot (position model) */
	stg_model_t *pos = GetModelByName(model);
	stg_model_set_property(pos, ZOO_SCORE_PROPERTY_NAME, score, siz);

	return 1;
}

/**
 * Erase the score data for this model.  Frees any allocated memory.
 * @param model a string for the name of the model (the "token" field of a
 * stg_model_t struct).
 * @return -1 if the model is not found, 0 otherwise.
 */
int
ZooDriver::ClearScore( const char *model )
{
	/* clear persistent score */
	rmap_t *rp = FindRobot(model);
	if (!rp || !rp->controller) return -1;
	if (rp->controller->score_data) free(rp->controller->score_data);
	rp->controller->score_size = 0;

	/* clear transient score: */
	stg_model_t *mod = GetModelByName(model);
	stg_model_set_property(mod, ZOO_SCORE_PROPERTY_NAME, NULL, 0);

	return 0;
}

/**
 * Set the callback for drawing score data on the GUI, but do so for every
 * species.
 */
void
ZooDriver::SetScoreDrawCB( zooref_score_draw_t sdcb, void *udata )
{
	for (int i=0; i<species_count; ++i)
		species[i]->SetScoreDrawCB(sdcb, udata);
}

/**
 * Locate a species by its name.
 * @param name the name of the species.
 * @return a pointer to the ZooSpecies object.
 */
ZooSpecies *
ZooDriver::GetSpeciesByName( const char *name )
{
	int i;
	for (i = 0; i < species_count; ++i)
		if (!strcmp(name, species[i]->name))
			return species[i];
	return NULL;
}

/********* ZooSpecies *********/

/**
 * ZooSpecies constructor.
 * @param _zoo the ZooDriver object that created me.
 */
ZooSpecies::ZooSpecies( ConfigFile *cf, int section, ZooDriver *_zoo )
{
	/* remember the ZooDriver for later use */
	zoo = _zoo;

	/* get the name; this is a const char * and it's not my problem
	 * to free it. */
	name = cf->ReadString(section, "name", ZOO_DEFAULT_SPECIES);

	/* get the list of models (and therefore ports) which are members of this
	 * species */
	population_size = cf->GetTupleCount(section, "population");
	port_list = new int[population_size];
	model_list = new char*[population_size];
	for (int i=0; i < population_size; ++i) {
		const char *mname;
		mname = cf->ReadTupleString(section, "population", i, "");
		if (!mname || !*mname) break;
		model_list[i] = strdup(mname);
		port_list[i] = zoo->GetModelPortByName(mname);
		if (port_list[i] == 0)
			zoo_err("Member \"%s\" of species \"%s\" doesn't exist.\n",
				model_list[i], name);
	}

	/* find all controllers belonging to me */
	controller.clear();
	for (int i=0; i < cf->GetSectionCount(); ++i) {
		int pi;

		/* only care about controllers */
		if (strcmp(cf->GetSectionType(i), ZOO_CONTROLLER_SECTYPE))
			continue;

		/* this should be a child section of my section */
		pi = cf->GetSectionParent(i);
		if (pi < 0
		 || strcmp(cf->GetSectionType(pi), "species")
		 || strcmp(cf->ReadString(pi, "name", ""), name))
			continue;

		ZooController zc(cf, i, zoo, this);
		controller.push_back(zc);
	}

	/* initialize controller selection */
	next_controller = 0;
	controller_instance = 0;

	/* default score display setup */
	score_draw_cb = NULL;
	score_draw_user_data = NULL;

	ZOO_DEBUGMSG("Zoo: Added species %s\n", name, 0);
}

/**
 * ZooSpecies destructor.  Destroys model list and port list.
 */
ZooSpecies::~ZooSpecies()
{
	ZOO_DEBUGMSG("Zoo: Cleaning up species %s", name, 0);
	delete port_list;
	delete model_list;
	ZOO_DEBUGMSG("Zoo: Done cleaning up %s\n", name, 0);
}

static int
clear_cb( stg_model_t *mod )
{
	stg_rtk_fig_t *fig;

	fig = stg_model_get_fig(mod, ZOOREF_SCORE_FIG_NAME);
	if (fig)
		stg_rtk_fig_clear(fig);

	return 0;
}

/**
 * Set a callback to be called when a robot's score changes.
 * FIXME: This means that if the score doesn't change, the display doesn't
 * move with the robot.  Oops.
 * @param draw_cb the callback.
 * @param userdata arbitrary data to be passed along.
 */
void
ZooSpecies::SetScoreDrawCB( zooref_score_draw_t draw_cb, void *userdata )
{
	score_draw_cb = draw_cb;
	score_draw_user_data = userdata;

	int i, n=zoo->GetModelCount();
	for (i=0; i<n; ++i) {
		stg_model_t *mod = zoo->GetModelByIndex(i);
		if (!mod) continue;
		stg_model_add_property_toggles(mod, ZOO_SCORE_PROPERTY_NAME,
			(stg_property_callback_t)draw_cb, userdata,
			(stg_property_callback_t)clear_cb, NULL,
			ZOO_SCORE_LABEL,
			0);
	}
}

/**
 * Run a controller on port a given port.
 * @param p the port to run on
 * @return a pointer to the ZooController that was executed.
 */
ZooController *
ZooSpecies::Run( int p )
{
	ZooController *zc = SelectController();
	if (!zc) {
		zoo_err("No controllers available for species %s\n", name);
		return NULL;
	}
	zc->Run(p);
	return zc;
}

/**
 * Run a controller for every member of my population.
 */
void
ZooSpecies::RunAll( void )
{
	int p;

	for (p = 0; p < population_size; ++p) {
		if (port_list[p] <= 0) continue;

		rmap_t *rp = zoo->FindRobot(port_list[p]);
		rp->controller = Run(port_list[p]);
	}
}

/**
 * Kill the controller on every member of my population.
 */
void
ZooSpecies::KillAll( void )
{
	int p;

	for (p = 0; p < population_size; ++p) {
		rmap_t *rp = zoo->FindRobot(port_list[p]);
		if (!rp || !rp->controller) continue;
		rp->controller->Kill();
		rp->controller = NULL;
	}
}

/**
 * Pick a controller from the pool.
 * Deterministic; uses a round-robin scheme with priority.
 */
ZooController *
ZooSpecies::SelectController( void )
{
	int n;
	ZooController *zc;

	n = controller.size();

	if (n == 0) return NULL;

	zc = &controller[next_controller];
	if (controller_instance++ == zc->GetFrequency()) {
		controller_instance = 0;
		next_controller = ++next_controller % n;
	}
	
	return zc;
}

/**
 * Determine whether the robot on this port is in the population of this
 * species.
 */
bool
ZooSpecies::Hosts( int p )
{
	for (int i=0; i < population_size; ++i)
		if (port_list[i] == p)
			return true;

	return false;
}

/**
 * Dump out details about this species.
 */
void
ZooSpecies::print( void )
{
	printf("Zoo species %s = { ", name);
	for (int i = 0; i < population_size; ++i)
		printf("%s(%d) ", model_list[i], port_list[i]);
	printf("}\n");
}

/********* ZooController *********/

const char *ZooController::path = "";

/**
 * ZooController constructor.
 * @param sp the species I belong to.
 */
ZooController::ZooController( ConfigFile *cf, int section,
                              ZooDriver *zd, ZooSpecies *sp )
{
	/* get the frequency */
	frequency = cf->ReadInt(section, "frequency", ZOO_DEFAULT_FREQUENCY);

	/* get the command-line and parse it into a ready-to-use form. */
	command = cf->ReadString(section, "command", "");

	/* get information for redirecting stdout/stderr */
	outfilename = cf->ReadString(section, "outfilename", NULL);
	outfilemode = cf->ReadString(section, "outfilemode", NULL);
	errfilename = cf->ReadString(section, "errfilename", NULL);
	errfilemode = cf->ReadString(section, "errfilemode", NULL);

	/* remember what species I'm in */
	species = sp;

	/* remember what Zoo i'm in */
	zoo = zd;

	/* initially there's no score information */
	score_data = 0x0;
	score_size = 0;

	ZOO_DEBUGMSG("Zoo: Added controller \"%s\" with frequency %d.\n",
		command, frequency);
}

/**
 * ZooController destructor.  A NOOP.
 */
ZooController::~ZooController()
{
}

/**
 * Run this controller on the specified port.  Prepends the controller_path
 * specified in the config file to the PATH variable before execution.
 * Controllers should accept a "-p #" option.
 * @param port the port to connect to.
 */
void
ZooController::Run( int port )
{
	char *cmdline, *newpath, *argv[256];
	int i;
	wordexp_t wex;

	pid = fork();
	if (pid) return;

	/* prepend the controller_path to the PATH. */
	char *oldpath = getenv("PATH");
	int len = strlen(path) + strlen(oldpath) + 1;
	newpath = (char *)calloc(len+1, sizeof(char));
	sprintf(newpath, "%s:%s", path, oldpath);
	wordexp(newpath, &wex, 0);
	if (wex.we_wordc > 0)
		setenv("PATH", wex.we_wordv[0], 1);
	free(newpath);

	/* build a command to execute */
	cmdline = strdup(command);
	for(i=0; cmdline; ++i)
		argv[i] = strsep(&cmdline, " \t");
	argv[i++] = "-p";
	argv[i] = (char *)calloc(6, sizeof(char));
	sprintf(argv[i++], "%d", port);
	argv[i] = NULL;
	printf("Zoo: Executing controller ");
	for (i=0; argv[i]; ++i)
		printf("%s ", argv[i]);
	putchar('\n');

	/* Redirect stdout and stderr */
	FILE *my_stdout, *my_stderr;
	char fullname[PATH_MAX];
	rmap_t *rp = zoo->FindRobot(port);
	if (outfilename) {
		cpathprintf(fullname, outfilename, rp);
		my_stdout = freopen(fullname, outfilemode?outfilemode:"w", stdout);
		if (!my_stdout) perror(outfilename);
	}
	if (errfilename) {
		cpathprintf(fullname, errfilename, rp);
		my_stderr = freopen(fullname, errfilemode?errfilemode:"w", stderr);
		if (!my_stderr) perror(errfilename);
	}

	/* execute */
	execvp(argv[0], argv);
	perror(argv[0]);
	exit(1);

	return;
}

/**
 * Generate a filename for a controller to redirect stdout or stderr to
 */
void
ZooController::cpathprintf( char *str, const char *fmt, const rmap_t *rp )
{
	const char *fmtp;

	for(fmtp=fmt; *fmtp; fmtp++)
		if (*fmtp == '%')
			switch(*++fmtp) {
			default:
				zoo_err("Controller outfilename/errfilename \"%s\" "
				        "contains a bad format argument '%c'.\n",
					fmt, *fmtp);
				break;
			case '%':
				*str++ = '%';
				break;
			case 'p':
				str += sprintf(str, "%d", rp->port);
				break;
			case 'm': case 'n':
				str += sprintf(str, "%s", rp->model_name);
				break;
			case 'c':
				str += sprintf(str, "%s", path);
				break;
			}
		else /* *fmtp == '%' */
			*str++ = *fmtp;
}

/**
 * Kill this controller, if it's currently running.
 */
void
ZooController::Kill( void )
{
	kill(pid, SIGTERM);
}

/************ Referee *************/

ZooDriver *ZooReferee::zoo = NULL;

/**
 * ZooReferee constructor.
 * @param zd the ZooDriver I will be refereeing for.
 */
ZooReferee::ZooReferee( ConfigFile *cf, int section, ZooDriver *zd )
{
	zoo = zd;
}

/**
 * Called by Zoo when everything is ready to start.  I should use this
 * opportunity to start controllers for the initial batch of robots.
 */
void
ZooReferee::Startup( void )
{
	int i;
	for (i = 0; i < zoo->GetModelCount(); ++i) {
		stg_model_t *mod = zoo->GetModelByIndex(i);
		printf("Zoo: model %s has id %d\n", mod->token, mod->id);
	}
	zoo->RunAll();
}

/**
 * Some default score-draw callbacks.  The ones for elementary data types
 * should *all* be implemented for arrays.  The length of the array will be
 * passed as a struct in the userdata, which will contain its own userdata.
 */

int
ZooReferee::draw_int_cb( stg_model_t *mod, const char *propname, 
                         const int *data, size_t siz )
{
	stg_rtk_fig_t *fig;

	/* Get an object to draw the score on, or create one if it doesn't
	 * exist. */
	fig = stg_model_get_fig(mod, ZOOREF_SCORE_FIG_NAME);
	if (!fig) {
		fig = stg_model_fig_create(mod, ZOOREF_SCORE_FIG_NAME, NULL,
			STG_LAYER_USER);
		stg_color_t *color = (stg_color_t *)
			stg_model_get_property_fixed(mod, "color", sizeof(stg_color_t));
		stg_rtk_fig_color_rgb32(fig, *color);
	}

	/* if the robot is uncontrolled, don't print the score */
	if (!mod->subs) return 0;

	/* what to draw */
	static char *buf=NULL, *cp;
	if (buf) free(buf); /* attempt to avoid leaks */
	buf = (char *)calloc(16*siz/sizeof(int), sizeof(char));
	strcpy(buf, "[ ");
	cp = buf+2;
	for (unsigned int i=0; i < siz/sizeof(int); ++i) {
		cp += snprintf(cp, 32, "%d", data[i]);
		if (i == siz/sizeof(int)-1)
			strcpy(cp, " ]");
		else
			strcpy(cp, ",\n  ");
		cp += 4;
	}

	/* where to print it */
	stg_pose_t pose;
	stg_model_get_global_pose(mod, &pose);
	stg_rtk_fig_origin(fig, pose.x, pose.y, 0);

	/* draw it, under and to the left of the position model */
	stg_rtk_fig_text(fig, -0.75, -0.75, 0.0, buf);

	/* keep marshalling callbacks */
	return 0;
}

int
ZooReferee::draw_double_cb( stg_model_t *mod, const char *propname, 
                         const double *data, size_t siz )
{
	stg_rtk_fig_t *fig;

	/* Get an object to draw the score on, or create one if it doesn't
	 * exist. */
	fig = stg_model_get_fig(mod, ZOOREF_SCORE_FIG_NAME);
	if (!fig) {
		fig = stg_model_fig_create(mod, ZOOREF_SCORE_FIG_NAME, NULL,
			STG_LAYER_USER);
		stg_color_t *color = (stg_color_t *)
			stg_model_get_property_fixed(mod, "color", sizeof(stg_color_t));
		stg_rtk_fig_color_rgb32(fig, *color);
	}

	/* if the robot is uncontrolled, don't print the score */
	if (!mod->subs) return 0;

	/* what to draw */
	static char *buf=NULL, *cp;
	if (buf) free(buf); /* attempt to avoid leaks */
	buf = (char *)calloc(16*siz/sizeof(double), sizeof(char));
	strcpy(buf, "[ ");
	cp = buf+2;
	for (unsigned int i=0; i < siz/sizeof(double); ++i) {
		cp += snprintf(cp, 32, "%g", data[i]);
		if (i == siz/sizeof(double)-1)
			strcpy(cp, " ]");
		else
			strcpy(cp, ",\n  ");
		cp += 4;
	}

	/* where to print it */
	stg_pose_t pose;
	stg_model_get_global_pose(mod, &pose);
	stg_rtk_fig_origin(fig, pose.x, pose.y, 0);

	/* draw it, under and to the left of the position model */
	stg_rtk_fig_clear(fig);
	stg_rtk_fig_text(fig, -0.75, -0.75, 0.0, buf);

	/* keep marshalling callbacks */
	return 0;
}

/// @}

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

/**
 * \addtogroup zoo
 * @{
 *  \defgroup ZooDriver
 *  @{
 */

/**
 * Description: An addon to the Stage plugin for Zoo functionality.
 * Author: Adam Lein
 * Date: July 14, 2005
 */

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

#include "zoo_driver.h"
#include "zoo_species.h"
#include "zoo_controller.h"
#include "zoo_referee.h"
#include "zoo.h"

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

/**
 *  @}
 * @}
 */

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

/*****
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

#define ZOO_DRIVER_NAME "zoo"
#define ZOO_SPECIES_SECTYPE "species"
#define ZOO_CONTROLLER_SECTYPE "controller"

#define ZOOREF_CREATE "zooref_create"
typedef ZooReferee *(*zooref_create_t)(ConfigFile *, int, ZooDriver *);

#define ZOO_DEFAULT_SPECIES "planktron"
#define ZOO_DEFAULT_FREQUENCY 1

#define ZOO_DIAGNOSTIC_LEVEL 5
#define ZOO_DEBUGMSG(f, a, b) PLAYER_MSG2(ZOO_DIAGNOSTIC_LEVEL, f, a, b)

extern bool quiet_startup;
extern DeviceTable *deviceTable;

/********* Driver ********/

Driver * ZooDriver_Init( ConfigFile *cf, int section )
{
	return (Driver *)(new ZooDriver(cf, section));
}

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
		if (!strcmp(cf->GetSectionType(i), "driver")
		 && !strcmp(cf->ReadString(i, "name", ""), "stage")
		 && strcmp(cf->ReadString(i, "model", ""), ""))
			++robot_count;
	}
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

		/* add the mapping */
		printf("Zoo: MODELMAP %s --> %d\n", modelname, port);
		robotMap[k].port = port;
		robotMap[k++].model_name = strdup(modelname);
	}

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
	} else
default_referee:
		referee = new ZooReferee(cf, section, this);
}

/* ZooDriver destructor */
ZooDriver::~ZooDriver()
{
	for (int i = 0; i < species_count; ++i)
		delete species[i];
	delete species;

	for (int i=0; i < robot_count; ++i)
		free((char *)robotMap[i].model_name);
	free(robotMap);
}

rmap_t *
ZooDriver::FindRobot( const char *modelName )
{
	for (int i=0; i < robot_count; ++i)
		if (!strcmp(robotMap[i].model_name, modelName))
			return robotMap+i;

	return NULL;
}

rmap_t *
ZooDriver::FindRobot( int port )
{
	for (int i=0; i < robot_count; ++i)
		if (robotMap[i].port == port)
			return robotMap+i;

	return NULL;
}

rmap_t *
ZooDriver::FindRobot( ZooController *zc )
{
	for (int i=0; i < robot_count; ++i)
		if (robotMap[i].controller == zc)
			return robotMap+i;

	return NULL;
}

int
ZooDriver::Setup( void )
{
	printf("ZooDriver::Setup()\n");
	return 0;
}

int
ZooDriver::Shutdown( void )
{
	printf("ZooDriver::Shutdown()\n");
	return 0;
}

void
ZooDriver::Prepare( void )
{
	referee->Startup();
}

#if 0
void
ZooDriver::Update( void )
{
}
#endif

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

void
ZooDriver::Run( const char *model )
{
	Run(GetModelPortByName(model));
}

void
ZooDriver::RunAll( void )
{
	int i, n;

	n = species_count;
	for (i = 0; i < n; ++i)
		species[i]->RunAll();
}

void
ZooDriver::Kill( int p )
{
	rmap_t *rp = FindRobot(p);
	if (!rp || !rp->controller) return;
	rp->controller->Kill();
	rp->controller = NULL;
}

void
ZooDriver::Kill( const char *model )
{
	rmap_t *rp = FindRobot(model);
	if (!rp || !rp->controller) return;
	rp->controller->Kill();
	rp->controller = NULL;
}

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

const char *
ZooDriver::GetModelNameByIndex( int k )
{
	return robotMap[k].model_name;
}

int
ZooDriver::GetModelCount( void )
{
	return robot_count;
}

stg_model_t *
ZooDriver::GetModelByName( const char *name )
{
	return stg_world_model_name_lookup(StgDriver::world, name);
}

stg_model_t *
ZooDriver::GetModelByIndex( int k )
{
	return GetModelByName(robotMap[k].model_name);
}

stg_model_t *
ZooDriver::GetModelByPort( int p )
{
	const char *mname = GetModelNameByPort(p);
	return mname ? GetModelByName(mname) : NULL;
}

const char *
ZooDriver::GetModelNameByPort( int p )
{
	rmap_t *rp = FindRobot(p);

	return rp ? rp->model_name : NULL;
}

int
ZooDriver::GetModelPortByName( const char *m )
{
	rmap_t *rp = FindRobot(m);
	return rp ? rp->port : NULL;
}

/**
 * ZooDriver::GetScore: Get the score data for this model.  If no score data
 * has been set yet (using ::SetScore), return 0.  The caller must provide
 * enough space to store the data, which is in a format unknown to Zoo.
 * @param model: a string for the name of the model (the "token" field of a
 * stg_model_t struct).
 * @param data: pointer to memory to copy the data to.
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
 * ZooDriver::GetScoreSize: Return the size of the score data for this model.
 * 0 could mean that no score data has been set yet.
 * @param model: a string for the name of the model (the "token" field of a
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
 * ZooDriver::SetScore: Set the score data for this model.  Reallocates
 * storage, stored locally, each time the function is called.
 * @param model: a string for the name of the model (the "token" field of a
 * stg_model_t struct).
 * @param score: a pointer to the data to be copied into the local score
 * buffer.
 * @param siz: the number of bytes to copy.
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
 * ZooDriver::ClearScore: Erase the score data for this model.  Frees any
 * allocated memory.
 * @param model: a string for the name of the model (the "token" field of a
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

		ZooController zc(cf, i, this);
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

ZooSpecies::~ZooSpecies()
{
	ZOO_DEBUGMSG("Zoo: Cleaning up species %s", name, 0);
	delete port_list;
	delete model_list;
	ZOO_DEBUGMSG("Zoo: Done cleaning up %s\n", name, 0);
}

/**
 * ::SetScoreDrawCB(draw_cb, userdata)
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
			NULL, NULL, ZOO_SCORE_LABEL, 1);
	}
}

/**
 * ZooSpecies::Run(p) -- run a controller on port p
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
 * ZooSpecies::RunAll() -- run a controller on every port
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
 * ZooSpecies::KillAll() -- kill the controller on every port
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
 * ZooSpecies::SelectController -- pick a controller from the pool.
 * Deterministic.
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

bool
ZooSpecies::Hosts( int p )
{
	for (int i=0; i < population_size; ++i)
		if (port_list[i] == p)
			return true;

	return false;
}

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

ZooController::ZooController( ConfigFile *cf, int section, ZooSpecies *sp )
{
	/* get the frequency */
	frequency = cf->ReadInt(section, "frequency", ZOO_DEFAULT_FREQUENCY);

	/* get the command-line and parse it into a ready-to-use form. */
	command = cf->ReadString(section, "command", "");

	/* remember what species I'm in */
	species = sp;

	/* initially there's no score information */
	score_data = 0x0;
	score_size = 0;

	ZOO_DEBUGMSG("Zoo: Added controller \"%s\" with frequency %d.\n",
		command, frequency);
}

ZooController::~ZooController()
{
}

void
ZooController::Run( int port )
{
	char *cmdline, *newpath, *argv[256];
	int i;
	wordexp_t wex;

	pid = fork();
	if (pid) return;

#if 0 // asprintf is a GNU nicety
	asprintf(&newpath, "%s:%s", path, getenv("PATH"));
#else
	char *oldpath = getenv("PATH");
	int len = strlen(path) + strlen(oldpath) + 1;
	newpath = (char *)calloc(len+1, sizeof(char));
	sprintf(newpath, "%s:%s", path, oldpath);
#endif
	wordexp(newpath, &wex, 0);
	if (wex.we_wordc > 0)
		setenv("PATH", wex.we_wordv[0], 1);
	free(newpath);

	/* the 10 extra bytes are for " -p #####\0" */
#if 0
	cmdline = (char *)malloc(strlen(command)+10);
	sprintf(cmdline, "%s -p %d", command, port);
	ZOO_DEBUGMSG("executing controller %s\n", cmdline, 0);

	system(cmdline); // TODO: Why do I feel like I should be using exec?
	free(cmdline);
	exit(0);
#else
	cmdline = strdup(command);
	for(i=0; cmdline; ++i)
		argv[i] = strsep(&cmdline, " \t");
	argv[i++] = "-p";
#if 0
	asprintf(argv+i++, "%d", port);
#else
	argv[i] = (char *)calloc(6, sizeof(char));
	sprintf(argv[i++], "%d", port);
#endif
	argv[i] = NULL;
	printf("Zoo: Executing controller ");
	for (i=0; argv[i]; ++i)
		printf("%s ", argv[i]);
	putchar('\n');
	execvp(argv[0], argv);
	perror(argv[0]);
	exit(1);
#endif

	return;
}

void
ZooController::Kill( void )
{
	kill(pid, SIGTERM);
}

/************ Referee *************/

ZooReferee::ZooReferee( ConfigFile *cf, int section, ZooDriver *zd )
{
	zoo = zd;
}

void
ZooReferee::Startup( void )
{
	int i;
	for (i = 0; i < zoo->GetModelCount(); ++i) {
		stg_model_t *mod = zoo->GetModelByIndex(i);
		printf("Zoo: model %s has id %d\n", mod->token, mod->id);
	}
	zoo->RunAll();
#if 0
	zoo->Run(6665);
	zoo->Run(6666);
#endif
}

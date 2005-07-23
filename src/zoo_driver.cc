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
#include <features.h>

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

#define ZOO_BAD_PORTRANGE_STR "Port ranges should be strings of the form " \
                              "min-max or a single number."

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
		PLAYER_ERROR("Zoo is confused\n");
	AddInterface(devid, PLAYER_ALL_MODE, 0, 0, 0, 0);

	/* find all species in the config file */
	species_count = 0;
	for (int i=0; i < cf->GetSectionCount(); ++i)
		if (!strcmp(cf->GetSectionType(i), ZOO_SPECIES_SECTYPE))
			++species_count;
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

		ZooSpecies *zs = new ZooSpecies(cf, i);
		species[si++] = zs;
	}
	/* FIXME: Something very bad happens here.  The port ranges on species
	 * are being overwritten somehow.
	 * Temporary fix: using entirely dynamic memory (i.e.
	 * "ZooSpecies **species" instead of "ZooSpecies *species")works... why?
	 */

	/* TODO: Grab sigint? */
	//signal(SIGINT, sigint_handler);

	/* build a model-to-port mapping */
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
		portMap[modelname] = port;
		modelList.push_back(modelname);
	}

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
			fprintf(stderr, "Zoo: cannot load referee %s: %s\n",
				referee_path, dlerror());
			goto default_referee;
		}

		/* get the zooref_create function */
		zooref_create = (zooref_create_t)
			dlsym(zooref_handle, ZOOREF_CREATE);
		if (!zooref_create) {
			fprintf(stderr, "Zoo: referee must define %s\n",
				ZOOREF_CREATE);
			dlclose(zooref_handle);
			goto default_referee;
		}

		/* create the referee */
		referee = zooref_create(cf, section, this);
	} else
default_referee:
		referee = new ZooReferee(cf, section, this);
}

ZooDriver::~ZooDriver()
{
	for (int i = 0; i < species_count; ++i)
		delete species[i];
	delete species;
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

void
ZooDriver::Run( int p )
{
	int i, n;

	n = species_count;
	for (i = 0; i < n; ++i)
		if (species[i]->Hosts(p))
			controllerMap[p] = species[i]->Run(p);
}

void
ZooDriver::Run( const char *model )
{
	Run(portMap[model]);
}

void
ZooDriver::RunAll( void )
{
	int i, n;

	n = species_count;
	for (i = 0; i < n; ++i)
		species[i]->RunAll(controllerMap);
}

void
ZooDriver::Kill( int p )
{
	controllerMap[p]->Kill();
	controllerMap.erase(p);
}

void
ZooDriver::Kill( const char *model )
{
	Kill(portMap[model]);
}

void
ZooDriver::KillAll( void )
{
	int i, n;

	n = species_count;
	for (i = 0; i < n; ++i)
		species[i]->KillAll(controllerMap);

	controllerMap.clear();
}

const char *
ZooDriver::GetModelName( int k )
{
	return modelList[k];
}

int
ZooDriver::GetModelCount( void )
{
	return modelList.size();
}

stg_model_t *
ZooDriver::GetModel( const char *name )
{
	return stg_world_model_name_lookup(StgDriver::world, name);
}

stg_model_t *
ZooDriver::GetModel( int k )
{
	return GetModel(modelList[k]);
}

int
ZooDriver::GetScore( const char *model, const char *species,
                     void *data, size_t *siz )
{
	ZooController *zc = controllerMap[portMap[model]];
	void *sdata;
	size_t ssize;

	if (!zc) return 0;

	sdata = zc->scoreMap[species];
	if (!sdata) return 0;
	ssize = zc->scoreSizeMap[species];

	memcpy(data, sdata, ssize);
	if (siz) *siz = ssize;

	return 1;
}

void
ZooDriver::SetScore( const char *model, const char *species,
                     void *score, size_t siz )
{
	ZooController *zc = controllerMap[portMap[model]];
	void *data;

	if (!zc) return;

	data = zc->scoreMap[species];
	if (data) realloc(data, siz);
	memcpy(data, score, siz);

	zc->scoreMap[species] = data;
	zc->scoreSizeMap[species] = siz;
}

void
ZooDriver::PrintScore( const char *filename )
{
	int i;
	FILE *fp;

	fp = fopen(filename, "w");
	if (!fp) {
		perror(filename);
		return;
	}

	for (i = 0; i < species_count; ++i)
		species[i]->PrintScore(fp);

	fclose(fp);
}

void
ZooDriver::SetScorePrintFunction( const char *species,
	zooref_score_printer_t printer, void *user_data )
{
	ZooSpecies *zs = GetSpeciesByName(species);
	zs->SetScorePrintFunction(printer, user_data);
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

ZooSpecies::ZooSpecies( void )
{
}

/**
 * ZooSpecies constructor.  Parse the port ranges and add controllers.
 */
ZooSpecies::ZooSpecies( ConfigFile *cf, int section )
{
	int err=0;

	/* get the name; this is a const char * and it's not my problem
	 * to free it. */
	name = cf->ReadString(section, "name", ZOO_DEFAULT_SPECIES);

	/* parse port ranges */
	range_count = cf->GetTupleCount(section, "portrange");
	min_port = new int[range_count];
	max_port = new int[range_count];
	controller.clear();
	for (int i=0; i < range_count; ++i) {
		const char *prs = cf->ReadTupleString(section, "portrange", i, "");
		int r = sscanf(prs, "%d-%d", min_port+i, max_port+i);
		switch(r) {
		case 0:
			/* TODO: would be nice if I could get the current line of the
			 * config file... */
			PLAYER_ERROR(ZOO_BAD_PORTRANGE_STR);
			++err;
			break;
		case 1:
			/* singleton range */
			max_port[i] = min_port[i];
			break;
		case 2:
			/* full range... don't need to do anything */
			break;
		}
	}

	/* find all controllers belonging to me */
	for (int i=0; i < cf->GetSectionCount(); ++i) {
		int pi;

		/* only care about controllers */
		if (strcmp(cf->GetSectionType(i), ZOO_CONTROLLER_SECTYPE))
			continue;

		/* should belong to me (the correct species) */
		if (strcmp(cf->ReadString(i, "species", ""), name))
			continue;

		/* should be the the child section of a zoo-driver section */
		pi = cf->GetSectionParent(i);
		if (pi < 0
		 || strcmp(cf->GetSectionType(pi), "driver")
		 || strcmp(cf->ReadString(pi, "name", ""), ZOO_DRIVER_NAME))
			continue;

		ZooController zc(cf, i);
		controller.push_back(zc);
	}

	ZOO_DEBUGMSG("Zoo: Added species %s\n", name, 0);
}

ZooSpecies::~ZooSpecies()
{
	ZOO_DEBUGMSG("Zoo: Cleaning up species %s", name, 0);
	delete min_port;
	delete max_port;
	ZOO_DEBUGMSG("Zoo: Done cleaning up %s\n", name, 0);
}

void
ZooSpecies::SetScorePrintFunction( zooref_score_printer_t printer,
	void *user_data )
{
	score_printer = printer;
	score_printer_user_data = user_data;
}

void
ZooSpecies::PrintScore( FILE *fp )
{
	void *score;
	size_t siz;
	char buf[ZOO_SCORE_BUFFER_SIZE];
	unsigned int i;
	ZooController *zc=NULL;

	for (i = 0; i < controller.size(); ++i) {
		zc = &controller[i];
		score = zc->scoreMap[name];
		siz = zc->scoreSizeMap[name];
		score_printer(buf, score, siz, score_printer_user_data);
	}

	fprintf(fp, "<tr><td>%s</td><td>%s</td></tr>\n",
		zc->GetCommand(), buf);
}

/**
 * ZooSpecies::Run(p) -- run a controller on port p
 */
ZooController *
ZooSpecies::Run( int p )
{
	ZooController *zc = SelectController();
	if (!zc) {
		fprintf(stderr, "Zoo: No controllers available for species %s\n",
			name);
		return NULL;
	}
	zc->Run(p);
	return zc;
}

/**
 * ZooSpecies::RunAll() -- run a controller on every port
 */
void
ZooSpecies::RunAll( cmap_t &cMap )
{
	int r, p;
	ZooController *zc;

	for (r = 0; r < range_count; ++r)
		for (p = min_port[r]; p <= max_port[r]; ++p) {
			zc = Run(p);
			if (!zc) return;
			cMap[p] = zc;
		}
}

/**
 * ZooSpecies::KillAll() -- kill the controller on every port
 */
void
ZooSpecies::KillAll( cmap_t &cMap )
{
	int r, p;

	for (r = 0; r < range_count; ++r)
		for (p = min_port[r]; p <= max_port[r]; ++p) {
			cMap[p]->Kill();
			cMap.erase(p);
		}
}

#if 0
/**
 * ZooSpecies::Kill(p) -- kill controller on port p
 */
void
ZooSpecies::Kill( int p )
{
	ZooController *zc;

	zc = portAlloc[p];
	if (!zc) return;

	zc->Kill();
	portAlloc.erase(p);
}
#endif

/**
 * ZooSpecies::SelectController -- pick a controller from the pool.
 * Deterministic.
 */
ZooController *
ZooSpecies::SelectController( void )
{
	static int next=0, inst=0;
	int n;
	ZooController *zc;

	n = controller.size();

	if (n == 0) return NULL;

	zc = &controller[next];
	if (inst++ == zc->GetFrequency()) {
		inst = 0;
		next = ++next % n;
	}
	
	return zc;
}

bool
ZooSpecies::Hosts( int p )
{
	int r;

	for (r = 0; r < range_count; ++r)
		if (min_port[r] <= p && p <= max_port[r])
			return true;

	return false;
}

void
ZooSpecies::print( void )
{
	int i;

	printf("Zoo species %s ( ", name);
	for (i = 0; i < range_count; ++i)
		printf("%d-%d ", min_port[i], max_port[i]);
	printf(")\n");
}

/********* ZooController *********/

const char *ZooController::path = "";

ZooController::ZooController( ConfigFile *cf, int section )
{
	/* get the frequency */
	frequency = cf->ReadInt(section, "frequency", ZOO_DEFAULT_FREQUENCY);

	/* get the command-line and parse it into a ready-to-use form. */
	command = cf->ReadString(section, "command", "");

	/* initially there's no score information */

	ZOO_DEBUGMSG("Zoo: Added controller \"%s\" with frequency %d.\n",
		command, frequency);
}

ZooController::~ZooController()
{
}

void
ZooController::Run( int port )
{
	char *cmdline, *newpath;
	wordexp_t wex;

	pid = fork();
	if (pid) return;

	asprintf(&newpath, "%s:%s", path, getenv("PATH"));
	wordexp(newpath, &wex, 0);
	if (wex.we_wordc > 0)
		setenv("PATH", wex.we_wordv[0], 1);
	free(newpath);

	/* the 10 extra bytes are for " -p #####\0" */
	cmdline = (char *)malloc(strlen(command)+10);
	sprintf(cmdline, "%s -p %d", command, port);
	ZOO_DEBUGMSG("executing controller %s\n", cmdline, 0);

	system(cmdline); // TODO: Why do I feel like I should be using exec?
	free(cmdline);
	exit(0);

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
		stg_model_t *mod = zoo->GetModel(i);
		printf("Zoo: model %s has id %d\n", mod->token, mod->id);
	}
	zoo->RunAll();
#if 0
	zoo->Run(6665);
	zoo->Run(6666);
#endif
}

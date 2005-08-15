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
 *  \defgroup ZooController
 *  @{
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

#include "zoo.h"

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
	rmap_t *rp;

	rp = zoo->FindRobot(port);

	/* draw a RUN indicator on the robot */
	stg_model_t *mod = zoo->GetModelByName(rp->model_name);
	stg_msec_t starttime=stg_timenow();
	stg_model_set_property(mod, ZOO_RUN_IND_PROP,
		&starttime, sizeof(stg_msec_t));
	int zero=0;
	stg_model_set_property(mod, "zoo_killed",
		&zero, sizeof(int));

	/* TODO: check whether forking this early might cause
	 * threading issues. */
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
	if (outfilename) {
		cpathprintf(fullname, outfilename, rp);
		my_stdout = freopen(fullname, outfilemode?outfilemode:"w", stdout);
		if (!my_stdout) zoo_err("Cannot redirect stdout to \"%s\": %s\n",
			fullname, strerror(errno));
	}
	if (errfilename) {
		cpathprintf(fullname, errfilename, rp);
		my_stderr = freopen(fullname, errfilemode?errfilemode:"w", stderr);
		if (!my_stderr) zoo_err("Cannot redirect stderr to \"%s\": %s\n",
			fullname, strerror(errno));
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

	*str = '\0';
}

/**
 * Kill this controller, if it's currently running.
 */
void
ZooController::Kill( void )
{
	kill(pid, SIGTERM);

	rmap_t *rp = zoo->FindRobot(this);
	if (!rp) return;
	stg_model_t *mod = zoo->GetModelByName(rp->model_name);

	int one=1;
	stg_model_set_property(mod, "zoo_killed", &one, sizeof(int));
}

/**
 *  @}
 * @}
 */

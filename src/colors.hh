/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Some useful color routines.  Define the default colors for all
 * devices here.
 * Author: Andrew Howard, Richard Vaughan
 * Date: 8 Jun 2002
 * CVS info: $Id: colors.hh,v 1.6.6.1 2003-02-01 02:14:29 rtv Exp $
 */

#ifndef _STAGECOLORS_H
#define _STAGECOLORS_H 

#include <stdint.h>

// Color type
typedef uint32_t StageColor;

// Look up the color in a database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
StageColor LookupColor(const char *name);

// GUI color prefs 
#define GRID_MAJOR_COLOR "gray85"
#define GRID_MINOR_COLOR "gray95"
#define MATRIX_COLOR "dark green"
#define BACKGROUND_COLOR "ivory"

#endif

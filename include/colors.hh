//-------------------------------------------------------------------------------------
// colors.hh - define the default colors for all devices here
// $Id: colors.hh,v 1.6 2002-01-28 22:32:25 inspectorg Exp $
//

#ifndef _STAGECOLORS_H
#define _STAGECOLORS_H 

#ifdef INCLUDE_RTK2

#define BACKGROUND_COLOR "light gray"
#define GRIPPER_COLOR "gray"
#define POSITION_COLOR "dark red"
#define LASER_COLOR  "blue"
#define SONAR_COLOR  "blue3"
#define PTZ_COLOR  "magenta"
#define PUCK_COLOR  "green"
#define BOX_COLOR  "yellow"
#define LBD_COLOR  "gray"
#define MISC_COLOR  "gray"
#define GPS_COLOR  "gray"
#define VISION_COLOR  "gray"
#define PLAYER_COLOR  "black"
#define LASERBEACON_COLOR  "cyan"
#define WALL_COLOR "black"
#define IDAR_COLOR "DarkRed"
#define DESCARTES_COLOR "green"

#else

#define GRIPPER_COLOR "gray"
#define POSITION_COLOR "red"
#define LASER_COLOR  "blue"
#define SONAR_COLOR  "blue3"
#define PTZ_COLOR  "magenta"
#define PUCK_COLOR  "green"
#define BOX_COLOR  "yellow"
#define LBD_COLOR  "gray"
#define MISC_COLOR  "gray"
#define GPS_COLOR  "gray"
#define VISION_COLOR  "gray"
#define PLAYER_COLOR  "white"
#define LASERBEACON_COLOR  "cyan"
#define WALL_COLOR "white"
#define IDAR_COLOR "DarkRed"
#define DESCARTES_COLOR "green"

#endif

#endif

/*************************************************************************
 * ports.h - is this even used?
 * RTV
 * $Id: ports.h,v 1.2 2001-02-06 01:27:07 gerkey Exp $
 ************************************************************************/

#ifndef PORTS_H
#define PORTS_H

const long int visionInPort = 50000;
const long int visionOutPort = 50010;
const long int GUIInPort = 50020;
const long int visionGUIInPort = 50030;
const long int visionGUIOutPort = 50040;

const long int robotServerPort = 50050;

#define POSITION_REQUEST_PORT 50102
#define COMMS_REQUEST_PORT 50103

#define MAXCMD  64
#define MAXMSG  1024
#define MAXREPLY  1024
#define MAXPOSITIONSTRING 1024
#define COMMSMESGLENGTH 2048

#endif

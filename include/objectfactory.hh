///////////////////////////////////////////////////////////////////////////
//
// File: createobject.hh
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: Naked C functions for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/objectfactory.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.1 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////


#ifndef OBJECTFACTORY_HH
#define OBJECTFACTORY_HH

// Forward dec's
//
class CObject;
class CWorld;
class CRobot;

// Create an object given a type
//
CObject* CreateObject(const char *type, CWorld *world, CObject *parent);

// Create a device given a type
//
CObject* CreateDevice(const char *type, CWorld *world, CRobot *robot);


#endif

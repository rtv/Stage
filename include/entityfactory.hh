///////////////////////////////////////////////////////////////////////////
//
// File: createobject.hh
// Author: Andrew Howard
// Date: 05 Dec 2000
// Desc: Naked C functions for creating different types of objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/entityfactory.hh,v $
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


#ifndef ENTITYFACTORY_HH
#define ENTITYFACTORY_HH

// Forward dec's
//
class CEntity;
class CWorld;

// Create an object given a type
// Additional arguments can be passed in through argv.
//
CEntity* CreateObject(const char *type, CWorld *world, CEntity *parent);

#endif

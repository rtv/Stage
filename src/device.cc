///////////////////////////////////////////////////////////////////////////
//
// File: device.cc
// Author: Andrew Howard
// Date: 28 Nov 2000
// Desc: Base class for all devices
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/device.cc,v $
//  $Author: ahoward $
//  $Revision: 1.5 $
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

#include "world.h"
#include "device.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
//
CDevice::CDevice(CRobot* robot)
{
    m_robot = robot;
    m_world = m_robot->world;
}


///////////////////////////////////////////////////////////////////////////
// Default startup -- doesnt do much
//
bool CDevice::Startup()
{
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Default shutdown -- doesnt do much
//
bool CDevice::Shutdown()
{
    return true;
}


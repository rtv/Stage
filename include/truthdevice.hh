///////////////////////////////////////////////////////////////////////////
//
// File: truthdevice.hh
// Author: Richard Vaughan
// Date: 5 Dec 2000
// Desc: Simulates the Pioneer robot base
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/truthdevice.hh,v $
//  $Author: vaughan $
//  $Revision: 1.1 $
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

#ifndef _TRUTHDEVICE_H
#define _TRUTHDEVICE_H

#include "entity.hh"
#include "stage.h"
//#include "playerdevice.hh"

class CTruthDevice : public CEntity
{
    // Minimal constructor
    //
    public: CTruthDevice(CWorld *world, CEntity *parent );

    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Extract command from the command buffer
    //
    private: void ParseCommandBuffer(player_position_cmd_t &command );
				    
    // Compose data
    //
    private: void ComposeDataBuffer(unsigned  &position );

};

#endif

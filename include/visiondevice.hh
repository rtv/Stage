///////////////////////////////////////////////////////////////////////////
//
// File: visiondevice.hh
// Author: Andrew Howard
// Date: 30 Nov 2000
// Desc: Simulates the ACTS vision system
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/visiondevice.hh,v $
//  $Author: ahoward $
//  $Revision: 1.2 $
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

#ifndef VISIONDEVICE_HH
#define VISIONDEVICE_HH

#include "playerdevice.hh"

class CVisionDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CVisionDevice(CRobot* robot, void *buffer, size_t data_len,
                          size_t command_len, size_t config_len);
    
    // Update the device
    //
    public: virtual bool Update();

    // Timing properties
    //
    public: double m_last_update, m_update_interval;

    // Camera properties
    //
    private: double cameraAngleMax, cameraAngleMin, cameraFOV;
    private: double cameraPanMin, cameraPanMax, cameraPan;
    private: int cameraImageWidth, cameraImageHeight;

    private: int numBlobs;
    private: ColorBlob blobs[MAXBLOBS];

    private: unsigned char actsBuf[ACTS_TOTAL_MAX_SIZE];
};

#endif

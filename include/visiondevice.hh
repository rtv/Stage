///////////////////////////////////////////////////////////////////////////
//
// File: visiondevice.hh
// Author: Andrew Howard
// Date: 30 Nov 2000
// Desc: Simulates the ACTS vision system
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/visiondevice.hh,v $
//  $Author: vaughan $
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

#ifndef VISIONDEVICE_HH
#define VISIONDEVICE_HH

#include "playerdevice.hh"

// Forward declaration for ptz device
//
class CPtzDevice;


class CVisionDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CVisionDevice(CRobot *robot, CPtzDevice *ptz_device,
                          void *buffer, size_t data_len,
                          size_t command_len, size_t config_len);
    
  // destructor explcitly deletes some buffers
    public: ~CVisionDevice( void );

    // Update the device
    //
    public: virtual bool Update();

    // Timing properties
    //
    private: double m_last_update, m_update_interval;

    // Pointer to the ptz device we attach to
    //
    private: CPtzDevice *m_ptz_device;

    // Camera properties
    //
    private: int cameraImageWidth, cameraImageHeight;

    // Detected blob data
    //
    private: int numBlobs;
    private: ColorBlob blobs[MAXBLOBS];
    private: unsigned char actsBuf[ACTS_TOTAL_MAX_SIZE];

  // working buffers for the update function
    private: unsigned char* colors;
    private: float* ranges;
};

#endif






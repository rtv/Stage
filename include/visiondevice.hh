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
//  $Revision: 1.12 $
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

#include "entity.hh"

// Forward declaration for ptz device
//
class CPtzDevice;

#define MAXBLOBS 100 // PDOMA!

typedef struct
{
  unsigned char channel;
  int area;
  int x, y;
  int left, top, right, bottom;
} ColorBlob;


class CVisionDevice : public CEntity
{
    // Default constructor
    //
    public: CVisionDevice(CWorld *world, CPtzDevice *parent, StageColor* channel_map );
    
    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Generate the scan-line image
    //
    private: void UpdateScan();

    // Generate ACTS data from scan-line image
    //
    private: size_t UpdateACTS();

    // Pointer to the ptz device we attach to (same as out parent
    //
    private: CPtzDevice *m_ptz_device;

    // Current pan/tilt/zoom settings (all angles)
    //
    private: double m_pan, m_tilt, m_zoom;

  // channel to color map array
  StageColor channel[ ACTS_NUM_CHANNELS ];
  
    // Camera properties
    //
    private: int cameraImageWidth, cameraImageHeight;
    private: double m_max_range;

    // Current scan-line data
    //
    private: int m_scan_width;
    private: int m_scan_channel[256];
    private: double m_scan_range[256];

    // Detected blob data
    //
    private: int numBlobs;
    private: ColorBlob blobs[MAXBLOBS];
    private: unsigned char actsBuf[ACTS_TOTAL_MAX_SIZE];

#ifdef INCLUDE_RTK
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

    // Draw the field of view
    //
    private: void DrawFOV(RtkUiDrawData *pData);
    
    // Draw the image scan-line
    //
    private: void DrawScan(RtkUiDrawData *pData);

    // Laser scan outline
    //
    private: int m_hit_count;
    private: double m_hit[256][6];
    
#endif  
};

#endif






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
 * Desc: Device to simulate the ACTS vision system.
 * Author: Richard Vaughan, Andrew Howard
 * Date: 28 Nov 2000
 * CVS info: $Id: visiondevice.hh,v 1.1 2002-08-23 00:19:39 rtv Exp $
 */

#ifndef VISIONDEVICE_HH
#define VISIONDEVICE_HH

#include "playerdevice.hh"

// Forward declaration for ptz device
class CPtzDevice;

#define MAXBLOBS 100 // PDOMA!

typedef struct
{
  unsigned char channel;
  int area;
  int x, y;
  int left, top, right, bottom;
  int range; //mm
} ColorBlob;


class CVisionDevice : public CPlayerEntity
{
  // Default constructor
  public: CVisionDevice(CWorld *world, CPtzDevice *parent);

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CVisionDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CVisionDevice( world, (CPtzDevice*)parent ) ); }


  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);
  
  // Update the device
  public: virtual void Update( double sim_time );

  // Generate the scan-line image
  private: void UpdateScan();

  // Generate ACTS data from scan-line image
  private: size_t UpdateACTS( player_blobfinder_data_t* data );

  // Pointer to the ptz device we attach to (same as out parent
  private: CPtzDevice *m_ptz_device;

  // Current pan/tilt/zoom settings (all angles)
  private: double m_pan, m_tilt, m_zoom;

  // channel to color map array
  private: int channel_count;
  private: StageColor channels[PLAYER_BLOBFINDER_MAX_CHANNELS];
  
  // Camera properties
  private: int cameraImageWidth, cameraImageHeight;
  private: double m_max_range;

  // Current scan-line data
  private: int m_scan_width;
  private: int m_scan_channel[256];
  private: double m_scan_range[256];

  // Detected blob data
  private: int numBlobs;
  private: ColorBlob blobs[MAXBLOBS];

  //private: player_vision_data_t actsBuf;

#ifdef INCLUDE_RTK2
  // Initialise the rtk gui
  protected: virtual void RtkStartup();
  
  // Finalise the rtk gui
  protected: virtual void RtkShutdown();
  
  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
  // For drawing a little screen with blobs on
  private: rtk_fig_t *vision_fig;
#endif

};

#endif






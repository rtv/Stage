///////////////////////////////////////////////////////////////////////////
//
// File: ptzdevice.hh
// Author: Andrew Howard
// Date: 01 Dec 2000
// Desc: Simulates the Sony pan-tilt-zoom camera
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/ptzdevice.hh,v $
//  $Author: rtv $
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

#ifndef PTZDEVICE_HH
#define PTZDEVICE_HH


#include "playerdevice.hh"


class CPtzDevice : public CPlayerEntity
{
    // Default constructor
    //
    public: CPtzDevice( LibraryItem *libit, CWorld *world, CEntity *parent );
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CPtzDevice* Creator(  LibraryItem *libit, CWorld *world, CEntity *parent )
  { return( new CPtzDevice( libit, world, parent ) ); }
  
  // Startup routine
  public: virtual bool Startup();

    // Load the entity from the worldfile
    public: virtual bool Load(CWorldFile *worldfile, int section);
    
    // Update the device
    //
    public: virtual void Update( double sim_time );

    // Get the pan/tilt/zoom values
    // The pan and tilt are angles (in radians)
    // The zoom is a the field of view (in radians)
    //
    public: void GetPTZ(double &pan, double &tilt, double &zoom);

    // Physical limits
    //
    private: double m_pan_min, m_pan_max;
    private: double m_tilt_min, m_tilt_max;
    private: double m_zoom_min, m_zoom_max;
    private: double m_fov_min, m_fov_max;

    // Current camera settings
    //
    private: double m_pan, m_tilt, m_zoom;  

#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);
    
#endif

#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
  // For drawing the laser scan & intensity values
  private: rtk_fig_t *fov_fig;
#endif

};

#endif





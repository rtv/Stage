///////////////////////////////////////////////////////////////////////////
//
// foofinderdevice.cc 
// Richard Vaughan 
// created 20030328
//
//  $Id: foofinderdevice.hh,v 1.1 2003-04-01 00:20:56 rtv Exp $
///////////////////////////////////////////////////////////////////////////

// LBD = Laser Beacon Detector!

#ifndef LBDDEVICE_HH
#define LBDDEVICE_HH

#include "playerdevice.hh"
#include "laserdevice.hh"

// the foofinder uses the fiducial interface, but selects it's targets
// are selected based on the following modes, rather than just
// fidicial ID:
typedef enum { STG_FOOFINDER_MODE_FIDUCIAL=0, // by fiducial id
	       STG_FOOFINDER_MODE_MODEL, // by model name
	       STG_FOOFINDER_MODE_TOPLEVEL, // only devices on the root
	       STG_FOOFINDER_MODE_MOVEMENT, // moving things
	       STG_FOOFINDER_MODE_SOUND, // sound emmitters
	       STG_FOOFINDER_MODE_RADIATION, // radiation emmitters
	       STG_FOOFINDER_MODE_TEAM, // by team identifier
	       STG_FOOFINDER_MODE_ROBOTID, // by robot identifier
	       STG_FOOFINDER_MODE_COLOR // by colrt
} stg_foofinder_mode_t;


class CFooFinder : public CPlayerEntity
{
  // Default constructor
  public: CFooFinder( LibraryItem *libit, CWorld *world, CEntity *parent );

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CFooFinder* Creator(  LibraryItem *libit, CWorld *world, CEntity *parent )
  { return( new CFooFinder( libit, world, parent ) ); }
    
  // Startup routine
  public: virtual bool Startup();

  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);

  // Update the device
  public: virtual void Update( double sim_time );

  public: virtual void UpdateConfig( void );

  // Time of last update
  private: uint32_t time_sec, time_usec;

  // Detection parameters
protected: double max_range, min_range, view_angle;

protected: bool perform_occlusion_test;
protected: bool OcclusionTest(CEntity* ent );

protected: 
  stg_foofinder_mode_t mode;
  double threshold; // general detection threshold, used by several modes

  // mode-specific parameters (syntax: <mode>_<variable name>)

  // if true, relative velocities measured, else absolute
  int movement_relative;

  // the angular speed hi-pass threshold
  float movement_theta_threshold;

  // filter by this worldfile token
  char model_token[STAGE_TOKEN_MAX];
  
  // filter by this color
  StageColor color_match;

#ifdef INCLUDE_RTK2

  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();
  
  // For drawing the sonar beams
  private: rtk_fig_t *beacon_fig;
#endif

};

#endif







///////////////////////////////////////////////////////////////////////////
//
// File: gripperdevice.hh
// Author: brian gerkey
// Date: 09 Jul 2001
// Desc: Simulates a simple gripper
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/gripperdevice.hh,v $
//  $Author: gerkey $
//  $Revision: 1.2 $
//
///////////////////////////////////////////////////////////////////////////

#ifndef GRIPPERDEVICE_HH
#define GRIPPERDEVICE_HH


#include "playerdevice.hh"

/* 
 * gripper constants
 * 
 * not a great place to put them.  in fact, is it a good idea to make
 * the interface to the simulated gripper match the P2 gripper?
 * 
 */
#define GRIPopen   1
#define GRIPclose  2
#define GRIPstop   3
#define LIFTup     4
#define LIFTdown   5
#define LIFTstop   6
#define GRIPstore  7
#define GRIPdeploy 8
#define GRIPhalt   15
#define GRIPpress  16
#define LIFTcarry  17

#define MAXGRIPPERCAPACITY 1000

class CGripperDevice : public CPlayerEntity
{
  // Default constructor
  //
  public: CGripperDevice(CWorld *world, CEntity *parent ); 

  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CGripperDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CGripperDevice( world, parent ) ); }

  // Startup routine
  public: virtual bool Startup();
  
  // update the break beams with the raytrace functions
  // beam is 0 - inside or 1 - outside
  // returns pointer to the puck in the beam, or NULL if none
  private: CEntity* BreakBeam( int beam );

  // Update the device
  //
  public: virtual void Update( double sim_time );

  // Package up the gripper's state in the right format
  //
  private: void MakeData(player_gripper_data_t* data, size_t len);

  // Drop an object from the gripper (if there is one)
  //
  private: void DropObject();
    
  // Try to pick up an object with the gripper
  //
  private: void PickupObject();

  // Current gripper state
  //
  //  note that these will be folded in as bitmasks to
  //  the gripper data. also note that not all of these are
  //  used right now 
  //
  //  we're adhering to the OLD P2 gripper (i.e., the one that
  //  attaches to the User I/O port on the pioneer)
  private: bool m_paddles_open, m_paddles_closed, m_paddles_moving,
    m_gripper_error, m_lift_up, m_lift_down,
    m_lift_moving, m_lift_error, m_inner_break_beam,
    m_outer_break_beam;
                   
  // this is the maximum number of pucks that we can carry
  private: int m_puck_capacity;

  // these are used both for locating break beams and for visualization
  private: double m_paddle_width;
  private: double m_paddle_height;

  // extra colors used for drawing break beams
  private: StageColor broken_beam_color;
  private: StageColor clear_beam_color;

  // whether or not we "consume" pucks.  if we do consume, then pucks
  // are essentially eaten by the gripper and can never be dropped (like
  // a vacuum  cleaner); if we do not consume, then we only hold one puck
  // at a time and it can be dropped
  private: bool m_gripper_consume;

  // these are the pucks we're carrying right now
  private: int m_puck_count;
  private: CEntity* m_pucks[MAXGRIPPERCAPACITY];
  
  // Load the entity from the worldfile
  public: virtual bool Load(CWorldFile *worldfile, int section);



  // Gripper dimenions
  //private: double m_width, m_height;

#ifdef INCLUDE_RTK2
    
  // Initialise the rtk gui
  protected: virtual void RtkStartup();

  // Finalise the rtk gui
  protected: virtual void RtkShutdown();

  // Update the rtk gui
  protected: virtual void RtkUpdate();

  private: rtk_fig_t *grip_fig;
  private: rtk_fig_t *inner_beam_fig;
  private: rtk_fig_t *outer_beam_fig;
#endif

};

#endif





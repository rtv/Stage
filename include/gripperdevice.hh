///////////////////////////////////////////////////////////////////////////
//
// File: gripperdevice.hh
// Author: brian gerkey
// Date: 09 Jul 2001
// Desc: Simulates a simple gripper
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/gripperdevice.hh,v $
//  $Author: gerkey $
//  $Revision: 1.3.2.1 $
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

class CGripperDevice : public CPlayerDevice
{
    // Default constructor
    //
    public: CGripperDevice(CWorld *world, 
                           CEntity *parent, 
                           CPlayerServer* server);
    
    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);
    
    // Update the device
    //
    public: virtual void Update();

    // Package up the gripper's state in the right format
    //
    private: void MakeData(player_gripper_data_t* data, size_t len);

    // Drop an object from the gripper (if there is one)
    //
    private: void DropObject();
    
    // Try to pick up an object with the gripper
    //
    private: void PickupObject();

    // Update times
    //
    private: double m_last_update, m_update_interval;
    
    // Current gripper state
    //
    //  note that these will be folded in as bitmasks to
    //  the gripper data. also note that not all of these are
    //  used right now (e.g., the simulated gripper doesn't actually
    //  have breakbeams)
    //
    //  we're adhering to the OLD P2 gripper (i.e., the one that
    //  attaches to the User I/O port on the pioneer)
    private: bool m_paddles_open, m_paddles_closed, m_paddles_moving,
                  m_gripper_error, m_lift_up, m_lift_down,
                  m_lift_moving, m_lift_error,m_inner_breakbeam,
                  m_outer_breakbeam;
                   
    // this is the range at which the gripper will actually pick
    // up a puck
    private: double m_gripper_range;

    // this is the maximum number of pucks that we can carry
    private: int m_puck_capacity;

    // whether or not we "consume" pucks.  if we do consume, then pucks
    // are essentially eaten by the gripper and can never be dropped (like
    // a vacuum  cleaner); if we do not consume, then we only hold one puck
    // at a time and it can be dropped
    private: bool m_gripper_consume;
    
    // these are the pucks we're carrying right now
    private: int m_puck_count;
    private: CEntity* m_pucks[MAXGRIPPERCAPACITY];

    // structure for exporting Gripper-specific data to a GUI
    private: ExportGripperData expGripper; 

#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);
    
#endif

};

#endif





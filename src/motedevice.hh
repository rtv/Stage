#ifndef MOTEDEVICE_HH
#define MOTEDEVICE_HH 

#include "playerdevice.hh"
#include <list>

/* useful macros */
/* change to cm (or hundredths of whatever units were in) */
#define Distance(a, b) 100*sqrt((a->px - b->px)*(a->px - b->px) + (a->py - b->py)*(a->py - b->py))

#define RSSI(a) (this->m_strength * (1 / (Distance(a, this) * Distance(a, this))))

// RTV - this def was missing from my file - is this a reasonable value?
// BPG - i have no idea, but it's now defined as 10 in
// player/include/messages.h, so i've commented out this one
//#define MAX_MOTE_Q_LEN 20

class CMoteDevice;

// update the connectivity graph
void UpdateGraph(void);

// update the adjacency list of node n and x
void UpdateAdjList(CMoteDevice* n, CMoteDevice* x);

class CMoteDevice : public CPlayerEntity
{
    // Default constructor
    public: CMoteDevice(CWorld *world, CEntity *parent );
 
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
public: static CMoteDevice* Creator( CWorld *world, CEntity *parent )
  { return( new CMoteDevice( world, parent ) ); }

    // Update the device
    //
    public: virtual void Update(double sim_time );
   
    public: int EnqueueMsg(player_mote_data_t *msg);

    public: int id;

    public: bool Load(CWorldFile *worldfile, int section);

    // Check to see if the configuration has changed
    // private: bool GetConfig();

    // function to compute signal coverage
    public: inline double SignalCoverage(void);

    // Mote timing settings
    private: double m_update_rate;

    public: double px, py, pth;

    // Buffers for storing data
    private: uint8_t m_strength;

    private: player_mote_config_t m_config;
    private: player_mote_data_t m_data;

    private: player_mote_data_t msg_q[MAX_MOTE_Q_LEN];
    private: uint8_t q_size;
  
    /* list of neighbors stuff */
    public: std::list<CMoteDevice*> adj;

    /* graph stuff */
    protected: char color;
    protected: char d_time;
    protected: char f_time;
    private: double m_last_graph_update;

#ifdef INCLUDE_RTK
    private: void OnUiUpdate(RtkUiDrawData *data);
#endif 

#ifdef INCLUDE_RTK2
    private: void RtkUpdate();
#endif 
};

#endif



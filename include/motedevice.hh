#ifndef MOTEDEVICE_HH
#define MOTEDEVICE_HH 

#include "entity.hh"
#include <slist.h>

/* useful macros */
/* change to cm (or hundredths of whatever units were in) */
#define Distance(a, b) 100*sqrt((a->px - b->px)*(a->px - b->px) + (a->py - b->py)*(a->py - b->py))

#define RSSI(a) (this->m_strength * (1 / (Distance(a, this) * Distance(a, this))))


class CMoteDevice;

// update the connectivity graph
void UpdateGraph(void);

// update the adjacency list of node n and x
void UpdateAdjList(CMoteDevice* n, CMoteDevice* x);

class CMoteDevice : public CEntity
{
    // Default constructor
    public: CMoteDevice(CWorld *world, CEntity *parent );
 
    // Update the device
    //
    public: virtual void Update(double sim_time );

    public: int id;

    public: bool Load(CWorldFile *worldfile, int section);

    // Check to see if the configuration has changed
  //    private: bool GetConfig();

    // function to compute signal coverage
    public: inline double SignalCoverage(void);

    // Mote timing settings
    private: double m_update_rate;

    public: double px, py, pth;

    // Buffers for storing data
    private: uint8_t m_strength;

    private: player_mote_config_t m_config;
    private: player_mote_data_t m_data;

    /* list of neighbors stuff */
    public: slist<CMoteDevice*> adj;

    /* graph stuff */
    protected: char color;
    protected: char d_time;
    protected: char f_time;
    private: double m_last_graph_update;

#ifdef INCLUDE_RTK
    private: void OnUiUpdate(RtkUiDrawData *data);
#endif 
};

#endif



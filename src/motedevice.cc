/* Author: GTS */

#include <stage.h>
#include <list> 
#include <algorithm> 
#include "world.hh"
#include "motedevice.hh" 

double m_graph_update_interval = 0.1;

slist<CMoteDevice*> m_mote_list;
#undef DEBUG
#undef VERBOSE
//#define DEBUG
//#define VERBOSE

///////////////////////////////////////////////////////////////////////////
// Default constructor
//

CMoteDevice::CMoteDevice(CWorld *world, CEntity *parent )
  : CEntity(world, parent)
{
  m_data_len    = MAX_MOTE_Q_LEN * sizeof( player_mote_data_t );
  m_command_len = sizeof( player_mote_data_t );
  m_config_len  = 1;
  m_reply_len  = 1;
  
  m_player_type = PLAYER_MOTE_CODE;
  m_stage_type = MoteType;


  m_interval = 0.01;
  m_graph_update_interval = 0.2;

  /* get global pose*/
  GetGlobalPose(this->px,this->py, this->pth);

  /* signal strength defaults to 100 */
  m_strength = 25;

  q_size = 0; /* no messeges to send */

  m_last_graph_update = 0.0;

  /* add the mote to the graph  */
  for(slist<CMoteDevice*>::iterator i = m_mote_list.begin(); 
      i != m_mote_list.end(); i++){
    UpdateAdjList(*i, this);
  }

  /* list of motes */
  m_mote_list.push_front(this); 
}

///////////////////////////////////////////////////////////////////////////
// Load the object from file
//
bool CMoteDevice::Load(CWorldFile *worldfile, int section)
{
  if (!CEntity::Load(worldfile, section))
    return false;
  return true; 
}


///////////////////////////////////////////////////////////////////////////
//
void CMoteDevice::Update( double sim_time )
{
  ASSERT(m_world != NULL);

  if(!Subscribed())
    return;

  if( sim_time - m_last_update < m_interval )
    return;

  /* time to update the connectivity graph? */
  if( sim_time - m_last_graph_update >= m_graph_update_interval){
    GetGlobalPose(this->px,this->py, this->pth);
    UpdateGraph();
    m_last_graph_update = sim_time;
  }
  m_last_update = sim_time;
  
  /* check if transmission power has changed */
  void* client;
  player_mote_config_t config;
  if(GetConfig(&client, &config, sizeof(config)) > 0)
  {
    m_strength = config.strength;
    PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, NULL, 0);
  }
  
  // Get messege
  //
  if( m_info_io->command_avail ){
    if(GetCommand(&m_data, sizeof(player_mote_data_t)) == 
       sizeof(player_mote_data_t)){
      /* copy the messege to the adjacent nodes msg queues*/
      for(slist<CMoteDevice*>::iterator it=adj.begin();it != adj.end();it++){
#ifdef DEBUG
	char dbg[32];
	memcpy(dbg, m_data.buf, 32);
	dbg[1] = 0;
	printf("%d sending %s to %d\n", 
	       m_player_index, dbg, (*it)->m_player_index);
#endif
	m_data.rssi = RSSI(((CMoteDevice*)*it));
	(*it)->EnqueueMsg(&m_data);
      }
    }
    /* show that we comsumed the messege */
    m_info_io->command_avail = 0;
  }
  /* send the queued messeges */
  if(q_size){
    PutData((void*)msg_q, MAX_MOTE_Q_LEN * sizeof(player_mote_data_t));
    q_size = 0;
  }
#ifdef INCLUDE_RTK2
  RtkUpdate();
#endif
}

///////////////////////////////////////////////////////////////////////////

int CMoteDevice::EnqueueMsg(player_mote_data_t* msg)
{
  if(q_size >= MAX_MOTE_Q_LEN){ /* error, q overflow */
    return -1;
  }
  memcpy((void*)&msg_q[q_size++], msg,  sizeof(player_mote_data_t));
  m_info_io->data_avail = MAX_MOTE_Q_LEN * sizeof(player_mote_data_t);
  msg_q[q_size].len = 0; /* end of the q */
  return 1;
}

///////////////////////////////////////////////////////////////////////////

inline double CMoteDevice::SignalCoverage()
{
  /* 1/r^2 is how signal strength fades */
  /* thus coverage is sqrt of signal strength 
     (into 100 since we convert in Distance() to cm */
  return 100*sqrt(this->m_strength);
}


///////////////////////////////////////////////////////////////////////////
/* 
   maintain pointers between adjacent covered nodes
*/

void UpdateAdjList(CMoteDevice* n, CMoteDevice* x)
{
  double distance = Distance(n, x);

  /* each node keeps track of who they cover, and that's it */
  /* n covers x */
  if(distance < n->SignalCoverage()){
    /* if x is not already in n's adjacency list, then add it */
    if( find(n->adj.begin(), n->adj.end(), x) == n->adj.end()){
      n->adj.push_front(x);
    }
  }
  else{
    n->adj.remove(x);
  }

  /* x covers n */
  if(distance < x->SignalCoverage()){
    /* if n is not already in x's adjacency list, then add it */
    if( find(x->adj.begin(), x->adj.end(), n) == x->adj.end()){
      x->adj.push_front(n);
    }
  }
  else{
    x->adj.remove(n);
  }
}

//////////////////////////////////////////////////////////////////////////
/* update adjacency list for each node in G */
void UpdateGraph(void)
{
  for(slist<CMoteDevice*>::iterator i = m_mote_list.begin(); 
      i != m_mote_list.end(); i++){
    for(slist<CMoteDevice*>::iterator j = i;
	j != m_mote_list.end(); j++){
      if(*i != *j){
	UpdateAdjList(*i, *j);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////
#ifdef INCLUDE_RTK
void CMoteDevice::OnUiUpdate(RtkUiDrawData *data)
{
  double link;
  
  data->begin_section("global","mote");
  if(data->draw_layer("coverage", false) && Subscribed()) {
    
    double r = SignalCoverage()/100;
    data->set_color(RTK_RGB(0, 200, 0));
    data->ellipse(this->px-r,this->py-r,this->px+r,this->py+r);
  }
    
  if(data->draw_layer("connectivity", false) && Subscribed()) {
    for( slist<CMoteDevice*>::iterator ptr = adj.begin(); 
	 ptr != adj.end(); ptr++){

    
      link =  2*sqrt(1/(RSSI( ((CMoteDevice*)*ptr)  )));
#ifdef DEBUG
      printf("dist %f, rssi %f, link %f\n", 
	     Distance(this, (*ptr)),
	     RSSI( ((CMoteDevice*)*ptr)),
	     link);
#endif

      data->set_color(RTK_RGB(link, link, link));
      data->line(this->px, this->py, (*ptr)->px, (*ptr)->py);
    }
  }
  data->end_section();
}
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef INCLUDE_RTK2
void CMoteDevice::RtkUpdate()
{

}

#endif


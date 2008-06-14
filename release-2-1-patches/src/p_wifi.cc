/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *                      
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
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Richard Vaughan
 * Date: 10 December 2004
 * CVS: $Id: p_wifi.cc,v 1.5 2007-12-05 21:51:47 gerkey Exp $
 */


#include "p_driver.h"

extern "C" { 
int wifi_init( stg_model_t* mod );
}

InterfaceWifi::InterfaceWifi( player_devaddr_t addr, 
                              StgDriver* driver,
                              ConfigFile* cf,
                              int section )
  
  : InterfaceModel( addr, driver, cf, section, wifi_init )
{
  // nothing to do
}

void InterfaceWifi::Publish( void )
{
  stg_wifi_data_t* sdata = (stg_wifi_data_t*)mod->data;
  assert(sdata);

  player_wifi_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );

  // Translate the Stage-formatted sdata into the Player-formatted pdata
  pdata.links_count = sdata->neighbours->len;
  pdata.links = new player_wifi_link_t[pdata.links_count];
  assert(pdata.links);
  memset(pdata.links,0,sizeof(player_wifi_link_t)*pdata.links_count);
  for(guint i=0;i<sdata->neighbours->len;i++)
  {
    stg_wifi_sample_t samp = g_array_index(sdata->neighbours, 
                                           stg_wifi_sample_t, i);
    memcpy(pdata.links[i].mac,samp.mac,sizeof(pdata.links[i].mac));
    pdata.links[i].mac_count = sizeof(pdata.links[i].mac);
    //printf("%f\n", samp.db);
  }

  // Publish it
  this->driver->Publish(this->addr,
			PLAYER_MSGTYPE_DATA,
			PLAYER_WIFI_DATA_STATE,
			(void*)&pdata, sizeof(pdata), NULL);
  delete [] pdata.links;
}

int InterfaceWifi::ProcessMessage(QueuePointer &resp_queue,
                                  player_msghdr_t* hdr,
                                  void* data)
{
  // PROCESS INCOMING REQUESTS HERE
  return 0;
}

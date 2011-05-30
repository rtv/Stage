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
 * CVS: $Id$
 */


#include "p_driver.h"
using namespace Stg;

InterfaceWifi::InterfaceWifi( player_devaddr_t addr, 
                              StgDriver* driver,
                              ConfigFile* cf,
                              int section )
  
  : InterfaceModel( addr, driver, cf, section, "wifi" )
{
  // nothing to do
}

void InterfaceWifi::Publish( void )
{

  ModelWifi* wifi =  dynamic_cast<ModelWifi*>(this->mod);
  wifi_data_t* sdata = wifi->GetLinkInfo();
  WifiConfig* config = wifi->GetConfig();

  assert(sdata);

  player_wifi_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  // Translate the Stage-formatted sdata into the Player-formatted pdata
  pdata.links_count = sdata->neighbors.size();
  printf("Got %d links\n", pdata.links_count);
  
  if (pdata.links_count > 0)
  {
	  pdata.links = new player_wifi_link_t[pdata.links_count];
	  memset(pdata.links, 0, sizeof(player_wifi_link_t)*pdata.links_count);	  
  }

  for (unsigned int i = 0; i < pdata.links_count; i++)
  {
  	unsigned int len = sdata->neighbors[i].GetEssid().length();
	  memcpy(pdata.links[i].essid, sdata->neighbors[i].GetEssid().c_str(), len);
	  pdata.links[i].essid_count = len;
	  
	  len = sdata->neighbors[i].GetMac().length();
	  memcpy(pdata.links[i].mac, sdata->neighbors[i].GetMac().c_str(), len);
	  pdata.links[i].mac_count = len;
	  
	  in_addr_t ipaddr = sdata->neighbors[i].GetIp();
	  char* ipstr = inet_ntoa(*(struct in_addr*) &ipaddr);
	  len = strlen(ipstr);
	  memcpy(&pdata.links[i].mac, ipstr, len);
	  pdata.links[i].mac_count = len;
	  
	  pdata.links[i].freq = round(sdata->neighbors[i].GetFreq());
	  pdata.links[i].level = sdata->neighbors[i].GetDb();
  }

  // Publish it
  this->driver->Publish(this->addr,
			PLAYER_MSGTYPE_DATA,
			PLAYER_WIFI_DATA_STATE,
			(void*)&pdata, sizeof(pdata), NULL);
			
	delete[] pdata.links;
}

int InterfaceWifi::ProcessMessage(QueuePointer& resp_queue,
                                  player_msghdr_t* hdr,
                                  void* data)
{
  return -1;
}

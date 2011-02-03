///////////////////////////////////////////////////////////////////////////
//
// File: communication.cc
// Author: Rahul Balani
// Date: 27 May 2009
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_comm Inter-Robot Data Communication Interface 

Description: To fill

<h2>Worldfile properties</h2>

@par SetReceiveFn()

@verbatim
Communication::
@endverbatim

@par SetArg()

@verbatim
Communication::
@endverbatim

@par CallReceiveFn()

@verbatim
Communication::
@endverbatim

@par Configuration Parameters
- ReceiveFn
  - Function to call when a message is received from another robot
- arg
  - User defined argument to pass to ReceiveFn along with received message
*/

//#include <assert.h>
//#include <sys/time.h>
//#include <math.h>
//#include <limits.h>
#include "stage.hh"
using namespace Stg;

//----------------------------------Communication System-----------------------------

/**
 * The contructor for simulated inter-robot communication overlaid on the WIFI model.
 * We need access to the WIFI model so we can determine who messages are sent to. 
 */
Communication::Communication(ModelWifi * parent_p)
{
  wifi_p = parent_p;

  PRINT_DEBUG( "Initializing Communication Interface" );


}//end comm constructor


/**
 * Destructor for communication system.
 */
Communication::~Communication( void )
{
}//end Communication destructor


/**
 * Set the message sender based on the configuration of the current comm interface. 
 */
void Communication::SetMessageSender(WifiMessageBase * to_send)
{
  WifiConfig *config = wifi_p->GetConfig();

  //Set some attributes on the message to send based on who the sender is
  to_send->SetSenderId(wifi_p->GetId());
  to_send->SetSenderIp(config->GetIp());
  to_send->SetSenderNetmask(config->GetNetmask());
  to_send->SetSenderMac(config->GetMac());
}//end SetMessageSender


/**
 * Send the message as a broadcast to the broadcast IP of the sender. 
 */
void Communication::SendBroadcastMessage(WifiMessageBase * to_send)
{
  SetMessageSender(to_send);
  to_send->SetBroadcastIp();
  SendMessage(to_send);
}//end SendBroadcastMessage


/**
 * Send the message as a unicast to the intended recipient.
 */
void Communication::SendUnicastMessage(WifiMessageBase * to_send)
{
  SetMessageSender(to_send);
  SendMessage(to_send);
}//end SendUnicastMessage


/**
 * Send a wifi message to recipient in range who are a member of the same ESSID, in the same network
 * and who's IP address/netmask match the recipient of the message.
 */
void Communication::SendMessage(WifiMessageBase * to_send) 
{
  //Get link and wifi info for the current wifi
  wifi_data_t *linkdata = wifi_p->GetLinkInfo();

  for (unsigned int i=0; i < linkdata->neighbors.size(); i++) 
  {
    //Essid must match
    if (linkdata->neighbors[i].GetEssid().compare(wifi_p->wifi_config.GetEssid()) == 0)
    {

      //Check if the networks match
      in_addr_t sender_network = wifi_p->wifi_config.GetIp() & wifi_p->wifi_config.GetNetmask();
      in_addr_t recip_network = linkdata->neighbors[i].GetIp() & linkdata->neighbors[i].GetNetmask();

      if ( sender_network == recip_network )
      {

        //Determine if the message is destined for the broadcast address or
        //specifically addressed to hte current neighbor
        if ( ( to_send->GetRecipientIp() == linkdata->neighbors[i].GetIp() ) ||
             ( to_send->IsBroadcastIp() ) )
        {
          //Yay its destined for here!
          //Get the nieghbor's comm interface and try to call its receive message function
          Communication * neighbor_comm = linkdata->neighbors[i].GetCommunication();
          if (neighbor_comm->IsReceiveMsgFnSet())
          {
            //Make a deep clone of the message to send (this way if we're really pointing at 
            //a subclass, we get the subclass copied over, not just the base).

            WifiMessageBase * message_copy = to_send->Clone();
            message_copy->SetRecipientId(neighbor_comm->wifi_p->GetId());      
            neighbor_comm->CallReceiveMsgFn(message_copy);
          }
        }
      }//end network match
    }//end same ESSID
  }//end for

}//end SendMessageToAllInRange


//------------------------------Wifi Messages----------------------------

/**
 * WifiMessageBase copy constructor.
 */
WifiMessageBase::WifiMessageBase(const WifiMessageBase& toCopy)
{
  sender_mac = toCopy.sender_mac;
  sender_ip_in = toCopy.sender_ip_in;
  sender_netmask_in = toCopy.sender_netmask_in;
  sender_id = toCopy.sender_id;
  recipient_ip_in = toCopy.recipient_ip_in;
  recipient_netmask_in = toCopy.recipient_netmask_in;
  recipient_id = toCopy.recipient_id;
  message = toCopy.message;

}

/**
 * WifiMessageBase equals operator.
 */
WifiMessageBase& WifiMessageBase::operator=(const WifiMessageBase& toCopy)
{
  sender_mac = toCopy.sender_mac;
  sender_ip_in = toCopy.sender_ip_in;
  sender_netmask_in = toCopy.sender_netmask_in;
  sender_id = toCopy.sender_id;
  recipient_ip_in = toCopy.recipient_ip_in;
  recipient_netmask_in = toCopy.recipient_netmask_in;
  recipient_id = toCopy.recipient_id;
  message = toCopy.message;

  return *this;
}


/**
 * Set the message recipient to the broadcast IP.  This means everyone in
 * radio range should receive the message.
 */
void WifiMessageBase::SetBroadcastIp()
{
  //Basically all we've got to do is look at the sender's IP and bit-wise OR in
  //the complement of the netmask. 
  // Thus sender 192.168.0.1 with netmask 255.255.255.0 would come back as 
  // broadcast IP 192.168.0.255.
  recipient_ip_in = ( sender_ip_in | ( ~sender_netmask_in  ) );
  recipient_netmask_in = sender_netmask_in;
}//end Setboardcast



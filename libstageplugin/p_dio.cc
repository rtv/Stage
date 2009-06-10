#include "p_driver.h"


InterfaceDio::InterfaceDio(player_devaddr_t addr, StgDriver* driver, ConfigFile* cf, int section)
	:  InterfaceModel( addr, driver, cf, section, Stg::MODEL_TYPE_LIGHTINDICATOR )
{
}


InterfaceDio::~InterfaceDio()
{
}


int InterfaceDio::ProcessMessage(QueuePointer & resp_queue, player_msghdr_t* hdr, void* data)
{
	Stg::ModelLightIndicator* mod = (Stg::ModelLightIndicator*) this->mod;

	if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_DIO_CMD_VALUES, this->addr))
	{
		player_dio_cmd* cmd = (player_dio_cmd*) data;
		if(cmd->count != 1)
		{
			PRINT_ERR("Invalid light indicator command.");
		}

		mod->SetState(cmd->digout != 0);
		return 0;
	}

	return -1;
}

void InterfaceDio::Publish()
{

}

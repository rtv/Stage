#include "stage.hh"
using namespace Stg;

const double cruisespeed = 0.4; 
const double avoidspeed = 0.05; 
const double avoidturn = 0.5;
const double minfrontdistance = 1.0; // 0.6  
const bool verbose = false;
const double stopdist = 0.3;
const int avoidduration = 10;

typedef struct
{
  uint32_t id;
  ModelPosition* pos;
  ModelRanger* laser;
  ModelWifi* wifi;
  int avoidcount, randcount;
} robot_t;

typedef struct {
      Pose gpose;
          uint32_t id;
} agent_data_t;

class MyWifiMessage : public WifiMessageBase {
  //This is the stuff I want to share above and beyond what the base message shares.
  public: 
    MyWifiMessage():WifiMessageBase(){};
    ~MyWifiMessage(){ };
    Pose gpose;
    
    MyWifiMessage(const MyWifiMessage& toCopy) : WifiMessageBase(toCopy)
    {
      gpose = toCopy.gpose;
    };           ///< Copy constructor
    MyWifiMessage& operator=(const MyWifiMessage& toCopy)
    {
      WifiMessageBase::operator=(toCopy);
      gpose = toCopy.gpose; 

      return *this;
    };///< Equals operator overload

    virtual WifiMessageBase* Clone() { return new MyWifiMessage(*this); }; 

};//end MyWifiMessage


int LaserUpdate( Model* mod, robot_t* robot );
int PositionUpdate( Model* mod, robot_t* robot );
int WifiUpdate( Model* mod, robot_t* robot );

// Function to process incoming wifi messages.
void ProcessMessage( WifiMessageBase * mesg );

// Stage calls this when the model starts up
extern "C" int Init( Model* mod, CtrlArgs* args )
{
  // local arguments
  printf( "\nWander Wifi controller initialised with:\n"
			 "\tworldfile string \"%s\"\n" 
			 "\tcmdline string \"%s\"",
			 args->worldfile.c_str(),
			 args->cmdline.c_str() );

  robot_t* robot = new robot_t;
 
  robot->avoidcount = 0;
  robot->randcount = 0;
  
  robot->pos = (ModelPosition*)mod;
  robot->laser = (ModelRanger*)mod->GetChild( "laser:0" );
  robot->laser->AddCallback( Model::CB_UPDATE, (model_callback_t)LaserUpdate, robot );
  robot->wifi = (ModelWifi*)mod->GetChild("wifi:0");
  robot->wifi->AddCallback( Model::CB_UPDATE, (model_callback_t)WifiUpdate, robot);
  //robot->wifi->comm.SetArg( (void*)robot );    // set up the Rx function and argument
  //robot->wifi->comm.SetReceiveFn( ProcessData );
  robot->wifi->comm.SetReceiveMsgFn( ProcessMessage);
  robot->id = robot->wifi->GetId();
  robot->wifi->Subscribe(); // start the wifi updates
  robot->laser->Subscribe(); // starts the laser updates
  robot->pos->Subscribe(); // starts the position updates
    
  return 0; //ok
}


// inspect the laser data and decide what to do
int LaserUpdate( Model* mod, robot_t* robot )
{
  // get the data
  uint32_t sample_count=robot->laser->GetSensors()[0].sample_count;
  std::vector<meters_t> scan = robot->laser->GetRanges();
  if( ! scan.size() )
    return 0;
  
  bool obstruction = false;
  bool stop = false;

  // find the closest distance to the left and right and check if
  // there's anything in front
  double minleft = 1e6;
  double minright = 1e6;

  for (uint32_t i = 0; i < sample_count; i++)
    {

		if( verbose ) printf( "%.3f ", scan[i] );

      if( (i > (sample_count/3)) 
			 && (i < (sample_count - (sample_count/3))) 
			 && scan[i] < minfrontdistance)
		  {
			 if( verbose ) puts( "  obstruction!" );
			 obstruction = true;
		  }
		
      if( scan[i] < stopdist )
		  {
			 if( verbose ) puts( "  stopping!" );
			 stop = true;
		  }
      
      if( i > sample_count/2 )
				minleft = std::min( minleft, scan[i] );
      else      
				minright = std::min( minright, scan[i]);
    }
  
  if( verbose ) 
	 {
		puts( "" );
		printf( "minleft %.3f \n", minleft );
		printf( "minright %.3f\n ", minright );
	 }

  if( obstruction || stop || (robot->avoidcount>0) )
    {
      if( verbose ) printf( "Avoid %d\n", robot->avoidcount );
	  		
      robot->pos->SetXSpeed( stop ? 0.0 : avoidspeed );      
      
      /* once we start avoiding, select a turn direction and stick
	 with it for a few iterations */
      if( robot->avoidcount < 1 )
        {
			 if( verbose ) puts( "Avoid START" );
          robot->avoidcount = random() % avoidduration + avoidduration;
			 
			 if( minleft < minright  )
				{
				  robot->pos->SetTurnSpeed( -avoidturn );
				  if( verbose ) printf( "turning right %.2f\n", -avoidturn );
				}
			 else
				{
				  robot->pos->SetTurnSpeed( +avoidturn );
				  if( verbose ) printf( "turning left %2f\n", +avoidturn );
				}
        }
		
      robot->avoidcount--;
    }
  else
    {
      if( verbose ) puts( "Cruise" );

      robot->avoidcount = 0;
      robot->pos->SetXSpeed( cruisespeed );	  
		robot->pos->SetTurnSpeed(  0 );
    }

 //  if( robot->pos->Stalled() )
// 	 {
// 		robot->pos->SetSpeed( 0,0,0 );
// 		robot->pos->SetTurnSpeed( 0 );
// 	 }
    
  return 0;
}

int PositionUpdate( Model* mod, robot_t* robot )
{
  Pose pose = robot->pos->GetPose();

  printf( "Pose: [%.2f %.2f %.2f %.2f]\n",
	  pose.x, pose.y, pose.z, pose.a );

  return 0; // run again
}

int WifiUpdate( Model* mod, robot_t* robot )
{
  
  // We are going to share our global pose with our neighbors.
  MyWifiMessage my_mesg;
  my_mesg.gpose = robot->pos->GetGlobalPose();
  WifiMessageBase * base_ptr = & my_mesg;
  // Call yupon the comm system to share this message with everyone in range.
  robot->wifi->comm.SendBroadcastMessage( base_ptr );

  return 0;
}


void ProcessMessage( WifiMessageBase * incoming) 
{
  MyWifiMessage * my_mesg = dynamic_cast<MyWifiMessage*>(incoming);
  if ( my_mesg )
  {
    //Yay it's my own special message type!!!
    printf("Robot [%u]: Neighbor [%u] is at (%.2f %.2f) and heading (%.2f)\n",
       my_mesg->GetRecipientId(), my_mesg->GetSenderId(), my_mesg->gpose.x, my_mesg->gpose.y, my_mesg->gpose.a );
  }

  //It's our responsibility to clean up the incoming message
  delete incoming;
}//end ProcessMessageData



///////////////////////////////////////////////////////////////////////////
//
// File: model_wifi.cc
// Authors: Michael Schroeder
// Date: 10 July 2007
// Ported to Stage-3.1.0 by Rahul Balani
// Ported to Stage-3.2.0 by Tyler Gunn
// Date: 24-Sept-2009
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_wifi.cc,v $
//  $Author: rtv $
//  $Revision$
//
///////////////////////////////////////////////////////////////////////////

/**
  @ingroup model
  @defgroup model_wifi Wifi model 

The wifi model simulates a wifi device. There are currently five radio propagation models
implemented. The wifi device reports simulated link information for all corresponding 
nodes within range.
  <h2>Worldfile properties</h2>

@par Simple Wifi Model
 
@verbatim
wifi
(
	# network properties
  ip "192.168.0.1"
  mac "08-00-20-ae-fd-7e"
  essid "test network"

	# radio propagation model
  model "simple"
	range 5
)
@endverbatim
For the simple model it is only necessary to specify the range property which serves as the
radio propagation radius. 


@par Friis Outdoor Model

@verbatim
wifi
(
	# network properties
  ip "192.168.0.1"
  mac "08-00-20-ae-fd-7e"
  essid "test network"

	# radio propagation model
  model "friis"
	power 30
	sensitivity -80
)
@endverbatim
The friis model simulates the path loss for each link according to Friis Transmission Equation. 
Basically it serves as a free space model. The parameters power(dbm) and sensitivity(dbm) are mandatory.


@par ITU Indoor Propagation Model

@verbatim
wifi
(
	# network properties
  ip "192.168.0.1"
  mac "08-00-20-ae-fd-7e"
  essid "test network"

	# radio propagation model
  model "ITU indoor"
	power 30
	sensitivity -70
	plc 30
)
@endverbatim
This model estimates the path loss inside rooms or closed areas. The distance power loss
coefficient must be set according to the environmental settings. Values vary between 20 to 35.
Higher values indicate higher path loss. 


@par Log Distance Path Loss Model

@verbatim
wifi
(
	# network properties
  ip "192.168.0.1"
  mac "08-00-20-ae-fd-7e"
  essid "test network"

	# radio propagation model
  model "log distance"
	power 30
	sensitivity -70
	ple 4
	sigma 6
)
@endverbatim
The Log Distance Path Loss Model approximates the total path loss inside a building. 
It uses the path loss distance exponent (ple) which varies between 1.8 and 10 to accommodate for 
different environmental settings. A Gaussian random variable with zero mean and standard deviation 
sigma is used to reflect shadow fading.


@par Simple Raytracing Model

@verbatim
wifi
(
	# network properties
  ip "192.168.0.1"
  mac "08-00-20-ae-fd-7e"
  essid "test network"

	# radio propagation model
  model "raytrace"
	wall_factor 10
)
@endverbatim
In this model Raytracing is used to reflect wifi-blocking obstacles within range. The wall_factor
is used to specify how hard it is for the signal to penetrate obstacles. 


@par Details
- model
  - the model to use. Choose between the five existing radio propagation models
- ip
  - the ip to fake
- mac
  - the corresponding mac addess
- essid
  - networks essid
- freq
  - the operating frequency [default 2450 (MHz)]
- power
  - Transmitter output power in dbm [default 45 (dbm)]
- sensitivity
  - Receiver sensitivity in dbm [default -75 (dbm)]
- range
  - propagation radius in meters used by simple model
- link_success_rate
  - (0-100) percent of times that links between robots are successfully establiehed.
  Note: It is entirely possible for a robot to be able to hear its neighbor, but its
  neighbour is unable to hear it.
- plc
  - power loss coefficient used by the ITU Indoor model
- ple
  - power loss exponent used by the Log Distance Path Loss Model
- sigma
  - standard deviation used by Log Distance Path Loss Model
- range_db
  - parameter used for visualization of radio wave propagation. It is usually set to a negative value
to visualize a certain db range.
- wall_factor
  - factor reflects the strength of obstacles
  - random_seed <int>\n
    The seed value for the random number generator.  If not defined, a random
    seed will be used.  Speciying the seed in the world file is important
    if you want to be able to repeatedly run the exact same simulation to
    receive repeatable results.
*/

#include <assert.h>
#include <sys/time.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <iomanip>
#include "stage.hh"
#include "worldfile.hh"
#include "option.hh"
#include <config.h>
#include <ctime>
#ifdef HAVE_BOOST_RANDOM
#include <boost/random.hpp>
#include <boost/nondet_random.hpp>
#endif

using namespace Stg;

#define RANDOM_LIMIT INT_MAX
#define STG_WIFI_WATTS 	2.5 // wifi power consumption
#define STG_DEFAULT_WIFI_IP	"192.168.0.1"
#define STG_DEFAULT_WIFI_NETMASK "255.255.255.0"
#define STG_DEFAULT_WIFI_MAC "00:00:00:00:00"
#define STG_DEFAULT_WIFI_ESSID "any"
#define STG_DEFAULT_WIFI_POWER 45
#define STG_DEFAULT_WIFI_FREQ	2450
#define STG_DEFAULT_WIFI_SENSITIVITY -75
#define STG_DEFAULT_WIFI_RANGE	0
#define STG_DEFAULT_WIFI_PLC		30
#define STG_DEFAULT_WIFI_SIGMA  5 // can take values from 2 to 12 or even higher
#define STG_DEFAULT_WIFI_PATH_LOSS_EXPONENT 2.5 // 2.0 for free space, 
                                                // 1.8 in corridor, higher 
                                                // values indicate more obstacles
#define STG_DEFAULT_WIFI_RANGE_DB -50 //in dbm
#define STG_DEFAULT_WIFI_WALL_FACTOR 1
#define STG_DEFAULT_LINK_SUCCESS_RATE 100

//-----------------------WifiConfig stuff-----------------------------------

const std::string WifiConfig::MODEL_NAME_FRIIS = "friis";
const std::string WifiConfig::MODEL_NAME_ITU_INDOOR = "ITU indoor";
const std::string WifiConfig::MODEL_NAME_LOG_DISTANCE = "log distance";
const std::string WifiConfig::MODEL_NAME_RAYTRACE = "raytrace";
const std::string WifiConfig::MODEL_NAME_SIMPLE = "simple";

/**
 * Constructor for WIFI config.  We'll set everything up using some sane
 * default values.
 */
WifiConfig::WifiConfig()
{
  SetEssid(STG_DEFAULT_WIFI_ESSID);
  SetIp(STG_DEFAULT_WIFI_IP);
  SetNetmask(STG_DEFAULT_WIFI_NETMASK);
  SetPower( STG_DEFAULT_WIFI_POWER);
  SetSensitivity(STG_DEFAULT_WIFI_SENSITIVITY);
  SetRange(STG_DEFAULT_WIFI_RANGE);
  SetFreq(STG_DEFAULT_WIFI_FREQ);
  SetPlc(STG_DEFAULT_WIFI_PLC);
  SetPle(STG_DEFAULT_WIFI_PATH_LOSS_EXPONENT);
  SetSigma(STG_DEFAULT_WIFI_SIGMA);
  SetRangeDb(STG_DEFAULT_WIFI_RANGE_DB);
  SetWallFactor(STG_DEFAULT_WIFI_WALL_FACTOR);
  SetLinkSuccessRate(STG_DEFAULT_LINK_SUCCESS_RATE);
}//end constructor
 
/**
 * Set the model attribute based ona string (i.e. the ones we would expect
 * to find in the world files.
 */
void WifiConfig::SetModel(const std::string & value)
{
  if (value.compare("friis") == 0)
    model = FRIIS;
  else if (value.compare("ITU indoor") == 0)
    model = ITU_INDOOR;
  else if (value.compare("log distance") == 0)
    model = LOG_DISTANCE;
  else if (value.compare("raytrace") == 0)
    model = RAYTRACE;
  else if (value.compare("simple") == 0)
    model = SIMPLE;
}//end SetModel

const std::string & WifiConfig::GetModelString()		{
  if ( model == FRIIS )
    return MODEL_NAME_FRIIS;
  else if ( model == ITU_INDOOR )
    return MODEL_NAME_ITU_INDOOR;
  else if ( model == LOG_DISTANCE )
    return MODEL_NAME_LOG_DISTANCE;
  else if ( model == RAYTRACE )
    return MODEL_NAME_RAYTRACE;
  else if ( model == SIMPLE )
    return MODEL_NAME_SIMPLE;
  else return MODEL_NAME_SIMPLE;
}

//-----------------------Visualizer Stuff-----------------------------------

/**
 * Return list of link information.
 */
wifi_data_t* ModelWifi::GetLinkInfo( )
{
    return &link_data;
}
/**
 * Visualizer option to show the wifi links between robots
 */
Option ModelWifi::showWifi( "Wifi", "show_wifi", "", true, NULL );

/**
 * Visualizer method to display links between robots 
 */
void ModelWifi::DataVisualize( Camera* cam )
{
  if (link_data.neighbors.size() == 0) return;

  glPushMatrix();

  unsigned int s, sample_count = link_data.neighbors.size();

  glDepthMask( GL_TRUE );

  if( showWifi )
  {
    PushColor( 0, 0, 1, 0.5 );		       
    glBegin( GL_LINES );
    for( s=0; s<sample_count; s++ )
    {
      Pose loc = GlobalToLocal(link_data.neighbors[s].GetPose());
      glVertex2f( 0,0 );
      glVertex2f( loc.x, loc.y );
		}
    glEnd();
    PopColor();
  }	 
  glPopMatrix();
}




//------------------------Wifi Model----------------------------------------


/**
 * Wifi model constructor 
 */
ModelWifi::ModelWifi( World* world, Model* parent, const std::string& type )
    : Model( world, parent, type ),
    comm(this)
		{   
  PRINT_DEBUG2( "Constructing ModelWifi %d (%s)\n",
            id, type.c_str() );
  //Try to seed the RNG with /dev/random.  If the file is opened okay, then we can rely on /dev/random.
  //If it wasn't opened okay, then we'll rely on the standard fallback of using the time.  This isn't optimal in the least
  //and probably results in bad randomness on windows systems.  Oh well. :)
  FILE* f = fopen("/dev/random", "r");
  if ( f  ) 
  {
    unsigned int seed_data;
    fread(&seed_data, sizeof(unsigned int), 1, f);
    fclose(f);
    random_seed = seed_data;
  }
  else
  {
    PRINT_ERR( "NOTE: RNG couldn't seed from /dev/random as /dev/random doesn't exist.  The random numbers may not be optimal in this case.\n");
    random_seed = static_cast<unsigned int>(std::time(0));
  }
  //Get a unique MAC address for this wifi model among all other wifi models 
  //currently defined in the world.
  wifi_config.SetMac(GetUniqueMac(world));

  //Add this model to the list of wifi models in the world
  world->WifiInsert(this);

  // assert that Update() is reentrant for this derived model
  thread_safe = true;

  // no power consumed until we're subscribed
  this->SetWatts( 0 );

  // Sensible wifi defaults; it doesn't take up any physical space
			Geom geom;
  memset( &geom, 0, sizeof(geom));
  SetGeom( geom );

			// get the sensor's pose in global coords
  // Wifi is invisible
  SetObstacleReturn( 0 );
  SetRangerReturn( 0.0f );
  SetBlobReturn( 0 );
  SetColor( Color() );

#ifdef HAVE_BOOST_RANDOM
  // For communication reliability simulation we need a generator that generates from 0-100 inclusive.

  comm_reliability_gen = new BoostRandomIntGenerator( random_generator, uniform_distribution_type(0, 100));
  // For the log method we need a normal distribution of random numbers.  Boost thoughtfully provides us with one! :)
  normal_gen = new BoostNormalGenerator( random_generator, normal_distribution_type(0,1));
#endif
  RegisterOption( &showWifi );
}//end ModelWifi const

/**
 * Destructor
 */
ModelWifi::~ModelWifi( void )
{
  world->WifiErase(this);

  // It will free the elements of the array as well
  link_data.neighbors.clear();
  //g_array_free(link_data.neighbors, TRUE);

#ifdef HAVE_BOOST_RANDOM
  delete comm_reliability_gen;
  delete normal_gen;
#endif

}//end ModelWifi dest


/**
 * Get a unique Mac address for this WIFI model among all other WIFI models in the current world.
 */
std::string ModelWifi::GetUniqueMac( World* world )
{
  std::stringstream macSs;
  

  bool duplicate = false;
  std::string mac_str = "";

  //Find MACs until we're sure no other models have this address.
  do
  {
    duplicate = false;

    macSs.str("");
    macSs.clear();

#ifdef HAVE_BOOST_RANDOM
    // For mac geneartion, we need a generator that generates from 0-255 inclusive.
    BoostRandomIntGenerator mac_rand_gen(random_generator, uniform_distribution_type(0, 255));
#endif


    //Start out our unique mac with 02:00:00 as the organization OID; this specifies this MAC as a
    //locally administered MAC address versus one assigned by a manufacturer.
    //(for more info see http://en.wikipedia.org/wiki/MAC_address)
    macSs << "02:00:00";
   
    //Now we need to generate 3 random hex bytes to make up the rest of the mac.
    for (unsigned int bNum = 0; bNum < 3; bNum++ )
    {
#ifdef HAVE_BOOST_RANDOM
      int randomByte = mac_rand_gen();
#else
      int randomByte = rand() % 256;
#endif
      macSs << ":" << std::hex << std::setfill('0') << std::setw(2) << randomByte;
		}
    mac_str = macSs.str();

    //Now, we need to check if any other WIFI model in the world uses this mac.
    FOR_EACH( other_model, *(world->GetModelsWithWifi()) )
		{ 
      ModelWifi *other_model_wifi = dynamic_cast<ModelWifi*>(*other_model);

      if ( other_model_wifi == this )
        continue; 

      if (other_model_wifi->GetConfig()->GetMac().compare( mac_str ) == 0 )
        duplicate = true;
		}


  } while (duplicate);
  
  return mac_str;
}//end AssignUniqueMac

/**
 * Load a wifi model from the world file.
 */
void ModelWifi::Load( void )
		{

  std::string ip_str = wifi_config.GetIpString();
  std::string netmask_str = wifi_config.GetNetmaskString();
  const std::string & model_str = wifi_config.GetModelString();

  // read wifi parameters from worldfile
  wifi_config.SetIp           (wf->ReadString(wf_entity, "ip",          ip_str.c_str()));
  wifi_config.SetNetmask      (wf->ReadString(wf_entity, "netmask",     netmask_str.c_str()));
  wifi_config.SetMac          (wf->ReadString(wf_entity, "mac",         wifi_config.GetMac().c_str()));
  wifi_config.SetEssid        (wf->ReadString(wf_entity, "essid",       wifi_config.GetEssid().c_str()));
  wifi_config.SetPower        (wf->ReadFloat (wf_entity, "power",       wifi_config.GetPower()));
  wifi_config.SetFreq         (wf->ReadFloat (wf_entity, "freq",        wifi_config.GetFreq()));
  wifi_config.SetSensitivity  (wf->ReadFloat (wf_entity, "sensitivity", wifi_config.GetSensitivity()));
  wifi_config.SetRange        (wf->ReadLength(wf_entity, "range",       wifi_config.GetRange()));
  wifi_config.SetModel        (wf->ReadString(wf_entity, "model",       model_str.c_str()));
  wifi_config.SetPlc          (wf->ReadFloat (wf_entity, "plc",         wifi_config.GetPlc()));
  wifi_config.SetPle          (wf->ReadFloat (wf_entity, "ple",         wifi_config.GetPle()));
  wifi_config.SetSigma        (wf->ReadFloat (wf_entity, "sigma",       wifi_config.GetSigma()));
  wifi_config.SetLinkSuccessRate(wf->ReadInt (wf_entity, "link_success_rate", wifi_config.GetLinkSuccessRate()));
  wifi_config.SetRangeDb      (wf->ReadFloat (wf_entity, "range_db",    wifi_config.GetRangeDb()));
  wifi_config.SetWallFactor   (wf->ReadFloat (wf_entity, "wall_factor", wifi_config.GetWallFactor()));
  wifi_config.SetPowerDbm     (10.0*log10( wifi_config.GetPower()));

  // read the random seed to use for this run.
  random_seed=                 wf->ReadInt(wf_entity,"random_seed", random_seed );
  
  // Initialize the random number generator with the seed.
#ifdef HAVE_BOOST_RANDOM
  random_generator.seed(random_seed);
#else
  srand(random_seed);
#endif

  Model::Load();
}//end Load


// Marsaglia polar method to obtain a gaussian random variable
double nrand()
{
  //Uhh why would we re-seed EVERY single time?  That does not make sense.
  //srand(time(0)+d); // time(0) is different only every second
  double u1,u2,s;

  do 
  {
    u1= ((double) rand()/RANDOM_LIMIT)*2.0-1.0;
    u2= ((double) rand()/RANDOM_LIMIT)*2.0-1.0;
    s=u1*u1+u2*u2;
		}
  while (s>=1.0);
  return ( u1*sqrt(-2* log(s) /s) );

}//end nrand



static bool wifi_raytrace_match( Model* hit, Model* finder,
        const void* dummy )
		{
  // Ignore the model that's looking and things that are invisible to
  // lasers  
  return( (!hit->IsRelated( finder )) && 
          ( hit->vis.ranger_return > 0) );
}//end wifi_raytrace_match

meters_t calc_distance( Model* mod1, 
                            Model* mod2, 
                            Pose* pose1,
                            Pose* pose2 )
{
  Ray r;
  r.func = wifi_raytrace_match; 
  // Dummy argument for us
  r.arg = NULL;
  r.ztest = true;
  // Max range of the ray should be the actual distance between two devices
  r.range = hypot(pose2->x - pose1->x, pose2->y - pose1->y);
  // Source model/device
  r.mod = mod1;
  // Set up the ray origins in global coords
  Pose rayorg = *pose1;
  // Direction of the target device
  rayorg.a = atan2( pose2->y - pose1->y, pose2->x - pose1->x );
  r.origin = rayorg;

  return mod1->GetWorld()->RaytraceWifi(r.origin, r.range, r.func, r.mod, mod2, r.arg, r.ztest);
}//end calc_distance


void ModelWifi::AppendLinkInfo( ModelWifi* mod, double db )
{
  Pose pose2 = mod->GetGlobalPose();
  WifiConfig* neighbor_config = mod->GetConfig();

  WifiNeighbor neighbor;
  neighbor.SetId        ( mod->GetId());
  neighbor.SetPose      ( mod->GetGlobalPose());
  neighbor.SetEssid     ( neighbor_config->GetEssid());
  neighbor.SetMac       ( neighbor_config->GetMac());
  neighbor.SetIp        ( neighbor_config->GetIp());
  neighbor.SetNetmask   ( neighbor_config->GetNetmask());
  neighbor.SetDb        ( db );
  neighbor.SetFreq      ( neighbor_config->GetFreq());
  neighbor.SetCommunication( mod->GetCommunication());

  link_data.neighbors.push_back(neighbor);
}//end AppendLinkInfo

bool ModelWifi::DoesLinkSucceed()
{
  //Take into account link success rate
  //Generate a random number from 0-100.
  //If the number is > the ssuccess rate, then skip this check
  if (wifi_config.GetLinkSuccessRate() < 100)
  {
#ifdef HAVE_BOOST_RANDOM
    int random_num = (*comm_reliability_gen)();
#else
    int random_num = rand() % 101;
#endif
    return !( random_num > wifi_config.GetLinkSuccessRate() );
		}
  return true;
}
void ModelWifi::CompareModels( Model * value, Model * user )
{
  // 'user' is a wifi device
  Model *mod1 = (Model *)user;
  Model *mod2 = (Model *)value;
  
  // proceed if models are different and wifi devices
  if ( (mod1->GetModelType() != mod2->GetModelType()) || 
       (mod1 == mod2) || 
       (mod1->IsRelated(mod2)) )
  return;

  // Get global poses of both devices
  Pose pose1 = mod1->GetGlobalPose();
  Pose pose2 = mod2->GetGlobalPose();

  // mod2 is also a wifi device
  ModelWifi *mod1_w = dynamic_cast<ModelWifi*>(mod1);
  ModelWifi *mod2_w = dynamic_cast<ModelWifi*>(mod2);

  WifiConfig * cfg1 = mod1_w->GetConfig();
  WifiConfig * cfg2 = mod2_w->GetConfig();


  double distance = hypot( pose2.x-pose1.x, pose2.y-pose1.y );

  // store link information for reachable nodes
  if ( cfg1->GetModel() == FRIIS )
  {
    double fsl = 32.44177+20*log10(cfg1->GetFreq())+20*log10(distance/1000);
    if( fsl <= ( cfg1->GetPowerDbm()-cfg2->GetSensitivity() ) && DoesLinkSucceed())
    {
      mod1_w->AppendLinkInfo( mod2_w, cfg1->GetPowerDbm()-fsl );
		}
  }
  else if ( cfg1->GetModel() == ITU_INDOOR  )
  { 
    double loss = 20*log10( cfg1->GetFreq() )+( cfg1->GetPlc() )*log10( distance )-28;
    if( loss <= ( cfg1->GetPowerDbm()-cfg2->GetSensitivity() ) && DoesLinkSucceed())
    {	
      mod1_w->AppendLinkInfo( mod2_w, cfg1->GetPowerDbm()-loss );
    }
  }
  else if( cfg1->GetModel() == LOG_DISTANCE )
  {
    double norand; 
    double loss_0 = 32.44177+20*log10( cfg1->GetFreq() ) + 20*log10( 0.001 ); // path loss in 1 meter
#ifdef HAVE_BOOST_RANDOM
    norand = (*normal_gen)();
#else
    norand = nrand( );
#endif
    double loss = loss_0+10*cfg1->GetPle()*log10( distance )+cfg1->GetSigma()*norand;

    if( loss <= (cfg1->GetPowerDbm()-cfg2->GetSensitivity()) && DoesLinkSucceed())
    {
      mod1_w->AppendLinkInfo( mod2_w, cfg1->GetPowerDbm()-loss );
    }
  }
  else if( cfg1->GetModel() == RAYTRACE  )
  {
    double loss = 20*log10( cfg1->GetFreq() ) + (cfg1->GetPlc())*log10( distance )-28;

    if ( loss <= ( cfg1->GetPowerDbm() - cfg2->GetSensitivity() ) )
    {
      meters_t me = calc_distance( mod1, mod2, &pose1, &pose2 );
      double new_loss = cfg1->GetWallFactor()*me*100+loss;

      if ( new_loss <=( cfg1->GetPowerDbm()-cfg2->GetSensitivity())  &&DoesLinkSucceed())
      {
        mod1_w->AppendLinkInfo( mod2_w, cfg1->GetPowerDbm()-new_loss );
      }			
    }
  }
  else if( cfg1->GetModel() == SIMPLE  )
  {
    if (cfg1->GetRange() >= distance && DoesLinkSucceed())
    {
      mod1_w->AppendLinkInfo( mod2_w, 1 );
    }
  }
}//end compare_models


void ModelWifi::Update( )
{   
  // no work to do if we're unsubscribed
  if( subs < 1 )
    return;

  // Discard old data
  link_data.neighbors.clear();

  //For all models out there with WIFI, compare to this model.
  ModelPtrVec* models_with_wifi = world->GetModelsWithWifi();

  for (unsigned int ix=0; ix < models_with_wifi->size(); ix ++)
  {
    CompareModels((*models_with_wifi)[ix], this);
  }
  /*
  FOR_EACH( other_model, *(world->GetModelsWithWifi()) )
  {
    CompareModels(*other_model, this);
  }*/
  Model::Update();
}//end Update


void ModelWifi::Startup(  void )
{ 
  Model::Startup();

  PRINT_DEBUG( "wifi startup" );
  // start consuming power
  SetWatts( STG_WIFI_WATTS );
}

void ModelWifi::Shutdown( void )
{ 
  PRINT_DEBUG( "wifi shutdown" );
  SetWatts( 0 );   // stop consuming power
  //stg_model_fig_clear( mod, "wifi_cfg_fig" );
  Model::Shutdown();
}





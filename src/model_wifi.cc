///////////////////////////////////////////////////////////////////////////
//
// File: model_wifi.cc
// Authors: Michael Schroeder
// Date: 10 July 2007
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_wifi.cc,v $
//  $Author: rtv $
//  $Revision: 1.2 $
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
The friis model simulates the path loss for each link according to Friis Transmission Equation. Basically it serves as a free space model. The parameters power(dbm) and sensitivity(dbm) are mandatory.


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
It uses the path loss distance exponent (ple) which varies between 1.8 and 10 to accommodate for different environmental settings. A Gaussian random variable with zero mean and standard deviation sigma
is used to reflect shadow fading.


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
*/

#include <assert.h>
#include <sys/time.h>
#include <math.h>
#include "gui.h"
#include <limits.h>
#include "stage_internal.h"


#define RANDOM_LIMIT INT_MAX
#define STG_WIFI_WATTS 	2.5 // wifi power consumption
#define STG_DEFAULT_WIFI_IP	"192.168.0.1"
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


// standard callbacks
extern "C" {
int wifi_update( stg_model_t* mod );
int wifi_startup( stg_model_t* mod );
int wifi_shutdown( stg_model_t* mod );
void wifi_load( stg_model_t* mod );

int wifi_render_cfg( stg_model_t* mod, void* userp );
int wifi_unrender_cfg( stg_model_t* mod, void* userp );
int wifi_render_data( stg_model_t* mod, void* userp );
int wifi_unrender_data( stg_model_t* mod, void* userp );

static stg_color_t cfg_color = 0;
static stg_color_t cfg_color_green= 0;
// variable to save outcome of random variable
static double norand; 
static double power_dbm;

void wifi_load( stg_model_t* mod )
{
  stg_wifi_config_t* cfg = (stg_wifi_config_t*) mod->cfg;

  // read wifi parameters from worldfile
	strncpy(cfg->ip, wf_read_string(mod->id, "ip", cfg->ip), 32 );
	strncpy(cfg->mac, wf_read_string(mod->id, "mac", cfg->mac), 32 );
	strncpy(cfg->essid, wf_read_string(mod->id, "essid", cfg->essid), 32 );
  cfg->power		= wf_read_float( mod->id, "power", cfg->power);
	cfg->freq			= wf_read_float( mod->id, "freq", cfg->freq);  
	cfg->sensitivity 	= wf_read_float( mod->id, "sensitivity", cfg->sensitivity);
  cfg->range		= wf_read_length( mod->id, "range", cfg->range);
	strncpy(cfg->model, wf_read_string(mod->id, "model", cfg->model), 32);
	cfg->plc			= wf_read_float( mod->id, "plc", cfg->plc);
	cfg->ple			= wf_read_float( mod->id, "ple", cfg->ple);
	cfg->sigma		= wf_read_float( mod->id, "sigma", cfg->sigma);
	// specifiy db-config ranges for rendering
	cfg->range_db		= wf_read_float( mod->id, "range_db", cfg->range);
	cfg->wall_factor= wf_read_float( mod->id, "wall_factor", cfg->wall_factor);
	power_dbm = 10*log10( cfg->power );

  model_change( mod, &mod->cfg );
}

// Marsaglia polar method to obtain a gaussian random variable
double nrand(double d) // additional parameter because of update cycle
{
	srand(time(0)+d); // time(0) is different only every second
	double u1,u2,s;

	do 
		{
			u1= ((double) rand()/RANDOM_LIMIT)*2.0-1.0;
			u2= ((double) rand()/RANDOM_LIMIT)*2.0-1.0;
			s=u1*u1+u2*u2;
		} 
	while (s>=1.0);

	return ( u1*sqrt(-2* log(s) /s) );
}


int wifi_init( stg_model_t* mod )
{
  // we don't consume any power until subscribed
  mod->watts = 0.0; 
  
  // override the default methods; these will be called by the simualtion
  // engine
  mod->f_startup = wifi_startup;
  mod->f_shutdown = wifi_shutdown;
  mod->f_update =  wifi_update;
  mod->f_load = wifi_load;

  // sensible wifi defaults; it doesn't take up any physical space
  stg_geom_t geom;
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = 0.0;
  geom.size.y = 0.0;
  stg_model_set_geom( mod, &geom );
  
  // wifi is invisible
  stg_model_set_obstacle_return( mod, 0 );
  stg_model_set_laser_return( mod, LaserTransparent );
  stg_model_set_blob_return( mod, 0 );
  stg_model_set_color( mod, (stg_color_t)0 );
  cfg_color = stg_lookup_color(STG_WIFI_CFG_COLOR);	
	cfg_color_green= stg_lookup_color(STG_WIFI_CFG_COLOR_GREEN);
	

  // set up a wifi-specific config structure
  stg_wifi_config_t lconf;
  memset(&lconf,0,sizeof(lconf));
	
	strncpy(lconf.ip, STG_DEFAULT_WIFI_IP, 32);
	strncpy(lconf.mac, STG_DEFAULT_WIFI_MAC, 32);
	strncpy(lconf.essid, STG_DEFAULT_WIFI_ESSID, 32);
  lconf.power	    	= STG_DEFAULT_WIFI_POWER;
  lconf.sensitivity = STG_DEFAULT_WIFI_SENSITIVITY; 
  lconf.range	    	= STG_DEFAULT_WIFI_RANGE;
	lconf.freq				= STG_DEFAULT_WIFI_FREQ;
	lconf.plc					= STG_DEFAULT_WIFI_PLC;
	lconf.ple					= STG_DEFAULT_WIFI_PATH_LOSS_EXPONENT;
	lconf.sigma				= STG_DEFAULT_WIFI_SIGMA;
	lconf.range_db		= STG_DEFAULT_WIFI_RANGE_DB;
	lconf.wall_factor = STG_DEFAULT_WIFI_WALL_FACTOR;
  stg_model_set_cfg( mod, &lconf, sizeof(lconf) );

  //create array to hold the link data
  stg_wifi_data_t data;
  data.neighbours = g_array_new( FALSE, FALSE, sizeof( stg_wifi_sample_t ) );
  stg_model_set_data( mod, &data, sizeof( data ) );

  // adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, &mod->data,
				  wifi_render_cfg, NULL,
				  wifi_unrender_cfg, NULL,
				  "wifi_cfg",
				  "wifi range",
				  TRUE );
	
	// adds a menu item and associated on-and-off callbacks
  stg_model_add_property_toggles( mod, &mod->data,
				  wifi_render_data, NULL,
				  wifi_unrender_data, NULL,
				  "wifi_data",
				  "wifi connections",
				  TRUE );

  return 0;
}


int wifi_raytrace_match( stg_model_t* mod, stg_model_t* hitmod )
{           
  // ignore my relatives and things that are invisible to lasers
  return( (!stg_model_is_related( mod,hitmod )) && 
	  		( hitmod->laser_return > 0) );
}


stg_meters_t calc_distance( stg_model_t* mod1, 
														stg_model_t* mod2, 
														stg_pose_t* pose1,
														stg_pose_t* pose2 )
{
	itl_t* itl = NULL;
	itl = itl_create( pose1->x, pose1->y, pose2->x, pose2->y, 
                    mod1->world->matrix, PointToPoint );

	return itl_wall_distance( itl, wifi_raytrace_match, mod1, mod2 );
}


void append_link_information( stg_model_t* mod1, stg_model_t* mod2, double db )
{
	stg_wifi_config_t* cfg2 = (stg_wifi_config_t*) mod2->cfg;
	stg_pose_t pose2;
  stg_model_get_global_pose( mod2, &pose2 );
	
	stg_wifi_sample_t sample;
	memset( &sample,0,sizeof( sample ) );
	strcpy( sample.ip, cfg2->ip );
	strcpy( sample.mac, cfg2->mac );
	strcpy( sample.essid, cfg2->essid) ;
	sample.db  = db;
	sample.pose = pose2;
	sample.freq = cfg2->freq;
	
	stg_wifi_data_t* data = (stg_wifi_data_t*) mod1->data;
	g_array_append_val( data->neighbours, sample );
}
							
void compare_models( gpointer key, gpointer value, gpointer user )
{
  stg_model_t *mod1 = (stg_model_t *) user;
  stg_model_t *mod2 = (stg_model_t *) value;

	// proceed if models are different and wifi devices
  if ( (mod1->typerec != mod2->typerec) || 
			 (mod1 ==mod2) || 
			 (stg_model_is_related(mod1,mod2)) )
   return;

  stg_pose_t pose1;
  stg_model_get_global_pose( mod1, &pose1 );
  stg_pose_t pose2;
  stg_model_get_global_pose( mod2, &pose2 );

	stg_wifi_config_t* cfg1 = (stg_wifi_config_t*) mod1->cfg;
  stg_wifi_config_t* cfg2 = (stg_wifi_config_t*) mod2->cfg;

	double distance = hypot( pose2.x-pose1.x, pose2.y-pose1.y );
	
	// store link information for reachable nodes
	if ( strcmp( cfg1->model, "friis" )== 0 )
	 {
			double fsl			= 32.44177+20*log10(cfg1->freq)+20*log10(distance/1000);
			if( fsl<= ( power_dbm-cfg2->sensitivity ) )
				{
	  			append_link_information( mod1, mod2, power_dbm-fsl );
				}
		}

	else if ( strcmp( cfg1->model, "ITU indoor" ) ==0 )
		{
			double loss = 20*log10( cfg1->freq )+( cfg1->plc )*log10( distance )-28;
			if( loss<=( power_dbm-cfg2->sensitivity ) )
				{	
					append_link_information( mod1, mod2, power_dbm-loss );
				}
	}

	else if(strcmp(cfg1->model, "log distance" )==0 )
	{
		double loss_0		= 32.44177+20*log10( cfg1->freq )+
											20*log10( 0.001 ); // path loss in 1 meter
		norand = nrand( power_dbm );
		double loss = loss_0+10*cfg1->ple*log10( distance )+cfg1->sigma*norand;

		if( loss<= (power_dbm-cfg2->sensitivity) )
			{
			append_link_information(mod1, mod2, power_dbm-loss);
			}
	}

	else if( strcmp( cfg1->model, "raytrace" )==0 )
	{
		double loss = 20*log10( cfg1->freq )+
									(cfg1->plc)*log10( distance )-28;

		if( loss<= ( power_dbm-cfg2->sensitivity ) )
		{
			stg_meters_t me = calc_distance( mod1, mod2, &pose1, &pose2 );
			double new_loss = cfg1->wall_factor*me*100+loss;
			if ( new_loss <=( power_dbm-cfg2->sensitivity ) )
			{
				append_link_information( mod1, mod2, power_dbm-new_loss );	
			}			
		}
	}

	else if( strcmp( cfg1->model, "simple" )==0 )
	{
		if (cfg1->range >= distance)
		{
			append_link_information( mod1, mod2, 1 );
		}
	}
}


int wifi_update( stg_model_t* mod )
{   
  // no work to do if we're unsubscribed
  if( mod->subs < 1 )
    return 0;

  //puts("\nwifi_update");
    
  // Retrieve current configuration
  stg_wifi_config_t cfg;
  memcpy( &cfg, mod->cfg, sizeof(cfg) );

  // Retrieve current geometry
  stg_geom_t geom;
  stg_model_get_geom( mod, &geom );

  // We'll store current data here
  stg_wifi_data_t data;
  memcpy( &data, mod->data, sizeof(data) );

	g_array_free( data.neighbours, TRUE );
 
	data.neighbours = g_array_new( FALSE, TRUE, sizeof( stg_wifi_sample_t ) );
  stg_model_set_data( mod, &data, sizeof( data ) );

  g_hash_table_foreach(mod->world->models_by_name, compare_models, mod); 
	stg_model_set_data( mod, &data, sizeof( data ) );
	
  return 0; //ok
}



int wifi_render_cfg(  stg_model_t* mod, void* userp )
{
  
	//puts( "\nwifi render cfg" );
	stg_rtk_fig_t* fig = 
  stg_model_fig_get_or_create( mod, "wifi_cfg_fig", "top", 
			 												 STG_LAYER_WIFICONFIG );
  stg_rtk_fig_t* bg = 
  stg_model_fig_get_or_create( mod, "wifi_cfg_bg_fig", "top", 
			 												 STG_LAYER_BACKGROUND );
  
  stg_rtk_fig_clear( bg );
  stg_rtk_fig_clear( fig );
		
  // get config
  stg_wifi_config_t *cfg = (stg_wifi_config_t*) mod->cfg;
  assert( cfg ); // make sure config is available

	// get pose
	stg_pose_t pose;
  stg_model_get_global_pose( mod, &pose );

	// render corresponding config ranges
	if( strcmp( cfg->model, "simple" )==0 )
		{
  		stg_rtk_fig_ellipse( fig, 0, 0, 0, 2*cfg->range, 2*cfg->range, 0 );
		}

	double dist_green=0;// dist_red=0, dist_yellow=0 ;

	if ( strcmp( cfg->model, "friis" )==0 )
		{
			dist_green	= pow( 10, ((-cfg->range_db+power_dbm-
															 32.44177-20*log10(cfg->freq))/20) )*1000;
		}

	if ( strcmp( cfg->model, "ITU indoor" )==0 )
		{
			dist_green	= pow( 10, (-cfg->range_db+power_dbm+28-
															 20*log10(cfg->freq))/cfg->plc );
		}

	if( strcmp( cfg->model, "log distance")==0 )
		{
			double loss_0	= 32.44177+20*log10(cfg->freq)+20*log10(0.001);
			dist_green		= pow( 10, (-cfg->range_db+power_dbm-loss_0-
																 cfg->sigma*norand)/(10*cfg->ple) );
		}

	if ( strcmp( cfg->model, "raytrace")==0 )
		{
			double points[72][2];
			stg_pose_t frompose; 
			stg_model_get_global_pose(mod, &frompose);  
			double bearing = 0; 
			double sample_incr = (2.0 * M_PI) / 72.0;
			double me = 0;
			double actualrange = 0;
			double dist_red = pow( 10, (-cfg->range_db+power_dbm+
														 28-20*log10(cfg->freq))/cfg->plc );
			itl_t* itl = NULL;

		for ( int s = 0; s < 72; s++ )
			{ 
				// calculate the maximum range!          
				itl = itl_create( frompose.x, frompose.y, bearing, dist_red,
						              mod->world->matrix, PointToBearingRange );
				// check the fieldstrength on first obstacle 
				// and in the end and just interpolate 
				itl_first_matching(itl, wifi_raytrace_match,mod);

				double x = itl->x;
				double y = itl->y;
				double dist_to_obst = hypot(pose.x-x,pose.y-y);
	
				// the ray hits an obstacle
				if ( dist_to_obst>0 )
					{
						double loss_to_obst = 20*log10(cfg->freq)+
																	(cfg->plc)*log10(dist_to_obst)-28;
						double loss_to_targ = 20*log10(cfg->freq)+
																	(cfg->plc)*log10(dist_red)-28;		
	
						// calculate distance through wall
						me = itl_wall_distance(itl, wifi_raytrace_match, mod, mod);
						itl_destroy( itl );
						double dis = (dist_red-dist_to_obst)*
												 (power_dbm - cfg->range_db-loss_to_obst) /
												 ((cfg->wall_factor*me*100+loss_to_targ)-loss_to_obst);
				
						actualrange = dist_to_obst+dis;  
					} 
				else 
					{
						actualrange= dist_red;
					}

				double x3 =actualrange * cos(bearing);
				double y4 =actualrange * sin(bearing);
			
				// are we inside the world?
				if ( -mod->world->width/2  <=  (x3+frompose.x)  && 
						 (x3+frompose.x)  <=  mod->world->width/2   && 
						 -mod->world->height/2  <=  (y4+frompose.y) && 
						 (y4+frompose.y) <=  mod->world->height/2 )
					{
						points[s][0]=x3*cos(-frompose.a)- sin(-frompose.a)*y4;
						points[s][1]=sin(-frompose.a)*x3 +y4*cos(-frompose.a);
					} // we are outside! correct to border!
				else
					{
						double ac = hypot(frompose.x-x,frompose.y-y);
						points[s][0]= ac*cos(bearing)*cos(-frompose.a)-
													sin(-frompose.a)*ac*sin(bearing);
						points[s][1]= sin(-frompose.a)*ac*cos(bearing)+
													ac*sin(bearing)*cos(-frompose.a);
					}

				bearing += sample_incr;
			}
			// print polygons
			stg_rtk_fig_color_rgb32( bg, 0xc1ffc1);
			stg_rtk_fig_color_rgb32( fig, 0x2e8b57);
  		stg_rtk_fig_polygon( bg, 0,0,0, 72, points, TRUE );
			stg_rtk_fig_polygon( fig, 0, 0, 0, 72, points, 0 );
			return 0;
		}
	// print ellipses
	stg_rtk_fig_color_rgb32( bg, 0xc1ffc1);
	stg_rtk_fig_color_rgb32( fig, 0x2e8b57);
	stg_rtk_fig_ellipse( fig, 0, 0, 0, 2*dist_green, 2*dist_green, 0);
	stg_rtk_fig_ellipse( bg, 0, 0, 0, 2*dist_green, 2*dist_green, 1);

  return 0; 
}

// draw some fancy arrows to indicate connections
void draw( stg_pose_t pose1, stg_pose_t pose2, stg_rtk_fig_t *fig )
{
	double length = sqrt(	pow( pose2.x-pose1.x, 2 ) + pow( pose2.y-pose1.y, 2 ) );
	double neworient;

	if ( pose1.x > pose2.x && pose1.y >pose2.y)
	{
		neworient = ( 270*3.14152/180 )-asin( ( pose1.x-pose2.x )/length );
		stg_rtk_fig_arrow_fancy( fig, 0, 0, neworient-pose1.a,
														 length-0.2, 0.15, 0.01, 1 );
	} 
	else if( pose1.x>pose2.x && pose1.y <= pose2.y)
	{
		neworient = ( 90*3.14152/180 )+asin( ( pose1.x-pose2.x )/length );
		stg_rtk_fig_arrow_fancy( fig, 0, 0, neworient-pose1.a,
														 length-0.2, 0.15, 0.01, 1 );
	} 
	else if( pose1.x<=pose2.x && pose1.y <= pose2.y )
	{
		neworient = -( 270*M_PI/180 )+asin( ( pose1.x-pose2.x )/length );		
		stg_rtk_fig_arrow_fancy( fig, 0, 0, neworient-pose1.a,
														 length-0.2, 0.15, 0.01, 1 );
	} 
	else if( pose1.x<=pose2.x && pose1.y > pose2.y )
	{
		neworient = -( 90*3.14152/180 )-asin( ( pose1.x-pose2.x )/length );
		stg_rtk_fig_arrow_fancy( fig, 0, 0, neworient-pose1.a,
														 length-0.2, 0.15, 0.01, 1 );
	}
}


int wifi_render_data( stg_model_t* mod, void* userp )
{
  if( mod->subs < 1 )
    return 0;

	//puts("\n----------------\nrender_data");

	stg_rtk_fig_t* fig = stg_model_get_fig( mod, "wifi_data_fig" );
  
  if( !fig )
    fig = stg_model_fig_create( mod, "wifi_data_fig", "top", STG_LAYER_WIFIDATA );
  
  // clear figure and get color for config visualisation
  stg_rtk_fig_clear(fig);
  stg_rtk_fig_color_rgb32( fig, cfg_color);

	stg_wifi_data_t* sdata = (stg_wifi_data_t*)mod->data;
	assert(sdata);

	stg_pose_t pose1;
  stg_model_get_global_pose( mod, &pose1 );
	
	// only draw connections between linked nodes
	for ( int j=0; j<sdata->neighbours->len; j++ )
		{
			stg_wifi_sample_t sam_temp = g_array_index( sdata->neighbours, 
																									stg_wifi_sample_t, j );
			stg_pose_t pose2 = sam_temp.pose;
			draw(pose1, pose2, fig);
		}
  return 0; 
}


int wifi_startup( stg_model_t* mod )
{ 
  PRINT_DEBUG( "wifi startup" );
  stg_model_set_watts( mod, STG_WIFI_WATTS );

  return 0; // ok
}


int wifi_shutdown( stg_model_t* mod )
{
  PRINT_DEBUG( "wifi shutdown" );
  stg_model_set_watts( mod, 0 );
  
  stg_model_fig_clear( mod, "wifi_cfg_fig" );
  return 0; // ok
}


int wifi_unrender_cfg( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "wifi_cfg_fig" );
	stg_model_fig_clear( mod, "wifi_cfg_bg_fig" );
  return 1; // callback just runs one time
}


int wifi_unrender_data( stg_model_t* mod, void* userp )
{
  stg_model_fig_clear( mod, "wifi_data_fig" );
  return 1; // callback just runs one time
}

}

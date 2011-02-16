/**
  \defgroup stage The Stage standalone robot simulator.

  Here is where I describe the Stage standalone program.
 */

#include <getopt.h>

extern "C" {
#include <avon.h>
}

#include "stage.hh"
#include "config.h"

const char* AVONSTAGE_VERSION = "1.0.0";

const char* USAGE = 
  "USAGE:  stage [options] <worldfile1> [worldfile2 ... worldfileN]\n"
  "Available [options] are:\n"
  "  --clock        : print simulation time peridically on standard output\n"
  "  -c             : equivalent to --clock\n"
  "  --gui          : run without a GUI\n"
  "  -g             : equivalent to --gui\n"
  "  --help         : print this message\n"
  "  --args \"str\" : define an argument string to be passed to all controllers\n"
  "  -a \"str\"     : equivalent to --args \"str\"\n"
  "  --host \"str\" : set the http server host name (default: \"localhost\")\n"
  " --port num      : set the http sever port number (default: 8000)\n" 
  " --verbose       : provide lots of informative output\n"
  " -v              : equivalent to --verbose\n"
  "  -?             : equivalent to --help\n";

/* options descriptor */
static struct option longopts[] = {
	{ "gui",  optional_argument,   NULL,  'g' },
	{ "clock",  optional_argument,   NULL,  'c' },
	{ "help",  optional_argument,   NULL,  'h' },
	{ "args",  required_argument,   NULL,  'a' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "port",  required_argument,   NULL,  'p' },
	{ "host",  required_argument,   NULL,  'h' },
	{ "rootdir",  required_argument,   NULL,  'r' },
	{ NULL, 0, NULL, 0 }
};



uint64_t GetTimeWorld( Stg::World* world )
{
  Stg::usec_t stgtime = world->SimTimeNow();  
  return static_cast<uint64_t>(stgtime);
}

uint64_t GetTime( Stg::Model* mod )
{
  assert(mod);	
  return GetTimeWorld( mod->GetWorld() );
}

int GetModelPVA( Stg::Model* mod, av_pva_t* pva  )
{
  assert(mod);
  assert(pva);

  bzero(pva,sizeof(av_pva_t));
  
  pva->time = GetTime(mod);  
  
  Stg::Pose sp = mod->GetPose(); 

  pva->p[0] = sp.x;
  pva->p[1] = sp.y;
  pva->p[2] = sp.z;
  pva->p[3] = 0;
  pva->p[4] = 0;
  pva->p[5] = sp.a;
  
  Stg::Velocity sv = mod->GetVelocity(); 		
  
  pva->v[0] = sv.x;
  pva->v[1] = sv.y;
  pva->v[2] = sv.z;
  pva->v[3] = 0;
  pva->v[4] = 0;
  pva->v[5] = sv.a;
  
  return 0; // ok
}


int SetModelPVA(  Stg::Model* mod, av_pva_t* p  )
{
  assert(mod);
  assert(p);
  
  mod->SetPose( Stg::Pose( p->p[0], // x
									p->p[1], // y
									p->p[2], // z
									p->p[5] )); // a
  
  mod->SetVelocity( Stg::Velocity( p->v[0], // x
											  p->v[1], // y
											  p->v[2], // z
											  p->v[5] )); // a  
  return 0; // ok
}


int SetModelGeom(  Stg::Model* mod, av_geom_t* g )
{	
  assert(mod);
  assert(g);
  
  mod->SetGeom( Stg::Geom( Stg::Pose(g->pose[0], 
																		 g->pose[1], 
																		 g->pose[2],
																		 g->pose[5] ),
													 Stg::Size(g->extent[0], 
																		 g->extent[1], 
																		 g->extent[2] )));
	
  // force GUI update to see the change if Stage was paused
  mod->Redraw();
  
  return 0; // ok
}

int GetModelGeom(  Stg::Model* mod, av_geom_t* g )
{	
  assert(mod);
  assert(g);
  
  bzero(g, sizeof(av_geom_t));

  g->time = GetTime(mod);  
  
  Stg::Geom ext(mod->GetGeom());    	
  g->pose[0] = ext.pose.x;
  g->pose[1] = ext.pose.y;
  g->pose[2] = ext.pose.a;
  g->extent[0] = ext.size.x;
  g->extent[1] = ext.size.y;
  g->extent[2] = ext.size.z;

  return 0; // ok
}


int RangerData( Stg::Model* mod, av_msg_t* data )
{
  assert(mod);
  assert(data);
  
	if( ! mod->HasSubscribers() )
		mod->Subscribe();

  /* Will set the data pointer in arg data to point to this static
	  struct, thus avoiding dynamic memory allocation. This is deeply
	  non-reentrant! but fast and simple code. */
  static av_ranger_data_t rd;
  bzero(&rd, sizeof(rd));  
  bzero(data, sizeof(av_msg_t));
  
  data->time = GetTime(mod);  
  data->interface = AV_INTERFACE_RANGER;
  data->data = (const void*)&rd;  
  
  Stg::ModelRanger* r = dynamic_cast<Stg::ModelRanger*>(mod);
  
  const std::vector<Stg::ModelRanger::Sensor>& sensors = r->GetSensors();
  
  rd.transducer_count = sensors.size();
  
  assert( rd.transducer_count <= AV_RANGER_TRANSDUCERS_MAX );
  
  rd.time = data->time;

	  
  return 0; //ok
}


int RangerSet( Stg::Model* mod, av_msg_t* data )
{
  assert(mod);
  assert(data);  
  puts( "ranger set does nothing" );  
  return 0; //ok
}

int RangerGet( Stg::Model* mod, av_msg_t* data )
{
  assert(mod);
  assert(data);  

  static av_ranger_t rgr;
  bzero(&rgr,sizeof(rgr));

  data->time = GetTime(mod);  
  data->interface = AV_INTERFACE_RANGER;
  data->data = (const void*)&rgr;  
 
  rgr.time = data->time;
  
  Stg::ModelRanger* r = dynamic_cast<Stg::ModelRanger*>(mod);
  
  const std::vector<Stg::ModelRanger::Sensor>& sensors = r->GetSensors();
  
  rgr.transducer_count = sensors.size();
  
  assert( rgr.transducer_count <= AV_RANGER_TRANSDUCERS_MAX );
  

  for( unsigned int c=0; c<rgr.transducer_count; c++ )
	 {		
		// bearing
		rgr.transducers[c].fov[0].min = -sensors[c].fov/2.0;
		rgr.transducers[c].fov[0].max =  sensors[c].fov/2.0;
		
		//azimuth
		rgr.transducers[c].fov[1].min = 0.0;
		rgr.transducers[c].fov[1].max = 0.0;
		
		// range
		rgr.transducers[c].fov[2].min = sensors[c].range.min;
		rgr.transducers[c].fov[2].min = sensors[c].range.max;
		
		// pose 6dof
		rgr.transducers[c].geom.pose[0] = sensors[c].pose.x;
		rgr.transducers[c].geom.pose[1] = sensors[c].pose.y;
		rgr.transducers[c].geom.pose[2] = sensors[c].pose.z;
		rgr.transducers[c].geom.pose[3] = 0.0;
		rgr.transducers[c].geom.pose[4] = 0.0;
		rgr.transducers[c].geom.pose[5] = sensors[c].pose.a;

		// extent 3dof
		rgr.transducers[c].geom.extent[0] = sensors[c].size.x;
		rgr.transducers[c].geom.extent[1] = sensors[c].size.y;
		rgr.transducers[c].geom.extent[2] = sensors[c].size.z;

		
		av_ranger_transducer_data_t& t =	rgr.transducers[c];
		const Stg::ModelRanger::Sensor& s = sensors[c];
		
		t.pose[0] = s.pose.x;
		t.pose[1] = s.pose.y;
		t.pose[2] = s.pose.z;
		t.pose[3] = 0.0;
		t.pose[4] = 0.0;
		t.pose[5] = s.pose.a;		
		
		const std::vector<Stg::meters_t>& ranges = s.ranges;
		const std::vector<double>& intensities = s.intensities;
		
		assert( ranges.size() == intensities.size() );
		
		t.sample_count = ranges.size(); 
		
		const double fov_max = s.fov / 2.0;
		const double fov_min = -fov_max;
		const double delta = (fov_max - fov_min) / (double)t.sample_count;
		
		for( unsigned int r=0; r<t.sample_count; r++ )
		  {
			 t.samples[r][AV_SAMPLE_BEARING] = fov_min + r*delta;
			 t.samples[r][AV_SAMPLE_AZIMUTH] = 0.0; // linear scanner
			 t.samples[r][AV_SAMPLE_RANGE] = ranges[r];
			 t.samples[r][AV_SAMPLE_INTENSITY] = intensities[r];
		  }
	 }
  return 0; //ok
}


int FiducialData( Stg::Model* mod, av_msg_t* data )
{
  assert(mod);
  assert(data);
  
	if( ! mod->HasSubscribers() )
		mod->Subscribe();

  /* Will set the data pointer in arg data to point to this static
	  struct, thus avoiding dynamic memory allocation. This is deeply
	  non-reentrant! but fast and simple code. */
  static av_fiducial_data_t fd;
  bzero(&fd, sizeof(fd));  
  bzero(data, sizeof(av_msg_t));
  
  data->time = GetTime(mod);  
  data->interface = AV_INTERFACE_FIDUCIAL;
  data->data = (const void*)&fd;  
  
  Stg::ModelFiducial* fm = dynamic_cast<Stg::ModelFiducial*>(mod);

	const std::vector<Stg::ModelFiducial::Fiducial>& sf = fm->GetFiducials();

	fd.fiducial_count = sf.size();
	
	printf( "FIDUCIALS %u\n", fd.fiducial_count );

	for( size_t i=0; i<fd.fiducial_count; i++ )
		{
			fd.fiducials[i].pose[0] = sf[i].bearing;
			fd.fiducials[i].pose[1] = 0.0; // no azimuth in Stage
			fd.fiducials[i].pose[2] = sf[i].range;
			
			fd.fiducials[i].geom.pose[0] = 0.0; // sf[i].pose_rel.x;
			fd.fiducials[i].geom.pose[1] = 0.0; // sf[i].pose_rel.y;
			fd.fiducials[i].geom.pose[2] = 0.0;
			fd.fiducials[i].geom.pose[3] = 0.0; 
			fd.fiducials[i].geom.pose[4] = 0.0; 
			fd.fiducials[i].geom.pose[5] = sf[i].geom.a;
			
			fd.fiducials[i].geom.extent[0] = sf[i].geom.x;
			fd.fiducials[i].geom.extent[1] = sf[i].geom.y;
			fd.fiducials[i].geom.extent[2] = sf[i].geom.z;
		}

	return 0;
}


int FiducialSet( Stg::Model* mod, av_msg_t* data )
{
  assert(mod);
  assert(data);  
  puts( "fiducial setcfg does nothing" );  
  return 0; //ok
}


int FiducialGet( Stg::Model* mod, av_msg_t* msg )
{
	assert(mod);
	assert(msg);
	
	static av_fiducial_cfg_t cfg;
	bzero(&cfg,sizeof(cfg));
	
  msg->time = GetTime(mod);  
  msg->interface = AV_INTERFACE_FIDUCIAL;
  msg->data = (const void*)&cfg;  

	//	cfg.time = msg->time;
	
	Stg::ModelFiducial* fid = dynamic_cast<Stg::ModelFiducial*>(mod);
  
	// bearing
	cfg.fov[0].min = -fid->fov/2.0;
	cfg.fov[0].max = fid->fov/2.0;

	// azimuth
	cfg.fov[1].min = 0;
	cfg.fov[1].max = 0;
	
	// range
	cfg.fov[2].min = fid->min_range;
	cfg.fov[2].max = fid->max_range_anon;

	return 0; // ok
}


class Reg
{
public:
	std::string prototype;
	av_interface_t interface;
	av_prop_get_t getter;
	av_prop_set_t setter;
	
	Reg( const std::string& prototype, 
			 av_interface_t interface,
			 av_prop_get_t getter,
			 av_prop_set_t setter ) :
		prototype(prototype), 
		interface(interface),
		getter(getter),
		setter(setter)
	{}
};

std::pair<std::string,av_interface_t> proto_iface_pair_t;

int RegisterModel( Stg::Model* mod, void* dummy )
{ 	
	// expensive to test this here! XX todo optmize this for large pops
	if( mod->TokenStr() == "_ground_model" )
		return 0;
	
	static std::map<std::string,Reg> type_table;
	
	if( type_table.size() == 0 ) // first call only
		{
			type_table[ "model" ] = 
				Reg( "generic", AV_INTERFACE_GENERIC,NULL,NULL );
			
			type_table[ "position" ] = 
				Reg( "position2d", AV_INTERFACE_GENERIC,NULL,NULL );
			
			type_table[ "ranger" ] = 
				Reg( "ranger", AV_INTERFACE_RANGER, RangerGet, RangerSet );
			
			type_table[ "fiducial" ] = 
				Reg( "fiducial", AV_INTERFACE_FIDUCIA, FiducualGet, FiducualSet );
		}
	
  printf( "[AvonStage] registering %s\n", mod->Token() );
		
	// look up the model type in the table

	const std::map<std::string,proto_iface_pair_t>::iterator it = 
		type_table.find( mod->GetModelType() );

	if( it != type_table.end() ) // if we found it in the table
		{			
			Stg::Model* parent = mod->Parent();
			const char* parent_name = parent ? parent->Token() : NULL;
			
			av_register_object( mod->Token(),
													parent_name,  
													it->prototype.c_str(), 
													it->interface, 
													it->getter,
													it->setter,
													dynamic_cast<void*>(mod) );
			return 0; // ok
		}
	
	// else
	printf( "Avonstage error: model type \"%s\" not found.\n", 
					mod->GetModelType().c_str() );
	return 1; //fail
}

int main( int argc, char* argv[] )
{
  // initialize libstage - call this first
  Stg::Init( &argc, &argv );

  printf( "%s %s ", PROJECT, VERSION );
  
  int ch=0, optindex=0;
  bool usegui = true;
  bool showclock = false;
  
  std::string host = "localhost";
  std::string rootdir = ".";
  unsigned short port = AV_DEFAULT_PORT;
  bool verbose = false;

  while ((ch = getopt_long(argc, argv, "cvrgh?p?", longopts, &optindex)) != -1)
	 {
		switch( ch )
		  {
		  case 0: // long option given
			 printf( "option %s given\n", longopts[optindex].name );
			 break;

			 // stage options
		  case 'a':
			 Stg::World::ctrlargs = std::string(optarg);
			 break;
		  case 'c': 
			 showclock = true;
			 printf( "[Clock enabled]" );
			 break;
		  case 'g': 
			 usegui = false;
			 printf( "[GUI disabled]" );
			 break;
			 
			 // avon options
		  case 'p':
			 port = atoi(optarg);
			 break;
		  case 'h':
			 host = std::string(optarg);
			 break;
			 // 		  case 'f':
			 // 			 fedfilename = optarg;
			 // 			 usefedfile = true;
			 // 			 break;
		  case 'r':
			 rootdir = std::string(optarg);
			 break;
		  case 'v':
			 verbose = true;
			 break;
			 // help options
		  case '?':  
			 puts( USAGE );
			 exit(0);
			 break;
		  default:
			 printf("unhandled option %c\n", ch );
			 puts( USAGE );
			 //exit(0);
		  }
	 }
  
  const char* worldfilename = argv[optind];

  if( worldfilename == NULL )
	 {	
		puts( "[AvonStage] no worldfile specified on command line. Quit.\n" );
		exit(-1);
	 }

  puts("");// end the first start-up line
  
  printf( "[AvonStage] host %s:%u world %s\n",
			 host.c_str(), 
			 port, 
			 worldfilename );
  
	char version[1024];
	snprintf( version, 1024, "%s (%s-%s)",
						AVONSTAGE_VERSION,
						PROJECT,
						VERSION );

  // avon
  av_init( host.c_str(), port, rootdir.c_str(), verbose, "AvonStage", version );

  
  // arguments at index [optindex] and later are not options, so they
  // must be world file names
  
  
  Stg::World* world = ( usegui ? 
												new Stg::WorldGui( 400, 300, worldfilename ) : 
												new Stg::World( worldfilename ) );
	
  world->Load( worldfilename );
  world->ShowClock( showclock );
	
	// now we have a world object, we can install a clock callback
	av_install_clock_callbacks( (av_clock_get_t)GetTimeWorld, world );
	
  // start the http server
  av_startup();
	
  // register all models here  
  world->ForEachDescendant( RegisterModel, NULL );
 
  if( ! world->paused ) 
	 world->Start();
    
  while( 1 )
	 {
		// TODO - loop properly

		Fl::check(); 
		av_check();	 

		usleep(100); // TODO - loop sensibly here
	 }
  
  puts( "\n[AvonStage: done]" );
  
  return EXIT_SUCCESS;
}

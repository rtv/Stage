#include <getopt.h>
#include <websim.hh>
#include <stage.hh>
using namespace Stg;
#include "config.h"


/* options descriptor */
static struct option longopts[] = {
  { "gui",  no_argument,   NULL,  'g' },
  { "port",  required_argument,   NULL,  'p' },
  { "host",  required_argument,   NULL,  'h' },
  { "federation",  required_argument,   NULL,  'f' },
  { NULL, 0, NULL, 0 }
};

class WebStage : public websim::WebSim
{
  Stg::World* world;
  
public:
  WebStage( Stg::World* world,
				const std::string& host, const unsigned short port ) :
    websim::WebSim( host, port ),
    world(world)
  {	
  }
  
  virtual ~WebStage()
  {}

  virtual std::string IdentificationString()
  { return "WebStage"; }

  virtual std::string VersionString()
  { return Stg::Version(); }
  
  virtual bool ClockStart()
  {
	 puts( "[WebStage]  Clock start" );
	 world->Start();
	 return true;
  }

  virtual bool ClockStop()
  {
	 puts( "[WebStage]  Clock stop" );
	 world->Stop();
	 return true;
  }

  virtual bool ClockRunFor( uint32_t msec )
  {
	 puts( "[WebStage]  Clock tick" );
	 
	 world->paused = true;
	 world->sim_interval = msec * 1e-3; // usec
	 world->Update();

	 return true;
  }

  virtual bool GetModelType( const std::string& name, std::string& type )
  {
	 Stg::Model* mod = world->GetModel( name.c_str() );

	 if( ! mod )
		return false;
	 
	 type = mod->GetModelType();
	 return true;
  }
 

  // void Push( const std::string& name )
  // {
	//  Stg::Model* mod = world->GetModel( name.c_str() );

	//  if( mod )
	// 	{
	// 	  websim::Pose p;
	// 	  websim::Velocity v;
	// 	  websim::Acceleration a;

	// 	  Stg::Pose sp = mod->GetPose(); 
	// 	  p.x = sp.x;
	// 	  p.y = sp.y;
	// 	  p.z = sp.z;
	// 	  p.a = sp.a;
		  
	// 	  Stg::Velocity sv = mod->GetVelocity(); 
	// 	  v.x = sv.x;
	// 	  v.y = sv.y;
	// 	  v.z = sv.z;
	// 	  v.a = sv.a;

	// 	  SetPuppetPVA( name, p, v, a );
	// 	}
	//  else
	// 	printf( "Warning: attempt to push PVA for unrecognized model \"%s\"\n",
	// 			  name.c_str() );
  // }
  
  // void Push()
  // {
	//  for( std::map<std::string,Puppet*>::iterator it = puppets.begin();
	// 		it != puppets.end();
	// 		it++ )
	// 	{
	// 	  Puppet* pup = it->second;
	// 	  assert(pup);
		  
	// 	  Stg::Model* mod = world->GetModel( pup->name.c_str() );
	// 	  assert(mod);
		  
	// 	  websim::Pose p;
	// 	  websim::Velocity v;
	// 	  websim::Acceleration a;
		  
	// 	  Stg::Pose sp = mod->GetPose(); 
	// 	  p.x = sp.x;
	// 	  p.y = sp.y;
	// 	  p.z = sp.z;
	// 	  p.a = sp.a;
		  
	// 	  Stg::Velocity sv = mod->GetVelocity(); 
	// 	  v.x = sv.x;
	// 	  v.y = sv.y;
	// 	  v.z = sv.z;
	// 	  v.a = sv.a;
		  
	// 	  pup->Push( p,v,a );
	// 	  printf( "pushing puppet %s\n", pup->name.c_str() );
	// 	}
  // }

  virtual bool GetModelChildren(const std::string& model, 
										  std::vector<std::string>& children)
  {
	 std::vector<Model*> c;

	 if(model == "")
		{
		  c = world->GetChildren();
		}
	 else
		{
		  Stg::Model* mod = world->GetModel( model.c_str() );
		  if( mod ){
			
			 c = mod->GetChildren();
				
		  }else{
			 printf("Warning: Tried to get the children of unknown model:%s \n", model.c_str());
			 return false;
		  }
			
		}
	
	 FOR_EACH( it, c )
		{		
		  children.push_back(std::string((*it)->Token()));
		}

	 return true;	
	
  }

  // Interface to be implemented by simulators
  virtual bool CreateModel(const std::string& name, 
									const std::string& type,
									std::string& error)
  { 
	 printf( "create model name:%s type:%s\n", name.c_str(), type.c_str() ); 
	 
	 Model* mod = world->CreateModel( NULL, type.c_str() ); // top level models only for now
	 
	 // rename the model and store it by the new name
	 mod->SetToken( name.c_str() );
	 world->AddModel( mod );
	 

	 printf( "done." );
	 return true;
  }

  virtual bool DeleteModel(const std::string& name,
									std::string& error)
  {
	 printf( "delete model name:%s \n", name.c_str() ); 
	 return true;
  }
	
	const websim::PVA ModToPVA( Model* mod )
	{
		// construct a PVA from Stage's values
		Stg::Pose sp = mod->GetPose(); 
		Stg::Velocity sv = mod->GetVelocity(); 		
		return( websim::PVA( websim::Pose( sp.x, sp.y, sp.z, 0.0, 0.0, sp.a ),
												 websim::Velocity( sv.x, sv.y, sv.z, 0.0, 0.0, sv.a ),
												 websim::Acceleration() ));
	}

  virtual bool GetModelData(const std::string& name,
	
									 std::string& response,
									 websim::Format format,
									 void* xmlparent) {
	 std::string str;
	 websim::Time t = GetTime();
	 
	 Model*mod = world->GetModel( name.c_str() );
	 if(mod){
		 std::string type = mod->GetModelType();
		 if(type == "position") 
			 {				 
				 response = WebSim::FormatPVA( name, t, ModToPVA(mod), format );			
				 return true;			
			 }
		 else if(type == "laser")
			 {			 
			 uint32_t resolution;
		  double fov;
		  websim::Pose p;
		  std::vector<double> ranges;												
			 

		  ModelLaser* laser = (ModelLaser*)mod;  		
		  uint32_t sample_count=0;
		  std::vector<ModelLaser::Sample> scan = laser->GetSamples();
        	     	 
		  ModelLaser::Config cfg = laser->GetConfig();
		  resolution =  cfg.resolution;
		  fov = cfg.fov;
               	     		
		  sample_count = scan.size();
		  for(unsigned int i=0;i<sample_count;i++)
			 ranges.push_back(scan[i].range);
			 
		  WebSim::GetLaserData(name, t, resolution, fov, p, ranges, format, response, xmlparent);
			 

		}else if(type == "ranger"){

		  std::vector<websim::Pose> p;
		  std::vector<double> ranges;
  			
		  ModelRanger* ranger = (ModelRanger*)mod;  		
                     
		  if(ranger){ 					
			 for(unsigned int i=0;i<ranger->sensors.size();i++){
				//char str[10];
				//sprintf(str,"size:%d",ranger->sensors.size());
				//puts(str);
				//puts("in the ranger loop");
				websim::Pose pos;
				Pose rpos;
				rpos = ranger->sensors[i].pose;
				pos.x = rpos.x;
				pos.y = rpos.y;
				pos.z = rpos.z;
				pos.a = rpos.a;
				p.push_back(pos);

				ranges.push_back(ranger->sensors[i].range);					
			 }
		  }
			
		  WebSim::GetRangerData(name, t, p, ranges, format, response, xmlparent);


		}else if(type == "fiducial"){

		  ModelFiducial::Fiducial* fids;
		  unsigned int n=0;
		  std::vector<websim::Fiducial> f;												

		  ModelFiducial* fiducial = (ModelFiducial*)mod;  		
        	        
			 
		  fids = fiducial->GetFiducials(&n);
     	     	 	           	     		
			 
		  for(unsigned int i=0;i<n;i++){
			 websim::Fiducial fid;
			 fid.pos.x = fids[i].range*cos(fids[i].bearing);
			 fid.pos.y = fids[i].range*sin(fids[i].bearing);
			 //fid.range = fids[i].range;
			 //fid.bearing = fids[i].bearing;
			 fid.id = fids[i].id;

			 f.push_back(fid);
			 //printf("stage: fiducials:%d\n",f.size());
		  }
	
        	     	          	     		
		  WebSim::GetFiducialData(name, t, f, format, response, xmlparent);
			 

		}else{
			
		  //printf("Warning: Unkown model type\n");	
		  return false;

		}
	 }
	 else{
		printf("Warning: tried to get the data of unkown model:%s .\n", name.c_str()); 
		return false;
	 }
	 return true;
  }
  
  virtual bool SetModelPVA(const std::string& name, 
													 const websim::PVA& pva,
													 std::string& response )
  {
	 //printf( "set model PVA name:%s\n", name.c_str() ); 	 

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		 {
			 mod->SetPose( Stg::Pose( pva.p.x,  pva.p.y,  pva.p.z,  pva.p.a ));
			 mod->SetVelocity( Stg::Velocity(  pva.v.x,  pva.v.y,  pva.v.z,  pva.v.a ));		 
			 // stage doesn't model acceleration

			// force GUI update to see the change if Stage was paused
			mod->Redraw();

			response = "OK";
			return true;
		}
	 //else
	 
	 printf( "Warning: attempt to set PVA for unrecognized model \"%s\"\n",
							 name.c_str() );
	 response = "unknown model " + name;
	 return false;
  }
	
  virtual bool GetModelPVA(const std::string& name, 
													 websim::Time& t,
													 websim::PVA& pva,
													 std::string& error )
  {
	 //printf( "get model name:%s\n", name.c_str() ); 

	 t = GetTime();
	 
	 Model* mod = world->GetModel( name.c_str() );

	 if( !mod  ) 
		 {			 
			 error = "attempt to set PVA for unrecognized model '" + name + "'";
			 puts( error.c_str() );
			 return false;
		 }

	 // else model is good	 
	 
	 // zero all fields
	 pva.Zero();
	 
	 Stg::Pose sp = mod->GetPose(); 
	 pva.p.x = sp.x;
	 pva.p.y = sp.y;
	 pva.p.z = sp.z;
	 pva.p.a = sp.a;
	 
	 Stg::Velocity sv = mod->GetVelocity(); 
	 pva.v.x = sv.x;
	 pva.v.y = sv.y;
	 pva.v.z = sv.z;
	 pva.v.a = sv.a;
	 
	 return true;
  }
  /*
	 virtual bool GetLaserData(const std::string& name,			
	 websim::Time& t,												
	 uint32_t& resolution,
	 double& fov,
	 websim::Pose& p,
	 std::vector<double>& ranges,													
	 std::string& error,
	 void* parent){


	 t = GetTime();

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
	 {
	 ModelLaser* laser = (ModelLaser*)mod->GetModel("laser:0");  		
			
	 if(laser){
	 uint32_t sample_count=0;
	 ModelLaser::Sample* scan = laser->GetSamples( &sample_count );
	 assert(scan);
		    
	 ModelLaser::Config  cfg = laser->GetConfig();
	 resolution =  cfg.resolution;
	 fov = cfg.fov;
        
	 for(unsigned int i=0;i<sample_count;i++)
	 ranges.push_back(scan[i].range);
	 }else{

	 printf( "Warning: attempt to get laser data for unrecognized laser model of model \"%s\"\n",
	 name.c_str() );
	 return false;


	 }
	         	  
	 }
	 else{
	 printf( "Warning: attempt to get laser data for unrecognized model \"%s\"\n",
	 name.c_str() );
	 return false;
	 }

	 return true;

	 }					   
	 virtual bool GetRangerData(const std::string& name,
	 websim::Time& t,
	 std::vector<websim::Pose>& p,
	 std::vector<double>& ranges,
	 std::string& response,
	 xmlNode* parent){
	 t = GetTime();

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
	 {
	 ModelRanger* ranger = (ModelRanger*)mod->GetModel("ranger:0");  		
       
	 if(ranger){
	 uint32_t count = ranger->sensors.size();
	 for(unsigned int i=0;i<count;i++)
	 ranges.push_back(ranger->sensors[i].range);
	 //std::copy(ranger->samples,ranger->samples+ranger->sensor_count,ranges.begin());
				 
	 for(unsigned int i=0;i<count;i++){
	 websim::Pose pos;
	 Pose rpos;
	 rpos = ranger->sensors[i].pose;
	 pos.x = rpos.x;
	 pos.y = rpos.y;
	 pos.z = rpos.z;
	 pos.a = rpos.a;
	 p.push_back(pos);					
	 }
				 
	 }else{
				 
	 printf( "Warning: attempt to get ranger data for unrecognized ranger model of model \"%s\"\n",
	 name.c_str() );
	 return false;				 				 
	 }	     
	 }
	 else{
	 printf( "Warning: attempt to get ranger data for unrecognized model \"%s\"\n",
	 name.c_str() );
	 return false;
	 }

	 return true;

	 }*/

  virtual bool GetModelGeometry(const std::string& name,
																websim::Time& t,
																websim::Geometry& geom,
																std::string& error )
  {
		if(name == "sim")
			{
				stg_bounds3d_t ext = world->GetExtent();    	
				geom.extent.x = ext.x.max - ext.x.min;
				geom.extent.y = ext.y.max - ext.y.min;
				geom.extent.z = ext.z.max - ext.z.min;				
				return true;
			}
		
		Model* mod = world->GetModel(name.c_str());
		if( !mod )
			{
				error = "Error: attempt to get the geometry of unrecognized model \"" + name + "\"";
				puts( error.c_str() );
				return false;		
			}
		
		Geom ext = mod->GetGeom();    	
		geom.extent.x = ext.size.x;
		geom.extent.y = ext.size.y;
		geom.extent.z = ext.size.z;
		geom.pose.x = ext.pose.x;
		geom.pose.y = ext.pose.y;
		geom.pose.a = ext.pose.a;
		return true;
  }
	
  static int CountRobots(Model * mod, int* n ){
 
	 if(n && mod->GetModelType() == "position")
		(*n)++;
  
	 return 0;
  } 
  
  virtual bool GetNumberOfRobots(unsigned int& n)
  {
	
	
	 world->ForEachDescendant((stg_model_callback_t)CountRobots, &n);	
	 return true;

  }
  /*
	 virtual bool GetModelTree()
	 {
	
	 //	world->ForEachDescendant((stg_model_callback_t)printname, NULL);	

	 return true;
	 }
  */
  virtual bool GetSayStrings(std::vector<std::string>& sayings)
  {
	 unsigned int n=0;
	 this->GetNumberOfRobots(n);
	
	 for(unsigned int i=0;i<n;i++){
		char temp[128];
		sprintf(temp,"position:%d",i);
		Model *mod = world->GetModel(temp);
		if(mod->GetSayString() != "")
		  {	
			
			 std::string str = temp;
			 str += " says: \" ";
			 str += mod->GetSayString();
			 str += " \" ";
			
			 sayings.push_back(str);
			
		  }
	 }

	 return true;
  }

  virtual websim::Time GetTime()
  {
	 stg_usec_t stgtime = world->SimTimeNow();

	 websim::Time t;
	 t.sec = stgtime / 1e6;
	 t.usec = stgtime - (t.sec * 1e6);
	 return t;
  }
  
	// add an FLTK event loop update to WebSim's implementation
	virtual void Wait()
	{
	 	Check(); // nonblocking check for server events
	 	Fl::check(); // nonblocking check for GUI events 		  
	}
  
}; // close WebStage class


int main( int argc, char** argv )
{
  // initialize libstage - call this first
  Stg::Init( &argc, &argv );

  //printf( "WebStage built on %s %s\n", PROJECT, VERSION );
  
  std::string fedfilename = "";
  std::string host = "localhost";
  unsigned short port = 8000;
  
  int ch=0, optindex=0;
  bool usegui = true;
  bool usefedfile = false;

  while ((ch = getopt_long(argc, argv, "gh:p:f:", longopts, &optindex)) != -1)
	 {
		switch( ch )
		  {
		  case 0: // long option given
			 printf( "option %s given", longopts[optindex].name );
			 break;
		  case 'g': 
			 usegui = false;
			 printf( "[GUI disabled]" );
			 break;
		  case 'p':
			 port = atoi(optarg);
			 break;
		  case 'h':
			 host = optarg;
			 break;
		  case 'f':
			 fedfilename = optarg;
			 usefedfile = true;
			 break;
		  case '?':  
			 break;
		  default:
			 printf("unhandled option %c\n", ch );
		  }
	 }
  
  puts("");// end the first start-up line

  const char* worldfilename = argv[optind];

  printf( "[webstage] %s:%u fed %s world %s\n",
			 host.c_str(), 
			 port, 
			 fedfilename.c_str(), 
			 worldfilename );
			 
  World* world = ( usegui ? 
						 new WorldGui( 400, 300, worldfilename ) : 
						 new World( worldfilename ) );
  world->Load( worldfilename );

  WebStage ws( world, host, port );

  //if( usefedfile )
		//	 ws.LoadFederationFile( fedfilename );

  ws.Startup( true ); // start http server
  
  if( ! world->paused ) 
	 world->Start(); // start sumulation running

  //close program once time has completed
  bool quit = false;
  while( ! quit )
	 {
		// todo? check for changes?
		// send my updates		
		//ws.Push();
		//puts( "push  done" );

		// run one step of the simulation
		//ws.Tick();
		
		//puts( "tick done" );
		
		// update Stage
		//world->Update();
		//puts( "update done" );
	  
		// wait until everyone report simulation step done
		ws.Wait();			

		usleep(100); // TODO - loop sensibly here

		//puts( "wait done" );
	 }

  printf( "Webstage done.\n" );
  return 0;			
}

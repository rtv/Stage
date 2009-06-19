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
  
  void Push( const std::string& name )
  {
	 Stg::Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
		  websim::Pose p;
		  websim::Velocity v;
		  websim::Acceleration a;

		  Stg::Pose sp = mod->GetPose(); 
		  p.x = sp.x;
		  p.y = sp.y;
		  p.z = sp.z;
		  p.a = sp.a;
		  
		  Stg::Velocity sv = mod->GetVelocity(); 
		  v.x = sv.x;
		  v.y = sv.y;
		  v.z = sv.z;
		  v.a = sv.a;

		  SetPuppetPVA( name, p, v, a );
		}
	 else
		printf( "Warning: attempt to push PVA for unrecognized model \"%s\"\n",
				  name.c_str() );
  }
  
  void Push()
  {
	 for( std::map<std::string,Puppet*>::iterator it = puppets.begin();
			it != puppets.end();
			it++ )
		{
		  Puppet* pup = it->second;
		  assert(pup);
		  
		  Stg::Model* mod = world->GetModel( pup->name.c_str() );
		  assert(mod);
		  
		  websim::Pose p;
		  websim::Velocity v;
		  websim::Acceleration a;
		  
		  Stg::Pose sp = mod->GetPose(); 
		  p.x = sp.x;
		  p.y = sp.y;
		  p.z = sp.z;
		  p.a = sp.a;
		  
		  Stg::Velocity sv = mod->GetVelocity(); 
		  v.x = sv.x;
		  v.y = sv.y;
		  v.z = sv.z;
		  v.a = sv.a;
		  
		  pup->Push( p,v,a );
		  printf( "pushing puppet %s\n", pup->name.c_str() );
		}
  }

  virtual bool GetModelChildren(const std::string& model, 
									std::vector<std::string>& children)
  {
	GList* c;

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
	
	for(;c;c = c->next){
		
		children.push_back(std::string(((Model*)c->data)->Token()));
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

  virtual bool GetModelData(const std::string& name,
	
								std::string& response,
								websim::Format format,
								void* xmlparent) {
	std::string str;
	websim::Time t = GetTime();

	Model*mod = world->GetModel( name.c_str() );
	if(mod){
		stg_model_type_t type = mod->GetModelType();
		if(type == MODEL_TYPE_POSITION){
			
								
			
			websim::Pose p;
			websim::Velocity v;
			websim::Acceleration a;
			
			Stg::Pose sp = mod->GetPose(); 
		  	p.x = sp.x;
			p.y = sp.y;
			p.z = sp.z;
		  	p.a = sp.a;

		  	Stg::Velocity sv = mod->GetVelocity(); 
		  	v.x = sv.x;
		  	v.y = sv.y;
		  	v.z = sv.z;
		  	v.a = sv.a;
		
			WebSim::GetPVA(name, t, p, v, a, format, response, xmlparent);
			
			

		}else if(type == MODEL_TYPE_LASER){

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
			 

		}else if(type == MODEL_TYPE_RANGER){

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


		}else if(type == MODEL_TYPE_FIDUCIAL){

 			 ModelFiducial::Fiducial* fids;
			 unsigned int n=0;
			 std::vector<websim::Fiducial> f;												

 	                 ModelFiducial* fiducial = (ModelFiducial*)mod;  		
        	        
			 
			 fids = fiducial->GetFiducials(&n);
     	     	 	           	     		
			 
			 for(unsigned int i=0;i<n;i++){
				websim::Fiducial fid;
				fid.range = fids[i].range;
				fid.bearing = fids[i].bearing;
				fid.id = fids[i].id;

				f.push_back(fid);
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
  
 bool GetModelType(const std::string& name, 
        								
        								std::string& type ){

	Model*mod = world->GetModel( name.c_str() );
	if(mod){
		stg_model_type_t mtype = mod->GetModelType();
		
		if(mtype == MODEL_TYPE_POSITION){
			type =  "Position";
		}else if(mtype == MODEL_TYPE_LASER){
			type =  "Laser";

		}else if(mtype == MODEL_TYPE_RANGER){
			type = "Ranger";

		}else if(mtype == MODEL_TYPE_FIDUCIAL){
			type = "Fiducial";

		}else{
			type = "";

		}
		
		return true;
	}	


	return false;

  }



 virtual bool SetModelPVA(const std::string& name, 
									const websim::Pose& p,
									const websim::Velocity& v,
									const websim::Acceleration& a,
									std::string& error)
  {
	 //printf( "set model PVA name:%s\n", name.c_str() ); 	 

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
		  mod->SetPose( Stg::Pose( p.x, p.y, p.z, p.a ));
		  mod->SetVelocity( Stg::Velocity( v.x, v.y, v.z, v.a ));		 
		  // stage doesn't model acceleration
		}
	 else
		printf( "Warning: attempt to set PVA for unrecognized model \"%s\"\n",
				  name.c_str() );

	 return true;
  }

  virtual bool GetModelPVA(const std::string& name, 
									websim::Time& t,
									websim::Pose& p,
									websim::Velocity& v,
									websim::Acceleration& a,
									std::string& error)
  {
	 //printf( "get model name:%s\n", name.c_str() ); 

	 t = GetTime();

	 Model* mod = world->GetModel( name.c_str() );
	 if( mod )
		{
		  Stg::Pose sp = mod->GetPose(); 
		  p.x = sp.x;
		  p.y = sp.y;
		  p.z = sp.z;
		  p.a = sp.a;

		  Stg::Velocity sv = mod->GetVelocity(); 
		  v.x = sv.x;
		  v.y = sv.y;
		  v.z = sv.z;
		  v.a = sv.a;
		}
	 else
		printf( "Warning: attempt to set PVA for unrecognized model \"%s\"\n",
				  name.c_str() );

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

   virtual bool GetModelExtent(const std::string& name,
									double& x,
									double& y,
									double& z,
									websim::Pose& center,
									std::string& response)
  {
	if(name == "sim"){
	
		stg_bounds3d_t ext = world->GetExtent();
    	
		x = ext.x.max - ext.x.min;
		y = ext.y.max - ext.y.min;
		z = ext.z.max - ext.z.min;

	}
	else
	{
		Model* mod = world->GetModel(name.c_str());
		if(mod){
			Geom ext = mod->GetGeom();
    	
			x = ext.size.x;
			y = ext.size.y;
			z = ext.size.z;
			center.x = ext.pose.x;
			center.y = ext.pose.y;
			center.a = ext.pose.a;
		}
		else
		{
		  printf("Warning: attemp to get the extent of unrecognized model \"%s\"\n", name.c_str());
		  return false;		
		}
	}
        
	return true;
  }

   static int CountRobots(Model * mod, int* n ){
 
 	if(n && mod->GetModelType() == MODEL_TYPE_POSITION)
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

};


int main( int argc, char** argv )
{
  // initialize libstage - call this first
  Stg::Init( &argc, &argv );

  printf( "WebStage built on %s %s\n", PROJECT, VERSION );
  
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
  
  if( usefedfile )
	 ws.LoadFederationFile( fedfilename );

  puts( "entering main loop" );

  //close program once time has completed
  bool quit = false;
  while( ! quit )
	 {
		// todo? check for changes?
		// send my updates		
		ws.Push();
		//puts( "pushes  done" );

		// tell my friends to start simulating
		ws.Go();
		
		// puts( "go done" );
		
		// update Stage
		world->Update();
		//puts( "update done" );

		// wait for goes from all my friends
		ws.Wait();			
		//puts( "wait done" );
	 }

  printf( "Webstage done.\n" );
  return 0;			
}

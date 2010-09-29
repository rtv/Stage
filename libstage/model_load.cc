#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <limits.h> 
#include <libgen.h> // for dirname()
#include <string.h>
#include <ltdl.h> // for library module loading

#include "stage.hh"
#include "worldfile.hh"
#include "file_manager.hh"
#include "config.h"
using namespace Stg;

//#define DEBUG

void Model::Load()
{  
  assert( wf );
  assert( wf_entity );
  
  PRINT_DEBUG1( "Model \"%s\" loading...", token );
  
  // choose the thread to run in, if thread_safe > 0 
  event_queue_num = wf->ReadInt( wf_entity, "event_queue", event_queue_num );

  if( wf->PropertyExists( wf_entity, "joules" ) )
	 {
		if( !power_pack )
		  power_pack = new PowerPack( this );
		
		stg_joules_t j = wf->ReadFloat( wf_entity, "joules", 
												  power_pack->GetStored() ) ;	 
		
		/* assume that the store is full, so the capacity is the same as
			the charge */
		power_pack->SetStored( j );
		power_pack->SetCapacity( j );
	 }

  if( wf->PropertyExists( wf_entity, "joules_capacity" ) )
	 {
		if( !power_pack )
		  power_pack = new PowerPack( this );
		
		power_pack->SetCapacity( wf->ReadFloat( wf_entity, "joules_capacity",
															 power_pack->GetCapacity() ) ); 		
	 }

  if( wf->PropertyExists( wf_entity, "kjoules" ) )
	 {
		if( !power_pack )
		  power_pack = new PowerPack( this );
		
		stg_joules_t j = 1000.0 * wf->ReadFloat( wf_entity, "kjoules", 
															  power_pack->GetStored() ) ;	 
		
		/* assume that the store is full, so the capacity is the same as
			the charge */
		power_pack->SetStored( j );
		power_pack->SetCapacity( j );
	 }
  
  if( wf->PropertyExists( wf_entity, "kjoules_capacity" ) )
	 {
		if( !power_pack )
		  power_pack = new PowerPack( this );
		
		power_pack->SetCapacity( 1000.0 * wf->ReadFloat( wf_entity, "kjoules_capacity",
																		 power_pack->GetCapacity() ) ); 		
	 }

    
  watts = wf->ReadFloat( wf_entity, "watts", watts );    
  watts_give = wf->ReadFloat( wf_entity, "give_watts", watts_give );    
  watts_take = wf->ReadFloat( wf_entity, "take_watts", watts_take );
  
  this->debug = wf->ReadInt( wf_entity, "debug", this->debug );
  
  const std::string& name = wf->ReadString(wf_entity, "name", token );
  if( name != token )
	 {
		//printf( "adding name %s to %s\n", name, this->token );
		this->token = name ;
		world->AddModelName( this, name ); // add this name to the world's table
	 }

  //PRINT_WARN1( "%s::Load", token );
  
  if( wf->PropertyExists( wf_entity, "origin" ) )
	 {
		Geom geom = GetGeom();
		geom.pose.Load( wf, wf_entity, "origin" );
		SetGeom( geom );
	 }

  if( wf->PropertyExists( wf_entity, "size" ) )
    {
      Geom geom = GetGeom();
			geom.size.Load( wf, wf_entity, "size" );
      SetGeom( geom );
    }
		
  if( wf->PropertyExists( wf_entity, "pose" ))
    {
      Pose pose = GetPose();
		pose.Load( wf, wf_entity, "pose" );
      SetPose( pose );
    }
  
  if( wf->PropertyExists( wf_entity, "velocity" ))
    {
      Velocity vel = GetVelocity();
		vel.Load( wf, wf_entity, "velocity" );
      SetVelocity( vel );
    }
  
  if( wf->PropertyExists( wf_entity, "color" ))
	 {      
		Color col( 1,0,0 ); // red;
		const std::string& colorstr = wf->ReadString( wf_entity, "color", "" );
		if( colorstr != "" )
		  {
			 if( colorstr == "random" )
				col = Color( drand48(), drand48(), drand48() );
			 else
				col = Color( colorstr );
		  }
		this->SetColor( col );
	 }        
  
  if( wf->PropertyExists( wf_entity, "color_rgba" ))
    {      
      if (wf->GetProperty(wf_entity,"color_rgba")->values.size() < 4)
      {
        PRINT_ERR1( "model %s color_rgba requires 4 values\n", this->token.c_str() );
      }
      else
      {
      double red   = wf->ReadTupleFloat( wf_entity, "color_rgba", 0, 0 );
      double green = wf->ReadTupleFloat( wf_entity, "color_rgba", 1, 0 );
      double blue  = wf->ReadTupleFloat( wf_entity, "color_rgba", 2, 0 );
      double alpha = wf->ReadTupleFloat( wf_entity, "color_rgba", 3, 1 );

      this->SetColor( Color( red, green, blue, alpha ));
    }  
    }
  
  if( wf->ReadInt( wf_entity, "noblocks", 0 ) )
	 {
      if( has_default_block )
		  {
			 blockgroup.Clear();
			 has_default_block = false;
			 blockgroup.CalcSize();
		  }		
	 }
  
  if( wf->PropertyExists( wf_entity, "bitmap" ) )
    {
      const std::string bitmapfile = wf->ReadString( wf_entity, "bitmap", "" );
		if( bitmapfile == "" )
		  PRINT_WARN1( "model %s specified empty bitmap filename\n", token.c_str() );
		
      if( has_default_block )
		  {
			 blockgroup.Clear();
			 has_default_block = false;
		  }
		
      blockgroup.LoadBitmap( this, bitmapfile, wf );
    }
  
  if( wf->PropertyExists( wf_entity, "boundary" ))
    {
      this->SetBoundary( wf->ReadInt(wf_entity, "boundary", this->boundary  ));
		
      if( boundary )
		  {
			 //PRINT_WARN1( "setting boundary for %s\n", token );
			 
			 blockgroup.CalcSize();
			 
			 double epsilon = 0.01;	      
			 Size bgsize = blockgroup.GetSize();
			 
			 AddBlockRect(blockgroup.minx,blockgroup.miny, epsilon, bgsize.y, bgsize.z );	      
			 AddBlockRect(blockgroup.minx,blockgroup.miny, bgsize.x, epsilon, bgsize.z );	      
			 AddBlockRect(blockgroup.minx,blockgroup.maxy-epsilon, bgsize.x, epsilon, bgsize.z );	      
			 AddBlockRect(blockgroup.maxx-epsilon,blockgroup.miny, epsilon, bgsize.y, bgsize.z );	      
		  }     
    }	  
  
  this->stack_children =
    wf->ReadInt( wf_entity, "stack_children", this->stack_children );
  
  stg_kg_t m = wf->ReadFloat(wf_entity, "mass", this->mass );
  if( m != this->mass ) 
	 SetMass( m );
  	
  vis.Load( wf, wf_entity );
  SetFiducialReturn( vis.fiducial_return ); // may have some work to do
  
  gui.Load( wf, wf_entity );

  double res = wf->ReadFloat(wf_entity, "map_resolution", this->map_resolution );
  if( res != this->map_resolution )
    SetMapResolution( res );
  
  velocity_enable = wf->ReadInt( wf_entity, "enable_velocity", velocity_enable );
	
  if( wf->PropertyExists( wf_entity, "friction" ))
  {
    this->SetFriction( wf->ReadFloat(wf_entity, "friction", this->friction ));
  }
  
  if( CProperty* ctrlp = wf->GetProperty( wf_entity, "ctrl" ) )
	 {
		for( unsigned int index=0; index < ctrlp->values.size(); index++ )
		  {
			 
			 const char* lib = wf->GetPropertyValue( ctrlp, index );
			 
			 if( !lib )
				printf( "Error - NULL library name specified for model %s\n", token.c_str() );
			 else
				LoadControllerModule( lib );
		  }
    }
    
	// internally interval is in usec, but we use msec in worldfiles
	interval = 1000 * wf->ReadInt( wf_entity, "update_interval", interval/1000 );

  Say( wf->ReadString(wf_entity, "say", "" ));
  
  trail_length = wf->ReadInt( wf_entity, "trail_length", trail_length );
  trail.resize( trail_length );

	trail_interval = wf->ReadInt( wf_entity, "trail_interval", trail_interval );

	this->alwayson = wf->ReadInt( wf_entity, "alwayson",  alwayson );
	if( alwayson )
	 Subscribe();
	
  // call any type-specific load callbacks
  this->CallCallbacks( CB_LOAD );
    
  // we may well have changed blocks or geometry
  blockgroup.CalcSize();
  UnMapWithChildren();
  MapWithChildren();
	 
  if( this->debug )
    printf( "Model \"%s\" is in debug mode\n", token.c_str() );

  PRINT_DEBUG1( "Model \"%s\" loading complete", token.c_str() );
}


void Model::Save( void )
{  
  //printf( "Model \"%s\" saving...\n", token.c_str() );

  // some models were not loaded, so have no worldfile. Just bail here.
  if( wf == NULL )
	 return;

  assert( wf_entity );
	
  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
								token,
								pose.x,
								pose.y,
								pose.a );
	
  // just in case
  pose.a = normalize( pose.a );
  geom.pose.a = normalize( geom.pose.a );
  
  if( wf->PropertyExists( wf_entity, "pose" ) )
    {
      wf->WriteTupleLength( wf_entity, "pose", 0, pose.x);
      wf->WriteTupleLength( wf_entity, "pose", 1, pose.y);
      wf->WriteTupleLength( wf_entity, "pose", 2, pose.z);
      wf->WriteTupleAngle( wf_entity, "pose", 3, pose.a );
    }
  
  if( wf->PropertyExists( wf_entity, "size" ) )
    {
      wf->WriteTupleLength( wf_entity, "size", 0, geom.size.x);
      wf->WriteTupleLength( wf_entity, "size", 1, geom.size.y);
      wf->WriteTupleLength( wf_entity, "size", 2, geom.size.z);
    }
  
  if( wf->PropertyExists( wf_entity, "origin" ) )
    {
      wf->WriteTupleLength( wf_entity, "origin", 0, geom.pose.x);
      wf->WriteTupleLength( wf_entity, "origin", 1, geom.pose.y);
      wf->WriteTupleLength( wf_entity, "origin", 2, geom.pose.z);
      wf->WriteTupleAngle( wf_entity, "origin", 3, geom.pose.a);
    }

  // call any type-specific save callbacks
  CallCallbacks( CB_SAVE );

  PRINT_DEBUG1( "Model \"%s\" saving complete.", token );
}


void Model::LoadControllerModule( const char* lib )
{
  //printf( "[Ctrl \"%s\"", lib );
  //fflush(stdout);

  /* Initialise libltdl. */
  int errors = lt_dlinit();
  if (errors)
  {
    printf( "Libtool error: %s. Failed to init libtool. Quitting\n",
      lt_dlerror() ); // report the error from libtool
    puts( "libtool error #1" );
    fflush( stdout );
    exit(-1);
  }

  lt_dlsetsearchpath( FileManager::stagePath().c_str() );

  // PLUGIN_PATH now defined in config.h
  lt_dladdsearchdir( PLUGIN_PATH );

  lt_dlhandle handle = NULL;
  
  // the library name is the first word in the string
  char libname[256];
  sscanf( lib, "%s %*s", libname );
  
  if(( handle = lt_dlopenext( libname ) ))
    {
      //printf( "]" );
		
		stg_model_callback_t initfunc = (stg_model_callback_t)lt_dlsym( handle, "Init" );
      if( initfunc  == NULL )
		  {
			 printf( "Libtool error: %s. Something is wrong with your plugin. Quitting\n",
						lt_dlerror() ); // report the error from libtool
			 puts( "libtool error #1" );
			 fflush( stdout );
			 exit(-1);
		  }
		//else
		
		AddCallback( CB_INIT, initfunc, new CtrlArgs(lib,World::ctrlargs) ); // pass complete string into initfunc
    }
  else
    {
      printf( "Libtool error: %s. Can't open your plugin controller. Quitting\n",
				  lt_dlerror() ); // report the error from libtool
		
      PRINT_ERR1( "Failed to open \"%s\". Check that it can be found by searching the directories in your STAGEPATH environment variable, or the current directory if STAGEPATH is not set.]\n", lib );
		puts( "libtool error #2" );
		fflush( stdout );
      exit(-1);
    }
  
  fflush(stdout);
}


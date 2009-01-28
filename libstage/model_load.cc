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
using namespace Stg;

//#define DEBUG

void Model::Load()
{  
  assert( wf );
  assert( wf_entity );

  PRINT_DEBUG1( "Model \"%s\" loading...", token );
  
  if( wf->PropertyExists( wf_entity, "joules" ) )
	 {
		if( !power_pack )
		  power_pack = new PowerPack( this );
		
		power_pack->stored = 
		  wf->ReadFloat( wf_entity, "joules", power_pack->stored );	 

		/* assume that the store is full, so the capacity is the same as
			the charge */
		power_pack->capacity = power_pack->stored;
	 }
  
  if( wf->PropertyExists( wf_entity, "joules_capacity" ) )
	 {
		if( !power_pack )
		  power_pack = new PowerPack( this );
		
		power_pack->capacity = 
		  wf->ReadFloat( wf_entity, "joules_stored", power_pack->capacity ); 
		
	 }

  /** if the capacity has been specified, limit the store to the capacity */
  if( power_pack && (power_pack->stored > power_pack->capacity) )
	 {			 
		power_pack->stored = power_pack->capacity;
		PRINT_WARN3( "model %s energy storage exceeds capacity (%.2f / %.2f joules). Limited stored energy to max capactity.",
						 token, 
						 power_pack->stored, 
						 power_pack->capacity );
	 }  
  
  if( wf->PropertyExists( wf_entity, "watts" ) )
	 {
		watts = wf->ReadFloat( wf_entity, "watts", watts );
		
		if( watts > 0 )
		  {
			 // find a power pack attached to me or an ancestor in my tree
			 while( (!power_pack) && parent )
				{
				  power_pack = parent->power_pack;
				}
			 
			 if( power_pack == NULL )
				{
				  PRINT_WARN2( "worldfile requests %.2f watts for model %s, but can not find an energy source. Setting watts has no effect unless you also specify  a \"joules\" value for this model or an  ancestor.", watts, token );
				  exit(-1);				  
				}			 
		  }
	 }

  if( wf->PropertyExists( wf_entity, "debug" ) )
    {
      PRINT_WARN2( "debug property specified for model %d  %s\n",
		   wf_entity, this->token );
      this->debug = wf->ReadInt( wf_entity, "debug", this->debug );
    }

  if( wf->PropertyExists( wf_entity, "name" ) )
    {
      char *name = (char*)wf->ReadString(wf_entity, "name", NULL );
      if( name )
	{
	  //printf( "adding name %s to %s\n", name, this->token );
	  this->token = strdup( name );
	  world->AddModel( this ); // add this name to the world's table
	}
      else
	PRINT_ERR1( "Name blank for model %s. Check your worldfile\n", this->token );
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
      stg_color_t col = 0xFFFF0000; // red;
      const char* colorstr = wf->ReadString( wf_entity, "color", NULL );
      if( colorstr )
		  {
			 if( strcmp( colorstr, "random" ) == 0 )
	    {
	      col = (uint32_t)random();
	      col |= 0xFF000000; // set the alpha channel to max
	    }
	  else
	    col = stg_lookup_color( colorstr );

	  this->SetColor( col );
	}
    }      


  if( wf->PropertyExists( wf_entity, "color_rgba" ))
    {      
      double red   = wf->ReadTupleFloat( wf_entity, "color_rgba", 0, 0 );
      double green = wf->ReadTupleFloat( wf_entity, "color_rgba", 1, 0 );
      double blue  = wf->ReadTupleFloat( wf_entity, "color_rgba", 2, 0 );
      double alpha = wf->ReadTupleFloat( wf_entity, "color_rgba", 3, 1 );

      this->SetColor( stg_color_pack( red, green, blue, alpha ));
    }  
  
  if( wf->PropertyExists( wf_entity, "bitmap" ) )
    {
      const char* bitmapfile = wf->ReadString( wf_entity, "bitmap", NULL );
      assert( bitmapfile );

      if( has_default_block )
	{
	  blockgroup.Clear();
	  has_default_block = false;
	}

      //puts( "clearing blockgroup" );
      //blockgroup.Clear();
      //puts( "loading bitmap" );
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

  if( wf->PropertyExists( wf_entity, "mass" ))
    this->SetMass( wf->ReadFloat(wf_entity, "mass", this->mass ));

  vis.Load( wf, wf_entity );
  gui.Load( wf, wf_entity );

  if( wf->PropertyExists( wf_entity, "map_resolution" ))
    this->SetMapResolution( wf->ReadFloat(wf_entity, "map_resolution", this->map_resolution ));

  if( wf->PropertyExists( wf_entity, "ctrl" ))
    {
      char* lib = (char*)wf->ReadString(wf_entity, "ctrl", NULL );
		
      if( !lib )
	printf( "Error - NULL library name specified for model %s\n", token );
      else
	LoadControllerModule( lib );
    }

  
  if( wf->PropertyExists( wf_entity, "say" ))
    this->Say( wf->ReadString(wf_entity, "say", NULL ));
  

  // call any type-specific load callbacks
  this->CallCallbacks( &hooks.load );

  // MUST BE THE LAST THING LOADED
  if( wf->PropertyExists( wf_entity, "alwayson" ))
    {
      if( wf->ReadInt( wf_entity, "alwayson", 0) > 0 )
	Startup();
    }
  
  MapWithChildren();

  if( this->debug )
    printf( "Model \"%s\" is in debug mode\n", token );

  PRINT_DEBUG1( "Model \"%s\" loading complete", token );
}


void Model::Save( void )
{  
  assert( wf );
  assert( wf_entity );
		
  PRINT_DEBUG1( "Model \"%s\" saving...", token );

  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
		this->token,
		this->pose.x,
		this->pose.y,
		this->pose.a );

  if( wf->PropertyExists( wf_entity, "pose" ) )
    {
      wf->WriteTupleLength( wf_entity, "pose", 0, this->pose.x);
      wf->WriteTupleLength( wf_entity, "pose", 1, this->pose.y);
      wf->WriteTupleLength( wf_entity, "pose", 2, this->pose.z);
      wf->WriteTupleAngle( wf_entity, "pose", 3, this->pose.a);
    }
  
  if( wf->PropertyExists( wf_entity, "size" ) )
    {
      wf->WriteTupleLength( wf_entity, "size", 0, this->geom.size.x);
      wf->WriteTupleLength( wf_entity, "size", 1, this->geom.size.y);
      wf->WriteTupleLength( wf_entity, "size", 2, this->geom.size.z);
    }
  
  if( wf->PropertyExists( wf_entity, "origin" ) )
    {
      wf->WriteTupleLength( wf_entity, "origin", 0, this->geom.pose.x);
      wf->WriteTupleLength( wf_entity, "origin", 1, this->geom.pose.y);
      wf->WriteTupleLength( wf_entity, "origin", 2, this->geom.pose.z);
      wf->WriteTupleAngle( wf_entity, "origin", 3, this->geom.pose.a);
    }

  // call any type-specific save callbacks
  this->CallCallbacks( &hooks.save );

  PRINT_DEBUG1( "Model \"%s\" saving complete.", token );
}


// todo - use GLib's portable module code here
void Model::LoadControllerModule( char* lib )
{
  //printf( "[Ctrl \"%s\"", lib );
  //fflush(stdout);

  /* Initialise libltdl. */
  int errors = lt_dlinit();
  assert(errors==0);

  lt_dlsetsearchpath( FileManager::stagePath().c_str() );

  lt_dlhandle handle = NULL;

  if(( handle = lt_dlopenext( lib ) ))
    {
      //printf( "]" );

      this->initfunc = (ctrlinit_t*)lt_dlsym( handle, "Init" );
      if( this->initfunc  == NULL )
	{
	  printf( "Libtool error: %s. Something is wrong with your plugin. Quitting\n",
		  lt_dlerror() ); // report the error from libtool
	  exit(-1);
	}
    }
  else
    {
      printf( "Libtool error: %s. Can't open your plugin controller. Quitting\n",
	      lt_dlerror() ); // report the error from libtool

      PRINT_ERR1( "Failed to open \"%s\". Check that it can be found by searching the directories in your STAGEPATH environment variable, or the current directory if STAGEPATH is not set.]\n", lib );
      exit(-1);
    }

  fflush(stdout);

  // as we now have a controller, the world needs to call our update function
  StartUpdating();
}



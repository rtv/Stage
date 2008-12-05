#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <limits.h> 
#include <libgen.h> // for dirname()
#include <string.h>
#include <ltdl.h>

#include "stage_internal.hh"
#include "file_manager.hh"

//#define DEBUG

void StgModel::Load()
{  
  assert( wf );
  assert( wf_entity );

  PRINT_DEBUG1( "Model \"%s\" loading...", token );
  

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
      stg_geom_t geom = GetGeom();
      geom.pose.x = wf->ReadTupleLength(wf_entity, "origin", 0, geom.pose.x );
      geom.pose.y = wf->ReadTupleLength(wf_entity, "origin", 1, geom.pose.y );
      geom.pose.z = wf->ReadTupleLength(wf_entity, "origin", 2, geom.pose.z );
      geom.pose.a =  wf->ReadTupleAngle(wf_entity, "origin", 3, geom.pose.a );
      this->SetGeom( geom );
    }

  if( wf->PropertyExists( wf_entity, "size" ) )
    {
      stg_geom_t geom = GetGeom();
      geom.size.x = wf->ReadTupleLength(wf_entity, "size", 0, geom.size.x );
      geom.size.y = wf->ReadTupleLength(wf_entity, "size", 1, geom.size.y );
      geom.size.z = wf->ReadTupleLength(wf_entity, "size", 2, geom.size.z );
      this->SetGeom( geom );
    }

  if( wf->PropertyExists( wf_entity, "pose" ))
    {
      stg_pose_t pose = GetPose();
      pose.x = wf->ReadTupleLength(wf_entity, "pose", 0, pose.x );
      pose.y = wf->ReadTupleLength(wf_entity, "pose", 1, pose.y );
      pose.z = wf->ReadTupleLength(wf_entity, "pose", 2, pose.z );
      pose.a =  wf->ReadTupleAngle(wf_entity, "pose", 3, pose.a );
      this->SetPose( pose );
    }

  if( wf->PropertyExists( wf_entity, "velocity" ))
    {
      stg_velocity_t vel = GetVelocity();
      vel.x = wf->ReadTupleLength(wf_entity, "velocity", 0, vel.x );
      vel.y = wf->ReadTupleLength(wf_entity, "velocity", 1, vel.y );
      vel.z = wf->ReadTupleLength(wf_entity, "velocity", 2, vel.z );
      vel.a = wf->ReadTupleAngle(wf_entity, "velocity", 3,  vel.a );
      this->SetVelocity( vel );
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
			 stg_size_t bgsize = blockgroup.GetSize();

			 AddBlockRect(blockgroup.minx,blockgroup.miny, epsilon, bgsize.y, bgsize.z );	      
			 AddBlockRect(blockgroup.minx,blockgroup.miny, bgsize.x, epsilon, bgsize.z );	      
  			 AddBlockRect(blockgroup.minx,blockgroup.maxy-epsilon, bgsize.x, epsilon, bgsize.z );	      
  			 AddBlockRect(blockgroup.maxx-epsilon,blockgroup.miny, epsilon, bgsize.y, bgsize.z );	      
		  }     
    }	  

    if( wf->PropertyExists( wf_entity, "mass" ))
    this->SetMass( wf->ReadFloat(wf_entity, "mass", this->mass ));

  if( wf->PropertyExists( wf_entity, "fiducial_return" ))
    this->SetFiducialReturn( wf->ReadInt( wf_entity, "fiducial_return", this->fiducial_return ));

  if( wf->PropertyExists( wf_entity, "fiducial_key" ))
    this->SetFiducialKey( wf->ReadInt( wf_entity, "fiducial_key", this->fiducial_key ));

  if( wf->PropertyExists( wf_entity, "obstacle_return" ))
    this->SetObstacleReturn( wf->ReadInt( wf_entity, "obstacle_return", this->obstacle_return ));

  if( wf->PropertyExists( wf_entity, "ranger_return" ))
    this->SetRangerReturn( wf->ReadInt( wf_entity, "ranger_return", this->ranger_return ));

  if( wf->PropertyExists( wf_entity, "blob_return" ))
    this->SetBlobReturn( wf->ReadInt( wf_entity, "blob_return", this->blob_return ));

  if( wf->PropertyExists( wf_entity, "laser_return" ))
    this->SetLaserReturn( (stg_laser_return_t)wf->ReadInt(wf_entity, "laser_return", this->laser_return ));

  if( wf->PropertyExists( wf_entity, "gripper_return" ))
    this->SetGripperReturn( wf->ReadInt( wf_entity, "gripper_return", this->gripper_return ));

  if( wf->PropertyExists( wf_entity, "gui_nose" ))
    this->SetGuiNose( wf->ReadInt(wf_entity, "gui_nose", this->gui_nose ));

  if( wf->PropertyExists( wf_entity, "gui_grid" ))
    this->SetGuiGrid( wf->ReadInt(wf_entity, "gui_grid", this->gui_grid ));

  if( wf->PropertyExists( wf_entity, "gui_outline" ))
    this->SetGuiOutline( wf->ReadInt(wf_entity, "gui_outline", this->gui_outline ));

  if( wf->PropertyExists( wf_entity, "gui_movemask" ))
    this->SetGuiMask( wf->ReadInt(wf_entity, "gui_movemask", this->gui_mask ));

  if( wf->PropertyExists( wf_entity, "map_resolution" ))
    this->SetMapResolution( wf->ReadFloat(wf_entity, "map_resolution", this->map_resolution ));

  if( wf->PropertyExists( wf_entity, "ctrl" ))
    {
      char* lib = (char*)wf->ReadString(wf_entity, "ctrl", NULL );
		
      if( !lib )
		  puts( "Error - NULL library name" );
      else
		  LoadControllerModule( lib );
    }
  
  if( wf->PropertyExists( wf_entity, "say" ))
    this->Say( wf->ReadString(wf_entity, "say", NULL ));

  // call any type-specific load callbacks
  this->CallCallbacks( &this->hook_load );

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


void StgModel::Save( void )
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
  this->CallCallbacks( &this->hook_save );

  PRINT_DEBUG1( "Model \"%s\" saving complete.", token );
}


// todo - use GLib's portable module code here
void StgModel::LoadControllerModule( char* lib )
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



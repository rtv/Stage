#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <limits.h> 
#include <libgen.h> // for dirname()
#include <string.h>
#include "stage_internal.hh"

//#define DEBUG 1

#include <ltdl.h>

void FooInit( StgModel* );
void FooUpdate( StgModel* );


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
			//printf( "changed %s to %s\n", this->token, token );
			this->token = strdup( name );
			world->AddModel( this ); // add this name to the world's table
		}
		else
			PRINT_ERR1( "Name blank for model %s. Check your worldfile\n", this->token );

		// add this name to the table

	}

	//PRINT_WARN1( "%s::Load", token );

	if( wf->PropertyExists( wf_entity, "origin" ) )
	{
		stg_geom_t geom = GetGeom();
		geom.pose.x = wf->ReadTupleLength(wf_entity, "origin", 0, geom.pose.x );
		geom.pose.y = wf->ReadTupleLength(wf_entity, "origin", 1, geom.pose.y );
		geom.pose.a =  wf->ReadTupleAngle(wf_entity, "origin", 2, geom.pose.a );
		this->SetGeom( geom );
	}

	if( wf->PropertyExists( wf_entity, "origin4" ) )
	{
		stg_geom_t geom = GetGeom();
		geom.pose.x = wf->ReadTupleLength(wf_entity, "origin4", 0, geom.pose.x );
		geom.pose.y = wf->ReadTupleLength(wf_entity, "origin4", 1, geom.pose.y );
		geom.pose.z = wf->ReadTupleLength(wf_entity, "origin4", 2, geom.pose.z );
		geom.pose.a =  wf->ReadTupleAngle(wf_entity, "origin4", 3, geom.pose.a );
		this->SetGeom( geom );
	}

	if( wf->PropertyExists( wf_entity, "size" ) )
	{
		stg_geom_t geom = GetGeom();
		geom.size.x = wf->ReadTupleLength(wf_entity, "size", 0, geom.size.x );
		geom.size.y = wf->ReadTupleLength(wf_entity, "size", 1, geom.size.y );
		this->SetGeom( geom );
	}

	if( wf->PropertyExists( wf_entity, "size3" ) )
	{
		stg_geom_t geom = GetGeom();
		geom.size.x = wf->ReadTupleLength(wf_entity, "size3", 0, geom.size.x );
		geom.size.y = wf->ReadTupleLength(wf_entity, "size3", 1, geom.size.y );
		geom.size.z = wf->ReadTupleLength(wf_entity, "size3", 2, geom.size.z );
		this->SetGeom( geom );
	}

	if( wf->PropertyExists( wf_entity, "pose" ))
	{
		stg_pose_t pose = GetPose();
		pose.x = wf->ReadTupleLength(wf_entity, "pose", 0, pose.x );
		pose.y = wf->ReadTupleLength(wf_entity, "pose", 1, pose.y );
		pose.a =  wf->ReadTupleAngle(wf_entity, "pose", 2, pose.a );
		this->SetPose( pose );
	}

	if( wf->PropertyExists( wf_entity, "pose4" ))
	{
		stg_pose_t pose = GetPose();
		pose.x = wf->ReadTupleLength(wf_entity, "pose4", 0, pose.x );
		pose.y = wf->ReadTupleLength(wf_entity, "pose4", 1, pose.y );
		pose.z = wf->ReadTupleLength(wf_entity, "pose4", 2, pose.z );
		pose.a = wf->ReadTupleAngle( wf_entity, "pose4", 3,  pose.a );

		this->SetPose( pose );
	}

	if( wf->PropertyExists( wf_entity, "velocity" ))
	{
		stg_velocity_t vel = GetVelocity();
		vel.x = wf->ReadTupleLength(wf_entity, "velocity", 0, vel.x );
		vel.y = wf->ReadTupleLength(wf_entity, "velocity", 1, vel.y );
		vel.a = wf->ReadTupleAngle(wf_entity, "velocity", 3,  vel.a );
		this->SetVelocity( vel );

		if( vel.x || vel.y || vel.z || vel.a )
			world->StartUpdatingModel( this );
	}

	if( wf->PropertyExists( wf_entity, "velocity4" ))
	{
		stg_velocity_t vel = GetVelocity();
		vel.x = wf->ReadTupleLength(wf_entity, "velocity4", 0, vel.x );
		vel.y = wf->ReadTupleLength(wf_entity, "velocity4", 1, vel.y );
		vel.z = wf->ReadTupleLength(wf_entity, "velocity4", 2, vel.z );
		vel.a =  wf->ReadTupleAngle(wf_entity, "velocity4", 3,  vel.a );
		this->SetVelocity( vel );
	}

	if( wf->PropertyExists( wf_entity, "boundary" ))
	{
		this->SetBoundary( wf->ReadInt(wf_entity, "boundary", this->boundary  ));
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
		double red   = wf->ReadTupleFloat( wf_entity, "color_rgba", 0, 0);
		double green = wf->ReadTupleFloat( wf_entity, "color_rgba", 1, 0);
		double blue  = wf->ReadTupleFloat( wf_entity, "color_rgba", 2, 0);
		double alpha = wf->ReadTupleFloat( wf_entity, "color_rgba", 3, 0);

		this->SetColor( stg_color_pack( red, green, blue, alpha ));
	}  

	if( wf->PropertyExists( wf_entity, "bitmap" ) )
	{
		const char* bitmapfile = wf->ReadString( wf_entity, "bitmap", NULL );
		assert( bitmapfile );

		char full[_POSIX_PATH_MAX];

		if( bitmapfile[0] == '/' )
			strcpy( full, bitmapfile );
		else
		{
			char *tmp = strdup(wf->filename);
			snprintf( full, _POSIX_PATH_MAX,
					"%s/%s",  dirname(tmp), bitmapfile );
			free(tmp);
		}

		PRINT_DEBUG1( "attempting to load image %s", full );

		stg_rotrect_t* rects = NULL;
		unsigned int rect_count = 0;
		unsigned int width, height;
		if( stg_rotrects_from_image_file( full,
					&rects,
					&rect_count,
					&width, &height ) )
		{
			PRINT_ERR1( "failed to load rects from image file \"%s\"",
					full );
			return;
		}

		this->UnMap();
		this->ClearBlocks();

		//printf( "found %d rects\n", rect_count );

		if( rects && (rect_count > 0) )
		{
			//puts( "loading rects" );
			for( unsigned int r=0; r<rect_count; r++ )
				this->AddBlockRect( rects[r].pose.x, rects[r].pose.y, 
						rects[r].size.x, rects[r].size.y );

			if( this->boundary )
			{
				// add thin bounding blocks
				double epsilon = 0.01;	      
				this->AddBlockRect(0,0, epsilon, height );	      
				this->AddBlockRect(0,0, width, epsilon );	      
				this->AddBlockRect(0, height-epsilon, width, epsilon );
				this->AddBlockRect(width-epsilon,0, epsilon, height );
			}     

			StgBlock::ScaleList( this->blocks, &this->geom.size );	  
			this->Map();
			this->NeedRedraw();

			g_free( rects );
		}      

		//printf( "model %s block count %d\n",
		//      token, g_list_length( blocks ));
	}

	if( wf->PropertyExists( wf_entity, "blocks" ) )
	{
		int blockcount = wf->ReadInt( wf_entity, "blocks", -1 );

		this->UnMap();
		this->ClearBlocks();

		//printf( "expecting %d blocks\n", blockcount );

		char key[256];
		for( int l=0; l<blockcount; l++ )
		{	  	  
			snprintf(key, sizeof(key), "block[%d].points", l);
			int pointcount = wf->ReadInt(wf_entity,key,0);

			//printf( "expecting %d points in block %d\n",
			//pointcount, l );

			stg_point_t* pts = stg_points_create( pointcount );

			int p;
			for( p=0; p<pointcount; p++ )	      {
				snprintf(key, sizeof(key), "block[%d].point[%d]", l, p );

				pts[p].x = wf->ReadTupleLength(wf_entity, key, 0, 0);
				pts[p].y = wf->ReadTupleLength(wf_entity, key, 1, 0);

				//printf( "key %s x: %.2f y: %.2f\n",
				//      key, pt.x, pt.y );
			}

			// block Z axis
			snprintf(key, sizeof(key), "block[%d].z", l);

			stg_meters_t zmin = 
				wf->ReadTupleLength(wf_entity, key, 0, 0.0 );

			stg_meters_t zmax = 
				wf->ReadTupleLength(wf_entity, key, 1, 1.0 );

			// block color
			stg_color_t blockcol = this->color;
			bool inherit_color = true;

			snprintf(key, sizeof(key), "block[%d].color", l);

			const char* colorstr = wf->ReadString( wf_entity, key, NULL );
			if( colorstr )
			{
				blockcol = stg_lookup_color( colorstr );
				inherit_color = false;
			}

			this->AddBlock( pts, pointcount, zmin, zmax, blockcol, inherit_color );	

			stg_points_destroy( pts );
		}

		StgBlock::ScaleList( this->blocks, &this->geom.size );

		if( this->boundary )
		{
			// add thin bounding blocks
			double epsilon = 0.001;	      
			double width =  geom.size.x;
			double height = geom.size.y;
			this->AddBlockRect(-width/2.0, -height/2.0, epsilon, height );	      
			this->AddBlockRect(-width/2.0, -height/2.0, width, epsilon );	      
			this->AddBlockRect(-width/2.0, height/2.0-epsilon, width, epsilon );
			this->AddBlockRect(width/2.0-epsilon, -height/2.0, epsilon, height );
		}     

		this->Map();
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
	this->CallCallbacks( &this->load_hook );

	// MUST BE THE LAST THING LOADED
	if( wf->PropertyExists( wf_entity, "alwayson" ))
	{
		if( wf->ReadInt( wf_entity, "alwayson", 0) > 0 )
			Startup();
	}

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

	// right now we only save poses
	wf->WriteTupleLength( wf_entity, "pose", 0, this->pose.x);
	wf->WriteTupleLength( wf_entity, "pose", 1, this->pose.y);
	wf->WriteTupleAngle( wf_entity, "pose", 2, this->pose.a);

	wf->WriteTupleLength( wf_entity, "pose3", 0, this->pose.x);
	wf->WriteTupleLength( wf_entity, "pose3", 1, this->pose.y);
	wf->WriteTupleLength( wf_entity, "pose3", 2, this->pose.z);
	wf->WriteTupleAngle( wf_entity, "pose3", 3, this->pose.a);

	// call any type-specific save callbacks
	this->CallCallbacks( &this->save_hook );

	PRINT_DEBUG1( "Model \"%s\" saving complete.", token );
}


void StgModel::LoadControllerModule( char* lib )
{
	printf( "[Ctrl \"%s\"", lib );
	fflush(stdout);

	/* Initialise libltdl. */
	int errors = lt_dlinit();
	assert(errors==0);

	char* stagepath = getenv("STAGEPATH");
	if( stagepath == NULL )
		stagepath = ".";

	lt_dlsetsearchpath( stagepath );

	lt_dlhandle handle = NULL;

	if(( handle = lt_dlopenext( lib ) ))
	{
		printf( "]" );

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
}



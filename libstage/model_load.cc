#define _GNU_SOURCE
#include <limits.h> 
#include <libgen.h> // for dirname()
#include <string.h>
#include "stage_internal.hh"

//#define DEBUG 1

void StgModel::Load( void )
{  
  PRINT_DEBUG1( "Model \"%s\" loading...", token );

  Worldfile* wf = world->wf;
  
  if( wf->PropertyExists( this->id, "debug" ) )
    {
      PRINT_WARN2( "debug property specified for model %d  %s\n",
		   this->id, this->token );
      this->debug = wf->ReadInt( this->id, "debug", this->debug );
    }

  if( wf->PropertyExists( this->id, "name" ) )
    {
      char *name = (char*)wf->ReadString(this->id, "name", NULL );
      if( name )
	{
	  //printf( "changed %s to %s\n", this->token, token );
	  this->token = strdup( name );
	  world->AddModelName( this ); // add this name to the world's table
	}
      else
	PRINT_ERR1( "Name blank for model %s. Check your worldfile\n", this->token );

      // add this name to the table
      
    }

  //PRINT_WARN1( "%s::Load", token );

  if( wf->PropertyExists( this->id, "origin" ) )
    {
      stg_geom_t geom = GetGeom();
      geom.pose.x = wf->ReadTupleLength(this->id, "origin", 0, geom.pose.x );
      geom.pose.y = wf->ReadTupleLength(this->id, "origin", 1, geom.pose.y );
      geom.pose.a =  wf->ReadTupleAngle(this->id, "origin", 2, geom.pose.a );
      this->SetGeom( geom );
    }

  if( wf->PropertyExists( this->id, "origin4" ) )
    {
      stg_geom_t geom = GetGeom();
      geom.pose.x = wf->ReadTupleLength(this->id, "origin4", 0, geom.pose.x );
      geom.pose.y = wf->ReadTupleLength(this->id, "origin4", 1, geom.pose.y );
      geom.pose.z = wf->ReadTupleLength(this->id, "origin4", 2, geom.pose.z );
      geom.pose.a =  wf->ReadTupleAngle(this->id, "origin4", 3, geom.pose.a );
      this->SetGeom( geom );
    }
  
  if( wf->PropertyExists( this->id, "size" ) )
    {
      stg_geom_t geom = GetGeom();
      geom.size.x = wf->ReadTupleLength(this->id, "size", 0, geom.size.x );
      geom.size.y = wf->ReadTupleLength(this->id, "size", 1, geom.size.y );
      this->SetGeom( geom );
    }

  if( wf->PropertyExists( this->id, "size3" ) )
    {
      stg_geom_t geom = GetGeom();
      geom.size.x = wf->ReadTupleLength(this->id, "size3", 0, geom.size.x );
      geom.size.y = wf->ReadTupleLength(this->id, "size3", 1, geom.size.y );
      geom.size.z = wf->ReadTupleLength(this->id, "size3", 2, geom.size.z );      
      this->SetGeom( geom );
    }

  if( wf->PropertyExists( this->id, "pose" ))
    {
      stg_pose_t pose = GetPose();
      pose.x = wf->ReadTupleLength(this->id, "pose", 0, pose.x );
      pose.y = wf->ReadTupleLength(this->id, "pose", 1, pose.y ); 
      pose.a =  wf->ReadTupleAngle(this->id, "pose", 2, pose.a );
      this->SetPose( pose );
    }
  
  if( wf->PropertyExists( this->id, "pose4" ))
    {
      stg_pose_t pose = GetPose();
      pose.x = wf->ReadTupleLength(this->id, "pose4", 0, pose.x );
      pose.y = wf->ReadTupleLength(this->id, "pose4", 1, pose.y ); 
      pose.z = wf->ReadTupleLength(this->id, "pose4", 2, pose.z ); 
      pose.a = wf->ReadTupleAngle( this->id, "pose4", 3,  pose.a );
      
      this->SetPose( pose );
    }

  if( wf->PropertyExists( this->id, "velocity" ))
    {
      stg_velocity_t vel = GetVelocity();
      vel.x = wf->ReadTupleLength(this->id, "velocity", 0, vel.x );
      vel.y = wf->ReadTupleLength(this->id, "velocity", 1, vel.y );
      vel.a = wf->ReadTupleAngle(this->id, "velocity", 3,  vel.a );      
      this->SetVelocity( vel );
    }

  if( wf->PropertyExists( this->id, "velocity4" ))
    {
      stg_velocity_t vel = GetVelocity();
      vel.x = wf->ReadTupleLength(this->id, "velocity4", 0, vel.x );
      vel.y = wf->ReadTupleLength(this->id, "velocity4", 1, vel.y );
      vel.z = wf->ReadTupleLength(this->id, "velocity4", 2, vel.z );
      vel.a =  wf->ReadTupleAngle(this->id, "velocity4", 3,  vel.a );      
      this->SetVelocity( vel );
    }
  
  if( wf->PropertyExists( this->id, "boundary" ))
    {
      this->SetBoundary( wf->ReadInt(this->id, "boundary", this->boundary  ));  
    }	  

  if( wf->PropertyExists( this->id, "color" ))
    {      
      stg_color_t col = 0xFFFF0000; // red;  
      const char* colorstr = wf->ReadString( this->id, "color", NULL );
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
  
  if( wf->PropertyExists( this->id, "bitmap" ) )
    {
      const char* bitmapfile = wf->ReadString( this->id, "bitmap", NULL );
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
	      double epsilon = 0.001;	      
	      this->AddBlockRect(0,0, epsilon, height );	      
	      this->AddBlockRect(0,0, width, epsilon );	      
	      this->AddBlockRect(0, height-epsilon, width, epsilon );      
	      this->AddBlockRect(width-epsilon,0, epsilon, height ); 
	    }     
	  
	  StgBlock::ScaleList( this->blocks, &this->geom.size );	  
	  this->Map();
	  this->body_dirty = true;

	  g_free( rects );
	}      

      //printf( "model %s block count %d\n",
      //      token, g_list_length( blocks ));
    }
    
    if( wf->PropertyExists( this->id, "blocks" ) )
      {
	int blockcount = wf->ReadInt( this->id, "blocks", -1 );
	
	this->UnMap();
	this->ClearBlocks();

	//printf( "expecting %d blocks\n", blockcount );
	
	char key[256]; 
	for( int l=0; l<blockcount; l++ )
	  {	  	  
	    snprintf(key, sizeof(key), "block[%d].points", l);
	    int pointcount = wf->ReadInt(this->id,key,0);
	    
	    //printf( "expecting %d points in block %d\n",
	    //pointcount, l );
	    
	    stg_point_t* pts = stg_points_create( pointcount );
	    
	    int p;
	    for( p=0; p<pointcount; p++ )	      {
		snprintf(key, sizeof(key), "block[%d].point[%d]", l, p );
		
		pts[p].x = wf->ReadTupleLength(this->id, key, 0, 0);
		pts[p].y = wf->ReadTupleLength(this->id, key, 1, 0);
		
		//printf( "key %s x: %.2f y: %.2f\n",
		//      key, pt.x, pt.y );
	      }
	    
	    // block Z axis
	    snprintf(key, sizeof(key), "block[%d].z", l);
	    
	    stg_meters_t zmin = 
	      wf->ReadTupleLength(this->id, key, 0, 0.0 );

	    stg_meters_t zmax = 
	      wf->ReadTupleLength(this->id, key, 1, 1.0 );
	    
	    // block color
	    stg_color_t blockcol = this->color;
	    bool inherit_color = true;

	    snprintf(key, sizeof(key), "block[%d].color", l);

	    const char* colorstr = wf->ReadString( this->id, key, NULL );
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
    
 if( wf->PropertyExists( this->id, "mass" ))
    this->SetMass( wf->ReadFloat(this->id, "mass", this->mass ));
  
  if( wf->PropertyExists( this->id, "fiducial_return" ))
    this->SetFiducialReturn( wf->ReadInt( this->id, "fiducial_return", this->fiducial_return ));
  
  if( wf->PropertyExists( this->id, "fiducial_key" ))
      this->SetFiducialKey( wf->ReadInt( this->id, "fiducial_key", this->fiducial_key ));
  
  if( wf->PropertyExists( this->id, "obstacle_return" ))
    this->SetObstacleReturn( wf->ReadInt( this->id, "obstacle_return", this->obstacle_return ));
  
  if( wf->PropertyExists( this->id, "ranger_return" ))
    this->SetRangerReturn( wf->ReadInt( this->id, "ranger_return", this->ranger_return ));
  
  if( wf->PropertyExists( this->id, "blob_return" ))
    this->SetBlobReturn( wf->ReadInt( this->id, "blob_return", this->blob_return ));
  
  if( wf->PropertyExists( this->id, "laser_return" ))
    this->SetLaserReturn( (stg_laser_return_t)wf->ReadInt(this->id, "laser_return", this->laser_return ));
  
  if( wf->PropertyExists( this->id, "gripper_return" ))
    this->SetGripperReturn( wf->ReadInt( this->id, "gripper_return", this->gripper_return ));
  
  if( wf->PropertyExists( this->id, "gui_nose" ))
    this->SetGuiNose( wf->ReadInt(this->id, "gui_nose", this->gui_nose ));    
  
  if( wf->PropertyExists( this->id, "gui_grid" ))
    this->SetGuiGrid( wf->ReadInt(this->id, "gui_grid", this->gui_grid ));  
  
  if( wf->PropertyExists( this->id, "gui_outline" ))
    this->SetGuiOutline( wf->ReadInt(this->id, "gui_outline", this->gui_outline )); 
  
  if( wf->PropertyExists( this->id, "gui_movemask" ))
    this->SetGuiMask( wf->ReadInt(this->id, "gui_movemask", this->gui_mask ));
  
  if( wf->PropertyExists( this->id, "map_resolution" ))
    this->SetMapResolution( wf->ReadFloat(this->id, "map_resolution", this->map_resolution )); 
  
    // call any type-specific load callbacks
    this->CallCallbacks( &this->load );


    if( this->debug )
      printf( "Model \"%s\" is in debug mode\n", token ); 

    PRINT_DEBUG1( "Model \"%s\" loading complete", token );
}


void StgModel::Save( void )
{  
  PRINT_DEBUG1( "Model \"%s\" saving...", token );

  Worldfile* wf = world->wf;

  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
		this->token,
		this->pose.x,
		this->pose.y,
		this->pose.a );
  
  // right now we only save poses
  wf->WriteTupleLength( this->id, "pose", 0, this->pose.x);
  wf->WriteTupleLength( this->id, "pose", 1, this->pose.y);
  wf->WriteTupleAngle( this->id, "pose", 2, this->pose.a);

  wf->WriteTupleLength( this->id, "pose3", 0, this->pose.x);
  wf->WriteTupleLength( this->id, "pose3", 1, this->pose.y);
  wf->WriteTupleLength( this->id, "pose3", 2, this->pose.z);
  wf->WriteTupleAngle( this->id, "pose3", 3, this->pose.a);

  // call any type-specific save callbacks
  this->CallCallbacks( &this->save );

  PRINT_DEBUG1( "Model \"%s\" saving complete.", token );
}

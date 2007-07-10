#define _GNU_SOURCE
#include <limits.h> 
#include <libgen.h> // for dirname()

#include "model.hh"

void StgModel::Load( void )
{  
  if( wf->PropertyExists( this->id, "name" ) )
    {
      char *token = (char*)wf->ReadString(this->id, "name", NULL );
      if( token )
	{
	  //printf( "changed %s to %s\n", this->token, token );
	  strncpy( this->token, token, STG_TOKEN_MAX );
	}
      else
	PRINT_ERR1( "Name blank for model %s. Check your worldfile\n", this->token );
    }

  PRINT_WARN1( "%s::Load", token );

  if( wf->PropertyExists( this->id, "origin" ) )
    {
      stg_geom_t geom;
      this->GetGeom( &geom );

      geom.pose.x = wf->ReadTupleLength(this->id, "origin", 0, geom.pose.x );
      geom.pose.y = wf->ReadTupleLength(this->id, "origin", 1, geom.pose.y );
      geom.pose.z = wf->ReadTupleLength(this->id, "origin", 2, geom.pose.z );
      geom.pose.a = wf->ReadTupleAngle(this->id, "origin", 3, geom.pose.a );

      this->SetGeom( &geom );
    }

  if( wf->PropertyExists( this->id, "size" ) )
    {
      stg_geom_t geom;
      this->GetGeom( &geom );
      
      geom.size.x = wf->ReadTupleLength(this->id, "size", 0, geom.size.x );
      geom.size.y = wf->ReadTupleLength(this->id, "size", 1, geom.size.y );
      geom.size.z = wf->ReadTupleLength(this->id, "size", 2, geom.size.z );
      
      this->SetGeom( &geom );
    }

  if( wf->PropertyExists( this->id, "pose" ))
    {
      stg_pose_t pose;
      this->GetPose( &pose );

      pose.x = wf->ReadTupleLength(this->id, "pose", 0, pose.x );
      pose.y = wf->ReadTupleLength(this->id, "pose", 1, pose.y ); 
      pose.z = wf->ReadTupleLength(this->id, "pose", 2, pose.z ); 
      pose.a = wf->ReadTupleAngle(this->id, "pose", 3,  pose.a );

      this->SetPose( &pose );
    }

  if( wf->PropertyExists( this->id, "velocity" ))
    {
      stg_velocity_t vel;
      this->GetVelocity( &vel );
      
      vel.x = wf->ReadTupleLength(this->id, "velocity", 0, vel.x );
      vel.y = wf->ReadTupleLength(this->id, "velocity", 1, vel.y );
      vel.z = wf->ReadTupleLength(this->id, "velocity", 2, vel.z );
      vel.a = wf->ReadTupleAngle(this->id, "velocity", 3,  vel.a );      

      this->SetVelocity( &vel );
    }
  
  if( wf->PropertyExists( this->id, "boundary" ))
      this->SetBoundary( wf->ReadInt(this->id, "boundary", this->boundary  ));
  
  if( wf->PropertyExists( this->id, "color" ))
    {      
      stg_color_t col = 0xFF0000; // red;  
      const char* colorstr = wf->ReadString( this->id, "color", NULL );
      if( colorstr )
	{
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
      
#ifdef DEBUG
      printf( "attempting to load image %s\n",
	      full );
#endif
      
      stg_rotrect_t* rects = NULL;
      int rect_count = 0;      
      int width, height;
      if( stg_rotrects_from_image_file( full,
					&rects,
					&rect_count,
					&width, &height ) )
	{
	  PRINT_ERR1( "failed to load rects from image file \"%s\"",
		      full );
	  return;
	}
      
      stg_block_list_destroy( this->blocks );
      this->blocks = NULL;

      //printf( "found %d rects\n", rect_count );
      
      if( rects && (rect_count > 0) )
	{
	  //puts( "loading rects" );
	  for( int r=0; r<rect_count; r++ )
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
	  
	  stg_block_list_scale( this->blocks, &this->geom.size );	  
	}      
    }
    
    if( wf->PropertyExists( this->id, "blocks" ) )
      {
	int blockcount = wf->ReadInt( this->id, "blocks", -1 );
	
	stg_block_list_destroy( this->blocks );
	this->blocks = NULL;

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
	
	stg_block_list_scale( this->blocks, &this->geom.size );

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
    this->SetLaserReturn( wf->ReadInt(this->id, "laser_return", this->laser_return ));
  
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
}


void StgModel::Save( void )
{  
  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
		this->token,
		this->pose->x,
		this->pose->y,
		this->pose->a );
  
  // right now we only save poses
  wf->WriteTupleLength( this->id, "pose", 0, this->pose.x);
  wf->WriteTupleLength( this->id, "pose", 1, this->pose.y);
  wf->WriteTupleLength( this->id, "pose", 2, this->pose.z);
  wf->WriteTupleAngle( this->id, "pose", 3, this->pose.a);

  // call any type-specific save callbacks
  this->CallCallbacks( &this->save );
}

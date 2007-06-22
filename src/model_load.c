#define _GNU_SOURCE
#include <limits.h> 
#include "stage_internal.h"

// defined in model.c
void polygon_local_bounds_calc_cb( stg_polygon_t* poly, gpointer unused );
void polygon_global_bounds_calc_cb( stg_polygon_t* poly, gpointer unused );

void stg_model_add_block_rect( stg_model_t* mod,  
			       double x, double y, 
			       double width, double height );


void stg_model_load( stg_model_t* mod )
{
  //const char* typestr = wf_read_string( mod->id, "type", NULL );
  //PRINT_MSG2( "Model %p is type %s", mod, typestr );
  assert(mod);
  
  if( wf_property_exists( mod->id, "name" ) )
    {
      char *token = (char*)wf_read_string(mod->id, "name", NULL );
      if( token )
	{
	  //printf( "changed %s to %s\n", mod->token, token );
	  strncpy( mod->token, token, STG_TOKEN_MAX );
	}
      else
	PRINT_ERR1( "Name blank for model %s. Check your worldfile\n", mod->token );
    }

  if( wf_property_exists( mod->id, "origin" ) )
    {
      stg_geom_t geom;
      stg_model_get_geom( mod, &geom );

      geom.pose.x = wf_read_tuple_length(mod->id, "origin", 0, geom.pose.x );
      geom.pose.y = wf_read_tuple_length(mod->id, "origin", 1, geom.pose.y );
      geom.pose.z = wf_read_tuple_length(mod->id, "origin", 2, geom.pose.z );
      geom.pose.a = wf_read_tuple_angle(mod->id, "origin", 3, geom.pose.a );

      stg_model_set_geom( mod, &geom );
    }

  if( wf_property_exists( mod->id, "size" ) )
    {
      stg_geom_t geom;
      stg_model_get_geom( mod, &geom );
      
      geom.size.x = wf_read_tuple_length(mod->id, "size", 0, geom.size.x );
      geom.size.y = wf_read_tuple_length(mod->id, "size", 1, geom.size.y );
      geom.size.z = wf_read_tuple_length(mod->id, "size", 2, geom.size.z );
      
      stg_model_set_geom( mod, &geom );
    }

  if( wf_property_exists( mod->id, "pose" ))
    {
      stg_pose_t pose;
      stg_model_get_pose( mod, &pose );
      
      pose.x = wf_read_tuple_length(mod->id, "pose", 0, pose.x );
      pose.y = wf_read_tuple_length(mod->id, "pose", 1, pose.y ); 
      pose.z = wf_read_tuple_length(mod->id, "pose", 2, pose.z ); 
      pose.a = wf_read_tuple_angle(mod->id, "pose", 3,  pose.a );

      stg_model_set_pose( mod, &pose );
    }

  if( wf_property_exists( mod->id, "velocity" ))
    {
      stg_velocity_t vel;
      stg_model_get_velocity( mod, &vel );
      
      vel.x = wf_read_tuple_length(mod->id, "velocity", 0, vel.x );
      vel.y = wf_read_tuple_length(mod->id, "velocity", 1, vel.y );
      vel.z = wf_read_tuple_length(mod->id, "velocity", 2, vel.z );
      vel.a = wf_read_tuple_angle(mod->id, "velocity", 3,  vel.a );      

      stg_model_set_velocity( mod, &vel );
    }
  
  if( wf_property_exists( mod->id, "mass" ))
    {
      stg_kg_t mass = wf_read_float(mod->id, "mass", mod->mass );      
      stg_model_set_mass( mod, mass );
    }
  
  if( wf_property_exists( mod->id, "fiducial_return" ))
    {
      int fid = 
	wf_read_int( mod->id, "fiducial_return", mod->fiducial_return );     
      stg_model_set_fiducial_return( mod, fid  );
    }

  if( wf_property_exists( mod->id, "fiducial_key" ))
    {
      int fkey = 
	wf_read_int( mod->id, "fiducial_key", mod->fiducial_key );     
      stg_model_set_fiducial_key( mod, fkey );
    }
  
  if( wf_property_exists( mod->id, "obstacle_return" ))
    {
      int obs = 
	wf_read_int( mod->id, "obstacle_return", mod->obstacle_return );
      stg_model_set_obstacle_return( mod, obs  );
    }
  
  if( wf_property_exists( mod->id, "ranger_return" ))
    {
      int rng = 
	wf_read_int( mod->id, "ranger_return", mod->ranger_return );
      stg_model_set_ranger_return( mod, rng );
    }
  
  if( wf_property_exists( mod->id, "blob_return" ))
    {
      int blb = 
	wf_read_int( mod->id, "blob_return", mod->blob_return );      
      stg_model_set_blob_return( mod, blb );
    }
  
  if( wf_property_exists( mod->id, "laser_return" ))
    {
      int lsr = 
	wf_read_int(mod->id, "laser_return", mod->laser_return );      
      stg_model_set_laser_return( mod, lsr );
    }
  
  if( wf_property_exists( mod->id, "gripper_return" ))
    {
      int grp = 
	wf_read_int( mod->id, "gripper_return", mod->gripper_return );
      stg_model_set_gripper_return( mod, grp );
    }
  
  if( wf_property_exists( mod->id, "boundary" ))
    {
      int bdy =  
	wf_read_int(mod->id, "boundary", mod->boundary  ); 
      stg_model_set_boundary( mod, bdy );
    }
  
  if( wf_property_exists( mod->id, "color" ))
    {      
      stg_color_t col = 0xFF0000; // red;  
      const char* colorstr = wf_read_string( mod->id, "color", NULL );
      if( colorstr )
	{
	  col = stg_lookup_color( colorstr );  
	  stg_model_set_color( mod, col );
	}
    }      
  
  if( wf_property_exists( mod->id, "bitmap" ) )
    {
      const char* bitmapfile = wf_read_string( mod->id, "bitmap", NULL );
      assert( bitmapfile );

      char full[_POSIX_PATH_MAX];
      
      if( bitmapfile[0] == '/' )
	strcpy( full, bitmapfile );
      else
	{
	  char *tmp = strdup(wf_get_filename());
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
      
      stg_block_list_destroy( mod->blocks );
      mod->blocks = NULL;

      printf( "found %d rects\n", rect_count );
      
      if( rects && (rect_count > 0) )
	{
	  puts( "loading rects" );

	  stg_point_t pts[4];	  
	  size_t r;
	  for( r=0; r<rect_count; r++ )
	    {  
	      pts[0].x = rects[r].pose.x;
	      pts[0].y = rects[r].pose.y;
	      pts[1].x = rects[r].pose.x + rects[r].size.x;
	      pts[1].y = rects[r].pose.y;
	      pts[2].x = rects[r].pose.x + rects[r].size.x;
	      pts[2].y = rects[r].pose.y + rects[r].size.y;
	      pts[3].x = rects[r].pose.x;
	      pts[3].y = rects[r].pose.y + rects[r].size.y;
	      
	      stg_model_add_block( mod, pts, 4 );	      
  	    }

	  if( mod->boundary )
	    {
	      // add thin bounding blocks
	      double epsilon = 0.001;	      
	      stg_model_add_block_rect( mod, 0,0, epsilon, height );	      
	      stg_model_add_block_rect( mod, 0,0, width, epsilon );	      
	      stg_model_add_block_rect( mod, 0, height-epsilon, width, epsilon );      
	      stg_model_add_block_rect( mod, width-epsilon,0, epsilon, height ); 
	    }     
	  
	  stg_block_list_scale( mod->blocks, &mod->geom.size );	  
	}      
    }
    
    if( wf_property_exists( mod->id, "blocks" ) )
      {
	int blockcount = wf_read_int( mod->id, "blocks", -1 );
	
	stg_block_list_destroy( mod->blocks );
	mod->blocks = NULL;

	printf( "expecting %d blocks\n", blockcount );
	
	char key[256]; 
	int l;
	for(l=0; l<blockcount; l++ )
	  {	  	  
	    snprintf(key, sizeof(key), "block[%d].points", l);
	    int pointcount = wf_read_int(mod->id,key,0);
	    
	    printf( "expecting %d points in block %d\n",
	      pointcount, l );
	    
	    stg_point_t* pts = stg_points_create( pointcount );
	    
	    int p;
	    for( p=0; p<pointcount; p++ )	      {
		snprintf(key, sizeof(key), "block[%d].point[%d]", l, p );
		
		pts[p].x = wf_read_tuple_length(mod->id, key, 0, 0);
		pts[p].y = wf_read_tuple_length(mod->id, key, 1, 0);
		
		//printf( "key %s x: %.2f y: %.2f\n",
		//      key, pt.x, pt.y );
	      }
	    
/* 	    // block height */
/* 	    double height = mod->geom.size.z; */
/* 	    snprintf(key, sizeof(key), "block[%d].height", l); */
/* 	    if( wf_property_exists( mod->id, key ) ) */
/* 	      height = wf_read_length(mod->id, key, height); */
	    
/* 	    // block color */
/* 	    stg_color_t color = mod->color; */
/* 	    snprintf(key, sizeof(key), "block[%d].color", l); */
/* 	    if( wf_property_exists( mod->id, key ) ) */
/* 	      color = stg_lookup_color( wf_read_string(mod->id, key, NULL ));  */
	    
	    stg_model_add_block( mod,
				 pts,
				 pointcount );	
	    
	    stg_points_destroy( pts );
	  }
	
	stg_block_list_scale( mod->blocks, &mod->geom.size );
      }

    
  int gui_nose = wf_read_int(mod->id, "gui_nose", mod->gui_nose );  
  stg_model_set_gui_nose( mod, gui_nose );
  
  int gui_grid = wf_read_int(mod->id, "gui_grid", mod->gui_grid );  
  stg_model_set_gui_grid( mod, gui_grid );
  
  int gui_outline = wf_read_int(mod->id, "gui_outline", mod->gui_outline );  
  stg_model_set_gui_outline( mod, gui_outline );
  
  stg_movemask_t gui_movemask = wf_read_int(mod->id, "gui_movemask", mod->gui_mask );  
  stg_model_set_gui_mask( mod, gui_movemask );
    
  stg_meters_t mres = wf_read_float(mod->id, "map_resolution", mod->map_resolution );  
  stg_model_set_map_resolution( mod, mres );
  
  // call any type-specific load callbacks
  model_call_callbacks( mod, &mod->load );
}


void stg_model_save( stg_model_t* mod )
{
  
  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
		mod->token,
		mod->pose->x,
		mod->pose->y,
		mod->pose->a );
  
  // right now we only save poses
  wf_write_tuple_length( mod->id, "pose", 0, mod->pose.x);
  wf_write_tuple_length( mod->id, "pose", 1, mod->pose.y);
  wf_write_tuple_length( mod->id, "pose", 2, mod->pose.z);
  wf_write_tuple_angle( mod->id, "pose", 3, mod->pose.a);

  // call any type-specific save callbacks
  model_call_callbacks( mod, &mod->save );
}

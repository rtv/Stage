#define _GNU_SOURCE
#include <limits.h> 
#include "stage_internal.h"

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
      geom.pose.x = wf_read_tuple_length(mod->id, "origin", 0, mod->geom.pose.x );
      geom.pose.y = wf_read_tuple_length(mod->id, "origin", 1, mod->geom.pose.y );
      geom.pose.a = wf_read_tuple_length(mod->id, "origin", 2, mod->geom.pose.a );
      geom.size.x = mod->geom.size.x;
      geom.size.y = mod->geom.size.y;
      stg_model_set_geom( mod, &geom );
    }
  
  if( wf_property_exists( mod->id, "size" ) )
    {
      stg_geom_t geom;
      geom.pose.x = mod->geom.pose.x;
      geom.pose.y = mod->geom.pose.y;
      geom.pose.a = mod->geom.pose.a;
      geom.size.x = wf_read_tuple_length(mod->id, "size", 0, mod->geom.size.x );
      geom.size.y = wf_read_tuple_length(mod->id, "size", 1, mod->geom.size.y );
      stg_model_set_geom( mod, &geom );
    }
  
  if( wf_property_exists( mod->id, "pose" ))
    {
      stg_pose_t pose;
      pose.x = wf_read_tuple_length(mod->id, "pose", 0, mod->pose.x );
      pose.y = wf_read_tuple_length(mod->id, "pose", 1, mod->pose.y ); 
      pose.a = wf_read_tuple_angle(mod->id, "pose", 2,  mod->pose.a );
      stg_model_set_pose( mod, &pose );
    }
  
  if( wf_property_exists( mod->id, "velocity" ))
    {
      stg_velocity_t vel;
      vel.x = wf_read_tuple_length(mod->id, "velocity", 0, mod->velocity.x );
      vel.y = wf_read_tuple_length(mod->id, "velocity", 1, mod->velocity.y );
      vel.a = wf_read_tuple_angle(mod->id, "velocity", 2,  mod->velocity.a );      
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

  if( wf_property_exists( mod->id, "audio_return" ))
    {
      int aud = 
	wf_read_int( mod->id, "audio_return", mod->audio_return );
      stg_model_set_audio_return( mod, aud );
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
  
  //size_t polydata = 0;
  stg_polygon_t* polys = NULL;
  size_t polycount = -1 ;//polydata / sizeof(stg_polygon_t);;
  
  const char* bitmapfile = wf_read_string( mod->id, "bitmap", NULL );
  if( bitmapfile )
    {
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
      
      polys = stg_polygons_from_image_file( full, &polycount );
      
      if( ! polys )
	PRINT_ERR1( "Failed to load polygons from image file \"%s\"", full );
    }
  
  int wf_polycount = wf_read_int( mod->id, "polygons", 0 );
  if( wf_polycount > 0 )
    {
      polycount = wf_polycount;
      //printf( "expecting %d polygons\n", polycount );
      
      char key[256];
      polys = stg_polygons_create( polycount );
      int l;
      for(l=0; l<polycount; l++ )
	{	  	  
	  snprintf(key, sizeof(key), "polygon[%d].points", l);
	  int pointcount = wf_read_int(mod->id,key,0);
	  
	  //printf( "expecting %d points in polygon %d\n",
	  //  pointcount, l );
	  
	  int p;
	  for( p=0; p<pointcount; p++ )
	    {
	      snprintf(key, sizeof(key), "polygon[%d].point[%d]", l, p );
	      
	      stg_point_t pt;	      
	      pt.x = wf_read_tuple_length(mod->id, key, 0, 0);
	      pt.y = wf_read_tuple_length(mod->id, key, 1, 0);

	      //printf( "key %s x: %.2f y: %.2f\n",
	      //      key, pt.x, pt.y );
	      
	      // append the point to the polygon
	      stg_polygon_append_points( &polys[l], &pt, 1 );
	    }
	}
    }
 
  // load properties of any polygons that we have
  for( int p=0; p<mod->polygons_count; p++ )
    {  
      char key[256];
      snprintf(key, sizeof(key), "polygon[%d].fill", p);
      
      if( wf_property_exists( mod->id, key ) )
	{
	  mod->polygons[p].unfilled = ! wf_read_int(mod->id, key, 1 );
	  //printf( "Set unfilled state of polygon[%d] to %d\n",
	  //  p, mod->polygons[p].unfilled );
	}
    }

  // if we created any polygons
  if( polycount != -1 )
    {
      if( polycount == 0 )
	PRINT_WARN1( "model \"%s\" has zero polygons", mod->token );
      
      stg_model_set_polygons( mod, polys, polycount );
    }

  int wf_linecount = wf_read_int( mod->id, "polylines", 0 );
  if( wf_linecount > 0 )
    {
      char key[256];
      stg_polyline_t *lines = calloc( sizeof(stg_polyline_t), wf_linecount );
      int l;

      //stg_polyline_print( &lines[0] );

      for(l=0; l<wf_linecount; l++ )
	{	  	  
	  //stg_polyline_print( &lines[l] );

	  snprintf(key, sizeof(key), "polyline[%d].points", l);
	  int pointcount = wf_read_int(mod->id,key,0);
	  
	  //printf( "expecting %d points in polyline %d\n",
	  //  pointcount, l );
	  
	  lines[l].points = calloc( sizeof(stg_point_t), pointcount );
	  lines[l].points_count = pointcount;
	  
	  int p;
	  for( p=0; p<pointcount; p++ )
	    {
	      snprintf(key, sizeof(key), "polyline[%d].point[%d]", l, p );
	      
	      stg_point_t pt;	      
	      pt.x = wf_read_tuple_length(mod->id, key, 0, 0);
	      pt.y = wf_read_tuple_length(mod->id, key, 1, 0);
	      
	      //printf( "key %s x: %.2f y: %.2f\n",
	      //      key, pt.x, pt.y );
	      
	      lines[l].points[p].x = wf_read_tuple_length(mod->id, key, 0, 0);
	      lines[l].points[p].y = wf_read_tuple_length(mod->id, key, 1, 0);
	      
	      //puts( "after read" );
	      //stg_polyline_print( &lines[l] );
	    }
	}

      stg_model_set_polylines( mod, lines, wf_linecount );
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
  
  // if a type-specific load callback has been set
  if( mod->f_load )
    mod->f_load( mod ); // call the load function
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
  wf_write_tuple_angle( mod->id, "pose", 2, mod->pose.a);
}

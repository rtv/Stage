
#include "stage.h"
#include "worldfile.hh"

static CWorldFile wf;

// create a world containing a passel of Stage models based on the
// worldfile

//void stg_world_save( stg_world_t* world, CWorldFile* wfp )
//{

void stg_client_save( stg_client_t* cli )
{
  if( cli->callback_save )(*cli->callback_save)();

  PRINT_MSG1( "Stage client: saving worldfile \"%s\"\n", wf.filename );
  wf.Save(NULL);
}  

void stg_client_load( stg_client_t* cli )
{
  if( cli->callback_load )(*cli->callback_load)();
  
  PRINT_WARN( "LOAD NOT YET IMPLEMENTED" );
  //wf.Load();
}  


stg_world_t* stg_client_worldfile_load( stg_client_t* client, 
					char* worldfile_path )
{
  wf.Load( worldfile_path );
  
  int section = 0;
      
  const char* world_name =
    wf.ReadString(section, "name", (char*)"Player world" );
  
  double resolution = 
    wf.ReadFloat(0, "resolution", 0.02 ); // 2cm default
  resolution = 1.0 / resolution; // invert res to ppm
  
  stg_msec_t interval_real = wf.ReadInt(section, "interval_real", 100 );
  stg_msec_t interval_sim = wf.ReadInt(section, "interval_sim", 100 );
      
  //printf( "interval sim %lu   interval real %lu\n",
  //      interval_sim, interval_real );

  // create a single world
  stg_world_t* world = 
    stg_client_createworld( client, 
			    0,
			    stg_token_create( world_name, STG_T_NUM, 99 ),
			    resolution, interval_sim, interval_real );
  
  if( world == NULL )
    return NULL; // failure
  
  // create a special model for the background
  stg_token_t* token = stg_token_create( "root", STG_T_NUM, 99 );
  stg_model_t* root = stg_world_createmodel( world, NULL, 0, token );
      
  stg_size_t sz;
  sz.x = wf.ReadTupleLength(section, "size", 0, 10.0 );
  sz.y = wf.ReadTupleLength(section, "size", 1, 10.0 );
  stg_model_prop_with_data( root, STG_PROP_SIZE, &sz, sizeof(sz) );
      
  stg_pose_t pose;
  pose.x = sz.x/2.0;
  pose.y = sz.y/2.0;
  pose.a = 0.0;
  stg_model_prop_with_data( root, STG_PROP_POSE, &pose, sizeof(pose) );
      
  stg_movemask_t mm = 0;
  stg_model_prop_with_data( root, STG_PROP_MOVEMASK, &mm, sizeof(mm) ); 
      
  const char* colorstr = wf.ReadString(section, "color", "black" );
  stg_color_t color = stg_lookup_color( colorstr );
  stg_model_prop_with_data( root, STG_PROP_COLOR, &color,sizeof(color));
      
  const char* bitmapfile = wf.ReadString(section, "bitmap", NULL );
  if( bitmapfile )
    {
      stg_rotrect_t* rects = NULL;
      int num_rects = 0;
      stg_load_image( bitmapfile, &rects, &num_rects );
	  
      // convert rects to an array of lines
      int num_lines = 4 * num_rects;
      stg_line_t* lines = stg_rects_to_lines( rects, num_rects );
      stg_normalize_lines( lines, num_lines );
      stg_scale_lines( lines, num_lines, sz.x, sz.y );
      stg_translate_lines( lines, num_lines, -sz.x/2.0, -sz.y/2.0 );
	  
      stg_model_prop_with_data( root, STG_PROP_LINES, 
				lines, num_lines * sizeof(stg_line_t ));
	  
      free( lines );
    }
      
  stg_bool_t boundary;
  boundary = wf.ReadInt(section, "boundary", 1 );
  stg_model_prop_with_data(root, STG_PROP_BOUNDARY, &boundary,sizeof(boundary));
      
  stg_bool_t grid;
  grid = wf.ReadInt(section, "grid", 1 );
  stg_model_prop_with_data(root, STG_PROP_GRID, &grid, sizeof(grid) );

  // Iterate through sections and create client-side models
  for (int section = 1; section < wf.GetEntityCount(); section++)
    {
      // a model has the macro name that defined it as it's name
      // unlesss a name is explicitly defined.  TODO - count instances
      // of macro names so we can autogenerate unique names
      const char *default_namestr = wf.GetEntityType(section);      
      const char *namestr = wf.ReadString(section, "name", default_namestr);
      stg_token_t* token = stg_token_create( namestr, STG_T_NUM, 99 );
      stg_model_t* mod = stg_world_createmodel( world, NULL, section, token );
      //stg_world_addmodel( world, mod );

      stg_pose_t pose;
      pose.x = wf.ReadTupleLength(section, "pose", 0, 0);
      pose.y = wf.ReadTupleLength(section, "pose", 1, 0);
      pose.a = wf.ReadTupleAngle(section, "pose", 2, 0);      
      stg_model_prop_with_data( mod, STG_PROP_POSE, &pose, sizeof(pose) );
      
      stg_size_t sz;
      sz.x = wf.ReadTupleLength(section, "size", 0, 0.4 );
      sz.y = wf.ReadTupleLength(section, "size", 1, 0.4 );
      stg_model_prop_with_data( mod, STG_PROP_SIZE, &sz, sizeof(sz) );
      
      stg_bool_t nose;
      nose = wf.ReadInt( section, "nose", 0 );
      stg_model_prop_with_data( mod, STG_PROP_NOSE, &nose, sizeof(nose) );

      stg_bool_t grid;
      grid = wf.ReadInt( section, "grid", 0 );
      stg_model_prop_with_data( mod, STG_PROP_GRID, &grid, sizeof(grid) );
      
      stg_bool_t boundary;
      boundary = wf.ReadInt( section, "boundary", 0 );
      stg_model_prop_with_data( mod, STG_PROP_BOUNDARY, &boundary,sizeof(boundary));
      
      stg_geom_t lgeom;
      lgeom.pose.x = wf.ReadTupleLength(section, "laser_geom", 0, 0);
      lgeom.pose.y = wf.ReadTupleLength(section, "laser_geom", 1, 0);
      lgeom.pose.a = wf.ReadTupleAngle(section, "laser_geom", 2, 0);
      lgeom.size.x = wf.ReadTupleLength(section, "laser_geom", 3, 0);
      lgeom.size.y = wf.ReadTupleLength(section, "laser_geom", 4, 0);
      stg_model_prop_with_data( mod, STG_PROP_LASERGEOM, &lgeom,sizeof(lgeom));

      stg_laser_config_t lconf;
      lconf.range_min = wf.ReadTupleLength(section, "laser", 0, 0);
      lconf.range_max = wf.ReadTupleLength(section, "laser", 1, 8);
      lconf.fov = wf.ReadTupleAngle(section, "laser", 2, 180);
      lconf.samples = (int)wf.ReadTupleFloat(section, "laser", 3, 180);
      
      stg_model_prop_with_data( mod, STG_PROP_LASERCONFIG, &lconf,sizeof(lconf));
  
      const char* colorstr = wf.ReadString( section, "color", "red" );
      if( colorstr )
	{
	  stg_color_t color = stg_lookup_color( colorstr );
	  PRINT_DEBUG2( "stage color %s = %X", colorstr, color );
	  stg_model_prop_with_data( mod, STG_PROP_COLOR, &color,sizeof(color));
	}

      const char* bitmapfile = wf.ReadString( section, "bitmap", NULL );
      if( bitmapfile )
	{
	  stg_rotrect_t* rects = NULL;
	  int num_rects = 0;
	  stg_load_image( bitmapfile, &rects, &num_rects );
	  
	  // convert rects to an array of lines
	  int num_lines = 4 * num_rects;
	  stg_line_t* lines = stg_rects_to_lines( rects, num_rects );
	  stg_normalize_lines( lines, num_lines );
	  stg_scale_lines( lines, num_lines, sz.x, sz.y );
	  stg_translate_lines( lines, num_lines, -sz.x/2.0, -sz.y/2.0 );
	  
	  stg_model_prop_with_data( mod, STG_PROP_LINES, 
				    lines, num_lines * sizeof(stg_line_t ));
	  	  
	  free( lines );
	  
	}

      // Load the geometry of a ranger array
      int scount = wf.ReadInt( section, "scount", 0);
      if (scount > 0)
	{
	  char key[256];
	  stg_ranger_config_t* configs = (stg_ranger_config_t*)
	    calloc( sizeof(stg_ranger_config_t), scount );

	  int i;
	  for(i = 0; i < scount; i++)
	    {
	      snprintf(key, sizeof(key), "spose[%d]", i);
	      configs[i].pose.x = wf.ReadTupleLength(section, key, 0, 0);
	      configs[i].pose.y = wf.ReadTupleLength(section, key, 1, 0);
	      configs[i].pose.a = wf.ReadTupleAngle(section, key, 2, 0);

	      snprintf(key, sizeof(key), "ssize[%d]", i);
	      configs[i].size.x = wf.ReadTupleLength(section, key, 0, 0.01);
	      configs[i].size.y = wf.ReadTupleLength(section, key, 1, 0.05);

	      snprintf(key, sizeof(key), "sbounds[%d]", i);
	      configs[i].bounds_range.min = 
		wf.ReadTupleLength(section, key, 0, 0);
	      configs[i].bounds_range.max = 
		wf.ReadTupleLength(section, key, 1, 5.0);

	      snprintf(key, sizeof(key), "sfov[%d]", i);
	      configs[i].fov = wf.ReadAngle(section, key, 30 );
	    }
	  
	  PRINT_DEBUG1( "loaded %d ranger configs", scount );	  
	  stg_model_prop_with_data( mod, STG_PROP_RANGERCONFIG,
				    configs, scount * sizeof(stg_ranger_config_t) );

	  free( configs );
	}
      
      
      int linecount = wf.ReadInt( section, "line_count", 0 );
      if( linecount > 0 )
	{
	  char key[256];
	  stg_line_t* lines = (stg_line_t*)calloc( sizeof(stg_line_t), linecount );
	  int l;
	  for(l=0; l<linecount; l++ )
	    {
	      snprintf(key, sizeof(key), "line[%d]", l);

	      lines[l].x1 = wf.ReadTupleLength(section, key, 0, 0);
	      lines[l].y1 = wf.ReadTupleLength(section, key, 1, 0);
	      lines[l].x2 = wf.ReadTupleLength(section, key, 2, 0);
	      lines[l].y2 = wf.ReadTupleLength(section, key, 3, 0);	      
	    }
	  
	  // printf( "NOTE: loaded line %d/%d (%.2f,%.2f - %.2f,%.2f)\n",
	  //      l, linecount, 
	  //      lines[l].x1, lines[l].y1, 
	  //      lines[l].x2, lines[l].y2 ); 
	  
	  stg_model_prop_with_data( mod, STG_PROP_LINES,
				    lines, linecount * sizeof(stg_line_t) );
	  
	  free( lines );
	}

      stg_velocity_t vel;
      vel.x = wf.ReadTupleLength(section, "velocity", 0, 0);
      vel.y = wf.ReadTupleLength(section, "velocity", 1, 0);
      vel.a = wf.ReadTupleAngle(section, "velocity", 2, 0);      
      stg_model_prop_with_data( mod, STG_PROP_VELOCITY, &vel, sizeof(vel) );

      
    }

  return world;
}


#include "stage.h"
#include "worldfile.hh"

// this file is a hacky use of the old C++ worldfile code. It will
// fade away as we move to a new worldfile implementation. 

static CWorldFile wf;


void stg_model_save( stg_model_t* model, CWorldFile* worldfile )
{
  stg_pose_t pose;
  if( stg_model_prop_get( model, STG_PROP_POSE, &pose,sizeof(pose)))
    PRINT_ERR( "error requesting STG_PROP_POSE" );
  
  // right noe we only save poses
  worldfile->WriteTupleLength( model->section, "pose", 0, pose.x);
  worldfile->WriteTupleLength( model->section, "pose", 1, pose.y);
  worldfile->WriteTupleAngle( model->section, "pose", 2, pose.a);
}

void stg_model_save_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_save( (stg_model_t*)data, (CWorldFile*)user );
}

void stg_world_save( stg_world_t* world, CWorldFile* wfp )
{
  // ask every model to save itself
  g_hash_table_foreach( world->models_id, stg_model_save_cb, wfp );
}


void stg_world_save_cb( gpointer key, gpointer data, gpointer user )
{
  stg_world_save( (stg_world_t*)data, (CWorldFile*)user );
}


void stg_client_save( stg_client_t* cli, stg_id_t world_id )
{
  if( cli->callback_save )(*cli->callback_save)();

  PRINT_MSG1( "Stage client: saving worldfile \"%s\"\n", wf.filename );

  // ask every model in the client to save itself
  g_hash_table_foreach( cli->worlds_id_server, stg_world_save_cb, &wf );

  wf.Save(NULL);
}  
  
  void stg_client_load( stg_client_t* cli, stg_id_t world_id )
    {
      if( cli->callback_load )(*cli->callback_load)();
      
      PRINT_WARN( "LOAD NOT YET IMPLEMENTED" );
      //wf.Load();
    }  


// create a world containing a passel of Stage models based on the
// worldfile

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
      
  stg_geom_t geom;
  geom.pose.x = wf.ReadTupleLength(section, "origin", 0, 0.0 );
  geom.pose.y = wf.ReadTupleLength(section, "origin", 1, 0.0 );
  geom.pose.a = wf.ReadTupleLength(section, "origin", 2, 0.0 );
  geom.size.x = wf.ReadTupleLength(section, "size", 0, 10.0 );
  geom.size.y = wf.ReadTupleLength(section, "size", 1, 10.0 );
  stg_model_prop_with_data( root, STG_PROP_GEOM, &geom, sizeof(geom) );
      
  stg_pose_t pose;
  pose.x = geom.size.x/2.0;
  pose.y = geom.size.y/2.0;
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
      stg_scale_lines( lines, num_lines, geom.size.x, geom.size.y );
      stg_translate_lines( lines, num_lines, -geom.size.x/2.0, -geom.size.y/2.0 );
	  
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
      pose.x = wf.ReadTupleLength(section, "pose", 0, STG_DEFAULT_POSEX );
      pose.y = wf.ReadTupleLength(section, "pose", 1, STG_DEFAULT_POSEY );
      pose.a = wf.ReadTupleAngle(section, "pose", 2, STG_DEFAULT_POSEA );      
      
      if( pose.x != STG_DEFAULT_POSEX ||
	  pose.y != STG_DEFAULT_POSEY ||
	  pose.a != STG_DEFAULT_POSEA )
	stg_model_prop_with_data( mod, STG_PROP_POSE, &pose, sizeof(pose) );
      
      stg_geom_t geom;
      geom.pose.x = wf.ReadTupleLength(section, "origin", 0, STG_DEFAULT_ORIGINX );
      geom.pose.y = wf.ReadTupleLength(section, "origin", 1, STG_DEFAULT_ORIGINY);
      geom.pose.a = wf.ReadTupleLength(section, "origin", 2, STG_DEFAULT_ORIGINA );
      geom.size.x = wf.ReadTupleLength(section, "size", 0, STG_DEFAULT_SIZEX );
      geom.size.y = wf.ReadTupleLength(section, "size", 1, STG_DEFAULT_SIZEY );

      if( geom.pose.x != STG_DEFAULT_ORIGINX ||
	  geom.pose.y != STG_DEFAULT_ORIGINY ||
	  geom.pose.a != STG_DEFAULT_ORIGINA ||
 	  geom.size.x != STG_DEFAULT_SIZEX ||
	  geom.size.y != STG_DEFAULT_SIZEY )
	stg_model_prop_with_data( mod, STG_PROP_GEOM, &geom, sizeof(geom) );
      
      stg_bool_t obstacle;
      obstacle = wf.ReadInt( section, "obstacle_return", STG_DEFAULT_OBSTACLERETURN );
      if( obstacle != STG_DEFAULT_OBSTACLERETURN ) 
	stg_model_prop_with_data( mod, STG_PROP_OBSTACLERETURN, &obstacle, sizeof(obstacle) );
      
      stg_bool_t nose;
      nose = wf.ReadInt( section, "nose", STG_DEFAULT_NOSE );
      if( nose != STG_DEFAULT_NOSE )
	stg_model_prop_with_data( mod, STG_PROP_NOSE, &nose, sizeof(nose) );
      
      stg_bool_t grid;
      grid = wf.ReadInt( section, "grid", STG_DEFAULT_GRID );
      if( grid != STG_DEFAULT_GRID )
	stg_model_prop_with_data( mod, STG_PROP_GRID, &grid, sizeof(grid) );
      
      stg_bool_t boundary;
      boundary = wf.ReadInt( section, "boundary", STG_DEFAULT_BOUNDARY );
      if( boundary != STG_DEFAULT_BOUNDARY )
	stg_model_prop_with_data( mod, STG_PROP_BOUNDARY, &boundary,sizeof(boundary));
      
      stg_laser_config_t lconf;
      memset( &lconf, 0, sizeof(lconf) );
      lconf.geom.pose.x = wf.ReadTupleLength(section, "laser_geom", 0, -9999.0 );
      lconf.geom.pose.y = wf.ReadTupleLength(section, "laser_geom", 1, 0);
      lconf.geom.pose.a = wf.ReadTupleAngle(section, "laser_geom", 2, 0);
      lconf.geom.size.x = wf.ReadTupleLength(section, "laser_geom", 3, 0);
      lconf.geom.size.y = wf.ReadTupleLength(section, "laser_geom", 4, 0);
      lconf.range_min = wf.ReadTupleLength(section, "laser", 0, 0);
      lconf.range_max = wf.ReadTupleLength(section, "laser", 1, 0);
      lconf.fov = wf.ReadTupleAngle(section, "laser", 2, 0);
      lconf.samples = (int)wf.ReadTupleFloat(section, "laser", 3, 0);

      if( lconf.geom.pose.x != -9999.0 )
	stg_model_prop_with_data( mod, STG_PROP_LASERCONFIG, &lconf,sizeof(lconf));
  
      
      stg_blobfinder_config_t bcfg;
      memset( &bcfg, 0, sizeof(bcfg) );
      bcfg.channel_count = (int)wf.ReadTupleFloat(section, "blobfinder", 0, -1 );
      bcfg.scan_width = (int)wf.ReadTupleFloat(section, "blobfinder", 1, -1 );
      bcfg.scan_height = (int)wf.ReadTupleFloat(section, "blobfinder", 2, -1 );
      bcfg.range_max = wf.ReadTupleLength(section, "blobfinder", 3, -1 );
      bcfg.pan = wf.ReadTupleAngle(section, "blobfinder", 4, -1 );
      bcfg.tilt = wf.ReadTupleAngle(section, "blobfinder", 5, -1 );
      bcfg.zoom =  wf.ReadTupleAngle(section, "blobfinder", 6, -1 );
      
      if( bcfg.channel_count > STG_BLOBFINDER_CHANNELS_MAX )
	bcfg.channel_count = STG_BLOBFINDER_CHANNELS_MAX;
      
      for( int ch = 0; ch<bcfg.channel_count; ch++ )
	bcfg.channels[ch] = 
	  stg_lookup_color( wf.ReadTupleString(section, "blobchannels", ch, "red" )); 
      
      if( bcfg.channel_count != -1 )
	stg_model_prop_with_data( mod, STG_PROP_BLOBCONFIG, &bcfg,sizeof(bcfg));
      
      const char* colorstr = wf.ReadString( section, "color", NULL );
      if( colorstr )
	{
	  stg_color_t color = stg_lookup_color( colorstr );
	  PRINT_DEBUG2( "stage color %s = %X", colorstr, color );
	  
	  if( color != STG_DEFAULT_COLOR )
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
	  stg_scale_lines( lines, num_lines, geom.size.x, geom.size.y );
	  stg_translate_lines( lines, num_lines, -geom.size.x/2.0, -geom.size.y/2.0 );
	  
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
      vel.x = wf.ReadTupleLength(section, "velocity", 0, 0 );
      vel.y = wf.ReadTupleLength(section, "velocity", 1, 0 );
      vel.a = wf.ReadTupleAngle(section, "velocity", 2, 0 );      
      if( vel.x || vel.y || vel.a )
	stg_model_prop_with_data( mod, STG_PROP_VELOCITY, &vel, sizeof(vel) );
      
      stg_fiducial_config_t fcfg;
      fcfg.min_range = wf.ReadTupleLength(section, "fiducial", 0, -9999.0 );
      fcfg.max_range_anon = wf.ReadTupleLength(section, "fiducial", 1, 0);
      fcfg.max_range_id = wf.ReadTupleLength(section, "fiducial", 2, 0);
      fcfg.fov = wf.ReadTupleAngle(section, "fiducial", 3, 0);

      if( fcfg.min_range != -9999.0 )
	stg_model_prop_with_data( mod, STG_PROP_FIDUCIALCONFIG, &fcfg, sizeof(fcfg) );
    }

  return world;
}

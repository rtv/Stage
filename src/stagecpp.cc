
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

  stg_model_t* root = stg_world_createmodel( world, NULL, 0, 
					     STG_MODEL_BASIC, token );
      
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
      
  stg_guifeatures_t gf;
  gf.boundary = wf.ReadInt(section, "gui.boundary", 1 );
  gf.nose = wf.ReadInt(section, "gui.nose", 0 );
  gf.grid = wf.ReadInt(section, "gui.grid", 1 );
  gf.movemask = wf.ReadInt(section, "gui.movemask", 0 );
  stg_model_prop_with_data(root, STG_PROP_GUIFEATURES, &gf, sizeof(gf));
      
  
  stg_energy_config_t ecfg;
  memset(&ecfg,0,sizeof(ecfg));
  ecfg.capacity = -1;
  stg_model_prop_with_data( root, STG_PROP_ENERGYCONFIG, &ecfg, sizeof(ecfg) );   

  // Iterate through sections and create client-side models
  for (int section = 1; section < wf.GetEntityCount(); section++)
    {
      // a model has the macro name that defined it as it's name
      // unlesss a name is explicitly defined.  TODO - count instances
      // of macro names so we can autogenerate unique names
      const char *typestr = wf.GetEntityType(section);      
      const char *namestr = wf.ReadString(section, "name", typestr );
      stg_token_t* token = stg_token_create( namestr, STG_T_NUM, 99 );

      int parent_section = wf.GetEntityParent( section );
      //printf( "section is %d parent section is %d\n", section, parent_section );

      stg_model_t* parent = 
	(stg_model_t*)g_hash_table_lookup( world->models_section, &parent_section );

      //if( parent )
      //printf( "parent has id %d name %s\n", parent->id_client, parent->token->token );
      //else
      //printf( "no parent\n" );
      
      stg_model_type_t type = STG_MODEL_BASIC;
      
      if( strcmp( typestr, "test" ) == 0 )
	type = STG_MODEL_TEST;
      else if( strcmp( typestr, "laser" ) == 0 )
	type = STG_MODEL_LASER;
      else if( strcmp( typestr, "ranger" ) == 0 )
	type = STG_MODEL_RANGER;
      else if( strcmp( typestr, "position" ) == 0 )
	type = STG_MODEL_POSITION;
      else
	type = STG_MODEL_BASIC;

      PRINT_DEBUG2( "creating model token %s type %d", typestr, type );

      stg_model_t* mod = stg_world_createmodel( world, parent, section, 
						type, token );

      stg_pose_t pose;
      pose.x = wf.ReadTupleLength(section, "pose", 0, STG_DEFAULT_POSEX );
      pose.y = wf.ReadTupleLength(section, "pose", 1, STG_DEFAULT_POSEY );
      pose.a = wf.ReadTupleAngle(section, "pose", 2, STG_DEFAULT_POSEA );      
      
      if( pose.x != STG_DEFAULT_POSEX ||
	  pose.y != STG_DEFAULT_POSEY ||
	  pose.a != STG_DEFAULT_POSEA )
	stg_model_prop_with_data( mod, STG_PROP_POSE, &pose, sizeof(pose) );
      
      stg_geom_t geom;
      geom.pose.x = wf.ReadTupleLength(section, "origin", 0, STG_DEFAULT_GEOM_POSEX );
      geom.pose.y = wf.ReadTupleLength(section, "origin", 1, STG_DEFAULT_GEOM_POSEY);
      geom.pose.a = wf.ReadTupleLength(section, "origin", 2, STG_DEFAULT_GEOM_POSEA );
      geom.size.x = wf.ReadTupleLength(section, "size", 0, STG_DEFAULT_GEOM_SIZEX );
      geom.size.y = wf.ReadTupleLength(section, "size", 1, STG_DEFAULT_GEOM_SIZEY );

      if( geom.pose.x != STG_DEFAULT_GEOM_POSEX ||
	  geom.pose.y != STG_DEFAULT_GEOM_POSEY ||
	  geom.pose.a != STG_DEFAULT_GEOM_POSEA ||
 	  geom.size.x != STG_DEFAULT_GEOM_SIZEX ||
	  geom.size.y != STG_DEFAULT_GEOM_SIZEY )
	stg_model_prop_with_data( mod, STG_PROP_GEOM, &geom, sizeof(geom) );
      
      stg_bool_t obstacle;
      obstacle = wf.ReadInt( section, "obstacle.return", STG_DEFAULT_OBSTACLERETURN );
      if( obstacle != STG_DEFAULT_OBSTACLERETURN ) 
	stg_model_prop_with_data( mod, STG_PROP_OBSTACLERETURN, &obstacle, sizeof(obstacle) );
      
      stg_guifeatures_t gf;
      gf.boundary = wf.ReadInt(section, "gui.boundary", STG_DEFAULT_BOUNDARY );
      gf.nose = wf.ReadInt(section, "gui.nose", STG_DEFAULT_NOSE );
      gf.grid = wf.ReadInt(section, "gui.grid", STG_DEFAULT_GRID );
      gf.movemask = wf.ReadInt(section, "gui.movemask", STG_DEFAULT_MOVEMASK );
      stg_model_prop_with_data(mod, STG_PROP_GUIFEATURES, &gf, sizeof(gf));
      
      stg_laser_config_t lconf;
      memset( &lconf, 0, sizeof(lconf) );
      //lconf.geom.pose.x = wf.ReadTupleLength(section, "laser.pose", 0, -9999.0 );
      //lconf.geom.pose.y = wf.ReadTupleLength(section, "laser.pose", 1, 0);
      //lconf.geom.pose.a = wf.ReadTupleAngle(section, "laser.pose", 2, 0);
      //lconf.geom.size.x = wf.ReadTupleLength(section, "laser.size", 0, STG_DEFAULT_LASER_SIZEX);
      //lconf.geom.size.y = wf.ReadTupleLength(section, "laser.size", 1, STG_DEFAULT_LASER_SIZEY);

      lconf.range_min   = wf.ReadTupleLength(section, "laser.view", 0, STG_DEFAULT_LASER_MINRANGE);
      lconf.range_max   = wf.ReadTupleLength(section, "laser.view", 1, STG_DEFAULT_LASER_MAXRANGE);
      lconf.fov         = wf.ReadTupleAngle(section, "laser.view", 2, 0);
      lconf.samples = wf.ReadInt(section, "laser.samples", STG_DEFAULT_LASER_SAMPLES);

      //if( lconf.geom.pose.x != -9999.0 )
       stg_model_prop_with_data( mod, STG_PROP_CONFIG, &lconf,sizeof(lconf));
      
      // laser visibility
      int laservis = 
	wf.ReadInt(section, "laser.return", STG_DEFAULT_LASERRETURN );      
      if( laservis != STG_DEFAULT_LASERRETURN )
	stg_model_prop_with_data(mod, STG_PROP_LASERRETURN, 
				 &laservis, sizeof(laservis) );
      
      // ranger visibility
      stg_bool_t rangervis = 
	wf.ReadInt( section, "ranger.return", STG_DEFAULT_RANGERRETURN );
      if( rangervis != STG_DEFAULT_RANGERRETURN ) 
	stg_model_prop_with_data( mod, STG_PROP_RANGERRETURN, 
				  &rangervis, sizeof(rangervis) );
      
      
      stg_blobfinder_config_t bcfg;
      memset( &bcfg, 0, sizeof(bcfg) );
      bcfg.channel_count = 
	wf.ReadInt(section, "blob.count", STG_DEFAULT_BLOB_CHANNELCOUNT);
      
      bcfg.scan_width = (int)wf.ReadTupleFloat(section, "blobf.image", 0, STG_DEFAULT_BLOB_SCANWIDTH );
      bcfg.scan_height = (int)wf.ReadTupleFloat(section, "blob.image", 1, STG_DEFAULT_BLOB_SCANHEIGHT );

      bcfg.range_max = wf.ReadLength(section, "blob.range", STG_DEFAULT_BLOB_RANGEMAX );

      bcfg.pan = wf.ReadTupleAngle(section, "blob.ptz", 0, STG_DEFAULT_BLOB_PAN );
      bcfg.tilt = wf.ReadTupleAngle(section, "blob.ptz", 1, STG_DEFAULT_BLOB_TILT );
      bcfg.zoom =  wf.ReadTupleAngle(section, "blob.ptz", 2, STG_DEFAULT_BLOB_ZOOM );
      
      if( bcfg.channel_count > STG_BLOBFINDER_CHANNELS_MAX )
	bcfg.channel_count = STG_BLOBFINDER_CHANNELS_MAX;
      
      for( int ch = 0; ch<bcfg.channel_count; ch++ )
	bcfg.channels[ch] = 
	  stg_lookup_color( wf.ReadTupleString(section, "blob.channels", ch, "red" )); 
      
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
      int scount = wf.ReadInt( section, "ranger.count", 0);
      if (scount > 0)
	{
	  char key[256];
	  stg_ranger_config_t* configs = (stg_ranger_config_t*)
	    calloc( sizeof(stg_ranger_config_t), scount );

	  int i;
	  for(i = 0; i < scount; i++)
	    {
	      snprintf(key, sizeof(key), "ranger.pose[%d]", i);
	      configs[i].pose.x = wf.ReadTupleLength(section, key, 0, 0);
	      configs[i].pose.y = wf.ReadTupleLength(section, key, 1, 0);
	      configs[i].pose.a = wf.ReadTupleAngle(section, key, 2, 0);

	      snprintf(key, sizeof(key), "ranger.size[%d]", i);
	      configs[i].size.x = wf.ReadTupleLength(section, key, 0, 0.01);
	      configs[i].size.y = wf.ReadTupleLength(section, key, 1, 0.05);

	      snprintf(key, sizeof(key), "ranger.view[%d]", i);
	      configs[i].bounds_range.min = 
		wf.ReadTupleLength(section, key, 0, 0);
	      configs[i].bounds_range.max = 
		wf.ReadTupleLength(section, key, 1, 5.0);
	      configs[i].fov 
		= DTOR(wf.ReadTupleAngle(section, key, 2, 5.0 ));
	    }
	  
	  PRINT_DEBUG1( "loaded %d ranger configs", scount );	  
	  stg_model_prop_with_data( mod, STG_PROP_RANGERCONFIG,
				    configs, scount * sizeof(stg_ranger_config_t) );

	  free( configs );
	}
      
      
      int linecount = wf.ReadInt( section, "lines.count", 0 );
      if( linecount > 0 )
	{
	  char key[256];
	  stg_line_t* lines = (stg_line_t*)calloc( sizeof(stg_line_t), linecount );
	  int l;
	  for(l=0; l<linecount; l++ )
	    {
	      snprintf(key, sizeof(key), "lines.points[%d]", l);

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
      fcfg.min_range = wf.ReadTupleLength(section, "fiducial.view", 0, -9999.0 );
      fcfg.max_range_anon = wf.ReadTupleLength(section, "fiducial.view", 1, 0);
      fcfg.fov = wf.ReadTupleAngle(section, "fiducial.view", 2, 0);
      fcfg.max_range_id = wf.ReadLength(section, "fiducial.id_limit", -1);
      
      if( fcfg.min_range != -9999.0 )
	stg_model_prop_with_data( mod, STG_PROP_FIDUCIALCONFIG, &fcfg, sizeof(fcfg) );
      
      int fid_return = wf.ReadInt( section, "fiducial.return", -99999 );

      if( fid_return != -99999 )
	stg_model_prop_with_data( mod, STG_PROP_FIDUCIALRETURN, 
				  &fid_return, sizeof(fid_return) );
		
      stg_energy_config_t ecfg;
      ecfg.capacity 
	= wf.ReadFloat(section, "energy.capacity", STG_DEFAULT_ENERGY_CAPACITY );
      ecfg.probe_range 
	= wf.ReadFloat(section, "energy.range", STG_DEFAULT_ENERGY_PROBERANGE );      
      ecfg.give_rate 
	= wf.ReadFloat(section, "energy.return", STG_DEFAULT_ENERGY_GIVERATE );
      ecfg.trickle_rate 
	= wf.ReadFloat(section, "energy.trickle", STG_DEFAULT_ENERGY_TRICKLERATE );
      
      if( ecfg.capacity != STG_DEFAULT_ENERGY_CAPACITY ||
	  ecfg.probe_range != STG_DEFAULT_ENERGY_PROBERANGE ||
	  ecfg.give_rate != STG_DEFAULT_ENERGY_GIVERATE ||
	  ecfg.trickle_rate != STG_DEFAULT_ENERGY_TRICKLERATE )
	stg_model_prop_with_data( mod, STG_PROP_ENERGYCONFIG, &ecfg, sizeof(ecfg) );         
      stg_kg_t mass;
      mass = wf.ReadFloat(section, "mass", STG_DEFAULT_MASS );
      if( mass != STG_DEFAULT_MASS )
	stg_model_prop_with_data( mod, STG_PROP_MASS, &mass, sizeof(mass) );  

    }

  return world;
}

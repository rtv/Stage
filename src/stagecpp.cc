// this file is a hacky use of the old C++ worldfile code. It will
// fade away as we move to a new worldfile implementation.  Every time
// I use this I get more pissed off with it. It works but it's ugly as
// sin. RTV.

// $Id: stagecpp.cc,v 1.56 2004-09-25 02:15:00 rtv Exp $

//#define DEBUG

/** @defgroup worldfile WorldFile tags */
/** @{ 

This is the worldfile description. It goes here! 

@} */

/** @addtogroup worldfile */
/** foo bar bash bang bat */

// TODO - get this from a system header?
#define MAXPATHLEN 256

#include "stage.h"
#include "worldfile.hh"


static CWorldFile wf;

void configure_model( stg_model_t* mod, int section )
{
  stg_pose_t pose;
  pose.x = wf.ReadTupleLength(section, "pose", 0, STG_DEFAULT_POSEX );
  pose.y = wf.ReadTupleLength(section, "pose", 1, STG_DEFAULT_POSEY );
  pose.a = wf.ReadTupleAngle(section, "pose", 2, STG_DEFAULT_POSEA );      
  
  //if( pose.x || pose.y || pose.a )
  //stg_model_prop_with_data( mod, STG_PROP_POSE, &pose, sizeof(pose) );
  stg_model_set_pose( mod, &pose );

  
  /** Load the geometry */
  stg_geom_t geom;
  geom.pose.x = wf.ReadTupleLength(section, "origin", 0, STG_DEFAULT_GEOM_POSEX );
  geom.pose.y = wf.ReadTupleLength(section, "origin", 1, STG_DEFAULT_GEOM_POSEY);
  geom.pose.a = wf.ReadTupleLength(section, "origin", 2, STG_DEFAULT_GEOM_POSEA );
  geom.size.x = wf.ReadTupleLength(section, "size", 0, STG_DEFAULT_GEOM_SIZEX );
  geom.size.y = wf.ReadTupleLength(section, "size", 1, STG_DEFAULT_GEOM_SIZEY );

//   if( geom.pose.x != STG_DEFAULT_GEOM_POSEX ||
//       geom.pose.y != STG_DEFAULT_GEOM_POSEY ||
//       geom.pose.a != STG_DEFAULT_GEOM_POSEA ||
//       geom.size.x != STG_DEFAULT_GEOM_SIZEX ||
//       geom.size.y != STG_DEFAULT_GEOM_SIZEY )
//     stg_model_prop_with_data( mod, STG_PROP_GEOM, &geom, sizeof(geom) );

  stg_model_set_geom( mod, &geom );

      
  stg_bool_t obstacle;
  obstacle = wf.ReadInt( section, "obstacle_return", STG_DEFAULT_OBSTACLERETURN );    
  stg_model_set_obstaclereturn( mod, &obstacle );
  
  stg_guifeatures_t gf;
  gf.boundary = wf.ReadInt(section, "gui.boundary", STG_DEFAULT_GUI_BOUNDARY );
  gf.nose = wf.ReadInt(section, "gui.nose", STG_DEFAULT_GUI_NOSE );
  gf.grid = wf.ReadInt(section, "gui.grid", STG_DEFAULT_GUI_GRID );
  gf.movemask = wf.ReadInt(section, "gui.movemask", STG_DEFAULT_GUI_MOVEMASK );
  stg_model_set_guifeatures( mod, &gf );
  
  // laser visibility
  int laservis = 
    wf.ReadInt(section, "laser_return", STG_DEFAULT_LASERRETURN );      
  stg_model_set_laserreturn( mod, (stg_laser_return_t*)&laservis );
  
  // blob visibility
  //int blobvis = 
  //wf.ReadInt(section, "blob_return", STG_DEFAULT_BLOBRETURN );      
 
  // TODO
  //stg_model_set_blobreturn( mod, &blobvis );
  
  // ranger visibility
  stg_bool_t rangervis = 
   wf.ReadInt( section, "ranger_return", STG_DEFAULT_RANGERRETURN );
  
  // TODO
  //odel_set_ra

  // fiducial visibility
  int fid_return = wf.ReadInt( section, "fiducial_id", FiducialNone );  
  stg_model_set_fiducialreturn( mod, &fid_return );

  const char* colorstr = wf.ReadString( section, "color", NULL );
  if( colorstr )
    {
      stg_color_t color = stg_lookup_color( colorstr );
      PRINT_DEBUG2( "stage color %s = %X", colorstr, color );
	  
      //if( color != STG_DEFAULT_COLOR )
      //stg_model_prop_with_data( mod, STG_PROP_COLOR, &color,sizeof(color));

      stg_model_set_color( mod, &color );
    }

  const char* bitmapfile = wf.ReadString( section, "bitmap", NULL );
  if( bitmapfile )
    {
      stg_rotrect_t* rects = NULL;
      int num_rects = 0;
      
#ifdef DEBUG
      char buf[MAXPATHLEN];
      char* path = getcwd( buf, MAXPATHLEN );
      PRINT_DEBUG2( "in %s attempting to load %s",
		    path, bitmapfile );
#endif

      if( stg_load_image( bitmapfile, &rects, &num_rects ) )
	exit( -1 );
	  
      // convert rects to an array of lines
      int num_lines = 4 * num_rects;
      stg_line_t* lines = stg_rects_to_lines( rects, num_rects );
      stg_normalize_lines( lines, num_lines );
      stg_scale_lines( lines, num_lines, geom.size.x, geom.size.y );
      stg_translate_lines( lines, num_lines, -geom.size.x/2.0, -geom.size.y/2.0 );
	
      stg_model_set_lines( mod, lines, num_lines );
      //stg_model_prop_with_data( mod, STG_PROP_LINES, 
      //			lines, num_lines * sizeof(stg_line_t ));
	  	  
      free( lines );
	  
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
	  
      //stg_model_prop_with_data( mod, STG_PROP_LINES,
      //			lines, linecount * sizeof(stg_line_t) );
      stg_model_set_lines( mod, lines, linecount );
      free( lines );
    }
      
  stg_velocity_t vel;
  vel.x = wf.ReadTupleLength(section, "velocity", 0, 0 );
  vel.y = wf.ReadTupleLength(section, "velocity", 1, 0 );
  vel.a = wf.ReadTupleAngle(section, "velocity", 2, 0 );      
  stg_model_set_velocity( mod, &vel );
    
  stg_energy_config_t ecfg;
  ecfg.capacity 
    = wf.ReadFloat(section, "energy.capacity", STG_DEFAULT_ENERGY_CAPACITY );
  ecfg.probe_range 
    = wf.ReadFloat(section, "energy.range", STG_DEFAULT_ENERGY_PROBERANGE );      
  ecfg.give_rate 
    = wf.ReadFloat(section, "energy_return", STG_DEFAULT_ENERGY_GIVERATE );
  ecfg.trickle_rate 
    = wf.ReadFloat(section, "energy.trickle", STG_DEFAULT_ENERGY_TRICKLERATE );
  stg_model_set_energy_config( mod, &ecfg );

  stg_kg_t mass;
  mass = wf.ReadFloat(section, "mass", STG_DEFAULT_MASS );
  stg_model_set_mass( mod, &mass );
}

/** @addtogroup worldfile */
/** @{ */
/** @defgroup worldfilelaser Laser model properties
 - laser.samples
 - laser.range_min
 - laser.range_max
 - laser.fov
*/
/** @} */


void configure_laser( stg_model_t* mod, int section )
{
  stg_laser_config_t lconf;
  memset( &lconf, 0, sizeof(lconf) );
  
  lconf.samples = 
    wf.ReadInt(section, "laser.samples", STG_DEFAULT_LASER_SAMPLES);
  lconf.range_min = 
    wf.ReadLength(section, "laser.range_min", STG_DEFAULT_LASER_MINRANGE);
  lconf.range_max = 
    wf.ReadLength(section, "laser.range_max", STG_DEFAULT_LASER_MAXRANGE);
  lconf.fov = 
    wf.ReadAngle(section, "laser.fov", STG_DEFAULT_LASER_FOV);

  stg_model_set_config( mod, &lconf, sizeof(lconf));
}

/** @addtogroup worldfile */
/** @{ */
/** @defgroup worldfilefiducial Fiducial model properties
 - fiducial.range_min
 - fiducial.range_max_anon
 - fiducial.range_max_id
 - fiducial.fov
*/
/** @} */

void configure_fiducial( stg_model_t* mod, int section )
{
  stg_fiducial_config_t fcfg;
  fcfg.min_range = 
    wf.ReadLength(section, "fiducial.range_min", 
		  STG_DEFAULT_FIDUCIAL_RANGEMIN );
  fcfg.max_range_anon = 
    wf.ReadLength(section, "fiducial.range_max_anon", 
		  STG_DEFAULT_FIDUCIAL_RANGEMAXANON );
  fcfg.fov = 
    wf.ReadAngle(section, "fiducial.fov",
		 STG_DEFAULT_FIDUCIAL_FOV );
  fcfg.max_range_id = 
    wf.ReadLength(section, "fiducial.range_max_id", 
		  STG_DEFAULT_FIDUCIAL_RANGEMAXID );

  stg_model_set_config( mod, &fcfg, sizeof(fcfg));
}

/** @addtogroup worldfile */
/** @{ */
/** @defgroup worldfileblobfinder Blobfinder model properties
 - 
*/
/** @} */

void configure_blobfinder( stg_model_t* mod, int section )
{
  stg_blobfinder_config_t bcfg;
  memset( &bcfg, 0, sizeof(bcfg) );
  
  bcfg.channel_count = 
    wf.ReadInt(section, "blob.count", STG_DEFAULT_BLOB_CHANNELCOUNT);
  
  bcfg.scan_width = (int)
    wf.ReadTupleFloat(section, "blobf.image", 0, STG_DEFAULT_BLOB_SCANWIDTH);
  bcfg.scan_height = (int)
    wf.ReadTupleFloat(section, "blob.image", 1, STG_DEFAULT_BLOB_SCANHEIGHT );	    
  bcfg.range_max = 
    wf.ReadLength(section, "blob.range_max", STG_DEFAULT_BLOB_RANGEMAX );
  bcfg.pan = 
    wf.ReadTupleAngle(section, "blob.ptz", 0, STG_DEFAULT_BLOB_PAN );
  bcfg.tilt = 
    wf.ReadTupleAngle(section, "blob.ptz", 1, STG_DEFAULT_BLOB_TILT );
  bcfg.zoom =  
    wf.ReadTupleAngle(section, "blob.ptz", 2, STG_DEFAULT_BLOB_ZOOM );
  
  if( bcfg.channel_count > STG_BLOB_CHANNELS_MAX )
    bcfg.channel_count = STG_BLOB_CHANNELS_MAX;
  
  for( int ch = 0; ch<bcfg.channel_count; ch++ )
    bcfg.channels[ch] = 
      stg_lookup_color( wf.ReadTupleString(section, 
					   "blob.channels", 
					   ch, "red" )); 
  
  stg_model_set_config( mod, &bcfg, sizeof(bcfg));
}

/** @addtogroup worldfile */
/** @{ */
/** @defgroup worldfileranger Ranger model properties
 - ranger.count int (number of transducers)
 - ranger.pose[\<transducer number\>] [x y theta]
 - ranger.size[\<transducer number\>] [x y]
 - ranger.view[\<transducer number\>] [range_min range_max fov]
*/
/** @} */



void configure_ranger( stg_model_t* mod, int section )
{
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

      stg_model_set_config( mod, configs, scount * sizeof(stg_ranger_config_t) );

      free( configs );
    }
}

/** @addtogroup worldfile */
/** @{ */
/** @defgroup worldfileposition Position model properties
 - none
*/
/** @} */


void configure_position( stg_model_t* mod, int section )
{
  stg_position_cfg_t cfg;
  memset(&cfg,0,sizeof(cfg));
  
  const char* mode_str =  wf.ReadString( section, "drive", "diff" );
  
  if( strcmp( mode_str, "diff" ) == 0 )
    cfg.steer_mode = STG_POSITION_STEER_DIFFERENTIAL;
  else if( strcmp( mode_str, "omni" ) == 0 )
    cfg.steer_mode = STG_POSITION_STEER_INDEPENDENT;
  else
    {
      PRINT_ERR1( "invalid position drive mode specified: \"%s\" - should be one of: \"diff\", \"omni\". Using \"diff\" as default.", mode_str );
      
      cfg.steer_mode = STG_POSITION_STEER_DIFFERENTIAL;
    }
  
  //stg_model_prop_with_data( mod, STG_PROP_CONFIG, &cfg, sizeof(cfg));

  stg_model_set_config( mod, &cfg, sizeof(cfg) ); 
}



void stg_model_save( stg_model_t* model, CWorldFile* worldfile )
{
  stg_pose_t* pose = stg_model_get_pose(model);
  
  // right noe we only save poses
  worldfile->WriteTupleLength( model->id, "pose", 0, pose->x);
  worldfile->WriteTupleLength( model->id, "pose", 1, pose->y);
  worldfile->WriteTupleAngle( model->id, "pose", 2, pose->a);
}

void stg_model_save_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_save( (stg_model_t*)data, (CWorldFile*)user );
}

void stg_world_save( stg_world_t* world, CWorldFile* wfp )
{
  // ask every model to save itself
  g_hash_table_foreach( world->models, stg_model_save_cb, wfp );
}




// create a world containing a passel of Stage models based on the
// worldfile

stg_world_t* stg_world_create_from_file( char* worldfile_path )
{
  wf.Load( worldfile_path );
  
  int section = 0;
  
  char* world_name =
    (char*)wf.ReadString(section, "name", worldfile_path );
  
  double resolution = 
    wf.ReadFloat(0, "resolution", STG_DEFAULT_RESOLUTION ); 
  resolution = 1.0 / resolution; // invert res to ppm
  
  stg_msec_t interval_real = 
    wf.ReadInt(section, "interval_real", STG_DEFAULT_INTERVAL_REAL );

  stg_msec_t interval_sim = 
    wf.ReadInt(section, "interval_sim", STG_DEFAULT_INTERVAL_SIM );
      
  //printf( "interval sim %lu   interval real %lu\n",
  //      interval_sim, interval_real );
  
  // create a single world
  stg_world_t* world = 
    stg_world_create( 0, 
		      world_name, 
		      interval_sim, 
		      interval_real,
		      resolution );

  if( world == NULL )
    return NULL; // failure
  
  // configure the GUI
  


  // Iterate through sections and create client-side models
  for (int section = 1; section < wf.GetEntityCount(); section++)
    {
      char *typestr = (char*)wf.GetEntityType(section);      

      int parent_section = wf.GetEntityParent( section );
      
      PRINT_DEBUG2( "section %d parent section %d\n", 
		    section, parent_section );
      
      stg_model_t* parent = NULL;
      
      parent = (stg_model_t*)
	g_hash_table_lookup( world->models, &parent_section );
      
      // select model type based on the worldfile token
      stg_model_type_t type;
      
      if( strcmp( typestr, "model" ) == 0 ) // basic model
	type = STG_MODEL_BASIC;
      else if( strcmp( typestr, "test" ) == 0 ) // specialized models
	type = STG_MODEL_TEST;
      else if( strcmp( typestr, "laser" ) == 0 )
	type = STG_MODEL_LASER;
      else if( strcmp( typestr, "ranger" ) == 0 )
	type = STG_MODEL_RANGER;
      else if( strcmp( typestr, "position" ) == 0 )
	type = STG_MODEL_POSITION;
      else if( strcmp( typestr, "blobfinder" ) == 0 )
	type = STG_MODEL_BLOB;
      else if( strcmp( typestr, "fiducialfinder" ) == 0 )
	type = STG_MODEL_FIDUCIAL;
      else 
	{
	  PRINT_ERR1( "unknown model type \"%s\". Model has not been created.",
		      typestr ); 
	  continue;
	}
      
      //PRINT_WARN3( "creating model token %s type %d instance %d", 
      //	    typestr, 
      //	    type,
      //	    parent ? parent->child_type_count[type] : world->child_type_count[type] );
      
      // generate a name and count this type in its parent (or world,
      // if it's a top-level object)
      char namebuf[STG_TOKEN_MAX];  
      if( parent == NULL )
	snprintf( namebuf, STG_TOKEN_MAX, "%s:%d", 
		  typestr, 
		  world->child_type_count[type]++);
      else
	snprintf( namebuf, STG_TOKEN_MAX, "%s.%s:%d", 
		  parent->token,
		  typestr, 
		  parent->child_type_count[type]++ );
      
      //PRINT_WARN1( "generated name %s", namebuf );
      
      // having done all that, allow the user to specify a name instead
      char *namestr = (char*)wf.ReadString(section, "name", namebuf );

      //PRINT_WARN2( "loading model name %s for type %s", namebuf, typestr );
      
      PRINT_DEBUG2( "creating model from section %d parent section %d",
		    section, parent_section );

      stg_model_t* mod = 
	//stg_world_createmodel( world, parent, section, type, namestr );
	stg_world_model_create( world, section, parent_section, type, namestr );
      
      // load all the generic specs from this section.
      configure_model( mod, section );
      
      
      // load type-specific configs from this section
      switch( type )
	{
	case STG_MODEL_LASER:
	  configure_laser( mod, section );
	  break;

	case STG_MODEL_RANGER:
	  configure_ranger( mod, section );
	  break;

	case STG_MODEL_BLOB:
	  configure_blobfinder( mod, section );
	  break;
	  
	case STG_MODEL_FIDUCIAL:
	  configure_fiducial( mod, section );
	  break;
	  
	case STG_MODEL_POSITION:
	  configure_position( mod, section );
	  break;
	  
	default:
	  PRINT_DEBUG1( "don't know how to configure type %d", type );
	}
    }

  return world;
}




// this file is a hacky use of the old C++ worldfile code. It will
// fade away as we move to a new worldfile implementation.  Every time
// I use this I get more pissed off with it. It works but it's ugly as
// sin. RTV.

// $Id: stagecpp.cc,v 1.74 2004-12-30 04:39:25 rtv Exp $

//#define DEBUG


/** @defgroup worldfile WorldFile Properties 
 * some world file text
 */


// TODO - get this from a system header?
#define MAXPATHLEN 256
#define STG_TOKEN_MAX 64

#include "stage_internal.h"
#include "gui.h"
#include "worldfile.hh"

static CWorldFile wf;

/** @defgroup model_window GUI Window

<h2>Worldfile Properties</h2>

@par Summary and default values

@verbatim
window
(
  # gui properties
  center [0 0]
  size [? ?]
  scale ?

  # model properties do not apply to the gui window
)
@endverbatim

@par Details
- size [int int]
  - [width height] 
  - size of the window in pixels
- center [float float]
  - [x y] 
  - location of the center of the window in world coordinates (meters)
- scale
  - ratio of world to pixel coordinates (window zoom)

*/

void configure_gui( gui_window_t* win, int section )
{
  // remember the section for saving later
  win->wf_section = section;

  int window_width = 
    (int)wf.ReadTupleFloat(section, "size", 0, STG_DEFAULT_WINDOW_WIDTH);
  int window_height = 
    (int)wf.ReadTupleFloat(section, "size", 1, STG_DEFAULT_WINDOW_HEIGHT);
  
  double window_center_x = 
    wf.ReadTupleFloat(section, "center", 0, 0.0 );
  double window_center_y = 
    wf.ReadTupleFloat(section, "center", 1, 0.0 );
  
  double window_scale = 
    wf.ReadFloat(section, "scale", 1.0 );
  
  PRINT_DEBUG2( "window width %d height %d", window_width, window_height );
  PRINT_DEBUG2( "window center (%.2f,%.2f)", window_center_x, window_center_y );
  PRINT_DEBUG1( "window scale %.2f", window_scale );
  
  // ask the canvas to comply
  gtk_window_resize( GTK_WINDOW(win->canvas->frame), window_width, window_height );
  rtk_canvas_scale( win->canvas, window_scale, window_scale );
  rtk_canvas_origin( win->canvas, window_center_x, window_center_y );
}

void save_gui( gui_window_t* win )
{
  int width, height;
  gtk_window_get_size(  GTK_WINDOW(win->canvas->frame), &width, &height );
  
  wf.WriteTupleFloat( win->wf_section, "size", 0, width );
  wf.WriteTupleFloat( win->wf_section, "size", 1, height );

  wf.WriteTupleFloat( win->wf_section, "center", 0, win->canvas->ox );
  wf.WriteTupleFloat( win->wf_section, "center", 1, win->canvas->oy );

  wf.WriteFloat( win->wf_section, "scale", win->canvas->sx );
}

/** @defgroup model_basic Basic model
    
The basic model simulates an object with basic properties; position,
size, velocity, color, visibility to various sensors, etc. The basic
model also has a body made up of a list of lines. Internally, the
basic model is used base class for all other model types. You can use
the basic model to simulate environmental objects.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
model
(
  pose [0 0 0]
  size [0 0]
  origin [0 0 0]
  velocity [0 0 0]

  color "red" # body colorx

  # determine how the model appears in various sensors
  obstacle_return 1
  laser_return 1
  ranger_return 1
  blobfinder_return 1
  fiducial_return 1

  # GUI properties
  gui_nose 0
  gui_grid 0
  gui_boundary 0
  gui_movemask ?

  # body shape
  line_count 4
  line[0][0 0 1 0]
  line[1][1 0 1 1]
  line[2][1 1 0 1]
  line[3][0 1 0 0]

  bitmap ""
  bitmap_resolution 0
)
@endverbatim

@par Details
- pose [float float float]
  - [x_position y_position heading_angle]
  - specify the pose of the model in its parent's coordinate system
- size [float float]
  - [x_size y_size]
  - specify the size of the model
- origin [float float float]
  - [x_position y_position heading_angle]
  - specify the position of the object's center, relative to its pose
- velocity [float float float]
  - [x_speed y_speed rotation_speed]
  - specify the initial velocity of the model. Not that if the model hits an obstacle, its velocity will be set to zero.
- color string
  - specify the color of the object using a color name from the X11 database (rgb.txt)
- line_count int
  - specify the number of lines that make up the model's body
- line[index] [float float float float]
  - [x1 y1 x2 y2]
  - creates a line from (x1,y1) to (x2,y2). A set of line_count lines defines the robot's body for the purposes of collision detection and rendering in the GUI window.
- bitmap string
  - filename
  - alternative way to set the model's line_count and lines. The file must be a bitmap recognized by libgtkpixbuf (most popular formats are supported). The file is opened and parsed into a set of lines. Unless the bitmap_resolution option is used, the lines are scaled to fit inside the rectangle defined by the model's current size.
- bitmap_resolution float
  - alternative way to set the model's size. Used with the bitmap option, this sets the model's size according to the size of the bitmap file, by multiplying the width and height of the bitmap, measured in pixels, by this scaling factor. 
- gui_nose bool
  - if 1, draw a nose on the model showing its heading (positive X axis)
- gui_grid bool
  - if 1, draw a scaling grid over the model
- gui_movemask bool
  - define how the model can be moved by the mouse in the GUI window
- gui_boundary bool
  - if 1, draw a bounding box around the model, indicating its size
- obstacle_return bool
  - if 1, this model can collide with other models that have this property set
- blob_return bool
  - if 1, this model can be detected in the blob_finder (depending on its color)
- ranger_return bool
  - if 1, this model can be detected by ranger sensors
- laser_return int
  - if 0, this model is not detected by laser sensors. if 1, the model shows up in a laser sensor with normal (0) reflectance. If 2, it shows up with high (1) reflectance.
- fiducial_return int
  - if non-zero, this model is detected by fiducialfinder sensors. The value is used as the fiducial ID.

*/

/*
TODO

- friction float
  - [WARNING: Friction model is not yet implemented; details may change] if > 0 the model can be pushed around by other moving objects. The value determines the proportion of velocity lost per second. For example, 0.1 would mean that the object would lose 10% of its speed due to friction per second. A value of zero (the default) means this model can not be pushed around (infinite friction). 
*/


  
void configure_model( stg_model_t* mod, int section )
{
  stg_pose_t pose, pose_default;
  stg_get_default_pose( &pose_default );

  pose.x = wf.ReadTupleLength(section, "pose", 0, pose_default.x );
  pose.y = wf.ReadTupleLength(section, "pose", 1, pose_default.y ); 
  pose.a = wf.ReadTupleAngle(section, "pose", 2,  pose_default.a );

  if( memcmp( &pose, &pose_default, sizeof(pose))) // if different to default
    stg_model_set_pose( mod, &pose );
  
  stg_geom_t geom, geom_default;
  stg_get_default_geom( &geom_default );
  
  geom.pose.x = wf.ReadTupleLength(section, "origin", 0, geom_default.pose.x );
  geom.pose.y = wf.ReadTupleLength(section, "origin", 1, geom_default.pose.y );
  geom.pose.a = wf.ReadTupleLength(section, "origin", 2, geom_default.pose.a );
  geom.size.x = wf.ReadTupleLength(section, "size", 0, geom_default.size.x );
  geom.size.y = wf.ReadTupleLength(section, "size", 1, geom_default.size.x );
  
  if( memcmp( &geom, &geom_default, sizeof(geom))) // if different to default
    stg_model_set_geom( mod, &geom );
  
  stg_bool_t obstacle;
  obstacle = wf.ReadInt( section, "obstacle_return", STG_DEFAULT_OBSTACLERETURN );    
  stg_model_set_obstaclereturn( mod, &obstacle );
  
  stg_guifeatures_t gf;
  gf.boundary = wf.ReadInt(section, "gui_boundary", STG_DEFAULT_GUI_BOUNDARY );
  gf.nose = wf.ReadInt(section, "gui_nose", STG_DEFAULT_GUI_NOSE );
  gf.grid = wf.ReadInt(section, "gui_grid", STG_DEFAULT_GUI_GRID );
  gf.movemask = wf.ReadInt(section, "gui_movemask", STG_DEFAULT_GUI_MOVEMASK );
  stg_model_set_guifeatures( mod, &gf );
  
  //stg_friction_t friction;
  //friction = wf.ReadFloat(section, "friction", 0.0 );
  //stg_model_set_friction(mod, &friction );

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
  int fid_return = wf.ReadInt( section, "fiducial_return", FiducialNone );  
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
      
      int image_width=0, image_height=0;
      if( stg_load_image( bitmapfile, &rects, &num_rects, 
			  &image_width, &image_height ) )
	exit( -1 );
      
      double bitmap_resolution = 
	wf.ReadFloat( section, "bitmap_resolution", 0 );
      
      // if a bitmap resolution was specified, we override the object
      // geometry
      if( bitmap_resolution )
	{
	  geom.size.x = image_width *  bitmap_resolution;
	  geom.size.y = image_height *  bitmap_resolution; 	    
	  stg_model_set_geom( mod, &geom );	  
	}
	  
      // convert rects to an array of lines and upload the lines
      //int num_lines = 4 * num_rects;
      //stg_line_t* lines = stg_rects_to_lines( rects, num_rects );
      //stg_normalize_lines( lines, num_lines );
      //stg_scale_lines( lines, num_lines, geom.size.x, geom.size.y );
      //stg_translate_lines( lines, num_lines, -geom.size.x/2.0, -geom.size.y/2.0 );     
      //stg_model_set_lines( mod, lines, num_lines );
      
      // convert rects to an array of polygons and upload the polygons
      stg_polygon_t* polys = stg_rects_to_polygons( rects, num_rects );
      //stg_normalize_polygons( polys, num_rects, geom.size.x, geom.size.y );
      stg_model_set_polygons( mod, polys, num_rects );

     
      //free( lines );
    }
      
  int polycount = wf.ReadInt( section, "polygons", 0 );
  if( polycount > 0 )
    {
      //printf( "expecting %d polygons\n", polycount );
      
      char key[256];
      stg_polygon_t* polys = stg_polygons_create( polycount );
      int l;
      for(l=0; l<polycount; l++ )
	{	  	  
	  snprintf(key, sizeof(key), "polygon[%d].points", l);
	  int pointcount = wf.ReadInt(section,key,0);
	  
	  //printf( "expecting %d points in polygon %d\n",
	  //  pointcount, l );
	  
	  int p;
	  for( p=0; p<pointcount; p++ )
	    {
	      snprintf(key, sizeof(key), "polygon[%d].point[%d]", l, p );
	      
	      stg_point_t pt;	      
	      pt.x = wf.ReadTupleLength(section, key, 0, 0);
	      pt.y = wf.ReadTupleLength(section, key, 1, 0);

	      //printf( "key %s x: %.2f y: %.2f\n",
	      //      key, pt.x, pt.y );
	      
	      // append the point to the polygon
	      stg_polygon_append_points( &polys[l], &pt, 1 );
	    }
	}
      
      stg_model_set_polygons( mod, polys, polycount );
    }


  stg_velocity_t vel;
  vel.x = wf.ReadTupleLength(section, "velocity", 0, 0 );
  vel.y = wf.ReadTupleLength(section, "velocity", 1, 0 );
  vel.a = wf.ReadTupleAngle(section, "velocity", 2, 0 );      
  stg_model_set_velocity( mod, &vel );
    
//   stg_energy_config_t ecfg;
//   ecfg.capacity 
//     = wf.ReadFloat(section, "energy_capacity", STG_DEFAULT_ENERGY_CAPACITY );
//   ecfg.probe_range 
//     = wf.ReadFloat(section, "energy_range", STG_DEFAULT_ENERGY_PROBERANGE );      
//   ecfg.give_rate 
//     = wf.ReadFloat(section, "energy_return", STG_DEFAULT_ENERGY_GIVERATE );
//   ecfg.trickle_rate 
//     = wf.ReadFloat(section, "energy_trickle", STG_DEFAULT_ENERGY_TRICKLERATE );
//   stg_model_set_energy_config( mod, &ecfg );

  stg_kg_t mass;
  mass = wf.ReadFloat(section, "mass", STG_DEFAULT_MASS );
  stg_model_set_mass( mod, &mass );
}


/** 
@defgroup model_laser Laser model 
The laser model simulates a scanning laser rangefinder

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
laser
(
  # laser properties
  samples 180
  range_min 0.0
  range_max 8.0
  fov 180.0

  # model properties
  size [0.1 0.1]
  color "blue"
)
@endverbatim

@par Details
- samples int
  - the number of laser samples per scan
- range_min float
  -  the minimum range reported by the scanner, in meters. The scanner will detect objects closer than this, but report their range as the minimum.
- range_max float
  - the maximum range reported by the scanner, in meters. The scanner will not detect objects beyond this range.
- fov float
  - the angular field of view of the scanner, in degrees. 

*/

void configure_laser( stg_model_t* mod, int section )
{
  stg_laser_config_t lconf;
  memset( &lconf, 0, sizeof(lconf) );
  
  lconf.samples = 
    wf.ReadInt(section, "samples", STG_DEFAULT_LASER_SAMPLES);
  lconf.range_min = 
    wf.ReadLength(section, "range_min", STG_DEFAULT_LASER_MINRANGE);
  lconf.range_max = 
    wf.ReadLength(section, "range_max", STG_DEFAULT_LASER_MAXRANGE);
  lconf.fov = 
    wf.ReadAngle(section, "fov", STG_DEFAULT_LASER_FOV);

  stg_model_set_config( mod, &lconf, sizeof(lconf));
}

/** @defgroup model_fiducial Fiducial detector
The fiducial model simulates a fiducial-detecting device.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
fiducialfinder
(
  # fiducialfinder properties
  range_min 0.0
  range_max 8.0
  range_max_id 5.0
  fov 180.0

  # model properties
  size [0 0]
)
@endverbatim

@par Details
- range_min float
  - the minimum range reported by the sensor, in meters. The sensor will detect objects closer than this, but report their range as the minimum.
- range_max float
  - the maximum range at which the sensor can detect a fiducial, in meters. The sensor may not be able to uinquely identify the fiducial, depending on the value of range_max_id.
- range_max_id float
  - the maximum range at which the sensor can detect the ID of a fiducial, in meters.
- fov float
  - the angular field of view of the scanner, in degrees. 

*/

void configure_fiducial( stg_model_t* mod, int section )
{
  stg_fiducial_config_t fcfg;
  fcfg.min_range = 
    wf.ReadLength(section, "range_min", 
		  STG_DEFAULT_FIDUCIAL_RANGEMIN );
  fcfg.max_range_anon = 
    wf.ReadLength(section, "range_max", 
		  STG_DEFAULT_FIDUCIAL_RANGEMAXANON );
  fcfg.fov = 
    wf.ReadAngle(section, "fov",
		 STG_DEFAULT_FIDUCIAL_FOV );
  fcfg.max_range_id = 
    wf.ReadLength(section, "range_max_id", 
		  STG_DEFAULT_FIDUCIAL_RANGEMAXID );

  stg_model_set_config( mod, &fcfg, sizeof(fcfg));
}

/** @defgroup model_blobfinder Blobfinder model

The blobfinder model simulates a color-blob-finding vision device,
like a CMUCAM2, or the ACTS image processing software. It can track
areas of color in a simulated 2D image, giving the location and size
of the color 'blobs'. Multiple colors can be tracked at once; they are
separated into channels, so that e.g. all red objects are tracked as
channel one, blue objects in channel two, etc. The color associated
with each channel is configurable.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
blobfinder
(
  # blobfinder properties
  channel_count 6
  channels ["red" "green" "blue" "cyan" "yellow" "magenta" ]
  range_max 8.0
  ptz[0 0 60.0]
  image[80 60]

  # model properties
  size [0.01 0.01]
)
@endverbatim

@par Details
- channel_count int
  - number of channels; i.e. the number of discrete colors detected
- channels [ string ... ]
  - define the colors detected in each channel, using color names from the X11 database 
   (rgb.txt). The number of strings should match channel_count.
- image [int int]
  - [width height]
  - dimensions of the image in pixels. This determines the blobfinder's 
    resolution
- ptz [float float float]
   - [pan_angle tilt_angle zoom_angle] 
   - control the panning, tilt and zoom angle (fov) of the blobfinder. Tilt angle currently has no effect.
- range_max float
   - maximum range of the sensor in meters.

*/

void configure_blobfinder( stg_model_t* mod, int section )
{
  stg_blobfinder_config_t bcfg;
  memset( &bcfg, 0, sizeof(bcfg) );
  
  bcfg.channel_count = 
    wf.ReadInt(section, "channel_count", 6 );
  
  bcfg.scan_width = (int)
    wf.ReadTupleFloat(section, "image", 0, STG_DEFAULT_BLOB_SCANWIDTH);
  bcfg.scan_height = (int)
    wf.ReadTupleFloat(section, "image", 1, STG_DEFAULT_BLOB_SCANHEIGHT );	    
  bcfg.range_max = 
    wf.ReadLength(section, "range_max", STG_DEFAULT_BLOB_RANGEMAX );
  bcfg.pan = 
    wf.ReadTupleAngle(section, "ptz", 0, STG_DEFAULT_BLOB_PAN );
  bcfg.tilt = 
    wf.ReadTupleAngle(section, "ptz", 1, STG_DEFAULT_BLOB_TILT );
  bcfg.zoom =  
    wf.ReadTupleAngle(section, "ptz", 2, STG_DEFAULT_BLOB_ZOOM );
  
  if( bcfg.channel_count > STG_BLOB_CHANNELS_MAX )
    bcfg.channel_count = STG_BLOB_CHANNELS_MAX;
  
  for( int ch = 0; ch<bcfg.channel_count; ch++ )
    {
      stg_color_t col = 0xFF0000; //red
      
      switch( ch%6 )
      {
      case 0:  bcfg.channels[ch] = stg_lookup_color( "red" ); break;
      case 1:  bcfg.channels[ch] = stg_lookup_color( "green" ); break;
      case 2:  bcfg.channels[ch] = stg_lookup_color( "blue" ); break;
      case 3:  bcfg.channels[ch] = stg_lookup_color( "cyan" ); break;
      case 4:  bcfg.channels[ch] = stg_lookup_color( "yellow" ); break;
      case 5:  bcfg.channels[ch] = stg_lookup_color( "magenta" ); break;
      }
    }    
  
  stg_model_set_config( mod, &bcfg, sizeof(bcfg));
}

/**
@defgroup model_ranger Ranger model 
The ranger model simulates an array of sonar or infra-red (IR) range sensors.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
ranger
(
  # ranger properties
  scount 16
  spose[0] [? ? ?]
  spose[1] [? ? ?]
  spose[2] [? ? ?]
  spose[3] [? ? ?]
  spose[4] [? ? ?]
  spose[5] [? ? ?]
  spose[6] [? ? ?]
  spose[7] [? ? ?]
  spose[8] [? ? ?]
  spose[9] [? ? ?]
  spose[10] [? ? ?]
  spose[11] [? ? ?]
  spose[12] [? ? ?]
  spose[13] [? ? ?]
  spose[14] [? ? ?]
  spose[15] [? ? ?]
   
  ssize [0.01 0.03]
  sview [? ? ?]

  # model properties
)
@endverbatim

@par Notes

The ranger model allows configuration of the pose, size and view parameters of each transducer seperately (using spose[index], ssize[index] and sview[index]). However, most users will set a common size and view (using ssize and sview), and just specify individual transducer poses.

@par Details
- scount int 
  - the number of range transducers
- spose[\<transducer index\>] [float float float]
  - [x y theta] 
  - pose of the transducer relative to its parent.
- ssize [float float]
  - [x y] 
  - size in meters. Has no effect on the data, but controls how the sensor looks in the Stage window.
- ssize[\<transducer index\>] [float float]
  - per-transducer version of the ssize property. Overrides the common setting.
- sview [float float float]
   - [range_min range_max fov] 
   - minimum range and maximum range in meters, field of view angle in degrees. Currently fov has no effect on the sensor model, other than being shown in the confgiuration graphic for the ranger device.
- sview[\<transducer index\>] [float float float]
  - per-transducer version of the sview property. Overrides the common setting.

*/



void configure_ranger( stg_model_t* mod, int section )
{
  // Load the geometry of a ranger array
  int scount = wf.ReadInt( section, "scount", 0);
  if (scount > 0)
    {
      char key[256];
      stg_ranger_config_t* configs = (stg_ranger_config_t*)
	calloc( sizeof(stg_ranger_config_t), scount );
      
      stg_size_t common_size;
      common_size.x = wf.ReadTupleLength(section, "ssize", 0, 0.01 );
      common_size.y = wf.ReadTupleLength(section, "ssize", 1, 0.03 );
      
      double common_min = wf.ReadTupleLength(section, "sview", 0, 0.0);
      double common_max = wf.ReadTupleLength(section, "sview", 1, 5.0);
      double common_fov = wf.ReadTupleAngle(section, "sview", 2, 5.0);

      // set all transducers with the common settings
      int i;
      for(i = 0; i < scount; i++)
	{
	  configs[i].size.x = common_size.x;
	  configs[i].size.y = common_size.y;
	  configs[i].bounds_range.min = common_min;
	  configs[i].bounds_range.max = common_max;
	  configs[i].fov = common_fov;
	}

      // allow individual configuration of transducers
      for(i = 0; i < scount; i++)
	{
	  snprintf(key, sizeof(key), "spose[%d]", i);
	  configs[i].pose.x = wf.ReadTupleLength(section, key, 0, 0);
	  configs[i].pose.y = wf.ReadTupleLength(section, key, 1, 0);
	  configs[i].pose.a = wf.ReadTupleAngle(section, key, 2, 0);
	  
	  snprintf(key, sizeof(key), "ssize[%d]", i);
	  configs[i].size.x = wf.ReadTupleLength(section, key, 0, 0.01);
	  configs[i].size.y = wf.ReadTupleLength(section, key, 1, 0.05);
	  
	  snprintf(key, sizeof(key), "sview[%d]", i);
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

/** 
@defgroup model_position Position model 
The position model simulates a mobile robot base. It can drive in one
of two modes; either <i>differential</i> like a Pioneer robot, or
<i>omnidirectional</i>.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
position
(
  # position properties
  drive "diff"

  # model properties
)
@endverbatim

@par Details
- drive "diff" or "omni"
  - select differential-steer mode (like a Pioneer) or omnidirectional mode.

*/


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
  stg_model_set_config( mod, &cfg, sizeof(cfg) ); 


  stg_pose_t odom;
  odom.x = wf.ReadTupleLength(section, "odom", 0, 0.0 );
  odom.y = wf.ReadTupleLength(section, "odom", 1, 0.0 );
  odom.a = wf.ReadTupleAngle(section, "odom", 2, 0.0 );
  stg_model_position_set_odom( mod, &odom );
}


void stg_model_save( stg_model_t* model, CWorldFile* worldfile )
{
  stg_pose_t pose;
  stg_model_get_pose(model, &pose);
  
  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
		model->token,
		pose.x,
		pose.y,
		pose.a );

  // right now we only save poses
  worldfile->WriteTupleLength( model->id, "pose", 0, pose.x);
  worldfile->WriteTupleLength( model->id, "pose", 1, pose.y);
  worldfile->WriteTupleAngle( model->id, "pose", 2, pose.a);
}

void stg_model_save_cb( gpointer key, gpointer data, gpointer user )
{
  stg_model_save( (stg_model_t*)data, (CWorldFile*)user );
}

void stg_world_save( stg_world_t* world, CWorldFile* wfp )
{
  // ask every model to save itself
  g_hash_table_foreach( world->models, stg_model_save_cb, wfp );

  save_gui( world->win );

  wfp->Save( NULL );
}

void stg_world_save( stg_world_t* world )
{
  // ask every model to save itself
  g_hash_table_foreach( world->models, stg_model_save_cb, &wf );

  // ask the gui to save itself
  save_gui( world->win );
  
  wf.Save( NULL );
}

// create a world containing a passel of Stage models based on the
// worldfile

stg_world_t* stg_world_create_from_file( char* worldfile_path )
{
  wf.Load( worldfile_path );
  
  int section = 0;
  
  char* world_name =
    (char*)wf.ReadString(section, "name", worldfile_path );
  
  stg_msec_t interval_real = 
    wf.ReadInt(section, "interval_real", STG_DEFAULT_INTERVAL_REAL );

  stg_msec_t interval_sim = 
    wf.ReadInt(section, "interval_sim", STG_DEFAULT_INTERVAL_SIM );
      
  double ppm_high = 
    1.0 / wf.ReadFloat(0, "resolution", STG_DEFAULT_RESOLUTION ); 

  double ppm_med = 
    1.0 / wf.ReadFloat(0, "resolution_med", 0.2 ); 
  
  double ppm_low = 
    1.0 / wf.ReadFloat(0, "resolution_low", 1.0 ); 
  
  // create a single world
  stg_world_t* world = 
    stg_world_create( 0, 
		      world_name, 
		      interval_sim, 
		      interval_real,
		      ppm_high,
		      ppm_med,
		      ppm_low );

  if( world == NULL )
    return NULL; // failure
  
  // configure the GUI
  


  // Iterate through sections and create client-side models
  for (int section = 1; section < wf.GetEntityCount(); section++)
    {
      if( strcmp( wf.GetEntityType(section), "window") == 0 )
	{
	  configure_gui( world->win, section ); 
	}
      else
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
	  
	  stg_model_t* mod = NULL;
	  stg_model_t* parent_mod = stg_world_get_model( world, parent_section );
	  
	  // load type-specific configs from this section
	  switch( type )
	    {
	    case STG_MODEL_BASIC:
	      mod = stg_model_create( world, parent_mod, section, STG_MODEL_BASIC, namestr );
	      configure_model( mod, section );	  
	      break;
	      
	    case STG_MODEL_BLOB:
	      mod = stg_blobfinder_create( world, parent_mod, section, namestr );
	      configure_model( mod, section );	  
	      configure_blobfinder( mod, section );
	      break;
	      
	      
	    case STG_MODEL_LASER:
	      mod = stg_laser_create( world,  parent_mod, section, namestr );
	      configure_model( mod, section );	  
	      configure_laser( mod, section );
	      break;
	      
	    case STG_MODEL_RANGER:
	      mod = stg_ranger_create( world,  parent_mod, section, namestr );
	      configure_model( mod, section );	  
	      configure_ranger( mod, section );
	      break;
	      
	    case STG_MODEL_FIDUCIAL:
	      mod = stg_fiducial_create( world,  parent_mod, section, namestr );
	      configure_model( mod, section );	  
	      configure_fiducial( mod, section );
	      break;
	      
	    case STG_MODEL_POSITION:
	      mod = stg_position_create( world,  parent_mod, section, namestr );
	      configure_model( mod, section );	  
	      configure_position( mod, section );
	      break;

	    default:
	      PRINT_ERR1( "don't know how to configure type %d", type );
	    }

	  assert( mod );
	  
	  // add the new model to the world
	  stg_world_add_model( world, mod );
	}
    }
  return world;
}




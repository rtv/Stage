///////////////////////////////////////////////////////////////////////////
//
// foofinderdevice.cc 
// Richard Vaughan 
// created 20030328
//
//  $Id: foofinderdevice.cc,v 1.1 2003-04-01 00:20:56 rtv Exp $
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <math.h>
#include <stage.h>

#include "world.hh"
#include "foofinderdevice.hh"
#include "raytrace.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
CFooFinder::CFooFinder(LibraryItem* libit,CWorld *world, CEntity *parent )
  : CPlayerEntity(libit,world, parent )
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = sizeof( player_fiducial_data_t );
  m_command_len = 0; 
  m_config_len  = 1;
  m_reply_len  = 1;

  m_player.code = PLAYER_FIDUCIAL_CODE;
  
  // This is not a real object, so by default we dont see it
  this->obstacle_return = false;
  this->sonar_return = false;
  this->puck_return = false;
  this->vision_return = false;
  this->idar_return = IDARTransparent;
  this->laser_return = LaserTransparent;

  // time of last update
  this->time_sec = 0;
  this->time_usec = 0;

  // we're a 5cm square by default
  this->size_x = this->size_y = 0.05;

  m_interval = 0.1; // 10Hz default

  // Set FOV defaults
  this->max_range = 5.0;
  this->min_range = 0.5;
  this->view_angle = M_PI/2.0;

#ifdef INCLUDE_RTK2
  this->beacon_fig = NULL;
#endif

}

///////////////////////////////////////////////////////////////////////////
// Startup routine
//
bool CFooFinder::Startup()
{
  if (!CPlayerEntity::Startup())
    return false;

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Load the entity from the world file
bool CFooFinder::Load(CWorldFile *worldfile, int section)
{
  if (!CPlayerEntity::Load(worldfile, section))
    return false;

  this->max_range = worldfile->ReadLength(section, "max_range",
                                               this->max_range);
  this->min_range = worldfile->ReadLength(section, "min_range",
                                               this->min_range);
  this->view_angle = worldfile->ReadLength(section, "fov",
                                               this->view_angle);

  this->threshold = worldfile->ReadFloat(section, "threshold", 0.1 );

  char occlusion_string[STAGE_TOKEN_MAX];
  strncpy( occlusion_string,
	   worldfile->ReadString(section, "occlusion", "yes" ),
	   STAGE_TOKEN_MAX );
  
  if( strcmp( occlusion_string, "yes" ) == 0 )
    this->perform_occlusion_test = true;
  else if( strcmp( occlusion_string, "on" ) == 0 )
    this->perform_occlusion_test = true;
  else if( strcmp( occlusion_string, "no" ) == 0 )
    this->perform_occlusion_test = false;
  else if( strcmp( occlusion_string, "off" ) == 0 )
    this->perform_occlusion_test = false;
  else
    PRINT_WARN1( "unrecognized occlusion setting \"%s\". "
		 " Please use \"on\" or \"off\"", occlusion_string );
  
  // determine which mode we are in, i.e. which filter we will use to
  // detect targets
  char modestring[64];
  strncpy( modestring, worldfile->ReadString(section,"mode","model"), 64 );
  
  if( strcmp( modestring, "model" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_MODEL;
      
      // read mode-specific settings
      // the token we must match
      strncpy( model_token, 
	       worldfile->ReadString(section, "model_token", "position" ),
	       STAGE_TOKEN_MAX );
    }

  else if( strcmp( modestring, "fiducial" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_FIDUCIAL;
      // read mode-specific settings
    }

  else if( strcmp( modestring, "toplevel" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_TOPLEVEL;
      // read mode-specific settings
    }

  else if( strcmp( modestring, "movement" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_MOVEMENT;
      // read mode-specific settings
      
      // set the angular speed hi-pass threshold
      this->movement_theta_threshold = 
	worldfile->ReadFloat(section, "theta_threshold", 0.0 );

      char frame[STAGE_TOKEN_MAX];
      strncpy( frame, 
	       worldfile->ReadString(section,"frame","absolute"),
	       STAGE_TOKEN_MAX);
      
      if( strcmp( frame, "absolute" ) == 0 )
	this->movement_relative = 0;
      else if( strcmp( frame, "relative" ) == 0 )
	this->movement_relative = 1;
      else
	PRINT_WARN1( "unknown reference frame requested (%s)."
		     " Use \"absolute\" or \"relative\"", frame );
    }
  
  else if( strcmp( modestring, "sound" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_SOUND;
      // read mode-specific settings
    }
  
  else if( strcmp( modestring, "radiation" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_RADIATION;
    }
  else if( strcmp( modestring, "team" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_TEAM;
    }
  else if( strcmp( modestring, "color" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_COLOR;
      this->color_match = 
	worldfile->ReadColor(section, "color_match", this->color );

    }
  else if( strcmp( modestring, "robotid" ) == 0 )
    {
      this->mode = STG_FOOFINDER_MODE_ROBOTID;
      PRINT_WARN1( "%s mode not implemented", modestring );
    }
  else
    PRINT_WARN2( "unknown foofinder mode %s. "
		 "defaulting to model mode with token %s", 
		 modestring, model_token );

  PRINT_DEBUG3( "Mode %d (%s) occlusion %s", 
		this->mode, modestring,
		perform_occlusion_test ? "on" : "off" );

  return true;
}

bool CFooFinder::OcclusionTest(CEntity* ent )
{
  // Compute parameters of scan line
  double startx, starty, endx, endy, dummyth;
  this->GetGlobalPose( startx, starty, dummyth );
  ent->GetGlobalPose( endx, endy, dummyth );
  
  CLineIterator lit( startx, starty, endx, endy,
		     m_world->ppm, m_world->matrix,  PointToPoint );
  CEntity* hit;
  
  while( (hit = lit.GetNextEntity()) )
    {
      if( hit == ent ) // if we hit the entity there was no occlusion
	return false;

      // we use laser visibility for basic occlusion
      if( hit != this && hit != m_parent_entity && hit->laser_return ) 
	{
	  return true;
	}
    }	
  
  // we didn't hit the target and we ran out of entities (a little
  // wierd, but it's certainly not a hit...
  return true;  
}

///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void CFooFinder::Update( double sim_time )
{
  CPlayerEntity::Update( sim_time ); // inherit debug output

  ASSERT(m_world != NULL );

  if(!Subscribed())
    return;

  // if its time to recalculate 
  //
  if( sim_time - m_last_update < m_interval )
    return;

  m_last_update = sim_time;

  // check for configuration requests
  // the client can request the geometry of all beacons

  UpdateConfig();

  // Get the pose of the detector in the global cs
  //
  double ox, oy, oth;
  GetGlobalPose(ox, oy, oth);

  // space to store information about the fiducials we find
  int f_count = 0;
  int f_ids[PLAYER_FIDUCIAL_MAX_SAMPLES];
  double f_poses[PLAYER_FIDUCIAL_MAX_SAMPLES][3];
  double f_pose_errors[PLAYER_FIDUCIAL_MAX_SAMPLES][3];
  
  f_count = 0; // initialise the count in the export structure

   
  // Search through the global device list looking for devices that
  // match our current search conditions

  int num_entities = m_world->GetEntityCount();

  for( int c=0; c<num_entities; c++ )
    {
      // check if we've run out of room
      if( f_count > PLAYER_FIDUCIAL_MAX_SAMPLES )
	break;

      CEntity* ent = m_world->GetEntity(c);
      
      // start out with no ID information for this target
      f_ids[f_count] = FiducialNone; 
      
      // filter by mode
      int reject = 1;
      switch( this->mode )
	{
	case STG_FOOFINDER_MODE_FIDUCIAL:
	  if( ent->fiducial_return != FiducialNone )
	    {
	      f_ids[f_count] = ent->fiducial_return;
	      reject = 0;
	    }
	  break;

	case STG_FOOFINDER_MODE_MODEL:
	  if( strcmp( ent->lib_entry->token, model_token ) == 0 )
	    reject = 0; // the token matches!
	  break;

	case STG_FOOFINDER_MODE_TOPLEVEL:
	  if( ent->m_parent_entity == m_world->root && 
	      strcmp( ent->lib_entry->token, "bitmap" ) != 0 )
	    reject = 0; // it's on the root object (and it's not a bitmap)!
	  break;
	  
	  // TODO
	case STG_FOOFINDER_MODE_ROBOTID:
	  break;

	case STG_FOOFINDER_MODE_COLOR:
	  if( this->color_match == ent->color )
	    reject = 0;
	  break;

	  // TODO
	case STG_FOOFINDER_MODE_MOVEMENT:
	  // if the target is moving faster than the threshold speed
	  // and it's on the root
	  double vx, vy, vth;
	  ent->GetGlobalVel(vx, vy, vth );
	  
	  if( this->movement_relative )
	    {
	      // subtract my speed from the target to get relative speeds
	      double myvx, myvy, myvth;
	      this->GetGlobalVel( myvx, myvy, myvth );
	      vx -= myvx;
	      vy -= myvy;
	    }
	  
	  if( (hypot( vx, vy ) > this->threshold || 
	       fabs(vth) > this->movement_theta_threshold) && 
	      ent->m_parent_entity == m_world->root )
	    reject = 0; //it's a moving thing!
	  break;
	  
	case STG_FOOFINDER_MODE_SOUND:
	  {
	    // Compute range of entity relative to sensor
	    double px, py, pth;   
	    ent->GetGlobalPose( px, py, pth );
	    
	    // ampltitude scales with inverse square of distance
	    double dx = px - ox;
	    double dy = py - oy;
	    double range = sqrt(dx * dx + dy * dy);
	    
	    if( (ent->noise.amplitude * 1.0/range ) > this->threshold )
	      {
		f_ids[f_count] = ent->noise.frequency;
		reject = 0; // it's audible!
	      }
	  }
	  break;

	case STG_FOOFINDER_MODE_RADIATION:
	  {
	    // Compute range of entity relative to sensor
	    double px, py, pth;   
	    ent->GetGlobalPose( px, py, pth );
	    
	    // ampltitude scales with inverse square of distance
	    double dx = px - ox;
	    double dy = py - oy;
	    double range = sqrt(dx * dx + dy * dy);
	    
	    if( (ent->radiation.amplitude * 1.0/range ) > this->threshold )
	      {
		f_ids[f_count] = ent->radiation.frequency;
		reject = 0; // it's detectable!
	      }
	  }
	  break;

	case STG_FOOFINDER_MODE_TEAM:
	  if( ent->team != 0 )
	    {
	      reject = 0; // it's on a team!
	      f_ids[f_count] = ent->team; // store the team number as the ID
	    }
	  break;
	  
	  

	default: PRINT_DEBUG1( "unknown filter mode %d", this->mode );
	  break;
	}

      if( reject ) // if we filtered this entity out by mode
	continue;

      double px, py, pth;   
      ent->GetGlobalPose( px, py, pth );
      
      // Compute range and bearing of entity relative to sensor
      //
      double dx = px - ox;
      double dy = py - oy;
      double range = sqrt(dx * dx + dy * dy);
      double bearing = NORMALIZE(atan2(dy, dx) - oth);
      double orientation = NORMALIZE(pth - oth);
      
      // filter root object
      if( ent->m_parent_entity == NULL )
	continue;

      // Filter by detection range
      if( range > this->max_range || range < this->min_range )
	continue;
 
      // Filter by view angle
      if( fabs(bearing) > view_angle/2.0 )
	continue;

      // filter by LOS
      if( this->perform_occlusion_test && OcclusionTest(ent) )
	continue;
      
      // store the fiducial location
      f_poses[f_count][0] = range; 
      f_poses[f_count][1] = bearing; 
      f_poses[f_count][2] = orientation; 
      
      // todo - add error estimates here
      f_pose_errors[f_count][0] = 0; // RANGE
      f_pose_errors[f_count][1] = 0; // BEARING
      f_pose_errors[f_count][2] = 0; // ORIENTATION
      
      f_count++;
      
      // record the size of this most recent target
      // as we can;t return individual sizes.
      //this->fiducial_width = nbeacon->size_x;
      //this->fiducial_height = nbeacon->size_y;
    }
  
  // pack into the network-safe player structure
  player_fiducial_data_t data;
  FiducialDataPack( &data, f_count, f_ids, f_poses, f_pose_errors ); 
  
  // Write beacon buffer to shared mem
  // Note that we apply the laser data's timestamp to this data.
  //
  PutData( &data, sizeof(data) );
  this->time_sec = time_sec;
  this->time_usec = time_usec;
}

void CFooFinder::UpdateConfig( void )
{
  int len;
  void *client;
  char buffer[PLAYER_MAX_REQREP_SIZE];
  
  while (true)
    {
      len = GetConfig(&client, buffer, PLAYER_MAX_REQREP_SIZE);
      if (len <= 0)
        break;
      
      switch (buffer[0])
        {
	  
	case PLAYER_FIDUCIAL_GET_GEOM: // Return geometry of this
				       // sensor and size of most
				       // recently seen fiducial
	  {
	    PRINT_DEBUG( "get GEOM" );	

	    player_fiducial_geom_t geom;	  
	    FiducialGeomPack( &geom, 
			      this->origin_x, this->origin_y, 0,
			      this->size_x, this->size_y,
			      0.0, 0.0 );
	    
	    PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, 
		     &geom, sizeof(geom));
	  }
	  break;

	case PLAYER_FIDUCIAL_SET_FOV: 	    // Set and return the FOV 
	  {	 
	    PRINT_DEBUG( "set FOV" );	    
	    assert( len == sizeof(player_fiducial_fov_t) );

	    player_fiducial_fov_t fov;	 
	    memcpy( &fov, buffer, sizeof(fov) );
	    
	    // parse the buffer into our fov members
	    FiducialFovUnpack( &fov, 
			       &this->min_range, 
			       &this->max_range, 
			       &this->view_angle ); 
	    
	    // reply with the new FOV (could just echo the buffer, but
	    // this is a little more future-proof)
	    player_fiducial_fov_t replyfov;	  
	    FiducialFovPack( &replyfov, 1,
			     this->min_range, this->max_range, 
			     this->view_angle );
	    
	    PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, 
		     &replyfov, sizeof(replyfov));
	  }
	  break;

	case PLAYER_FIDUCIAL_GET_FOV: // return the FOV
	  {
	    PRINT_DEBUG( "get FOV" );

	    // return the FOV 
	    player_fiducial_fov_t fov;	  
	    FiducialFovPack( &fov, 0,
			     this->min_range, this->max_range, 
			     this->view_angle );
	    
	    PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &fov, sizeof(fov));
	  }
	  break;
	    
	    
	  
	default:
	  PRINT_WARN1("got unknown fiducialfinder config request \"%c\"\n", 
		      buffer[0]);
	  PutReply(client, PLAYER_MSGTYPE_RESP_NACK);
	  break;
	}
    }
}
#ifdef INCLUDE_RTK2

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CFooFinder::RtkStartup()
{
  CPlayerEntity::RtkStartup();
  
  // Create a figure representing this object
  this->beacon_fig = rtk_fig_create(m_world->canvas, NULL, 49);

  // Set the color
  rtk_fig_color_rgb32(this->beacon_fig, this->color);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CFooFinder::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->beacon_fig);

  CPlayerEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CFooFinder::RtkUpdate()
{
  CPlayerEntity::RtkUpdate();
 
  rtk_fig_clear(this->beacon_fig);
  
  int light_gray_color = RGB(200,200,200);

  // if a client is subscribed to this device
  if( Subscribed() > 0 && m_world->ShowDeviceData( this->lib_entry->type_num ) )
  {
    player_fiducial_data_t data;
    
    // attempt to get the right size chunk of data from the mmapped buffer
    if( GetData( &data, sizeof(data) ) == sizeof(data) )
    { 
      // unpack the data into native formats
      int count;
      int ids[PLAYER_FIDUCIAL_MAX_SAMPLES];
      double poses[PLAYER_FIDUCIAL_MAX_SAMPLES][3];
      double errors[PLAYER_FIDUCIAL_MAX_SAMPLES][3];

      FiducialDataUnpack( &data, &count, ids, poses, errors );

      char text[16];

      // Get global pose
      double gx, gy, gth;
      GetGlobalPose(gx, gy, gth);
      
      rtk_fig_origin( this->beacon_fig, gx, gy, gth );
      
      // draw the FOV
      rtk_fig_color_rgb32( this->beacon_fig, light_gray_color );

      // todo - these don't quite work right - need to fix them in
      // RTK.  they are useful enough for now though - rtv
      rtk_fig_ellipse_arc( this->beacon_fig, 0,0,0,
			  2.0*this->max_range, 2.0*this->max_range, 
			  -view_angle/2.0, view_angle/2.0);

      rtk_fig_ellipse_arc( this->beacon_fig, 0,0,0,
			  2.0*this->min_range, 2.0*this->min_range, 
			  -view_angle/2.0, view_angle/2.0);

      double cosa =  cos( view_angle/2.0 );
      double sina =  sin( view_angle/2.0 );
      double vx = this->min_range * cosa;
      double vy = this->min_range * sina;
      double wx = this->max_range * cosa;
      double wy = this->max_range * sina;

      rtk_fig_line( this->beacon_fig, vx, vy, wx, wy );
      rtk_fig_line( this->beacon_fig, vx, -vy, wx, -wy );

      // draw the individual fiducials
      // revert to our specific color
      rtk_fig_color_rgb32( this->beacon_fig, this->color );

      for( int b=0; b < count; b++ )
	{
	  double range = poses[b][0];
	  double bearing = poses[b][1];
	  double orientation = poses[b][2];

	  double px = range * cos(bearing);
	  double py = range * sin(bearing);
	  double pa = orientation;

	  rtk_fig_line( this->beacon_fig, 0, 0, px, py );	

	  double wx = 0;//this->fiducial_width;
	  double wy = 0;//this->fiducial_height;
	  
	  rtk_fig_rectangle(this->beacon_fig, px, py, pa, wx, wy, 0);
	  rtk_fig_arrow(this->beacon_fig, px, py, pa, wy, 0.10);
	  snprintf(text, sizeof(text), "  %d", ids[b] );
	  rtk_fig_text(this->beacon_fig, px, py, pa, text);
	}  
    }
    //else
    //  puts( "subscribed but no data avail!!!!!!!!!!!!!!" );
  }
}
#endif

#include "model.hh"

class StgModelPosition : public StgModel
{
public:
  // constructor
  StgModelPosition( StgWorld* world,
		 StgModel* parent, 
		 stg_id_t id, CWorldFile* wf );
  
  // destructor
  ~StgModelPosition( void );
  
  // control state
  stg_pose_t goal; //< the current velocity or pose to reach,
		   //depending on the value of control_mode
  stg_position_control_mode_t control_mode;
  stg_position_drive_mode_t drive_mode;

  // localization state
  stg_pose_t est_pose; ///< position estimate in local coordinates
  stg_pose_t est_pose_error; ///< estimated error in position estimate
  stg_pose_t est_origin; ///< global origin of the local coordinate system

  stg_position_localization_mode_t localization_mode; ///< global or local mode
  stg_velocity_t integration_error; ///< errors to apply in simple odometry model
  
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void Load( void );

  /** Set the current pose estimate.*/
  void SetOdom( stg_pose_t* odom );
  
  /** Set the goal for the position device. If member control_mode ==
      STG_POSITION_CONTROL_VELOCITY, these are x,y, and rotation
      velocities. If control_mode == STG_POSITION_CONTROL_POSITION,
      [x,y,a] defines a 2D position and heading goal to achieve. */
  void Do( double x, double y, double a ) 
  { goal.x = x; goal.y = y; goal.a = a; }
};


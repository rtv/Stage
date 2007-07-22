#include "stage_internal.h"
#include "worldfile.hh"

/** Define a callback function type that can be attached to a
    record within a model and called whenever the record is set.
*/
class StgModel; // forward declaration
typedef int (*stg_model_callback_t)( StgModel* mod, void* user );

class StgModel
{
public:
  
  // constructor
  StgModel( stg_world_t* world,
	    StgModel* parent, 
	    stg_id_t id,
	    CWorldFile* wf );
  
  // destructor
  virtual ~StgModel( void );
  
  stg_id_t id; // used as hash table key

  
  stg_world_t* world; // pointer to the world in which this model exists
  char token[STG_TOKEN_MAX]; // automatically-generated unique ID string
  CWorldFile* wf; //< worldfile entry for this model

  const char* typestr;
  stg_pose_t pose;
  stg_velocity_t velocity;    
  stg_watts_t watts; //< power consumed by this model
  stg_color_t color;
  stg_kg_t mass;
  stg_geom_t geom;
  int laser_return;
  int obstacle_return;
  int blob_return;
  int gripper_return;
  int ranger_return;
  int fiducial_return;
  int fiducial_key;
  int boundary;
  stg_meters_t map_resolution;
  stg_bool_t stall;
  
  bool on_velocity_list;

  int gui_nose;
  int gui_grid;
  int gui_outline;
  int gui_mask;
  
  StgModel* parent; //< the model that owns this one, possibly NULL
  GList* children; //< the models owned by this model
  
  /** GData datalist can contain arbitrary named data items. Can be used
      by derived model types to store properties, and for user code
      to associate arbitrary items with a model. */
  GData* props;
  
  /** callback functions can be attached to any field in this
      structure. When the internal function model_change(<mod>,<field>)
      is called, the callback is triggered */
  GHashTable* callbacks;
  
  // the number of children of each type is counted so we can
  // automatically generate names for them
  GHashTable* child_type_count;

  int subs;     //< the number of subscriptions to this model
  
  stg_msec_t update_interval_ms; //< time between updates in ms
  stg_msec_t last_update_ms; //< time of last update in ms
  
  stg_bool_t disabled; //< if non-zero, the model is disabled
  
 
  // hooks for data, command and configuration structures for
  // derived types
  //void *data, *cmd, *cfg;
  //size_t data_len, cmd_len, cfg_len;
  
  GList* d_list;    
  GList* blocks; //< list of stg_block_t structs that comprise a body
  
  // display lists
  int dl_body, dl_data, dl_cmd, dl_cfg, dl_grid, dl_debug;
  //GList* raytrace_dl_list;

  bool body_dirty; //< iff true, regenerate block display list before redraw
  bool data_dirty; //< iff true, regenerate data display list before redraw

  stg_pose_t global_pose;

  bool gpose_dirty; //< set this to indicate that global pose may have
		    //changed

  // TODO - optionally thread-safe version allow exclusive access
  // to this model 
  // pthread_mutex_t mutex;
  
public:
  // end experimental    

  //virtual int Initialize( void ); //< post-construction initialiser
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void UpdatePose( void );

  void UpdateIfDue( void );
  //void UpdateTree( void );
  //void UpdateTreeIfDue( void );

  /** configure a model by reading from the current world file */
  virtual void Load( void );
  /** save the state of the model to the current world file */
  virtual void Save( void );
  virtual void Draw( void );
  virtual void DrawData( void );

  // call this to ensure the GUI window is redrawn
  void NeedRedraw( void );

  void AddBlock( stg_point_t* pts, 
		 size_t pt_count, 
		 stg_meters_t zmin, 
		 stg_meters_t zmax,
		 stg_color_t color,
		 bool inherit_color );

  void AddBlockRect( double x, double y, 
		     double width, double height );

  /** remove all blocks from this model, freeing their memory */
  void ClearBlocks( void );

  char* Token( void )
  { return this->token; }

  const char* TypeStr( void )
  { return this->typestr; }

  StgModel* Parent( void )
  { return this->parent; }

  bool Stall( void )
  { return this->stall; }

  GList* Children( void )
  { return this->children; }
  
  int GuiMask( void )
  { return this->gui_mask; };

  stg_world_t* World( void )
  { return this->world; }
  
  bool ObstacleReturn( void )
  { return this->obstacle_return; }
  
  int LaserReturn( void )
  { return this->laser_return; }

  int RangerReturn( void )
  { return this->ranger_return; }
  
  void MapWithChildren( bool render );

  StgModel* TestCollision( stg_pose_t* pose, 
			   double* hitx, double* hity );

  // if render is true, render the model into the matrix, else unrender
  // the model
  void Map( bool render );

  bool IsAntecedent( StgModel* testmod );

  // returns true if model [testmod] is a descendent of model [mod]
  bool IsDescendent( StgModel* testmod );

  bool IsRelated( StgModel* mod2 );

    /** get the pose of a model in the global CS */
  void GetGlobalPose(  stg_pose_t* pose );

  /** get the velocity of a model in the global CS */
  void GetGlobalVelocity(  stg_velocity_t* gvel );

  /* set the velocity of a model in the global coordinate system */
  void SetGlobalVelocity(  stg_velocity_t* gvel );

  /** subscribe to a model's data */
  void Subscribe( void );

  /** unsubscribe from a model's data */
  void Unsubscribe( void );

  /** set the pose of model in global coordinates */
  void SetGlobalPose(  stg_pose_t* gpose );
  
  /** When called, this causes this model and its children to
      recompute their global position instead of using a cached pose
      in StgModel::GetGlobalPose()..*/
  void GPoseDirtyTree( void );

  /** set a model's velocity in its parent's coordinate system */
  void SetVelocity(  stg_velocity_t* vel );
 
  /** set a model's pose in its parent's coordinate system */
  void SetPose(  stg_pose_t* pose );

  /** add values to a model's pose in its parent's coordinate system */
  void AddToPose(  stg_pose_t* pose );

  /** add values to a model's pose in its parent's coordinate system */
  void AddToPose(  double dy, double dy, double dz, double da );

  /** set a model's geometry (size and center offsets) */
  void SetGeom(  stg_geom_t* src );

  /** set a model's geometry (size and center offsets) */
  void SetFiducialReturn(  int fid );

  /** set a model's fiducial key: only fiducial finders with a
      matching key can detect this model as a fiducial. */
  void SetFiducialKey(  int key );
  
  void GetColor( stg_color_t* col )
  { if(color) memcpy( col, &this->color, sizeof(stg_color_t)); }
  
  void GetLaserReturn( int* val )
  { if(val) memcpy( val, &this->laser_return, sizeof(int)); }
  
  /** Change a model's parent - experimental*/
  int SetParent( StgModel* newparent);
  
  /** Get a model's geometry - it's size and local pose (offset from
      origin in local coords) */
  void GetGeom(  stg_geom_t* dest );

  /** Get the pose of a model in its parent's coordinate system  */
  void GetPose(  stg_pose_t* dest );

  /** Get a model's velocity (in its local reference frame) */
  void GetVelocity(  stg_velocity_t* dest );

  // guess what these do?
  void SetColor( stg_color_t col );
  void SetMass( stg_kg_t mass );
  void SetStall( stg_bool_t stall );
  void SetGripperReturn( int val );
  void SetLaserReturn( int val );
  void SetObstacleReturn( int val );
  void SetBlobReturn( int val );
  void SetRangerReturn( int val );
  void SetBoundary( int val );
  void SetGuiNose( int val );
  void SetGuiMask( int val );
  void SetGuiGrid( int val );
  void SetGuiOutline( int val );
  void SetWatts( stg_watts_t watts );
  void SetMapResolution( stg_meters_t res );
  
  /* attach callback functions to data members. The function gets
     called when the member is changed using SetX() accessor method */

  void AddCallback( void* address, 
		    stg_model_callback_t cb, 
		    void* user );
  
  int RemoveCallback( void* member,
		      stg_model_callback_t callback );
  
  void CallCallbacks(  void* address );
  
  /* hooks for attaching special callback functions (not used as
     variables) */
  char startup, shutdown, load, save, update;
  
  /* wrappers for the generic callback add & remove functions that hide
     some implementation detail */
  
  void AddStartupCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &startup, cb, user ); };
  
  void RemoveStartupCallback( stg_model_callback_t cb )
  { RemoveCallback( &startup, cb ); };
  
  void AddShutdownCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &shutdown, cb, user ); };
  
  void RemoveShutdownCallback( stg_model_callback_t cb )
  { RemoveCallback( &shutdown, cb ); }
  
  void AddLoadCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &load, cb, user ); }
  
  void RemoveLoadCallback( stg_model_callback_t cb )
  { RemoveCallback( &load, cb ); }
  
  void AddSaveCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &save, cb, user ); }
  
  void RemoveSaveCallback( stg_model_callback_t cb )
  { RemoveCallback( &save, cb ); }
  
  void AddUpdateCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &update, cb, user ); }
  
  void RemoveUpdateCallback( stg_model_callback_t cb )
  { RemoveCallback( &update, cb ); }
  
  // string-based property interface
  void* GetProperty( char* key );
  int SetProperty( char* key, void* data );    
  void UnsetProperty( char* key );
  
  // GUI stuff

  //int gui_model_create( stg_model_t* mod );

  void AddPropertyToggles( void* member, 
			   stg_model_callback_t callback_on,
			   void* arg_on,
			   stg_model_callback_t callback_off,
			   void* arg_off,
			   char* name,
			   char* label,
			   gboolean enabled );  

  virtual void Print( char* prefix );

  /** return the top-level model above mod */
  StgModel* RootModel( void );
  
  /** Convert a pose in the world coordinate system into a model's
      local coordinate system. Overwrites [pose] with the new
      coordinate. */
  void GlobalToLocal( stg_pose_t* pose );
  
  /** Convert a pose in the model's local coordinate system into the
      world coordinate system. Overwrites [pose] with the new
      coordinate. */
  void LocalToGlobal( stg_pose_t* pose );

  virtual void DListBody( void );
  virtual void DListData( void );
  
};


class StgModelLaser : public StgModel
{
public:
  // constructor
  StgModelLaser( stg_world_t* world,
		 StgModel* parent, 
		 stg_id_t id, CWorldFile* wf );
  
  // destructor
  ~StgModelLaser( void );
  
  stg_laser_sample_t* samples;
  size_t sample_count;
  stg_meters_t range_min, range_max;
  stg_radians_t fov;
  unsigned int resolution;
  
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void Load( void );  
  virtual void Print( char* prefix );
  virtual void DListData( void );
};

class StgModelPosition : public StgModel
{
public:
  // constructor
  StgModelPosition( stg_world_t* world,
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

class StgModelRanger : public StgModel
{
public:
  // constructor
  StgModelRanger( stg_world_t* world,
	       StgModel* parent, 
	       stg_id_t id, CWorldFile* wf );
  
  // destructor
  virtual ~StgModelRanger( void );
  
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void Load( void );
  virtual void Print( char* prefix );
  virtual void DListData( void );

  size_t sensor_count;
  stg_ranger_sensor_t* sensors;
};

//etc.

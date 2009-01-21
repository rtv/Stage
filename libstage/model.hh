#ifndef MODEL_H
#define MODEL_H

class Camera;

/** energy data packet */
class PowerPack
{
public:
  PowerPack( Model* mod );

  /** The model that owns this object */
  Model* mod;
    
  /** Energy stored */
  stg_joules_t stored;

  /** Energy capacity */
  stg_joules_t capacity;

  /** TRUE iff the device is receiving energy from a charger */
  bool charging;

  /** OpenGL visualization of the powerpack state */
  void Visualize( Camera* cam );

  /** Print human-readable status on stdout, prefixed with the
		argument string */
  void Print( char* prefix );
};

class Visibility
{
public:
  bool blob_return;
  int fiducial_key;
  int fiducial_return;
  bool gripper_return;
  stg_laser_return_t laser_return;
  bool obstacle_return;
  bool ranger_return;
  
  Visibility() : 
    blob_return( true ),
    fiducial_key( 0 ),
    fiducial_return( 0 ),
    gripper_return( false ),
    laser_return( LaserVisible ),
    obstacle_return( true ),
    ranger_return( true )
  { /* nothing do do */ }
  
  void Load( Worldfile* wf, int wf_entity )
  {
    blob_return = wf->ReadInt( wf_entity, "blob_return", blob_return);    
    fiducial_key = wf->ReadInt( wf_entity, "fiducial_key", fiducial_key);
    fiducial_return = wf->ReadInt( wf_entity, "fiducial_return", fiducial_return);
    gripper_return = wf->ReadInt( wf_entity, "gripper_return", gripper_return);    
    laser_return = (stg_laser_return_t)wf->ReadInt( wf_entity, "laser_return", laser_return);    
    obstacle_return = wf->ReadInt( wf_entity, "obstacle_return", obstacle_return);    
    ranger_return = wf->ReadInt( wf_entity, "ranger_return", ranger_return);    
  }    
};

/** Abstract class for adding visualizations to models. DataVisualize must be overloaded, and is then called in the models local coord system */
class CustomVisualizer {
public:
	//TODO allow user to specify name - which will show up in display filter
	virtual ~CustomVisualizer( void ) { }
	virtual void DataVisualize( Camera* cam ) = 0;
	virtual const std::string& name() = 0; //must return a name for visualization (careful not to return stack-memory)
};


/* Hooks for attaching special callback functions (not used as
   variables - we just need unique addresses for them.) */  
class CallbackHooks
{
public:
  char load;
  char save;
  char shutdown;
  char startup;
  char update;
};

/** Records model state and functionality in the GUI, if used */
class GuiState
{
public:
  bool grid;
  unsigned int mask;
  bool nose;
  bool outline;
  
  GuiState() :
    grid( false ),
    mask( 0 ),
    nose( false ),
    outline( false )
  { /* nothing to do */}

  void Load( Worldfile* wf, int wf_entity )
  {
    nose = wf->ReadInt( wf_entity, "gui_nose", nose);    
    grid = wf->ReadInt( wf_entity, "gui_grid", grid);    
    outline = wf->ReadInt( wf_entity, "gui_outline", outline);    
    mask = wf->ReadInt( wf_entity, "gui_movemask", mask);    
  }    

};

/// %Model class
class Model : public Ancestor
{
  friend class Ancestor;
  friend class World;
  friend class WorldGui;
  friend class Canvas;
  friend class Block;
  friend class Region;
  friend class BlockGroup;
  
private:
  /** the number of models instatiated - used to assign unique IDs */
  static uint32_t count;
  static GHashTable*  modelsbyid;
  std::vector<Option*> drawOptions;
  const std::vector<Option*>& getOptions() const { return drawOptions; }
  
protected:
  GMutex* access_mutex;
  GPtrArray* blinkenlights;  
  BlockGroup blockgroup;
  /**  OpenGL display list identifier for the blockgroup */
  int blocks_dl;

  /** Iff true, 4 thin blocks are automatically added to the model,
      forming a solid boundary around the bounding box of the
      model. */
  int boundary;

  /** callback functions can be attached to any field in this
      structure. When the internal function model_change(<mod>,<field>)
      is called, the callback is triggered */
  GHashTable* callbacks;

  /** Default color of the model's blocks.*/
  stg_color_t color;

  /** This can be set to indicate that the model has new data that
      may be of interest to users. This allows polling the model
      instead of adding a data callback. */
  bool data_fresh;
  stg_bool_t disabled; //< if non-zero, the model is disabled  
  GList* custom_visual_list;
  GList* flag_list;
  Geom geom;
  Pose global_pose;
  bool gpose_dirty; //< set this to indicate that global pose may have changed  
  /** Controls our appearance and functionality in the GUI, if used */
  GuiState gui;
  
  bool has_default_block;
  
  /* hooks for attaching special callback functions (not used as
     variables - we just need unique addresses for them.) */  
  CallbackHooks hooks;
  
  /** unique process-wide identifier for this model */
  uint32_t id;	
  ctrlinit_t* initfunc;
  stg_usec_t interval; //< time between updates in us
  stg_usec_t last_update; //< time of last update in us  
  bool map_caches_are_invalid;
  stg_meters_t map_resolution;
  stg_kg_t mass;
  bool on_update_list;
  bool on_velocity_list;

  /** Pointer to the parent of this model, possibly NULL. */
  Model* parent; 

  /** The pose of the model in it's parents coordinate frame, or the
      global coordinate frame is the parent is NULL. */
  Pose pose;

  /** Optional attached PowerPack, defaults to NULL */
  PowerPack* power_pack;

  /** GData datalist can contain arbitrary named data items. Can be used
      by derived model types to store properties, and for user code
      to associate arbitrary items with a model. */
  GData* props;
  bool rebuild_displaylist; //< iff true, regenerate block display list before redraw
  char* say_string;   //< if non-null, this string is displayed in the GUI 

  stg_bool_t stall;
  /** Thread safety flag. Iff true, Update() may be called in
      parallel with other models. Defaults to false for safety */
  int subs;     //< the number of subscriptions to this model
  bool thread_safe;
  GArray* trail;
  stg_model_type_t type;  
  bool used;    //< TRUE iff this model has been returned by GetUnusedModelOfType()  
  Velocity velocity;
  stg_watts_t watts; //< power consumed by this model
  Worldfile* wf;
  int wf_entity;
  World* world; // pointer to the world in which this model exists
  WorldGui* world_gui; //pointer to the GUI world - NULL if running in non-gui mode
  	 
public:

  Visibility vis;

  void Lock()
  { 
    if( access_mutex == NULL )
      access_mutex = g_mutex_new();
		
    assert( access_mutex );
    g_mutex_lock( access_mutex );
  }
	 
  void Unlock()
  { 
    assert( access_mutex );
    g_mutex_unlock( access_mutex );
  }
	 

private: 
  /** Private copy constructor declared but not defined, to make it
      impossible to copy models. */
  explicit Model(const Model& original);

  /** Private assignment operator declared but not defined, to make it
      impossible to copy models */
  Model& operator=(const Model& original);

protected:

  /// Register an Option for pickup by the GUI
  void registerOption( Option* opt )
  { drawOptions.push_back( opt ); }

  /** Check to see if the current pose will yield a collision with
      obstacles.  Returns a pointer to the first entity we are in
      collision with, or NULL if no collision exists. */
  Model* TestCollision();
  
  void CommitTestedPose();

  void Map();
  void UnMap();

  void MapWithChildren();
  void UnMapWithChildren();
  
  int TreeToPtrArray( GPtrArray* array );
  
  /** raytraces a single ray from the point and heading identified by
      pose, in local coords */
  stg_raytrace_result_t Raytrace( const Pose &pose,
				  const stg_meters_t range, 
				  const stg_ray_test_func_t func,
				  const void* arg,
				  const bool ztest = true );
  
  /** raytraces multiple rays around the point and heading identified
      by pose, in local coords */
  void Raytrace( const Pose &pose,
		 const stg_meters_t range, 
		 const stg_radians_t fov, 
		 const stg_ray_test_func_t func,
		 const void* arg,
		 stg_raytrace_result_t* samples,
		 const uint32_t sample_count,
		 const bool ztest = true  );
  
  stg_raytrace_result_t Raytrace( const stg_radians_t bearing, 			 
				  const stg_meters_t range,
				  const stg_ray_test_func_t func,
				  const void* arg,
				  const bool ztest = true );
  
  void Raytrace( const stg_radians_t bearing, 			 
		 const stg_meters_t range,
		 const stg_radians_t fov,
		 const stg_ray_test_func_t func,
		 const void* arg,
		 stg_raytrace_result_t* samples,
		 const uint32_t sample_count,
		 const bool ztest = true );
  

  /** Causes this model and its children to recompute their global
      position instead of using a cached pose in
      Model::GetGlobalPose()..*/
  void GPoseDirtyTree();

  virtual void Startup();
  virtual void Shutdown();
  virtual void Update();
  virtual void UpdatePose();

  void StartUpdating();
  void StopUpdating();

  Model* ConditionalMove( Pose newpose );

  stg_meters_t ModelHeight();

  bool UpdateDue( void );
  void UpdateIfDue();
  
  void DrawBlocksTree();
  virtual void DrawBlocks();
  void DrawBoundingBox();
  void DrawBoundingBoxTree();
  virtual void DrawStatus( Camera* cam );
  void DrawStatusTree( Camera* cam );
  
  void DrawOriginTree();
  void DrawOrigin();
  
  void PushLocalCoords();
  void PopCoords();
  
  /** Draw the image stored in texture_id above the model */
  void DrawImage( uint32_t texture_id, Camera* cam, float alpha );
  
  
  /** static wrapper for DrawBlocks() */
  static void DrawBlocks( gpointer dummykey, 
			  Model* mod, 
			  void* arg );
  
  virtual void DrawPicker();
  virtual void DataVisualize( Camera* cam );
  
  virtual void DrawSelected(void);
  
  void DrawTrailFootprint();
  void DrawTrailBlocks();
  void DrawTrailArrows();
  void DrawGrid();
  

  void DrawBlinkenlights();

  void DataVisualizeTree( Camera* cam );
	
  virtual void PushColor( stg_color_t col )
  { world->PushColor( col ); }
	
  virtual void PushColor( double r, double g, double b, double a )
  { world->PushColor( r,g,b,a ); }
	
  virtual void PopColor(){ world->PopColor(); }
	
  void DrawFlagList();

  void DrawPose( Pose pose );

public:
  void RecordRenderPoint( GSList** head, GSList* link, 
			  unsigned int* c1, unsigned int* c2 );

  void PlaceInFreeSpace( stg_meters_t xmin, stg_meters_t xmax, 
			 stg_meters_t ymin, stg_meters_t ymax );
	
  /** Look up a model pointer by a unique model ID */
  static Model* LookupId( uint32_t id )
  { return (Model*)g_hash_table_lookup( modelsbyid, (void*)id ); }
	
  /** Constructor */
  Model( World* world, 
	 Model* parent, 
	 stg_model_type_t type = MODEL_TYPE_PLAIN );

  /** Destructor */
  virtual ~Model();
	
  void Say( const char* str );
  /** Attach a user supplied visualization to a model */
  void AddCustomVisualizer( CustomVisualizer* custom_visual );
  /** remove user supplied visualization to a model - supply the same ptr passed to AddCustomVisualizer */
  void RemoveCustomVisualizer( CustomVisualizer* custom_visual );

	
  void Load( Worldfile* wf, int wf_entity )
  {
    /** Set the worldfile and worldfile entity ID - must be called
	before Load() */
    SetWorldfile( wf, wf_entity );
    Load(); // call virtual load
  }
	
  /** Set the worldfile and worldfile entity ID */
  void SetWorldfile( Worldfile* wf, int wf_entity )
  { this->wf = wf; this->wf_entity = wf_entity; }
	
  /** configure a model by reading from the current world file */
  virtual void Load();
	
  /** save the state of the model to the current world file */
  virtual void Save();
	
  /** Should be called after all models are loaded, to do any last-minute setup */
  void Init();	
  void InitRecursive();

  void AddFlag(  Flag* flag );
  void RemoveFlag( Flag* flag );
	
  void PushFlag( Flag* flag );
  Flag* PopFlag();
	
  int GetFlagCount(){ return g_list_length( flag_list ); }
  
  /** Add a pointer to a blinkenlight to the model. */
  void AddBlinkenlight( stg_blinkenlight_t* b )
  { g_ptr_array_add( this->blinkenlights, b ); }
  
  /** Clear all blinkenlights from the model. Does not destroy the
      blinkenlight objects. */
  void ClearBlinkenlights()
  {  g_ptr_array_set_size( this->blinkenlights, 0 ); }
  
  /** Disable the model. Its pose will not change due to velocity
      until re-enabled using Enable(). This is used for example when
      dragging a model with the mouse pointer. The model is enabled by
      default. */
  void Disable(){ disabled = true; };

  /** Enable the model, so that non-zero velocities will change the
      model's pose. Models are enabled by default. */
  void Enable(){ disabled = false; };
  
  /** Load a control program for this model from an external
      library */
  void LoadControllerModule( char* lib );
	
  /** Sets the redraw flag, so this model will be redrawn at the
      earliest opportunity */
  void NeedRedraw();
	
  /** Add a block to this model by loading it from a worldfile
      entity */
  void LoadBlock( Worldfile* wf, int entity );

  /** Add a block to this model centered at [x,y] with extent [dx, dy,
      dz] */
  void AddBlockRect( stg_meters_t x, stg_meters_t y, 
		     stg_meters_t dx, stg_meters_t dy, 
		     stg_meters_t dz );
	
  /** remove all blocks from this model, freeing their memory */
  void ClearBlocks();
  
  /** Returns a pointer to this model's parent model, or NULL if this
      model has no parent */
  Model* Parent(){ return this->parent; }

  Model* GetModel( const char* name );
  //int GuiMask(){ return this->gui_mask; };

  /** Returns a pointer to the world that contains this model */
  World* GetWorld(){ return this->world; }
  
  /** return the root model of the tree containing this model */
  Model* Root(){ return(  parent ? parent->Root() : this ); }
  
  bool IsAntecedent( Model* testmod );
	
  /** returns true if model [testmod] is a descendent of this model */
  bool IsDescendent( Model* testmod );
	
  /** returns true if model [testmod] is a descendent or antecedent of this model */
  bool IsRelated( Model* testmod );

  /** get the pose of a model in the global CS */
  Pose GetGlobalPose();
	
  /** get the velocity of a model in the global CS */
  Velocity GetGlobalVelocity();
	
  /* set the velocity of a model in the global coordinate system */
  void SetGlobalVelocity(  Velocity gvel );
	
  /** subscribe to a model's data */
  void Subscribe();
	
  /** unsubscribe from a model's data */
  void Unsubscribe();
	
  /** set the pose of model in global coordinates */
  void SetGlobalPose(  Pose gpose );
	
  /** set a model's velocity in its parent's coordinate system */
  void SetVelocity(  Velocity vel );
	
  /** set a model's pose in its parent's coordinate system */
  void SetPose(  Pose pose );
	
  /** add values to a model's pose in its parent's coordinate system */
  void AddToPose(  Pose pose );
	
  /** add values to a model's pose in its parent's coordinate system */
  void AddToPose(  double dx, double dy, double dz, double da );
	
  /** set a model's geometry (size and center offsets) */
  void SetGeom(  Geom src );
	
  /** set a model's geometry (size and center offsets) */
  void SetFiducialReturn(  int fid );
	
  /** set a model's fiducial key: only fiducial finders with a
      matching key can detect this model as a fiducial. */
  void SetFiducialKey(  int key );
	
  stg_color_t GetColor(){ return color; }
	
  //  stg_laser_return_t GetLaserReturn(){ return laser_return; }
	
  /** Change a model's parent - experimental*/
  int SetParent( Model* newparent);
	
  /** Get a model's geometry - it's size and local pose (offset from
      origin in local coords) */
  Geom GetGeom(){ return geom; }
	
  /** Get the pose of a model in its parent's coordinate system  */
  Pose GetPose(){ return pose; }
	
  /** Get a model's velocity (in its local reference frame) */
  Velocity GetVelocity(){ return velocity; }
	
  // guess what these do?
  void SetColor( stg_color_t col );
  void SetMass( stg_kg_t mass );
  void SetStall( stg_bool_t stall );
  void SetGripperReturn( int val );
  void SetLaserReturn( stg_laser_return_t val );
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
	
  bool DataIsFresh(){ return this->data_fresh; }
	
  /* attach callback functions to data members. The function gets
     called when the member is changed using SetX() accessor method */
	
  void AddCallback( void* address, 
		    stg_model_callback_t cb, 
		    void* user );
	
  int RemoveCallback( void* member,
		      stg_model_callback_t callback );
	
  void CallCallbacks(  void* address );
	
  /* wrappers for the generic callback add & remove functions that hide
     some implementation detail */
	
  void AddStartupCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &hooks.startup, cb, user ); };
	
  void RemoveStartupCallback( stg_model_callback_t cb )
  { RemoveCallback( &hooks.startup, cb ); };
	
  void AddShutdownCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &hooks.shutdown, cb, user ); };
	
  void RemoveShutdownCallback( stg_model_callback_t cb )
  { RemoveCallback( &hooks.shutdown, cb ); }
	
  void AddLoadCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &hooks.load, cb, user ); }
	
  void RemoveLoadCallback( stg_model_callback_t cb )
  { RemoveCallback( &hooks.load, cb ); }
	
  void AddSaveCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &hooks.save, cb, user ); }
	
  void RemoveSaveCallback( stg_model_callback_t cb )
  { RemoveCallback( &hooks.save, cb ); }
	
  void AddUpdateCallback( stg_model_callback_t cb, void* user )
  { AddCallback( &hooks.update, cb, user ); }
	
  void RemoveUpdateCallback( stg_model_callback_t cb )
  { RemoveCallback( &hooks.update, cb ); }
	
  /** named-property interface 
   */
  void* GetProperty( char* key );
	
  /** @brief Set a named property of a Stage model.
	 
      Set a property of a Stage model. 
	 
      This function can set both predefined and user-defined
      properties of a model. Predefined properties are intrinsic to
      every model, such as pose and color. Every supported predefined
      properties has its identifying string defined as a preprocessor
      macro in stage.h. Users should use the macro instead of a
      hard-coded string, so that the compiler can help you to avoid
      mis-naming properties.
	 
      User-defined properties allow the user to attach arbitrary data
      pointers to a model. User-defined property data is not copied,
      so the original pointer must remain valid. User-defined property
      names are simple strings. Names beginning with an underscore
      ('_') are reserved for internal libstage use: users should not
      use names beginning with underscore (at risk of causing very
      weird behaviour).
	 
      Any callbacks registered for the named property will be called.      
	 
      Returns 0 on success, or a positive error code on failure.
	 
      *CAUTION* The caller is responsible for making sure the pointer
      points to data of the correct type for the property, so use
      carefully. Check the docs or the equivalent
      stg_model_set_<property>() function definition to see the type
      of data required for each property.
  */ 
  int SetProperty( char* key, void* data );
  void UnsetProperty( char* key );
		
  virtual void Print( char* prefix );
  virtual const char* PrintWithPose();
	
  /** Convert a pose in the world coordinate system into a model's
      local coordinate system. Overwrites [pose] with the new
      coordinate. */
  Pose GlobalToLocal( const Pose pose );
	
  /** Return the global pose (i.e. pose in world coordinates) of a
      pose specified in the model's local coordinate system */
  Pose LocalToGlobal( const Pose pose );
	
  /** Return the 3d point in world coordinates of a 3d point
      specified in the model's local coordinate system */
  stg_point3_t LocalToGlobal( const stg_point3_t local );
	
  /** returns the first descendent of this model that is unsubscribed
      and has the type indicated by the string */
  Model* GetUnsubscribedModelOfType( const stg_model_type_t type );
	
  /** returns the first descendent of this model that is unused
      and has the type indicated by the string. This model is tagged as used. */
  Model* GetUnusedModelOfType( const stg_model_type_t type );
  
  /** Returns the value of the model's stall boolean, which is true
      iff the model has crashed into another model */
  bool Stalled(){ return this->stall; }
};


// BLOBFINDER MODEL --------------------------------------------------------

/** blobfinder data packet 
 */
typedef struct
{
  stg_color_t color;
  uint32_t left, top, right, bottom;
  stg_meters_t range;
} stg_blobfinder_blob_t;

/// %ModelBlobfinder class
class ModelBlobfinder : public Model
{
private:
  GArray* colors;
  GArray* blobs;

  // predicate for ray tracing
  static bool BlockMatcher( Block* testblock, Model* finder );

  static Option showBlobData;
	
  virtual void DataVisualize( Camera* cam );

public:
  unsigned int scan_width;
  unsigned int scan_height;
  stg_meters_t range;
  stg_radians_t fov;
  stg_radians_t pan;

  static  const char* typestr;

  // constructor
  ModelBlobfinder( World* world,
		   Model* parent );
  // destructor
  ~ModelBlobfinder();

  virtual void Startup();
  virtual void Shutdown();
  virtual void Update();
  virtual void Load();

  stg_blobfinder_blob_t* GetBlobs( unsigned int* count )
  { 
    if( count ) *count = blobs->len;
    return (stg_blobfinder_blob_t*)blobs->data;
  }

  /** Start finding blobs with this color.*/
  void AddColor( stg_color_t col );

  /** Stop tracking blobs with this color */
  void RemoveColor( stg_color_t col );

  /** Stop tracking all colors. Call this to clear the defaults, then
      add colors individually with AddColor(); */
  void RemoveAllColors();
};




// LASER MODEL --------------------------------------------------------

/** laser sample packet
 */
typedef struct
{
  stg_meters_t range; ///< range to laser hit in meters
  double reflectance; ///< intensity of the reflection 0.0 to 1.0
} stg_laser_sample_t;

typedef struct
{
  uint32_t sample_count;
  uint32_t resolution;
  Bounds range_bounds;
  stg_radians_t fov;
  stg_usec_t interval;
} stg_laser_cfg_t;

/// %ModelLaser class
class ModelLaser : public Model
{
private:
  /** OpenGL displaylist for laser data */
  int data_dl; 
  bool data_dirty;

  stg_laser_sample_t* samples;
  uint32_t sample_count;
  stg_meters_t range_min, range_max;
  stg_radians_t fov;
  uint32_t resolution;
  
  static Option showLaserData;
  static Option showLaserStrikes;
  
public:
  static const char* typestr;
  // constructor
  ModelLaser( World* world,
	      Model* parent ); 
  
  // destructor
  ~ModelLaser();
  
  virtual void Startup();
  virtual void Shutdown();
  virtual void Update();
  virtual void Load();
  virtual void Print( char* prefix );
  virtual void DataVisualize( Camera* cam );
  
  uint32_t GetSampleCount(){ return sample_count; }
  
  stg_laser_sample_t* GetSamples( uint32_t* count=NULL);
  
  void SetSamples( stg_laser_sample_t* samples, uint32_t count);
  
  // Get the user-tweakable configuration of the laser
  stg_laser_cfg_t GetConfig( );
  
  // Set the user-tweakable configuration of the laser
  void SetConfig( stg_laser_cfg_t cfg );  
};

// \todo  GRIPPER MODEL --------------------------------------------------------

//   typedef enum {
//     STG_GRIPPER_PADDLE_OPEN = 0, // default state
//     STG_GRIPPER_PADDLE_CLOSED, 
//     STG_GRIPPER_PADDLE_OPENING,
//     STG_GRIPPER_PADDLE_CLOSING,
//   } stg_gripper_paddle_state_t;

//   typedef enum {
//     STG_GRIPPER_LIFT_DOWN = 0, // default state
//     STG_GRIPPER_LIFT_UP, 
//     STG_GRIPPER_LIFT_UPPING, // verbed these to match the paddle state
//     STG_GRIPPER_LIFT_DOWNING, 
//   } stg_gripper_lift_state_t;

//   typedef enum {
//     STG_GRIPPER_CMD_NOP = 0, // default state
//     STG_GRIPPER_CMD_OPEN, 
//     STG_GRIPPER_CMD_CLOSE,
//     STG_GRIPPER_CMD_UP, 
//     STG_GRIPPER_CMD_DOWN    
//   } stg_gripper_cmd_type_t;

//   /** gripper configuration packet
//    */
//   typedef struct
//   {
//     Size paddle_size; ///< paddle dimensions 

//     stg_gripper_paddle_state_t paddles;
//     stg_gripper_lift_state_t lift;

//     double paddle_position; ///< 0.0 = full open, 1.0 full closed
//     double lift_position; ///< 0.0 = full down, 1.0 full up

//     stg_meters_t inner_break_beam_inset; ///< distance from the end of the paddle
//     stg_meters_t outer_break_beam_inset; ///< distance from the end of the paddle  
//     stg_bool_t paddles_stalled; // true iff some solid object stopped
// 				// the paddles closing or opening

//     GSList *grip_stack;  ///< stack of items gripped
//     int grip_stack_size; ///< maximum number of objects in stack, or -1 for unlimited

//     double close_limit; ///< How far the gripper can close. If < 1.0, the gripper has its mouth full.

//   } stg_gripper_config_t;

//   /** gripper command packet
//    */
//   typedef struct
//   {
//     stg_gripper_cmd_type_t cmd;
//     int arg;
//   } stg_gripper_cmd_t;


//   /** gripper data packet
//    */
//   typedef struct
//   {
//     stg_gripper_paddle_state_t paddles;
//     stg_gripper_lift_state_t lift;

//     double paddle_position; ///< 0.0 = full open, 1.0 full closed
//     double lift_position; ///< 0.0 = full down, 1.0 full up

//     stg_bool_t inner_break_beam; ///< non-zero iff beam is broken
//     stg_bool_t outer_break_beam; ///< non-zero iff beam is broken

//     stg_bool_t paddle_contacts[2]; ///< non-zero iff paddles touch something

//     stg_bool_t paddles_stalled; // true iff some solid object stopped
// 				// the paddles closing or opening

//     int stack_count; ///< number of objects in stack


//   } stg_gripper_data_t;


// \todo BUMPER MODEL --------------------------------------------------------

//   typedef struct
//   {
//     Pose pose;
//     stg_meters_t length;
//   } stg_bumper_config_t;

//   typedef struct
//   {
//     Model* hit;
//     stg_point_t hit_point;
//   } stg_bumper_sample_t;


// FIDUCIAL MODEL --------------------------------------------------------

/** fiducial config packet 
 */
typedef struct
{
  stg_meters_t max_range_anon; //< maximum detection range
  stg_meters_t max_range_id; ///< maximum range at which the ID can be read
  stg_meters_t min_range; ///< minimum detection range
  stg_radians_t fov; ///< field of view 
  stg_radians_t heading; ///< center of field of view

  /// only detects fiducials with a key string that matches this one
  /// (defaults to NULL)
  int key;
} stg_fiducial_config_t;

/** fiducial data packet 
 */
typedef struct
{
  stg_meters_t range; ///< range to the target
  stg_radians_t bearing; ///< bearing to the target 
  Pose geom; ///< size and relative angle of the target
  Pose pose; ///< Absolute accurate position of the target in world coordinates (it's cheating to use this in robot controllers!)
  int id; ///< the identifier of the target, or -1 if none can be detected.

} stg_fiducial_t;

/// %ModelFiducial class
class ModelFiducial : public Model
{
private:
  // if neighbor is visible, add him to the fiducial scan
  void AddModelIfVisible( Model* him );

  // static wrapper function can be used as a function pointer
  static int AddModelIfVisibleStatic( Model* him, ModelFiducial* me )
  { if( him != me ) me->AddModelIfVisible( him ); return 0; }

  virtual void Update();
  virtual void DataVisualize( Camera* cam );

  GArray* data;
	
  static Option showFiducialData;
	
public:
  static const char* typestr;
  // constructor
  ModelFiducial( World* world,
		 Model* parent );
  // destructor
  virtual ~ModelFiducial();
  
  virtual void Load();
  void Shutdown( void );

  stg_meters_t max_range_anon; //< maximum detection range
  stg_meters_t max_range_id; ///< maximum range at which the ID can be read
  stg_meters_t min_range; ///< minimum detection range
  stg_radians_t fov; ///< field of view 
  stg_radians_t heading; ///< center of field of view
  int key; ///< /// only detect fiducials with a key that matches this one (defaults 0)

  stg_fiducial_t* fiducials; ///< array of detected fiducials
  uint32_t fiducial_count; ///< the number of fiducials detected
};


// RANGER MODEL --------------------------------------------------------

typedef struct
{
  Pose pose;
  Size size;
  Bounds bounds_range;
  stg_radians_t fov;
  int ray_count;
} stg_ranger_sensor_t;

/// %ModelRanger class
class ModelRanger : public Model
{
protected:

  virtual void Startup();
  virtual void Shutdown();
  virtual void Update();
  virtual void DataVisualize( Camera* cam );

public:
  static const char* typestr;
  // constructor
  ModelRanger( World* world,
	       Model* parent );
  // destructor
  virtual ~ModelRanger();

  virtual void Load();
  virtual void Print( char* prefix );

  uint32_t sensor_count;
  stg_ranger_sensor_t* sensors;
  stg_meters_t* samples;
	
private:
  static Option showRangerData;
  static Option showRangerTransducers;
	
};

// BLINKENLIGHT MODEL ----------------------------------------------------
class ModelBlinkenlight : public Model
{
private:
  double dutycycle;
  bool enabled;
  stg_msec_t period;
  bool on;

  static Option showBlinkenData;
public:

  static const char* typestr;
  ModelBlinkenlight( World* world,
		     Model* parent );

  ~ModelBlinkenlight();

  virtual void Load();
  virtual void Update();
  virtual void DataVisualize( Camera* cam );
};

// CAMERA MODEL ----------------------------------------------------
typedef struct {
  // GL_V3F
  GLfloat x, y, z;
} ColoredVertex;
	
/// %ModelCamera class
class ModelCamera : public Model
{
private:
  Canvas* _canvas;

  GLfloat* _frame_data;  //opengl read buffer
  GLubyte* _frame_color_data;  //opengl read buffer

  bool _valid_vertexbuf_cache;
  ColoredVertex* _vertexbuf_cache; //cached unit vectors with appropriate rotations (these must be scalled by z-buffer length)
	
  int _width;         //width of buffer
  int _height;        //height of buffer
  static const int _depth = 4;
	
  int _camera_quads_size;
  GLfloat* _camera_quads;
  GLubyte* _camera_colors;
	
  static Option showCameraData;
	
  PerspectiveCamera _camera;
  float _yaw_offset; //position camera is mounted at
  float _pitch_offset;
		
  ///Take a screenshot from the camera's perspective. return: true for sucess, and data is available via FrameDepth() / FrameColor()
  bool GetFrame();
	
public:
  ModelCamera( World* world,
	       Model* parent ); 

  ~ModelCamera();

  virtual void Load();
	
  ///Capture a new frame ( calls GetFrame )
  virtual void Update();
	
  ///Draw Camera Model - TODO
  //virtual void Draw( uint32_t flags, Canvas* canvas );
	
  ///Draw camera visualization
  virtual void DataVisualize( Camera* cam );
	
  ///width of captured image
  inline int getWidth( void ) const { return _width; }
	
  ///height of captured image
  inline int getHeight( void ) const { return _height; }
	
  ///get reference to camera used
  inline const PerspectiveCamera& getCamera( void ) const { return _camera; }
	
  ///get a reference to camera depth buffer
  inline const GLfloat* FrameDepth() const { return _frame_data; }
	
  ///get a reference to camera color image. 3 bytes (RGB) per pixel
  inline const GLubyte* FrameColor() const { return _frame_color_data; }
	
  ///change the pitch
  inline void setPitch( float pitch ) { _pitch_offset = pitch; _valid_vertexbuf_cache = false; }
	
  ///change the yaw
  inline void setYaw( float yaw ) { _yaw_offset = yaw; _valid_vertexbuf_cache = false; }
};

// POSITION MODEL --------------------------------------------------------

/** Define a position  control method */
typedef enum
  { STG_POSITION_CONTROL_VELOCITY, 
    STG_POSITION_CONTROL_POSITION 
  } stg_position_control_mode_t;

/** Define a localization method */
typedef enum
  { STG_POSITION_LOCALIZATION_GPS, 
    STG_POSITION_LOCALIZATION_ODOM 
  } stg_position_localization_mode_t;

/** Define a driving method */
typedef enum
  { STG_POSITION_DRIVE_DIFFERENTIAL, 
    STG_POSITION_DRIVE_OMNI, 
    STG_POSITION_DRIVE_CAR 
  } stg_position_drive_mode_t;


/// %ModelPosition class
class ModelPosition : public Model
{
  friend class Canvas;

private:
  Pose goal; //< the current velocity or pose to reach, depending on the value of control_mode
  stg_position_control_mode_t control_mode;
  stg_position_drive_mode_t drive_mode;
  stg_position_localization_mode_t localization_mode; ///< global or local mode
  Velocity integration_error; ///< errors to apply in simple odometry model

  Waypoint* waypoints;
  uint32_t waypoint_count;	 
  void DrawWaypoints();

private:
  static Option showCoords;
  static Option showWaypoints;

public:
  static const char* typestr;
  // constructor
  ModelPosition( World* world,
		 Model* parent );
  // destructor
  ~ModelPosition();

  virtual void Startup();
  virtual void Shutdown();
  virtual void Update();
  virtual void Load();
	 
  virtual void DataVisualize( Camera* cam );

  /** Set the waypoint array pointer. Returns the old pointer, in case you need to free/delete[] it */
  Waypoint* SetWaypoints( Waypoint* wps, uint32_t count );
	
  /** Set the current pose estimate.*/
  void SetOdom( Pose odom );

  /** Sets the control_mode to STG_POSITION_CONTROL_VELOCITY and sets
      the goal velocity. */
  void SetSpeed( double x, double y, double a );
  void SetXSpeed( double x );
  void SetYSpeed( double y );
  void SetZSpeed( double z );
  void SetTurnSpeed( double a );
  void SetSpeed( Velocity vel );

  /** Sets the control mode to STG_POSITION_CONTROL_POSITION and sets
      the goal pose */
  void GoTo( double x, double y, double a );
  void GoTo( Pose pose );

  // localization state
  Pose est_pose; ///< position estimate in local coordinates
  Pose est_pose_error; ///< estimated error in position estimate
  Pose est_origin; ///< global origin of the local coordinate system
};

#endif // MODEL_H

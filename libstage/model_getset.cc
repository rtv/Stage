#include "stage.hh"
using namespace Stg;


void Model::SetGeom( const Geom& val )
{
  UnMapWithChildren();
  
  geom = val;
  
  blockgroup.CalcSize();
  
  NeedRedraw();
  
  MapWithChildren();
  
  CallCallbacks( CB_GEOM );
}

void Model::SetColor( Color val )
{
  color = val;
  NeedRedraw();
}

void Model::SetMass( stg_kg_t val )
{
  mass = val;
}

void Model::SetStall( stg_bool_t val )
{
  stall = val;
}

void Model::SetGripperReturn( int val )
{
  vis.gripper_return = val;
}

void Model::SetFiducialReturn(  int val )
{
  vis.fiducial_return = val;
  
  // non-zero values mean we need to be in the world's set of
  // detectable models
  if( val == 0 )
	 world->FiducialErase( this );
  else
	 world->FiducialInsert( this );
}

void Model::SetFiducialKey( int val )
{
  vis.fiducial_key = val;
}

void Model::SetLaserReturn( stg_laser_return_t val )
{
  vis.laser_return = val;
}

void Model::SetObstacleReturn( int val )
{
  vis.obstacle_return = val;
}

void Model::SetBlobReturn( int val )
{
  vis.blob_return = val;
}

void Model::SetRangerReturn( int val )
{
  vis.ranger_return = val;
}

void Model::SetBoundary( int val )
{
  boundary = val;
}

void Model::SetGuiNose(  int val )
{
  gui.nose = val;
}

void Model::SetGuiMove( int val )
{
  gui.move = val;
}

void Model::SetGuiGrid(  int val )
{
  gui.grid = val;
}

void Model::SetGuiOutline( int val )
{
  gui.outline = val;
}

void Model::SetWatts( stg_watts_t val )
{
  watts = val;
}

void Model::SetMapResolution(  stg_meters_t val )
{
  map_resolution = val;
}

// set the pose of model in global coordinates 
void Model::SetGlobalPose( const Pose& gpose )
{
  SetPose( parent ? parent->GlobalToLocal( gpose ) : gpose );
}

int Model::SetParent( Model* newparent)
{  
  // remove the model from its old parent (if it has one)
  if( parent )
	 EraseAll( this, parent->children );
  
  if( newparent )
	 newparent->children.push_back( this );

  // link from the model to its new parent
  this->parent = newparent;

	CallCallbacks( CB_PARENT );
  return 0; //ok
}

// get the model's velocity in the global frame
Velocity Model::GetGlobalVelocity() const
{
  Pose gpose = GetGlobalPose();
  
  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );

  Velocity gv;
  gv.x = velocity.x * cosa - velocity.y * sina;
  gv.y = velocity.x * sina + velocity.y * cosa;
  gv.a = velocity.a;

  return gv;
}

// set the model's velocity in the global frame
void Model::SetGlobalVelocity( const Velocity& gv )
{
  Pose gpose = GetGlobalPose();

  double cosa = cos( gpose.a );
  double sina = sin( gpose.a );

  Velocity lv;
  lv.x = gv.x * cosa + gv.y * sina;
  lv.y = -gv.x * sina + gv.y * cosa;
  lv.a = gv.a;

  this->SetVelocity( lv );
}

// get the model's position in the global frame
Pose Model::GetGlobalPose() const
{ 
  // if I'm a top level model, my global pose is my local pose
  if( parent == NULL )
    return pose;
  
  // otherwise    
  Pose global_pose = parent->GetGlobalPose() + pose;		
  
  if ( parent->stack_children ) // should we be on top of our parent?
    global_pose.z += parent->geom.size.z;
  
  return global_pose;
}

void Model::VelocityEnable()
{
	velocity_enable = true;
	world->active_velocity.insert( this );
}

void Model::VelocityDisable()
{
	velocity_enable = false;
	world->active_velocity.erase( this );
}

void Model::SetVelocity( const Velocity& val )
{
  velocity = val;  
	CallCallbacks( CB_VELOCITY );
}


// set the model's pose in the local frame
void Model::SetPose( const Pose& newpose )
{
  // if the pose has changed, we need to do some work
  if( memcmp( &pose, &newpose, sizeof(Pose) ) != 0 )
    {
      pose = newpose;
      pose.a = normalize(pose.a);

//       if( isnan( pose.a ) )
// 		  printf( "SetPose bad angle %s [%.2f %.2f %.2f %.2f]\n",
// 					 token, pose.x, pose.y, pose.z, pose.a );
			
      NeedRedraw();
      MapWithChildren();
      world->dirty = true;
    }
	
	CallCallbacks( CB_POSE );
}

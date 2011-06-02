#include "stage.hh"
using namespace Stg;


void Model::SetGeom( const Geom& val )
{  
  UnMapWithChildren(0);
  UnMapWithChildren(1);
  
  geom = val;
  
  blockgroup.CalcSize();
  
  NeedRedraw();
  
  MapWithChildren(0);
  MapWithChildren(1);
    
  CallCallbacks( CB_GEOM );
}

void Model::SetColor( Color val )
{
  color = val;
  NeedRedraw();
}

void Model::SetMass( kg_t val )
{
  mass = val;
}

void Model::SetStall( bool val )
{
  stall = val;
}

void Model::SetGripperReturn( bool val )
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

void Model::SetObstacleReturn( bool val )
{
  vis.obstacle_return = val;
}

void Model::SetBlobReturn( bool val )
{
  vis.blob_return = val;
}

void Model::SetRangerReturn( float val )
{
  vis.ranger_return = val;
}

void Model::SetBoundary( bool val )
{
  boundary = val;
}

void Model::SetGuiNose(  bool val )
{
  gui.nose = val;
}

void Model::SetGuiMove( bool val )
{
  gui.move = val;
}

void Model::SetGuiGrid(  bool val )
{
  gui.grid = val;
}

void Model::SetGuiOutline( bool val )
{
  gui.outline = val;
}

void Model::SetWatts( watts_t val )
{
  watts = val;
}

void Model::SetMapResolution(  meters_t val )
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
  Pose oldPose = GetGlobalPose();

  // remove the model from its old parent (if it has one)
  if( parent )
    parent->RemoveChild( this );
  else
    world->RemoveChild( this );
  // link from the model to its new parent
  this->parent = newparent;
  
  if( newparent )
    newparent->AddChild( this );
  else
    world->AddModel( this );

  CallCallbacks( CB_PARENT );

  SetGlobalPose( oldPose ); // Needs to recalculate position due to change in parent
  
  return 0; //ok
}

// get the model's velocity in the global frame
Velocity Model::GetGlobalVelocity() const
{
  const Pose gpose(GetGlobalPose());  
  const double cosa(cos(gpose.a));
  const double sina(sin(gpose.a));
  
  return Velocity( velocity.x * cosa - velocity.y * sina,
		   velocity.x * sina + velocity.y * cosa,
		   velocity.z,
		   velocity.a );
}

// set the model's velocity in the global frame
void Model::SetGlobalVelocity( const Velocity& gv )
{
  const Pose gpose = GetGlobalPose();  
  const double cosa(cos(gpose.a));
  const double sina(sin(gpose.a));
  
  this->SetVelocity( Velocity( gv.x * cosa + gv.y * sina,
			       -gv.x * sina + gv.y * cosa,
			       gv.z,
			       gv.a ));
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
  world->EnableVelocity(this);
}

void Model::VelocityDisable()
{
  velocity_enable = false;
  world->DisableVelocity( this );
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

      UnMapWithChildren(0);
      UnMapWithChildren(1);

      MapWithChildren(0);
      MapWithChildren(1);

      world->dirty = true;
    }
	
  CallCallbacks( CB_POSE );
}

///////////////////////////////////////////////////////////////////////////
//
// File: puck.cc
// Author: brian gerkey, Richard Vaughan
// Date: 4 Dec 2000
// Desc: Simulates the pucks
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/models/puck.cc,v $
//  $Author: rtv $
//  $Revision: 1.3.6.3 $
//
///////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include "puck.hh"
#include "raytrace.hh"

///////////////////////////////////////////////////////////////////////////
// Default constructor
//
CPuck::CPuck( int id,  char* token, char* color, CEntity *parent)
  : CEntity( id, token, color, parent)
{
  this->vision_return = true;
  this->vision_return_held = false;
  this->vision_return_notheld = true;
  this->gripper_return = GripperEnabled;
  this->obstacle_return = false; // we don't stop robots
  this->puck_return = true; // yes! we interact with pucks!
  
  // Set default geometry
  this->size_x = 0.08;
  this->size_y = 0.08;
  
  this->m_interval = 0.01; // update very fast!
  this->m_friction = 0.05;
  
  // assume puck is 200g
  this->mass = 0.2;
}

///////////////////////////////////////////////////////////////////////////
// Update the puck
int CPuck::Update( void )
{
  CEntity::Update();

  assert( m_parent_entity ); // everyone has a parent these days
  
  // have we been picked up?  then make ourselves transparent so the vision
  // doesn't see us in the gripper. otherwise make ourselves visible.
  if(m_parent_entity != CEntity::root ) // root is the top-level entity
    this->vision_return = this->vision_return_held;
  else
    this->vision_return = this->vision_return_notheld;
  
  // if its time to recalculate state and we're on the ground
  //
  if( (CEntity::simtime - m_last_update > m_interval) 
      && (m_parent_entity == CEntity::root) )
    {	
      m_last_update = CEntity::simtime;
      
      PuckMove();      
    }
  
  //double x, y, th;
  //GetGlobalPose( x,y,th );
  //ReMap(x, y, th);
  return 0; //
}


///////////////////////////////////////////////////////////////////////////
// Move the puck
void CPuck::PuckMove()
{
  // TODO - FIX THIS
  double step_time = 0.1;//CEntity::timestep;
  
  // don't move if we've been picked up
  if(m_parent_entity != CEntity::root ) return;
    
  // Get the current puck pose
  double px, py, pth;
  GetGlobalPose(px, py, pth);
    
  CCircleIterator cit( px, py, this->size_x/2.0, CEntity::matrix );
  CEntity* ent;

  while( (ent = cit.GetNextEntity()) )
  {
    if( ent && ent != this )
    {
      if( ent->puck_return ) 
      {
        // Get the position of the thing we hit
        double ox,oy,oth;
	      ent->GetGlobalPose(ox,oy,oth);
        
        // Get the velocity of the thing we hit
        double wx, wy, wth;
        ent->GetGlobalVel(wx, wy, wth);
        double impact_velocity = sqrt(wx * wx + wy * wy);
        
        // Get the mass of the thing we hit
        double impact_mass = ent->GetMass();

        // Compute bearing FROM impacted ent
        // Align ourselves so we are facing away from this point
        pth = atan2( py-oy, px-ox );
	      
        if(impact_velocity)
        {
          // we only have really big things(robots) and really small things
          // (pucks).  
          double vr = 0;
          if(impact_mass > this->mass)
            vr = 2*fabs(impact_velocity);
          else
            vr = fabs(impact_velocity);
          SetGlobalVel(vr * cos(pth), vr * sin(pth), 0);
        }
      }
      else if( ent->obstacle_return )
      {
        // If we have collided, stop moving
        SetGlobalVel(0, 0, 0);
        // REMOVE return; // don;t move at all
      }
    }
  }

  // Get our current vel
  double vx, vy, vth;
  GetGlobalVel(vx, vy, vth);
  double vr = sqrt(vx * vx + vy * vy);
  
  // lower bound on velocity
  if (vr < 0.01 )
    SetGlobalVel(0, 0, 0);
        
  if( vr != 0 ) // only move if we have some speed  
  {
    double qx = px + step_time * vx;
    double qy = py + step_time * vy;
    double qth = pth;
    // set pose now takes care of marking us dirty to clients
    SetGlobalPose( qx, qy, qth );

    // compute a new velocity, based on "friction"
    vr -= vr * m_friction;
    vx = vr * cos(pth);
    vy = vr * sin(pth);
    SetGlobalVel( vx, vy, vth);
  }    
}






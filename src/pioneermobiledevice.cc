// 	$Id: pioneermobiledevice.cc,v 1.1 2000-11-30 00:29:25 vaughan Exp $	

#include "world.h"
#include "robot.h"
#include "pioneermobiledevice.h"

const double TWOPI = 6.283185307;

// constructor

CPioneerMobileDevice::CPioneerMobileDevice( void *buffer, 
					    size_t data_len, 
					    size_t command_len, 
					    size_t config_len)
        : CDevice(buffer, data_len, command_len, config_len)
{
  m_update_interval = 0.01 // update me very fast indeed

  width = w;
  length = l;

  halfWidth = width / 2.0;   // the halves are used often in calculations
  halfLength = length / 2.0;

  xorigin = oldx = x = startx;
  yorigin = oldy = y = starty;
  aorigin = olda = a = starta;

  xodom = yodom = aodom = 0;

  speed = 0.0;
  turnRate = 0.0;

  stall = 0;

  memset( &rect, 0, sizeof( rect ) );
  memset( &oldRect, 0, sizeof( rect ) );

  StoreRect();

}

///////////////////////////////////////////////////////////////////////////
// Default startup -- doesnt do much
//
bool CPioneerMobileDevice::Startup(CRobot *robot, CWorld *world)
  :Startup( robot, world )
{

  color = id+1;


    return true;
}

///////////////////////////////////////////////////////////////////////////
// Update the position of the robot base
//
bool CPioneerMobileDevice::Update()
{
    TRACE0("updating CPioneerMobileDevice");
    ASSERT(m_robot != NULL);
    ASSERT(m_world != NULL);
    
    // find out what to do
    ParseCommandBuffer();
    
    // do things
    Move( m_world->img );
    
    // report the new state of things
    ComposeDataBuffer();
    
    PutData( 

    return true;
}


int CPioneerMobileDevice::Move( Nimage* img )
{
  int moved = false;

  if( ( win->dragging != this ) &&  (speed != 0.0 || turnRate != 0.0) )
    {
      // record speeds now - they can be altered in another thread
      float nowSpeed = speed;
      float nowTurn = turnRate;
      float nowTimeStep = world->timeStep;

      // find the new position for the robot
      float tx = x + nowSpeed * world->ppm * cos( a ) * nowTimeStep;
      float ty = y + nowSpeed * world->ppm * sin( a ) * nowTimeStep;
      float ta = a + (nowTurn * nowTimeStep);
      ta = fmod( ta + TWOPI, TWOPI );  // normalize angle

      // calculate the rectangle for the robot's tentative new position
      CalculateRect( tx, ty, ta );

      // trace the robot's outline to see if it will hit anything
      char hit = 0;
      if( hit = img->rect_detect( rect, color ) > 0 )
	// hit! so don't do the move - just redraw the robot where it was
	{
	  //cout << "HIT! " << endl;
	  memcpy( &rect, &oldRect, sizeof( struct Rect ) );
	  Draw( img );
	  
	  stall = 1; // motors stalled due to collision
	}
      else // do the move
	{
	  moved = true;
	  
	  stall = 0; // motors not stalled
	  
	  x = tx; // move to the new position
	  y = ty;
	  a = ta;
	  
	  //update the robot's odometry estimate
	  xodom +=  nowSpeed * world->ppm * cos( aodom ) * nowTimeStep;
	  yodom +=  nowSpeed * world->ppm * sin( aodom ) * nowTimeStep;
	  
	  if( world->maxAngularError != 0.0 ) //then introduce some error
	    {
	      float error = 1.0 + ( drand48()* world->maxAngularError );
	      aodom += nowTurn * nowTimeStep * error;
	    }
	  else
	    aodom += nowTurn * nowTimeStep;
	  
	  aodom = fmod( aodom + TWOPI, TWOPI );  // normalize angle
	  
	  // erase and redraw the robot in the world bitmap
	  UnDraw( img );
	  Draw( img );
	  
	  // erase and redraw the robot on the X display
	  if( win ) win->DrawRobotIfMoved( this );
	  
	  // update the `old' stuff
	  memcpy( &oldRect, &rect, sizeof( struct Rect ) );
	  oldCenterx = centerx;
	  oldCentery = centery;
	}
    }
  
  oldx = x;
  oldy = y;
  olda = a;
  
  return moved;
}

void CPioneerMobileDevice::PutData( void )
{
  // this will put the data into P2OS packet format to be shipped
  // to Player

  // copy position data into the memory buffer shared with player

  double rtod = 180.0/M_PI;

  unsigned char* deviceData = (unsigned char*)(playerIO +
P2OS_DATA_START);

  //printf( "arena: playerIO: %d   devicedata: %d offset: %d\n", playerIO,
  // deviceData, (int)deviceData - (int)playerIO );

  // COPY DATA
  // position device
  *((int*)(deviceData + 0)) =
    htonl((int)((world->timeNow - world->timeBegan) * 1000.0));
  *((int*)(deviceData + 4)) = htonl((int)((x - xorigin)/ world->ppm *
1000.0));
  *((int*)(deviceData + 8)) = htonl((int)((y - yorigin)/ world->ppm *
1000.0));

  // odometry heading
  float odoHeading = fmod( aorigin -a + TWOPI, TWOPI );  // normalize angle
  
  *((unsigned short*)(deviceData + 12)) =
    htons((unsigned short)(odoHeading * rtod ));

  // speeds
  *((unsigned short*)(deviceData + 14))
    = htons((unsigned short)speed);

  *((unsigned short*)(deviceData + 16))
    = htons((unsigned short)(turnRate * rtod ));

  // compass heading
  float comHeading = fmod( a + M_PI/2.0 + TWOPI, TWOPI );  // normalize angle
  
  *((unsigned short*)(deviceData + 18))
    = htons((unsigned short)( comHeading * rtod ));

  // stall - currently shows if the robot is being dragged by the user
  *(unsigned char*)(deviceData + 20) = stall;

}


int CPioneerMobileDevice::HasMoved( void )
   {
     //if the robot has moved on the world bitmap
  if( oldRect.toplx != rect.toplx ||
      oldRect.toply != rect.toply ||
      oldRect.toprx != rect.toprx ||
      oldRect.topry != rect.topry ||
      oldRect.botlx != rect.botlx ||
      oldRect.botly != rect.botly ||
      oldRect.botrx != rect.botrx ||
      oldRect.botry != rect.botry  ||
      centerx != oldCenterx ||
      centery != oldCentery )

    return true;
  else
    return false;
}

void CPioneerMobileDevice::UnDraw( Nimage* img )
{
  //undraw the robot's old rectangle
  img->draw_line( oldRect.toplx, oldRect.toply,
    oldRect.toprx, oldRect.topry, 0 );
  img->draw_line( oldRect.toprx, oldRect.topry,
    oldRect.botlx, oldRect.botly, 0 );
  img->draw_line( oldRect.botlx, oldRect.botly,
    oldRect.botrx, oldRect.botry, 0 );
  img->draw_line( oldRect.botrx, oldRect.botry,
    oldRect.toplx, oldRect.toply, 0 );
}


void CPioneerMobileDevice::Draw( Nimage* img )
{
  //draw the robot's rectangle
  img->draw_line( rect.toplx, rect.toply,
    rect.toprx, rect.topry, color );
  img->draw_line( rect.toprx, rect.topry,
    rect.botlx, rect.botly, color );
  img->draw_line( rect.botlx, rect.botly,
    rect.botrx, rect.botry, color );
  img->draw_line( rect.botrx, rect.botry,
    rect.toplx, rect.toply, color );
}

void CPioneerMobileDevice::Stop( void )
{
  speed = turnRate = 0;
}

void CPioneerMobileDevice::StoreRect( void )
{
  memcpy( &oldRect, &rect, sizeof( struct Rect ) );
  oldCenterx = centerx;
  oldCentery = centery;
}

void CPioneerMobileDevice:CalculateRect( float x, float y, float a )
{
  // fill an array of Points with the corners of the robots new position
  float cosa = cos( a );
  float sina = sin( a );
  float cxcosa = halfLength * cosa;
  float cycosa = halfWidth  * cosa;
  float cxsina = halfLength * sina;
  float cysina = halfWidth  * sina;

  rect.toplx = (int)(x + (-cxcosa + cysina));
  rect.toply = (int)(y + (-cxsina - cycosa));
  rect.toprx = (int)(x + (+cxcosa + cysina));
  rect.topry = (int)(y + (+cxsina - cycosa));
  rect.botlx = (int)(x + (+cxcosa - cysina));
  rect.botly = (int)(y + (+cxsina + cycosa));
  rect.botrx = (int)(x + (-cxcosa - cysina));
  rect.botry = (int)(y + (-cxsina + cycosa));

  centerx = (int)x;
  centery = (int)y;
}

void CPioneerMobileDevice::ParseCommandBuffer( void )
{
  // this will parse the P2OS command buffer to find out what to do

  // read the command buffer and set the robot's speeds
  short v = *(short*)(playerIO + P2OS_COMMAND_START );
  short w = *(short*)(playerIO + P2OS_COMMAND_START + 2 );

  // unlock
  world->UnlockShmem();

  float fv = (float)(short)ntohs(v);//1000.0 * (float)ntohs(v);
  float fw = (float)(short)ntohs(w);//M_PI/180) * (float)ntohs(w);

  // set speeds unless we're being dragged
  if( win->dragging != this )
    {
      speed = 0.001 * fv;
      turnRate = -(M_PI/180.0) * fw;
    }

}



#include <X11/keysym.h> 
#include <X11/keysymdef.h> 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "stage_types.hh"
#include "truthserver.hh"
#include "xs.hh"

bool CXGui::PoseFromId( int stage_id, 
			double& x, double& y, double& th, unsigned long& col )
{
  TruthMap::iterator it;
  // find this object
  for( it = truth_map.begin(); it != truth_map.end(); it++ )
    if( it->second.stage_id == stage_id )
      {
	GetGlobalPose( it->second, x, y, th );
	
	col = it->second.pixel_color;
	
	//printf( "object %d using color %ld\n", stage_id, col );

	return true;
      }
  return false;
}
    

// Get the objects pose in the global cs
//
void CXGui::GetGlobalPose( xstruth_t &truth, double &px, double &py, double &pth)
{
    // Get the pose of our parent in the global cs
    //
    double ox = 0;
    double oy = 0;
    double oth = 0;

    // if we have a parent
    if( truth.parent_id != -1 )
      {
	// find the parent
	TruthMap::iterator it;
	for( it = truth_map.begin(); it != truth_map.end(); it++ )
	  if( it->second.stage_id == truth.parent_id )
	    {
	      GetGlobalPose( it->second, ox, oy, oth);
	      break;
	    }
      }
    
    // Compute our pose in the global cs
    //
    px = ox + truth.x * cos(oth) - truth.y * sin(oth);
    py = oy + truth.x * sin(oth) + truth.y * cos(oth);
    pth = oth + truth.th;
}

void CXGui::RenderObject( xstruth_t &orig_truth )
  {
    xstruth_t truth;
    
    // copy the original truth
    memcpy( &truth, &orig_truth, sizeof(xstruth_t) );
    
    // and replace it's local with global coordinates
    
    double x, y, th;
    GetGlobalPose( truth, x, y, th );
    
    truth.x = x;
    truth.y = y;
    truth.th = th;


#ifdef DEBUG
    printf( "rendering %d at %.4f %.4f %.4f\n", 
	    truth.stage_id, truth.x, truth.y, truth.th );
#endif

    bool extended = false;

    SetDrawMode( GXxor );

    // check the object type and call the appropriate render function 
    switch( truth.stage_type )
      {
      case NullType: break; // do nothing 
      case RectRobotType: RenderRectRobot( &truth, extended ); break; 	
      case RoundRobotType: RenderRoundRobot( &truth, extended ); break; 
      case LaserTurretType: RenderLaserTurret( &truth, extended ); break;
      case BoxType: RenderBox( &truth, extended ); break;
      case LaserBeaconType: RenderLaserBeacon( &truth, extended ); break;
      case PtzType: RenderPTZ( &truth, extended ); break;  
      case VisionBeaconType: RenderVisionBeacon( &truth, extended ); break;  
      case GripperType: RenderGripper( &truth, extended ); break;  
      case PuckType: RenderPuck( &truth, extended ); break;  

      case VisionType: if( draw_all_devices )
	RenderVision( &truth, extended ); break;
      case PlayerType: if( draw_all_devices )
	RenderPlayer( &truth, extended ); break;
      case MiscType: if( draw_all_devices )
	RenderMisc( &truth , extended); break;
      case SonarType: if( draw_all_devices )
	RenderSonar( &truth, extended ); break;
      case LBDType: if( draw_all_devices )
	RenderLaserBeaconDetector( &truth, extended ); break;
      case GpsType: if( draw_all_devices )
	RenderGps( &truth, extended ); break;

#ifdef HRL_DEVICES
      case DescartesType: RenderRoundRobot( &truth, extended ); break; 
      case IDARType: if( draw_all_devices )
	RenderIDAR( &truth, extended ); break;	
#endif

      default: cout << "XS: unknown object type " 
		    << truth.id.type << endl;

      PrintMetricTruthVerbose( 0, truth );
      break;
      }

    SetDrawMode( GXcopy );    
  }

void CXGui::HeadingStick( xstruth_t* exp )
{
  double len = 0.5;

  double dx = len * cos( exp->th );
  double dy = len * sin( exp->th );

  DPoint d[2];

  d[0].x = exp->x;
  d[0].y = exp->y;

  d[1].x = exp->x + dx;
  d[1].y = exp->y + dy;


  DrawLines( d, 2 );
}


void CXGui::RenderObjectLabel( xstruth_t* exp, char* str, int len )
{
  if( str && strlen(str) > 0 )
    {
      //SetForeground( RGB(255,255,255) );
      //DrawString( exp->x + 0.6, exp->y - (8.0/ppm + exp->id.type * 11.0/ppm) , 
      //	  str, len );
    }
}

void CXGui::RenderLaserTurret( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );  
  DrawNoseBox( exp->x, exp->y, exp->w/2.0, exp->h/2.0, exp->th );
}


void CXGui::RenderLaserBeaconDetector( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawCircle( exp->x, exp->y, exp->w/2.0 );
}

void CXGui::RenderTruth( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawString( exp->x, exp->y, "Truth", 5 );
}

void CXGui::RenderOccupancy( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawString( exp->x, exp->y, "Occupancy", 9 ); 
}

void CXGui::RenderPTZ( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );    
  DrawNoseBox( exp->x, exp->y, exp->w/2.0, exp->h/2.0, exp->th );
}

void CXGui::RenderSonar( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );

  for( int l=0; l < PLAYER_NUM_SONAR_SAMPLES; l++ )
    {
      double xoffset, yoffset, angle;
      GetSonarPose( l, xoffset, yoffset, angle );
      
      angle += exp->th;
      
      double sonarx = exp->x + (xoffset*cos(exp->th) - yoffset*sin(exp->th));
      double sonary = exp->y + (xoffset*sin(exp->th) + yoffset*cos(exp->th));
      
      char buf[10];
      sprintf( buf, "%d", l );
      DrawString( sonarx, sonary, buf, strlen(buf ) ); 
      DrawNoseBox( sonarx, sonary, 0.02, 0.02, angle );
    }
}

void CXGui::RenderVision( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawCircle( exp->x, exp->y, exp->w/2.0 );
}

void CXGui::RenderPlayer( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );

  DPoint pts[8];
  GetRect( exp->x, exp->y, exp->w/2.0, exp->h/2.0, exp->th, pts );  
  
  DrawPolygon( pts, 4 );

  // draw an X across the box
  DrawLine( pts[0], pts[2] );
  DrawLine( pts[1], pts[3] );
}

void CXGui::RenderMisc( xstruth_t* exp, bool extended )
{
}


void CXGui::RenderRectRobot( xstruth_t* exp, bool extended )
 { 
   SelectColor( exp );
   
   double sinth = sin( exp->th );
   double costh = cos( exp->th );
   double dx = exp->rotdx * costh + exp->rotdy * sinth;
   double dy = exp->rotdx * sinth - exp->rotdy * costh;
   
   DPoint pts[7];
   GetRect( exp->x + dx, 
	    exp->y + dy,
	    exp->w/2.0, exp->h/2.0, exp->th, pts );
   
   pts[4].x = pts[0].x;
   pts[4].y = pts[0].y;
   
   pts[5].x = exp->x;
    pts[5].y = exp->y;
    
    pts[6].x = pts[3].x;
    pts[6].y = pts[3].y;
    
    DrawLines( pts, 7 );
 }

void CXGui::RenderLaserBeacon( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );  
  DrawNoseBox( exp->x, exp->y, exp->w/2.0, exp->h/2.0, exp->th );
}

void CXGui::RenderGripper( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );  
  DrawRect( exp->x, exp->y, exp->w/2.0, exp->h/2.0, exp->th );
  
  double x_offset, y_offset;
  double ox = exp->x;
  double oy = exp->y;
  double oth = exp->th;
  double w = exp->w/2.0;
  double h = exp->h/2.0;

  double pw = w;
  double ph = h/6.0;

  //if(expGripper.paddles_open)
  //    {
  x_offset = w + (pw);
  y_offset = h - (ph);
  //    }
  //    else
  //    {
  //      x_offset = (exp.width/2.0)+(expGripper.paddle_width/2.0);
  //      y_offset = (expGripper.paddle_height/2.0);
  //    }

  DrawRect( ox+(x_offset*cos(oth))+(y_offset*-sin(oth)),
	     oy+(x_offset*sin(oth))+(y_offset*cos(oth)),            
	     pw, ph, oth );
	   
   y_offset = -y_offset;

   DrawRect( ox+(x_offset*cos(oth))+(y_offset*-sin(oth)),
	     oy+(x_offset*sin(oth))+(y_offset*cos(oth)),
	     pw, ph, oth );
}

void CXGui::RenderRoundRobot( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawCircle( exp->x, exp->y, exp->w/2.0 );

  //DrawNose( exp->x, exp->y, exp->w/2.0, exp->th - 0.6 );
  //DrawNose( exp->x, exp->y, exp->w/2.0, exp->th + 0.6 );
  DrawNose( exp->x, exp->y, exp->w/2.0, exp->th );
}

void CXGui::RenderVisionBeacon( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawCircle( exp->x, exp->y, exp->w/2.0 );
  FillCircle( exp->x, exp->y, exp->w/3.0 );
}

void CXGui::RenderBroadcast( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawCircle( exp->x, exp->y, exp->w/2.0 );
}

void CXGui::RenderPuck( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  FillCircle( exp->x, exp->y, exp->w/2.0 );
}

void CXGui::RenderGps( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawCircle( exp->x, exp->y, exp->w/2.0 );
}

#ifdef HRL_DEVICES
void CXGui::RenderIDAR( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawCircle( exp->x, exp->y, exp->w/2.0 );
}
#endif

void CXGui::RenderBox( xstruth_t* exp, bool extended )
{ 
  SelectColor( exp );
  DrawRect( exp->x, exp->y, exp->w/2.0, exp->h/2.0, exp->th );
}

void CXGui::SelectColor( xstruth_t* exp )
{
  XSetForeground( display, gc, exp->pixel_color );
}

void CGraphicLaserProxy::ProcessData( void )
{
  // find the pose and color of the matching truth
  double x,y,th;
  
  if( !win->PoseFromId( stage_id, x, y, th, pixel ) )
    {
      printf( "XS Warning: couldn't find object (%d:%d:%d)"
	      " so abandoned render\n", 
	      client->port, device, index );
      return;
    }
   
  double start_angle = th + DTOR( min_angle/100.0 );
  double angle_incr = 0;
  
  samples = range_count;
  
  if( samples > 0 ) // catch a pesky /0 error!
    angle_incr = DTOR( (max_angle - min_angle)/100.0) / samples;
  
  //printf( "XS: RenderLaserProxy() "
  //"x:%.2f y:%.2f th:%.2f min:%d max:%d incr:%.2f ranges:%d\n",
  //x, y, th, prox->min_angle, prox->max_angle, angle_incr, samples );
  
  double range, angle;
  for( int l=0; l < samples; l++ )
    {
      range = ranges[l];
      angle = start_angle + l * angle_incr;
      
      pts[l].x = x + range/1000.0 * cos( angle ); 
      pts[l].y = y + range/1000.0 * sin( angle );       
    }
}

void CGraphicSonarProxy::ProcessData( void )
{
  // figure out where to draw this data
  double x,y,th;

  if( !win->PoseFromId( stage_id, x, y, th, pixel ) )
    {
      printf( "XS Warning: couldn't find object (%d:%d:%d)"
	      " so abandoned render\n", 
	      client->port, device, index );
      return;
    }
  
  for( int l=0; l < PLAYER_NUM_SONAR_SAMPLES; l++ )
    {
      double xoffset, yoffset, angle;
      win->GetSonarPose( l, xoffset, yoffset, angle );
      
      double range = ranges[l];
      
      angle += th;

      startpts[l].x = x + xoffset * cos(th) - yoffset * sin(th);
      startpts[l].y = y + xoffset * sin(th) + yoffset * cos(th);
         
      endpts[l].x = startpts[l].x + range/1000.0 * cos( angle ); 
      endpts[l].y = startpts[l].y + range/1000.0 * sin( angle );       
    }
}

#ifdef HRL_DEVICES
void CGraphicIDARProxy::ProcessData( void )
{
  // figure out where to draw this data
  double x,y,th;

  if( !win->PoseFromId( stage_id, x, y, th, pixel ) )
    {
      printf( "XS Warning: couldn't find object (%d:%d:%d)"
	      " so abandoned render\n", 
	      client->port, device, index );
      return;
    }
  
  origin.x = x;
  origin.y = y;

  double angle_per_sensor = (2.0 * M_PI)/(double)PLAYER_NUM_IDAR_SAMPLES;
  
  for( int l=0; l < PLAYER_NUM_IDAR_SAMPLES; l++ )
    {
      double angle = l * angle_per_sensor + th;

      // max (255) intensity will be a 1m stick
      //double sticklen = (double)(data.rx[l].intensity) / 255.0;

      double sticklen = 0.40; // 30cm from center of robot

      //intensity works out as 0-255 proportion of max range 
      //double baselen = data.rx[l].intensity * tan( angle_per_sensor/2.0 );

      double baselen = sticklen * tan( angle_per_sensor/2.0 );

      //double dx = baselen * cos( angle + M_PI/2.0 );
      //double dy = baselen * sin( angle + M_PI/2.0 );

      intensitypts[l].x = x + sticklen * cos( angle ); 
      intensitypts[l].y = y + sticklen * sin( angle );       

      double ray_start_angle = angle - angle_per_sensor/2.0;
      double angle_per_ray = angle_per_sensor / RAYS_PER_SENSOR;

      for( int f=0; f<RAYS_PER_SENSOR; f++ )
	{
	  double ray_angle = ray_start_angle + f * angle_per_ray;
	  double range = data.rx[l].ranges[f] / 1000.0; // meters

	  //printf( "range: %.2f\n", range );

	  rangepts[l][f].x = x + range * cos( ray_angle );
	  rangepts[l][f].y = y + range * sin( ray_angle );
	}
	   
      
      // data[2] is the pheremone type
      // store the value here for display later
      sprintf( rx_buf[l], "%d:%u", 
	       l,
	       //data.tx[l].intensity,
	       data.rx[l].intensity );
    }
}
#endif


void CGraphicGpsProxy::ProcessData( void )
{
  // figure out where to draw this data
  double dummyth;
  
  if( !win->PoseFromId(  stage_id, drawx, drawy, dummyth, pixel ) )
    {
      printf( "XS Warning: couldn't find object (%d:%d:%d)"
	      " so abandoned render\n", 
	      client->port, device, index );
      return;
    }

  // fill the output string
  sprintf( buf, "GPS(%d,%d,%d)", xpos, ypos, heading );
}

void CGraphicPtzProxy::ProcessData( void )
{
  // figure out where to draw this data
  double x,y,th;
  
  if( !win->PoseFromId( stage_id, x, y, th, pixel ) )
    {
      printf( "XS Warning: couldn't find object (%d:%d:%d)"
	      " so abandoned render\n", 
	      client->port, device, index );
      return;
    }
  
  double len = 0.5; // meters
  // have to figure out the zoom with Andrew
  // for now i have a fixed width triangle
  
  double startAngle =  th + DTOR(pan) - DTOR( 30 );
  double stopAngle  =  startAngle + DTOR( 60 );
      
  pts[0].x = x;
  pts[0].y = y;
  
  pts[1].x = x + len * cos( startAngle );  
  pts[1].y = y + len * sin( startAngle );  
  
  pts[2].x = x + len * cos( stopAngle );  
  pts[2].y = y + len * sin( stopAngle );  
}

void CGraphicLaserBeaconProxy::ProcessData( void )
{
  // figure out where to draw this data
  double x,y,th;
  
  if( !win->PoseFromId( stage_id, x, y, th, pixel ) )
    {
      printf( "XS Warning: couldn't find object (%d:%d:%d)"
	      " so abandoned render\n", 
	      client->port, device, index );
      return;
    }

  // store the home position
  origin.x = x;
  origin.y = y;
  
  // calculate the beacon positions
  for( int c=0; c<count; c++ )
    {
      pts[c].x = x + (beacons[c].range/1000.0) * cos( th + DTOR(beacons[c].bearing) );  
      pts[c].y = y + (beacons[c].range/1000.0) * sin( th + DTOR(beacons[c].bearing) );

      ids[c] = beacons[c].id;
    }
  
  stored = count;
}



///////////////////////////////////////////////////////////////////////////
// Get the pose of the sonar
//
void CXGui::GetSonarPose(int s, double &px, double &py, double &pth )
{
  double xx = 0;
  double yy = 0;
  double a = 0;
  double angle, tangle;
  
  switch( s )
    {
#ifdef PIONEER1
    case 0: angle = a - 1.57; break; //-90 deg
    case 1: angle = a - 0.52; break; // -30 deg
    case 2: angle = a - 0.26; break; // -15 deg
    case 3: angle = a       ; break;
    case 4: angle = a + 0.26; break; // 15 deg
    case 5: angle = a + 0.52; break; // 30 deg
    case 6: angle = a + 1.57; break; // 90 deg
#else
    case 0:
      angle = a - 1.57;
      tangle = a - 0.900;
      xx += 0.172 * cos( tangle );
      yy += 0.172 * sin( tangle );
      break; //-90 deg
    case 1: angle = a - 0.87;
      tangle = a - 0.652;
      xx += 0.196 * cos( tangle );
      yy += 0.196 * sin( tangle );
      break; // -50 deg
    case 2: angle = a - 0.52;
      tangle = a - 0.385;
      xx += 0.208 * cos( tangle );
      yy += 0.208 * sin( tangle );
      break; // -30 deg
    case 3: angle = a - 0.17;
      tangle = a - 0.137;
      xx += 0.214 * cos( tangle );
      yy += 0.214 * sin( tangle );
      break; // -10 deg
    case 4: angle = a + 0.17;
      tangle = a + 0.137;
      xx += 0.214 * cos( tangle );
      yy += 0.214 * sin( tangle );
      break; // 10 deg
    case 5: angle = a + 0.52;
      tangle = a + 0.385;
      xx += 0.208 * cos( tangle );
      yy += 0.208 * sin( tangle );
      break; // 30 deg
    case 6: angle = a + 0.87;
      tangle = a + 0.652;
      xx += 0.196 * cos( tangle );
      yy += 0.196 * sin( tangle );
      break; // 50 deg
    case 7: angle = a + 1.57;
      tangle = a + 0.900;
      xx += 0.172 * cos( tangle );
      yy += 0.172 * sin( tangle );
      break; // 90 deg
    case 8: angle = a + 1.57;
      tangle = a + 2.240;
      xx += 0.172 * cos( tangle );
      yy += 0.172 * sin( tangle );
      break; // 90 deg
    case 9: angle = a + 2.27;
      tangle = a + 2.488;
      xx += 0.196 * cos( tangle );
      yy += 0.196 * sin( tangle );
      break; // 130 deg
    case 10: angle = a + 2.62;
      tangle = a + 2.755;
      xx += 0.208 * cos( tangle );
      yy += 0.208 * sin( tangle );
      break; // 150 deg
    case 11: angle = a + 2.97;
      tangle = a + 3.005;
      xx += 0.214 * cos( tangle );
      yy += 0.214 * sin( tangle );
      break; // 170 deg
    case 12: angle = a - 2.97;
      tangle = a - 3.005;
      xx += 0.214 * cos( tangle );
      yy += 0.214 * sin( tangle );
      break; // -170 deg
    case 13: angle = a - 2.62;
      tangle = a - 2.755;
      xx += 0.208 * cos( tangle );
      yy += 0.208 * sin( tangle );
      break; // -150 deg
    case 14: angle = a - 2.27;
      tangle = a - 2.488;
      xx += 0.196 * cos( tangle );
      yy += 0.196 * sin( tangle );
      break; // -130 deg
    case 15: angle = a - 1.57;
      tangle = a - 2.240;
      xx += 0.172 * cos( tangle );
      yy += 0.172 * sin( tangle );
      break; // -90 deg
    default:
      angle = 0;
      xx = 0;
      yy = 0;
      break;
#endif
    }

  px = xx;
  py = -yy;
  pth = -angle;
}







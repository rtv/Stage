
#include <player.h>
#include <netinet/in.h>


// convert meters to Player mm in NBO in various sizes
#define MM_32(A)  (htonl((int32_t)(A * 1000.0)))
#define MM_16(A)  (htons((int16_t)(A * 1000.0)))
#define MM_U16(A) (htons((uint16_t)(A * 1000.0)))

// convert mm of various sizes in NBO to local meters
#define M_32(A)  ((int)ntohl((int32_t)A) / 1000.0)
#define M_16(A)  ((int)ntohs((int16_t)A) / 1000.0)
#define M_U16(A) ((unsigned short)ntohs((uint16_t)A) / 1000.0)

// convert local radians to Player degrees in various sizes
#define Deg_32(A) (htonl((int32_t)(RTOD(A))))
#define Deg_16(A) (htons((int16_t)(RTOD(A))))
#define Deg_U16(A) (htons((uint16_t)(RTOD(A))))

// convert Player degrees in various sizes to local radians
#define Rad_32(A) (DTOR(ntohl((int32_t)A)))
#define Rad_16(A) (DTOR(ntohs((int16_t)A)))
#define Rad_U16(A) (DTOR(ntohs((uint16_t)A)))

//#define PACK_INT_32(A) (htonl((int32_t)A)
//#define PACK_UINT_32(A) (htonl((uint32_t)A)

//#define PACK_INT_16(A) (htons((int16_t)A)
//#define PACK_UINT_16(A) (htons((uint16_t)A)

//#define P_8(A) 

#define TWOPI (M_PI*2.0)

void SonarDataPack( player_sonar_data_t* data, 
		    int num_samples, double ranges[])
{
  int s;

  // check pointer
  assert( data );
  // Check bounds
  assert( num_samples <= PLAYER_SONAR_MAX_SAMPLES );
  
  data->range_count = htons((uint16_t)num_samples);
  
  // Store range in mm
  for (s = 0; s < num_samples; s++)
    data->ranges[s] = MM_U16(ranges[s]);
}


void SonarDataUnpack( player_sonar_data_t* data, 
		      int* num_samples, double ranges[])
{
  int s;
  
  assert( data );
  assert( ranges );
  
  *num_samples = htons((uint16_t)data->range_count);
  
  assert( *num_samples <= PLAYER_SONAR_MAX_SAMPLES );
  
  // convert from integer mm to double m
  for (s = 0; s < *num_samples; s++)
    ranges[s] = M_U16(data->ranges[s]);
}


void SonarGeomPack( player_sonar_geom_t* geom, 
		    int num_samples, double poses[][3] )
{
  int s;
  
  geom->pose_count = htons((uint16_t)num_samples);
  
  // Store range in mm
  for (s = 0; s < num_samples; s++)
    {
      geom->poses[s][0] = MM_16(poses[s][0]);
      geom->poses[s][1] = MM_16(poses[s][1]);
      geom->poses[s][2] = Deg_16(poses[s][2]);
    }
}

void SonarGeomUnpack( player_sonar_geom_t* geom, 
		      int* num_samples, double poses[][3] )
{
  int s;
  
  *num_samples = ntohs(geom->pose_count);
  
  for( s=0; s<*num_samples; s++ )
    {
      poses[s][0] = M_16( geom->poses[s][0] );
      poses[s][1] = M_16( geom->poses[s][1] );
      poses[s][2] = Rad_16( geom->poses[s][2] );
    }
}	    



void PositionDataPack( player_position_data_t* data,
		       double xpos, double ypos, double yaw,
		       double xspeed, double yspeed, double yawspeed, 
		       int stall )
{
  // Compute odometric pose
  // Convert to mm and degrees (0 - 360)
  // commands for velocity control
  data->xspeed = MM_32(xspeed);
  data->yspeed = MM_32(yspeed);
  data->yawspeed = Deg_32(yawspeed);  
  
  // commands for position control
  data->xpos = MM_32(xpos);
  data->ypos = MM_32(ypos);
  data->yaw = Deg_32(yaw); 
  
  //printf( "DATA RAW %.2f %.2f %.2f\n", xpos, ypos, yaw );
  //printf( "DATA PACKED %d %d %d\n", data->xpos, data->ypos, data->yaw );

  data->stall = (uint8_t)(stall ? 1 : 0 );  
}

void PositionDataUnpack( player_position_data_t* data,
			 double* xpos, double* ypos, double* yaw,
			 double* xspeed, double* yspeed, double* yawspeed,
			 int* stall )
{ 
  assert( data );

  if(xpos) *xpos = M_32(data->xpos);
  if(xpos) *ypos = M_32(data->ypos);
  if(xpos) *yaw  = Rad_32(data->yaw);

  if(xpos) *xspeed = M_32(data->xspeed);
  if(xpos) *yspeed = M_32(data->yspeed);
  if(xpos) *yawspeed = Rad_32(data->yawspeed);
  


  if(stall) *stall = data->stall ? 1 : 0;
}

void PositionCmdUnpack( player_position_cmd_t* cmd,
			double* xpos, double* ypos, double* yaw,
			double* xspeed, double* yspeed, double* yawspeed )
{ 
  assert( cmd );
  
  if(xpos) *xpos = M_32(cmd->xpos);
  if(ypos) *ypos = M_32(cmd->ypos);
  if(yaw) *yaw  = Rad_32(cmd->yaw);
  
  if(xspeed) *xspeed = M_32(cmd->xspeed);
  if(yspeed) *yspeed = M_32(cmd->yspeed);
  if(yawspeed) *yawspeed = Rad_32(cmd->yawspeed);
}

void PositionGeomPack( player_position_geom_t* geom,
		       double x, double y, double a, 
		       double width, double height )
{
  assert( geom );
  
  geom->pose[0] = MM_16(x);
  geom->pose[1] = MM_16(y);
  geom->pose[2] = Deg_16(a);
  geom->size[0] = MM_U16(width);
  geom->size[1] = MM_U16(height);
}     

void PositionSetOdomReqUnpack( player_position_set_odom_req_t* req,
			       double* x, double* y, double* a )
{
  assert( req );
  
  if(x) *x = M_32(req->x);
  if(y) *y = M_32(req->y);
  if(a) *a = Rad_U16(req->theta);
}

      


















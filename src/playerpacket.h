
/* Functions to pack and unpack the Player message structures 
 * - 
 * - important to keep this in sync with player.h!
 *
 * TODO - inline documentation and parser for this file?
 *
 * Created 2002-11-18 rtv
 * $Id: playerpacket.h,v 1.1.2.1 2002-12-04 01:35:07 rtv Exp $
 */


#include <player.h>

#ifdef __cplusplus
extern "C" {
#endif
  
  /* libplayerpacket provides the following functions to pack and
     unpack Player's message structures using native types and SI
     units: */
  
  void SonarDataPack( player_sonar_data_t* data, 
		      int num_samples, double ranges[]);
  
  void SonarDataUnpack( player_sonar_data_t* data, 
			int* num_samples, double ranges[]);
  
  void SonarGeomPack( player_sonar_geom_t* geom, 
		      int num_samples, double poses[][3] );
  
  void SonarGeomUnpack( player_sonar_geom_t* geom, 
			int* num_samples, double poses[][3] );
  
  void PositionDataPack( player_position_data_t* data,
			 double xpos, double ypos, double yaw,
			 double xspeed, double yspeed, double yawspeed, 
			 int stall );
  
  void PositionDataUnpack( player_position_data_t* data,
			   double* xpos, double* ypos, double* yaw,
			   double* xspeed, double* yspeed, double* yawspeed,
			   int* stall );
  
  void PositionCmdUnpack( player_position_cmd_t* cmd,
			  double* xpos, double* ypos, double* yaw,
			  double* xspeed, double* yspeed, double* yawspeed );
  
  void PositionGeomPack( player_position_geom_t* geom,
			 double x, double y, double a, 
			 double width, double height );
  
  void PositionSetOdomReqUnpack( player_position_set_odom_req_t* req,
				 double* x, double* y, double* a );
  
  
#ifdef __cplusplus
}
#endif





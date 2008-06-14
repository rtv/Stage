/*
 * File: ptest.c
 * Desc: Player test program for use with libstageplugin
 * Author: Richard Vaughan
 * License: GPL v2
 * CVS info:
 *  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/examples/libplayerc/ptest.c,v $
 *  $Author: pooya $
 *  $Revision: 1.1 $
 */

#include <stdio.h>
#include <string.h>
#include <libplayerc/playerc.h>

const char* USAGE = \
"Usage: ptest <string>\n"
" where <string> is comprised of any number of the following characters:\n"
"  'p' - position\n"
"  'l' - laser";
 
int test_laser( playerc_client_t* client )
{
  // Create and subscribe to a laser device.
  playerc_laser_t *laser = 
    playerc_laser_create(client, 0);
  if (playerc_laser_subscribe(laser, PLAYER_OPEN_MODE))
    return -1;
  
  int i;  
  for(i = 0; i < 100; i++)
    {
      // Wait for new data from server
      playerc_client_read(client);	
    } 
  
  playerc_laser_unsubscribe(laser);
  playerc_laser_destroy(laser);
  
  return 0;
}

int test_position( playerc_client_t* client )
{
  // Create and subscribe to a position device.
  playerc_position2d_t *position = 
    playerc_position2d_create(client, 0);
  if (playerc_position2d_subscribe(position, PLAYER_OPEN_MODE))
    return -1;
  
  // Make the robot spin!
  if (playerc_position2d_set_cmd_vel(position, 0, 0, DTOR(40.0), 1) != 0)
    return -1;
  
  int i;
  for( i = 0; i < 100; i++)
    {
      // Wait for new data from server
      playerc_client_read(client);
      
      // Print current robot pose
      printf("position : %f %f %f\n",
	     position->px, position->py, position->pa);
    } 
  
  playerc_position2d_unsubscribe(position);
  playerc_position2d_destroy(position);

  return 0;
}

int main(int argc, const char **argv)
{
  const char *host;
  int port;
  playerc_client_t *client;

  host = "localhost";
  port = 6665;
  
  if( argc != 2 )
    {
      puts( USAGE );
      return(-1);
    }
  
  printf( "Attempting to connect to a Player server on %s:%d\n",
	      host, port );

  // Create a client and connect it to the server.
  client = playerc_client_create(NULL, host, port);
  if (playerc_client_connect(client) != 0)
    {
      puts( "Failed. Quitting." );
      return -1;
    }
  
  puts( "Connected. Running tests." );

  // run through the input string 
  size_t len = strlen( argv[1] );
  size_t i;
  for( i=0; i<len; i++ )
    switch( argv[1][i] )
      {
      case 'p':
	test_position(client);
	break;
	
      case 'l':
	test_laser(client);
	break;

      default:
	printf( "unrecognized test '%c'\n",  argv[1][i] );
      }

  puts( "Disconnecting" );
  // Shutdown
  playerc_client_disconnect(client);
  playerc_client_destroy(client);

  puts( "Done." );
  return 0;
}

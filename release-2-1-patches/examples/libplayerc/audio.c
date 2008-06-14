/*
 * File: audio.c
 * Desc: Test program for use of audio through an opaque interface to player
 * Author: Pooya Karimian
 * License: GPL v2
 * CVS info:
 *  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/examples/libplayerc/audio.c,v $
 *  $Author: pooya $
 *  $Revision: 1.1 $
 */

#include <stdio.h>
#include <string.h>
#include <libplayerc/playerc.h>

int main(int argc, const char **argv)
{
  const char *host;
  int port;
  playerc_client_t *client;

  host = "localhost";
  port = 6665;
  
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

  // Create and subscribe to an audio device.
  playerc_opaque_t *audio = playerc_opaque_create(client, 0);
  if (playerc_opaque_subscribe(audio, PLAYER_OPEN_MODE))
    return -1;

  int i;
  for( i = 0; i < 1000; i++)
    {
      // Wait/Read for new data from server, or continue after 100ms
      if (playerc_client_peek(client, 100) > 0) 
        playerc_client_read(client);

      // If there's new data
      if (audio->data_count>0) 
      {
        // print it
        printf("%s\n", audio->data);

        // clear read data
        audio->data_count=0;
      }

      // send a message approximately every two seconds
      if ( i % 20 == 0 ) 
      {   
        // clear a new data packet
        player_opaque_data_t audio_msg;
        char *temp_str="Hello World!";
        sprintf((char *) audio_msg.data, "%s", temp_str);
        audio_msg.data_count=strlen(temp_str)+1;
      
        playerc_opaque_cmd(audio, &audio_msg);
      }
      
    } 
  
  playerc_opaque_unsubscribe(audio);
  playerc_opaque_destroy(audio);

  return 0;


  puts( "Disconnecting" );
  // Shutdown
  playerc_client_disconnect(client);
  playerc_client_destroy(client);

  puts( "Done." );
  return 0;
}

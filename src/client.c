
#if HAVE_CONFIG_H
  #include <config.h>
#endif
#if HAVE_STRINGS_H
  #include <strings.h>
#endif


#define DEBUG
#define VERBOSE
//#undef DEBUG

#include "stageio.h"

int main( int argc, char** argv )
{
  int stage_fd = InitStageClient( argc, argv);

  //while( 1 )
    {
      stage_model_t ent[5];

      ent[0].id = 0;
      ent[0].parent_id = -1; // only the root can have no parent
      strcpy( ent[0].token, "root" );

      ent[1].id = 1;
      ent[1].parent_id = 0;
      strcpy( ent[1].token, "box" );

      ent[2].id = 2;
      ent[2].parent_id = 0;
      strcpy( ent[2].token, "box" );


      
      printf( "result %d\n", CreateModels( stage_fd, ent, 3 ) );


      //stage_report_header_t rep;
      //ReadPacket


  }
}


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

      int m;
      for( m=0; m<5; m++)
	{
	  ent[m].id = m;
	  ent[m].parent = 0;
	  strcpy( ent[m].token, "position" );
	}

      printf( "result %d\n", CreateModels( stage_fd, ent, 5 ) );
  }
}

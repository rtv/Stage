#ifdef __cplusplus
extern "C" {
#endif
  
  #include "stage.h"

  // C wrappers to access the library
  int CreateEntityFromLibrary( stage_model_t* model );
  int Startup( void );
  int Update( double simtime );

#ifdef __cplusplus
}
#endif


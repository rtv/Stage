#ifdef __cplusplus
extern "C" {
#endif
  
  // C wrappers to access the library
  int CreateEntityFromLibrary( char* token, int id, int parent_id );
  int Startup( void );
  int Update( double simtime );

#ifdef __cplusplus
}
#endif


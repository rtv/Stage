#include "stage.h"
//#include "token.h"

#ifdef __cplusplus
 extern "C" {
#endif 

#define STG_DEFAULT_WORLDFILE "default.world"

// Stage Client ---------------------------------------------------------

typedef struct
{
  char* host;
  int port;
  struct pollfd pfd;
 
  stg_id_t next_id;
  
  GHashTable* worlds_id_server; // contains stg_world_ts indexed by server-side id
  GHashTable* worlds_id; // contains stg_world_ts indexed by client-side id
  GHashTable* worlds_name; // the worlds indexed by namex
} stg_client_t;

// Stage Client functions. Most client programs will call all of these
// in this order.

// create a Stage Client.
stg_client_t* stg_client_create( void );


// load a worldfile into the client
// returns zero on success, else an error code.
int stg_client_load( stg_client_t* client, char* worldfilename );

// connect to a Stage server at [host]:[port]. If [host] and [port]
// are not specified, client uses it's current values, which have
// sensible defaults.  returns zero on success, else an error code.
int stg_client_connect( stg_client_t* client, char* host, int port );

// ask a connected Stage server to create simulations of all our objects
void stg_client_push( stg_client_t* client );

// read a message from the server
stg_msg_t* stg_client_read( stg_client_t* cli );

// remove all our objects from from the server
void stg_client_pull( stg_client_t* client );

// destroy a Stage client
void stg_client_destroy( stg_client_t* client );


typedef struct
{
  stg_client_t* client; // the client that created this world
  
  int id_client; // client-side id
  stg_id_t id_server; // server-side id
  stg_token_t* token;  
  double ppm;
  double interval_sim;
  double interval_real;
  
  int section; // worldfile index
  
  GHashTable* models_id_server;
  GHashTable* models_id;   // the models index by client-side id
  GHashTable* models_name; // the models indexed by name
} stg_world_t;

typedef struct _stg_model
{
  int id_client; // client-side if of this object
  stg_id_t id_server; // server-side id of this object
  
  struct _stg_model* parent; 
  stg_world_t* world; 
  
  int section; // worldfile index

  stg_token_t* token;

  GHashTable* props;
  
} stg_model_t;

// add a new world to a client, based on a token
stg_world_t* stg_client_createworld( stg_client_t* client, 
				     int section,
				     stg_token_t* token,
				     double ppm, 
				     double interval_sim, 
				     double interval_real  );

void stg_world_destroy( stg_world_t* world );
int stg_world_pull( stg_world_t* world );

void stg_world_resume( stg_world_t* world );
void stg_world_pause( stg_world_t* world );



// add a new model to a world, based on a parent, section and token
stg_model_t* stg_world_createmodel( stg_world_t* world, 
				    stg_model_t* parent, 
				    int section, 
				    stg_token_t* token );

void stg_model_destroy( stg_model_t* model );
int stg_model_pull( stg_model_t* model );
void stg_model_attach_prop( stg_model_t* mod, stg_property_t* prop );
void stg_model_prop_with_data( stg_model_t* mod, 
			      stg_id_t type, void* data, size_t len );

int stg_model_subscribe( stg_model_t* mod, int prop, double interval );
int stg_model_unsubscribe( stg_model_t* mod, int prop );


void stg_world_push( stg_world_t* model );
void stg_model_push( stg_model_t* model );
void stg_world_push_cb( gpointer key, gpointer value, gpointer user );
void stg_model_push_cb( gpointer key, gpointer value, gpointer user );


stg_id_t stg_world_new(  stg_client_t* cli, char* token, 
			 double width, double height, int ppm, 
			 double interval_sim, double interval_real  );

stg_id_t stg_model_new(  stg_client_t* cli, 
			 stg_id_t world,
			 char* token );

int stg_client_property_set( stg_client_t* cli, stg_id_t world, stg_id_t model, 
		      stg_id_t prop, void* data, size_t len );


stg_model_t* stg_client_get_model( stg_client_t* cli, 
				   const char* wname, const char* mname );

stg_model_t* stg_client_get_model_serverside( stg_client_t* cli, stg_id_t wid, stg_id_t mid );

//stg_token_t* stg_tokenize( FILE* wf );
//stg_world_t* sc_load_worldblock( stg_client_t* cli, stg_token_t** tokensptr );
//stg_model_t* sc_load_modelblock( stg_world_t* world, stg_model_t* parent, 
//				stg_token_t** tokensptr );



#ifdef __cplusplus
}
#endif 

#include "stage.h"
#include "token.h"

#ifdef __cplusplus
 extern "C" {
#endif 

// Stage Client ---------------------------------------------------------

typedef struct
{
  char* host;
  int port;
  struct pollfd pfd;
 
  stg_id_t next_id;
  
  GHashTable* worlds_id_server; // contains sc_world_ts indexed by server-side id
  GHashTable* worlds_id; // contains sc_world_ts indexed by client-side id
  GHashTable* worlds_name; // the worlds indexed by namex
} sc_t;

// Stage Client functions. Most client programs will call all of these
// in this order.

// create a Stage Client.
sc_t* sc_create( void );


// load a worldfile into the client
// returns zero on success, else an error code.
int sc_load( sc_t* client, char* worldfilename );

// connect to a Stage server at [host]:[port]. If [host] and [port]
// are not specified, client uses it's current values, which have
// sensible defaults.  returns zero on success, else an error code.
int sc_connect( sc_t* client, char* host, int port );

// ask a connected Stage server to create simulations of all our objects
void sc_push( sc_t* client );

// read a message from the server
stg_msg_t* sc_read( sc_t* cli );

// remove all our objects from from the server
void sc_pull( sc_t* client );

// destroy a Stage client
void sc_destroy( sc_t* client );


typedef struct
{
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
} sc_world_t;

typedef struct _sc_model
{
  int id_client; // client-side if of this object
  stg_id_t id_server; // server-side id of this object
  
  struct _sc_model* parent; 
  sc_world_t* world; 
  
  int section; // worldfile index

  stg_token_t* token;

  GHashTable* props;
  
} sc_model_t;

// add a new world to a client, based on a token
sc_world_t* sc_world_create( stg_token_t* token,
			     double ppm, double interval_sim, double interval_real  );
void sc_world_destroy( sc_world_t* world );
int sc_world_pull( sc_t* cli, sc_world_t* world );
void sc_world_addmodel( sc_world_t* world, sc_model_t* model );

// add a new model to a world, based on a parent and a token
sc_model_t* sc_model_create( sc_world_t* world, sc_model_t* parent, stg_token_t* token );
void sc_model_destroy( sc_model_t* model );
int sc_model_pull( sc_t* cli, sc_model_t* model );
void sc_model_attach_prop( sc_model_t* mod, stg_property_t* prop );
void sc_model_prop_with_data( sc_model_t* mod, 
			      stg_id_t type, void* data, size_t len );

int sc_model_subscribe( sc_t* cli, sc_model_t* mod, int prop, double interval );
int sc_model_unsubscribe(sc_t* cli, sc_model_t* mod, int prop );


void sc_world_push( sc_t* cli, sc_world_t* value );
void sc_model_push( sc_t* cli, sc_model_t* value );
void sc_world_push_cb( gpointer key, gpointer value, gpointer user );
void sc_model_push_cb( gpointer key, gpointer value, gpointer user );


stg_id_t stg_world_new(  sc_t* cli, char* token, 
			 double width, double height, int ppm, 
			 double interval_sim, double interval_real  );

stg_id_t stg_model_new(  sc_t* cli, 
			 stg_id_t world,
			 char* token );

int stg_property_set( sc_t* cli, stg_id_t world, stg_id_t model, 
		      stg_id_t prop, void* data, size_t len );

void sc_addworld( sc_t* cli, sc_world_t* world );
void sc_world_addmodel( sc_world_t* world, sc_model_t* model );


sc_world_t* sc_load_worldblock( sc_t* cli, stg_token_t** tokensptr );
sc_model_t* sc_load_modelblock( sc_world_t* world, sc_model_t* parent, 
				stg_token_t** tokensptr );

sc_model_t* sc_get_model( sc_t* cli, char* wname, char* mname );

sc_model_t* sc_get_model_serverside( sc_t* cli, stg_id_t wid, stg_id_t mid );

stg_token_t* stg_tokenize( FILE* wf );


#ifdef __cplusplus
}
#endif 



#include <stdlib.h>

//#define DEBUG

#include "subscription.h"
#include "model.h"

subscription_t* subscription_create( void )
{
  subscription_t* sub = calloc( sizeof(subscription_t), 1 );

  return sub;
}

void subscription_destroy( subscription_t* sub )
{
  free( sub );
}

void subscription_destroy_cb( gpointer sub, gpointer user )
{
  subscription_destroy( (subscription_t*)sub);
}


int subscription_due( subscription_t* sub, double timenow )
{
  return( timenow - sub->timestamp > sub->interval );
}

void subscription_print( subscription_t* sub, char* prefix )
{
  printf( "%s[%d:%d:%s %.2f]",
	  prefix ? prefix : " ", // use a space if no prefix supplied
	  sub->target.world, sub->target.model, 
	  stg_property_string(sub->target.prop),
	  sub->interval );
}

void subscription_print_cb( gpointer value, gpointer user )
{
  subscription_print( (subscription_t*)value, (char*)user ); 
}

void subscription_update( subscription_t* sub )
{
  //double timenow = stg_timenow();
  double timenow = server_world_time( sub->server, sub->target.world );
  
  // if it's time to send some data for this subscription
  if( subscription_due( sub, timenow ) )
    {
#ifdef DEBUG      
      printf( "servicing subscription to " ); 
      subscription_print( sub, "" );

      printf( " %.4f  ",
	      timenow - sub->timestamp );
      
#endif
      
      // if this property is out of date, we generate new data
      
      model_t* mod = server_get_model( sub->server, 
					   sub->target.world,
					   sub->target.model );
      
      if( timenow - mod->update_times[ sub->target.prop ] >= sub->interval )
	model_update_prop( mod, sub->target.prop );
	

      // set the last update time
      sub->timestamp = timenow;

      // compose a message and send it out
      
      // TODO - buffer these messages and send in in batches for speed
      // TODO - send only if changed
      
#ifdef DEBUG           
      printf( "FOUND\n" );
#endif
      // get the correct property out of the model
      
      void* data = NULL;
      size_t len = 0;
      
      if( model_get_prop( mod, sub->target.prop, &data, &len ) )      
	PRINT_WARN2( "failed to service subscription for property %d(%s)",
		     sub->target.prop, stg_property_string(sub->target.prop) );
      else
	{
	  size_t mplen = sizeof(stg_prop_t) + len;
	  stg_prop_t* mp = calloc( mplen,1  );
	  
	  mp->timestamp = timenow;
	  //server_world_time( sub->server, sub->target.world );
	  
	  //printf( "mp->timestamp %.3f\n", mp->timestamp );
	  
	  mp->world = sub->target.world;
	  mp->model = sub->target.model;
	  mp->prop =  sub->target.prop;
	  mp->datalen = len;
	  memcpy( mp->data, data, len );
	  
	  //printf( "timestamping prop %d(%s) at %.3f seconds\n",
	  //      mp->prop, stg_property_string(mp->prop), mp->timestamp );
	  
	  stg_msg_t*  msg = stg_msg_create( STG_MSG_CLIENT_PROPERTY, 
					    STG_RESPONSE_NONE,
					    mp, mplen );

	  stg_fd_msg_write( sub->client->fd, msg );
	  
	  free( mp );
	}
    }
}


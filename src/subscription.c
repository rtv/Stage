
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


int subscription_due( subscription_t* sub, stg_msec_t timenow )
{
  stg_msec_t dif = timenow - sub->timestamp;
  
  //printf( "dif is %lu which is %s interval %lu\n",
  //  dif, 
  //  (dif < sub->interval) ? "less than" : "greater or equal to",
  //  sub->interval );
  
  return( dif >= sub->interval );
}

void subscription_print( subscription_t* sub, char* prefix )
{
  printf( "%s[%d:%d:%s %lu]",
	  prefix ? prefix : " ", // use a space if no prefix supplied
	  sub->target.world, sub->target.model, 
	  stg_property_string(sub->target.prop),
	  sub->interval );
}

void subscription_print_cb( gpointer value, gpointer user )
{
  subscription_print( (subscription_t*)value, (char*)user ); 
}

// returns 1 if a delta is sent, else 0
int subscription_update( subscription_t* sub )
{
  //double timenow = stg_timenow();
  stg_msec_t timenow = server_world_time( sub->server, sub->target.world );
  
  //printf( "subscription interval %lu time now %lu  last stamp %lu  difference %lu\n",
  //  sub->interval, timenow, sub->timestamp, timenow-sub->timestamp );

  // if it's time to send some data for this subscription
  if( subscription_due( sub, timenow ) )
    {
      //printf( "due\n" );

#ifdef DEBUG      
      printf( "servicing subscription to " ); 
      subscription_print( sub, "" );

      printf( " %lu  ",
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
	  mp->world = sub->target.world;
	  mp->model = sub->target.model;
	  mp->prop =  sub->target.prop;
	  mp->datalen = len;
	  memcpy( mp->data, data, len );
	  
	  //printf( "timestamping prop %d(%s) at %lu ms\n",
	  //  mp->prop, stg_property_string(mp->prop), mp->timestamp );
	  
	  stg_msg_t*  msg = stg_msg_create( STG_MSG_CLIENT_PROPERTY, 
					    STG_RESPONSE_NONE,
					    mp, mplen );

	  //stg_fd_msg_write( sub->client->fd, msg );	  

	  stg_buffer_append_msg( sub->client->outbuf, msg );
	  free( msg );

	  free( mp );

	  return 1; // we sent a message
	}
    }

  return 0; // we didn't send a message
}


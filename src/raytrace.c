#include <stdlib.h>
#include <math.h>

#include "stage.h"

extern rtk_fig_t* fig_debug;

void itl_destroy( itl_t* itl )
{
  free( itl );
}

// TODO take a filter function in here, so raytrace will only return
// pointers that satisfy the filter - thus we can test for relatives,
// etc in the large grids

itl_t* itl_create( double x, double y, double a, double b, 
		   stg_matrix_t* matrix, itl_mode_t pmode )
{   
  itl_t* itl = calloc( sizeof(itl_t), 1 );
  
  itl->matrix = matrix;
  itl->x = x;
  itl->y = y;
  itl->models = NULL;
  itl->index = 0;
  itl->range = 0;
  
  itl->big_incr = 1.0 / itl->matrix->bigppm;
  itl->med_incr = 1.0 / itl->matrix->medppm;  
  itl->small_incr = 1.0 / itl->matrix->ppm;  

  switch( pmode )
    {
    case PointToBearingRange:
    {
      double range = b;
      double bearing = a;
	
      itl->a = bearing;
      itl->cosa = cos( itl->a );
      itl->sina = sin( itl->a );
      itl->max_range = range;
    }
    break;
    case PointToPoint:
    {
      double x1 = a;
      double y1 = b;
           
      itl->a = atan2( y1-y, x1-x );
      itl->cosa = cos( itl->a );
      itl->sina = sin( itl->a );
      itl->max_range = hypot( x1-x, y1-y );
    }
    break;
    default:
      puts( "Stage Warning: unknown LineIterator mode" );
  }

  //printf( "a = %.2f remaining_range = %.2f\n", itl->a,
  //remaining_range ); fflush( stdout );

  return itl;
};

void itl_raytrace( itl_t* itl )
{
  //printf( "Ray from %.2f,%.2f angle: %.2f range %.2f max_range %.2f\n", 
  //  itl->x, itl->y, RTOD(itl->a), itl->range, itl->max_range );
  
  itl->index = 0;
  itl->models = NULL;
  
  while( itl->range < itl->max_range )
    {
      itl->models = stg_matrix_bigcell_get( itl->matrix, itl->x, itl->y );
     
      if( fig_debug ) // draw the big rectangle
	rtk_fig_rectangle( fig_debug, 
			   itl->x-fmod(itl->x,itl->big_incr)+itl->big_incr/2.0,
			   itl->y-fmod(itl->y,itl->big_incr)+itl->big_incr/2.0,
			   0, 
			   itl->big_incr, itl->big_incr, 0 );

      // if there is nothing in the big cell
      if( !(itl->models && itl->models->len > 0) )
	{	      
	  long bigcell_x = (long)floor(itl->x * itl->matrix->bigppm);
	  long bigcell_y = (long)floor(itl->y * itl->matrix->bigppm);
	  
	  // move forward in increments of one small cell until we
	  // get into a new big cell
	  while( bigcell_x == (long)floor(itl->x * itl->matrix->bigppm) &&
		 bigcell_y == (long)floor(itl->y * itl->matrix->bigppm) )
	    {
	      itl->x += itl->small_incr * itl->cosa;
	      itl->y += itl->small_incr * itl->sina;
	      itl->range += itl->small_incr;      
	    }	      
	  continue;
	}

      // check a medium cell
      itl->models = stg_matrix_medcell_get( itl->matrix, itl->x, itl->y );
     
      if( fig_debug ) // draw the big rectangle
	rtk_fig_rectangle( fig_debug, 
			   itl->x-fmod(itl->x,itl->med_incr)+itl->med_incr/2.0,
			   itl->y-fmod(itl->y,itl->med_incr)+itl->med_incr/2.0,
			   0, 
			   itl->med_incr, itl->med_incr, 0 );

      // if there is nothing in the big cell
      if( !(itl->models && itl->models->len > 0) )
	{	      
	  long medcell_x = (long)floor(itl->x * itl->matrix->medppm);
	  long medcell_y = (long)floor(itl->y * itl->matrix->medppm);
	  
	  // move forward in increments of one small cell until we
	  // get into a new medium cell
	  while( medcell_x == (long)floor(itl->x * itl->matrix->medppm) &&
		 medcell_y == (long)floor(itl->y * itl->matrix->medppm) )
	    {
	      itl->x += itl->small_incr * itl->cosa;
	      itl->y += itl->small_incr * itl->sina;
	      itl->range += itl->small_incr;      
	    }	      
	  continue;
	}

      // check a small cell
      itl->models = stg_matrix_cell_get( itl->matrix, itl->x, itl->y );
      if( !(itl->models && itl->models->len>0) ) 
	itl->models = 
	  stg_matrix_cell_get( itl->matrix, itl->x+itl->small_incr, itl->y );

      if( fig_debug ) // draw the small rectangle
	rtk_fig_rectangle( fig_debug, 
			   itl->x-fmod(itl->x,itl->small_incr)+itl->small_incr/2.0,
			   itl->y-fmod(itl->y,itl->small_incr)+itl->small_incr/2.0,
			   0, 
			   itl->small_incr, itl->small_incr, 0 );      

      itl->x += itl->small_incr * itl->cosa;// - itl->offx;
      itl->y += itl->small_incr * itl->sina;// - itl->offy;
      itl->range += itl->small_incr;
      
      if( itl->models && itl->models->len > 0 )
	{
	  break;
	}
    }
}

void PrintArray( GPtrArray* arr )
{
  if( arr )
    {
      printf( "model array %p len %d\n", arr, arr->len );
      int i;
      for( i=0; i<arr->len; i++ )
	printf( " (model %s)", ((stg_model_t*)g_ptr_array_index( arr, i ))->token );
    }
  else
    printf( "null array\n" );
}

stg_model_t* itl_next( itl_t* itl )
{
  // if we have no models or we've reached the end of the model array
  if( !(itl->models &&  itl->index > itl->models->len) )
    {
      // get a new array of pointers
      itl_raytrace( itl ); 
    }

  return( (itl->models && itl->models->len > 0 )? g_ptr_array_index( itl->models, itl->index++ ) : NULL ); 
}


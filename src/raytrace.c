#include <stdlib.h>
#include <math.h>

#include "raytrace.h"

void itl_destroy( itl_t* itl )
{
  free( itl );
}

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
  
  switch( pmode )
    {
    case PointToBearingRange:
    {
      double range = b;
      double bearing = a;
	
      itl->a = bearing;
      itl->max_range = range;
    }
    break;
    case PointToPoint:
    {
      double x1 = a;
      double y1 = b;
	
      //printf( "From %.2f,%.2f to %.2f,%.2f\n", x,y,x1,y1 );
      
      itl->a = atan2( y1-y, x1-x );
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

  // hops along each axis 1 cell at a time
  double cosa = cos( itl->a );
  double sina = sin( itl->a );
  
  double incr = 1.0 / itl->matrix->ppm;

  // Look along scan line for obstacles
  while( itl->range < itl->max_range  )
    {
      itl->x += incr * cosa;
      itl->y += incr * sina;
      
      // stop this ray if we're out of bounds
      if( itl->x < 0 ) 
	{ 
	  itl->x = 0; 
	  itl->range = itl->max_range;
	  break;
	}
      
      if( itl->y < 0 ) 
	{ 
	  itl->y = 0; 
	  itl->range = itl->max_range;
	  break;
	}
      
      //printf( "looking in %.2f,%.2f range %.2f max_range %.2f\n", 
      //    itl->x, itl->y, itl->range, itl->max_range );
      
      // TODO - use a hexagonal grid for the matrix so we can avoid this
      // double-lookup.
      
      itl->models = stg_matrix_cell_m( itl->matrix, itl->x, itl->y );
      if( !itl->models ) 
	itl->models = stg_matrix_cell_m( itl->matrix, itl->x+incr, itl->y );

      itl->range+=incr;

      if( itl->models && itl->models->len > 0 ) break;// we hit something!
    }
}; 

void PrintArray( GPtrArray* arr )
{
  if( arr )
    {
      printf( "model array %p len %d\n", arr, arr->len );
      int i;
      for( i=0; i<arr->len; i++ )
	printf( " (model %s)", ((model_t*)g_ptr_array_index( arr, i ))->token );
    }
  else
    printf( "null array\n" );
}

model_t* itl_next( itl_t* itl )
{
  //printf( "current ptr: %p index: %d\n", itl->ent[itl->index], index );
  
  //if( itl->models )
  //PrintArray(itl->models);

  // if we have no models or we've reached the end of the model array
  if( !(itl->models &&  itl->index > itl->models->len) )
    {
      // get a new array of pointers
      itl_raytrace( itl ); 
    }
  
  //assert( itl->mod != 0 ); // should be a valid array, even if empty
  
  //PrintArray( itl->ent );
  //printf( "returning %p (index: %d) at: %d %d rng: %d\n",  
  //  itl->ent[itl->index], itl->index, (int)itl->x, (int)itl->y, (int)itl->remaining_range );
  
  return( itl->models ? g_ptr_array_index( itl->models, itl->index++ ) : NULL ); 
}


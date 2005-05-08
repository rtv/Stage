#include <stdlib.h>
#include <math.h>

#include "stage_internal.h"

extern stg_rtk_fig_t* fig_debug_rays;

void itl_destroy( itl_t* itl )
{
  if( itl )
    {
      if( itl->incr ) 
	free( itl->incr );

      free( itl );
    }
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
  itl->incr = NULL;

  switch( pmode )
    {
    case PointToBearingRange:
      {
	double range = b;
	double bearing = a;	
	itl->a = NORMALIZE(bearing);
	itl->max_range = range;
      }
      break;
    case PointToPoint:
      {
	double x1 = a;
	double y1 = b;           
	itl->a = atan2( y1-y, x1-x );
	itl->max_range = hypot( x1-x, y1-y );
      }
      break;
    default:
      puts( "Stage Warning: unknown LineIterator mode" );
    }
  
  //printf( "a = %.2f remaining_range = %.2f\n", itl->a,
  //remaining_range ); fflush( stdout );
  
  itl->cosa = cos( itl->a );
  itl->sina = sin( itl->a );
  itl->tana = tan( itl->a );
  
  return itl;
};




// returns the first model in the array that matches, else NULL.
stg_model_t* gslist_first_matching( GSList* list, 
				    stg_itl_test_func_t func, 
				    stg_model_t* finder )
{
  int l=0;
  for( ; list ; list=list->next )
    {
      if( (*func)( finder, (stg_model_t*)(list->data) ) )
	return (stg_model_t*)(list->data);
    }
  
  return NULL; // nothing in the array matched
}


void print_thing( char* prefix, stg_cell_t* cell, double x, double y )
{
  printf( "%s %p x[%.7f %.7f %.7f] y[%.7f %.7f %.7f] (x %s xmin  x %s xmax) (y %s ymin  y %s ymax)\n", 
	  prefix,
	  cell,	   
	  cell->xmin, x, cell->xmax,
	  cell->ymin, y, cell->ymax,
	  GTE(x,cell->xmin) ? ">=" : "<",
	  LT(x,cell->xmax) ? "<" : ">=",
	  GTE(y,cell->ymin) ? ">=" : "<",
	  LT(y,cell->ymax) ? "<" : ">=" );
}

// in the tree that contains cell, find the smallest node at x,y. cell
// does not have to be the root. non-recursive for speed.
stg_cell_t* stg_cell_locate( stg_cell_t* cell, double x, double y )
{
  //printf( "\nlocate %.5f, %.5f\n",
  //  x,y );

  //print_thing( "start", cell, x, y );

  // start by going up the tree until the cell contains the point

  // if x,y is NOT contained in the cell we jump to its parent
  while( !( GTE(x,cell->xmin) && 
	    LT(x,cell->xmax) && 
	    GTE(y,cell->ymin) && 
	    LT(y,cell->ymax) ))
    {
      //print_thing( "ascending", cell, x, y );
      
      if( cell->parent )
	cell = cell->parent;
      else
	return NULL; // the point is outside the root node!
    }
  
  //print_thing( "after ascending", cell, x, y );

  // now we know that the point is contained in this cell, we go down
  // the tree to the leaf node that contains the point
  
  // if we have children, we must jump down into the child
  while( cell->children[0] )
    {
      //print_thing( "descending", cell, x, y );

      // choose the right quadrant 
      int index;
      if( LT(x,cell->x) )
	index = LT(y,cell->y) ? 0 : 2; 
      else
	index = LT(y,cell->y) ? 1 : 3; 

      //printf( "descent index %d {%.5f < %.5f}%d {%.5f < %.5f}%d\n",
      //      index, x, cell->x, LT(x,cell->x), y, cell->y, LT(y,cell->y));
      
      cell = cell->children[index];
    }

  //print_thing( "after descending", cell, x, y );

  // the cell has no children and contains the point - we're done.
  return cell;
}

stg_model_t* itl_first_matching( itl_t* itl, 
				 stg_itl_test_func_t func, 
				 stg_model_t* finder )
{
  itl->index = 0;
  itl->models = NULL;  
  stg_model_t* found = NULL;

  stg_cell_t* cell = itl->matrix->root;

  double xstart, ystart;

  while( LT(itl->range,itl->max_range) )
    {
      // locate the leaf cell at X,Y
      cell = stg_cell_locate( cell, itl->x, itl->y );      
      
      // the cell is null iff the point was outside the root
      if( cell == NULL )
	{
	  itl->range = itl->max_range; // stop the ray here
	  return NULL;
	}
      
      if( fig_debug_rays ) // draw the cell rectangle
	stg_rtk_fig_rectangle( fig_debug_rays,
			       cell->x, cell->y, 0, cell->size, cell->size, 0 );
            
      if( cell->data ) 
	{ 
	  stg_model_t* hitmod = 
	    gslist_first_matching( (GSList*)cell->data, func, finder );
	  
	  if( hitmod ) 
	    return hitmod; // done!	  
	}
            
      double c = itl->y - itl->tana * itl->x; // line offset
      
      double xleave = itl->x;
      double yleave = itl->y;
      
      if( GT(itl->a,0) ) // up
	{
	  // ray could leave through the top edge
	  // solve x for known y      
	  yleave = cell->ymax; // top edge
	  xleave = (yleave - c) / itl->tana;
	  
	  // if the edge crossing was not in cell bounds     
	  if( !(GTE(xleave,cell->xmin) && LT(xleave,cell->xmax)) )
	    {
	      // it must have left the cell on the left or right instead 
	      // solve y for known x
	      
	      if( GT(itl->a,M_PI/2.0) ) // left side
		{
		  xleave = cell->xmin-0.00001;
		}
	      else // right side
		{
		  xleave = cell->xmax;
		}
	      
	      yleave = itl->tana * xleave + c;
	    }
	}	 
      else 
	{
	  // ray could leave through the bottom edge
	  // solve x for known y      
	  yleave = cell->ymin; // bottom edge
	  xleave = (yleave - c) / itl->tana;
	  
	  // if the edge crossing was not in cell bounds     
	  if( !(GTE(xleave,cell->xmin) && LT(xleave,cell->xmax)) )
	    {
	      // it must have left the cell on the left or right instead 
	      // solve y for known x	  
	      if( LT(itl->a,-M_PI/2.0) ) // left side
		{
		  xleave = cell->xmin-0.00001;
		}
	      else
		{
		  xleave = cell->xmax;
		}
	      
	      yleave = itl->tana * xleave + c;
	    }
	  else
	    yleave-=0.00001;
	}
      
      // jump slightly into the next cell
      //xleave += 0.0001 * itl->cosa;
      //yleave += 0.0001 * itl->sina;
      
      if( fig_debug_rays ) // draw the cell rectangle
	{
	  stg_rtk_fig_color_rgb32( fig_debug_rays, 0xFFBBBB );
	  stg_rtk_fig_arrow_ex( fig_debug_rays, itl->x, itl->y, xleave, yleave, 0.01 );
	  stg_rtk_fig_color_rgb32( fig_debug_rays, 0xFF0000 );
	}

      // jump to the leave point
      itl->range += hypot( yleave - itl->y, xleave - itl->x );
      
      itl->x = xleave;
      itl->y = yleave;
     
      
    }

  return NULL; // we didn't find anything
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



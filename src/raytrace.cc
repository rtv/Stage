#include <stdlib.h>
#include <math.h>

//#include "stage_internal.h"
#include "model.hh"

//extern stg_rtk_fig_t* fig_debug_rays;

extern int dl_raytrace;

/* useful debug */
static void print_thing( char* prefix, stg_cell_t* cell, double x, double y )
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

/* useful debug */
static void PrintArray( GPtrArray* arr )
{
  if( arr )
    {
      printf( "model array %p len %d\n", arr, arr->len );
      int i;
      for( i=0; i<arr->len; i++ )
	printf( " (model %s)", ((StgModel*)g_ptr_array_index( arr, i ))->Token() );
    }
  else
    printf( "null array\n" );
}

extern stg_world_t* global_world;

void box3d_wireframe( stg_bbox3d_t* bbox );
void push_color( double col[4] );
void pop_color( void );

int stg_model_ray_intersect( StgModel* mod, 
			     double x1, double y1,
			     double x2, double y2,
			     double *hitx, double* hity )
{
  stg_pose_t pose;
  mod->GetPose( &pose ); // yuk - slow!
  
  *hitx = pose.x;
  *hity = pose.y;
  return TRUE;
}



itl_t* itl_create( double x, double y, double z, double a, double b, 
		   stg_matrix_t* matrix, itl_mode_t pmode )
{   
  itl_t* itl = (itl_t*)g_new( itl_t, 1 );
  
  itl->matrix = matrix;
  itl->x = x;
  itl->y = y;
  itl->z = z;
  itl->models = NULL;
  itl->index = 0;
  itl->range = 0;  

  stg_bbox3d_t bbox;

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
	itl->a = atan2( b-y, a-x );
	itl->max_range = hypot( a-x, b-y );       
      }
      break;
    default:
      puts( "Stage Warning: unknown LineIterator mode" );
    }
  //printf( "a = %.2f remaining_range = %.2f\n", itl->a,
  //remaining_range ); fflush( stdout );
  
  // find all the models that overlap with this bbox
  itl->cosa = cos( itl->a );
  itl->sina = sin( itl->a );
  itl->tana = tan( itl->a );
  
  return itl;
};

int hit_range_compare( stg_hit_t* a, stg_hit_t* b )
{
  if( a->range > b->range )
    return 1;

  if( a->range < b->range )
    return -1;

  return 0;
}

StgModel* stg_first_model_on_ray( double x, double y, double z, 
				  double a, double b, 
				  stg_matrix_t* matrix, 
				  itl_mode_t pmode,
				  stg_itl_test_func_t func, 
				  StgModel* finder,
				  stg_meters_t* hitrange,
				  stg_meters_t* hitx,
				  stg_meters_t* hity )
{
  itl_t* itl = itl_create( x,y,z,a,b,matrix,pmode );

  StgModel* hit = itl_first_matching( itl, func, finder );

  if( hit )
    {
      *hitrange = itl->range;
      *hitx = itl->x;
      *hity = itl->y;
    }

  itl_destroy( itl );

  return hit;
}


void itl_destroy( itl_t* itl )
{
  if( itl )
      g_free( itl );
}



/** Returns TRUE iff the vertical center of the first model falls
    within the height of the second model. Used for testing ray hits.
*/
int stg_model_height_check( StgModel* mod1, StgModel* mod2 ) 
{
  // TODO - optimize this - it's very slow and gets called a
  // LOT. probably should cache the global extents of each model to
  // avoid the get_global_pose()..

  stg_pose_t gpose1, gpose2;  
  mod1->GetGlobalPose( &gpose1 );
  mod2->GetGlobalPose( &gpose2 );

  stg_geom_t geom;  
  mod1->GetGeom( &geom );
  
  stg_meters_t look_height = gpose1.z + geom.size.z/2.0;

  stg_meters_t mod2_bottom = gpose2.z;
  stg_meters_t mod2_top = gpose2.z + geom.size.z;

  return( (look_height >= mod2_bottom) && (look_height <= mod2_top ) );
}


// returns the first model in the array that matches, else NULL.
static StgModel* gslist_first_matching( GSList* list, 
					stg_meters_t z,
					stg_itl_test_func_t func, 
					StgModel* finder )
{
  assert(list);

  //printf( "Finder %s is testing..\n", finder->Token() );

  for( ; list ; list=list->next )
    {
      assert(list->data);

      
      stg_block_t* block = (stg_block_t*)list->data;

      // test to see if the block exists at height z
      StgModel* candidate = block->mod;

      assert( candidate );

      //printf( "block of %s\n", candidate->Token() );

      stg_pose_t gpose;
      candidate->GetGlobalPose( &gpose );

     double block_min = gpose.z + block->zmin;
      double block_max = gpose.z + block->zmax;
      
      if( block_min < z && 
	  block_max > z && 
	  (*func)( finder, candidate ) )
	return candidate;
    }
  
  return NULL; // nothing in the array matched
}


// in the tree that contains cell, find the smallest node at x,y. cell
// does not have to be the root. non-recursive for speed.
stg_cell_t* stg_cell_locate( stg_cell_t* cell, double x, double y )
{
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
  
  // now we know that the point is contained in this cell, we go down
  // the tree to the leaf node that contains the point
  
  // if we have children, we must jump down into the child
  while( cell->children[0] )
    {
      // choose the right quadrant 
      int index;
      if( LT(x,cell->x) )
	index = LT(y,cell->y) ? 0 : 2; 
      else
	index = LT(y,cell->y) ? 1 : 3; 

      cell = cell->children[index];
    }

  // the cell has no children and contains the point - we're done.
  return cell;
}

StgModel* itl_first_matching( itl_t* itl, 
			      stg_itl_test_func_t func, 
			      StgModel* finder )
{
  itl->index = 0;
  itl->models = NULL;  

  stg_cell_t* cell = itl->matrix->root;

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
      
      //if( fig_debug_rays ) // draw the cell rectangle
	  //stg_rtk_fig_rectangle( fig_debug_rays,
      //	       cell->x, cell->y, 0, 
      //		       cell->size, cell->size, 0 );            

      if( cell->data ) 
	{ 
	  StgModel* hitmod = 
	    gslist_first_matching( (GSList*)cell->data, itl->z, func, finder );
	  
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
      
      //     if( fig_debug_rays ) // draw the cell rectangle
      //{
      //  stg_rtk_fig_color_rgb32( fig_debug_rays, 0xFFBBBB );
      //  stg_rtk_fig_arrow_ex( fig_debug_rays, 
      //			itl->x, itl->y, xleave, yleave, 0.01 );
      //  stg_rtk_fig_color_rgb32( fig_debug_rays, 0xFF0000 );
      //}

      // jump to the leave point
      itl->range += hypot( yleave - itl->y, xleave - itl->x );
      
      itl->x = xleave;
      itl->y = yleave;      
    }

  return NULL; // we didn't find anything
}

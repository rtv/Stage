/*************************************************************************
 * RTV
 * $Id: cell.cc,v 1.1.2.2 2007-10-13 07:42:55 rtv Exp $
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h> // for memcpy(3)
#include <GL/gl.h>

#include "stage.hh"

#define DEBUG


StgCell::StgCell( StgCell* parent, 
		  int32_t xmin, int32_t xmax, 
		  int32_t ymin, int32_t ymax )
{
  this->parent = parent;

  this->xmin = xmin;
  this->xmax = xmax;
  this->ymin = ymin;
  this->ymax = ymax;
  
  // cache w and h for fast checking
  this->width = this->xmax - this->xmin;
  this->height = this->ymax - this->ymin;
  
  this->split = STG_SPLIT_NONE;
  this->left = NULL;
  this->right = NULL;
  this->list = NULL;
}

StgCell::~StgCell()
{
  // recursively delete child cells
  if( left )
    delete left;

  if( right )
    delete right;
}

bool StgCell::Atomic()
{
  //printf( "atomic test width %d height %d atom %d\n",
  //  width, height, sz );

  return( (width < 2) && (height < 2)); 
}

GLvoid glPrint( const char *fmt, ... );

void StgCell::Draw()
{
  //this->Print( "Draw" );
  //printf( "left %.2f right %.2f bottom %.2f top %.2f\n",
  //  left, right, bottom, top );

  glRecti( xmin, ymin, xmax, ymax );


/*   int offset = 0; */

/*   if( this->data ) */
/*     { */
      
/*       int offset = 0; */
/*       for( GSList* it = (GSList*)this->data; it; it=it->next ) */
/* 	{ */
/* 	  glRasterPos2f( this->x, this->y + 0.02*offset++ ); */
/* 	  glPrint( "%s", ((stg_block_t*)it->data)->mod->Token()); */
/* 	}       */
/*     }   */
}

void StgCell::DrawTree( bool leaf_only )
{
  if( (!leaf_only) || (leaf_only && list) ) 
    Draw();
  
  if( left )
    left->DrawTree( leaf_only );

  if( right )
    right->DrawTree( leaf_only );
}

void StgCell::Split()
{
  //printf( "Splitting %p\n", this );

  assert( left == NULL ); // make sure
  assert( right == NULL );
  
  // split the larger dimension into two
  if( width > height )
    {
      split = STG_SPLIT_X;
      
      left = new StgCell( this, 
			  xmin, xmin+width/2,
			  ymin, ymax );
      
      right = new StgCell( this, 
			   xmin+width/2, xmax,
			   ymin, ymax );
    }
  else
    {
      split = STG_SPLIT_Y;

      left = new StgCell( this, 
			  xmin, xmax,
			  ymin, ymin+height/2 );
      
      right = new StgCell( this,
			   xmin, xmax,
			   ymin+height/2, ymax );
    }

  //left->Draw();
  //right->Draw();

  //this->Print( "this" );
  //left->Print( "left" );
  //right->Print( "right" );
  //puts("");
}

void StgCell::Print( char* prefix )
{
  printf( "%s: cell %p at [%d:%d][%d:%d] (%d:%d) split %d left %p right %p list %p\n",
	  prefix ? prefix : "",
	  this, 
	  xmin,
	  xmax,
	  ymin,
	  ymax,
	  width,
	  height,
	  (int)split,
	  left, 
	  right,
	  list );  
}

void StgCell::AddBlock( StgBlock* block )
{
  assert( block );

  list = g_slist_prepend( list, block ); 
  
  assert(list);
  
  block->AddCell( this );
}

void StgCell::RemoveBlock( StgCell* cell, StgBlock* block )
{
  assert( cell );
  assert( cell->list );
  assert( block );
  
  cell->list = g_slist_remove( cell->list, block ); 
  
  // if the cell is empty and has a parent, we might be able to delete it
  if( cell->list == NULL && cell->parent )
    {
      // hop up in the tree
      cell = cell->parent;
      
      // while all children are empty
      while( cell && 
	     !(cell->left->left || cell->left->list ||
	       cell->right->left || cell->right->list ))
	{	      
	  // detach siblings from parent and free them
	  
	  delete cell->left;
	  delete cell->right;
	  cell->left = NULL;
	  cell->right = NULL;	  
	  cell->split = STG_SPLIT_NONE;

	  cell = cell->parent;
	}
    }     
}


// StgCell* StgCell::ExpandRight()
// {
//   Print( "ExpandRight" );
//   assert( parent == NULL );

//   parent = new StgCell( NULL, 
// 			xmin, xmax+width, 
// 			ymin, ymax );
  
//   parent->split = STG_SPLIT_X;
  
//   parent->left = this;      
//   parent->right = new StgCell( parent,
// 			       xmax, xmax+width,
// 			       ymin, ymax );
//   return parent;
// }


// StgCell* StgCell::ExpandLeft()
// {
//   Print( "ExpandLeft" );
//   assert( parent == NULL );

//   parent = new StgCell( NULL, 
// 			xmin-width, xmax, 
// 			ymin, ymax );
  
//   parent->split = STG_SPLIT_X;
  
//   parent->right = this;
//   parent->left = new StgCell( parent, STG_EXPAND_LEFT,
// 			      xmin-width, xmin,
// 			      ymin, ymax );

//   return parent;
// }

// StgCell* StgCell::ExpandDown()
// {
//   Print( "ExpandDown" );
//   assert( parent == NULL );

//   parent = new StgCell( NULL, STG_EXPAND_DOWN,
// 			xmin, xmax, 
// 			ymin-height, ymax );
  
//   parent->split = STG_SPLIT_Y;
  
//   parent->right = this;
//   parent->left = new StgCell( parent,
// 			      xmin, xmax,
// 			      ymin-height, ymin );
  
//   return parent;
// }

// StgCell* StgCell::ExpandUp()
// {
//   Print( "ExpandUp" );
//   assert( parent == NULL );
  
//   parent = new StgCell( NULL, STG_EXPAND_UP,
// 			xmin, xmax, 
// 			ymin, ymax+height );
  
//   parent->split = STG_SPLIT_Y;
  
//   parent->left = this;
//   parent->right = new StgCell( parent, STG_EXPAND_UP,
// 			       xmin, xmax,
// 			       ymax, ymax+height );
 
//   return parent;
// }


// StgCell* StgCell::Ascend()
// {
//   if( parent == NULL )
//     {
//       switch( expand_dir )
// 	{
// 	case STG_EXPAND_DOWN: return ExpandRight();
// 	case STG_EXPAND_RIGHT: return ExpandUp();
// 	case STG_EXPAND_UP: return ExpandLeft();
// 	case STG_EXPAND_LEFT: return ExpandDown();
// 	default:
// 	  PRINT_ERR1( "Invalid expand direction (%d)", expand_dir );
// 	}
//     }
  
//   return parent;
// }


bool StgCell::Contains( int32_t x, int32_t y )
{
  return( x >= xmin && x < xmax && y >= ymin && y < ymax );
}


StgCell* StgCell::LocateAtom( int32_t x, int32_t y )
{
  // locate the leaf cell at X,Y
  StgCell* cell = Locate( x,y );
  
  // if the cell isn't small enough, we need to create children
  if( cell )
    while( ! cell->Atomic() )
      {
	cell->Split();	  
	cell = cell->Locate( x,y );	  
      }
  
  return cell;
} 



// in the tree that contains this cell, find the smallest node at
// x,y. this cell does not have to be the root. non-recursive for
// speed.
StgCell* StgCell::Locate( int32_t x, int32_t y )
{
  // start by going up the tree until the cell contains the point
  
  //printf( "\nLocating (%d %d)\n", x, y );  
  //cell->Print( "START" );
  
  StgCell * cell = this;
  
  while( cell && ! cell->Contains( x, y ) )
    cell = cell->parent;
  
  if( cell == NULL )
    return NULL;

  // now we know that the point is contained in this cell, we go down
  // the tree to the leaf node that contains the point
  
  // if the cell has children, we must jump down into the child
  while( cell->split != STG_SPLIT_NONE )
    {
      assert( cell->left ); // really should be there!
      assert( cell->right ); // really should be there!

      switch( cell->split )
	{
	case STG_SPLIT_X:
	  {	    
	    cell = (x < cell->left->xmax) ? cell->left : cell->right;

// 	    if( x < cell->left->xmax )
// 	      {
// 		puts( " going left" );
// 		cell = cell->left;
// 	      }
// 	    else
// 	      {
// 		puts( " going right" );
// 		cell = cell->right;
// 	      }
	  }		
	  break;
	case STG_SPLIT_Y:
	  {
	    cell = (y < cell->left->ymax ) ? cell->left : cell->right;

// 	    if( y < cell->left->ymax )
// 	      {
// 		puts( " going left (down)" );
// 		cell = cell->left;
// 	      }
// 	    else
// 	      {
// 		puts( " going right (up)" );
// 		cell = cell->right;
// 	      }
	  }		
	  break;
	  
	case STG_SPLIT_NONE:
	default:
	  PRINT_ERR2( "Cell %p has invalid split value %d\n", 
		      cell, cell->split );
	}

      //cell->Print( "Descended into " );
    }

  //cell->Print( "FOUND" );

  // the cell has no children and contains the point - we're done.
  return cell;
}

/*************************************************************************
 * RTV
 * $Id: cell.cc,v 1.1.2.1 2007-10-04 01:17:02 rtv Exp $
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h> // for memcpy(3)
#include <GL/gl.h>

#include "stage.hh"

#define DEBUG

StgCell::StgCell( StgCell* parent, double x, double y, double sz )
{
  this->parent = parent;
  this->x = x;
  this->y = y;
  this->size = sz;

  this->list = NULL;
  
  // store bounds for fast checking
  this->xmin = x - size/2.0;
  this->xmax = x + size/2.0;
  this->ymin = y - size/2.0;
  this->ymax = y + size/2.0;

  for( int i=0; i<4; i++ )
    this->children[i] = NULL;
}

StgCell::~StgCell()
{
  // recursively delete child cells
  for( int i=0; i<4; i++ )
    if( children[i] )
      delete children[i];
}


GLvoid glPrint( const char *fmt, ... );

void StgCell::Draw()
{
  double dx = this->size/2.0;
  glRectf( this->x-dx , this->y-dx, this->x+dx, this->y+dx );
  
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
  
  for( int i=0; i<4; i++ )
    if( children[i] )
      children[i]->DrawTree( leaf_only );
}


void StgCell::Split()
{
  assert( children[0] == NULL ); // make sure
  assert( children[1] == NULL ); // make sure
  assert( children[2] == NULL ); // make sure
  assert( children[3] == NULL ); // make sure
  
  double quarter = size/4.0;
  double half = size/2.0;
  
  children[0] = new StgCell( this, 
			     x - quarter,
			     y - quarter,
			     half );
  
  children[1] = new StgCell( this, 
			     x + quarter,
			     y - quarter,
			     half );
  
  children[2] = new StgCell( this, 
			     x - quarter,
			     y + quarter,
			     half );
  
  children[3] = new StgCell( this, 
			     x + quarter,
			     y + quarter,
			     half );
}

void StgCell::Print( char* prefix )
{
  printf( "%s: cell %p at %.4f,%.4f size %.4f [%.4f:%.4f][%.4f:%.4f] children %p %p %p %p list %p\n",
	  prefix,
	  this, 
	  this->x, 
	  this->y, 
	  this->size, 
	  this->xmin,
	  this->xmax,
	  this->ymin,
	  this->ymax,
	  this->children[0],
	  this->children[1],
	  this->children[2],
	  this->children[3],
	  this->list );  
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
	     !(cell->children[0]->children[0] || cell->children[0]->list ||
	       cell->children[1]->children[0] || cell->children[1]->list ||
	       cell->children[2]->children[0] || cell->children[2]->list ||
	       cell->children[3]->children[0] || cell->children[3]->list) )
	{	      
	  // detach siblings from parent and free them
	  int i;
	  for(i=0; i<4; i++ )
	    {
	      delete cell->children[i];
	      cell->children[i] = NULL; 
	    }
	  
	  cell = cell->parent;
	}
    }     
}

// in the tree that contains this cell, find the smallest node at
// x,y. this cell does not have to be the root. non-recursive for
// speed.
StgCell* StgCell::Locate( StgCell* cell, double x, double y )
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
  
  // if the cell has children, we must jump down into the child
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


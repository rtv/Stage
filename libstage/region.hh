#pragma once
/*
  region.hh
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

#include "stage.hh"

#include <algorithm>
#include <vector>

namespace Stg 
{

// a bit of experimenting suggests that these values are fast. YMMV.
#define RBITS  4 // regions contain (2^RBITS)^2 pixels
#define SBITS  5 // superregions contain (2^SBITS)^2 regions
#define SRBITS (RBITS+SBITS)

#define REGIONWIDTH (1<<RBITS)
#define REGIONSIZE REGIONWIDTH*REGIONWIDTH

#define SUPERREGIONWIDTH (1<<SBITS)
#define SUPERREGIONSIZE SUPERREGIONWIDTH*SUPERREGIONWIDTH


class Cell 
{
  friend class Region;
  friend class SuperRegion;
  friend class World;
  friend class Block;
  
private:
  Region* region;  
  std::vector<Block*> blocks;
  bool boundary;

public:
  Cell() 
	 : region( NULL),
		blocks() 
  { 
  }  
  
  inline void RemoveBlock( Block* b );
  inline void AddBlock( Block* b );  
  inline void AddBlockNoRecord( Block* b );
};  


class Region
{
public:
  
  Cell* cells;
  SuperRegion* superregion;

  static const uint32_t WIDTH;
  static const uint32_t SIZE;

  static int32_t CELL( const int32_t x )
  {  
	 const int32_t _cell_coord_mask = ~ ( ( ~ 0x00 ) << RBITS );
	 return( x & _cell_coord_mask );															  
  }

  unsigned long count; // number of blocks rendered into these cells
  
  Region();
  ~Region();
  
  Cell* GetCellCreate( int32_t x, int32_t y )
  { 
	 if( ! cells )
		{
		  cells = new Cell[REGIONSIZE];
		  for( unsigned int i=0; i<Region::SIZE; i++ )
			 cells[i].region = this;		  
		}
	 return( &cells[CELL(x) + (CELL(y)*Region::WIDTH)] ); 
  }

  Cell* GetCellGlobalCreate( const stg_point_int_t& c ) 
  { 
	 if( ! cells )
		{
		  cells = new Cell[REGIONSIZE];
		  for( unsigned int i=0; i<Region::SIZE; i++ )
			 cells[i].region = this;		  
		}
	 return( &cells[CELL(c.x) + (CELL(c.y)*Region::WIDTH)] ); 
  }
  
  Cell* GetCellGlobalNoCreate( int32_t x, int32_t y ) const
  { 
	 return( &cells[CELL(x) + (CELL(y)*Region::WIDTH)] ); 
  }

  Cell* GetCellLocallNoCreate( int32_t x, int32_t y ) const
  { 
	 return( &cells[x + y*Region::WIDTH] ); 
  }
  

  Cell* GetCellGlobalNoCreate( const stg_point_int_t& c ) const
  { 
	 return( &cells[CELL(c.x) + (CELL(c.y)*Region::WIDTH)] ); 
  }
     


  void DecrementOccupancy();
  void IncrementOccupancy();
};
  

 class SuperRegion
  {
	 friend class World;
	 friend class Model;
	 
  private:
	 static const uint32_t WIDTH;
	 static const uint32_t SIZE;
	 
	 Region regions[SUPERREGIONSIZE];
	 unsigned long count; // number of blocks rendered into these regions
	 
	 stg_point_int_t origin;
	 World* world;
	 
  public:

	 static int32_t REGION( const int32_t x )
	 {  
		const int32_t _region_coord_mask = ~ ( ( ~ 0x00 ) << SRBITS );
		return( ( x & _region_coord_mask ) >> RBITS );
	 }
	 	 
	 
	 SuperRegion( World* world, stg_point_int_t origin );
	 ~SuperRegion();
	 
	 Region* GetRegionGlobal( int32_t x, int32_t y )
	 {
		return( &regions[ REGION(x) + (REGION(y)*SuperRegion::WIDTH) ] );
	 }

	 Region* GetRegionLocal( int32_t x, int32_t y )
	 {
		return( &regions[ x + (y*SuperRegion::WIDTH) ] );
	 }

	 Region* GetRegionGlobal( const stg_point_int_t& r )
	 {
		return( &regions[ REGION(r.x) + (REGION(r.y)*SuperRegion::WIDTH) ] );
	 }
	 
	 void Draw( bool drawall );
	 void Floor();
	 
	 void DecrementOccupancy(){ --count; };
	 void IncrementOccupancy(){ ++count; };
  };

inline void Region::DecrementOccupancy()
{ 
  assert( superregion );
  superregion->DecrementOccupancy();
  --count; 
};

inline void Region::IncrementOccupancy()
{ 
  assert( superregion );
  superregion->IncrementOccupancy();
  ++count; 
}

inline  void printvec( std::vector<Block*>& vec )
  {
	 printf( "Vec: ");
	 for( size_t i=0; i<vec.size(); i++ )
		printf( "%p ", vec[i] );
	 puts( "" );
  }

inline void Cell::RemoveBlock( Block* b )
{
  // linear time removal, but these vectors are very short, usually 1
  // or 2 elements.  Fast removal - our strategy is to copy the last
  // item in the vector over the item we want to remove, then pop off
  // the tail. This avoids moving the other items in the vector. Saves
  // maybe 1 or 2% run time in my tests.
  
  // find the value in the vector     
  //   printf( "\nremoving %p\n", b );
  //   puts( "before" );
  //   printvec( blocks );
  
  // copy the last item in the vector to overwrite this one
  copy_backward( blocks.end(), blocks.end(), std::find( blocks.begin(), blocks.end(), b ));
  blocks.pop_back(); // and remove the redundant copy at the end of
							// the vector
  
  region->DecrementOccupancy();
}

inline void Cell::AddBlock( Block* b )
{
  // constant time prepend
  blocks.push_back( b );
  region->IncrementOccupancy();
  b->RecordRendering( this );
}


}; // namespace Stg

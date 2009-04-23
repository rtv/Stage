#pragma once
/*
  region.hh
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

#include <list> // STL containers
// #include <vector>

#include "stage.hh"

namespace Stg 
{

// a bit of experimenting suggests that these values are fast. YMMV.
#define RBITS  4 // regions contain (2^RBITS)^2 pixels
#define SBITS  4 // superregions contain (2^SBITS)^2 regions
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
  std::list<Block*> blocks;

public:
  Cell() 
	 : region( NULL),
		blocks() 
  { /* empty */ }  
  
  inline void RemoveBlock( Block* b );
  inline void AddBlock( Block* b );  
  inline void AddBlockNoRecord( Block* b );
};


class Region
{
  friend class SuperRegion;
  
private:  
  static const uint32_t WIDTH;
  static const uint32_t SIZE;
  
  Cell* cells;
  SuperRegion* superregion;

public:
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
		return( &cells[x + (y*Region::WIDTH)] ); 

  };
  
  Cell* GetCellNoCreate( int32_t x, int32_t y )
  { 
	 return( &cells[x + (y*Region::WIDTH)] ); 
  };
  
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
	 
	 SuperRegion( World* world, stg_point_int_t origin );
	 ~SuperRegion();
	 
	 Region* GetRegion( int32_t x, int32_t y )
	 {
		return( &regions[ x + (y*SuperRegion::WIDTH) ] );
	 };
	 
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

inline void Cell::RemoveBlock( Block* b )
{
  // linear time removal, but these lists should be very short.
  blocks.remove( b );

  region->DecrementOccupancy();
}

inline void Cell::AddBlock( Block* b )
{
  // constant time prepend
  blocks.push_front( b );
  region->IncrementOccupancy();
  b->RecordRendering( this );
}


}; // namespace Stg

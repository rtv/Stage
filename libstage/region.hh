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
  const int32_t RBITS( 5 ); // regions contain (2^RBITS)^2 pixels
  const int32_t SBITS( 5 );// superregions contain (2^SBITS)^2 regions
  const int32_t SRBITS( RBITS+SBITS );
		
  const int32_t REGIONWIDTH( 1<<RBITS );
  const int32_t REGIONSIZE( REGIONWIDTH*REGIONWIDTH );

  const int32_t SUPERREGIONWIDTH( 1<<SBITS );
  const int32_t SUPERREGIONSIZE( SUPERREGIONWIDTH*SUPERREGIONWIDTH );
		
  const int32_t CELLMASK( ~((~0x00)<< RBITS ));
  const int32_t REGIONMASK( ~((~0x00)<< SRBITS ));
		
  inline int32_t GETCELL( const int32_t x ) { return( x & CELLMASK); }
  inline int32_t GETREG(  const int32_t x ) { return( ( x & REGIONMASK ) >> RBITS); }
  inline int32_t GETSREG( const int32_t x ) { return( x >> SRBITS); }

  // this is slightly faster than the inline method above, but not as safe
  //#define GETREG(X) (( (static_cast<int32_t>(X)) & REGIONMASK ) >> RBITS)


  class Cell 
  {
	 friend class Region;
	 friend class SuperRegion;
	 friend class World;
	 friend class Block;
  
  private:
	 Region* region;  
	 std::vector<Block*> blocks;
	 //std::set<Block*> blocks;
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
	 unsigned long count; // number of blocks rendered into this region
  
	 Region();
	 ~Region();
	 
	 Cell* GetCell( int32_t x, int32_t y )
	 {
		if( ! cells )
		  {
			 cells = new Cell[REGIONSIZE];
			 
			 for( int i=0; i<REGIONSIZE; ++i )
				cells[i].region = this;
		  }

		return( (Cell*)&cells[ x + y * REGIONWIDTH ] ); 
	 }	 
  }; // end class Region

  class SuperRegion
  {
	 friend class World;
	 friend class Model;	 
	 
  private:
	 
	 Region* regions;	 
	 stg_point_int_t origin;
	 World* world;
	 
  public:
	 
	 SuperRegion( World* world, stg_point_int_t origin );
	 ~SuperRegion();
	 
	 Region* GetRegion( int32_t x, int32_t y )
	 { return( &regions[ x + y * SUPERREGIONWIDTH ] ); }
	 
	 void Draw( bool drawall );
	 void Floor();
	 
	 unsigned long count; // number of blocks rendered into this superregion
  }; // class SuperRegion;
  
  void Cell::RemoveBlock( Block* b )
  {
	 // linear time removal, but these vectors are very short, usually 1
	 // or 2 elements.	 
	 blocks.erase( std::remove( blocks.begin(), blocks.end(), b ), blocks.end() );
	 
	 --region->count;
	 --region->superregion->count;  	 	 
  }
  
  void Cell::AddBlock( Block* b )
  {
	 blocks.push_back( b );  
	 b->RecordRendering( this );
	 
	 ++region->count;
	 ++region->superregion->count;
  } 


}; // namespace Stg

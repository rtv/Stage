#pragma once
/*
  region.hh
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

#include "stage.hh"

namespace Stg 
{

  // a bit of experimenting suggests that these values are fast. YMMV.
  //const int32_t RBITS( 5 ); // regions contain (2^RBITS)^2 pixels
  //const int32_t SBITS( 5 );// superregions contain (2^SBITS)^2 regions
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

	//	class Region;

  class Cell 
  {
		//friend class Region;
		friend class SuperRegion;
		friend class World;
		//friend class Block;
	 
  private:
		std::vector<Block*> blocks;		
		
  public:
		Cell() 
			: blocks(), 
				region(NULL)
		{ /* nothing to do */ }  				
		
		void RemoveBlock( Block* b );
		void AddBlock( Block* b );
				
		const std::vector<Block*>& GetBlocks(){ return blocks; }

		Region* region;  
  };  // class Cell
	
  class Region
  {
		friend class SuperRegion;
		friend class World; // for raytracing

  private:
		Cell* cells;
		
		unsigned long count; // number of blocks rendered into this region
		
		// vector of garbage collected cell arrays to reallocate before
		// using new in GetCell()
		static std::vector<Cell*> dead_pool;
		
	public:
		Region();
		~Region();
		
		inline Cell* GetCell( int32_t x, int32_t y )
		{	
			if( cells == NULL )
				{
					assert(count == 0 );
					
					if( dead_pool.size() )
						{
							cells = dead_pool.back();
							dead_pool.pop_back();

							//printf( "reusing cells @ %p (pool %u)\n", cells, dead_pool.size() );
						}
					else
						cells = new Cell[REGIONSIZE];
					
					for( int32_t c=0; c<REGIONSIZE;++c)
						cells[c].region = this;
				}
			
			return( &cells[ x + y * REGIONWIDTH ] );
		}
		
		inline void AddBlock();
		inline void RemoveBlock(); 

		SuperRegion* superregion;	
		
		//bool Occupied() const { return( count > 0 ); }
		//bool Used() const { return( cells ? true : false ); }

	}; // class Region
	
  class SuperRegion
  {
		// friend class World;
		// friend class Model;	 
		
  private:
		Region regions[SUPERREGIONSIZE];
		
		point_int_t origin;
		World* world;
		
		unsigned long count; // number of blocks rendered into this superregion
		
  public:	 
	 SuperRegion( World* world, point_int_t origin );
	 ~SuperRegion();
		
		inline Region* GetRegion( int32_t x, int32_t y )
		{ 
			return( &regions[ x + y * SUPERREGIONWIDTH ]);
		}

		//inline const Region* GetRegionImmut( int32_t x, int32_t y ) const;
		
		void DrawOccupancy() const;
		void DrawVoxels() const;
		
		inline void AddBlock();
		inline void RemoveBlock();		

		const point_int_t& GetOrigin() const { return origin; }
  }; // class SuperRegion;
  
}; // namespace Stg

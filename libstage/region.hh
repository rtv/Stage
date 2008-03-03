/*
  region.hh
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

const double STG_DEFAULT_WORLD_PPM = 50;  // 2cm pixels
const stg_msec_t STG_DEFAULT_WORLD_INTERVAL_REAL = 100; ///< real time between updates
const stg_msec_t STG_DEFAULT_WORLD_INTERVAL_SIM = 100;  ///< duration of sim timestep
const uint32_t RBITS = 6; // regions contain (2^RBITS)^2 pixels
const uint32_t SBITS = 5; // superregions contain (2^SBITS)^2 regions
const uint32_t SRBITS = RBITS+SBITS;

typedef struct
{
  GSList* list;
}  stg_cell_t;

class Stg::Region
{
  friend class SuperRegion;
  
private:  
  static const uint32_t REGIONWIDTH = 1<<RBITS;
  static const uint32_t REGIONSIZE = REGIONWIDTH*REGIONWIDTH;
  
  stg_cell_t cells[REGIONSIZE];
  
public:
  uint32_t count; // number of blocks rendered into these cells
  
  Region()
  {
    bzero( cells, REGIONSIZE * sizeof(stg_cell_t));
    count = 0;
  }
  
  stg_cell_t* GetCell( int32_t x, int32_t y )
  {
    uint32_t index = x + (y*REGIONWIDTH);
    assert( index >=0 );
    assert( index < REGIONSIZE );    
    return &cells[index];    
  }
  
  // add a block to a region cell specified in REGION coordinates
  void AddBlock( StgBlock* block, int32_t x, int32_t y, unsigned int* count2 )
  {
    stg_cell_t* cell = GetCell( x, y );
    cell->list = g_slist_prepend( cell->list, block ); 
    block->RecordRenderPoint( &cell->list, cell->list, &this->count, count2 );    
    count++;
  }  
};


class Stg::SuperRegion
{
  friend class StgWorld;
 
private:
  static const uint32_t SUPERREGIONWIDTH = 1<<SBITS;
  static const uint32_t SUPERREGIONSIZE = SUPERREGIONWIDTH*SUPERREGIONWIDTH;
  
  Region regions[SUPERREGIONSIZE];  
  uint32_t count; // number of blocks rendered into these regions
  
  stg_point_int_t origin;
  
public:
  
  SuperRegion( int32_t x, int32_t y )
  {
    count = 0;
    origin.x = x;
    origin.y = y;
  }
  
  ~SuperRegion()
  {
  }

  // get the region x,y from the region array
  Region* GetRegion( int32_t x, int32_t y )
  {
    int32_t index =  x + (y*SUPERREGIONWIDTH);
    assert( index >=0 );
    assert( index < (int)SUPERREGIONSIZE );
    return &regions[ index ];
  } 
  
  // add a block to a cell specified in superregion coordinates
  void AddBlock( StgBlock* block, int32_t x, int32_t y )
  {
    GetRegion( x>>RBITS, y>>RBITS )
      ->AddBlock( block,  
		  x - ((x>>RBITS)<<RBITS),
		  y - ((y>>RBITS)<<RBITS),
		  &count);		   
    count++;
  }

  // callback wrapper for Draw()
  static void Draw_cb( gpointer dummykey, 
		       SuperRegion* sr, 
		       gpointer dummyval )
  {
    sr->Draw();
  }
  
  void Draw()
  {
    glPushMatrix();
    glTranslatef( origin.x<<SRBITS, origin.y<<SRBITS,0);
        
    glColor3f( 1,0,0 );    

    for( unsigned int x=0; x<SUPERREGIONWIDTH; x++ )
      for( unsigned int y=0; y<SUPERREGIONWIDTH; y++ )
	{
	  Region* r = GetRegion(x,y);
	  
	  for( unsigned int p=0; p<Region::REGIONWIDTH; p++ )
	    for( unsigned int q=0; q<Region::REGIONWIDTH; q++ )
	      if( r->cells[p+(q*Region::REGIONWIDTH)].list )
		glRecti( p+(x<<RBITS), q+(y<<RBITS),
			 1+p+(x<<RBITS), 1+q+(y<<RBITS) );
	}

    // outline regions
    glColor3f( 0,1,0 );    
    for( unsigned int x=0; x<SUPERREGIONWIDTH; x++ )
      for( unsigned int y=0; y<SUPERREGIONWIDTH; y++ )
 	glRecti( x<<RBITS, y<<RBITS, 
		 (x+1)<<RBITS, (y+1)<<RBITS );
    
    // outline superregion
    glColor3f( 0,0,1 );    
    glRecti( 0,0, 1<<SRBITS, 1<<SRBITS );

    glPopMatrix();    
  }
  
  static void Floor_cb( gpointer dummykey, 
			SuperRegion* sr, 
			gpointer dummyval )
  {
    sr->Floor();
  }
  
  void Floor()
  {
    glPushMatrix();
    glTranslatef( origin.x<<SRBITS, origin.y<<SRBITS, 0 );        
    glRecti( 0,0, 1<<SRBITS, 1<<SRBITS );
    glPopMatrix();    
  }
};

/*
  region.hh
  data structures supporting multi-resolution ray tracing in world class.
  Copyright Richard Vaughan 2008
*/

const uint32_t RBITS = 6; // regions contain (2^RBITS)^2 pixels
const uint32_t SBITS = 5; // superregions contain (2^SBITS)^2 regions
const uint32_t SRBITS = RBITS+SBITS;

class Stg::Region
{
  friend class SuperRegion;
  
private:  
  static const uint32_t REGIONWIDTH = 1<<RBITS;
  static const uint32_t REGIONSIZE = REGIONWIDTH*REGIONWIDTH;
  
  //stg_cell_t cells[REGIONSIZE];
  stg_cell_t* cells;

public:
  uint32_t count; // number of blocks rendered into these cells
  
  Region() 
	 : cells( new stg_cell_t[REGIONSIZE] ),
		count(0)
  { 
	 /* do nothing */ 
  }

  ~Region()
  {
	 delete[] cells;
  }

  stg_cell_t* GetCell( int32_t x, int32_t y )
  {
    uint32_t index = x + (y*REGIONWIDTH);
#ifdef DEBUG
    assert( index >=0 );
    assert( index < REGIONSIZE );
#endif
	  
   printf("GET CELL [%d %d] index %d gives %p\n",
		  x, y, index, &cells[index] );
	  
    return &cells[index];    
  }
  
  // add a block to a region cell specified in REGION coordinates
  void AddBlock( Model* mod, 
					  stg_color_t col,
					  Bounds zbounds,
					  int32_t x, int32_t y, 
					  unsigned int* count2 )
  {
	 assert( mod );

    stg_cell_t* cell = GetCell( x, y );
	 assert( cell );

	 // put a new empty pointer on the front of the list to use below
    cell->list = g_slist_prepend( cell->list, NULL ); 

	 // this is the data to be stored in this cell
	 stg_list_entry_t el;
	 el.cell = cell;
	 el.link = cell->list;
	 el.counter1 = &count;
	 el.counter2 = count2;
	 el.mod = mod;
	 el.color = col;
	 el.zbounds = zbounds;

	 // copy this struct onto the end of the model's array
	 g_array_append_val( mod->rendered_points, el );
	 
	 // stash this record at the head of the list
	 cell->list->data = 
		& g_array_index( mod->rendered_points, 
							  stg_list_entry_t, 
							  mod->rendered_points->len-1 );

	 count++;
  }  

};


class Stg::SuperRegion
{
  friend class World;
 
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
  void AddBlock( Model* mod, 
					  stg_color_t col, 
					  Bounds zbounds, 
					  int32_t x, int32_t y )
  {
    GetRegion( x>>RBITS, y>>RBITS )
      ->AddBlock( mod, col, zbounds,   
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

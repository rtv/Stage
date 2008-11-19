/** Stage internal header file - all libstage implementation *.cc
    files include this header 
    Author: Richard Vaughan (vaughan@sfu.ca)
    Date: 13 Jan 2008
    $Id$
*/

#ifndef STG_INTERNAL_H
#define STG_INTERNAL_H

// external definitions for libstage users
#include "stage.hh" 

// internal-only definitions and includes go below

#include "worldfile.hh"


// implementation files use our namespace by default
using namespace Stg;


namespace Stg 
{


// COLOR STACK CLASS
class GlColorStack
{
	public:
		GlColorStack();
		~GlColorStack();

		void Push( GLdouble col[4] );
		void Push( double r, double g, double b, double a );
		void Push( double r, double g, double b );
		void Push( stg_color_t col );

		void Pop();

		unsigned int Length()
		{ return g_queue_get_length( colorstack ); }

	private:
		GQueue* colorstack;
};


class StgCanvas : public Fl_Gl_Window
{
	friend class StgWorldGui; // allow access to private members
	friend class StgModel;
  
private:
	GlColorStack colorstack;

   GList* models_sorted;

	Camera* current_camera;
	OrthoCamera camera;
	PerspectiveCamera perspective_camera;
	bool dirty_buffer;
	Worldfile* wf;

	int startx, starty;
	bool selectedModel;
	bool clicked_empty_space;
	int empty_space_startx, empty_space_starty;
	GList* selected_models; ///< a list of models that are currently
	///selected by the user
	StgModel* last_selection; ///< the most recently selected model
	///(even if it is now unselected).

	stg_msec_t interval; // window refresh interval in ms

	GList* ray_list;
	void RecordRay( double x1, double y1, double x2, double y2 );
	void DrawRays();
	void ClearRays();
	void DrawGlobalGrid();
  
  void AddModel( StgModel* mod );

	Option
		showBlinken, 
		showBlocks, 
		showClock, 
		showData, 
		showFlags,
		showFollow,
		showFootprints, 
		showGrid, 
		showOccupancy, 
		showScreenshots,
		showStatus,
		showTrailArrows, 
		showTrailRise, 
		showTrails, 
		showTree,
		pCamOn,
		visualizeAll;
  
public:
	StgCanvas( StgWorldGui* world, int x, int y, int W,int H);
	~StgCanvas();

	bool graphics;
	StgWorldGui* world;

	void Screenshot();
  
	void createMenuItems( Fl_Menu_Bar* menu, std::string path );
  
	void FixViewport(int W,int H);
	void DrawFloor(); //simpler floor compared to grid
	void DrawBlocks();
	void resetCamera();
	virtual void renderFrame();
	virtual void draw();
	virtual int handle( int event );
	void resize(int X,int Y,int W,int H);

	void CanvasToWorld( int px, int py, 
			double *wx, double *wy, double* wz );

	StgModel* getModel( int x, int y );
	bool selected( StgModel* mod );
	void select( StgModel* mod );
	void unSelect( StgModel* mod );
	void unSelectAll();

	inline void setDirtyBuffer( void ) { dirty_buffer = true; }
	inline bool dirtyBuffer( void ) const { return dirty_buffer; }
	
	inline void PushColor( stg_color_t col )
	{ colorstack.Push( col ); } 

	void PushColor( double r, double g, double b, double a )
	{ colorstack.Push( r,g,b,a ); }

	void PopColor(){ colorstack.Pop(); } 
  
  void InvertView( uint32_t invertflags );
  
  static void TimerCallback( StgCanvas* canvas );
  static void perspectiveCb( Fl_Widget* w, void* p );
  
  void Load( Worldfile* wf, int section );
  void Save( Worldfile* wf, int section );
};


class Cell 
{
  friend class Region;
  friend class SuperRegion;
  friend class StgWorld;
  friend class StgBlock;
  
private:
  Region* region;
  GSList* list;
public:
  Cell() 
	 : region( NULL),
		list(NULL) 
  { /* do nothing */ }  
  
  inline void RemoveBlock( StgBlock* b );
  inline void AddBlock( StgBlock* b );  
  inline void AddBlockNoRecord( StgBlock* b );
};

// a bit of experimenting suggests that these values are fast. YMMV.
#define RBITS  4 // regions contain (2^RBITS)^2 pixels
#define SBITS  4 // superregions contain (2^SBITS)^2 regions
#define SRBITS (RBITS+SBITS)

#define REGIONWIDTH (1<<RBITS)
#define REGIONSIZE REGIONWIDTH*REGIONWIDTH

#define SUPERREGIONWIDTH (1<<SBITS)
#define SUPERREGIONSIZE SUPERREGIONWIDTH*SUPERREGIONWIDTH

class Region
{
  friend class SuperRegion;
  
private:  
  static const uint32_t WIDTH;
  static const uint32_t SIZE;
  
  Cell cells[REGIONSIZE];
  SuperRegion* superregion;
  
public:
  unsigned long count; // number of blocks rendered into these cells
  
  Region();
  ~Region();
  
  Cell* GetCell( int32_t x, int32_t y )
  { 
	 return( &cells[x + (y*Region::WIDTH)] ); 
  };
  
  void DecrementOccupancy();
  void IncrementOccupancy();
};


class SuperRegion
{
  friend class StgWorld;
  friend class StgModel;
  
private:
  static const uint32_t WIDTH;
  static const uint32_t SIZE;
  
  Region regions[SUPERREGIONSIZE];
  unsigned long count; // number of blocks rendered into these regions
  
  stg_point_int_t origin;
  
public:
  
  SuperRegion( int32_t x, int32_t y );
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
  

// INLINE METHOD DEFITIONS

inline void Region::DecrementOccupancy()
{ 
  assert( superregion );
  superregion->DecrementOccupancy();
  --count; 
}

inline void Region::IncrementOccupancy()
{ 
  assert( superregion );
  superregion->IncrementOccupancy();
  ++count; 
}

inline void Cell::RemoveBlock( StgBlock* b )
{
  // linear time removal, but these lists should be very short.
  list = g_slist_remove( list, b );
  region->DecrementOccupancy();
}

inline void Cell::AddBlock( StgBlock* b )
{
  // constant time prepend
  list = g_slist_prepend( list, b );	 
  region->IncrementOccupancy();
  b->RecordRendering( this );
}

inline void Cell::AddBlockNoRecord( StgBlock* b )
{
  list = g_slist_prepend( list, b );
  // don't add this cell to the block - we assume it's already there
}

}; // namespace Stg



#endif // STG_INTERNAL_H

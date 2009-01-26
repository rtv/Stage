
#include "stage.hh"
#include "option.hh"

#include <map>

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


class Canvas : public Fl_Gl_Window
{
	friend class WorldGui; // allow access to private members
	friend class Model;
  
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
	Model* last_selection; ///< the most recently selected model
	///(even if it is now unselected).

	stg_msec_t interval; // window refresh interval in ms

	GList* ray_list;
	void RecordRay( double x1, double y1, double x2, double y2 );
	void DrawRays();
	void ClearRays();
	void DrawGlobalGrid();
  
  void AddModel( Model* mod );

  Option showBlinken, 
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
	 showBBoxes,
	 showBlur,
	 pCamOn,
	 visualizeAll;
  
public:
  Canvas( WorldGui* world, int x, int y, int width, int height);
	~Canvas();
  
  bool graphics;
  WorldGui* world;
  unsigned long frames_rendered_count;
  int screenshot_frame_skip;
  
  std::map< std::string, Option* > _custom_options;

	void Screenshot();
  void InitGl();
	void createMenuItems( Fl_Menu_Bar* menu, std::string path );
  
	void FixViewport(int W,int H);
	void DrawFloor(); //simpler floor compared to grid
	void DrawBlocks();
  void DrawBoundingBoxes();
	void resetCamera();
	virtual void renderFrame();
	virtual void draw();
	virtual int handle( int event );
	void resize(int X,int Y,int W,int H);

	void CanvasToWorld( int px, int py, 
			double *wx, double *wy, double* wz );

	Model* getModel( int x, int y );
	bool selected( Model* mod );
	void select( Model* mod );
	void unSelect( Model* mod );
	void unSelectAll();

	inline void setDirtyBuffer( void ) { dirty_buffer = true; }
	inline bool dirtyBuffer( void ) const { return dirty_buffer; }
	
	inline void PushColor( stg_color_t col )
	{ colorstack.Push( col ); } 

	void PushColor( double r, double g, double b, double a )
	{ colorstack.Push( r,g,b,a ); }

	void PopColor(){ colorstack.Pop(); } 
  
  void InvertView( uint32_t invertflags );
  
  static void TimerCallback( Canvas* canvas );
  static void perspectiveCb( Fl_Widget* w, void* p );
  
  void Load( Worldfile* wf, int section );
  void Save( Worldfile* wf, int section );
};

} // namespace Stg


#include "stage.hh"

namespace Stg
{
  /** @brief Internal low-level drawing convenience routines. */
  namespace Gl
  {
	 void pose_shift( const Pose &pose );
	 void pose_inverse_shift( const Pose &pose );
	 void coord_shift( double x, double y, double z, double a  );
	 void draw_grid( stg_bounds3d_t vol );
	 /** Render a string at [x,y,z] in the current color */
	 void draw_string( float x, float y, float z, const char *string);
	 void draw_speech_bubble( float x, float y, float z, const char* str );
	 void draw_octagon( float w, float h, float m );
	 void draw_vector( double x, double y, double z );
	 void draw_origin( double len );
  }
}

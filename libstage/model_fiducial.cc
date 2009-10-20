///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_fiducial.cc,v $
//  $Author: rtv $
//  $Revision: 1.4 $
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG 1

#include "stage.hh"
#include "option.hh"
#include "worldfile.hh"
using namespace Stg;

//TODO make instance attempt to register an option (as customvisualizations do)
Option ModelFiducial::showData( "Fiducials", "show_fiducial", "", true, NULL );
Option ModelFiducial::showFov( "Fiducial FOV", "show_fiducial_fov", "", false, NULL );

/** 
  @ingroup model
  @defgroup model_fiducial Fiducial detector model

  The fiducial model simulates a fiducial-detecting device.

API: Stg::ModelFiducial

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
fiducial
(
  # fiducial properties
  range_min 0.0
  range_max 8.0
  range_max_id 5.0
  fov 3.14159

  # model properties
  size [ 0.1 0.1 0.1 ]
)
@endverbatim

@par Details
 
- range_min <float>\n
  the minimum range reported by the sensor, in meters. The sensor will detect objects closer than this, but report their range as the minimum.
- range_max <float>\n
  the maximum range at which the sensor can detect a fiducial, in meters. The sensor may not be able to uinquely identify the fiducial, depending on the value of range_max_id.
- range_max_id <float>\n
  the maximum range at which the sensor can detect the ID of a fiducial, in meters.
- fov <float>
  the angular field of view of the scanner, in radians.

 */
  
  ModelFiducial::ModelFiducial( World* world, 
										  Model* parent,
										  const std::string& type ) : 
  Model( world, parent, type ),
  fiducials(),
  max_range_anon( 8.0 ),
  max_range_id( 5.0 ),
  min_range( 0.0 ),
  fov( M_PI ),
  heading( 0 ),
  key( 0 )
{
  //PRINT_DEBUG2( "Constructing ModelFiducial %d (%s)\n", 
  //		id, typestr );
  
  // assert that Update() is reentrant for this derived model
  thread_safe = true;
  
  // sensible fiducial defaults 
  //  interval = 200; // common for a SICK LMS200
  
  this->ClearBlocks();
  
  Geom geom;
  geom.Zero();
  SetGeom( geom );
  
  RegisterOption( &showData );
  RegisterOption( &showFov );
}

ModelFiducial::~ModelFiducial( void )
{
}

static bool fiducial_raytrace_match( Model* candidate, 
												 Model* finder, 
												 const void* dummy )
{
  (void)dummy; // avoid warning about unused var

  return( ! finder->IsRelated( candidate ) );
}	


void ModelFiducial::AddModelIfVisible( Model* him )  
{
	//PRINT_DEBUG2( "Fiducial %s is testing model %s", token, him->Token() );

	// don't consider models with invalid returns  
	if( him->vis.fiducial_return == 0 )
	{
		//PRINT_DEBUG1( "  but model %s has a zero fiducial ID", him->Token());
		return;
	}

	// check to see if this neighbor has the right fiducial key
	if( vis.fiducial_key != him->vis.fiducial_key )
	{
		//PRINT_DEBUG1( "  but model %s doesn't match the fiducial key", him->Token());
		return;
	}

	Pose mypose = this->GetGlobalPose();

	// are we within range?
	Pose hispose = him->GetGlobalPose();
	double dx = hispose.x - mypose.x;
	double dy = hispose.y - mypose.y;
	double range = hypot( dy, dx );

	//  printf( "range to target %.2f m (

	if( range > max_range_anon )
	{
		//PRINT_DEBUG3( "  but model %s is %.2f m away, outside my range of %.2f m", 
		//	    him->Token(),
		//	    range,
		//	    max_range_anon );
		return;
	}

	// is he in my field of view?
	double bearing = atan2( dy, dx );
	double dtheta = normalize(bearing - mypose.a);

	if( fabs(dtheta) > fov/2.0 )
	{
		//PRINT_DEBUG1( "  but model %s is outside my FOV", him->Token());
		return;
	}

	if( IsRelated( him ) )
		return;

	PRINT_DEBUG1( "  %s is a candidate. doing ray trace", him->Token());


	//printf( "bearing %.2f\n", RTOD(bearing) );
	
	stg_raytrace_result_t ray( Raytrace( dtheta,
													 max_range_anon,
													 fiducial_raytrace_match,
													 NULL,
													 true ) );
	
	Model* hitmod( ray.mod );

 	//printf( "ray hit %s and was seeking LOS to %s\n",
	//	  hitmod ? hitmod->Token() : "null",
	//	  him->Token() );

	// if it was him, we can see him
	if( hitmod != him )
	  return;

	assert( range >= 0 );
	
	// passed all the tests! record the fiducial hit
	
	Geom hisgeom( him->GetGeom() );
	
	// record where we saw him and what he looked like
	Fiducial fid;
	fid.mod = him;
	fid.range = range;
	fid.bearing = dtheta;
	fid.geom.x = hisgeom.size.x;
	fid.geom.y = hisgeom.size.y;
	fid.geom.a = normalize( hispose.a - mypose.a);
	
	// store the global pose of the fiducial (mainly for the GUI)
	fid.pose = hispose;
	
	// if he's within ID range, get his fiducial.return value, else
	// we see value 0
	fid.id = range < max_range_id ? hitmod->vis.fiducial_return : 0;
	
	PRINT_DEBUG2( "adding %s's value %d to my list of fiducials",
					  him->Token(), him->vis.fiducial_return );
	
	fiducials.push_back( fid );
}	

///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void ModelFiducial::Update( void )
{
	PRINT_DEBUG( "fiducial update" );

	if( subs < 1 )
		return;
	
	// reset the array of detected fiducials
	fiducials.clear();
	
	FOR_EACH( it, world->models_with_fiducials )
	  AddModelIfVisible( *it );	

	Model::Update();
}

void ModelFiducial::Load( void )
{  
	PRINT_DEBUG( "fiducial load" );

	Model::Load();

	// load fiducial-specific properties
	min_range      = wf->ReadLength( wf_entity, "range_min",    min_range );
	max_range_anon = wf->ReadLength( wf_entity, "range_max",    max_range_anon );
	max_range_id   = wf->ReadLength( wf_entity, "range_max_id", max_range_id );
	fov            = wf->ReadAngle( wf_entity, "fov",          fov );
}  


void ModelFiducial::DataVisualize( Camera* cam )
{
  (void)cam; // avoid warning about unused var

	if( showFov )
	  {
		 PushColor( 1,0,1,0.2  ); // magenta, with a bit of alpha

		 GLUquadric* quadric = gluNewQuadric();
		 
		 gluQuadricDrawStyle( quadric, GLU_SILHOUETTE );
		 
		 gluPartialDisk( quadric,
							  0, 
							  max_range_anon,
							  20, // slices	
							  1, // loops
							  rtod( M_PI/2.0 + fov/2.0), // start angle
							  rtod(-fov) ); // sweep angle
		 
		 gluDeleteQuadric( quadric );

		 PopColor();
	  }
	
	if( showData )
	  {
		 PushColor( 1,0,1,0.4  ); // magenta, with a bit of alpha
		 
		 // draw fuzzy dotted lines	
		 glLineWidth( 2.0 );
		 glLineStipple( 1, 0x00FF );
		 
		 // draw lines to the fiducials
		 FOR_EACH( it, fiducials )
			{
			  Fiducial& fid = *it;
			  
			  double dx = fid.range * cos( fid.bearing);
			  double dy = fid.range * sin( fid.bearing);
			  
			  glEnable(GL_LINE_STIPPLE);
			  glBegin( GL_LINES );
			  glVertex2f( 0,0 );
			  glVertex2f( dx, dy );
			  glEnd();
			  glDisable(GL_LINE_STIPPLE);
			  			  
			  glPushMatrix();
			  Gl::coord_shift( dx,dy,0,fid.geom.a );
			  
			  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			  glRectf( -fid.geom.x/2.0, -fid.geom.y/2.0,
						  fid.geom.x/2.0, fid.geom.y/2.0 );
			  
			  // show the fiducial ID
			  char idstr[32];
			  snprintf(idstr, 31, "%d", fid.id );
			  Gl::draw_string( 0,0,0, idstr );
			  
			  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			  glPopMatrix();			
			}
		 
		 PopColor();			 
		 glLineWidth( 1.0 );		
	  }	 
}
	
void ModelFiducial::Shutdown( void )
{ 
	PRINT_DEBUG( "fiducial shutdown" );
	fiducials.clear();	
	Model::Shutdown();
}

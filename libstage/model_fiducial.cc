///////////////////////////////////////////////////////////////////////////
//
// File: model_fiducial.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_fiducial.cc,v $
//  $Author: rtv $
//  $Revision$
//
///////////////////////////////////////////////////////////////////////////

#undef DEBUG 

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
  ignore_zloc 0

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
- ignore_zloc <1/0>\n
  default is 0.  When set to 1, the fiducial finder ignores the z component when checking a fiducial.  Using the default behaviour, a short object would not been seen
  by a fiducial finder placed on top of a tall robot.  With this flag set to 1, the fiducial finder will see the shorter robot.   
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
  key( 0 ),
  ignore_zloc(false)
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

	// only non-zero IDs should ever be checked
	assert( him->vis.fiducial_return != 0 );
	
	// check to see if this neighbor has the right fiducial key
	// if keys are used extensively, then perhaps a vector per key would be faster
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

	if( range >= max_range_anon )
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

	//PRINT_DEBUG1( "  %s is a candidate. doing ray trace", him->Token());


	//printf( "bearing %.2f\n", RTOD(bearing) );

	//If we've gotten to this point, a few things are true:
	// 1. The fiducial is in the field of view of the finder.
	// 2. The fiducial is in range of the finder.
	//At this point the purpose of the ray trace is to start at the finder and see if there is anything
	//in between the finder and the fiducial.  If we simply trace out to the distance we know the finder
	//is at, then the resulting ray.mod can be one of three things:
	// 1. A pointer to the model we're tracing to.  In this case the model is at the right Zloc to be 
	//    returned by the ray tracer.
	// 2. A pointer to another model that blocked the ray.
	// 3. NULL.  If it's null, then it means that the ray traced to where the fiducial should be but
	//    it's zloc was such that the ray didn't hit it.  However, we DO know its there, so we can 
	//    return this as a hit.

	//printf( "range %.2f\n", range );
	
	RaytraceResult ray( Raytrace( dtheta,
																max_range_anon, // TODOscan only as far as the object
																fiducial_raytrace_match,
																NULL,
																true ) );
	
	// TODO
	if( ignore_zloc && ray.mod == NULL ) // i.e. we didn't hit anything *else*
		ray.mod = him; // so he was just at the wrong height
	
	//printf( "ray hit %s and was seeking LOS to %s\n",
	//			ray.mod ? ray.mod->Token() : "null",
	//			him->Token() );
	
	// if it was him, we can see him
	if( ray.mod != him )
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
	fid.geom.z = hisgeom.size.z;
	fid.geom.a = normalize( hispose.a - mypose.a);
	
	//fid.pose_rel = hispose - this->GetGlobalPose();

	// store the global pose of the fiducial (mainly for the GUI)
	fid.pose = hispose;
	
	// if he's within ID range, get his fiducial.return value, else
	// we see value 0
	fid.id = range < max_range_id ? him->vis.fiducial_return : 0;
	
	//PRINT_DEBUG2( "adding %s's value %d to my list of fiducials",
	//			  him->Token(), him->vis.fiducial_return );
	
	fiducials.push_back( fid );
}	


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void ModelFiducial::Update( void )
{
  //PRINT_DEBUG( "fiducial update" );

	if( subs < 1 )
		return;

	// reset the array of detected fiducials
	fiducials.clear();

#if( 1 )	
	// BEGIN EXPERIMENT
	
	// find two sets of fiducial-bearing models, within sensor range on
	// the two different axes
	
	double rng = max_range_anon;
	Pose gp = GetGlobalPose();	
	Model edge;	// dummy model used to find bounds in the sets
	
	edge.pose = Pose( gp.x-rng, gp.y, 0, 0 ); // LEFT
	std::set<Model*,World::ltx>::iterator xmin = 
		world->models_with_fiducials_byx.lower_bound( &edge ); // O(log(n))
	
	edge.pose = Pose( gp.x+rng, gp.y, 0, 0 ); // RIGHT
	const std::set<Model*,World::ltx>::iterator xmax = 
		world->models_with_fiducials_byx.upper_bound( &edge );
	
	edge.pose = Pose( gp.x, gp.y-rng, 0, 0 ); // BOTTOM
	std::set<Model*,World::lty>::iterator ymin = 
		world->models_with_fiducials_byy.lower_bound( &edge );
	
	edge.pose = Pose( gp.x, gp.y+rng, 0, 0 ); // TOP
	const std::set<Model*,World::lty>::iterator ymax = 
		world->models_with_fiducials_byy.upper_bound( &edge );
		
	// put these models into sets keyed on model pointer, rather than position
	std::set<Model*> horiz, vert;
	
	for( ; xmin != xmax;  ++xmin)
		horiz.insert( *xmin);
	
	for( ; ymin != ymax; ++ymin )
		vert.insert( *ymin );
	
	// the intersection of the sets is all the fiducials close by
	std::vector<Model*> nearby;
	std::set_intersection( horiz.begin(), horiz.end(),
												 vert.begin(), vert.end(),
												 std::inserter( nearby, nearby.end() ) ); 
	
	//	printf( "cand sz %lu\n", nearby.size() );
			
	// create sets sorted by x and y position
 	FOR_EACH( it, nearby ) 
 			AddModelIfVisible( *it );	
#else
 	FOR_EACH( it, world->models_with_fiducials )
 			AddModelIfVisible( *it );	

#endif

	// find the range of fiducials within range in X

	Model::Update();
}

void ModelFiducial::Load( void )
{  
	PRINT_DEBUG( "fiducial load" );

	Model::Load();

	// load fiducial-specific properties
	min_range             = wf->ReadLength( wf_entity, "range_min",    min_range );
	max_range_anon        = wf->ReadLength( wf_entity, "range_max",    max_range_anon );
	max_range_id          = wf->ReadLength( wf_entity, "range_max_id", max_range_id );
	fov                   = wf->ReadAngle ( wf_entity, "fov",          fov );
  ignore_zloc            = wf->ReadInt  ( wf_entity, "ignore_zloc",  ignore_zloc);
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
  //PRINT_DEBUG( "fiducial shutdown" );
	fiducials.clear();	
	Model::Shutdown();
}

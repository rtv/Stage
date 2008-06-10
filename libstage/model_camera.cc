///////////////////////////////////////////////////////////////////////////
//
// File: model_camera.cc
// Author: Alex Couture-Beil
// Date: 09 June 2008
//
// CVS info:
//
///////////////////////////////////////////////////////////////////////////


//#define DEBUG 1
#include "stage_internal.hh"

StgModelCamera::StgModelCamera( StgWorld* world, 
		StgModel* parent,
		stg_id_t id,
		char* typestr )
: StgModel( world, parent, id, typestr )
{
	PRINT_DEBUG2( "Constructing StgModelCamera %d (%s)\n", 
			id, typestr );

	// Set up sensible defaults

	this->SetColor( stg_lookup_color( "green" ) );


	stg_geom_t geom;
	memset( &geom, 0, sizeof(geom)); // no size
	geom.size.x = 0.02;
	geom.size.y = 0.02;
	geom.size.z = 0.02;
	this->SetGeom( geom );

	this->Startup();
}

StgModelCamera::~StgModelCamera()
{
}

void StgModelCamera::Load( void )
{
	StgModel::Load();	
	Worldfile* wf = world->GetWorldFile();

}


void StgModelCamera::Update( void )
{   
	StgModel::Update();
	static StgPerspectiveCamera camera;
	camera.update();
	camera.SetProjection( 400, 300, 0.01, 200.0 );
	camera.setPose( parent->GetGlobalPose().x, parent->GetGlobalPose().y, 0.1 );
	camera.setYaw( rtod( parent->GetGlobalPose().a ) - 90.0 );
	//camera.setPose( 0.1, 0.1, 0.1 );
	camera.Draw();
	std::cout << "updated" << std::endl;
	std::cout << parent->GetGlobalPose().x << std::endl;
}


void StgModelCamera::Draw( uint32_t flags, StgCanvas* canvas )
{
	StgModel::Draw( flags, canvas );
}


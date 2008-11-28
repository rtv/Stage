///////////////////////////////////////////////////////////////////////////
//
// File: model_blinkenlight
// Author: Richard Vaughan
// Date: 7 March 2008
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_blinkenlight.cc,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
///////////////////////////////////////////////////////////////////////////

/**
  @ingroup model
  @defgroup model_blinkenlight Blinkenlight model 
  Simulates a blinking light.

API: Stg::StgModelBlinkenlight

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
blinkenlight
(
# generic model properties
size3 [0.02 0.02 0.02]

# type-specific properties
period 250
dutycycle 1.0
enabled 1
)
@endverbatim

@par Notes

@par Details
 
- enabled int 
- if 0, the light is off, else it is on
- period int
- the period of one on/of cycle, in msec
- dutycycle float
- the ratio of on-time to off-time
 */

//#define DEBUG 1
#include "stage_internal.hh"
#include "option.hh"

Option StgModelBlinkenlight::showBlinkenData( "Show Blink", "show_blinken", "", true );

											
StgModelBlinkenlight::StgModelBlinkenlight( StgWorld* world,
														  StgModel* parent ) 
  : StgModel( world, parent, MODEL_TYPE_BLINKENLIGHT ),
	 dutycycle( 1.0 ),
	 enabled( true ),
	 period( 1000 ),
	 on( true )
{
	PRINT_DEBUG2( "Constructing StgModelBlinkenlight %d (%s)\n", 
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
	
	registerOption( &showBlinkenData );
}

StgModelBlinkenlight::~StgModelBlinkenlight()
{
}

void StgModelBlinkenlight::Load( void )
{
  StgModel::Load();
  
  this->dutycycle = wf->ReadFloat( wf_entity, "dutycycle", this->dutycycle );
  this->period = wf->ReadInt( wf_entity, "period", this->period );
  this->enabled = wf->ReadInt( wf_entity, "dutycycle", this->enabled );
}


void StgModelBlinkenlight::Update( void )
{     
	StgModel::Update();

	// invert
	this->on = ! this->on;
}


void StgModelBlinkenlight::DataVisualize( Camera* cam )
{
  // TODO XX
	if( on && showBlinkenData )
	{
		//LISTMETHOD( this->blocks, StgBlock*, Draw );
	}
}


///////////////////////////////////////////////////////////////////////////
//
// File: model_blinkenlight
// Author: Richard Vaughan
// Date: 7 March 2008
//
// CVS info:
//  $Source:
//  /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_blinkenlight.cc,v
//  $
//  $Author: rtv $
//  $Revision$
//
///////////////////////////////////////////////////////////////////////////

/**
  @ingroup model
  @defgroup model_blinkenlight Blinkenlight model
  Simulates a blinking light.

API: Stg::ModelBlinkenlight

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
#include "option.hh"
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

// TODO make instance attempt to register an option (as customvisualizations do)
Option ModelBlinkenlight::showBlinkenData("Show Blink", "show_blinken", "", true, NULL);

ModelBlinkenlight::ModelBlinkenlight(World *world, Model *parent, const std::string &type)
    : Model(world, parent, type), dutycycle(1.0), enabled(true), period(1000), on(true)
{
  PRINT_DEBUG2("Constructing ModelBlinkenlight %u (%s)\n", id, type.c_str());

  // Set up sensible defaults

  this->SetColor(Color("green"));

  // Geom geom;
  // memset( &geom, 0, sizeof(geom)); // no size
  // geom.size.x = 0.02;
  // geom.size.y = 0.02;
  // geom.size.z = 0.02;
  this->SetGeom(Geom(Pose(), Size(0.02, 0.02, 0.02)));

  this->Startup();

  RegisterOption(&showBlinkenData);
}

ModelBlinkenlight::~ModelBlinkenlight()
{
}

void ModelBlinkenlight::Load(void)
{
  Model::Load();

  this->dutycycle = wf->ReadFloat(wf_entity, "dutycycle", this->dutycycle);
  this->period = wf->ReadInt(wf_entity, "period", this->period);
  this->enabled = wf->ReadInt(wf_entity, "dutycycle", this->enabled);
}

void ModelBlinkenlight::Update(void)
{
  // invert
  this->on = !this->on;
  Model::Update();
}

void ModelBlinkenlight::DataVisualize(Camera *cam)
{
  (void)cam; // avoid warning about unused var

  // TODO XX
  if (on && showBlinkenData) {
    // LISTMETHOD( this->blocks, Block*, Draw );
  }
}

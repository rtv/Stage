#include "stage.hh"

using namespace Stg;

ModelLightIndicator::ModelLightIndicator(World* world, Model* parent)
	: Model(world, parent, MODEL_TYPE_LIGHTINDICATOR)
	, m_IsOn(false)
{
}


ModelLightIndicator::~ModelLightIndicator()
{
}


void ModelLightIndicator::SetState(bool isOn)
{
	m_IsOn = isOn;
}


void ModelLightIndicator::DrawBlocks()
{
	if(m_IsOn)
	{
		Model::DrawBlocks();
	}
	else
	  {
		const double scaleFactor = 0.8;
		
		Color keep = this->GetColor();
		Color c = this->GetColor();
		c.r *= scaleFactor;
		c.g *= scaleFactor;
		c.b *= scaleFactor;
		
		this->SetColor( c );
		Model::DrawBlocks();
		
		this->SetColor( keep );
	  }
}

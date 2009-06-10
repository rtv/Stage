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
		stg_color_t currColour = this->GetColor();

		const double scaleFactor = 0.8;
		double r = 0, g = 0, b = 0, a = 0;
		stg_color_unpack(currColour, &r, &g, &b, &a);
		r *= scaleFactor;
		g *= scaleFactor;
		b *= scaleFactor;

		this->SetColor(stg_color_pack(r, g, b, a));
		Model::DrawBlocks();

		this->SetColor(currColour);
	}
}

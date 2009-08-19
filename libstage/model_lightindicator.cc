#include "stage.hh"

using namespace Stg;

ModelLightIndicator::ModelLightIndicator( World* world, 
														Model* parent,
														const std::string& type ) : 
  Model( world, parent, type ),
  m_IsOn(false)
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

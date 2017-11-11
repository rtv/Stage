#include "image.hh"
#include <cassert>
#include <stdlib.h>

namespace Stg{

void Image::Initialize()
{

}

Image::Image()
{
	width = 0;
	height = 0;
	bpp = 0;
}

int Image::getWidth() const
{
	return width;
}

int Image::getHeight() const
{
	return height;
}

int Image::getDepth() const
{
	return bpp;
}

uint8_t * Image::getData()
{
	if(data.empty())
		return NULL;
	return &data[0];
}

bool Image::load(const char * path)
{
	assert(false);
	return false;
}

}

/*
 * image.hh
 *
 *  Created on: Nov 10, 2017
 *      Author: vrobot
 */

#ifndef LIBSTAGE_IMAGE_HH_
#define LIBSTAGE_IMAGE_HH_

#include <stdint.h>
#include <vector>

namespace Stg
{

/** Wraps process of loading image from the file */
class Image
{
public:
	Image();
	bool load(const char * path);
	int getWidth() const;
	int getHeight() const;
	int getDepth() const;
	uint8_t * getData();
	/** Initializes image loading library */
	static void Initialize();
protected:
	int width;
	int height;
	/** Bytes per pixel */
	int bpp;
	std::vector<uint8_t> data;
};

}

#endif /* LIBSTAGE_IMAGE_HH_ */

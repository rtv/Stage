/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: The internal map.
 * Author: Boyoon Jung
 * Date: 22 Nov 2002
 */

#include "regularmcldevice.hh"

#include <iostream>
#include <fstream>
#include <cmath>

using std::ifstream;

#if HAVE_LIBZ
#include <zlib.h>
#endif


// macro
#define INDEX(i,j) ((this->height-1-j)*this->width + i)



// constructor
WorldModel::WorldModel(const char* filename, uint16_t ppm, uint16_t max_range, uint8_t threshold)
{
    // initialize variables
    this->max_range = max_range;
    this->threshold = threshold;
    this->map = 0;

    // load the occupancy grid map if its name is given
    if (filename) this->readMap(filename, ppm);
}


// destructor
WorldModel::~WorldModel(void)
{
    // de-allocate memory
    if (this->map) delete[] this->map;
}


// Load the occupancy grid map data from a file
bool WorldModel::readMap(const char* filename, uint16_t ppm)
{
    bool result = true;

    // determine the file format
    int len = strlen(filename);
    if (! strcmp(&(filename[len-3]), ".gz"))		// compressed
	result =  this->readFromCompressedPGM(filename);
    else						// plain PGM
	result =  this->readFromPGM(filename);

    // compute other constants
    if (result) {
	this->ppm = ppm;
	this->grid_size = 1000.0 / this->ppm;
	this->loaded = true;
    }
    else {
	this->loaded = false;
    }
    return result;
}


// Load the occupancy grid map data from an image file
bool WorldModel::readFromPGM(const char* filename)
{
    char magicNumber[10];
    int whiteNum;

    // open the image file
    ifstream source(filename);
    if (! source) {
	PLAYER_WARN1("cannot open the map file (%s)", filename);
	return false;
    }

    // read the magic number, which should be "P5"
    source >> magicNumber;
    if (strcmp(magicNumber, "P5" ) != 0)
    {
        PLAYER_WARN("image file is of incorrect type:"
                    " should be pnm, binary, monochrome (magic number P5)");
        return false; 
    }
  
    // ignore the end of this line and the following comment-line
    source.ignore( 1024, '\n' );
    source.ignore( 1024, '\n' );
  
    // get the width, height and white value  
    source >> this->width >> this->height >> whiteNum;

    // skip to the end of the line again
    source.ignore( 1024, '\n' );

    // make space for the pixels
    unsigned int numPixels = this->width * this->height;
    if (this->map) delete[] this->map;
    this->map = new uint8_t[numPixels];
  
    // load map data
    source.read( (char*)this->map, numPixels );

    // check that we read the right amount of data
    assert( (unsigned int)(source.gcount()) == numPixels );

    // close the map file
    source.close();

    // done
    return true;
}


// Load the occupancy grid map data from a compressed image file
bool WorldModel::readFromCompressedPGM(const char* filename)
{
#ifndef HAVE_LIBZ

    PRINT_WARN("gziped file is not supported.");
    return false;

#else

    char line[1024];
    char magicNumber[10];
    char comment[256];
    int whiteNum; 

    // open the image file
    gzFile file = gzopen(filename, "r");
    if (file == NULL) {
        PLAYER_WARN1("unable to open image file (%s)", filename);
        return false;
    }

    // Read and check the magic number
    gzgets(file, magicNumber, sizeof(magicNumber));
    if (strcmp(magicNumber, "P5\n") != 0) {
        PLAYER_WARN("image file is of incorrect type:"
                    " should be pnm, binary, monochrome (magic number P5)");
        return false;
    }
  
    // ignore the following comment-line
    gzgets(file, comment, sizeof(comment));

    // get the width and height
    gzgets(file, line, sizeof(line));
    sscanf(line, "%d %d", &(this->width), &(this->height) );

    // ignore the white value
    gzgets(file, line, sizeof(line));
    sscanf(line, "%d", &whiteNum);

    // make space for the pixels
    if (this->map) delete[] this->map;
    this->map = new uint8_t[this->width * this->height];
  
    // load the map data
    for (unsigned int n=0; n<this->width*this->height; n++) {
	this->map[n] = gzgetc(file);
    }

    // close the map file
    gzclose(file);

    // done
    return true;

#endif
}


// compute the distance to the closest wall from (x,y,a)
uint16_t WorldModel::estimateRange(pose_t from)
{
    // constant
    const static float determinent = 0.70710678118654746;	//  1.0/sqrt(2)

    // initialize the distance
    float d = 0.0;

    // initialize indices
    int i = int(from.x / this->grid_size + 0.5);
    int j = int(from.y / this->grid_size + 0.5);

    // invalid data check
    if (i<0 || i>=int(this->width) || j<0 || j>=int(this->height))
	return 0;

    // trigonometry
    float rheading = from.a / 180.0 * M_PI;
    float sin_rheading = (float) sin(rheading);
    float cos_rheading = (float) cos(rheading);
    float tan_rheading = (float) tan(rheading);

    // decide which axis it searches along
    if (cos_rheading >= determinent)			// move along +x axis
    {
	float y = from.y;
	float delta_y = this->grid_size * tan_rheading;
	float unit_distance = fabs(this->grid_size / cos_rheading);

	// how long should it move along
	int c_width = int(this->max_range * cos_rheading / this->grid_size);

	// compute the distance
	for (int x=i; x<=i+c_width; x++)
	{
	    // check boundaries
	    if (x<0 || x>=int(this->width) || j<0 || j>=int(this->height))
		break;
	    // collision check
	    if (this->map[INDEX(x,j)] < this->threshold)
		break;
	    else
		d += unit_distance;
	    // the next index to check
	    y += delta_y;
	    j = (int)(y / this->grid_size);
	}
    }
    else if (sin_rheading >= determinent)		// move along +y axis
    {
	float x = from.x;
	float delta_x = this->grid_size / tan_rheading;
	float unit_distance = fabs(this->grid_size / sin_rheading);

	// how long should it move along
	int c_height = int(this->max_range * sin_rheading / this->grid_size);

	// compute the distance
	for (int y=j; y<=j+c_height; y++)
	{
	    // check boundaries
	    if (i<0 || i>=int(this->width) || y<0 || y>=int(this->height))
		break;
	    // collision check
	    if (this->map[INDEX(i,y)] < this->threshold)
		break;
	    else
		d += unit_distance;
	    // the next index to check
	    x += delta_x;
	    i = (int)(x / this->grid_size);
	}
    }
    else if (cos_rheading <= -determinent)		// move along -x axis
    {
	float y = from.y;
	float delta_y = -this->grid_size * tan_rheading;
	float unit_distance = fabs(this->grid_size / cos_rheading);

	// how long should it move along
	int c_width = int(this->max_range * cos_rheading / this->grid_size);

	// compute the distance
	for (int x=i; x>=i+c_width; x--)
	{
	    // check boundaries
	    if (x<0 || x>=int(this->width) || j<0 || j>=int(this->height))
		break;
	    // collision check
	    if (this->map[INDEX(x,j)] < this->threshold)
		break;
	    else
		d += unit_distance;
	    // the next index to check
	    y += delta_y;
	    j = (int)(y / this->grid_size);
	}
    }
    else						// move along -y axis
    {
	float x = from.x;
	float delta_x = -this->grid_size / tan_rheading;
	float unit_distance = fabs(this->grid_size / sin_rheading);

	// how long should it move along
	int c_height = int(this->max_range * sin_rheading / this->grid_size);

	// compute the distance
	for (int y=j; y>=j+c_height; y--)
	{
	    // check boundaries
	    if (i<0 || i>=int(this->width) || y<0 || y>=int(this->height))
		break;
	    // collision check
	    if (this->map[INDEX(i,y)] < this->threshold)
		break;
	    else
		d += unit_distance;
	    // the next index to check
	    x += delta_x;
	    i = (int)(x / this->grid_size);
	}
    }

    return (d > this->max_range) ? this->max_range : int(d + 0.5);
}

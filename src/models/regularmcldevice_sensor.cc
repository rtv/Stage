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
 * Desc: Beam-based sensor mode for MCL device.
 * Author: Boyoon Jung
 * Date: 22 Nov 2002
 */

#include "regularmcldevice.hh"


#include <iostream>
#include <fstream>
#include <cmath>

using std::ofstream;
using std::cerr;
using std::endl;


// constructor
MCLSensorModel::MCLSensorModel(mcl_sensor_t type,
			       int num_ranges, pose_t* poses, uint16_t max_range,
			       float s_hit, float lambda, float o_small,
			       float z_hit, float z_unexp, float z_max, float z_rand,
			       bool precompute)
{
    // store the configurations
    this->type = type;
    this->num_ranges = num_ranges;
    this->poses = new pose_t[num_ranges];
    if (poses)
	for (int i=0; i<num_ranges; i++)
	    this->poses[i] = poses[i];
    else		// default sensor positions
	for (int i=0; i<num_ranges; i++) {
	    this->poses[i].x = 0.0f;
	    this->poses[i].y = 0.0f;
	    this->poses[i].a = i * 360.0f / num_ranges;
	}
    this->max_range = max_range;

    this->s_hit = s_hit;
    this->lambda = lambda;
    this->o_small = o_small;

    this->z_hit = z_hit;
    this->z_unexp = z_unexp;
    this->z_max = z_max;
    this->z_rand = z_rand;

    this->precompute = precompute;

    // build a pre-computed table
    if (this->precompute) this->buildTable();
}


// destructor
MCLSensorModel::~MCLSensorModel(void)
{
    // de-allocate memory
    delete[] this->poses;
    if (table) {
	for (int i=0; i<int(this->max_range/10+0.5); i++)
	    delete[] this->table[i];
	delete[] this->table;
    }
}


// construct a pre-computed table
void MCLSensorModel::buildTable(void)
{
    //cerr << "  building a pre-computed table for sensor model... ";
    // determine the size of a table
    int size = this->max_range / 10 + 1;
    // allocate memory
    this->table = new (double*)[size];
    for (int i=0; i<size; i++)
	this->table[i] = new double[size];
    // fill out the table
    for (int o=0; o<size; o++)
	for (int e=0; e<size; e++)
	    this->table[o][e] = this->iProbability((uint16_t)(o*10), (uint16_t)(e*10));
    //cerr << "done." << endl;
    // dump the table into a file
    //this->dumpTable("sensor_model.dump");
}


// dump the pre-computed table into a file (debugging purpose)
void MCLSensorModel::dumpTable(const char* filename)
{
    ofstream dumpfile(filename);
    // determine the size of a table
    int size = this->max_range / 10 + 1;
    // dump data
    for (int o=0; o<size; o++) {
	for (int e=0; e<size; e++)
	    dumpfile << this->table[o][e] << " ";
	dumpfile << endl;
    }
}


// model measurement noise
double MCLSensorModel::measurementNoise(uint16_t observation, uint16_t estimation)
{
    const static double sqrt2pi = sqrt(2*M_PI);
    // compute the normalizer
    /*
     *  the normalizer eta should be computed as follows:
     *
     *	    double eta = 0.0;
     *	    for (int x=0; x<=this->max_range; x++)
     *		eta += exp(-SQR(x-estimation)/2.0/SQR(this->s_hit)) /
     *		       (sqrt2pi * this->s_hit);
     *	    eta = 1.0 / eta;
     *
     *  however, a constant normalizer (1.0) is used for efficiency. therefore,
     *  the parameter, z_hit, must be set considering this.
     */
    const static double eta = 1.0;
    // compute the measurement noise
    return eta * exp(-SQR(observation-estimation)/2.0/SQR(this->s_hit)) /
	   (sqrt2pi * this->s_hit);
}


// model unexpected objects
double MCLSensorModel::unexpectedObjectError(uint16_t observation)
{
    // compute the normalizer
    double eta = 1.0 / (1 - exp(-this->lambda * this->max_range));
    // compute the unexpected object error
    return eta * this->lambda * exp(-this->lambda * observation);
}


// model sensor failure (e.g. max range readings)
double MCLSensorModel::sensorFailure(uint16_t observation)
{
    const static double inv_o_small = 1.0 / this->o_small;
    const static double threshold = this->max_range - this->o_small;
    if (observation >= threshold)
	return inv_o_small;
    else
	return 0.0;
}


// model random measurements (e.g. crosstalk)
double MCLSensorModel::randomMeasurement(void)
{
    const static double inv_o_max = 1.0 / this->max_range;
    return inv_o_max;
}


// compute the probability : P( observation | estimation )
double MCLSensorModel::iProbability(uint16_t observation, uint16_t estimation)
{
    return this->z_hit   * this->measurementNoise(observation, estimation) +
	   this->z_unexp * this->unexpectedObjectError(observation) +
	   this->z_max   * this->sensorFailure(observation) +
	   this->z_rand  * this->randomMeasurement();
}


// compute the probability : P(o|s,m)
double MCLSensorModel::probability(uint16_t observation[], pose_t state, WorldModel* map)
{
    double p = 1.0;
    double sina = sin(D2R(state.a));
    double cosa = cos(D2R(state.a));
    for (int i=0; i<this->num_ranges; i++) {
	// compute the estimation distance
	pose_t from;
	from.x = state.x + (poses[i].x * cosa - poses[i].y * sina);
	from.y = state.y + (poses[i].x * sina + poses[i].y * cosa);
	from.a = state.a + poses[i].a;
	uint16_t estimation = map->estimateRange(from);
	// compute the probability P(observation|estimation)
	if (this->precompute) {
	    if (observation[i] > this->max_range)	// limit check
		observation[i] = this->max_range;
	    p *= this->table[observation[i]/10][estimation/10];
	}
	else
	    p *= this->iProbability(observation[i], estimation);
    }
    return p;
}



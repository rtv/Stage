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
 * Desc: Odometry-based action model for MCL device.
 * Author: Boyoon Jung
 * Date: 22 Nov 2002
 */

#include "regularmcldevice.hh"


#include <iostream>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>

using std::ofstream;
using std::cerr;
using std::endl;


// constructor
MCLActionModel::MCLActionModel(float a1, float a2, float a3, float a4)
{
    // store the configurations
    this->a1 = a1;
    this->a2 = a2;
    this->a3 = a3;
    this->a4 = a4;
    // initialize a sequence of pseudo-random integers
    timeval tv;
    gettimeofday(&tv, NULL);
    srand((unsigned long)tv.tv_usec);
}


// sampling method
float MCLActionModel::normal(float b)
{
    float sum = 0.0;
    for (int i=0; i<12; i++)
	sum += rand() / (RAND_MAX/2.0f) - 1.0;
    return (b+100)/6.0f * sum;

}


// abs() function that also does range conversion
double MCLActionModel::abs(double angle)
{
    angle -= ((int)(angle/(2*M_PI))) * 2*M_PI;
    if (angle > M_PI) angle -= 2*M_PI;
    else if (angle < -M_PI) angle += 2*M_PI;
    return fabs(angle);
}


// draw a sample from P(s'|s,a)
pose_t MCLActionModel::sample(pose_t state, pose_t from, pose_t to)
{
    double d_rot1 = atan2( to.y-from.y, to.x-from.x ) - D2R(from.a);
    double d_trans = sqrt( SQR(to.x-from.x) + SQR(to.y-from.y) );
    double d_rot2 = D2R(to.a) - D2R(from.a) - d_rot1;

    double abs_d_rot1 = this->abs(d_rot1);
    double abs_d_rot2 = this->abs(d_rot2);

    double nd_rot1 = d_rot1 + this->normal(this->a1*abs_d_rot1 + this->a2*d_trans);
    double nd_trans = d_trans + this->normal(this->a3*d_trans + this->a4*(abs_d_rot1+abs_d_rot2));
    double nd_rot2 = d_rot2 + this->normal(this->a1*abs_d_rot2 + this->a2*d_trans);

    pose_t sample;
    sample.x = state.x + nd_trans * cos(D2R(state.a) + nd_rot1);
    sample.y = state.y + nd_trans * sin(D2R(state.a) + nd_rot1);
    sample.a = R2D(D2R(state.a) + nd_rot1 + nd_rot2);

    return sample;
}

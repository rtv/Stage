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
 * Desc: Clustering algorithm for grouping particles.
 * Author: Boyoon Jung
 * Date: 22 Nov 2002
 * $Id: regularmcldevice_clustering.cc,v 1.5 2002-12-05 04:34:57 rtv Exp $
 */

#include "regularmcldevice.hh"

#include <iostream>
#include <cstdlib>
#include <cmath> 

using std::cerr;
using std::endl;

//#define DEBUG 1

#ifdef DEBUG
static bool m_isnan(const double m[][3]);
static bool m_isinf(const double m[][3]);
static void printMatrix(const double m[][3]);
#endif


// constant variable
const double MCLClustering::C1 = 1.0 / pow(2*M_PI, 3/2.0);


// constructor
MCLClustering::MCLClustering(uint32_t N, uint32_t iteration)
{
    if (N > 0) {
	// allocate a 3xN matrix
	this->N = N;
	this->X = new double[this->N][3];
	this->Z = new double[this->N][PLAYER_LOCALIZATION_MAX_HYPOTHESIS];
	this->likelihood = new double[this->N][PLAYER_LOCALIZATION_MAX_HYPOTHESIS];
    }
    else {
	this->N = 0;		// no table has been created
	this->X = 0;
	this->Z = 0;
	this->likelihood = 0;
    }
    this->iteration = iteration;
}


// destructor
MCLClustering::~MCLClustering(void)
{
    // de-allocate memory
    if (this->X) delete[] this->X;
    if (this->Z) delete[] this->Z;
    if (this->likelihood) delete[] this->likelihood;
}


// compute the determinant of a 3x3 covariance matrix
double MCLClustering::determinant(const double cov[][3])
{
    double det = (cov[0][0] * cov[1][1] * cov[2][2] -
		  cov[0][0] * cov[1][2] * cov[1][2] -
		  cov[0][1] * cov[0][1] * cov[2][2] +
	      2 * cov[0][1] * cov[0][2] * cov[1][2] -
		  cov[0][2] * cov[0][2] * cov[1][1]);
#ifdef DEBUG
    if (det < 0.0) {
	cerr << "the determinant of cov. matrix is not supposed to be negative." << endl;
	printMatrix(cov);
    }
#endif

    // a trick to get around the singular matrix problem
    if (det == 0.0) {
	double pcov[3][3];
	pcov[0][0] = cov[0][0] + 1e-3;
	pcov[0][1] = cov[0][1];
	pcov[0][2] = cov[0][2];
	pcov[1][0] = cov[1][0];
	pcov[1][1] = cov[1][1] + 1e-3;
	pcov[1][2] = cov[1][2];
	pcov[2][0] = cov[2][0];
	pcov[2][1] = cov[2][1];
	pcov[2][2] = cov[2][2] + 1e-3;
	return this->determinant(pcov);
    }
    else
	return det;
}


// compute the inverse matrix of a 3x3 covariance matrix
void MCLClustering::inverse(const double cov[][3], double inv[][3])
{
    double n = - this->determinant(cov);
    inv[0][0] = 	    (-cov[1][1] * cov[2][2] + cov[1][2] * cov[1][2]) / n;
    inv[0][1] = inv[1][0] = ( cov[0][1] * cov[2][2] - cov[0][2] * cov[1][2]) / n;
    inv[0][2] = inv[2][0] = (-cov[0][1] * cov[1][2] + cov[0][2] * cov[1][1]) / n;
    inv[1][1] = 	    (-cov[0][0] * cov[2][2] + cov[0][2] * cov[0][2]) / n;
    inv[1][2] = inv[2][1] = ( cov[0][0] * cov[1][2] - cov[0][1] * cov[0][2]) / n;
    inv[2][2] = 	    (-cov[0][0] * cov[1][1] + cov[0][1] * cov[0][1]) / n;
#ifdef DEBUG
    if (m_isnan(inv)) cerr << "inverse: inv matrix contains NaN." << endl;
    if (m_isinf(inv)) cerr << "inverse: inv matrix contains Inf." << endl;
#endif
}

// compute the (3D) multivariate gaussian
double MCLClustering::gaussian(const double x[], const double mean[], const double cov[][3])
{
    static double inv[3][3];
    static double d[3];

#ifdef DEBUG
    if (m_isnan(cov)) cerr << "gaussian: cov matrix contains NaN." << endl;
    if (m_isinf(cov)) cerr << "gaussian: cov matrix contains Inf." << endl;
#endif

    double C2 = 1.0 / sqrt(fabs(determinant(cov)));
#ifdef DEBUG
    if (isnan(C2)) cerr << "gaussian: C2 is NaN." << endl;
    if (isinf(C2)) cerr << "gaussian: C2 is Inf." << endl;
#endif

    this->inverse(cov, inv);
    d[0] = x[0] - mean[0];
    d[1] = x[1] - mean[1];
    d[2] = x[2] - mean[2];
    double xsx = (d[0] * (d[0]*inv[0][0] + d[1]*inv[1][0] + d[2]*inv[2][0]) +
		  d[1] * (d[0]*inv[0][1] + d[1]*inv[1][1] + d[2]*inv[2][1]) +
		  d[2] * (d[0]*inv[0][2] + d[1]*inv[1][2] + d[2]*inv[2][2]));
#ifdef DEBUG
    if (xsx < 0.0) cerr << "gaussian: (x'*S-1*x) is negative." << endl;
#endif
    double C3 = exp(-0.5 * xsx);
    // exp() is supposed to return 0 when underflow, but gcc return -Inf.
    // The ideal solution is to link with libieee.a, but this library is
    // not available on some system. This is a simple hack to fix it.
    if (isinf(C3)) C3 = 0.0;
#ifdef DEBUG
    if (isnan(C3)) cerr << "gaussian: C3 is NaN." << endl;
    if (isinf(C3)) cerr << "gaussian: C3 is Inf." << endl;
#endif

    double result = C1 * C2 * C3;
#ifdef DEBUG
    if (isnan(result)) cerr << "gaussian: final result is NaN." << endl;
    if (isinf(result)) cerr << "gaussian: final result is Inf." << endl;
#endif
    return result;
}


// estimate model parameters based on the current grouping (E-step)
void MCLClustering::estimate(void)
{
    // update the likelihood
    for (uint32_t i=0; i<this->N; i++)
	for (int m=0; m<PLAYER_LOCALIZATION_MAX_HYPOTHESIS; m++)
	    this->likelihood[i][m] = this->gaussian(this->X[i], this->mean[m], this->cov[m]);

    // update the hidden variables
    for (uint32_t i=0; i<this->N; i++) {
	double sum = 0.0;
	for (int m=0; m<PLAYER_LOCALIZATION_MAX_HYPOTHESIS; m++) {
	    this->Z[i][m] = this->likelihood[i][m] * pi[m];
	    sum += this->Z[i][m];
	}
	if (sum == 0.0)		// uniform distribution
	    for (int m=0; m<PLAYER_LOCALIZATION_MAX_HYPOTHESIS; m++)
		this->Z[i][m] = 1.0 / PLAYER_LOCALIZATION_MAX_HYPOTHESIS;
	else			// normalization
	    for (int m=0; m<PLAYER_LOCALIZATION_MAX_HYPOTHESIS; m++)
		this->Z[i][m] /= sum;
    }
}


// maximize the expected complete log-likelihood (M-step)
void MCLClustering::maximize(void)
{
    static double z;
    static double zx[3];
    static double zc[3][3];
    static double d[3];

    // update the gussian parameters
    for (int m=0; m<PLAYER_LOCALIZATION_MAX_HYPOTHESIS; m++) {
	z = 0.0;
	zx[0] = zx[1] = zx[2] = 0.0;
	zc[0][0] = zc[0][1] = zc[0][2] =
	zc[1][0] = zc[1][1] = zc[1][2] =
	zc[2][0] = zc[2][1] = zc[2][2] = 0.0;

	for (uint32_t i=0; i<this->N; i++) {
	    z += this->Z[i][m];
	    zx[0] += this->Z[i][m] * this->X[i][0];
	    zx[1] += this->Z[i][m] * this->X[i][1];
	    zx[2] += this->Z[i][m] * this->X[i][2];
	}

	if (z > 0.0) {
	    // update the coefficient
	    this->pi[m] = z / this->N;

	    // update the mean
	    this->mean[m][0] = zx[0] / z;
	    this->mean[m][1] = zx[1] / z;
	    this->mean[m][2] = zx[2] / z;

	    // update the covariance matrix
	    for (uint32_t i=0; i<this->N; i++) {
		d[0] = this->X[i][0] - this->mean[m][0];
		d[1] = this->X[i][1] - this->mean[m][1];
		d[2] = this->X[i][2] - this->mean[m][2];

		zc[0][0] += this->Z[i][m] * d[0] * d[0];
		zc[0][1] += this->Z[i][m] * d[0] * d[1];
		zc[0][2] += this->Z[i][m] * d[0] * d[2];
		zc[1][1] += this->Z[i][m] * d[1] * d[1];
		zc[1][2] += this->Z[i][m] * d[1] * d[2];
		zc[2][2] += this->Z[i][m] * d[2] * d[2];
	    }
	    this->cov[m][0][0] = zc[0][0] / z;
	    this->cov[m][0][1] = this->cov[m][1][0] = zc[0][1] / z;
	    this->cov[m][0][2] = this->cov[m][2][0] = zc[0][2] / z;
	    this->cov[m][1][1] = zc[1][1] / z;
	    this->cov[m][1][2] = this->cov[m][2][1] = zc[1][2] / z;
	    this->cov[m][2][2] = zc[2][2] / z;
	}
	else {
	    // re-randomize the parameter
	    this->pi[m] = 0.0;
	    this->mean[m][0] = rand() / this->width_ratio;
	    this->mean[m][1] = rand() / this->height_ratio;
	    this->mean[m][2] = rand() / this->yaw_ratio;
	    this->cov[m][0][0] = rand() / this->cov_ratio;
	    this->cov[m][1][1] = rand() / this->cov_ratio;
	    this->cov[m][2][2] = rand() / this->yaw_ratio;
	    this->cov[m][0][1] = this->cov[m][1][0] = 0.0;
	    this->cov[m][0][2] = this->cov[m][2][0] = 0.0;
	    this->cov[m][1][2] = this->cov[m][2][1] = 0.0;
	}
    }
}


// EM procedure to model the distribution of a given particle set
// using a mixture of Gaussians
void MCLClustering::EM(void)
{
    for (uint32_t i=0; i<this->iteration; i++) {
	// estimation step
	this->estimate();
	// maximization step
	this->maximize();
    }
}


// initialize the parameters
void MCLClustering::Reset(uint32_t width, uint32_t height)
{
    // pre-compute constants for efficiency
    this->width_ratio = double(RAND_MAX) / width;
    this->height_ratio = double(RAND_MAX) / height;
    this->yaw_ratio = double(RAND_MAX) / 360;
    this->cov_ratio = double(RAND_MAX) / 10000;	// 10 meters
    this->coefficient = 1.0 / PLAYER_LOCALIZATION_MAX_HYPOTHESIS;
  
    // initialize gaussian parameters
    for (int m=0; m<PLAYER_LOCALIZATION_MAX_HYPOTHESIS; m++)
    {
	// mean pose is random
	this->mean[m][0] = rand() / this->width_ratio;
	this->mean[m][1] = rand() / this->height_ratio;
	this->mean[m][2] = rand() / this->yaw_ratio;

	// covariance matrix is random
	this->cov[m][0][0] = rand() / this->cov_ratio;
	this->cov[m][1][1] = rand() / this->cov_ratio;
	this->cov[m][2][2] = rand() / this->yaw_ratio;
	this->cov[m][0][1] = this->cov[m][1][0] = 0.0;
	this->cov[m][0][2] = this->cov[m][2][0] = 0.0;
	this->cov[m][1][2] = this->cov[m][2][1] = 0.0;

	// coefficient for linear combination is uniform
	this->pi[m] = this->coefficient;
    }
}


// group the particles using a fixed number of Gaussians
void MCLClustering::cluster(vector<particle_t>& particles)
{
    // check the input size
    if (this->N != particles.size()) {
	if (!this->X) delete[] this->X;
	this->N = particles.size();
	this->X = new double[this->N][3];
	this->Z = new double[this->N][PLAYER_LOCALIZATION_MAX_HYPOTHESIS];
	this->likelihood = new double[this->N][PLAYER_LOCALIZATION_MAX_HYPOTHESIS];
    }

    // construct a matrix of input particles
    vector<particle_t>::iterator p = particles.begin();
    for (int i=0; p<particles.end(); p++, i++) {
	this->X[i][0] = p->pose.x;
	this->X[i][1] = p->pose.y;
	this->X[i][2] = p->pose.a;
    }

    // run EM algorithm to compute the parameters of Gaussians
    this->EM();
}


#ifdef DEBUG
// [for debugging only] check if a matrix contains NaN
static bool m_isnan(const double m[][3])
{
    if (isnan(m[0][0]) || isnan(m[0][1]) || isnan(m[0][2]) ||
	isnan(m[1][0]) || isnan(m[1][1]) || isnan(m[1][2]) ||
	isnan(m[2][0]) || isnan(m[2][1]) || isnan(m[2][2]))
	return true;
    else 
	return false;
}

// [for debugging only] check if a matrix contains +/-Inf
static bool m_isinf(const double m[][3])
{
    if (isinf(m[0][0]) || isinf(m[0][1]) || isinf(m[0][2]) ||
	isinf(m[1][0]) || isinf(m[1][1]) || isinf(m[1][2]) ||
	isinf(m[2][0]) || isinf(m[2][1]) || isinf(m[2][2]))
	return true;
    else 
	return false;
}

// [for debugging only] print a matrix on the screen
static void printMatrix(const double m[][3])
{
    cerr << "[" << endl;
    for (int i=0; i<3; i++) {
	cerr << "\t";
	for (int j=0; j<3; j++)
	    cerr << m[i][j] << "\t";
	cerr << "\n";
    }
    cerr << "]" << endl;
}
#endif

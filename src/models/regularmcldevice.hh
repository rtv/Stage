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
 * Desc: Device to perform the regular MCL (Monte-Carlo Localization).
 * Author: Boyoon Jung
 * Date: 22 Nov 2002
 */

#ifndef REGULARMCLDEVICE_HH
#define REGULARMCLDEVICE_HH

#include "playerdevice.hh"
#include "positiondevice.hh"
#include "sonardevice.hh"
#include "laserdevice.hh"

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <cmath>

using std::vector;
using std::sort;


/*****************************************************************************
 * common data structures
 *****************************************************************************/

// pose
struct pose_t
{
    float x;
    float y;
    float a;
};


// particle
struct particle_t
{
    pose_t pose;	// robot pose on a given occupancy map
    double importance;	// importance factor
    double cumulative;	// cumulative importance factor - for importance sampling
};


/*****************************************************************************
 * constants
 *****************************************************************************/

// valid distance sensors
enum mcl_sensor_t { PLAYER_MCL_SONAR,		// sonar
    		    PLAYER_MCL_LASER,		// laser rangefinder
		    PLAYER_MCL_NOSENSOR };	// no sensor (error)


/*****************************************************************************
 * macros
 *****************************************************************************/

#define SQR(x) ((x)*(x))
#define SQR2(x,y) (((x)-(y)) * ((x)-(y)))
#define D2R(a) ((float)((a)*M_PI/180))
#define R2D(a) ((int)((a)*180/M_PI))


/*****************************************************************************
 * inline functions
 *****************************************************************************/

inline bool operator < (const particle_t& x, const particle_t& y) {
    return x.importance < y.importance;
}

inline bool operator == (const particle_t& x, const particle_t& y) {
    return x.importance == y.importance;
}

inline bool operator == (const pose_t& p, const pose_t& q) {
    return (p.x == q.x && p.y == q.y && p.a == q.a);
}

inline bool operator != (const pose_t& p, const pose_t& q) {
    return (p.x != q.x || p.y != q.y || p.a != q.a);
}

// compute eigen values and eigen vectors of a 2x2 covariance matrix
inline void eigen(double cm[][2], double values[], double vectors[][2])
{
    double s = (double) sqrt(cm[0][0]*cm[0][0] - 2*cm[0][0]*cm[1][1] +
			     cm[1][1]*cm[1][1] + 4*cm[0][1]*cm[0][1]);
    values[0] = 0.5 * (cm[0][0] + cm[1][1] + s);
    values[1] = 0.5 * (cm[0][0] + cm[1][1] - s);
    vectors[0][0] = -0.5 * (-cm[0][0] + cm[1][1] - s);
    vectors[0][1] = -0.5 * (-cm[0][0] + cm[1][1] + s);
    vectors[1][0] = vectors[1][1] = cm[0][1];
}


/*****************************************************************************
 * WorldModel class : the internal map
 *****************************************************************************/

class WorldModel
{
    public:	// supposed to be private, but for efficiency
	// the size of a world
	uint32_t width, height;
	// the number of pixels per meter
	uint16_t ppm;
	// the size of a grid
	float grid_size;
	// occupancy grid map
	uint8_t *map;
	// the maximum range of a distance sensor
	uint16_t max_range;
	// threshold for empty/occupied decision
	uint8_t threshold;
	// is a map loaded ?
	bool loaded;

    protected:
	// Load the occupancy grid map data from a PGM file
	bool readFromPGM(const char* filename);
	// Load the occupancy grid map data from a compressed PGM file
	bool readFromCompressedPGM(const char* filename);

    public:
	// constructor
	WorldModel(const char* filename=0,	// map file
		   uint16_t ppm=0,		// pixels per meter
		   uint16_t max_range=8000,	// maximum range of a distance sensor
		   uint8_t threshold=240);	// threshold for empty/occupied
	// destructor
	~WorldModel(void);

	// Load the occupancy grid map data from a file
	bool readMap(const char* filename, uint16_t ppm);

	// compute the distance to the closest wall from (x,y,a)
	uint16_t estimateRange(pose_t from);

	// access functions
	bool isLoaded(void) { return loaded; }
	int32_t Width(void) { return int(width * grid_size + 0.5); }
	int32_t Height(void) { return int(height * grid_size + 0.5); }
};


/*****************************************************************************
 * MCLSensorModel class : the sensor model
 *****************************************************************************/
/*
 * Beam-based sensor model for Monte-Carlo Localization algorithms. The model
 * compute the probability P(o|s,m) where `o' is an observation, `s' is a pose
 * state, and `m' is an internal map.
*/

class MCLSensorModel
{
    private:
	// type of a distance sensor
	mcl_sensor_t type;
	// the number of ranges
	int num_ranges;
	// the pose of ranges
	pose_t* poses;
	// the maximum ranges
	int max_range;
	// build a pre-computed table for fast computing ?
	bool precompute;
	// pointer to the pre-computed table
	double **table;

	// sensor model parameters for linear combination of error models
	double z_hit, z_unexp, z_max, z_rand;

	// error model parameters
	double s_hit;			// for measurement noise
	double lambda;			// for unexpectedd object error
	double o_small;			// for sensor failures

    protected:
	// construct a pre-computed table
	void buildTable(void);

	// dump the pre-computed table into a file (debugging purpose)
	void dumpTable(const char* filename);

	// model measurement noise
	inline double measurementNoise(uint16_t observation, uint16_t estimation);

	// model unexpected objects
	inline double unexpectedObjectError(uint16_t observation);

	// model sensor failure (e.g. max range readings)
	inline double sensorFailure(uint16_t observation);

	// model random measurements (e.g. crosstalk)
	inline double randomMeasurement(void);

	// compute the probability : P( observation | estimation )
	double iProbability(uint16_t observation, uint16_t estimation);

    public:
	// constructor
	MCLSensorModel(mcl_sensor_t type,	// type of a distance sensor
		       int num_ranges,		// the number of ranges
		       pose_t* poses,		// the poses of ranges
		       uint16_t max_range,	// maximum range of the distance sensor
		       float s_hit,		// error model parameters
		       float lambda,
		       float o_small,
		       float z_hit,		// parameters for linear combination
		       float z_unexp,
		       float z_max,
		       float z_rand,
		       bool precompute=true);	// build a table for fast computing ?
	// destructor
	~MCLSensorModel(void);

	// compute the probability : P(o|s,m)
	double probability(uint16_t observation[],	// observations `o'
			   pose_t state,		// state `s'
			   WorldModel* map);		// map `m'
};


/*****************************************************************************
 * MCLActionModel class : the action model
 *****************************************************************************/
/*
 * Odometry-based action model for Monte-Carlo Localization algorithms. The model
 * draws a sample from the probability P(s'|s,a) where "s" is a pose state, "a"
 * is an action performed, and "s'" is a new pose state.
*/

class MCLActionModel
{
    private:
	// action model parameters
	float a1, a2, a3, a4;

    protected:
	// sampling method
	float normal(float b);

	// abs() function that also does range conversion
	inline double abs(double angle);

    public:
	// constructor
	MCLActionModel(float a1, float a2, float a3, float a4);

	// draw a sample from P(s'|s,a)
	pose_t sample(pose_t state,			// state "s"
		      pose_t from, pose_t to);		// action "a"
};


/*****************************************************************************
 * MCLClustering class : the mcl (Monte-Carlo Localization) class
 *****************************************************************************/
/*
 *   Clustering algorithm that utilize EM algorithm to estimate
 *   particle sets using a Gaussian mixture.
 */

class MCLClustering
{
    public:
	// the parameters of mixure gaussian
	double mean[PLAYER_LOCALIZATION_MAX_HYPOTHESIS][3];
	double cov[PLAYER_LOCALIZATION_MAX_HYPOTHESIS][3][3];
	double pi[PLAYER_LOCALIZATION_MAX_HYPOTHESIS];

    private:
	// input size (the number of particles)
	uint32_t N;
	// the number of iteration for each step
	uint32_t iteration;

	// input matrix
	double (*X)[3];
	// hidden variables
	double (*Z)[PLAYER_LOCALIZATION_MAX_HYPOTHESIS];
	// likelihood
	double (*likelihood)[PLAYER_LOCALIZATION_MAX_HYPOTHESIS];

	// constant variable
	const static double C1;
	double width_ratio;
	double height_ratio;
	double yaw_ratio;
	double cov_ratio;
	double coefficient;

    protected:
	// compute the determinant of a 3x3 covariance matrix
	inline double determinant(const double cov[][3]);

	// compute the inverse matrix of a 3x3 covariance matrix
	inline void inverse(const double cov[][3], double inv[][3]);

	// compute the (3D) multivariate gaussian 
	inline double gaussian(const double x[], const double mean[], const double cov[][3]);

	// estimate model parameters based on the current grouping (E-step)
	void estimate(void);

	// maximize the expected complete log-likelihood (M-step)
	void maximize(void);

	// EM procedure to model the distribution of a given particle set
	// using a mixture of Gaussians
	void EM(void);

    public:
	// constructor
	MCLClustering(uint32_t N=0, uint32_t iteration=10);
	// destructor
	~MCLClustering(void);

	// initialize the parameters
	void Reset(uint32_t width, uint32_t height);

	// group the particles using a fixed number of Gaussians
	void cluster(vector<particle_t>& particles);
};


/*****************************************************************************
 * CRegularMCLDevice class : the mcl (Monte-Carlo Localization) class
 *****************************************************************************/
/*
 *  This device implements the regular MCL only. Other extensions
 *  (i.g. mixture MCL, adaptive MCL) will be implemented in separate MCL
 *  devices.
 */

class CRegularMCLDevice : public CPlayerEntity 
{
    private:
	// configuration : update speed
	float frequency;			// "frequency <float>"

	// configuration : the number of particles
	unsigned int num_particles;		// "num_particles <int>"

	// configuration : distance sensor
	mcl_sensor_t sensor_type;		// "sensor_type <string>"
	int sensor_index;			// "sensor_index <int>"
	uint16_t sensor_max;			// "sensor_max <int>"
	uint16_t sensor_num_samples;		// "sensor_num_samples <int>"

	// configuration : motion sensor
	int motion_index;			// "motion_index <int>"

	// configuration : world model (map)
	const char *map_file;			// "map <string>"
	uint16_t map_ppm;			// "map_ppm <int>"
	uint8_t map_threshold;			// "map_threshold <int>"

	// configuration : sensor model
	float sm_s_hit;				// "sm_s_hit" <float>
	float sm_lambda;			// "sm_lambda" <float>
	float sm_o_small;			// "sm_o_small" <float>
	float sm_z_hit;				// "sm_z_hit" <float>
	float sm_z_unexp;			// "sm_z_unexp" <float>
	float sm_z_max;				// "sm_z_max" <float>
	float sm_z_rand;			// "sm_z_rand" <float>
	int sm_precompute;			// "sm_precompute" <int>

	// configuration : action model
	float am_a1;				// "am_a1" <float>
	float am_a2;				// "am_a2" <float>
	float am_a3;				// "am_a3" <float>
	float am_a4;				// "am_a4" <float>

    protected:
	// Pointer to sensors to get sensor data from
	CPlayerEntity *distance_device;		// distance sensor
	CPlayerEntity *motion_device;		// motion sensor

	// Pointer to models
	WorldModel *map;			// world model (map)
	MCLSensorModel *sensor_model;		// distance sensor model
	MCLActionModel *action_model;		// motion sensor model

	// Pointer to a clustering algorithm
	MCLClustering *clustering;

	// Particle set
	double unit_importance;			// uniform importance
	vector<particle_t> particles;		// a particle set
	vector<particle_t> p_buffer;		// a temporary buffer

	// Hypothesis set
	player_localization_data_t hypothesis;	// hypothesis storage

	// Distance sensor data
	int num_ranges;				// the number of ranges
	int inc;				// for subsampling
	pose_t *poses;				// poses of distance sensors
	uint16_t *ranges;			// range data buffer

	// Motion sensor data
	pose_t p_odometry;			// previous odometry reading

	// for drawing
	StageColor particle_color;
	StageColor hypothesis_color;

	// this one keeps track of whether or not we've already subscribed to
	// the underlying laser device
	bool m_distance_subscribedp;		// distance sensor
	bool m_motion_subscribedp;		// motion sensor


    protected:
	// Process configuration requests. Returns 1 if the configuration has changed.
	void UpdateConfig(void);

	// Reset MCL device
	void Reset(void);

	// Read the configuration of a distance sensor
	void ReadConfiguration(CWorldFile* worldfile);

	// Read range data from a distance senosr
	bool ReadRanges(uint16_t* buffer);

	// Read odometry data from a motion sensor
	bool ReadOdometry(pose_t& pose);

	//////////////////////////////////////////////////////////////////////
	// Monte-Carlo Localization Algorithm
	//
	// [step 1] : draw new samples from the previous PDF
	void SamplingImportanceResampling(pose_t from, pose_t to);

	// [step 2] : update importance factors based on the sensor model
	void ImportanceFactorUpdate(uint16_t *ranges);

	// [step 3] : construct hypothesis by grouping particles
	void HypothesisConstruction(player_localization_data_t& data);
	//
	//////////////////////////////////////////////////////////////////////

    public:
	// Default constructor
	CRegularMCLDevice(LibraryItem *libit, CWorld *world, CEntity *parent);

	// a static named constructor - a pointer to this function is given
        // to the Library object and paired with a string.  When the string
        // is seen in the worldfile, this function is called to create an
        // instance of this entity
	static CRegularMCLDevice* Creator(LibraryItem *libit,
					  CWorld *world, CEntity *parent ) {
	    return  new CRegularMCLDevice(libit, world, parent);
        }

	// Initialize entity
	virtual bool Startup(void);

	// Finalize entity
	virtual void Shutdown(void);

	// Load the entity from the worldfile
	virtual bool Load(CWorldFile *worldfile, int section);

	// Update the device
	virtual void Update(double sim_time);

#ifdef INCLUDE_RTK2

    private:
    	// for drawing the particles
	rtk_fig_t *particle_fig;

	// for drawing the hypothesis
	rtk_fig_t *hypothesis_fig;

    protected:
        // Initialize the RTK GUI
	virtual void RtkStartup(void);

	// Finalize the RTK GUI
	virtual void RtkShutdown(void);

	// Update the RTK GUI
	virtual void RtkUpdate(void);

#endif

};


#endif

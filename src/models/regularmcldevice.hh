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
 * $Id: regularmcldevice.hh,v 1.7 2002-12-10 02:35:40 boyoon Exp $
 */

#ifndef REGULARMCLDEVICE_HH
#define REGULARMCLDEVICE_HH

/*****************************************************************************
 * This Stage model is not an actual device model. All this model does is
 * to reveive MCL data, which are computed in Player side, and visualize them
 * on the Stage window.
 *****************************************************************************/

#include "playerdevice.hh"
#include "world.hh"
#include "library.hh"

#include <stdint.h>
#include <cmath> 


/*****************************************************************************
 * constants
 *****************************************************************************/

#define MCL_MAX_PARTICLES 100000		// packet size must be fixed

#define MCL_CMD_NONE	0			// there is no command
#define MCL_CMD_CONFIG	1			// command 'send configuration'
#define MCL_CMD_UPDATE	2			// command 'update'
#define MCL_CMD_STOP	3			// command 'stop update'


/*****************************************************************************
 * common data structures
 *****************************************************************************/

// pose
typedef struct pose
{
    float x;			// x position
    float y;			// y position
    float a;			// heading
} __attribute__ ((packed)) pose_t;


// particle
typedef struct particle
{
    pose_t pose;		// robot pose on a given occupancy map
    double importance;		// importance factor
    double cumulative;		// cumulative importance factor - for importance sampling
} __attribute__ ((packed)) particle_t;


// valid distance sensors
enum mcl_sensor_t { PLAYER_MCL_SONAR,		// sonar
    		    PLAYER_MCL_LASER,		// laser rangefinder
		    PLAYER_MCL_NOSENSOR };	// no sensor (error)


// configuration
typedef struct mcl_config
{
    float frequency;			// update speed
    unsigned int num_particles;		// the number of particles

    mcl_sensor_t sensor_type;		// distance sensor
    int sensor_index;			// distance sensor index
    uint16_t sensor_max;		// the maximum range of the sensor
    uint16_t sensor_num_ranges;		// the number of ranges of the snesor

    int motion_index;			// motion sensor index

    char map_file[PATH_MAX];		// the path of the map file
    uint16_t map_ppm;			// pixels per meter
    uint8_t map_threshold;		// empty threshold

    float sm_s_hit;			// sensor model parameter: sm_s_hit
    float sm_lambda;			// sensor model parameter: sm_lambda
    float sm_o_small;			// sensor model parameter: sm_o_small
    float sm_z_hit;			// sensor model parameter: sm_z_hit
    float sm_z_unexp;			// sensor model parameter: sm_z_unexp
    float sm_z_max;			// sensor model parameter: sm_z_max
    float sm_z_rand;			// sensor model parameter: sm_z_rand
    int sm_precompute;			// pre-compute sensor models ?

    float am_a1;			// action model parameter: am_a1
    float am_a2;			// action model parameter: am_a2
    float am_a3;			// action model parameter: am_a3
    float am_a4;			// action model parameter: am_a4
} __attribute__ ((packed)) mcl_config_t;


// hypothesis type for internal use
typedef struct mcl_hypothesis
{
    double mean[3];			// mean of a hypothesis
    double cov[3][3];			// covariance matrix
    double alpha;			// coefficient for linear combination
} __attribute__ ((packed)) mcl_hypothesis_t;


// data coming from a Player device
typedef struct mcl_data
{
    int command;			// is a request message ?
    uint32_t num_hypothesis;
    mcl_hypothesis_t hypothesis[PLAYER_LOCALIZATION_MAX_HYPOTHESIS];
    uint32_t num_particles;
    particle_t particles[MCL_MAX_PARTICLES];
} __attribute__ ((packed)) mcl_data_t;


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
    protected:

        // configuration
	mcl_config_t config;

	// data
	mcl_data_t data;

	// subscribed?
	bool subscribe_flag;

	// start update?
	bool update_flag;

	// for drawing
	StageColor particle_color;
	StageColor hypothesis_color;

	// compute eigen values and eigen vectors of a 2x2 covariance matrix
	inline void eigen(double cm[][3], double values[], double vectors[][2]);

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

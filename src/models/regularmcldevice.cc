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
 * Desc: the regular MCL (Monte-Carlo Localization) device.
 * Author: Boyoon Jung
 * Date: 22 Nov 2002
 * $Id: regularmcldevice.cc,v 1.7 2003-02-08 07:54:42 inspectorg Exp $
 */

#include "regularmcldevice.hh"

#include <cmath> 
#include <cstring>
#include <iostream>

using std::cerr;
using std::endl;


// constructor
CRegularMCLDevice::CRegularMCLDevice(LibraryItem* libit, CWorld *world, CEntity *parent)
    : CPlayerEntity(libit, world, parent)
{
    // set the Player IO sizes correctly for this type of Entity
    m_data_len = sizeof(mcl_config_t);
    m_command_len = sizeof(mcl_data_t);
    m_config_len = 0;
    m_reply_len = 0;

    // set the device code
    m_player.code = PLAYER_LOCALIZE_CODE;	// from player's <player.h>

    // This is not a real object, so by default we dont see it
    this->obstacle_return = false;
    this->sonar_return = false;
    this->puck_return = false;
    this->vision_return = false;
    this->idar_return = IDARTransparent;
    this->laser_return = LaserTransparent;

    // retrieve colors
    this->particle_color = ::LookupColor("cyan");
    this->hypothesis_color = ::LookupColor("blue");

    // no update
    this->subscribe_flag = false;
    this->update_flag = false;
    this->data.command = MCL_CMD_NONE;
    m_last_update = 0;

#ifdef INCLUDE_RTK2
    this->particle_fig = NULL;
    this->hypothesis_fig = NULL;
#endif
}


// compute eigen values and eigen vectors of a 2x2 covariance matrix
void CRegularMCLDevice::eigen(double cm[][3], double values[], double vectors[][2])
{
    double s = (double) sqrt(cm[0][0]*cm[0][0] - 2*cm[0][0]*cm[1][1] +
			     cm[1][1]*cm[1][1] + 4*cm[0][1]*cm[0][1]);
    values[0] = 0.5 * (cm[0][0] + cm[1][1] + s);
    values[1] = 0.5 * (cm[0][0] + cm[1][1] - s);
    vectors[0][0] = -0.5 * (-cm[0][0] + cm[1][1] - s);
    vectors[0][1] = -0.5 * (-cm[0][0] + cm[1][1] + s);
    vectors[1][0] = vectors[1][1] = cm[0][1];
}


// Initialize entity
bool CRegularMCLDevice::Startup(void)
{
    if (!CPlayerEntity::Startup())
	return false;

    // set update speed
    m_interval = 1.0 / this->config.frequency;

    return 0;
}


// Load the entity from the worldfile
bool CRegularMCLDevice::Load(CWorldFile *worldfile, int section)
{
    if (!CPlayerEntity::Load(worldfile, section))
	return false;

    // update speed
    this->config.frequency = worldfile->ReadFloat(section, "frequency", 1.0);

    // the number of particles
    this->config.num_particles = worldfile->ReadInt(section, "num_particles", 5000);

    // a distance sensor
    const char *sensor_str = worldfile->ReadString(section, "sensor_type", "sonar");
    if (strcmp(sensor_str, "sonar") == 0)
	this->config.sensor_type = PLAYER_MCL_SONAR;
    else if (strcmp(sensor_str, "laser") == 0)
	this->config.sensor_type = PLAYER_MCL_LASER;
    else
	this->config.sensor_type = PLAYER_MCL_NOSENSOR;
    this->config.sensor_index = worldfile->ReadInt(section, "sensor_index", 0);
    this->config.sensor_max = worldfile->ReadInt(section, "sensor_max", 5000);
    this->config.sensor_num_ranges = worldfile->ReadInt(section, "sensor_num_ranges", 0);

    // a motion sensor
    this->config.motion_index = worldfile->ReadInt(section, "motion_index", 0);

    // a world model (map)
    strcpy(config.map_file, worldfile->ReadFilename(section, "map", ""));
    this->config.map_ppm = worldfile->ReadInt(section, "map_ppm", 10);
    this->config.map_threshold = worldfile->ReadInt(section, "map_threshold", 240);

    // a sensor model
    this->config.sm_s_hit = worldfile->ReadFloat(section, "sm_s_hit", 300.0);
    this->config.sm_lambda = worldfile->ReadFloat(section, "sm_lambda", 0.001);
    this->config.sm_o_small = worldfile->ReadFloat(section, "sm_o_small", 100.0);
    this->config.sm_z_hit = worldfile->ReadFloat(section, "sm_z_hit", 50.0);
    this->config.sm_z_unexp = worldfile->ReadFloat(section, "sm_z_unexp", 30.0);
    this->config.sm_z_max = worldfile->ReadFloat(section, "sm_z_max", 5.0);
    this->config.sm_z_rand = worldfile->ReadFloat(section, "sm_z_rand", 200.0);
    this->config.sm_precompute = worldfile->ReadInt(section, "sm_precompute", 1);

    // load configuration : an action model
    this->config.am_a1 = worldfile->ReadFloat(section, "am_a1", 0.01);
    this->config.am_a2 = worldfile->ReadFloat(section, "am_a2", 0.0002);
    this->config.am_a3 = worldfile->ReadFloat(section, "am_a3", 0.03);
    this->config.am_a4 = worldfile->ReadFloat(section, "am_a4", 0.1);

    return true;
}


// main update function
void CRegularMCLDevice::Update(double sim_time)
{
    // slow down the update speed
    if (sim_time - m_last_update < 0.1)		// 10 Hz
	return;
    else
	m_last_update = sim_time;

    // retrieve the data from Player device
    if (GetCommand((void*)&this->data, sizeof(mcl_data_t)) > 0)
    {
	// mark the current data has been processed
	int command = this->data.command;
	this->data.command = MCL_CMD_NONE;
	PutCommand((void*)&this->data, sizeof(this->data.command));

	switch (command)
	{
	    case MCL_CMD_NONE:
		// there is no new data; do nothing
		this->update_flag = false;
		break;

	    case MCL_CMD_CONFIG:
		// send the configuration to Player device.
		PutData((void*)&this->config, sizeof(mcl_config_t));
		break;

	    case MCL_CMD_UPDATE:
		// keep updating
		this->subscribe_flag = true;
		this->update_flag = true;
		break;

	    case MCL_CMD_STOP:
		// stop updating
		this->subscribe_flag = false;
		this->update_flag = false;
		break;

	    default:
		// error ?
		PLAYER_WARN1("invalid MCL command (%d)\n", this->data.command);
		this->update_flag = false;
		break;
	}
    }
}


#ifdef INCLUDE_RTK2

// Initialize the Rtk GUI
void CRegularMCLDevice::RtkStartup(void)
{
    CPlayerEntity::RtkStartup();

    // Create a figure representing particles
    this->particle_fig = rtk_fig_create(m_world->canvas, NULL, 51);
    rtk_fig_color_rgb32(this->particle_fig, this->particle_color);

    // Create a figure representing hypothesis
    this->hypothesis_fig = rtk_fig_create(m_world->canvas, NULL, 99);
    rtk_fig_color_rgb32(this->hypothesis_fig, this->hypothesis_color);
}


// Finalize the Rtk GUI
void CRegularMCLDevice::RtkShutdown(void)
{
    // Clean up the figure we created
    rtk_fig_destroy(this->particle_fig);
    rtk_fig_destroy(this->hypothesis_fig);

    CPlayerEntity::RtkShutdown();
}


// Update the rtk gui
void CRegularMCLDevice::RtkUpdate()
{
    static double eval[2], evec[2][2];

    CPlayerEntity::RtkUpdate();

    // if a client is not subscribed to this device
    if (! this->subscribe_flag)
    {
	// clear the canvas
	rtk_fig_clear(this->particle_fig);
	rtk_fig_clear(this->hypothesis_fig);
    }
    // if a client is subscribed to this device
    else if (this->update_flag)
    {
	// clear the canvas
	rtk_fig_clear(this->particle_fig);
	rtk_fig_clear(this->hypothesis_fig);
  
	// draw particles
	if (this->data.num_particles > 0) {
	    rtk_fig_color_rgb32(this->particle_fig, this->particle_color);
            for (uint32_t i=0; i<this->data.num_particles; i++)
        	rtk_fig_rectangle(this->particle_fig,
			this->data.particles[i].pose.x / 1000.0,
			this->data.particles[i].pose.y / 1000.0,
			D2R(this->data.particles[i].pose.a),
			0.1, 0.1, TRUE);
	}
    
        // draw hypothesis
	if (this->data.num_hypothesis > 0) {
	    rtk_fig_color_rgb32(this->hypothesis_fig, this->hypothesis_color);
	    for (uint32_t i=0; i<this->data.num_hypothesis; i++)
	    {
		double ox = this->data.hypothesis[i].mean[0] / 1000.0;
		double oy = this->data.hypothesis[i].mean[1] / 1000.0;

		eigen(this->data.hypothesis[i].cov, eval, evec);

		double oa = atan(evec[1][0] / evec[0][0]);
		double sx = 3 * sqrt(fabs(eval[0])) / 1000.0;
		double sy = 3 * sqrt(fabs(eval[1])) / 1000.0;

		rtk_fig_line_ex(this->hypothesis_fig, ox, oy, oa, 1.0);
		rtk_fig_line_ex(this->hypothesis_fig, ox, oy, oa+M_PI_2, 1.0);
		rtk_fig_ellipse(this->hypothesis_fig, ox, oy, oa, sx, sy, FALSE);
	    }
	}
    }
}


#endif

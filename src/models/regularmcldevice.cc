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
 */

#include "regularmcldevice.hh"

#include <cmath>
#include <iostream>
using std::cerr;
using std::endl;


// constructor
CRegularMCLDevice::CRegularMCLDevice(LibraryItem* libit, CWorld *world, CEntity *parent)
    : CPlayerEntity(libit, world, parent)
{
    // set the Player IO sizes correctly for this type of Entity
    m_data_len = sizeof(player_localization_data_t);
    m_command_len = 0;
    m_config_len = 1;
    m_reply_len = 1;

    // set the device code
    m_player.code = PLAYER_LOCALIZATION_CODE;	// from player's <player.h>

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

    // initialize variables
    this->p_odometry.x = 0;
    this->p_odometry.y = 0;
    this->p_odometry.a = 0;
    m_distance_subscribedp = false;
    m_motion_subscribedp = false;
#ifdef INCLUDE_RTK2
    this->particle_fig = NULL;
    this->hypothesis_fig = NULL;
#endif
}


// Initialize entity
bool CRegularMCLDevice::Startup(void)
{
    if (!CPlayerEntity::Startup())
	return false;

    // set update speed
    m_interval = 1.0 / this->frequency;

    // initialize clustering algorithm
    this->clustering = new MCLClustering(this->num_particles);

    // get the pointer to the distance sensor
    player_device_id_t device;
    if (this->sensor_type == PLAYER_MCL_SONAR) device.code = PLAYER_SONAR_CODE;
    else if (this->sensor_type == PLAYER_MCL_LASER) device.code = PLAYER_LASER_CODE;
    else {
	PRINT_ERR("  invalid distance sensor.");
	return 1;
    }
    device.index = this->sensor_index;
    this->distance_device = NULL;
    for (int i=0; i<this->m_world->GetEntityCount(); i++) {
	CPlayerEntity* e = (CPlayerEntity*)this->m_world->GetEntity(i);
	if (e->m_player.code == device.code && e->m_player.index == device.index) {
	    this->distance_device = e;
	    break;
	}
    }
    if (!this->distance_device) {
	PRINT_ERR("  unable to find a distance sensor device");
	return (-1);
    }

    // get the pointer to the motion sensor
    device.code = PLAYER_POSITION_CODE;
    device.index = this->motion_index;
    this->motion_device = NULL;
    for (int i=0; i<this->m_world->GetEntityCount(); i++) {
	CPlayerEntity* e = (CPlayerEntity*)this->m_world->GetEntity(i);
	if (e->m_player.code == device.code && e->m_player.index == device.index) {
	    this->motion_device = e;
	    break;
	}
    }
    if (!this->motion_device) {
	PRINT_ERR("  unable to find a motion sensor device");
	return (-1);
    }

    // construct a world model (map)
    this->map = new WorldModel(this->map_file,
	    		       this->map_ppm,
			       this->sensor_max,
			       this->map_threshold);
    if (! this->map->isLoaded())
	PRINT_ERR1("  cannot load the map file (%s)", this->map_file);

    // construct a sensor model
    this->sensor_model = new MCLSensorModel(this->sensor_type,
	    				    this->num_ranges,
					    this->poses,
					    this->sensor_max,
					    this->sm_s_hit,
					    this->sm_lambda,
					    this->sm_o_small,
					    this->sm_z_hit,
					    this->sm_z_unexp,
					    this->sm_z_max,
					    this->sm_z_rand,
					    (this->sm_precompute)?true:false);

    // construct an action model
    this->action_model = new MCLActionModel(this->am_a1,
					    this->am_a2,
					    this->am_a3,
					    this->am_a4);

    // initialize particles
    this->Reset();

    return 0;
}


// when the last client unsubscribes from a device
void CRegularMCLDevice::Shutdown(void)
{
    // unsubscribe from the distance sensor device
    if (this->distance_device->Subscribed()) this->distance_device->Unsubscribe();
    if (this->motion_device->Subscribed()) this->motion_device->Unsubscribe();

    // delete models
    delete this->map;
    delete this->sensor_model;
    delete this->action_model;

    // de-allocate memory
    delete[] this->poses;
    delete[] this->ranges;

    // call clean-up procedure of the parent class
    CPlayerEntity::Shutdown();
}


// Load the entity from the worldfile
bool CRegularMCLDevice::Load(CWorldFile *worldfile, int section)
{
    if (!CPlayerEntity::Load(worldfile, section))
	return false;

    // load configuration : update speed
    this->frequency = worldfile->ReadFloat(section, "frequency", 1.0);

    // load configuration : the number of particles
    this->num_particles = worldfile->ReadInt(section, "num_particles", 1000);

    // load configuration : a distance sensor
    const char *sensor_str = worldfile->ReadString(section, "sensor_type", "sonar");
    if (strcmp(sensor_str, "sonar") == 0)
	this->sensor_type = PLAYER_MCL_SONAR;
    else if (strcmp(sensor_str, "laser") == 0)
	this->sensor_type = PLAYER_MCL_LASER;
    else
	this->sensor_type = PLAYER_MCL_NOSENSOR;
    this->sensor_index = worldfile->ReadInt(section, "sensor_index", 0);
    this->sensor_max = worldfile->ReadInt(section, "sensor_max", 8000);
    this->sensor_num_samples = worldfile->ReadInt(section, "sensor_num_samples", 0);

    // load configuration : a motion sensor
    this->motion_index = worldfile->ReadInt(section, "motion_index", 0);

    // load configuration : a world model (map)
    this->map_file = worldfile->ReadString(section, "map", NULL);
    this->map_ppm = worldfile->ReadInt(section, "map_ppm", 10);
    this->map_threshold = worldfile->ReadInt(section, "map_threshold", 240);

    // load configuration : a sensor model
    this->sm_s_hit = worldfile->ReadFloat(section, "sm_s_hit", 500.0);
    this->sm_lambda = worldfile->ReadFloat(section, "sm_lambda", 0.001);
    this->sm_o_small = worldfile->ReadFloat(section, "sm_o_small", 100.0);
    this->sm_z_hit = worldfile->ReadFloat(section, "sm_z_hit", 200.0);
    this->sm_z_unexp = worldfile->ReadFloat(section, "sm_z_unexp", 50.0);
    this->sm_z_max = worldfile->ReadFloat(section, "sm_z_max", 10.0);
    this->sm_z_rand = worldfile->ReadFloat(section, "sm_z_rand", 100.0);
    this->sm_precompute = worldfile->ReadInt(section, "sm_precompute", 1);

    // load configuration : an action model
    this->am_a1 = worldfile->ReadFloat(section, "am_a1", 0.01);
    this->am_a2 = worldfile->ReadFloat(section, "am_a2", 0.0002);
    this->am_a3 = worldfile->ReadFloat(section, "am_a3", 0.03);
    this->am_a4 = worldfile->ReadFloat(section, "am_a4", 0.1);

    // read the configuartion of the distance sensor
    this->ReadConfiguration(worldfile);

    return true;
}


// Read the configuration of a distance sensor
void CRegularMCLDevice::ReadConfiguration(CWorldFile* worldfile)
{
    // read a sonar configuration
    if (this->sensor_type == PLAYER_MCL_SONAR)
    {
	int sonar_section = worldfile->LookupEntity("sonar");
	player_sonar_geom_t config;
        char key[256];

	// Load the geometry of the sonar ring
	int scount = worldfile->ReadInt(sonar_section, "scount", SONARSAMPLES);
	config.pose_count = htons(scount);
	for (int i=0; i<scount; i++) {
	    snprintf(key, sizeof(key), "spose[%d]", i);
	    config.poses[i][0] = htons((uint16_t)
		    (worldfile->ReadTupleLength(sonar_section, key, 0, 0) * 1000));
	    config.poses[i][1] = htons((uint16_t)
		    (worldfile->ReadTupleLength(sonar_section, key, 1, 0) * 1000));
	    config.poses[i][2] = htons((uint16_t)
		    (worldfile->ReadTupleAngle(sonar_section, key, 2,
			    (i*2*M_PI/SONARSAMPLES)) * 180/M_PI));
	}

	// store proper settings
	int pose_count = (uint16_t) ntohs(config.pose_count);
	if (this->sensor_num_samples) {		// subsampling is required
	    int subsampling = pose_count / this->sensor_num_samples;
	    this->num_ranges = this->sensor_num_samples;
	    this->poses = new pose_t[this->sensor_num_samples];
	    for (int i=0, s_idx=0; i<this->sensor_num_samples; i++, s_idx+=subsampling) {
		this->poses[i].x = (int16_t) ntohs(config.poses[s_idx][0]);
		this->poses[i].y = (int16_t) ntohs(config.poses[s_idx][1]);
		this->poses[i].a = (int16_t) ntohs(config.poses[s_idx][2]);
	    }
	    this->ranges = new uint16_t[this->sensor_num_samples];
	    this->inc = subsampling;
	}
	else {					// use a sensor default
	    this->num_ranges = pose_count;
	    this->poses = new pose_t[pose_count];
	    for (int i=0; i<pose_count; i++) {
		this->poses[i].x = (int16_t) ntohs(config.poses[i][0]);
		this->poses[i].y = (int16_t) ntohs(config.poses[i][1]);
		this->poses[i].a = (int16_t) ntohs(config.poses[i][2]);
	    }
	    this->ranges = new uint16_t[pose_count];
	    this->inc = 1;
	}
    }

    // read a laser configuration
    else if (this->sensor_type == PLAYER_MCL_LASER)
    {
	player_laser_geom_t geom;
	player_laser_config_t config;

	// Load the geometry & configuration of the laser
	geom.pose[0] = htons(0);
	geom.pose[1] = htons(0);
	geom.pose[2] = htons(0);
	config.min_angle = htons(-9000);
	config.max_angle = htons(9000);
	config.resolution = htons(50);

	// store proper settings
	float cx = (int16_t) ntohs(geom.pose[0]);
	float cy = (int16_t) ntohs(geom.pose[1]);
	float ca = (int16_t) ntohs(geom.pose[2]);
	float max_angle = (int16_t) ntohs(config.max_angle) / 100.0;
	float min_angle = (int16_t) ntohs(config.min_angle) / 100.0;
	float resolution = (uint16_t) ntohs(config.resolution) / 100.0;
	int pose_count = int((max_angle - min_angle) / resolution);
	if (this->sensor_num_samples) {		// subsampling is required
	    int subsampling = pose_count / this->sensor_num_samples;
	    this->num_ranges = this->sensor_num_samples;
	    this->poses = new pose_t[this->sensor_num_samples];
	    for (int i=0, s_idx=0; i<this->sensor_num_samples; i++, s_idx+=subsampling) {
		this->poses[i].x = cx;
		this->poses[i].y = cy;
		this->poses[i].a = ca + min_angle + (i*resolution*subsampling);
	    }
	    this->ranges = new uint16_t[this->sensor_num_samples];
	    this->inc = subsampling;
	}
	else {					// use a sensor default
	    this->num_ranges = pose_count;
	    this->poses = new pose_t[pose_count];
	    for (int i=0; i<pose_count; i++) {
		this->poses[i].x = cx;
		this->poses[i].y = cy;
		this->poses[i].a = ca + min_angle + (i*resolution);
	    }
	    this->ranges = new uint16_t[pose_count];
	    this->inc = 1;
	}
    }
}


// main update function
void CRegularMCLDevice::Update(double sim_time)
{
    // do nothing until it is subscribed
    if (! this->Subscribed()) {
	if (m_distance_subscribedp) {
	    this->distance_device->Unsubscribe();
	    m_distance_subscribedp = false;
	}
	if (m_motion_subscribedp) {
	    this->motion_device->Unsubscribe();
	    m_motion_subscribedp = false;
	}
	return;
    }

    // subscribe devices if they have not
    if (!m_distance_subscribedp) {
	this->distance_device->Subscribe();
	m_distance_subscribedp = true;
    }
    if (!m_motion_subscribedp) {
	this->motion_device->Subscribe();
	m_motion_subscribedp = true;
	// generate random particles whenever the motion sensor is reset
	this->Reset();
    }

    // Update the configuration.
    UpdateConfig();

    // see if it's time to recalculate MCL
    if (sim_time - m_last_update < m_interval)
	return;
    m_last_update = sim_time;

    // collect sensor data
    pose_t c_odometry;
    if (!this->ReadRanges(this->ranges) || !this->ReadOdometry(c_odometry))
	return; // do nothing until sensors are ready 

    // when a robot didn't move, don't update particles
    if (c_odometry == this->p_odometry) {
	player_localization_data_t data;
	HypothesisConstruction(data);
	PutData((void*)&data, sizeof(data));
	return;
    }

    //////////////////////////////////////////////////////////////////
    // Monte-Carlo Localization Algorithm
    //
    // [step 1] : draw new samples from the previous PDF
    SamplingImportanceResampling(this->p_odometry, c_odometry);

    // [step 2] : update importance factors based on the sensor model
    ImportanceFactorUpdate(this->ranges);

    // [step 3] : generate hypothesis by grouping particles
    player_localization_data_t data;
    HypothesisConstruction(data);
    //
    //////////////////////////////////////////////////////////////////

    // remember the current odometry information
    this->p_odometry = c_odometry;

    // Make data available
    PutData((void*)&data, sizeof(data));
}


// Process configuration requests.
void CRegularMCLDevice::UpdateConfig(void)
{
    int len;
    void *client;
    static char buffer[PLAYER_MAX_REQREP_SIZE];
  
    while ((len = GetConfig(&client, &buffer, sizeof(buffer))) > 0)
    {
	switch (buffer[0])
	{
	    case PLAYER_LOCALIZATION_RESET_REQ:
	    {
		player_localization_reset_t reset;
		// check if the config request is valid
		if (len != sizeof(player_localization_reset_t))
		{
		    PRINT_WARN2("config request len is invalid (%d != %d)", len, sizeof(reset));
		    if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK) != 0)
			PRINT_WARN("PutReply() failed");
		    continue;
		}
		// reset MCL device
		this->Reset();
		// send an acknowledgement to the client
	        if(PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &reset, sizeof(reset)) != 0)
		    PRINT_WARN("PutReply() failed");
		break;
	    }

	    case PLAYER_LOCALIZATION_GET_CONFIG_REQ:
	    {
		player_localization_config_t config;
		// check if the config request is valid
		if (len != sizeof(config.subtype))
		{
		    PRINT_WARN2("config request len is invalid (%d != %d)", len, sizeof(config.subtype));
		    if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK) != 0)
			PRINT_WARN("PutReply() failed");
		    continue;
		}
		// load configuration info
		config.num_particles = htonl(this->num_particles);
		// send the information to the client
	        if(PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &config, sizeof(config)) != 0)
		    PRINT_WARN("PutReply() failed");
		break;
	    }

	    case PLAYER_LOCALIZATION_SET_CONFIG_REQ:
	    {
		player_localization_config_t config;
		// check if the config request is valid
		if (len != sizeof(player_localization_config_t))
		{
		    PRINT_WARN2("config request len is invalid (%d != %d)", len, sizeof(config));
		    if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK) != 0)
			PRINT_WARN("PutReply() failed");
		    continue;
		}
		// change the current configuration
		memcpy(&config, buffer, sizeof(config));
		this->num_particles = (uint32_t) ntohl(config.num_particles);
		// reset the device after changing configuration
		this->Reset();
		// send an acknowledgement to the client
	        if(PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &config, sizeof(config)) != 0)
		    PRINT_WARN("PutReply() failed");
		break;
	    }

	    case PLAYER_LOCALIZATION_GET_MAP_HDR_REQ:
	    {
		player_localization_map_header_t map_header;
		// check if the config request is valid
		if (len != sizeof(player_localization_map_header_t))
		{
		    PRINT_WARN2("config request len is invalid (%d != %d)", len, sizeof(map_header));
		    if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK) != 0)
			PRINT_WARN("PutReply() failed");
		    continue;
		}
		// load the size information of map
		memcpy(&map_header, buffer, sizeof(map_header));
		uint8_t scale = map_header.scale;
		map_header.width = htonl((uint32_t)(this->map->width / float(scale) + 0.5));
		map_header.height = htonl((uint32_t)(this->map->height / float(scale) + 0.5));
		map_header.ppkm = htonl((uint32_t)(this->map->ppm * 1000 / float(scale) + 0.5));
		// send the information to the client
	        if(PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &map_header, sizeof(map_header)) != 0)
		    PRINT_WARN("PutReply() failed");
		break;
	    }

	    case PLAYER_LOCALIZATION_GET_MAP_DATA_REQ:
	    {
		player_localization_map_data_t map_data;
		// check if the config request is valid
		if (len != sizeof(player_localization_map_data_t))
		{
		    PRINT_WARN2("config request len is invalid (%d != %d)", len, sizeof(map_data));
		    if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK) != 0)
			PRINT_WARN("PutReply() failed");
		    continue;
		}
		// retrieve user input
		memcpy(&map_data, buffer, sizeof(map_data));
		uint32_t scale = map_data.scale;
		uint32_t row = (uint32_t) ntohs(map_data.row);
		// compute the size of scaled world
		uint32_t width = (uint32_t)(this->map->width / float(scale) + 0.5);
		uint32_t height = (uint32_t)(this->map->height / float(scale) + 0.5);
		// check if the request is valid
		if (width >= PLAYER_MAX_REQREP_SIZE-4 || row >= height) {
		    if (PutReply(client, PLAYER_MSGTYPE_RESP_NACK) != 0)
			PRINT_WARN("PutReply() failed");
		    continue;
		}
		// fill out the map data
		memset(map_data.data, 255, sizeof(map_data.data));
		uint32_t nrows = sizeof(map_data.data) / width;
		if (row+nrows > height) nrows = height - row;
		for (uint32_t i=0; i<nrows; i++, row++)
		    for (uint32_t h=row*scale; h<(row+1)*scale; h++)
			for (uint32_t w=0; w<this->map->width; w++) {
			    uint8_t np = this->map->map[h*this->map->width + w];
			    uint32_t idx = i*width + (uint32_t)(w/float(scale)+0.5);
			    if (map_data.data[idx] > np) map_data.data[idx] = np;
			}
		// send the information to the client
	        if(PutReply(client, PLAYER_MSGTYPE_RESP_ACK, NULL, &map_data, sizeof(map_data)) != 0)
		    PRINT_WARN("PutReply() failed");
		break;
	    }

	    default:
	    {
		if(PutReply(client, PLAYER_MSGTYPE_RESP_NACK) != 0)
		    PRINT_WARN("PutReply() failed");
		break;
	    }
	}
    }
}


// Reset MCL device
void CRegularMCLDevice::Reset(void)
{
    // pre-compute constants for efficiency
    float width_ratio = float(RAND_MAX) / this->map->Width();
    float height_ratio = float(RAND_MAX) / this->map->Height();
    float yaw_ratio = float(RAND_MAX) / 360;

    // re-generate particles in random
    this->unit_importance = 1.0f / this->num_particles;
    this->particles.clear();
    this->p_buffer.clear();
    particle_t p;
    for (uint32_t i=0; i<this->num_particles; i++) {
	// random pose
	p.pose.x = int(rand() / width_ratio);
	p.pose.y = int(rand() / height_ratio);
	p.pose.a = int(rand() / yaw_ratio);
	// uniform importance
	p.importance = this->unit_importance;
	// cumulative importance
	p.cumulative = this->unit_importance * (i+1);
	// store the particle
	this->particles.push_back(p);
    }

    // re-initialize a clustering algorithm
    this->clustering->Reset(this->map->Width(), this->map->Height());

    // store the current odometry data
    this->ReadOdometry(this->p_odometry);
}


// Read range data from a distance sensor
bool CRegularMCLDevice::ReadRanges(uint16_t* buffer)
{
    // retrieve data from a sonar device
    if (this->sensor_type == PLAYER_MCL_SONAR) {
	player_sonar_data_t data;
	size_t r = this->distance_device->GetData((unsigned char*)&data, sizeof(data));
	if (r != sizeof(data))
	    return false;
	// copy the data into the given buffer
	for (int i=0, s_idx=0; i<this->num_ranges; i++, s_idx+=this->inc)
	    buffer[i] = (uint16_t) ntohs(data.ranges[s_idx]);
	return true;
    }

    // retrieve data from a laser device
    else if (this->sensor_type == PLAYER_MCL_LASER) {
	player_laser_data_t data;
	size_t r = this->distance_device->GetData((unsigned char*)&data, sizeof(data));
	if (r != sizeof(data))
	    return false;
	// copy the data into the given buffer
	for (int i=0, s_idx=0; i<this->num_ranges; i++, s_idx+=this->inc)
	    buffer[i] = (uint16_t) ntohs(data.ranges[s_idx]);
	return true;
    }

    return false;
}


// Read odometry data from a motion sensor
bool CRegularMCLDevice::ReadOdometry(pose_t& pose)
{
    // retrieve data from the motion device
    player_position_data_t data;
    size_t r = this->motion_device->GetData((unsigned char*)&data, sizeof(data));
    if (r != sizeof(data))
	return false;
    // return only pose data
    pose.x = (int32_t) ntohl(data.xpos);
    pose.y = (int32_t) ntohl(data.ypos);
    pose.a = ((int32_t)ntohl(data.yaw));
    return true;
}


// draw new sample from the previous PDF
void CRegularMCLDevice::SamplingImportanceResampling(pose_t from, pose_t to)
{
    for (uint32_t i=0; i<this->num_particles; i++)
    {
	// select a random particle according to the current PDF
	double selected = rand() / double(RAND_MAX);
	int start = 0;
	int end = this->num_particles - 1;
	while (end - start > 1) {
	    int center = (end + start) / 2;
	    if (this->particles[center].cumulative > selected)
		end = center;
	    else if (this->particles[center].cumulative < selected)
		start = center;
	    else {	// bingo!
		start = end = center;
		break;
	    }
	}

	// generate a new particle based on the action model
	particle_t p;
	p.pose = this->action_model->sample(this->particles[end].pose,
					    from, to);
	this->p_buffer.push_back(p);
    }

    // done
    this->p_buffer.swap(this->particles);
    this->p_buffer.clear();
}


// update importance factors based on the sensor model
void CRegularMCLDevice::ImportanceFactorUpdate(uint16_t *ranges)
{
    vector<particle_t>::iterator p;

    // position constraints
    float w_width = this->map->Width();
    float w_height = this->map->Height();

    // update the importance factors
    double sum = 0.0;
    for (p=this->particles.begin(); p<this->particles.end(); p++) {
	// check if the particle is inside of valid environment
	if (p->pose.x >= 0 && p->pose.x <= w_width &&
			      p->pose.y >=0 && p->pose.y <= w_height) {
	    p->importance = this->sensor_model->probability(ranges, p->pose, this->map);
	    sum += p->importance;
	}
	else				// importance of an outside particle will
	    p->importance = 0.0;	// zero, which means it will be never selected
    }

    if (sum == 0.0) {
	PRINT_WARN("the sum of all importance factors is zero. system is reset.");
	this->Reset();
	return;
    }

    // normalize the importance factors
    for (p=this->particles.begin(); p<this->particles.end(); p++)
	p->importance /= sum;

    // compute the cumulative importance factors
    sum = 0.0f;
    for (p=this->particles.begin(); p<this->particles.end(); p++) {
	sum += p->importance;
	p->cumulative = sum;
    }
}


// construct hypothesis by grouping particles
void CRegularMCLDevice::HypothesisConstruction(player_localization_data_t& data)
{
    // reset the clustering algorithm
    this->clustering->Reset(this->map->Width(), this->map->Height());

    // run clustering algorithm
    this->clustering->cluster(this->particles);

    // Prepare packet and byte swap
    int n = 0;
    for (int i=0; i<PLAYER_LOCALIZATION_MAX_HYPOTHESIS; i++)
    {
	if (this->clustering->pi[i] > 0.0) {
	    data.hypothesis[n].alpha = htonl((uint32_t)(this->clustering->pi[i] * 1000000000));

	    data.hypothesis[n].mean[0] = htonl((int32_t)(this->clustering->mean[i][0]));
	    data.hypothesis[n].mean[1] = htonl((int32_t)(this->clustering->mean[i][1]));
	    data.hypothesis[n].mean[2] = htonl((int32_t)(this->clustering->mean[i][2]));

	    data.hypothesis[n].cov[0][0] = htonl((int32_t)(this->clustering->cov[i][0][0]));
	    data.hypothesis[n].cov[0][1] = htonl((int32_t)(this->clustering->cov[i][0][1]));
	    data.hypothesis[n].cov[0][2] = htonl((int32_t)(this->clustering->cov[i][0][2]));
	    data.hypothesis[n].cov[1][0] = htonl((int32_t)(this->clustering->cov[i][1][0]));
	    data.hypothesis[n].cov[1][1] = htonl((int32_t)(this->clustering->cov[i][1][1]));
	    data.hypothesis[n].cov[1][2] = htonl((int32_t)(this->clustering->cov[i][1][2]));
	    data.hypothesis[n].cov[2][0] = htonl((int32_t)(this->clustering->cov[i][2][0]));
	    data.hypothesis[n].cov[2][1] = htonl((int32_t)(this->clustering->cov[i][2][1]));
	    data.hypothesis[n].cov[2][2] = htonl((int32_t)(this->clustering->cov[i][2][2]));

	    n++;
	}
    }
    data.num_hypothesis = htonl(n);
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
    static double cov[2][2], eval[2], evec[2][2];

    CPlayerEntity::RtkUpdate();
 
    rtk_fig_clear(this->particle_fig);
    rtk_fig_clear(this->hypothesis_fig);
  
    // if a client is subscribed to this device
    if (this->Subscribed() > 0 && m_world->ShowDeviceData(this->lib_entry->type_num))
    {
	// draw particles
	if (this->num_particles > 0) {
            for( unsigned int i=0; i<this->num_particles; i++ ) {
		rtk_fig_color_rgb32(this->particle_fig, this->particle_color);
        	rtk_fig_rectangle(this->particle_fig,
			this->particles[i].pose.x / 1000.0,
			this->particles[i].pose.y / 1000.0,
			D2R(this->particles[i].pose.a),
			0.1, 0.1, TRUE);
	    }
	}
    
        // attempt to get the right size chunk of data from the mmapped buffer
	player_localization_data_t data;
	if (this->GetData(&data, sizeof(data)) == sizeof(data)) {
            for( unsigned int i=0; i<(uint32_t)ntohl(data.num_hypothesis); i++ )
	    {
		double ox = (int32_t) ntohl(data.hypothesis[i].mean[0]) / 1000.0;
		double oy = (int32_t) ntohl(data.hypothesis[i].mean[1]) / 1000.0;

		cov[0][0] = (int32_t) ntohl(data.hypothesis[i].cov[0][0]);
		cov[0][1] = (int32_t) ntohl(data.hypothesis[i].cov[0][1]);
		cov[1][1] = (int32_t) ntohl(data.hypothesis[i].cov[1][1]);
		eigen(cov, eval, evec);

		double oa = atan(evec[1][0] / evec[0][0]);
		double sx = 3 * sqrt(fabs(eval[0])) / 1000.0;
		double sy = 3 * sqrt(fabs(eval[1])) / 1000.0;

        	rtk_fig_line_ex(this->hypothesis_fig, ox, oy, oa, 1.0);
        	rtk_fig_line_ex(this->hypothesis_fig, ox, oy, oa+M_PI_2, 1.0);
        	rtk_fig_ellipse(this->hypothesis_fig, ox, oy, oa, sx, sy, FALSE);
            }  
        }

	/*
	// [debug] draw the ranges of the best particle
	vector<particle_t>::iterator best_particle = 
	    max_element(this->particles.begin(), this->particles.end());
	pose_t robot;
	robot.x = best_particle->pose.x;
	robot.y = best_particle->pose.y;
	robot.a = best_particle->pose.a;
	double sina = sin(robot.a * M_PI / 180);
	double cosa = cos(robot.a * M_PI / 180);
	for (int i=0; i<this->num_ranges; i++)
	{
	    // estimated range
	    pose_t from;
	    from.x = robot.x +
		int(this->poses[i].x * cosa - this->poses[i].y * sina);
	    from.y = robot.y +
		int(this->poses[i].x * sina + this->poses[i].y * cosa);
	    from.a = robot.a + this->poses[i].a;
	    uint16_t estimation = this->map->estimateRange(from);
	    // destimation
	    pose_t to;
	    to.x = from.x + int(estimation * cos(from.a * M_PI / 180));
	    to.y = from.y + int(estimation * sin(from.a * M_PI / 180));
	    // draw
	    rtk_fig_line(this->hypothesis_fig,
		    from.x / 1000.0,
		    from.y / 1000.0,
		    to.x / 1000.0,
		    to.y / 1000.0);
	}
	*/
    }
}


#endif

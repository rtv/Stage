/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *
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
 * Desc: Ranger interface for Stage's player driver
 * Author: Richard Vaughan
 * Date: 25 November 2010
 * CVS: $Id$
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player
    @par Ranger interface
    -	PLAYER_RANGER_DATA_RANGE
    - PLAYER_RANGER_DATA_INTENS
    - PLAYER_RANGER_REQ_GET_CONFIG
    - PLAYER_RANGER_REQ_GET_GEOM
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"
using namespace Stg;

InterfaceRanger::InterfaceRanger(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf,
                                 int section)
    : InterfaceModel(addr, driver, cf, section, "ranger")
{
  this->scan_id = 0;
}

void InterfaceRanger::Publish(void)
{
  ModelRanger *rgr = dynamic_cast<ModelRanger *>(this->mod);

  // the Player interface dictates that if multiple sensor poses are
  // given, then we have exactly one range reading per sensor. To give
  // multiple ranges from the same origin, only one sensor is allowed.

  // need to use the mutable version since we access the range data via a regular pointer
  std::vector<ModelRanger::Sensor> &sensors = rgr->GetSensorsMutable();

  player_ranger_data_range_t prange;
  memset(&prange, 0, sizeof(prange));

  player_ranger_data_intns_t pintens;
  memset(&pintens, 0, sizeof(pintens));

  std::vector<double> rv, iv;

  if (sensors.size() == 1) // a laser scanner type, with one beam origin and many ranges
  {
    prange.ranges_count = sensors[0].ranges.size();
    prange.ranges = prange.ranges_count ? &sensors[0].ranges[0] : NULL;

    pintens.intensities_count = sensors[0].intensities.size();
    pintens.intensities = pintens.intensities_count ? &sensors[0].intensities[0] : NULL;
  } else { // a sonar/IR type with one range per beam origin
    FOR_EACH (it, sensors) {
      if (it->ranges.size())
        rv.push_back(it->ranges[0]);

      if (it->intensities.size())
        iv.push_back(it->intensities[0]);
    }

    prange.ranges_count = rv.size();
    prange.ranges = rv.size() ? &rv[0] : NULL;

    pintens.intensities_count = iv.size();
    pintens.intensities = iv.size() ? &iv[0] : NULL;
  }

  if (prange.ranges_count)
    this->driver->Publish(this->addr, PLAYER_MSGTYPE_DATA, PLAYER_RANGER_DATA_RANGE,
                          (void *)&prange, sizeof(prange), NULL);

  if (pintens.intensities_count)
    this->driver->Publish(this->addr, PLAYER_MSGTYPE_DATA, PLAYER_RANGER_DATA_INTNS,
                          (void *)&pintens, sizeof(pintens), NULL);
}

int InterfaceRanger::ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data)
{
  ModelRanger *mod = (ModelRanger *)this->mod;

  // Is it a request to get the ranger's config?
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_CONFIG, this->addr)) {
    if (hdr->size == 0) {
      // the Player ranger config is a little weaker than Stage's
      // natice device, so all we can do is warn about this.
      PRINT_WARN("stageplugin ranger config describes only the first sensor of the ranger.");

      player_ranger_config_t prc;
      bzero(&prc, sizeof(prc));

      const ModelRanger::Sensor &s = mod->GetSensors()[0];

      prc.min_angle = -s.fov / 2.0;
      prc.max_angle = +s.fov / 2.0;
      prc.angular_res = s.fov / (double)s.sample_count;
      prc.max_range = s.range.max;
      prc.min_range = s.range.min;
      prc.range_res = 1.0 / mod->GetWorld()->Resolution();
      prc.frequency = 1.0E6 / mod->GetInterval();

      this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                            PLAYER_RANGER_REQ_GET_CONFIG, (void *)&prc, sizeof(prc), NULL);
      return (0);
    } else {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size, 0);
      return (-1);
    }
  } else // Is it a request to get the ranger's geom?
      if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, PLAYER_RANGER_REQ_GET_GEOM, this->addr)) {
    if (hdr->size == 0) {
      Geom geom = mod->GetGeom();
      Pose pose = mod->GetPose();

      const std::vector<ModelRanger::Sensor> &sensors = mod->GetSensors();

      // fill in the geometry data formatted player-like
      player_ranger_geom_t pgeom;
      bzero(&pgeom, sizeof(pgeom));
      pgeom.pose.px = pose.x;
      pgeom.pose.py = pose.y;
      pgeom.pose.pyaw = pose.a;
      pgeom.size.sl = geom.size.x;
      pgeom.size.sw = geom.size.y;

      pgeom.element_poses_count = pgeom.element_sizes_count = sensors.size();

      player_pose3d_t poses[sensors.size()];
      player_bbox3d_t sizes[sensors.size()];

      for (size_t s = 0; s < pgeom.element_poses_count; s++) {
        poses[s].px = sensors[s].pose.x;
        poses[s].py = sensors[s].pose.y;
        poses[s].pz = sensors[s].pose.z;
        poses[s].proll = 0.0;
        poses[s].ppitch = 0.0;
        poses[s].pyaw = sensors[s].pose.a;

        sizes[s].sw = sensors[s].size.x;
        sizes[s].sl = sensors[s].size.y;
        sizes[s].sh = sensors[s].size.z;
      }

      pgeom.element_poses = poses;
      pgeom.element_sizes = sizes;

      this->driver->Publish(this->addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
                            PLAYER_RANGER_REQ_GET_GEOM, (void *)&pgeom, sizeof(pgeom), NULL);
      return (0);
    } else {
      PRINT_ERR2("config request len is invalid (%d != %d)", (int)hdr->size, 0);
      return (-1);
    }
  }

  // Don't know how to handle this message.
  PRINT_WARN2("stage ranger doesn't support message %d:%d.", hdr->type, hdr->subtype);
  return (-1);
}

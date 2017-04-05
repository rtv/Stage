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
 * Desc: A plugin driver for Player that gives access to Stage's camera.
 * Author: Kevin Barry
 * Date: 05 Febuary 2009
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player
@par Camera interface
- PLAYER_CAMERA_DATA_STATE
*/

// CODE ----------------------------------------------------------------------

#include "p_driver.h"
using namespace Stg;

InterfaceCamera::InterfaceCamera(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf,
                                 int section)
    : InterfaceModel(addr, driver, cf, section, "camera")
{
}

void InterfaceCamera::Publish(void)
{
  ModelCamera *mod = (ModelCamera *)this->mod;

  // don't publish anything until we have some real data
  if (!mod->FrameColor())
    return;

  player_camera_data_t pdata;
  memset(&pdata, 0, sizeof(pdata));

  pdata.width = mod->getWidth();
  pdata.height = mod->getHeight();
  pdata.bpp = 8 * 3;
  pdata.format = PLAYER_CAMERA_FORMAT_RGB888;
  pdata.image_count = 3 * pdata.width * pdata.height;
  pdata.image = new uint8_t[pdata.image_count];

  /* Image from Stage is stored as R G B A R G B A ...
   * With the row fartherest from the camera as row 0 (opposite of Player).
   *
   * Here we dump the Alpha data (so just R G B R G B and also flip the image
   */
  uint8_t *ptr = (uint8_t *)mod->FrameColor();
  const int RGB_SIZE = 3;

  for (int row = pdata.height - 1; row >= 0; row--) {
    for (int col = 0; col < pdata.width; col++) {
      for (int color = 0; color < 4; color++) {
        if (color < 3) /* is red green or blue */
          pdata.image[color + RGB_SIZE * col + row * pdata.width * RGB_SIZE] = *(ptr++);
        else
          ptr++; /* throw out the alpha */
      }
    }
  }
  // Write camera data
  this->driver->Publish(this->addr, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE, (void *)(&pdata),
                        sizeof(pdata), NULL);

  delete[] pdata.image;
}

int InterfaceCamera::ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data)
{
  // ModelCamera* mod = (ModelCamera*)this->mod;

  // Is it a request to set the camera's config?
  return (-1);
}

/*
 * vim:set ts=2:sw=2
 */

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
 * Desc: Simulates a root ring.
 * Author: Andrew Howard, Richard Vaughan
 * Date: 28 Nov 2000
 * CVS info: $Id: root.hh,v 1.1.2.5 2003-02-24 04:47:12 rtv Exp $
 */

#ifndef ROOTDEVICE_HH
#define ROOTDEVICE_HH

#include "entity.hh"
#include "library.hh"


class CRootEntity : public CEntity
{
  // Default constructor
public: CRootEntity( void );
  
  // a static named constructor - a pointer to this function is given
  // to the Library object and paired with a string.  When the string
  // is seen in the worldfile, this function is called to create an
  // instance of this entity
  //public: static CRootEntity* Creator( LibraryItem *libit,
  //			     int id, CEntity *parent )
  //{
  // return( new CRootEntity( libit ) );
  //}
  
  // Startup routine
private: int CreateMatrix( void );

  double ppm; // passed into CMatrix creator

  // Update the device
public: 
  //  virtual void Update(double sim_time);
  virtual int Property( int con, stage_prop_id_t property, 
			void* value, size_t len, stage_buffer_t* reply );
  
};

#endif


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
 * Desc: Simulates a sonar ring.
 * Author: Andrew Howard, Richard Vaughan
 * Date: 28 Nov 2000
 * CVS info: $Id: sonar.cc,v 1.1.2.7 2003-02-24 04:47:13 rtv Exp $
 */

#include <assert.h>
#include <math.h>
#include "sonar.hh"
#include "raytrace.hh"
#include "sio.h"

// constructor
CSonarModel::CSonarModel( int id, char* token, char* color, CEntity *parent )
  : CEntity( id, token, color, parent )    
{
  this->min_range = 0.20;
  this->max_range = 5.0;

  this->power_on = true;

  this->m_interval = 0.1; // 10Hz update

  // Initialise the sonar poses to default values
  this->sonar_count = SONARSAMPLES;
  
  // we don't need a rectangle 
  SetRects( NULL, 0 );
  
  for (int i = 0; i < this->sonar_count; i++)
    {
      this->sonars[i][0] = 0;
      this->sonars[i][1] = 0;
      this->sonars[i][2] = i * 2 * M_PI / this->sonar_count;
    }
  
}

///////////////////////////////////////////////////////////////////////////
// Update the sonar data
int CSonarModel::Update() 
{ 
  //PRINT_WARN( "update" );

  CEntity::Update();
  
  // is anyone interested in my data? if not, bail here.
  if( !IsSubscribed( STG_PROP_ENTITY_DATA ) )
    return 0;

  //PRINT_WARN( "subscribed" );
  
  //PRINT_WARN3( "time now %.4f time since last update %.4f interval %.4f",
  //       CEntity::simtime,  CEntity::simtime - m_last_update, m_interval );

  // Check to see if it is time to update
  //  - if not, return right away.
  if( CEntity::simtime - m_last_update < m_interval)
    return 0;

  //PRINT_WARN( "time to upate" );

  m_last_update = CEntity::simtime;
  
  // place to store the ranges as we generate them
  double ranges[SONARSAMPLES];
  
  if( this->power_on )
    {
      //PRINT_WARN( "power on" );
      
      // Do each sonar
      for (int s = 0; s < this->sonar_count; s++)
	{
	  // Compute parameters of scan line
	  double ox = this->sonars[s][0];
	  double oy = this->sonars[s][1];
	  double oth = this->sonars[s][2];
	  LocalToGlobal(ox, oy, oth);
	  
	  ranges[s] = this->max_range;
	  
	  CLineIterator lit( ox, oy, oth, this->max_range, 
			     CEntity::matrix, PointToBearingRange );
	  CEntity* ent;
	  
	  while( (ent = lit.GetNextEntity()) )
	    {
	      if( ent != this && ent != m_parent_entity && ent->sonar_return ) 
		{
		  ranges[s] = lit.GetRange();
		  break;
		}
	    }	
	}
    }
  
  //PRINT_WARN1( "sonar exporting data at %.4f seconds", CEntity::simtime );
  
  // this is the right way to export data, so everyone else finds out about it
  Property( -1, STG_PROP_ENTITY_DATA, 
	    (char*)ranges, sonar_count * sizeof(ranges[0]), NULL );
  
  return 0;
}

int CSonarModel::Property( int con, stage_prop_id_t property, 
			   void* value, size_t len, stage_buffer_t* reply )
{
  PRINT_DEBUG3( "setting prop %s (%d bytes) for sonar ent %d",
		SIOPropString(property), (int)len, stage_id );
  
  switch( property )
    {
    case STG_PROP_SONAR_POWER:
      if( value ) // set our power state
	{
	  PRINT_WARN( "setting sonar power" );
	  assert(len == sizeof(bool) );
	  this->power_on = (bool*)value;
	}
      if( reply ) // reply with the current power state
	{
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_SONAR_POWER,
			     &this->power_on, sizeof(this->power_on), 
			     STG_ISREPLY );
	}
      break;
      
    case STG_PROP_SONAR_GEOM:
      if( value ) // set the poses of our transducers 
	{
	  PRINT_WARN( "set sonar geometry not implemented" );
	}
      if( reply ) // reply with our array of sonar poses
	{
	  SIOBufferProperty( reply, this->stage_id, STG_PROP_SONAR_GEOM,
			     &this->sonars,
			     sonar_count * 3 * sizeof(sonars[0][0]), 
			     STG_ISREPLY );
	}
      break;
      
    default: 
      break;
    }
  
  
  // inherit the 
  CEntity::Property( con, property, value, len, reply );
  
  return 0; // success
}

int SonarTest( int connection )
{
  stage_model_t sonar;
  sonar.parent_id = 0; // should be the root object
  strncpy( sonar.token, "test_sonar", STG_TOKEN_MAX );
  //assert( CreateModels( connection,  &sonar ) == 0 );



  // define some properties
  stage_buffer_t* props = SIOCreateBuffer();
  assert(props);
  
  bool power = false;
  //SIOBufferProperty( props, box.id, STG_PROP_ENTITY_SUBSCRIBE,
  //	     subs, 1*sizeof(subs[0]) );
  
  
  // turn power off
  SIOBufferProperty( props, sonar.id, STG_PROP_SONAR_POWER,
		     &power, sizeof(power), STG_NOREPLY );
  
  // turn power on
  power = true;
  SIOBufferProperty( props, sonar.id, STG_PROP_SONAR_POWER,
		     &power, sizeof(power), STG_NOREPLY );
  
  return 0; //success
}

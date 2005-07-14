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
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Richard Vaughan
 * Date: 10 December 2004
 * CVS: $Id: player_interfaces.cc,v 1.16 2005-07-14 23:37:23 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

// TODO - configs I should implement
//  - PLAYER_SONAR_POWER_REQ
//  - PLAYER_BLOBFINDER_SET_COLOR_REQ
//  - PLAYER_BLOBFINDER_SET_IMAGER_PARAMS_REQ

// CODE ------------------------------------------------------------

#define DEBUG

#include "player_interfaces.h"
#include "player_driver.h"

#include "playerclient.h"

// this is a Player global
extern bool quiet_startup;

#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )


InterfaceModel::InterfaceModel(  player_device_id_t id, 
				 StgDriver* driver,
				 ConfigFile* cf, 
				 int section,
				 stg_model_type_t modtype )
  : Interface( id, driver, cf, section )
{
  //puts( "InterfaceModel constructor" );
  
  const char* model_name = cf->ReadString(section, "model", NULL );
  
  if( model_name == NULL )
    {
      PRINT_ERR1( "device \"%s\" uses the Stage driver but has "
		  "no \"model\" value defined. You must specify a "
		  "model name that matches one of the models in "
		  "the worldfile.",
		  model_name );
      return; // error
    }
  
  this->mod = driver->LocateModel( model_name, modtype );
  
  if( !this->mod )
    {
      printf( " ERROR! no model available for this device."
	      " Check your world and config files.\n" );
      return;
    }
  
  if( !quiet_startup )
    printf( "\"%s\"\n", this->mod->token );
}      



/////////////////////////////////////////////////////////////////////////////////////////////////


// 
// BLOBFINDER INTERFACE
//

InterfaceBlobfinder::InterfaceBlobfinder( player_device_id_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, STG_MODEL_BLOB )
{
  this->data_len = sizeof(player_blobfinder_data_t);
  this->cmd_len = 0;
}

void InterfaceBlobfinder::Publish( void )
{
  size_t len=0;
  stg_blobfinder_blob_t* blobs = (stg_blobfinder_blob_t*)
    stg_model_get_property( this->mod, "blob_data", &len );
  
  size_t bcount = len / sizeof(stg_blobfinder_blob_t);
  
  // limit the number of samples to Player's maximum
  if( bcount > PLAYER_BLOBFINDER_MAX_BLOBS )
    bcount = PLAYER_BLOBFINDER_MAX_BLOBS;      
  
  player_blobfinder_data_t bfd;
  memset( &bfd, 0, sizeof(bfd) );
  
  // get the configuration
  stg_blobfinder_config_t *cfg = (stg_blobfinder_config_t*)
    stg_model_get_property_fixed( this->mod, "blob_cfg", sizeof(stg_blobfinder_config_t));
  assert(cfg);
  
  // and set the image width * height
  bfd.width = htons((uint16_t)cfg->scan_width);
  bfd.height = htons((uint16_t)cfg->scan_height);
  bfd.blob_count = htons((uint16_t)bcount);
  
  // now run through the blobs, packing them into the player buffer
  // counting the number of blobs in each channel and making entries
  // in the acts header 
  unsigned int b;
  for( b=0; b<bcount; b++ )
    {
      // I'm not sure the ACTS-area is really just the area of the
      // bounding box, or if it is in fact the pixel count of the
      // actual blob. Here it's just the rectangular area.
      
      // useful debug - leave in
      /*
	cout << "blob "
	<< " channel: " <<  (int)blobs[b].channel
	<< " area: " <<  blobs[b].area
	<< " left: " <<  blobs[b].left
	<< " right: " <<  blobs[b].right
	<< " top: " <<  blobs[b].top
	<< " bottom: " <<  blobs[b].bottom
	<< endl;
      */
      
      bfd.blobs[b].x      = htons((uint16_t)blobs[b].xpos);
      bfd.blobs[b].y      = htons((uint16_t)blobs[b].ypos);
      bfd.blobs[b].left   = htons((uint16_t)blobs[b].left);
      bfd.blobs[b].right  = htons((uint16_t)blobs[b].right);
      bfd.blobs[b].top    = htons((uint16_t)blobs[b].top);
      bfd.blobs[b].bottom = htons((uint16_t)blobs[b].bottom);
      
      bfd.blobs[b].color = htonl(blobs[b].color);
      bfd.blobs[b].area  = htonl(blobs[b].area);          
    }
  
  size_t size = sizeof(bfd) - sizeof(bfd.blobs) + bcount * sizeof(bfd.blobs[0]);   
  this->driver->PutData( this->id, &bfd, size, NULL);
}

void InterfaceBlobfinder::Configure( void* client, void* buffer, size_t len )
{
  printf("got blobfinder request\n");
  
  if (this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
    DRIVER_ERROR("PutReply() failed");  
}

//
// MAP INTERFACE
// 

typedef struct
{
  double ppm;
  unsigned int width, height;
  int8_t* data;
} stg_map_info_t;

void MapData( Interface* device, void* data, size_t len )
{
  //PRINT_WARN( "someone subscribed to the map interface, but we don't generate any data" );
  device->driver->PutData( device->id, NULL, 0, NULL); 
}

// Handle map info request - adapted from Player's mapfile driver by
// Brian Gerkey
void  MapConfigInfo( Interface* device,  
		     void *client, 
		     void *request, 
		     size_t len)
{
  printf( "Stage: device \"%s\" received map info request\n", device->mod->token );
  
  // prepare the map info for the client
  player_map_info_t info;  
  size_t reqlen =  sizeof(info.subtype); // Expected length of request
  
  // check if the config request is valid
  if(len != reqlen)
    {
      PLAYER_ERROR2("config request len is invalid (%d != %d)", len, reqlen);
      if (device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
	PLAYER_ERROR("PutReply() failed");
      return;
    }
  
  // create and render a map for this model
  stg_geom_t geom;
  stg_model_get_geom( device->mod, &geom );
  
  // if we already have a map info for this model, destroy it
  stg_map_info_t* minfo = (stg_map_info_t*)
    stg_model_get_property_fixed( device->mod, "_map", 
				       sizeof(stg_map_info_t) );
  
  if( minfo )
    {
      if( minfo->data ) delete [] minfo->data;
      delete minfo;
    }
  
  minfo = new stg_map_info_t();
  minfo->ppm = device->mod->world->ppm;  
  minfo->width = (unsigned int)( geom.size.x * minfo->ppm );
  minfo->height = (unsigned int)( geom.size.y * minfo->ppm );
  
  // create an occupancy grid of the correct size, initially full of
  // unknown cells
  //int8_t* map = NULL; 
  assert( (minfo->data = new int8_t[minfo->width*minfo->height] ));
  memset( minfo->data, 0, minfo->width * minfo->height );
  
  printf( "Stage: creating map for model \"%s\" of %d by %d cells... ", 
	  device->mod->token, minfo->width, minfo->height );
  fflush(stdout);

  size_t sz=0;
  stg_polygon_t* polys = (stg_polygon_t*)
    stg_model_get_property( device->mod, "polygons", &sz);
  size_t count = sz / sizeof(stg_polygon_t);
  
  if( polys && count ) // if we have something to render
    {
      
      // we'll use a temporary matrix, as it knows how to render objects
      
      stg_matrix_t* matrix = stg_matrix_create( minfo->ppm, 
						device->mod->world->matrix->width,
						device->mod->world->matrix->height );    
      if( count > 0 ) 
	{
	  // render the model into the matrix
	  stg_matrix_polygons( matrix, 
			       geom.size.x/2.0,
			       geom.size.y/2.0,
			       0,
			       polys, count, 
			       device->mod );  
	  
	  stg_bool_t* boundary = (stg_bool_t*)
	    stg_model_get_property_fixed( device->mod, 
					  "boundary", sizeof(stg_bool_t));
	  if( *boundary )    
	    stg_matrix_rectangle( matrix,
				  geom.size.x/2.0,
				  geom.size.y/2.0,
				  0,
				  geom.size.x,
				  geom.size.y,
				  device->mod );
	}
      
      // Now convert the matrix to a Player occupancy grid. if the
      // matrix contains anything in a cell, we set that map cell to
      // be occupied, else unoccupied
      
      for( unsigned int p=0; p<minfo->width; p++ )
	for( unsigned int q=0; q<minfo->height; q++ )
        {	  
	  //printf( "%d,%d \n", p,q );

	  stg_cell_t* cell = 
	    stg_cell_locate( matrix->root, p/minfo->ppm, q/minfo->ppm );

	  if( cell && cell->data )
	    minfo->data[ q * minfo->width + p ] =  1;
	  else
	    minfo->data[ q * minfo->width + p ] =  0;
	}
      
      // we're done with the matrix
      stg_matrix_destroy( matrix );
    }

  puts( "done." );

  // store the map as a model property named "_map"
  stg_model_set_property( device->mod, "_map", (void*)minfo, sizeof(minfo) );

  //if(this->mapdata == NULL)
  //{
  //PLAYER_ERROR("NULL map data");
  //if(PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
  //  PLAYER_ERROR("PutReply() failed");
  // return;
  
  // pixels per kilometer
  info.scale = htonl((uint32_t)rint(minfo->ppm * 1e3 ));
  
  // size in pixels
  info.width = htonl((uint32_t)minfo->width);
  info.height = htonl((uint32_t)minfo->height);
  
  // Send map info to the client
  if (device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_ACK, &info, sizeof(info), NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
  
  return;
}

// Handle map data request
void MapConfigData( Interface* device,  
		    void *client, 
		    void *request, 
		    size_t len)
{
  //printf( "device %s received map data request\n", device->mod->token );

  //int i, j;
  unsigned int oi, oj, si, sj;
  size_t reqlen;
  player_map_data_t data;
  
  // Expected length of request
  reqlen = sizeof(data) - sizeof(data.data);
  
  // check if the config request is valid
  if(len != reqlen)
    {
      PLAYER_ERROR2("config request len is invalid (%d != %d)", len, reqlen);
      if( device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
      PLAYER_ERROR("PutReply() failed");
      return;
    }
  
  stg_map_info_t* minfo = (stg_map_info_t*)
    stg_model_get_property_fixed( device->mod, "_map", sizeof(minfo));

  assert( minfo );

  int8_t* map = NULL;  
  assert( (map = minfo->data ) );

  // Construct reply
  memcpy(&data, request, len);
  
   oi = ntohl(data.col);
   oj = ntohl(data.row);
   si = ntohl(data.width);
   sj = ntohl(data.height);
      
  const char* spin = "|/-\\";
  static int spini = 0;

  if( spini == 0 )
    printf( "Stage: downloading map... " );
  
  putchar( 8 ); // backspace
  
  if( oi+si == minfo->width && oj+sj == minfo->height )
    {
      puts( " done." );
      spini = 0;
    }
  else
    {
      putchar( spin[spini++%4] );
      fflush(stdout);
    }

  fflush(stdout);
  
   //   // Grab the pixels from the map
   for( unsigned int j = 0; j < sj; j++)
     {
       for( unsigned int i = 0; i < si; i++)
	 {
	   if((i * j) <= PLAYER_MAP_MAX_CELLS_PER_TILE)
	     {
	       if( i+oi >= 0 &&
		   i+oi < minfo->width &&
		   j+oj >= 0 &&
		   j+oj < minfo->height )
		 data.data[i + j * si] = map[ i+oi + (minfo->width * (j+oj)) ];
	       else
		 {
		   PLAYER_WARN2("requested cell (%d,%d) is offmap", i+oi, j+oj);
		   data.data[i + j * si] = 0;
		 }
	     }
	   else
	     {
	       PLAYER_WARN("requested tile is too large; truncating");
	       if(i == 0)
		 {
		   data.width = htonl(si-1);
		   data.height = htonl(j-1);
		 }
	       else
		 {
		   data.width = htonl(i);
		   data.height = htonl(j);
		 }
	     }
	 }
     }
      
   
   //printf( "Stage driver: providing map data %d/%d %d/%d\n",
   // oi+si, minfo->width, oj+si, minfo->height );


   //fflush(stdout);
     

  // Send map info to the client
  if(device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_ACK, &data, 
              sizeof(data) - sizeof(data.data) + 
              ntohl(data.width) * ntohl(data.height),NULL) != 0)
    PLAYER_ERROR("PutReply() failed");
  return;
}


void MapConfig( Interface* device, void* client, void* buffer, size_t len )
{
  switch( ((unsigned char*)buffer)[0])
    {
    case PLAYER_MAP_GET_INFO_REQ:
      MapConfigInfo( device, client, buffer, len);
      break;
    case PLAYER_MAP_GET_DATA_REQ:
      MapConfigData( device, client, buffer, len);
      break;
    default:
      PLAYER_ERROR("got unknown map config request; ignoring");
      if(device->driver->PutReply(client, PLAYER_MSGTYPE_RESP_NACK,NULL) != 0)
        PLAYER_ERROR("PutReply() failed");
      break;
    }
}

// 
// FIDUCIAL INTERFACE
//

InterfaceFiducial::InterfaceFiducial(  player_device_id_t id, StgDriver* driver, ConfigFile* cf, int section )
  : InterfaceModel( id, driver, cf, section, STG_MODEL_FIDUCIAL )
{
  this->data_len = sizeof(player_fiducial_data_t);
  this->cmd_len = 0; // no commands
}

void InterfaceFiducial::Publish( void )
{
  size_t len = 0;
  stg_fiducial_t* fids = (stg_fiducial_t*)
    stg_model_get_property( this->mod, "fiducial_data", &len );
  
  player_fiducial_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  if( len > 0 )
    {
      size_t fcount = len / sizeof(stg_fiducial_t);      
      assert( fcount > 0 );
      
      pdata.count = htons((uint16_t)fcount);
      
      //printf( "reporting %d fiducials\n",
      //      fcount );

      if( fcount > PLAYER_FIDUCIAL_MAX_SAMPLES )
	{
	  PRINT_WARN2( "A Stage model has detected more fiducials than will fit in Player's buffer (%d/%d)\n",
		      fcount, PLAYER_FIDUCIAL_MAX_SAMPLES );
	  fcount = PLAYER_FIDUCIAL_MAX_SAMPLES;
	}

      for( int i=0; i<(int)fcount; i++ )
	{
	  pdata.fiducials[i].id = htons((int16_t)fids[i].id);
	  
	  // 2D x,y only
	  
	  double xpos = fids[i].range * cos(fids[i].bearing);
	  double ypos = fids[i].range * sin(fids[i].bearing);
	  
	  pdata.fiducials[i].pos[0] = htonl((int32_t)(xpos*1000.0));
	  pdata.fiducials[i].pos[1] = htonl((int32_t)(ypos*1000.0));
	  
	  // yaw only
	  pdata.fiducials[i].rot[2] = htonl((int32_t)(fids[i].geom.a*1000.0));	      	  
	  // player can't handle per-fiducial size.
	  // we leave uncertainty (upose) at zero
	}
    }
  
  // publish this data
  this->driver->PutData( this->id, &pdata, sizeof(pdata), NULL);
}

void InterfaceFiducial::Configure( void* client, void* src, size_t len  )
{
  //printf("got fiducial request\n");

  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_FIDUCIAL_GET_GEOM:
      {	
	// just get the model's geom - Stage doesn't have separate
	// fiducial geom (yet)
	stg_geom_t geom;
	stg_model_get_geom(this->mod,&geom);
	
	// fill in the geometry data formatted player-like
	player_fiducial_geom_t pgeom;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 
	
	pgeom.fiducial_size[0] = ntohs((uint16_t)100); // TODO - get this info
	pgeom.fiducial_size[1] = ntohs((uint16_t)100);
	
	if( this->driver->PutReply(  this->id, client, PLAYER_MSGTYPE_RESP_ACK,  
				       &pgeom, sizeof(pgeom), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
      }
      break;
      
    case PLAYER_FIDUCIAL_SET_FOV:
      
      if( len == sizeof(player_fiducial_fov_t) )
	{
	  player_fiducial_fov_t* pfov = (player_fiducial_fov_t*)src;
	  
	  // convert from player to stage FOV packets
	  stg_fiducial_config_t setcfg;
	  memset( &setcfg, 0, sizeof(setcfg) );
	  setcfg.min_range = (uint16_t)ntohs(pfov->min_range) / 1000.0;
	  setcfg.max_range_id = (uint16_t)ntohs(pfov->max_range) / 1000.0;
	  setcfg.max_range_anon = setcfg.max_range_id;
	  setcfg.fov = DTOR((uint16_t)ntohs(pfov->view_angle));
	  
	  //printf( "setting fiducial FOV to min %f max %f fov %f\n",
	  //  setcfg.min_range, setcfg.max_range_anon, setcfg.fov );
	  
	  //stg_model_set_config( this->mod, &setcfg, sizeof(setcfg));
	  stg_model_set_property( this->mod, "fiducial_cfg", 
				  &setcfg, sizeof(setcfg)); 
	}    
      else
	PRINT_ERR2("Incorrect packet size setting fiducial FOV (%d/%d)",
		      (int)len, (int)sizeof(player_fiducial_fov_t) );      
      
      // deliberate no-break - SET_FOV needs the current FOV as a reply
      
    case PLAYER_FIDUCIAL_GET_FOV:
      {
	stg_fiducial_config_t *cfg = (stg_fiducial_config_t*)
	  stg_model_get_property_fixed( this->mod, "fiducial_cfg", sizeof(stg_fiducial_config_t));
	assert(cfg);

	// fill in the geometry data formatted player-like
	player_fiducial_fov_t pfov;
	pfov.min_range = htons((uint16_t)(1000.0 * cfg->min_range));
	pfov.max_range = htons((uint16_t)(1000.0 * cfg->max_range_anon));
	pfov.view_angle = htons((uint16_t)RTOD(cfg->fov));
	
	if( this->driver->PutReply(  this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				       &pfov, sizeof(pfov), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_FOV or PLAYER_FIDUCIAL_SET_FOV");      
      }
      break;
      
    case PLAYER_FIDUCIAL_SET_ID:
      
      if( len == sizeof(player_fiducial_id_t) )
	{
	  int id = ntohl(((player_fiducial_id_t*)src)->id);
	  
	  stg_model_set_property( this->mod, "fidicial_return", &id, sizeof(id));
	}
      else
	PRINT_ERR2("Incorrect packet size setting fiducial ID (%d/%d)",
		     (int)len, (int)sizeof(player_fiducial_id_t) );      
      
      // deliberate no-break - SET_ID needs the current ID as a reply

  case PLAYER_FIDUCIAL_GET_ID:
      {
	stg_fiducial_return_t* ret = (stg_fiducial_return_t*) 
	  stg_model_get_property_fixed( this->mod, 
					"fidicial_return",
					sizeof(stg_fiducial_return_t));

	// fill in the data formatted player-like
	player_fiducial_id_t pid;
	pid.id = htonl((int)*ret);
	
	if( this->driver->PutReply(  this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				       &pid, sizeof(pid), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_ID or PLAYER_FIDUCIAL_SET_ID");      
      }
      break;
      
      
    default:
      {
	PRINT_WARN1( "Warning: stg_fiducial doesn't support config id %d\n", buf[0] );
        if ( this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0) 
          DRIVER_ERROR("PutReply() failed");
      }
    }  
}


// 
// LASER INTERFACE
//

InterfaceLaser::InterfaceLaser( player_device_id_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, STG_MODEL_LASER )
{
  this->data_len = sizeof(player_laser_data_t);
  this->cmd_len = 0;
}

void InterfaceLaser::Publish( void )
{
  size_t len = 0;
  stg_laser_sample_t* samples = (stg_laser_sample_t*)
    stg_model_get_property( this->mod, "laser_data", &len );
  
  int sample_count = len / sizeof( stg_laser_sample_t );
  
  //for( int i=0; i<sample_count; i++ )
  //  printf( "rrrange %d %d\n", i, samples[i].range);
  
  player_laser_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  stg_laser_config_t *cfg = (stg_laser_config_t*) 
    stg_model_get_property_fixed( this->mod, "laser_cfg", sizeof(stg_laser_config_t));
  assert(cfg);
  
  if( sample_count != cfg->samples )
    {
      //PRINT_ERR2( "bad laser data: got %d/%d samples",
      //	  sample_count, cfg->samples );
    }
  else
    {      
      double min_angle = -RTOD(cfg->fov/2.0);
      double max_angle = +RTOD(cfg->fov/2.0);
      double angular_resolution = RTOD(cfg->fov / cfg->samples);
      double range_multiplier = 1; // TODO - support multipliers
      
      pdata.min_angle = htons( (int16_t)(min_angle * 100 )); // degrees / 100
      pdata.max_angle = htons( (int16_t)(max_angle * 100 )); // degrees / 100
      pdata.resolution = htons( (uint16_t)(angular_resolution * 100)); // degrees / 100
      pdata.range_res = htons( (uint16_t)range_multiplier);
      pdata.range_count = htons( (uint16_t)cfg->samples );
      
      for( int i=0; i<cfg->samples; i++ )
	{
	  //printf( "range %d %d\n", i, samples[i].range);
	  
	  pdata.ranges[i] = htons( (uint16_t)(samples[i].range) );
	  pdata.intensity[i] = (uint8_t)samples[i].reflectance;
	}
      
      // Write laser data
      this->driver->PutData( this->id, &pdata, sizeof(pdata), NULL);
    }

  //return 0;
}

void InterfaceLaser::Configure( void* client, void* buffer, size_t len )
{
  printf("got laser request\n");
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  
     case PLAYER_LASER_SET_CONFIG:
      {
	player_laser_config_t* plc = (player_laser_config_t*)buffer;
	
        if( len == sizeof(player_laser_config_t) )
	  {
	    int min_a = (int16_t)ntohs(plc->min_angle);
	    int max_a = (int16_t)ntohs(plc->max_angle);
	    int ang_res = (int16_t)ntohs(plc->resolution);
	    // TODO
	    // int intensity = plc->intensity;
	    
	    //PRINT_DEBUG4( "requested laser config:\n %d %d %d %d",
	    //	  min_a, max_a, ang_res, intensity );
	    
	    min_a /= 100;
	    max_a /= 100;
	    ang_res /= 100;
	    
	    stg_laser_config_t *current = (stg_laser_config_t*)
	      stg_model_get_property_fixed( this->mod, "laser_cfg", 
					    sizeof(stg_laser_config_t));
	    assert( current );
	    
	    stg_laser_config_t slc;
	    // copy the existing config
	    memcpy( &slc, current, sizeof(slc));
	    
	    // tweak the parts that player knows about
	    slc.fov = DTOR(max_a - min_a);
	    slc.samples = (int)(slc.fov / DTOR(ang_res));
	    
	    PRINT_DEBUG2( "setting laser config: fov %.2f samples %d", 
			  slc.fov, slc.samples );
	    
	    stg_model_set_property( this->mod, "laser_cfg", 
				  &slc, sizeof(slc)); 
	    
	    if(this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, plc, len, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", 
	           (int)len, (int)sizeof(player_laser_config_t));

	    if( this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
      }
      break;
      
    case PLAYER_LASER_GET_CONFIG:
      {   
	if( len == 1 )
	  {
	    stg_laser_config_t *slc = (stg_laser_config_t*) 
	      stg_model_get_property_fixed( this->mod, "laser_cfg", 
					    sizeof(stg_laser_config_t));
	    assert(slc);

	    // integer angles in degrees/100
	    int16_t  min_angle = (int16_t)(-RTOD(slc->fov/2.0) * 100);
	    int16_t  max_angle = (int16_t)(+RTOD(slc->fov/2.0) * 100);
	    //uint16_t angular_resolution = RTOD(slc->fov / slc->samples) * 100;
	    uint16_t angular_resolution =(uint16_t)(RTOD(slc->fov / (slc->samples-1)) * 100);
	    uint16_t range_multiplier = 1; // TODO - support multipliers
	    
	    int intensity = 1; // todo
	    
	    //printf( "laser config:\n %d %d %d %d\n",
	    //	    min_angle, max_angle, angular_resolution, intensity );
	    
	    player_laser_config_t plc;
	    memset(&plc,0,sizeof(plc));
	    plc.min_angle = htons(min_angle); 
	    plc.max_angle = htons(max_angle); 
	    plc.resolution = htons(angular_resolution);
	    plc.range_res = htons(range_multiplier);
	    plc.intensity = intensity;

	    if(this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, &plc, 
					 sizeof(plc), NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");      
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", (int)len, 1);
	    if( this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");
	  }
      }
      break;

    case PLAYER_LASER_GET_GEOM:
      {	
	stg_geom_t geom;
	stg_model_get_geom( this->mod, &geom );
	
	PRINT_DEBUG5( "received laser geom: %.2f %.2f %.2f -  %.2f %.2f",
		      geom.pose.x, 
		      geom.pose.y, 
		      geom.pose.a, 
		      geom.size.x, 
		      geom.size.y ); 
	
	// fill in the geometry data formatted player-like
	player_laser_geom_t pgeom;
        pgeom.subtype = PLAYER_LASER_GET_GEOM;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 

	if( this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				      &pgeom, sizeof(pgeom), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
      }
      break;
      
    default:
      {
	PRINT_WARN1( "stg_laser doesn't support config id %d", buf[0] );
        if (this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	  DRIVER_ERROR("PutReply() failed");
        break;
      }
      
    }
}

//
// GRIPPER INTERFACE
//

void GripperCommand( Interface* device, void* src, size_t len )
{
  if( len == sizeof(player_gripper_cmd_t) )
    {
      player_gripper_cmd_t* pcmd = (player_gripper_cmd_t*)src;
      //printf("GripperCommand: %d\n", pcmd->cmd);
      // Pass it to stage:
      stg_gripper_cmd_t cmd; 
      cmd.cmd = STG_GRIPPER_CMD_NOP;
      cmd.arg = 0;

      switch( pcmd->cmd )
	{
	case GRIPclose: cmd.cmd = STG_GRIPPER_CMD_CLOSE; break;
	case GRIPopen:  cmd.cmd = STG_GRIPPER_CMD_OPEN; break;
	case LIFTup:    cmd.cmd = STG_GRIPPER_CMD_UP; break;
	case LIFTdown:  cmd.cmd = STG_GRIPPER_CMD_DOWN; break;
	  //default:
	  //printf( "Stage: player gripper command %d is not implemented\n", 
	  //  pcmd->cmd );
	}

      //cmd.cmd  = pcmd->cmd;
      cmd.arg = pcmd->arg;

      //stg_model_set_command( device->mod, &cmd, sizeof(cmd) ) ;
      stg_model_set_property( device->mod, "gripper_cmd", &cmd, sizeof(cmd));
    }
  else
    PRINT_ERR2( "wrong size gripper command packet (%d/%d bytes)",
		(int)len, (int)sizeof(player_position_cmd_t) );
}

void GripperData( Interface* device, void* data, size_t len )
{
  //puts( "publishing gripper data\n" );
  
  if( len == sizeof(stg_gripper_data_t) )
    {
      stg_gripper_data_t* sdata = (stg_gripper_data_t*)data;

      player_gripper_data_t pdata;
      memset( &pdata, 0, sizeof(pdata) );
            
      // set the proper bits
      
      pdata.beams = 0;
      pdata.beams |=  sdata->outer_break_beam ? 0x04 : 0x00;
      pdata.beams |=  sdata->inner_break_beam ? 0x08 : 0x00;
      
      pdata.state = 0;  
      pdata.state |= (sdata->paddles == STG_GRIPPER_PADDLE_OPEN) ? 0x01 : 0x00;
      pdata.state |= (sdata->paddles == STG_GRIPPER_PADDLE_CLOSED) ? 0x02 : 0x00;
      pdata.state |= ((sdata->paddles == STG_GRIPPER_PADDLE_OPENING) ||
		      (sdata->paddles == STG_GRIPPER_PADDLE_CLOSING))  ? 0x04 : 0x00;
      
      pdata.state |= (sdata->lift == STG_GRIPPER_LIFT_UP) ? 0x10 : 0x00;
      pdata.state |= (sdata->lift == STG_GRIPPER_LIFT_DOWN) ? 0x20 : 0x00;
      pdata.state |= ((sdata->lift == STG_GRIPPER_LIFT_UPPING) ||
		      (sdata->lift == STG_GRIPPER_LIFT_DOWNING))  ? 0x040 : 0x00;
      
      //pdata.state |= sdata->lift_error ? 0x80 : 0x00;
      //pdata.state |= sdata->gripper_error ? 0x08 : 0x00;

      device->driver->PutData( device->id, &pdata, sizeof(pdata), NULL); 
    }
}

void GripperConfig( Interface* device, void* client, void* buffer, size_t len )
{
  printf("got gripper request\n");
}


//
// SONAR INTERFACE
//

InterfaceSonar::InterfaceSonar( player_device_id_t id, 
				StgDriver* driver,
				ConfigFile* cf,
				int section )
  : InterfaceModel( id, driver, cf, section, STG_MODEL_RANGER )
{
  this->data_len = sizeof(player_sonar_data_t);
  this->cmd_len = 0;
}

void InterfaceSonar::Publish( void )
{
  player_sonar_data_t sonar;
  memset( &sonar, 0, sizeof(sonar) );
  
  size_t len=0;
  stg_ranger_sample_t* rangers = (stg_ranger_sample_t*)
    stg_model_get_property( mod, "ranger_data", &len );

  if( len > 0 )
    {      
      size_t rcount = len / sizeof(stg_ranger_sample_t);
      
      // limit the number of samples to Player's maximum
      if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	rcount = PLAYER_SONAR_MAX_SAMPLES;
      
      //if( son->power_on ) // set with a sonar config
      {
	sonar.range_count = htons((uint16_t)rcount);
	
	for( int i=0; i<(int)rcount; i++ )
	  sonar.ranges[i] = htons((uint16_t)(1000.0*rangers[i].range));
      }
    }
  
  this->driver->PutData( this->id, &sonar, sizeof(sonar), NULL); 
}


void InterfaceSonar::Configure( void* client, void* src, size_t len )
{
  //printf("got sonar request\n");
  
 // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_SONAR_GET_GEOM_REQ:
      { 
	size_t cfglen = 0;
	stg_ranger_config_t* cfgs = (stg_ranger_config_t*)
	  stg_model_get_property( this->mod, "ranger_cfg", &cfglen );
	assert( cfgs );
	
	size_t rcount = cfglen / sizeof(stg_ranger_config_t);
	
	// convert the ranger data into Player-format sonar poses	
	player_sonar_geom_t pgeom;
	memset( &pgeom, 0, sizeof(pgeom) );
	
	pgeom.subtype = PLAYER_SONAR_GET_GEOM_REQ;
	
	// limit the number of samples to Player's maximum
	if( rcount > PLAYER_SONAR_MAX_SAMPLES )
	  rcount = PLAYER_SONAR_MAX_SAMPLES;

	pgeom.pose_count = htons((uint16_t)rcount);

	for( int i=0; i<(int)rcount; i++ )
	  {
	    // fill in the geometry data formatted player-like
	    pgeom.poses[i][0] = 
	      ntohs((uint16_t)(1000.0 * cfgs[i].pose.x));
	    
	    pgeom.poses[i][1] = 
	      ntohs((uint16_t)(1000.0 * cfgs[i].pose.y));
	    
	    pgeom.poses[i][2] 
	      = ntohs((uint16_t)RTOD( cfgs[i].pose.a));	    
	  }

	  if(this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, 
	  &pgeom, sizeof(pgeom), NULL ) )
	  DRIVER_ERROR("failed to PutReply");
      }
      break;
      
//     case PLAYER_SONAR_POWER_REQ:
//       /*
// 	 * 1 = enable sonars
// 	 * 0 = disable sonar
// 	 */
// 	if( len != sizeof(player_sonar_power_config_t))
// 	  {
// 	    PRINT_WARN2( "stg_sonar: arg to sonar state change "
// 			  "request wrong size (%d/%d bytes); ignoring",
// 			  (int)len,(int)sizeof(player_sonar_power_config_t) );
	    
// 	    if(PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL ))
// 	      DRIVER_ERROR("failed to PutReply");
// 	  }
	
// 	this->power_on = ((player_sonar_power_config_t*)src)->value;
	
// 	if(PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL ))
// 	  DRIVER_ERROR("failed to PutReply");
	
// 	break;
	
    default:
      {
	PRINT_WARN1( "stage sonar model doesn't support config id %d\n", buf[0] );
	if (this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	  DRIVER_ERROR("PutReply() failed");
	break;
      }      
    }
}

//
// ENERGY INTERFACE
//

void EnergyConfig( Interface* device, 
		   void* client, 
		   void* src, 
		   size_t len )
{
  //printf("got energy request\n");
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    default:
      {
	PRINT_WARN1( "stage energy model doesn't support config id %d\n", buf[0] );
	if( device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL)
	    != 0)
	  DRIVER_ERROR("PutReply() failed");
	break;
      }      
    }
}

void EnergyData( Interface* device, void* data, size_t len )
{  
  if( len == sizeof(stg_energy_data_t) )
    {
      stg_energy_data_t* edata = (stg_energy_data_t*)data;
      
      // translate stage data to player data packet
      player_energy_data_t pen;
      pen.mjoules = htonl( (int32_t)(edata->stored * 1000.0));
      pen.mwatts  = htonl( (int32_t)(edata->output_watts * 1000.0));
      pen.charging = (uint8_t)( edata->charging );      

      device->driver->PutData( device->id, &pen, sizeof(pen), NULL); 
    }
  else
    PRINT_ERR2( "energy device returned wrong size data (%d/%d)", 
		(int)len, (int)sizeof(stg_energy_data_t));
}



// POSITION INTERFACE -----------------------------------------------------------------------------

//int PositionData( stg_model_t* mod, char* name, void* data, size_t len, void* userp )
void InterfacePosition::Publish( void )
{
  //puts( "publishing position data" ); 
  
  //Interface* device = (Interface*)userp;
  
  stg_position_data_t* stgdata = (stg_position_data_t*)
    stg_model_get_property_fixed( this->mod, "position_data", 
				  sizeof(stg_position_data_t ));
  
  //if( len == sizeof(stg_position_data_t) )      
  //  {      
      //stg_position_data_t* stgdata = (stg_position_data_t*)data;
                  
      //PLAYER_MSG3( "get data data %.2f %.2f %.2f", stgdatax, stgdatay, stgdataa );
      player_position_data_t position_data;
      memset( &position_data, 0, sizeof(position_data) );

      // pack the data into player format
      position_data.xpos = ntohl((int32_t)(1000.0 * stgdata->pose.x));
      position_data.ypos = ntohl((int32_t)(1000.0 * stgdata->pose.y));
      position_data.yaw = ntohl((int32_t)(RTOD(stgdata->pose.a)));
      
      // speeds
      position_data.xspeed= ntohl((int32_t)(1000.0 * stgdata->velocity.x)); // mm/sec
      position_data.yspeed = ntohl((int32_t)(1000.0 * stgdata->velocity.y));// mm/sec
      position_data.yawspeed = ntohl((int32_t)(RTOD(stgdata->velocity.a))); // deg/sec
      
      // etc
      position_data.stall = stgdata->stall;// ? 1 : 0;
      
      // publish this data
      this->driver->PutData( this->id, &position_data, sizeof(position_data), NULL);      
      //}
      //else
      //PRINT_ERR2( "wrong size position data (%d/%d bytes)",
      //	(int)len, (int)sizeof(player_position_data_t) );
}


InterfacePosition::InterfacePosition(  player_device_id_t id, 
				       StgDriver* driver,
				       ConfigFile* cf,
				       int section )
						   
  : InterfaceModel( id, driver, cf, section, STG_MODEL_POSITION )
{
  //puts( "InterfacePosition constructor" );
  this->data_len = sizeof(player_position_data_t);
  this->cmd_len = sizeof(player_position_cmd_t);  
  //stg_model_add_property_callback( this->mod, "position_data", PositionData, this );  
}

void InterfacePosition::Command( void* src, size_t len )
{
  if( len == sizeof(player_position_cmd_t) )
    {
      // convert from Player to Stage format
      player_position_cmd_t* pcmd = (player_position_cmd_t*)src;
      
      // only velocity control mode works yet
      stg_position_cmd_t cmd; 
      cmd.x = ((double)((int32_t)ntohl(pcmd->xspeed))) / 1000.0;
      cmd.y = ((double)((int32_t)ntohl(pcmd->yspeed))) / 1000.0;
      cmd.a = DTOR((double)((int32_t)ntohl(pcmd->yawspeed)));
      cmd.mode = STG_POSITION_CONTROL_VELOCITY;
      
      // TODO
      // only set the command if it's different from the old one
      // (saves aquiring a mutex lock from the sim thread)
            
      //if( memcmp( &cmd, buf, sizeof(cmd) ))
	{	  
	  //static int g=0;
	  //printf( "setting command %d\n", g++ );
	  //stg_model_set_command( device->mod, &cmd, sizeof(cmd) ) ;
	  stg_model_set_property( this->mod, "position_cmd", &cmd, sizeof(cmd));
	}
    }
  else
    PRINT_ERR2( "wrong size position command packet (%d/%d bytes)",
		(int)len, (int)sizeof(player_position_cmd_t) );
}


void InterfacePosition::Configure( void* client, void* buffer, size_t len  )
{
  //printf("got position request\n");
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  
    case PLAYER_POSITION_GET_GEOM_REQ:
      {
	stg_geom_t geom;
	stg_model_get_geom( this->mod,&geom );

	// fill in the geometry data formatted player-like
	player_position_geom_t pgeom;
	pgeom.subtype = PLAYER_POSITION_GET_GEOM_REQ;
	pgeom.pose[0] = ntohs((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = ntohs((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = ntohs((uint16_t)RTOD(geom.pose.a));
	
	pgeom.size[0] = ntohs((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = ntohs((uint16_t)(1000.0 * geom.size.y)); 
	
	this->driver->PutReply( this->id, client, 
				PLAYER_MSGTYPE_RESP_ACK, 
				&pgeom, sizeof(pgeom), NULL );
      }
      break;
      
    case PLAYER_POSITION_RESET_ODOM_REQ:
      {
	PRINT_DEBUG( "resetting odometry" );
	
	stg_pose_t origin;
	memset(&origin,0,sizeof(origin));
	stg_model_position_set_odom( this->mod, &origin );
	
	this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_SET_ODOM_REQ:
      {
	player_position_set_odom_req_t* req = 
	  (player_position_set_odom_req_t*)buffer;
	
	PRINT_DEBUG3( "setting odometry to (%.d,%d,%.2f)",
		      (int32_t)ntohl(req->x),
		      (int32_t)ntohl(req->y),
		      DTOR((int32_t)ntohl(req->theta)) );
	
	stg_pose_t pose;
	pose.x = ((double)(int32_t)ntohl(req->x)) / 1000.0;
	pose.y = ((double)(int32_t)ntohl(req->y)) / 1000.0;
	pose.a = DTOR( (double)(int32_t)ntohl(req->theta) );

	stg_model_position_set_odom( this->mod, &pose );
	
	PRINT_DEBUG3( "set odometry to (%.2f,%.2f,%.2f)",
		      pose.x,
		      pose.y,
		      pose.a );

	this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_MOTOR_POWER_REQ:
      {
	player_position_power_config_t* req = 
	  (player_position_power_config_t*)buffer;

	int motors_on = req->value;

	PRINT_WARN1( "Stage ignores motor power state (%d)",
		     motors_on );
	this->driver->PutReply( this->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    default:
      {
	PRINT_WARN1( "stg_position doesn't support config id %d", buf[0] );
        if( this->driver->PutReply( this->id, 
				    client, 
				    PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
          DRIVER_ERROR("PutReply() failed");
        break;
      }
    }
}


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
 * CVS: $Id: player_interfaces.cc,v 1.3 2005-03-05 00:20:53 rtv Exp $
 */

// DOCUMENTATION ------------------------------------------------------------

// TODO - configs I should implement
//  - PLAYER_SONAR_POWER_REQ
//  - PLAYER_BLOBFINDER_SET_COLOR_REQ
//  - PLAYER_BLOBFINDER_SET_IMAGER_PARAMS_REQ

// CODE ------------------------------------------------------------

#define DEBUG

#include "player_interfaces.h"

#define DRIVER_ERROR(X) printf( "Stage driver error: %s\n", X )



// 
// SIMULATION INTERFACE
//

void SimulationData( device_record_t* device, void* data, size_t len )
{
  PRINT_WARN( "refresh data requested for simulation device - devices produces no data" );
  device->driver->PutData( device->id, NULL, 0, NULL); 
}

void SimulationConfig(  device_record_t* device,
			void* client, 
			void* buffer, size_t len )
{
  printf("got simulation request\n");
  
  // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  
      
      // if Player has defined this config, implement it
#ifdef PLAYER_SIMULATION_SET_POSE2D
    case PLAYER_SIMULATION_SET_POSE2D:
      {
	player_simulation_pose2d_req_t* req = (player_simulation_pose2d_req_t*)buffer;
	
	stg_pose_t pose;
	pose.x = (int32_t)ntohl(req->x) / 1000.0;
	pose.y = (int32_t)ntohl(req->y) / 1000.0;
	pose.a = DTOR( ntohl(req->a) );
	
	printf( "Stage: received request to move object \"%s\" to (%.2f,%.2f,%.2f)\n",
		req->name, pose.x, pose.y, pose.a );
	
	// look up the named model
	
	stg_model_t* mod = 
	  stg_world_model_name_lookup( StgDriver::world, req->name );
 	
	if( mod )
	  {
	    // move it 
	    // printf( "moving model \"%s\"", req->name );	    
	    stg_model_set_pose( mod, &pose );  
	    device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
	  }
	else
	  {
	    PRINT_WARN1( "simulation model \"%s\" not found", req->name );
	    device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL );
	  }
      }      
      break;
#endif
          // if Player has defined this config, implement it
#ifdef PLAYER_SIMULATION_GET_POSE2D
    case PLAYER_SIMULATION_GET_POSE2D:
      {
	player_simulation_pose2d_req_t* req = (player_simulation_pose2d_req_t*)buffer;
	
	printf( "Stage: received request for position of object \"%s\"\n", req->name );
	
	// look up the named model	
	stg_model_t* mod = 
	  stg_world_model_name_lookup( StgDriver::world, req->name );
 	
	if( mod )
	  {
	    stg_pose_t pose;
	    stg_model_get_pose( mod, &pose );
	    
	    printf( "Stage: returning location (%.2f,%.2f,%.2f)\n",
		    pose.x, pose.y, pose.a );
	    
	    player_simulation_pose2d_req_t reply;
	    memcpy( &reply, req, sizeof(reply));
	    reply.x = htonl((int32_t)(pose.x*1000.0));
	    reply.y = htonl((int32_t)(pose.y*1000.0));
	    reply.a = htonl((int32_t)RTOD(pose.a));
	    
	    printf( "Stage: returning location (%d %d %d)\n",
		    reply.x, reply.y, reply.a );

	    device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				      &reply, sizeof(reply),  NULL );
	  }
	else
	  {
	    PRINT_WARN1( "Stage: simulation model \"%s\" not found", req->name );
	    device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL );
	  }
      }      
      break;
#endif
      
    default:      
      PRINT_WARN1( "Stage: simulation device doesn't implement config code %d.", buf[0] );
      if (device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	DRIVER_ERROR("PutReply() failed");  
      break;
    }
}

// 
// BLOBFINDER INTERFACE
//

void BlobfinderData( device_record_t* device, void* data, size_t len )
{
  stg_blobfinder_blob_t* blobs = (stg_blobfinder_blob_t*)data;
  
  if( len == 0 )
    {
      device->driver->PutData( device->id, NULL, 0, NULL);
    }
  else
    {
      size_t bcount = len / sizeof(stg_blobfinder_blob_t);
      
      // limit the number of samples to Player's maximum
      if( bcount > PLAYER_BLOBFINDER_MAX_BLOBS )
	bcount = PLAYER_BLOBFINDER_MAX_BLOBS;      
     
      player_blobfinder_data_t bfd;
      memset( &bfd, 0, sizeof(bfd) );
      
      // get the configuration
      stg_blobfinder_config_t cfg;      
      assert( stg_model_get_config( device->mod, &cfg, sizeof(cfg) ) == sizeof(cfg) );
      
      // and set the image width * height
      bfd.width = htons((uint16_t)cfg.scan_width);
      bfd.height = htons((uint16_t)cfg.scan_height);
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

      //if( bf->ready )
	{
	  size_t size = sizeof(bfd) - sizeof(bfd.blobs) + bcount * sizeof(bfd.blobs[0]);
	  
	  //PRINT_WARN1( "blobfinder putting %d bytes of data", size );
	  device->driver->PutData( device->id, &bfd, size, NULL);
	}
    }
}

void BlobfinderConfig( device_record_t* device, void* client, void* buffer, size_t len )
{
  printf("got blobfinder request\n");
  
  if (device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
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

void MapData( device_record_t* device, void* data, size_t len )
{
  //PRINT_WARN( "someone subscribed to the map interface, but we don't generate any data" );
  device->driver->PutData( device->id, NULL, 0, NULL); 
}

// Handle map info request - adapted from Player's mapfile driver by
// Brian Gerkey
void  MapConfigInfo( device_record_t* device,  
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
  stg_map_info_t* minfo = (stg_map_info_t*)stg_model_get_prop( device->mod, "_map" );
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
  
  size_t count=0;
  stg_polygon_t* polys = stg_model_get_polygons( device->mod, &count);
  
  if( polys && count ) // if we have something to render
    {
      
      // we'll use a temporary matrix, as it knows how to render objects
      
      stg_matrix_t* matrix = stg_matrix_create( minfo->ppm, 1.0, 1.0 );
      
      if( count > 0 ) 
	{
	  // render the model into the matrix
	  stg_matrix_polygons( matrix, 
			       geom.size.x/2.0,
			       geom.size.y/2.0,
			       0,
			       polys, count, 
			       device->mod, 1 );  
	  
	  if( device->mod->guifeatures.boundary )    
	    stg_matrix_rectangle( matrix,
				  geom.size.x/2.0,
				  geom.size.y/2.0,
				  0,
				  geom.size.x,
				  geom.size.y,
				  device->mod, 1 );
	}
      
      // Now convert the matrix to a Player occupancy grid. if the
      // matrix contains anything in a cell, we set that map cell to
      // be occupied, else unoccupied
      
      for( unsigned int p=0; p<minfo->width; p++ )
	for( unsigned int q=0; q<minfo->height; q++ )
	  {
	    //printf( "%d,%d \n", p,q );
	    GPtrArray* pa = stg_table_cell( matrix->table, p, q );
	    minfo->data[ q * minfo->width + p ] =  pa ? 1 : -1;
	  }
      
      // we're done with the matrix
      stg_matrix_destroy( matrix );
    }

  puts( "done." );

  // store the map as a model property named "_map"
  stg_model_set_prop( device->mod, "_map", (void*)minfo );

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
void MapConfigData( device_record_t* device,  
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
  
  stg_map_info_t* minfo = NULL;
  assert( (minfo=(stg_map_info_t*)stg_model_get_prop( device->mod, "_map" )));
  
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


void MapConfig( device_record_t* device, void* client, void* buffer, size_t len )
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

void FiducialData( device_record_t* device, void* data, size_t len )
{
  player_fiducial_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  if( len > 0 )
    {
      stg_fiducial_t* fids = (stg_fiducial_t*)data;
      
      size_t fcount = len / sizeof(stg_fiducial_t);      
      assert( fcount > 0 );
      
      pdata.count = htons((uint16_t)fcount);
      
      //printf( "reporting %d fiducials\n",
      //      fcount );

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
  device->driver->PutData( device->id, &pdata, sizeof(pdata), NULL);
}

void FiducialConfig( device_record_t* device, void* client, void* src, size_t len  )
{
  printf("got fiducial request\n");
   // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_FIDUCIAL_GET_GEOM:
      {	
	// just get the model's geom - Stage doesn't have separate
	// fiducial geom (yet)
	stg_geom_t geom;
	stg_model_get_geom(device->mod,&geom);
	
	// fill in the geometry data formatted player-like
	player_fiducial_geom_t pgeom;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 
	
	pgeom.fiducial_size[0] = ntohs((uint16_t)100); // TODO - get this info
	pgeom.fiducial_size[1] = ntohs((uint16_t)100);
	
	if( device->driver->PutReply(  device->id, client, PLAYER_MSGTYPE_RESP_ACK,  
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
	  
	  stg_model_set_config( device->mod, &setcfg, sizeof(setcfg));
	}    
      else
	PRINT_ERR2("Incorrect packet size setting fiducial FOV (%d/%d)",
		      (int)len, (int)sizeof(player_fiducial_fov_t) );      
      
      // deliberate no-break - SET_FOV needs the current FOV as a reply
      
    case PLAYER_FIDUCIAL_GET_FOV:
      {
	stg_fiducial_config_t cfg;
	assert( stg_model_get_config( device->mod, &cfg, sizeof(stg_fiducial_config_t))
		== sizeof(stg_fiducial_config_t) );
	
	// fill in the geometry data formatted player-like
	player_fiducial_fov_t pfov;
	pfov.min_range = htons((uint16_t)(1000.0 * cfg.min_range));
	pfov.max_range = htons((uint16_t)(1000.0 * cfg.max_range_anon));
	pfov.view_angle = htons((uint16_t)RTOD(cfg.fov));
	
	if( device->driver->PutReply(  device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				       &pfov, sizeof(pfov), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_FOV or PLAYER_FIDUCIAL_SET_FOV");      
      }
      break;
      
    case PLAYER_FIDUCIAL_SET_ID:
      
      if( len == sizeof(player_fiducial_id_t) )
	{
	  int id = ntohl(((player_fiducial_id_t*)src)->id);

	  stg_model_set_fiducialreturn( device->mod, &id );
	}
      else
	PRINT_ERR2("Incorrect packet size setting fiducial ID (%d/%d)",
		     (int)len, (int)sizeof(player_fiducial_id_t) );      
      
      // deliberate no-break - SET_ID needs the current ID as a reply

  case PLAYER_FIDUCIAL_GET_ID:
      {
	stg_fiducial_return_t ret;
	stg_model_get_fiducialreturn(device->mod,&ret); 

	// fill in the data formatted player-like
	player_fiducial_id_t pid;
	pid.id = htonl((int)ret);
	
	if( device->driver->PutReply(  device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				       &pid, sizeof(pid), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for "
		       "PLAYER_FIDUCIAL_GET_ID or PLAYER_FIDUCIAL_SET_ID");      
      }
      break;
      
      
    default:
      {
	PRINT_WARN1( "Warning: stg_fiducial doesn't support config id %d\n", buf[0] );
        if ( device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0) 
          DRIVER_ERROR("PutReply() failed");
      }
    }  
}

// 
// LASER INTERFACE
//

void LaserData( device_record_t* device, void* data, size_t len )
{
  //puts( "publishing laser data" );
  
  
  int sample_count = len / sizeof( stg_laser_sample_t );
  
  //for( int i=0; i<sample_count; i++ )
  //  printf( "rrrange %d %d\n", i, samples[i].range);
  
  player_laser_data_t pdata;
  memset( &pdata, 0, sizeof(pdata) );
  
  stg_laser_config_t cfg;
  assert( stg_model_get_config( device->mod, &cfg, sizeof(cfg)) == sizeof(cfg));
  
  if( sample_count != cfg.samples )
    {
      //PRINT_ERR2( "bad laser data: got %d/%d samples",
      //	  sample_count, cfg.samples );
    }
  else
    {      
      stg_laser_sample_t* samples = (stg_laser_sample_t*)data;
      
      double min_angle = -RTOD(cfg.fov/2.0);
      double max_angle = +RTOD(cfg.fov/2.0);
      double angular_resolution = RTOD(cfg.fov / cfg.samples);
      double range_multiplier = 1; // TODO - support multipliers
      
      pdata.min_angle = htons( (int16_t)(min_angle * 100 )); // degrees / 100
      pdata.max_angle = htons( (int16_t)(max_angle * 100 )); // degrees / 100
      pdata.resolution = htons( (uint16_t)(angular_resolution * 100)); // degrees / 100
      pdata.range_res = htons( (uint16_t)range_multiplier);
      pdata.range_count = htons( (uint16_t)cfg.samples );
      
      for( int i=0; i<cfg.samples; i++ )
	{
	  //printf( "range %d %d\n", i, samples[i].range);
	  
	  pdata.ranges[i] = htons( (uint16_t)(samples[i].range) );
	  pdata.intensity[i] = (uint8_t)samples[i].reflectance;
	}
      
      // Write laser data
      device->driver->PutData( device->id, &pdata, sizeof(pdata), NULL);
    }
}

void LaserConfig( device_record_t* device, void* client, void* buffer, size_t len )
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
	    
	    stg_laser_config_t current;
	    assert( stg_model_get_config( device->mod, &current, sizeof(current))
		     == sizeof(current));
	    
	    stg_laser_config_t slc;
	    // copy the existing config
	    memcpy( &slc, &current, sizeof(slc));
	    
	    // tweak the parts that player knows about
	    slc.fov = DTOR(max_a - min_a);
	    slc.samples = (int)(slc.fov / DTOR(ang_res));
	    
	    PRINT_DEBUG2( "setting laser config: fov %.2f samples %d", 
			  slc.fov, slc.samples );
	    
	    stg_model_set_config( device->mod, &slc, sizeof(slc));
	    
	    if(device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, plc, len, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", 
	           (int)len, (int)sizeof(player_laser_config_t));

	    if( device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_SET_CONFIG");
	  }
      }
      break;
      
    case PLAYER_LASER_GET_CONFIG:
      {   
	if( len == 1 )
	  {
	    stg_laser_config_t slc;
	    assert( stg_model_get_config( device->mod, &slc, sizeof(slc)) 
		    == sizeof(slc) );

	    // integer angles in degrees/100
	    int16_t  min_angle = (int16_t)(-RTOD(slc.fov/2.0) * 100);
	    int16_t  max_angle = (int16_t)(+RTOD(slc.fov/2.0) * 100);
	    //uint16_t angular_resolution = RTOD(slc.fov / slc.samples) * 100;
	    uint16_t angular_resolution =(uint16_t)(RTOD(slc.fov / (slc.samples-1)) * 100);
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

	    if(device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, &plc, 
					 sizeof(plc), NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");      
	  }
	else
	  {
	    PRINT_ERR2("config request len is invalid (%d != %d)", (int)len, 1);
	    if( device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	      DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_CONFIG");
	  }
      }
      break;

    case PLAYER_LASER_GET_GEOM:
      {	
	stg_geom_t geom;
	stg_model_get_geom( device->mod, &geom );
	
	PRINT_DEBUG5( "received laser geom: %.2f %.2f %.2f -  %.2f %.2f",
		      geom.pose.x, 
		      geom.pose.y, 
		      geom.pose.a, 
		      geom.size.x, 
		      geom.size.y ); 
	
	// fill in the geometry data formatted player-like
	player_laser_geom_t pgeom;
	pgeom.pose[0] = htons((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = htons((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = htons((uint16_t)RTOD( geom.pose.a));
	
	pgeom.size[0] = htons((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = htons((uint16_t)(1000.0 * geom.size.y)); 
	
	if( device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
				      &pgeom, sizeof(pgeom), NULL ) != 0 )
	  DRIVER_ERROR("PutReply() failed for PLAYER_LASER_GET_GEOM");      
      }
      break;
      
    default:
      {
	PRINT_WARN1( "stg_laser doesn't support config id %d", buf[0] );
        if (device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL, 0, NULL) != 0)
	  DRIVER_ERROR("PutReply() failed");
        break;
      }
      
    }
}

//
// SONAR INTERFACE
//

void SonarData( device_record_t* device, void* data, size_t len )
{
  player_sonar_data_t sonar;
  memset( &sonar, 0, sizeof(sonar) );
  
  if( len > 0 )
    {
      stg_ranger_sample_t* rangers = (stg_ranger_sample_t*)data;
      
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
      
  device->driver->PutData( device->id, &sonar, sizeof(sonar), NULL); 
}


void SonarConfig( device_record_t* device, void* client, void* src, size_t len )
{
  //printf("got sonar request\n");
  
 // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)src;
  switch( buf[0] )
    {  
    case PLAYER_SONAR_GET_GEOM_REQ:
      { 
	stg_ranger_config_t cfgs[100];
	size_t cfglen = 0;
	cfglen = stg_model_get_config( device->mod, cfgs, sizeof(cfgs[0])*100);
	
	//assert( cfglen == sizeof(cfgs[0])*100 );

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

	  if(device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, 
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
	if (device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
	  DRIVER_ERROR("PutReply() failed");
	break;
      }
      
    }
}

//
// ENERGY INTERFACE
//

void EnergyConfig( device_record_t* device, 
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

void EnergyData( device_record_t* device, void* data, size_t len )
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
		len, sizeof(stg_energy_data_t));
}


//
// POSITION INTERFACE 
//

void PositionCommand( device_record_t* device, void* src, size_t len )
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
      
      // we only set the command if it's different from the old one
      // (saves aquiring a mutex lock from the sim thread)
      
      char buf[32767];
      stg_model_get_command( device->mod, buf, 32767 );
      
      if( memcmp( &cmd, buf, sizeof(cmd) ))
	{	  
	  //static int g=0;
	  //printf( "setting command %d\n", g++ );
	  stg_model_set_command( device->mod, &cmd, sizeof(cmd) ) ;
	}
    }
  else
    PRINT_ERR2( "wrong size position command packet (%d/%d bytes)",
		(int)len, (int)sizeof(player_position_cmd_t) );
}


void PositionData( device_record_t* device, void* data, size_t len )
{
  //puts( "publishing position data" ); 
  
  if( len == sizeof(stg_position_data_t) )      
    {      
      stg_position_data_t* stgdata = (stg_position_data_t*)data;
                  
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
      device->driver->PutData( device->id, &position_data, sizeof(position_data), NULL);      
    }
  else
    PRINT_ERR2( "wrong size position data (%d/%d bytes)",
		(int)len, (int)sizeof(player_position_data_t) );
}

void PositionConfig( device_record_t* device, void* client, void* buffer, size_t len  )
{
  //printf("got position request\n");
  
 // switch on the config type (first byte)
  uint8_t* buf = (uint8_t*)buffer;
  switch( buf[0] )
    {  
    case PLAYER_POSITION_GET_GEOM_REQ:
      {
	stg_geom_t geom;
	stg_model_get_geom( device->mod,&geom );

	// fill in the geometry data formatted player-like
	player_position_geom_t pgeom;
	pgeom.pose[0] = ntohs((uint16_t)(1000.0 * geom.pose.x));
	pgeom.pose[1] = ntohs((uint16_t)(1000.0 * geom.pose.y));
	pgeom.pose[2] = ntohs((uint16_t)RTOD(geom.pose.a));
	
	pgeom.size[0] = ntohs((uint16_t)(1000.0 * geom.size.x)); 
	pgeom.size[1] = ntohs((uint16_t)(1000.0 * geom.size.y)); 
	
	device->driver->PutReply( device->id, client, 
		  PLAYER_MSGTYPE_RESP_ACK, 
		  &pgeom, sizeof(pgeom), NULL );
      }
      break;
      
    case PLAYER_POSITION_RESET_ODOM_REQ:
      {
	PRINT_DEBUG( "resetting odometry" );
	
	stg_pose_t origin;
	memset(&origin,0,sizeof(origin));
	stg_model_position_set_odom( device->mod, &origin );
	
	device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
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

	stg_model_position_set_odom( device->mod, &pose );
	
	PRINT_DEBUG3( "set odometry to (%.2f,%.2f,%.2f)",
		      pose.x,
		      pose.y,
		      pose.a );

	device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    case PLAYER_POSITION_MOTOR_POWER_REQ:
      {
	player_position_power_config_t* req = 
	  (player_position_power_config_t*)buffer;

	int motors_on = req->value;

	PRINT_WARN1( "Stage ignores motor power state (%d)",
		     motors_on );
	device->driver->PutReply( device->id, client, PLAYER_MSGTYPE_RESP_ACK, NULL );
      }
      break;

    default:
      {
	PRINT_WARN1( "stg_position doesn't support config id %d", buf[0] );
        if( device->driver->PutReply( device->id, 
				      client, 
				      PLAYER_MSGTYPE_RESP_NACK, NULL) != 0)
          DRIVER_ERROR("PutReply() failed");
        break;
      }
    }
}

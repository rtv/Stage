///////////////////////////////////////////////////////////////////////////
//
// File: fiducialfinderdevice.cc
// Author: Richard Vaughan
// Date: 12 Jan 2000
// Desc: Simulates the laser-based beacon detector
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_fiducial.c,v $
//  $Author: rtv $
//  $Revision: 1.1 $
//
// Usage: detects objects that were laser bright and had non-zero
// ficucial_return
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <math.h>
#include <stage1p3.h>
#include "world.hh"
#include "laserdevice.hh"
#include "fiducialfinderdevice.hh"

void model_add_prop( model_t* mod, stg_id_t propid, void* data, size_t len )
{
  g_hash_table_replace( mod->props, &propid, data, len );
}

void model_fiducial_init( model_t* mod )
{
  stg_fiducial_config_t* fid = calloc( sizeof(stg_fiducial_config_t), 1 );
  
  // add this property to the model
  model_add_prop( mod, STG_PROP_FIDUCIALCONFIG, fid,  sizeof(stg_fiducial_config_t);


  memset( fid, 0, sizeof(stg_fiducial_config_t) );
  
  fid->max_range_anon = 4.0;
  fid->max_range_id = 1.5;
  
  // fill in the default values for these fake parameters
  //fid->m_bit_count = 8;
  //fid->m_bit_size = 50;
	  //fid->m_zero_thresh = m_one_thresh = 60;
  
  
  /*
    mod->max_range_anon = worldfile->ReadLength(0, "lbd_range_anon",
    mod->max_range_anon);
    mod->max_range_anon = worldfile->ReadLength(section, "range_anon",
    mod->max_range_anon);
    mod->max_range_id = worldfile->ReadLength(0, "lbd_range_id",
    mod->max_range_id);
    mod->max_range_id = worldfile->ReadLength(section, "range_id",
    mod->max_range_id);
  */
}


///////////////////////////////////////////////////////////////////////////
// Update the beacon data
//
void model_fiducial_update( model_t* mod )
{
  stg_fiducial_config_t* fid = mod->fid_config;

  UpdateConfig();

}

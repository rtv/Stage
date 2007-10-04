#include "model.hh"

class StgModelLaser : public StgModel
{
public:
  // constructor
  StgModelLaser( StgWorld* world,
		 StgModel* parent, 
		 stg_id_t id, CWorldFile* wf );
  
  // destructor
  ~StgModelLaser( void );
  
  stg_laser_sample_t* samples;
  size_t sample_count;
  stg_meters_t range_min, range_max;
  stg_radians_t fov;
  unsigned int resolution;
  
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void Load( void );  
  virtual void Print( char* prefix );
  virtual void DListData( void );
};


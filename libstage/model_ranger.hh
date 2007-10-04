#include "model.hh"

class StgModelRanger : public StgModel
{
public:
  // constructor
  StgModelRanger( StgWorld* world,
	       StgModel* parent, 
	       stg_id_t id, CWorldFile* wf );
  
  // destructor
  virtual ~StgModelRanger( void );
  
  virtual void Startup( void );
  virtual void Shutdown( void );
  virtual void Update( void );
  virtual void Load( void );
  virtual void Print( char* prefix );
  virtual void DListData( void );

  size_t sensor_count;
  stg_ranger_sensor_t* sensors;
};

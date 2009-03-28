
#include "stage.hh"
using namespace Stg;

#define MATCH(A,B) (strcmp(A,B)== 0)

void* Model::GetProperty( const char* key ) const
{
	// see if the key has the predefined-property prefix
	if( strncmp( key, MP_PREFIX, strlen(MP_PREFIX)) == 0 )
	{
		if( MATCH( key, MP_COLOR))            return (void*)&color;
		if( MATCH( key, MP_MASS))             return (void*)&mass;
		if( MATCH( key, MP_WATTS))            return (void*)&watts;
		if( MATCH( key, MP_FIDUCIAL_RETURN))  return (void*)&vis.fiducial_return;
		if( MATCH( key, MP_LASER_RETURN))     return (void*)&vis.laser_return;
		if( MATCH( key, MP_OBSTACLE_RETURN))  return (void*)&vis.obstacle_return;
		if( MATCH( key, MP_RANGER_RETURN))    return (void*)&vis.ranger_return;
		if( MATCH( key, MP_GRIPPER_RETURN))   return (void*)&vis.gripper_return;

		PRINT_WARN1( "Requested non-existent model core property \"%s\"", key );
		return NULL;
	}

	// otherwise it may be an arbitrary named property
	return g_datalist_get_data( (GData**)&this->props, key ); // cast to discard const
}

int Model::SetProperty( const char* key,
								const void* data )
{
	// see if the key has the predefined-property prefix
	if( strncmp( key, MP_PREFIX, strlen(MP_PREFIX)) == 0 )
	{
		PRINT_DEBUG1( "Looking up model core property \"%s\"\n", key );

		if( MATCH(key,MP_FIDUCIAL_RETURN) )
		{
			this->SetFiducialReturn( *(int*)data );
			return 0;
		}
		if( MATCH( key, MP_LASER_RETURN ) )
		{
			this->SetLaserReturn( *(stg_laser_return_t*)data );
			return 0;
		}
		if( MATCH( key, MP_OBSTACLE_RETURN ) )
		{
			this->SetObstacleReturn( *(int*)data );
			return 0;
		}
		if( MATCH( key, MP_RANGER_RETURN ) )
		{
			this->SetRangerReturn( *(int*)data );
			return 0;
		}
		if( MATCH( key, MP_GRIPPER_RETURN ) )
		{
			this->SetGripperReturn( *(int*)data );
			return 0;
		}
		if( MATCH( key, MP_COLOR ) )
		{
			this->SetColor( *(int*)data );
			return 0;
		}
		if( MATCH( key, MP_MASS ) )
		{
			this->SetMass( *(double*)data );
			return 0;
		}
		if( MATCH( key, MP_WATTS ) )
		{
			this->SetWatts( *(double*)data );
			return 0;
		}

		PRINT_ERR1( "Attempt to set non-existent model core property \"%s\"", key );
		return 1; // error code
	}

	// otherwise it's an arbitary property and we store the pointer
	g_datalist_set_data( &this->props, key, (void*)data );
	return 0; // ok
}


void Model::UnsetProperty( const char* key )
{
	if( strncmp( key, MP_PREFIX, strlen(MP_PREFIX)) == 0 )
		PRINT_WARN1( "Attempt to unset a model core property \"%s\" has no effect", key );
	else
		g_datalist_remove_data( &this->props, key );
}


bool Model::GetPropertyFloat( const char* key, float* f, float defaultval ) const
{ 
  float* fp = (float*)GetProperty( key ); 
  if( fp )
	 {
		*f = *fp;
		return true;
	 }
  
  *f = defaultval;
  return false;
}

bool Model::GetPropertyInt( const char* key, int* i, int defaultval ) const
{ 
  int* ip = (int*)GetProperty( key ); 
  if( ip )
	 {
		*i = *ip;
		return true;
	 }
  
  *i = defaultval;
  return false;
}

bool Model::GetPropertyStr( const char* key, char** c, char* defaultval ) const
{
  char* cp = (char*)GetProperty( key ); 
  
  if( cp )
	 {
		*c = cp;
		return true;
	 }
  
  *c = defaultval;
  return false;
}

void Model::SetPropertyInt( const char* key, int i )
{
  int* ip = new int(i);
  SetProperty( key, (void*)ip );
}

void Model::SetPropertyFloat( const char* key, float f )
{
  float* fp = new float(f);
  SetProperty( key, (void*)fp );
}

void Model::SetPropertyStr( const char* key, const char* str )
{
  SetProperty( key, (void*)strdup(str) );
}


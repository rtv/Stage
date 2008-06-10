
#include "stage_internal.hh"

#define MATCH(A,B) (strcmp(A,B)== 0)

void* StgModel::GetProperty( char* key )
{
	// see if the key has the predefined-property prefix
	if( strncmp( key, STG_MP_PREFIX, strlen(STG_MP_PREFIX)) == 0 )
	{
		if( MATCH( key, STG_MP_COLOR))            return (void*)&this->color;
		if( MATCH( key, STG_MP_MASS))             return (void*)&this->mass;
		if( MATCH( key, STG_MP_WATTS))            return (void*)&this->watts;
		if( MATCH( key, STG_MP_FIDUCIAL_RETURN))  return (void*)&this->fiducial_return;
		if( MATCH( key, STG_MP_LASER_RETURN))     return (void*)&this->laser_return;
		if( MATCH( key, STG_MP_OBSTACLE_RETURN))  return (void*)&this->obstacle_return;
		if( MATCH( key, STG_MP_RANGER_RETURN))    return (void*)&this->ranger_return;
		if( MATCH( key, STG_MP_GRIPPER_RETURN))   return (void*)&this->gripper_return;

		PRINT_WARN1( "Requested non-existent model core property \"%s\"", key );
		return NULL;
	}

	// otherwise it may be an arbitrary named property
	return g_datalist_get_data( &this->props, key );
}

int StgModel::SetProperty( char* key,
		void* data )
{
	// see if the key has the predefined-property prefix
	if( strncmp( key, STG_MP_PREFIX, strlen(STG_MP_PREFIX)) == 0 )
	{
		PRINT_DEBUG1( "Looking up model core property \"%s\"\n", key );

		if( MATCH(key,STG_MP_FIDUCIAL_RETURN) )
		{
			this->SetFiducialReturn( *(int*)data );
			return 0;
		}
		if( MATCH( key, STG_MP_LASER_RETURN ) )
		{
			this->SetLaserReturn( *(stg_laser_return_t*)data );
			return 0;
		}
		if( MATCH( key, STG_MP_OBSTACLE_RETURN ) )
		{
			this->SetObstacleReturn( *(int*)data );
			return 0;
		}
		if( MATCH( key, STG_MP_RANGER_RETURN ) )
		{
			this->SetRangerReturn( *(int*)data );
			return 0;
		}
		if( MATCH( key, STG_MP_GRIPPER_RETURN ) )
		{
			this->SetGripperReturn( *(int*)data );
			return 0;
		}
		if( MATCH( key, STG_MP_COLOR ) )
		{
			this->SetColor( *(int*)data );
			return 0;
		}
		if( MATCH( key, STG_MP_MASS ) )
		{
			this->SetMass( *(double*)data );
			return 0;
		}
		if( MATCH( key, STG_MP_WATTS ) )
		{
			this->SetWatts( *(double*)data );
			return 0;
		}

		PRINT_ERR1( "Attempt to set non-existent model core property \"%s\"", key );
		return 1; // error code
	}

	// otherwise it's an arbitary property and we store the pointer
	g_datalist_set_data( &this->props, key, data );
	return 0; // ok
}


void StgModel::UnsetProperty( char* key )
{
	if( strncmp( key, STG_MP_PREFIX, strlen(STG_MP_PREFIX)) == 0 )
		PRINT_WARN1( "Attempt to unset a model core property \"%s\" has no effect", key );
	else
		g_datalist_remove_data( &this->props, key );
}

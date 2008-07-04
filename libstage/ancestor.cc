#include "stage_internal.hh"

StgAncestor::StgAncestor()
{
	token = NULL;
	children = NULL;
	
	for( int i=0; i<MODEL_TYPE_COUNT; i++ )
	  child_type_counts[i] = 0;

	debug = false;
}

StgAncestor::~StgAncestor()
{
	if( children )
	{
		for( GList* it = children; it; it=it->next )
			delete (StgModel*)it->data;

		g_list_free( children );
	}

}

void StgAncestor::AddChild( StgModel* mod )
{
  
  // poke a name into the child  
  char* buf = new char[TOKEN_MAX];	

  if( token ) // if this object has a name, use it
    snprintf( buf, TOKEN_MAX, "%s.%s:%d", 
	      token, 
	      typetable[mod->type].token, 
	      child_type_counts[mod->type] );
  else
    snprintf( buf, TOKEN_MAX, "%s:%d", 
	      typetable[mod->type].token, 
	      child_type_counts[mod->type] );
    
  //printf( "%s generated a name for my child %s\n", token, buf );

  mod->SetToken( buf );

  children = g_list_append( children, mod );

  child_type_counts[mod->type]++;

  delete[] buf;
}

void StgAncestor::RemoveChild( StgModel* mod )
{
  child_type_counts[mod->type]--;
  children = g_list_remove( children, mod );
}

stg_pose_t StgAncestor::GetGlobalPose()
{
	stg_pose_t pose;
	bzero( &pose, sizeof(pose));
	return pose;
}


#include "stage.hh"
using namespace Stg;

Ancestor::Ancestor()
{
	token = NULL;
	children = NULL;
	
	for( int i=0; i<MODEL_TYPE_COUNT; i++ )
	  child_type_counts[i] = 0;

	debug = false;
}

Ancestor::~Ancestor()
{
	if( children )
	{
		for( GList* it = children; it; it=it->next )
			delete (Model*)it->data;

		g_list_free( children );
	}

}

void Ancestor::AddChild( Model* mod )
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

void Ancestor::RemoveChild( Model* mod )
{
  child_type_counts[mod->type]--;
  children = g_list_remove( children, mod );
}

Pose Ancestor::GetGlobalPose()
{
	Pose pose;
	bzero( &pose, sizeof(pose));
	return pose;
}

void Ancestor::ForEachDescendant( stg_model_callback_t func, void* arg )
{
	for( GList* it=children; it; it=it->next ) {
		Model* mod = (Model*)it->data;
		func( mod, arg );
		mod->ForEachDescendant( func, arg );
	}
}


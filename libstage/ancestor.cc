#include "stage.hh"
using namespace Stg;

Ancestor::Ancestor() :
  children(),
  debug( false ),
  puck_list( NULL ),
  token( NULL )
{
  for( int i=0; i<MODEL_TYPE_COUNT; i++ )
	 child_type_counts[i] = 0;
}

Ancestor::~Ancestor()
{
  FOR_EACH( it, children )
	 delete (*it);
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

  children.push_back( mod );

  child_type_counts[mod->type]++;

  delete[] buf;
}

void Ancestor::RemoveChild( Model* mod )
{
  child_type_counts[mod->type]--;

  //children = g_list_remove( children, mod );
  children.erase( remove( children.begin(), children.end(), mod ) );
}

Pose Ancestor::GetGlobalPose()
{
	Pose pose;
	bzero( &pose, sizeof(pose));
	return pose;
}

void Ancestor::ForEachDescendant( stg_model_callback_t func, void* arg )
{
  FOR_EACH( it, children )
	 {
		Model* mod = (*it);
		func( mod, arg );
		mod->ForEachDescendant( func, arg );
	 }
}



void Ancestor::Load( Worldfile* wf, int section )
{
}

void Ancestor::Save( Worldfile* wf, int section )
{
}

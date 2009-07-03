#include "stage.hh"
using namespace Stg;
//using names

Ancestor::Ancestor() :
  children(),
  debug( false ),
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
  
  //  printf( "adding child of type %d token %s\n", mod->type, mod->Token() );
  
  std::string typestr =  Model::type_map[ mod->type ];

  if( token ) // if this object has a name, use it
    snprintf( buf, TOKEN_MAX, "%s.%s:%d", 
			  token, 
			  typestr.c_str(),
			  child_type_counts[mod->type] );
  else
    snprintf( buf, TOKEN_MAX, "%s:%d", 
			  typestr.c_str(),
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
  
  children.erase( std::remove( children.begin(), children.end(), mod ) );
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

#include <sstream> // for converting values to strings

#include "stage.hh"
using namespace Stg;


Ancestor::Ancestor() :
  children(),
  debug( false ),
  token(),
  access_mutex(),
  child_type_counts()
{
  pthread_mutex_init( &access_mutex, NULL );
}

Ancestor::~Ancestor()
{
  FOR_EACH( it, children )
	 delete (*it);
}

void Ancestor::AddChild( Model* mod )
{
  // if the child is already there, this is a serious error
  if( std::find( children.begin(), children.end(), mod ) != children.end() )
	 {
		PRINT_ERR2( "Attempting to add child %s to %s - child already exists",
						mod->Token(), this->Token() );
		exit( -1 );
	 }
  
  // poke a name into the child  
  std::ostringstream name;
  
  //  printf( "adding child of type %d token %s\n", mod->type, mod->Token() );
  
  // if this object has a name, use it
  if( token.size() )
	 name << this->token <<  '.';
  
  name <<  mod->type <<  ':' << child_type_counts[mod->type]; 
  
  //  printf( "%s generated a name for my child %s\n", Token(),  name.str().c_str() );
  
  mod->SetToken( name.str() );
  
  children.push_back( mod );
  
  child_type_counts[mod->type]++;  
}

void Ancestor::RemoveChild( Model* mod )
{
  child_type_counts[mod->type]--;
  EraseAll( mod, children );
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

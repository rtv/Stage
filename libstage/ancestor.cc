#include "stage_internal.hh"

StgAncestor::StgAncestor()
{
  token = NULL;
  children = NULL;
  child_types = g_hash_table_new( g_str_hash, g_str_equal );
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

  g_hash_table_destroy( child_types );
}

unsigned int StgAncestor::GetNumChildrenOfType( const char* typestr )
{
  unsigned int *c = (unsigned int*)g_hash_table_lookup( child_types, typestr);

  if( c )
    return *c;
  else
    return 0;
}

void StgAncestor::IncrementNumChildrenOfType( const char* typestr )
{
  unsigned int* c = (unsigned int*)g_hash_table_lookup( child_types, typestr);
  
  if( c == NULL )
    {
      c = new unsigned int;
      g_hash_table_insert( child_types, (gpointer)typestr, (gpointer)c);
      *c = 1;
    }
  else
    (*c)++;
}

void StgAncestor::AddChild( StgModel* mod )
{
  // increment the count of models of this type
  IncrementNumChildrenOfType( mod->typestr );
  
  // store as a child
  children = g_list_append( children, mod );     
}

void StgAncestor::RemoveChild( StgModel* mod )
{
  puts( "TODO: StgWorld::RemoveChild()" );    
}

stg_pose_t StgAncestor::GetGlobalPose()
{
  stg_pose_t pose;
  bzero( &pose, sizeof(pose));
  return pose;
}


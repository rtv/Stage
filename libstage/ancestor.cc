#include "stage.hh"

//#include "ancestor.hh"
//#include "model.hh"

StgAncestor::StgAncestor()
{
  token = NULL;
  children = NULL;
  child_types = g_hash_table_new( g_str_hash, g_str_equal );
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
  return (unsigned int)g_hash_table_lookup( child_types, typestr);
}

void StgAncestor::SetNumChildrenOfType( const char* typestr, unsigned int count )
{
  g_hash_table_insert( child_types, (gpointer)typestr, (gpointer)count);
}

void StgAncestor::AddChild( StgModel* mod )
{
  // increment the count of models of this type
  unsigned int count = GetNumChildrenOfType( mod->typestr );
  SetNumChildrenOfType( mod->typestr, ++count );
  
  // store as a child
  children = g_list_append( children, mod );     
}

void StgAncestor::RemoveChild( StgModel* mod )
{
  puts( "TODO: StgWorld::RemoveChild()" );    
}

void StgAncestor::GetGlobalPose( stg_pose_t* gpose )
{
  assert( gpose );
  memset( gpose, 0, sizeof(stg_pose_t));
}


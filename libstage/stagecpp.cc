// Thin-as-possible C Wrappers for C++ worldfile calls, using a single static worldfile.
// This is a hacky use of the old C++ worldfile code.
// $Id: stagecpp.cc 7151 2008-11-15 03:11:20Z rtv $

#include "stage_internal.h"
#include "gui.h"
#include "worldfile.hh"


static CWorldFile wf;

void wf_warn_unused( void )
{
	wf.WarnUnused();
}

int wf_property_exists( int section, char* token )
{
	return( wf.GetProperty( section, token ) > -1 );
}

// read wrappers
int wf_read_int( int section, char* token, int def )
{
	return wf.ReadInt( section, token, def );
}

double wf_read_float( int section, char* token, double def )
{
	return wf.ReadFloat( section, token, def );
}

double wf_read_length( int section, char* token, double def )
{
	return wf.ReadLength( section, token, def );
}

double wf_read_angle( int section, char* token, double def )
{
	return wf.ReadAngle( section, token, def );
}

const char* wf_read_string( int section, char* token, char* def )
{
	return wf.ReadString( section, token, def );
}

const char* wf_read_tuple_string( int section, char* token, int index, char* def )
{
	return wf.ReadTupleString( section, token, index, def );
}

double wf_read_tuple_float( int section, char* token, int index, double def )
{
	return wf.ReadTupleFloat( section, token, index, def );
}

double wf_read_tuple_length( int section, char* token, int index, double def )
{
	return wf.ReadTupleLength( section, token, index, def );
}

double wf_read_tuple_angle( int section, char* token, int index, double def )
{
	return wf.ReadTupleAngle( section, token, index, def );
}

// write wrappers

void wf_write_int( int section, char* token, int value )
{
	wf.WriteInt( section, token, value );
}

void wf_write_float( int section, char* token, double value )
{
	wf.WriteFloat( section, token, value );
}

void wf_write_length( int section, char* token, double value )
{
	wf.WriteLength( section, token, value );
}

//void wf_write_angle( int section, char* token, double value )
//{
//wf.WriteAngle( section, token, value );
//}

void wf_write_string( int section, char* token, char* value )
{
	wf.WriteString( section, token, value );
}

void wf_write_tuple_string( int section, char* token, int index, char* value )
{
	wf.WriteTupleString( section, token, index, value );
}

void wf_write_tuple_float( int section, char* token, int index, double value )
{
	wf.WriteTupleFloat( section, token, index, value );
}

void wf_write_tuple_length( int section, char* token, int index, double value )
{
	wf.WriteTupleLength( section, token, index, value );
}

void wf_write_tuple_angle( int section, char* token, int index, double value )
{
	wf.WriteTupleAngle( section, token, index, value );
}


void wf_save( void )
{
	wf.Save( NULL );
}

void wf_load( char* path )
{
	wf.Load( path );
}

int wf_section_count( void )
{
	return wf.GetEntityCount();
}

const char* wf_get_section_type( int section )
{
	return wf.GetEntityType( section );
}

int wf_get_parent_section( int section )
{
	return wf.GetEntityParent( section );
}

const char* wf_get_filename( )
{
	return( (const char*)wf.filename );
}

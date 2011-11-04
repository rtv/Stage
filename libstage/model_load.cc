#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <limits.h> 
#include <libgen.h> // for dirname()
#include <string.h>
#include <ltdl.h> // for library module loading

#include "stage.hh"
#include "worldfile.hh"
#include "file_manager.hh"
#include "config.h"
using namespace Stg;

//#define DEBUG


void Model::Load()
{  
  assert( wf );
  assert( wf_entity );
  
  PRINT_DEBUG1( "Model \"%s\" loading...", token );
  
  // choose the thread to run in, if thread_safe > 0 
  event_queue_num = wf->ReadInt( wf_entity, "event_queue", event_queue_num );

  if( wf->PropertyExists( wf_entity, "joules" ) )
    {
      if( !power_pack )
	power_pack = new PowerPack( this );
		
      joules_t j = wf->ReadFloat( wf_entity, "joules", 
				  power_pack->GetStored() ) ;	 
		
      /* assume that the store is full, so the capacity is the same as
	 the charge */
      power_pack->SetStored( j );
      power_pack->SetCapacity( j );
    }

  if( wf->PropertyExists( wf_entity, "joules_capacity" ) )
    {
      if( !power_pack )
	power_pack = new PowerPack( this );
		
      power_pack->SetCapacity( wf->ReadFloat( wf_entity, "joules_capacity",
					      power_pack->GetCapacity() ) ); 		
    }

  if( wf->PropertyExists( wf_entity, "kjoules" ) )
    {
      if( !power_pack )
	power_pack = new PowerPack( this );
		
      joules_t j = 1000.0 * wf->ReadFloat( wf_entity, "kjoules", 
					   power_pack->GetStored() ) ;	 
		
      /* assume that the store is full, so the capacity is the same as
	 the charge */
      power_pack->SetStored( j );
      power_pack->SetCapacity( j );
    }
  
  if( wf->PropertyExists( wf_entity, "kjoules_capacity" ) )
    {
      if( !power_pack )
	power_pack = new PowerPack( this );
		
      power_pack->SetCapacity( 1000.0 * wf->ReadFloat( wf_entity, "kjoules_capacity",
						       power_pack->GetCapacity() ) ); 		
    }

    
  watts = wf->ReadFloat( wf_entity, "watts", watts );    
  watts_give = wf->ReadFloat( wf_entity, "give_watts", watts_give );    
  watts_take = wf->ReadFloat( wf_entity, "take_watts", watts_take );
  
  debug = wf->ReadInt( wf_entity, "debug", debug );
  
  const std::string& name = wf->ReadString(wf_entity, "name", token );
  if( name != token )
    SetToken( name );
  
  //PRINT_WARN1( "%s::Load", token );
  
  Geom g( GetGeom() );
  
  if( wf->PropertyExists( wf_entity, "origin" ) )
    g.pose.Load( wf, wf_entity, "origin" );
  
  if( wf->PropertyExists( wf_entity, "size" ) )
    g.size.Load( wf, wf_entity, "size" );
  
  SetGeom( g );

  if( wf->PropertyExists( wf_entity, "pose" ))
    SetPose( GetPose().Load( wf, wf_entity, "pose" ) );
  
  if( wf->PropertyExists( wf_entity, "velocity" ))
    SetVelocity( GetVelocity().Load( wf, wf_entity, "velocity" ));

  if( wf->PropertyExists( wf_entity, "color" ))
    {      
      Color col( 1,0,0 ); // red;
      const std::string& colorstr = wf->ReadString( wf_entity, "color", "" );
      if( colorstr != "" )
	{
	  if( colorstr == "random" )
	    col = Color( drand48(), drand48(), drand48() );
	  else
	    col = Color( colorstr );
	}
      this->SetColor( col );
    }        
  
  this->SetColor( GetColor().Load( wf, wf_entity ) );
  
  if( wf->ReadInt( wf_entity, "noblocks", 0 ) )
    {
      if( has_default_block )
	{
	  blockgroup.Clear();
	  has_default_block = false;
	  blockgroup.CalcSize();
	}		
    }
  
  if( wf->PropertyExists( wf_entity, "bitmap" ) )
    {
      const std::string bitmapfile = wf->ReadString( wf_entity, "bitmap", "" );
      if( bitmapfile == "" )
	PRINT_WARN1( "model %s specified empty bitmap filename\n", Token() );
		
      if( has_default_block )
	{
	  blockgroup.Clear();
	  has_default_block = false;
	}
		
      blockgroup.LoadBitmap( this, bitmapfile, wf );
    }
  
  if( wf->PropertyExists( wf_entity, "boundary" ))
    {
      this->SetBoundary( wf->ReadInt(wf_entity, "boundary", this->boundary  ));
		
      if( boundary )
	{
	  //PRINT_WARN1( "setting boundary for %s\n", token );
			 
	  blockgroup.CalcSize();
			 
	  double epsilon = 0.01;	      
	  Size bgsize = blockgroup.GetSize();
			 
	  AddBlockRect(blockgroup.minx,blockgroup.miny, epsilon, bgsize.y, bgsize.z );	      
	  AddBlockRect(blockgroup.minx,blockgroup.miny, bgsize.x, epsilon, bgsize.z );	      
	  AddBlockRect(blockgroup.minx,blockgroup.maxy-epsilon, bgsize.x, epsilon, bgsize.z );	      
	  AddBlockRect(blockgroup.maxx-epsilon,blockgroup.miny, epsilon, bgsize.y, bgsize.z );	      
	}     
    }	  
  
  this->stack_children =
    wf->ReadInt( wf_entity, "stack_children", this->stack_children );
  
  kg_t m = wf->ReadFloat(wf_entity, "mass", this->mass );
  if( m != this->mass ) 
    SetMass( m );
  	
  vis.Load( wf, wf_entity );
  SetFiducialReturn( vis.fiducial_return ); // may have some work to do
  
  gui.Load( wf, wf_entity );

  double res = wf->ReadFloat(wf_entity, "map_resolution", this->map_resolution );
  if( res != this->map_resolution )
    SetMapResolution( res );
  
  velocity_enable = wf->ReadInt( wf_entity, "enable_velocity", velocity_enable );
	
  if( wf->PropertyExists( wf_entity, "friction" ))
    {
      this->SetFriction( wf->ReadFloat(wf_entity, "friction", this->friction ));
    }
  
  if( CProperty* ctrlp = wf->GetProperty( wf_entity, "ctrl" ) )
    {
      for( unsigned int index=0; index < ctrlp->values.size(); index++ )
	{
			 
	  const char* lib = wf->GetPropertyValue( ctrlp, index );
			 
	  if( !lib )
	    printf( "Error - NULL library name specified for model %s\n", Token() );
	  else
	    LoadControllerModule( lib );
	}
    }
    
  // internally interval is in usec, but we use msec in worldfiles
  interval = 1000 * wf->ReadInt( wf_entity, "update_interval", interval/1000 );

  Say( wf->ReadString(wf_entity, "say", "" ));
  
  trail_length = wf->ReadInt( wf_entity, "trail_length", trail_length );
  trail.resize( trail_length );

  trail_interval = wf->ReadInt( wf_entity, "trail_interval", trail_interval );

  this->alwayson = wf->ReadInt( wf_entity, "alwayson",  alwayson );
  if( alwayson )
    Subscribe();
	
  // call any type-specific load callbacks
  this->CallCallbacks( CB_LOAD );
  
  // we may well have changed blocks or geometry
  blockgroup.CalcSize();
  
  UnMapWithChildren(0);
  UnMapWithChildren(1);
  MapWithChildren(0);
  MapWithChildren(1);
  
  if( this->debug )
    printf( "Model \"%s\" is in debug mode\n", Token() );

  PRINT_DEBUG1( "Model \"%s\" loading complete", Token() );
}


void Model::Save( void )
{  
  //printf( "Model \"%s\" saving...\n", Token() );

  // some models were not loaded, so have no worldfile. Just bail here.
  if( wf == NULL )
    return;

  assert( wf_entity );
	
  PRINT_DEBUG4( "saving model %s pose %.2f %.2f %.2f",
		token,
		pose.x,
		pose.y,
		pose.a );
	
  // just in case
  pose.a = normalize( pose.a );
  geom.pose.a = normalize( geom.pose.a );
  
  if( wf->PropertyExists( wf_entity, "pose" ) )
    pose.Save( wf, wf_entity, "pose" );
  
  if( wf->PropertyExists( wf_entity, "size" ) )
    geom.size.Save( wf, wf_entity, "size" );
  
  if( wf->PropertyExists( wf_entity, "origin" ) )
    geom.pose.Save( wf, wf_entity, "origin" );

  vis.Save( wf, wf_entity );

  // call any type-specific save callbacks
  CallCallbacks( CB_SAVE );

  PRINT_DEBUG1( "Model \"%s\" saving complete.", token );
}


void Model::LoadControllerModule( const char* lib )
{
  //printf( "[Ctrl \"%s\"", lib );
  //fflush(stdout);

  /* Initialise libltdl. */
  int errors = lt_dlinit();
  if (errors)
    {
      printf( "Libtool error: %s. Failed to init libtool. Quitting\n",
	      lt_dlerror() ); // report the error from libtool
      puts( "libtool error #1" );
      fflush( stdout );
      exit(-1);
    }

  lt_dlsetsearchpath( FileManager::stagePath().c_str() );

  // PLUGIN_PATH now defined in config.h
  lt_dladdsearchdir( PLUGIN_PATH );

  lt_dlhandle handle = NULL;
  
  // the library name is the first word in the string
  char libname[256];
  sscanf( lib, "%s %*s", libname );
  
  if(( handle = lt_dlopenext( libname ) ))
    {
      //printf( "]" );
		
      model_callback_t initfunc = (model_callback_t)lt_dlsym( handle, "Init" );
      if( initfunc  == NULL )
	{
	  printf( "Libtool error: %s. Something is wrong with your plugin. Quitting\n",
		  lt_dlerror() ); // report the error from libtool
	  puts( "libtool error #1" );
	  fflush( stdout );
	  exit(-1);
	}
		
      AddCallback( CB_INIT, initfunc, new CtrlArgs(lib,World::ctrlargs) ); // pass complete string into initfunc
    }
  else
    {
      printf( "Libtool error: %s. Can't open your plugin controller. Quitting\n",
	      lt_dlerror() ); // report the error from libtool
		
      PRINT_ERR1( "Failed to open \"%s\". Check that it can be found by searching the directories in your STAGEPATH environment variable, or the current directory if STAGEPATH is not set.]\n", lib );
      puts( "libtool error #2" );
      fflush( stdout );
      exit(-1);
    }
  
  fflush(stdout);
}


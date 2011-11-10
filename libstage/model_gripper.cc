///////////////////////////////////////////////////////////////////////////
//
// File: model_gripper.cc
// Authors: Richard Vaughan <vaughan@sfu.ca>
//          Doug Blank
// Date: 21 April 2005
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/model_gripper.c,v $
//  $Author: thjc $
//  $Revision$
//
///////////////////////////////////////////////////////////////////////////

/**
@ingroup model
@defgroup model_gripper Gripper model 

The ranger model simulates a simple two-fingered gripper with two
internal break-beams, similar to the the Pioneer gripper.

<h2>Worldfile properties</h2>

@par Summary and default values

@verbatim
gripper
(
  # gripper properties
  paddle_size [ 0.66 0.1 0.4 ]
  paddle_state [ "open" "down" ]
  autosnatch 0

  # model properties
  size [ 0.2 0.3 0.2 ]
)
@endverbatim

@par Notes

@par Details

- autosnatch < 0 or 1>\n
  iff 1, gripper will close automatically when break beams are broken
- paddle_size [ <float x> <float y < float z> ]\n
  Gripper paddle size as a proportion of the model's body size (0.0 to 1.0)
- paddle_state [ <string open/close> <string up/down> ]\n
  Gripper arms state, either "open" or "closed", and lift state, either "up" or "down"
*/


#include <sys/time.h>
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

#include "option.hh"
Option ModelGripper::showData( "Gripper data", "show_gripper_data", "", true, NULL );

// TODO - simulate energy use when moving grippers

ModelGripper::ModelGripper( World* world, 
									 Model* parent,
									 const std::string& type ) : 
  Model( world, parent, type ),	 
  cfg(), // configured below
  cmd( CMD_NOOP )
{
  // set up a gripper-specific config structure
  cfg.paddle_size.x = 0.66; // proportion of body length that is paddles
  cfg.paddle_size.y = 0.1; // proportion of body width that is paddles
  cfg.paddle_size.z = 0.4; // proportion of body height that is paddles

  cfg.paddles = PADDLE_OPEN;
  cfg.lift = LIFT_DOWN;
  cfg.paddle_position = 0.0;
  cfg.lift_position = 0.0;
  cfg.paddles_stalled = false;
  cfg.autosnatch = false;
  cfg.gripped = NULL;
  cfg.beam[0] = 0;
  cfg.beam[1] = 0;
  cfg.contact[0] = 0;
  cfg.contact[1] = 0;

  // place the break beam sensors at 1/4 and 3/4 the length of the paddle 
  cfg.break_beam_inset[0] = 3.0/4.0 * cfg.paddle_size.x;
  cfg.break_beam_inset[1] = 1.0/4.0 * cfg.paddle_size.x;
  
  cfg.close_limit = 1.0;
  
  SetColor( Color(0.3, 0.3, 0.3, 1.0) );

  FixBlocks();

  // Update() is not reentrant
  thread_safe = false;

  // set default size
  SetGeom( Geom( Pose(0,0,0,0), Size( 0.2, 0.3, 0.2)));
  
  PositionPaddles();  

  RegisterOption( &showData );
}

ModelGripper::~ModelGripper()
{
  /* do nothing */
}


void ModelGripper::Load()
{
  cfg.autosnatch = wf->ReadInt( wf_entity, "autosnatch", cfg.autosnatch );
    
  // cfg.paddle_size.x = wf->ReadTupleFloat( wf_entity, "paddle_size", 0, cfg.paddle_size.x );
  // cfg.paddle_size.y = wf->ReadTupleFloat( wf_entity, "paddle_size", 1, cfg.paddle_size.y );
  // cfg.paddle_size.z = wf->ReadTupleFloat( wf_entity, "paddle_size", 2, cfg.paddle_size.z );

  wf->ReadTuple( wf_entity, "paddle_size", 0, 3, "lll",
		 &cfg.paddle_size.x, 
		 &cfg.paddle_size.y, 
		 &cfg.paddle_size.z );
  
  //const char* paddles =  wf->ReadTupleString( wf_entity, "paddle_state", 0, NULL );
  //const char* lift =     wf->ReadTupleString( wf_entity, "paddle_state", 1, NULL );
  
  char* paddles = NULL;
  char* lift = NULL;
  wf->ReadTuple( wf_entity, "paddle_state", 0, 2, "ss", &paddles, &lift );

  if( paddles && strcmp( paddles, "closed" ) == 0 )
	 {
		cfg.paddle_position = 1.0;
		cfg.paddles = PADDLE_CLOSED;
	 }

  if( paddles && strcmp( paddles, "open" ) == 0 )
	 {
		cfg.paddle_position = 0.0;
		cfg.paddles = PADDLE_OPEN;
	 }

  if( lift && strcmp( lift, "up" ) == 0 )	 
	 {
		cfg.lift_position = 1.0;
		cfg.lift = LIFT_UP;
	 }

  if( lift && strcmp( lift, "down" ) == 0 )	 
	 {
		cfg.lift_position = 0.0;
		cfg.lift = LIFT_DOWN;
	 }
	   
  FixBlocks();
  
  // do this at the end to ensure that the blocks are resize correctly
  Model::Load();	
}

void ModelGripper::Save()
{
  Model::Save();

  // wf->WriteTupleFloat( wf_entity, "paddle_size", 0, cfg.paddle_size.x );
  // wf->WriteTupleFloat( wf_entity, "paddle_size", 1, cfg.paddle_size.y );
  // wf->WriteTupleFloat( wf_entity, "paddle_size", 2, cfg.paddle_size.z );    

  wf->WriteTuple( wf_entity, "paddle_size", 0, 3, "lll", 
		  cfg.paddle_size.x,
		  cfg.paddle_size.y,
		  cfg.paddle_size.z );    

  // wf->WriteTupleString( wf_entity, "paddle_state", 0, (cfg.paddles == PADDLE_CLOSED ) ? "closed" : "open" );
  // wf->WriteTupleString( wf_entity, "paddle_state", 1, (cfg.lift == LIFT_UP ) ? "up" : "down" );
  
  wf->WriteTuple( wf_entity, "paddle_state", 0, 2, "ss", 
		  (cfg.paddles == PADDLE_CLOSED ) ? "closed" : "open",
		  (cfg.lift == LIFT_UP ) ? "up" : "down" );
  
}

void ModelGripper::FixBlocks()
{
  // get rid of the default cube
  ClearBlocks(); 
  
  // add three blocks that make the gripper
  // base
  AddBlockRect( 0, 0, 1.0-cfg.paddle_size.x, 1.0, 1.0 );

  // left (top) paddle
  paddle_left = AddBlockRect( 1.0-cfg.paddle_size.x, 0, cfg.paddle_size.x, cfg.paddle_size.y, cfg.paddle_size.z );

  // right (bottom) paddle
  paddle_right = AddBlockRect( 1.0-cfg.paddle_size.x, 1.0-cfg.paddle_size.y, cfg.paddle_size.x, cfg.paddle_size.y, cfg.paddle_size.z );

  PositionPaddles();
}

// Update the blocks that are the gripper's body
void ModelGripper::PositionPaddles()
{
	unsigned int layer = world->GetUpdateCount()%2;
  UnMap(layer);

  double paddle_center_pos = cfg.paddle_position * (0.5 - cfg.paddle_size.y );
  paddle_left->SetCenterY( paddle_center_pos + cfg.paddle_size.y/2.0 );
  paddle_right->SetCenterY( 1.0 - paddle_center_pos - cfg.paddle_size.y/2.0);

  double paddle_bottom = cfg.lift_position * (1.0 - cfg.paddle_size.z);
  double paddle_top = paddle_bottom +  cfg.paddle_size.z;

  paddle_left->SetZ( paddle_bottom, paddle_top );
  paddle_right->SetZ( paddle_bottom, paddle_top );
  
  Map(layer);
 }


void ModelGripper::Update()
{   
  float start_paddle_position = cfg.paddle_position;
  float start_lift_position = cfg.lift_position;

  switch( cmd )
    {
    case CMD_NOOP:
		break;
      
    case CMD_CLOSE:     
      if( cfg.paddles != PADDLE_CLOSED )
		  {
			 //puts( "closing gripper paddles" );
			 cfg.paddles = PADDLE_CLOSING;
		  }
      break;
      
    case CMD_OPEN:
      if( cfg.paddles != PADDLE_OPEN )
		  {
			 //puts( "opening gripper paddles" );      
			 cfg.paddles = PADDLE_OPENING;
		  }
      break;
      
    case CMD_UP:
      if( cfg.lift != LIFT_UP )
		  {
			 //puts( "raising gripper lift" );      
			 cfg.lift = LIFT_UPPING;
		  }
      break;
      
    case CMD_DOWN:
      if( cfg.lift != LIFT_DOWN )
		  {
			 //puts( "lowering gripper lift" );      
			 cfg.lift = LIFT_DOWNING;
		  }      
      break;
      
    default:
      printf( "unknown gripper command %d\n",cmd );
    }
  
  //   // move the paddles 
  if( cfg.paddles == PADDLE_OPENING )// && !cfg.paddles_stalled  )
	 {
		cfg.paddle_position -= 0.05;
      
		if( cfg.paddle_position < 0.0 ) // if we're fully open
		  {
			 cfg.paddle_position = 0.0;
			 cfg.paddles = PADDLE_OPEN; // change state
		  }
		
		
		// drop the thing we're carrying
		if( cfg.gripped &&
			 (cfg.paddle_position == 0.0 || cfg.paddle_position < cfg.close_limit ))
		  {
			 // move it to the new location
			 cfg.gripped->SetParent( NULL );	      
			 cfg.gripped->SetPose( this->GetGlobalPose() );
			 cfg.gripped = NULL;
			 
			 cfg.close_limit = 1.0;
		  }
	 }

  else if( cfg.paddles == PADDLE_CLOSING ) //&& !cfg.paddles_stalled  )
	 {
		cfg.paddle_position += 0.05;
		//printf( "paddle position %.2f\n", cfg.paddle_position );
      
		if( cfg.paddle_position > cfg.close_limit ) // if we're fully closed
		  {
			 cfg.paddle_position = cfg.close_limit;
			 cfg.paddles = PADDLE_CLOSED; // change state
		  }
	 }
  
  switch( cfg.lift )
	 {
	 case LIFT_DOWNING:
		cfg.lift_position -= 0.05;
      
      if( cfg.lift_position < 0.0 ) // if we're fully down
		  {
			 cfg.lift_position = 0.0;
			 cfg.lift = LIFT_DOWN; // change state
		  }
	  break;
	 
	 case LIFT_UPPING:
		cfg.lift_position += 0.05;
      
		if( cfg.lift_position > 1.0 ) // if we're fully open
		  {
			 cfg.lift_position = 1.0;
			 cfg.lift = LIFT_UP; // change state
		  }
		break;	

	 case LIFT_DOWN: // nothing to do for these cases
	 case LIFT_UP:
	 default:
		break;
	 }
  
  // if the paddles or lift have changed position
  if( start_paddle_position != cfg.paddle_position || 
		start_lift_position != cfg.lift_position )
	 // figure out where the paddles should be
	 PositionPaddles();
  

  UpdateBreakBeams();
  UpdateContacts();
  
  Model::Update();
}


	
static bool gripper_raytrace_match( Model* hit, 
												Model* finder,
												const void* dummy )
{
  (void)dummy; // avoid warning about unused var

  return( (hit != finder) && hit->vis.gripper_return );
  // can't use the normal relation check, because we may pick things
  // up and we must still see them.
}

void ModelGripper::UpdateBreakBeams() 
{
  for( unsigned int index=0; index < 2; index++ )
	 {  
		Pose pz;
		
		// x location of break beam origin
		double inset = cfg.break_beam_inset[index];
		
		pz.x = (geom.size.x - inset * geom.size.x) - geom.size.x/2.0;
		
		// y location of break beam origin
		pz.y = (1.0 - cfg.paddle_position) * ((geom.size.y/2.0)-(geom.size.y*cfg.paddle_size.y));
		
		pz.z = 0.0; // TODO
		
		// break beam local heading
		pz.a = -M_PI/2.0;
		
		// break beam max range
		double bbr = 
		  (1.0 - cfg.paddle_position) * (geom.size.y - (geom.size.y * cfg.paddle_size.y * 2.0 ));
		
		RaytraceResult sample = 
		  Raytrace( pz,  // ray origin
						bbr, // range
						gripper_raytrace_match, // match func
						NULL, // match arg
						true ); // ztest
		
		cfg.beam[index] = sample.mod;
	 }

  // autosnatch grabs anything that breaks the inner beam
  if( cfg.autosnatch )
	 {
		if( cfg.beam[0] || cfg.beam[1] )
		  cmd = CMD_CLOSE;
		else
		  cmd = CMD_OPEN;
	 }
}

void ModelGripper::UpdateContacts()
{
  cfg.paddles_stalled = false; // may be changed below

  Pose lpz, rpz;
  
  // x location of contact sensor origin  
  lpz.x = ((1.0 - cfg.paddle_size.x) * geom.size.x) - geom.size.x/2.0 ;
  rpz.x = ((1.0 - cfg.paddle_size.x) * geom.size.x) - geom.size.x/2.0 ;

//   //double inset = beam ? cfg->inner_break_beam_inset : cfg->outer_break_beam_inset;
//   //pz.x = (geom.size.x - inset * geom.size.x) - geom.size.x/2.0;

  // y location of paddle beam origin
  
  lpz.y = (1.0 - cfg.paddle_position) * 
	 ((geom.size.y/2.0) - (geom.size.y*cfg.paddle_size.y));
  
  rpz.y = (1.0 - cfg.paddle_position) * 
	 -((geom.size.y/2.0) - (geom.size.y*cfg.paddle_size.y));
  
  lpz.z = 0.0; // todo
  rpz.z = 0.0;

  // paddle beam local heading
  lpz.a = 0.0;
  rpz.a = 0.0;
  
  // paddle beam max range
  double bbr = cfg.paddle_size.x * geom.size.x; 
  
  RaytraceResult leftsample = 
	 Raytrace( lpz,  // ray origin
				  bbr, // range
				  gripper_raytrace_match, // match func
				  NULL, // match arg
				  true ); // ztest
  
  cfg.contact[0] = leftsample.mod;
  
  RaytraceResult rightsample = 
	 Raytrace( rpz,  // ray origin
				  bbr, // range
				  gripper_raytrace_match, // match func
				  NULL, // match arg
				  true ); // ztest

  cfg.contact[1] = rightsample.mod;
  
  if( cfg.contact[0] || cfg.contact[1] )
	 {
		cfg.paddles_stalled = true;;
  
		//   if( lhit && (lhit == rhit) )
		//   {
		//     //puts( "left and right hit same thing" );
		
		if(  cfg.paddles == PADDLE_CLOSING )
		  {
			 Model* hit = cfg.contact[0];
			 if( !hit )
				hit = cfg.contact[1];
			 
			 if( cfg.gripped == NULL ) // if we're not carrying something already
				{
				  // get the global pose of the gripper for calculations of the gripped object position
				  // and move it to be right between the paddles
				  Geom hitgeom = hit->GetGeom();
				  //Pose hitgpose = hit->GetGlobalPose();
				  
				  //       pose_t pose = {0,0,0};
				  //       model_local_to_global( lhit, &pose );
				  //       model_global_to_local( mod, &pose );
				  
				  //       // grab the model we hit - very simple grip model for now
				  hit->SetParent( this );
				  hit->SetPose( Pose(0,0, -1.0 * geom.size.z ,0) );
				  
				  cfg.gripped = hit;
				  
				  //       // calculate how far closed we can get the paddles now
				  double puckw = hitgeom.size.y;
				  double gripperw = geom.size.y;	      
				  cfg.close_limit = std::max( 0.0, 1.0 - puckw/(gripperw - cfg.paddle_size.y/2.0 ));
				}
		  }
	 }
}


void ModelGripper::DataVisualize( Camera* cam )
{
  (void)cam; // avoid warning about unused var

  // only draw if someone is using the gripper
  if( subs < 1 )
	 return;
  
  //if( ! showData )
  //return;

	// outline the sensor lights in black
	PushColor( 0,0,0,1.0 ); // black
	glTranslatef( 0,0, geom.size.z * cfg.paddle_size.z );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

   // different x location for each beam
   double ibbx =  (geom.size.x - cfg.break_beam_inset[0] * geom.size.x) - geom.size.x/2.0;
   double obbx =  (geom.size.x - cfg.break_beam_inset[1] * geom.size.x) - geom.size.x/2.0;
	
	// common y position
	double invp = 1.0 - cfg.paddle_position;
   double bby = 
     invp * ((geom.size.y/2.0)-(geom.size.y*cfg.paddle_size.y));
	
	//   // size of the paddle indicator lights
   double led_dx = cfg.paddle_size.y * 0.5 * geom.size.y;
		
	// paddle break beams
  	Gl::draw_centered_rect( ibbx, bby+led_dx, led_dx, led_dx );
  	Gl::draw_centered_rect( ibbx, -bby-led_dx, led_dx, led_dx );
  	Gl::draw_centered_rect( obbx, bby+led_dx, led_dx, led_dx );
  	Gl::draw_centered_rect( obbx, -bby-led_dx, led_dx, led_dx );
	
	// paddle contacts
	double cx = ((1.0 - cfg.paddle_size.x/2.0) * geom.size.x) - geom.size.x/2.0;
	double cy = (geom.size.y/2.0)-(geom.size.y * 0.8 * cfg.paddle_size.y);
	double plen = cfg.paddle_size.x * geom.size.x;
	double pwidth = 0.4 * cfg.paddle_size.y * geom.size.y;	
	
	Gl::draw_centered_rect( cx, invp * +cy, plen, pwidth );	
	Gl::draw_centered_rect( cx, invp * -cy, plen, pwidth );
	
	// if the gripper detects anything, fill the lights in with yellow
	if( cfg.beam[0] || cfg.beam[1] || cfg.contact[0] || cfg.contact[1] )
	  {
		 PushColor( 1,1,0,1.0 ); // yellow
		 glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		 
		 if( cfg.contact[0] )
			Gl::draw_centered_rect( cx, invp * +cy, plen, pwidth );	
		 
		 if( cfg.contact[1] )
			Gl::draw_centered_rect( cx, invp * -cy, plen, pwidth );	
		 
		 if( cfg.beam[0] )
			{
			  Gl::draw_centered_rect( ibbx, bby+led_dx, led_dx, led_dx );
			  Gl::draw_centered_rect( ibbx, -bby-led_dx, led_dx, led_dx );	
			}
		 
		 if( cfg.beam[1] )			  
			{
			  Gl::draw_centered_rect( obbx, bby+led_dx, led_dx, led_dx );
			  Gl::draw_centered_rect( obbx, -bby-led_dx, led_dx, led_dx );	
			}

		 PopColor(); // yellow	  
	  }
		
	  PopColor(); // black
 }



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
//  $Revision: 1.26.8.1 $
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
  <none>

  # model properties
  size [0.12 0.28]
)
@endverbatim

@par Notes

@par Details

*/


#include <sys/time.h>
#include <math.h>
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

// TODO - simulate energy use when moving grippers

ModelGripper::ModelGripper( World* world, 
									 Model* parent )
  : Model( world, parent, MODEL_TYPE_GRIPPER ),	 
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
  cfg.grip_stack = NULL;
  cfg.grip_stack_size = 1;

  // place the break beam sensors at 1/4 and 3/4 the length of the paddle 
  cfg.break_beam_inset[0] = 3.0/4.0 * cfg.paddle_size.x;
  cfg.break_beam_inset[1] = 1.0/4.0 * cfg.paddle_size.x;
  
  cfg.close_limit = 1.0;
  
  SetColor( stg_color_pack( 0.3, 0.3, 0.3, 0 ));

  FixBlocks();

  // default size
  Geom geom;
  geom.pose.x = 0.0;
  geom.pose.y = 0.0;
  geom.pose.a = 0.0;
  geom.size.x = 0.2;
  geom.size.y = 0.3;
  geom.size.z = 0.2;
  SetGeom( geom );

  //Startup();
 
  PositionPaddles();  
}

ModelGripper::~ModelGripper()
{
  /* do nothing */
}


void ModelGripper::Load()
{
  cfg.autosnatch = wf->ReadInt( wf_entity, "autosnatch", cfg.autosnatch );
    
  cfg.paddle_size.x = wf->ReadTupleFloat( wf_entity, "paddle_size", 0, cfg.paddle_size.x );
  cfg.paddle_size.y = wf->ReadTupleFloat( wf_entity, "paddle_size", 1, cfg.paddle_size.y );
  cfg.paddle_size.z = wf->ReadTupleFloat( wf_entity, "paddle_size", 2, cfg.paddle_size.z );
  
  const char* paddles =  wf->ReadTupleString( wf_entity, "paddles", 0, NULL );
  const char* lift =     wf->ReadTupleString( wf_entity, "paddles", 1, NULL );

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
	   
  cfg.grip_stack_size = wf->ReadInt( wf_entity , "stack_size", cfg.grip_stack_size );
  
  FixBlocks();
  
  // do this at the end to ensure that the blocks are resize correctly
  Model::Load();	
}

void ModelGripper::Save()
{
  Model::Save();

  wf->WriteTupleFloat( wf_entity, "paddle_size", 0, cfg.paddle_size.x );
  wf->WriteTupleFloat( wf_entity, "paddle_size", 1, cfg.paddle_size.y );
  wf->WriteTupleFloat( wf_entity, "paddle_size", 2, cfg.paddle_size.z );
    
  wf->WriteInt( wf_entity , "stack_size", cfg.grip_stack_size );
}

void ModelGripper::DataVisualize( Camera* cam )
{
  /* do nothing */
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
  UnMap();
  double paddle_center_pos = cfg.paddle_position * (0.5 - cfg.paddle_size.y );
  paddle_left->SetCenterY( paddle_center_pos + cfg.paddle_size.y/2.0 );
  paddle_right->SetCenterY( 1.0 - paddle_center_pos - cfg.paddle_size.y/2.0);

  double paddle_bottom = cfg.lift_position * (1.0 - cfg.paddle_size.z);
  double paddle_top = paddle_bottom +  cfg.paddle_size.z;

  paddle_left->SetZ( paddle_bottom, paddle_top );
  paddle_right->SetZ( paddle_bottom, paddle_top );
  
  Map();
 }


void ModelGripper::Update()
{   
  // no work to do if we're unsubscribed
  if( subs < 1 )
	 {
		Model::Update();		
		return;
	 }
  
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
  if( cfg.paddles == PADDLE_OPENING && !cfg.paddles_stalled  )
	 {
		cfg.paddle_position -= 0.05;
      
		if( cfg.paddle_position < 0.0 ) // if we're fully open
		  {
			 cfg.paddle_position = 0.0;
			 cfg.paddles = PADDLE_OPEN; // change state
		  }
		
		// drop the thing at the head of the stack
		if( cfg.grip_stack &&  
			 (cfg.paddle_position == 0.0 || cfg.paddle_position < cfg.close_limit ))
		  {
			 Model* head = (Model*)cfg.grip_stack->data;
			 cfg.grip_stack = g_list_remove( cfg.grip_stack, head );
			 
			 // move it to the new location
			 head->SetParent( NULL );	      
			 head->SetPose( this->GetGlobalPose() );
			 
			 cfg.close_limit = 1.0;
		  }
	 }

  else if( cfg.paddles == PADDLE_CLOSING && !cfg.paddles_stalled  )
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

ModelGripper::data_t ModelGripper::GetData()
{
  data_t data;
  
  data.paddles = cfg.paddles;
  data.paddle_position = cfg.paddle_position;
  data.lift = cfg.lift;
  data.lift_position = cfg.lift_position;
  data.beam[0] = cfg.beam[0];
  data.beam[1] = cfg.beam[1];

  data.contact[0] = cfg.contact[0];
  data.contact[1] = cfg.contact[1];

  if( cfg.grip_stack )
	 data.stack_count = g_list_length( cfg.grip_stack );
  else
	 data.stack_count = 0;
    
   data.paddles_stalled = cfg.paddles_stalled;
	
	return data;
}


void ModelGripper::SetConfig( config_t & cfg )
{
  this->cfg = cfg;
}

	
static bool gripper_raytrace_match( Model* hit, 
												Model* finder,
												const void* dummy )
{
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
		
		stg_raytrace_result_t sample = 
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
  
  stg_raytrace_result_t leftsample = 
	 Raytrace( lpz,  // ray origin
				  bbr, // range
				  gripper_raytrace_match, // match func
				  NULL, // match arg
				  true ); // ztest
  
  cfg.contact[0] = leftsample.mod;
  
  stg_raytrace_result_t rightsample = 
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
			 
			 if( cfg.grip_stack_size > 0 && 
				  (g_list_length( cfg.grip_stack ) < cfg.grip_stack_size) )
				{
				  // get the global pose of the gripper for calculations of the gripped object position
				  // and move it to be right between the paddles
				  Geom hitgeom = hit->GetGeom();
				  //Pose hitgpose = hit->GetGlobalPose();
				  
				  //       stg_pose_t pose = {0,0,0};
				  //       stg_model_local_to_global( lhit, &pose );
				  //       stg_model_global_to_local( mod, &pose );
				  
				  //       // grab the model we hit - very simple grip model for now
				  hit->SetParent( this );
				  hit->SetPose( Pose(0,0, -1.0 * geom.size.z ,0) );
				  
				  cfg.grip_stack = g_list_prepend( cfg.grip_stack, hit );
				  
				  //       // calculate how far closed we can get the paddles now
				  double puckw = hitgeom.size.y;
				  double gripperw = geom.size.y;	      
				  cfg.close_limit = MAX( 0.0, 1.0 - puckw/(gripperw - cfg.paddle_size.y/2.0 ));
				}
		  }
	 }
}

// int gripper_render_data(  stg_model_t* mod, void* userp )
// {
//   //puts( "gripper render data" );

//   // only draw if someone is using the gripper
//   if( mod->subs < 1 )
//     return 0;

//   stg_rtk_fig_t* fig = stg_model_get_fig( mod, "gripper_data_fig" );  
  
//   if( ! fig )
//     {
//       fig = stg_model_fig_create( mod, "gripper_data_fig", "top", STG_LAYER_GRIPPERDATA );
//       //stg_rtk_fig_color_rgb32( fig, gripper_col );
//       stg_rtk_fig_color_rgb32( fig, 0xFF0000 ); // red
  
//     }
//   else
//     stg_rtk_fig_clear( fig );

//   //printf( "SUBS %d\n", mod->subs );
  
//   stg_gripper_data_t* data = (stg_gripper_data_t*)mod->data;
//   assert(data);
  
//   stg_gripper_config_t *cfg = (stg_gripper_config_t*)mod->cfg;
//   assert(cfg);

//   stg_geom_t *geom = &mod->geom;  
  
//   //stg_rtk_fig_rectangle( gui->data, 0,0,0, geom.size.x, geom.size.y, 0 );
  
//   // different x location for each beam
//   double ibbx =  (geom->size.x - cfg->inner_break_beam_inset * geom->size.x) - geom->size.x/2.0;
//   double obbx =  (geom->size.x - cfg->outer_break_beam_inset * geom->size.x) - geom->size.x/2.0;
  
//   // common y position
//   double bby = 
//     (1.0-data->paddle_position) * ((geom->size.y/2.0)-(geom->size.y*cfg->paddle_size.y));
  
//   // size of the paddle indicator lights
//   double led_dx = cfg->paddle_size.y * 0.5 * geom->size.y;
  
  
//   if( data->inner_break_beam )
// 	{
// 	  stg_rtk_fig_rectangle( fig, ibbx, bby+led_dx, 0, led_dx, led_dx, 1 );
// 	  stg_rtk_fig_rectangle( fig, ibbx, -bby-led_dx, 0, led_dx, led_dx, 1 );
// 	}
//       else
// 	{
// 	  stg_rtk_fig_line( fig, ibbx, bby, ibbx, -bby );
// 	  stg_rtk_fig_rectangle( fig, ibbx, bby+led_dx, 0, led_dx, led_dx, 0 );
// 	  stg_rtk_fig_rectangle( fig, ibbx, -bby-led_dx, 0, led_dx, led_dx, 0 );
// 	}
      
//       if( data->outer_break_beam )
// 	{
// 	  stg_rtk_fig_rectangle( fig, obbx, bby+led_dx, 0, led_dx, led_dx, 1 );
// 	  stg_rtk_fig_rectangle( fig, obbx, -bby-led_dx, 0, led_dx, led_dx, 1 );
// 	}
//       else
// 	{
// 	  stg_rtk_fig_line( fig, obbx, bby, obbx, -bby );
// 	  stg_rtk_fig_rectangle( fig, obbx, bby+led_dx, 0, led_dx, led_dx, 0 );
// 	  stg_rtk_fig_rectangle( fig, obbx, -bby-led_dx, 0, led_dx, led_dx, 0 );
// 	}
      
//       // draw the contact indicators
//       stg_rtk_fig_rectangle( fig, 
// 			     ((1.0 - cfg->paddle_size.x/2.0) * geom->size.x) - geom->size.x/2.0,
// 			     (1.0 - cfg->paddle_position) * ((geom->size.y/2.0)-(geom->size.y*cfg->paddle_size.y)),
// 			     0.0,
// 			     cfg->paddle_size.x * geom->size.x,
// 			     cfg->paddle_size.y/6.0 * geom->size.y, 
// 			     data->paddle_contacts[0] );
      
//       stg_rtk_fig_rectangle( fig, 
// 			     ((1.0 - cfg->paddle_size.x/2.0) * geom->size.x) - geom->size.x/2.0,
// 			     (1.0 - cfg->paddle_position) * -((geom->size.y/2.0)-(geom->size.y*cfg->paddle_size.y)),
// 			     0.0,
// 			     cfg->paddle_size.x * geom->size.x,
// 			     cfg->paddle_size.y/6.0 * geom->size.y, 
// 			     data->paddle_contacts[1] );
      
//       //stg_rtk_fig_color_rgb32( fig,  gripper_col );      

//       return 0; 
// }


// int gripper_render_cfg( stg_model_t* mod, void* user )
// { 
//   puts( "gripper render cfg" );
//   stg_rtk_fig_t* fig = stg_model_get_fig( mod, "gripper_cfg_fig" );  
  
//   if( ! fig )
//     {
//       fig = stg_model_fig_create( mod, "gripper_cfg_fig", "top", 
// 				  STG_LAYER_GRIPPERCONFIG );
      
//       stg_rtk_fig_color_rgb32( fig, stg_lookup_color( STG_GRIPPER_CFG_COLOR ));
//     }
//   else
//     stg_rtk_fig_clear( fig );
  
//   stg_geom_t geom;
//   stg_model_get_geom( mod, &geom );
  
//   // get the config and make sure it's the right size
//   stg_gripper_config_t* cfg = (stg_gripper_config_t*)mod->cfg;
//   assert( mod->cfg_len == sizeof(stg_gripper_config_t) );
  
//   // different x location for each beam
//   double ibbx = (cfg->inner_break_beam_inset) * geom.size.x - geom.size.x/2.0;
//   double obbx = (cfg->outer_break_beam_inset) * geom.size.x - geom.size.x/2.0;
  
//   // common y position
//   double bby = 
//     (1.0-cfg->paddle_position) * ((geom.size.y/2.0)-(geom.size.y*cfg->paddle_size.y));
  
//   // draw the position of the break beam sensors
//   stg_rtk_fig_rectangle( fig, ibbx, bby, 0, 0.01, 0.01, 0 );
//   stg_rtk_fig_rectangle( fig, obbx, bby, 0, 0.01, 0.01, 0 );

//   return 0; //ok
// }


// int gripper_startup( stg_model_t* mod )
// { 
//   PRINT_DEBUG( "gripper startup" );
//   stg_model_set_watts( mod, STG_GRIPPER_WATTS );
//   return 0; // ok
// }

// int gripper_shutdown( stg_model_t* mod )
// { 
//   PRINT_DEBUG( "gripper shutdown" );
//   stg_model_set_watts( mod, 0 );
  
//   // unrender the break beams & lights
//   stg_model_fig_clear( mod, "gripper_data_fig" );
//   return 0; // ok
// }

// void stg_print_gripper_config( stg_gripper_config_t* cfg ) 
// {
//   char* pstate;
//   switch( cfg->paddles )
//     {
//     case STG_GRIPPER_PADDLE_OPEN: pstate = "OPEN"; break;
//     case STG_GRIPPER_PADDLE_CLOSED: pstate = "CLOSED"; break;
//     case STG_GRIPPER_PADDLE_OPENING: pstate = "OPENING"; break;
//     case STG_GRIPPER_PADDLE_CLOSING: pstate = "CLOSING"; break;
//     default: pstate = "*unknown*";
//     }
  
//   char* lstate;
//   switch( cfg->lift )
//     {
//     case STG_GRIPPER_LIFT_UP: lstate = "UP"; break;
//     case STG_GRIPPER_LIFT_DOWN: lstate = "DOWN"; break;
//     case STG_GRIPPER_LIFT_DOWNING: lstate = "DOWNING"; break;
//     case STG_GRIPPER_LIFT_UPPING: lstate = "UPPING"; break;
//     default: lstate = "*unknown*";
//     }
  
//   printf("gripper state: paddles(%s)[%.2f] lift(%s)[%.2f] stall(%s)\n", 
// 	 pstate, cfg->paddle_position, lstate, cfg->lift_position,
// 	 cfg->paddles_stalled ? "true" : "false" );
// }


// int gripper_unrender_data( stg_model_t* mod, void* userp )
// {
//   stg_model_fig_clear( mod, "gripper_data_fig" );
//   return 1; // callback just runs one time
// }

// int gripper_unrender_cfg( stg_model_t* mod, void* userp )
// {
//   stg_model_fig_clear( mod, "gripper_cfg_fig" );
//   return 1; // callback just runs one time
// }


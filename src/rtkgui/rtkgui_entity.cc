
#define DEBUG

#include <assert.h>
#include <string.h>
#include "math.h"
#include "entity.hh"
#include "rtkgui.hh"

extern rtk_canvas_t* canvas;
extern rtk_app_t* app;

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
int CEntity::RtkStartup()
{
  PRINT_DEBUG2( "RTKSTARTUP ent %d (%s)", this->stage_id, this->token );
  
  if( canvas == NULL )
    {
      PRINT_WARN1( "rtk startup for ent %d no canvas!", this->stage_id );
      return 0; // it's ok though..
    }

  if( this->fig )
    {
      PRINT_WARN2( " fig already exists for  ent %d (%s). deleting it.", 
		   this->stage_id, this->token );
      
      rtk_fig_destroy( this->fig );
      fig_count--;
    }

  // Create a figure representing this entity
  if( m_parent_entity == NULL )
    this->fig = rtk_fig_create( canvas, NULL, 50);
  else
    this->fig = rtk_fig_create( canvas, m_parent_entity->fig, 50);

  assert( this->fig );

  fig_count++;
  
  PRINT_DEBUG3( "FIG COUNT %d for  ent %d (%s)", 
	       fig_count, this->stage_id, this->token );
  
  // Set the mouse handler
  this->fig->userdata = this;
  rtk_fig_add_mouse_handler(this->fig, RtkOnMouse);

  // add this device to the world's device menu 
  //this->m_world->AddToDeviceMenu( this, true); 
    
  // visible by default
  rtk_fig_show( this->fig, true );

  // Set the color
  rtk_fig_color_rgb32(this->fig, this->color);

  if( m_parent_entity == NULL )
    // the root device is drawn from the corner, not the center
    rtk_fig_origin( this->fig, local_px, local_py, local_pth );
  else
    // put the figure's origin at the entity's position
    rtk_fig_origin( this->fig, local_px, local_py, local_pth );



#ifdef RENDER_INITIAL_BOUNDING_BOXES
  double xmin, ymin, xmax, ymax;
  xmin = ymin = 999999.9;
  xmax = ymax = 0.0;
  this->GetBoundingBox( xmin, ymin, xmax, ymax );
  
  rtk_fig_t* boundaries = rtk_fig_create( canvas, NULL, 99);
  double width = xmax - xmin;
  double height = ymax - ymin;
  double xcenter = xmin + width/2.0;
  double ycenter = ymin + height/2.0;

  rtk_fig_rectangle( boundaries, xcenter, ycenter, 0, width, height, 0 ); 

   
#endif
   
  // everything except the root object has a label
  //if( m_parent_entity )
  //{
    // Create the label
    // By default, the label is not shown
    this->fig_label = rtk_fig_create( canvas, this->fig, 51);
    rtk_fig_show(this->fig_label, true); // TODO - re-hide the label    
    rtk_fig_movemask(this->fig_label, 0);
      
    char label[1024];
    char tmp[1024];
      
    label[0] = 0;
    snprintf(tmp, sizeof(tmp), "%s %s", 
             this->name, this->token );
    strncat(label, tmp, sizeof(label));
      
    rtk_fig_color_rgb32(this->fig, this->color);
    rtk_fig_text(this->fig_label,  0.75 * size_x,  0.75 * size_y, 0, label);
      
    // attach the label to the main figure
    // rtk will draw the label when the mouse goes over the figure
    // TODO: FIX
    //this->fig->mouseover_fig = fig_label;
    
    // we can be moved only if we are on the root node
    if (m_parent_entity != CEntity::root )
      rtk_fig_movemask(this->fig, 0);
    else
      rtk_fig_movemask(this->fig, this->movemask);  

    //}

 
  if( this->grid_enable )
    {
      this->fig_grid = rtk_fig_create(canvas, this->fig, -49);
      if ( this->grid_minor > 0)
	{
	  rtk_fig_color( this->fig_grid, 0.9, 0.9, 0.9);
	  rtk_fig_grid( this->fig_grid, this->origin_x, this->origin_y, 
			this->size_x, this->size_y, grid_minor);
	}
      if ( this->grid_major > 0)
	{
	  rtk_fig_color( this->fig_grid, 0.75, 0.75, 0.75);
	  rtk_fig_grid( this->fig_grid, this->origin_x, this->origin_y, 
			this->size_x, this->size_y, grid_major);
	}
      rtk_fig_show( this->fig_grid, 1);
    }
  else
    this->fig_grid = NULL;
  

  PRINT_DEBUG1( "rendering %d rectangles", this->rect_count );
  
  // now create GUI rects
  int r;
  for( r=0; r<this->rect_count; r++ )
    {
      stage_rotrect_t* src = &this->rects[r];
      double x,y,a,w,h;
      
      x = ((src->x + src->w/2.0) * this->size_x) - this->size_x/2.0 + origin_x;
      y = ((src->y + src->h/2.0) * this->size_y) - this->size_y/2.0 + origin_y;
      a = src->a;
      w = src->w * this->size_x;
      h = src->h * this->size_y;
      
      rtk_fig_rectangle(this->fig, x,y,a,w,h, 0 ); 
    }

  // add rects showing transducer positions
  for( int s=0; s< this->transducer_count; s++ )
    rtk_fig_rectangle( this->fig, 
		       this->transducers[s][0], 
		       this->transducers[s][1], 
		       this->transducers[s][2], this->size_x/10.0, this->size_y/10.0, 0 ); 

  
  // we're done - now do our childre
  CHILDLOOP( child ) 
    if( child->RtkStartup() == -1 )
      {
	PRINT_ERR( "failed to rtkstartup child" );
	return -1;
      }

  PRINT_DEBUG( "RTK STARTUP DONE" );

  return 0;
}



///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CEntity::RtkShutdown()
{
  PRINT_DEBUG2( "RTKSHUTDOWN ent %d (%s)", this->stage_id, this->token );

  // do our children first
  CHILDLOOP( child )
    child->RtkShutdown();
  
  // Clean up the figure we created
  if( this->fig ) 
    {
      rtk_fig_destroy(this->fig);
      this->fig = NULL;
     
      fig_count--;
      
      PRINT_DEBUG3( "FIG COUNT %d for  ent %d (%s)", 
		   fig_count, this->stage_id, this->token );
    }


  if( this->fig_label ) rtk_fig_destroy(this->fig_label);
  if( this->fig_grid ) rtk_fig_destroy( this->fig_grid );

  this->fig = NULL;
  this->fig_label = NULL;
  this->fig_grid = NULL;
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
int CEntity::RtkUpdate()
{
  PRINT_DEBUG1( "RTK update for ent %d", this->stage_id );
  //CHILDLOOP( child ) child->RtkUpdate();

  // TODO this is nasty and inefficient - figure out a better way to
  // do this  

  // if we're not looking at this device, hide it 
  //if( !m_world->ShowDeviceBody( this->lib_entry->type_num ) )
  //{
  //rtk_fig_show(this->fig, false);
  //}
  //else // we need to show and update this figure
  {
    rtk_fig_show( this->fig, true );  }
  PRINT_DEBUG1( "RTK update for ent %d done", this->stage_id );  

  return 0; // success
}



///////////////////////////////////////////////////////////////////////////
// Process mouse events 
void RtkOnMouse(rtk_fig_t *fig, int event, int mode)
{
  CEntity *entity = (CEntity*) fig->userdata;
  
  double px, py, pth;
  
  switch (event)
    {
    case RTK_EVENT_PRESS:
      break;

    case RTK_EVENT_MOTION:
    case RTK_EVENT_RELEASE:
      rtk_fig_get_origin(fig, &px, &py, &pth);
      entity->SetGlobalPose(px, py, pth);
      break;
      
    default:
      break;
    }
  return;
}

#include "sonar.hh"

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
int CSonarModel::RtkStartup()
{
  if( CEntity::RtkStartup() == -1 )
    {
      PRINT_ERR1( "model %d (sonar) base startup failed", this->stage_id );
      return -1;
    }
  
  // there still might not be a canvas
  if( canvas )
    {
      // Create a figure representing this object
      this->scan_fig = rtk_fig_create( canvas, this->fig, 49);
      assert( scan_fig );

      // Set the color
      rtk_fig_color_rgb32(this->scan_fig, 0xCCCCCC );
    }

  return 0; //success
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CSonarModel::RtkShutdown()
{
  // Clean up the figure we created
  if( this->scan_fig) 
    {
      rtk_fig_destroy(this->scan_fig);
      this->scan_fig = NULL;
    }
  
  CEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
int CSonarModel::RtkUpdate()
{
  if( CEntity::RtkUpdate() == -1 )
    {
      PRINT_ERR1( "model %d (sonar) base update failed", this->stage_id );
      return -1;
    } 
  if( this->scan_fig )
    {
      rtk_fig_clear(this->scan_fig);
 
      double* ranges = new double[transducer_count];
      size_t len = transducer_count * sizeof(ranges[0]);
      size_t gotlen = len;

      if( IsSubscribed( STG_PROP_ENTITY_DATA ) )//&& ShowDeviceData( this->lib_entry->type_num) )
	{
	  if( this->GetData( (char*)ranges, &gotlen ) == 0 )
	    {
	      assert( len == gotlen );

	      for (int s = 0; s < transducer_count; s++)
		{
		  // draw an arrow from the sonar transducer to it's hit point
		  rtk_fig_arrow(this->scan_fig, 
				this->transducers[s][0], 
				this->transducers[s][1], 
				this->transducers[s][2],
				ranges[s], 0.05 );
		  
		}
	    }
	}
    }

  return 0; //success
}

// IDAR ------------------------------------------------------------------
#include "idar.hh"

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
int CIdarModel::RtkStartup()
{
  if( CEntity::RtkStartup() == -1 )
    {
      PRINT_ERR( "idar failed to rtkstartup base" );
      return -1;
    }
  
  if( canvas )
    {
      // Create a figure representing this object
      this->data_fig = rtk_fig_create( canvas, this->fig, 49);
      this->rays_fig = rtk_fig_create( canvas, NULL, 48);
      
      rtk_fig_origin(this->data_fig, 0,0,0 );
      rtk_fig_origin(this->rays_fig, 0,0,0 );
      
      // show our orientation in the body figure
      rtk_fig_line( this->fig, 0,0, size_x/2.0, 0);

      // Set the color
      rtk_fig_color_rgb32(this->data_fig, this->color);
      rtk_fig_color_rgb32(this->rays_fig, LookupColor( "gray50") );      

  

      // add rects showing transducer positions
      for( int s=0; s< this->transducer_count; s++ )
	{
	  this->incoming_figs[s] = rtk_fig_create( canvas, this->fig, 60 );
	  this->outgoing_figs[s] = rtk_fig_create( canvas, this->fig, 60 );
	  
	  // draw an outgoing data arrow in green
	  rtk_fig_color_rgb32(this->outgoing_figs[s], 0x00AA00 );
	  rtk_fig_arrow(this->outgoing_figs[s], 
			this->transducers[s][0], 
			this->transducers[s][1], 
			this->transducers[s][2], 0.3 * size_x, 0.1 * size_y );
	  
	  rtk_fig_show( this->outgoing_figs[s], 0 ); // hide it
	  
	  // draw an incoming data arrow in red
	  rtk_fig_color_rgb32(this->incoming_figs[s], 0xAA0000 );
	  rtk_fig_arrow(this->incoming_figs[s], this->transducers[s][0], 
			this->transducers[s][1], 
			this->transducers[s][2] - M_PI, -0.1 * size_x, 0.1 * size_y );
	  
	  rtk_fig_show( this->incoming_figs[s], 0 ); // hide it
	}
    }
  return 0;
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CIdarModel::RtkShutdown()
{
  // Clean up the figure we created
  for( int s=0; s< this->transducer_count; s++ )
    {
      if(this->incoming_figs[s]) rtk_fig_destroy(this->incoming_figs[s]);
      if(this->outgoing_figs[s]) rtk_fig_destroy(this->outgoing_figs[s]);
    }
  
  if(this->data_fig) rtk_fig_destroy(this->data_fig);
  if(this->rays_fig) rtk_fig_destroy(this->rays_fig);
  
  this->data_fig = NULL;
  this->rays_fig = NULL;
  this->outgoing_figs[0] = NULL;
  this->incoming_figs[0] = NULL;
  
  CEntity::RtkShutdown();
} 


void CIdarModel::RtkShowReceived( void )
{
  for( int s=0; s < this->transducer_count; s++ )
    rtk_canvas_flash( canvas, this->incoming_figs[s], 30 );
}

void CIdarModel::RtkShowSent( void )
{
  for( int s=0; s < this->transducer_count; s++ )
    rtk_canvas_flash( canvas, this->outgoing_figs[s], 30 );
}



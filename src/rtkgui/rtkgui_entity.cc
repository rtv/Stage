
#include <assert.h>
#include <string.h>
#include "math.h"
#include "entity.hh"
#include "rtkgui.hh"

extern rtk_canvas_t* canvas;
extern rtk_app_t* app;


///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CEntity::RtkStartup()
{
  // Create a figure representing this entity
  if( m_parent_entity == NULL )
    this->fig = rtk_fig_create( canvas, NULL, 50);
  else
    this->fig = rtk_fig_create( canvas, m_parent_entity->fig, 50);

  assert( this->fig );

  // Set the mouse handler
  this->fig->userdata = this;
  rtk_fig_add_mouse_handler(this->fig, StaticRtkOnMouse);

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
  
  double scalex = size_x / rects_max_x;
  double scaley = size_y / rects_max_y;
  
  //rtk_fig_origin( this->fig, local_px, local_py, local_pth );
  
  // now create GUI rects
  int r;
  for( r=0; r<this->rect_count; r++ )
    {
      double x, y, a, w, h;
      
      x = ((this->rects[r].x + this->rects[r].w/2.0) * scalex) - size_x/2.0;
      y = ((this->rects[r].y + this->rects[r].h/2.0)* scaley) - size_y/2.0;
      a = this->rects[r].a;
      w = this->rects[r].w * scalex;
      h = this->rects[r].h * scaley;
      
      rtk_fig_rectangle(this->fig, x, y, a, w, h, 0 ); 
    }

  // draw a boundary rectangle around the root device
  if( this == CEntity::root )
    rtk_fig_rectangle( this->fig, size_x/2.0, size_y/2.0, 0.0, 
		       size_x, size_y, 0 );
  
  // do our children after we're set
  CHILDLOOP( child ) 
    child->RtkStartup();

  PRINT_DEBUG( "RTK STARTUP DONE" );
}



///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CEntity::RtkShutdown()
{
  PRINT_DEBUG1( "RTK SHUTDOWN %d", this->stage_id );
  
  // do our children first
  CHILDLOOP( child )
    child->RtkShutdown();

  // Clean up the figure we created
  if( this->fig ) rtk_fig_destroy(this->fig);
  if( this->fig_label ) rtk_fig_destroy(this->fig_label);
  if( this->fig_grid ) rtk_fig_destroy( this->fig_grid );

  this->fig = NULL;
  this->fig_label = NULL;
  this->fig_grid = NULL;
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CEntity::RtkUpdate()
{
  PRINT_WARN1( "RTK update for ent %d", this->stage_id );
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
}


///////////////////////////////////////////////////////////////////////////
// Process mouse events
void CEntity::RtkOnMouse(rtk_fig_t *fig, int event, int mode)
{
  double px, py, pth;

  switch (event)
  {
    case RTK_EVENT_PRESS:
    case RTK_EVENT_MOTION:
    case RTK_EVENT_RELEASE:
      rtk_fig_get_origin(fig, &px, &py, &pth);
      this->SetGlobalPose(px, py, pth);
      break;

    default:
      break;
  }

  return;
}


///////////////////////////////////////////////////////////////////////////
// Process mouse events (static callback)
void CEntity::StaticRtkOnMouse(rtk_fig_t *fig, int event, int mode)
{
  CEntity *entity;
  entity = (CEntity*) fig->userdata;
  entity->RtkOnMouse(fig, event, mode);
  return;
}

#include "sonar.hh"

///////////////////////////////////////////////////////////////////////////
// Initialise the rtk gui
void CSonarModel::RtkStartup()
{
  CEntity::RtkStartup();
  
  // Create a figure representing this object
  this->scan_fig = rtk_fig_create( canvas, NULL, 49);

  // Set the color
  rtk_fig_color_rgb32(this->scan_fig, this->color);
}


///////////////////////////////////////////////////////////////////////////
// Finalise the rtk gui
void CSonarModel::RtkShutdown()
{
  // Clean up the figure we created
  rtk_fig_destroy(this->scan_fig);

  CEntity::RtkShutdown();
} 


///////////////////////////////////////////////////////////////////////////
// Update the rtk gui
void CSonarModel::RtkUpdate()
{
  CEntity::RtkUpdate();

  rtk_fig_clear(this->scan_fig);

  double ranges[SONARSAMPLES];
  size_t len = SONARSAMPLES * sizeof(ranges[0]);
  
  if( IsSubscribed( STG_PROP_ENTITY_DATA ) )//&& ShowDeviceData( this->lib_entry->type_num) )
    {
      if( this->GetData( (char*)ranges, &len ) == 0 )
	{
	  int range_count = len / sizeof(ranges[0]);
	  
	  for (int s = 0; s < range_count; s++)
	    {
	      // draw a line from the position of the sonar sensor...
	      double ox = this->sonars[s][0];
	      double oy = this->sonars[s][1];
	      double oth = this->sonars[s][2];
	      LocalToGlobal(ox, oy, oth);
	      
	      // ...out to the range indicated by the data
	      double x1 = ox;
	      double y1 = oy;
	      double x2 = x1 + ranges[s] * cos(oth); 
	      double y2 = y1 + ranges[s] * sin(oth);       
	      
	      //PRINT_WARN4( "line: %.2f %.2f to  %.2f %.2f",
	      //   x1, y1, x2, y2 );

	      rtk_fig_line(this->scan_fig, x1, y1, x2, y2 );
	    }
	}
    }
}


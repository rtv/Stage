#include "stage.hh"
#include "worldfile.hh"
#include "canvas.hh"
#include "texture_manager.hh"
using namespace Stg;

// speech bubble colors
static const stg_color_t BUBBLE_FILL = 0xFFC8C8FF; // light blue/grey
static const stg_color_t BUBBLE_BORDER = 0xFF000000; // black
static const stg_color_t BUBBLE_TEXT = 0xFF000000; // black

void Model::DrawSelected()
{
  glPushMatrix();
  
  glTranslatef( pose.x, pose.y, pose.z+0.01 ); // tiny Z offset raises rect above grid
  
  Pose gpose = GetGlobalPose();
  
  char buf[64];
  snprintf( buf, 63, "%s [%.2f %.2f %.2f %.2f]", 
	    token, gpose.x, gpose.y, gpose.z, rtod(gpose.a) );
  
  PushColor( 0,0,0,1 ); // text color black
  Gl::draw_string( 0.5,0.5,0.5, buf );
  
  glRotatef( rtod(pose.a), 0,0,1 );
  
  Gl::pose_shift( geom.pose );
  
  double dx = geom.size.x / 2.0 * 1.6;
  double dy = geom.size.y / 2.0 * 1.6;
  
  PopColor();

  PushColor( 0,1,0,0.4 ); // highlight color blue
  glRectf( -dx, -dy, dx, dy );
  PopColor();

  PushColor( 0,1,0,0.8 ); // highlight color blue
  glLineWidth( 1 );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glRectf( -dx, -dy, dx, dy );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  PopColor();

  glPopMatrix();
}


void Model::DrawTrailFootprint()
{
  double r,g,b,a;

  for( int i=trail->len-1; i>=0; i-- )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      glPushMatrix();
		Gl::pose_shift( checkpoint->pose );
		Gl::pose_shift( geom.pose );

      stg_color_unpack( checkpoint->color, &r, &g, &b, &a );
      PushColor( r, g, b, 0.1 );

      blockgroup.DrawFootPrint( geom );

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      PushColor( r/2, g/2, b/2, 0.1 );

      blockgroup.DrawFootPrint( geom );

      PopColor();
      PopColor();
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glPopMatrix();
    }
}

void Model::DrawTrailBlocks()
{
  double timescale = 0.0000001;

  for( int i=trail->len-1; i>=0; i-- )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      Pose pose;
      memcpy( &pose, &checkpoint->pose, sizeof(pose));
      pose.z =  (world->sim_time - checkpoint->time) * timescale;

      PushLocalCoords();
      //blockgroup.Draw( this );

      DrawBlocksTree();
      PopCoords();
    }
}

void Model::DrawTrailArrows()
{
  double r,g,b,a;

  double dx = 0.2;
  double dy = 0.07;
  double timescale = 0.0000001;

  for( unsigned int i=0; i<trail->len; i++ )
    {
      stg_trail_item_t* checkpoint = & g_array_index( trail, stg_trail_item_t, i );

      Pose pose;
      memcpy( &pose, &checkpoint->pose, sizeof(pose));
      pose.z =  (world->sim_time - checkpoint->time) * timescale;

      PushLocalCoords();

      // set the height proportional to age

      PushColor( checkpoint->color );

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0, 1.0);

      glBegin( GL_TRIANGLES );
      glVertex3f( 0, -dy, 0);
      glVertex3f( dx, 0, 0 );
      glVertex3f( 0, +dy, 0 );
      glEnd();
      glDisable(GL_POLYGON_OFFSET_FILL);

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

      stg_color_unpack( checkpoint->color, &r, &g, &b, &a );
      PushColor( r/2, g/2, b/2, 1 ); // darker color

      glDepthMask(GL_FALSE);
      glBegin( GL_TRIANGLES );
      glVertex3f( 0, -dy, 0);
      glVertex3f( dx, 0, 0 );
      glVertex3f( 0, +dy, 0 );
      glEnd();
      glDepthMask(GL_TRUE);

      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      PopColor();
      PopColor();
      PopCoords();
    }
}

void Model::DrawOriginTree()
{
  DrawPose( GetGlobalPose() );  
  for( GList* it=children; it; it=it->next )
    ((Model*)it->data)->DrawOriginTree();
}
 

void Model::DrawBlocksTree( )
{
  PushLocalCoords();
  LISTMETHOD( children, Model*, DrawBlocksTree );
  DrawBlocks();  
  PopCoords();
}
  
void Model::DrawPose( Pose pose )
{
  PushColor( 0,0,0,1 );
  glPointSize( 4 );
  
  glBegin( GL_POINTS );
  glVertex3f( pose.x, pose.y, pose.z );
  glEnd();
  
  PopColor();  
}

void Model::DrawBlocks( )
{ 
  blockgroup.CallDisplayList( this );
}

void Model::DrawBoundingBoxTree()
{
  PushLocalCoords();
  LISTMETHOD( children, Model*, DrawBoundingBoxTree );
  DrawBoundingBox();
  PopCoords();
}

void Model::DrawBoundingBox()
{
  Gl::pose_shift( geom.pose );  

  PushColor( color );
  
  glBegin( GL_QUAD_STRIP );
  
  glVertex3f( -geom.size.x/2.0, -geom.size.y/2.0, geom.size.z );
  glVertex3f( -geom.size.x/2.0, -geom.size.y/2.0, 0 );
 
  glVertex3f( +geom.size.x/2.0, -geom.size.y/2.0, geom.size.z );
  glVertex3f( +geom.size.x/2.0, -geom.size.y/2.0, 0 );
 
  glVertex3f( +geom.size.x/2.0, +geom.size.y/2.0, geom.size.z );
  glVertex3f( +geom.size.x/2.0, +geom.size.y/2.0, 0 );

  glVertex3f( +geom.size.x/2.0, +geom.size.y/2.0, geom.size.z );
  glVertex3f( +geom.size.x/2.0, +geom.size.y/2.0, 0 );

  glVertex3f( -geom.size.x/2.0, +geom.size.y/2.0, geom.size.z );
  glVertex3f( -geom.size.x/2.0, +geom.size.y/2.0, 0 );

  glVertex3f( -geom.size.x/2.0, -geom.size.y/2.0, geom.size.z );
  glVertex3f( -geom.size.x/2.0, -geom.size.y/2.0, 0 );

  glEnd();

  glBegin( GL_LINES );
  glVertex2f( -0.02, 0 ); 
  glVertex2f( +0.02, 0 ); 

  glVertex2f( 0, -0.02 ); 
  glVertex2f( 0, +0.02 ); 
  glEnd();

  PopColor();
}

// move into this model's local coordinate frame
void Model::PushLocalCoords()
{
  glPushMatrix();  
  
  if( parent )
    glTranslatef( 0,0, parent->geom.size.z );
  
  Gl::pose_shift( pose );
}

void Model::PopCoords()
{
  glPopMatrix();
}

void Model::AddVisualizer( Visualizer* custom_visual, bool on_by_default )
{
	if( !custom_visual )
		return;

	//Visualizations can only be added to stage when run in a GUI
	if( world_gui == NULL ) 
	  {
		 printf( "Unable to add custom visualization - it must be run with a GUI world\n" );
		 return;
	  }
	
	//save visual instance
	custom_visual_list = g_list_append(custom_visual_list, custom_visual );

	//register option for all instances which share the same name
	Canvas* canvas = world_gui->GetCanvas();
	std::map< std::string, Option* >::iterator i = canvas->_custom_options.find( custom_visual->GetMenuName() );
	if( i == canvas->_custom_options.end() ) {
	  Option* op = new Option( custom_visual->GetMenuName(), 
										custom_visual->GetWorldfileName(), 
										"", 
										on_by_default, 
										world_gui );
		canvas->_custom_options[ custom_visual->GetMenuName() ] = op;
		RegisterOption( op );
	}
}

void Model::RemoveVisualizer( Visualizer* custom_visual )
{
	if( custom_visual )
		custom_visual_list = g_list_remove(custom_visual_list, custom_visual );

	//TODO unregister option - tricky because there might still be instances attached to different models which have the same name
}


void Model::DrawStatusTree( Camera* cam ) 
{
  PushLocalCoords();
  DrawStatus( cam );
  LISTMETHODARG( children, Model*, DrawStatusTree, cam );  
  PopCoords();
}

void Model::DrawStatus( Camera* cam ) 
{
  // quick hack
//   if( power_pack && power_pack->stored < 0.0 )
// 	 {
//       glPushMatrix();
//       glTranslatef( 0.3, 0, 0.0 );		
// 		DrawImage( TextureManager::getInstance()._mains_texture_id, cam, 0.85 );
// 		glPopMatrix();
// 	 }

  if( say_string || power_pack )	  
    {
      float yaw, pitch;
      pitch = - cam->pitch();
      yaw = - cam->yaw();			
      
		Pose gpz = GetGlobalPose();

      float robotAngle = -rtod(gpz.a);
      glPushMatrix();
				
      // move above the robot
      glTranslatef( 0, 0, 0.5 );		
		
      // rotate to face screen
      glRotatef( robotAngle - yaw, 0,0,1 );
      glRotatef( -pitch, 1,0,0 );

		if( power_pack )
		  power_pack->Visualize( cam );

		if( say_string )
		  {
			 glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

			 //get raster positition, add gl_width, then project back to world coords
			 glRasterPos3f( 0, 0, 0 );
			 GLfloat pos[ 4 ];
			 glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
			 
			 GLboolean valid;
			 glGetBooleanv( GL_CURRENT_RASTER_POSITION_VALID, &valid );
			 
			 if( valid ) 
				{				  
				  //fl_font( FL_HELVETICA, 12 );
				  float w = gl_width( this->say_string ); // scaled text width
				  float h = gl_height(); // scaled text height
				  
				  GLdouble wx, wy, wz;
				  GLint viewport[4];
				  glGetIntegerv(GL_VIEWPORT, viewport);
				  
				  GLdouble modelview[16];
				  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
				  
				  GLdouble projection[16];	
				  glGetDoublev(GL_PROJECTION_MATRIX, projection);
				  
				  //get width and height in world coords
				  gluUnProject( pos[0] + w, pos[1], pos[2], modelview, projection, viewport, &wx, &wy, &wz );
				  w = wx;
				  gluUnProject( pos[0], pos[1] + h, pos[2], modelview, projection, viewport, &wx, &wy, &wz );
				  h = wy;
				  
				  // calculate speech bubble margin
				  const float m = h/10;
				  
				  // draw inside of bubble
				  PushColor( BUBBLE_FILL );
				  glPushAttrib( GL_POLYGON_BIT | GL_LINE_BIT );
				  glPolygonMode( GL_FRONT, GL_FILL );
				  glEnable( GL_POLYGON_OFFSET_FILL );
				  glPolygonOffset( 1.0, 1.0 );
				  Gl::draw_octagon( w, h, m );
				  glDisable( GL_POLYGON_OFFSET_FILL );
				  PopColor();
				  
				  // draw outline of bubble
				  PushColor( BUBBLE_BORDER );
				  glLineWidth( 1 );
				  glEnable( GL_LINE_SMOOTH );
				  glPolygonMode( GL_FRONT, GL_LINE );
				  Gl::draw_octagon( w, h, m );
				  glPopAttrib();
				  PopColor();
				  
				  PushColor( BUBBLE_TEXT );
				  // draw text inside the bubble
				  Gl::draw_string( m, 2.5*m, 0, this->say_string );
				  PopColor();			
				}
		  }
		glPopMatrix();
	 }
  
  if( stall )
    {
      DrawImage( TextureManager::getInstance()._stall_texture_id, cam, 0.85 );
    }
}

void Model::DrawImage( uint32_t texture_id, Camera* cam, float alpha, double width, double height )
{
  float yaw, pitch;
  pitch = - cam->pitch();
  yaw = - cam->yaw();
  float robotAngle = -rtod(pose.a);

  glEnable(GL_TEXTURE_2D);
  glBindTexture( GL_TEXTURE_2D, texture_id );

  glColor4f( 1.0, 1.0, 1.0, alpha );
  glPushMatrix();

  //position image above the robot
  glTranslatef( 0.0, 0.0, ModelHeight() + 0.3 );

  // rotate to face screen
  glRotatef( robotAngle - yaw, 0,0,1 );
  glRotatef( -pitch - 90, 1,0,0 );

  //draw a square, with the textured image
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.25f, 0, -0.25f );
  glTexCoord2f(width, 0.0f); glVertex3f( 0.25f, 0, -0.25f );
  glTexCoord2f(width, height); glVertex3f( 0.25f, 0,  0.25f );
  glTexCoord2f(0.0f, height); glVertex3f(-0.25f, 0,  0.25f );
  glEnd();

  glPopMatrix();
  glBindTexture( GL_TEXTURE_2D, 0 );
  glDisable(GL_TEXTURE_2D);
}


void Model::DrawFlagList( void )
{	
  if( flag_list  == NULL )
    return;
  
  PushLocalCoords();
  
  glPolygonMode( GL_FRONT, GL_FILL );

  GLUquadric* quadric = gluNewQuadric();
  glTranslatef(0,0,1); // jump up
  Pose gpose = GetGlobalPose();
  glRotatef( 180 + rtod(-gpose.a),0,0,1 );
  

  GList* list = g_list_copy( flag_list );
  list = g_list_reverse(list);
  
  for( GList* item = list; item; item = item->next )
    {
		
      Flag* flag = (Flag*)item->data;
		
      glTranslatef( 0, 0, flag->size/2.0 );
				
      PushColor( flag->color );
		

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0, 1.0);
      gluQuadricDrawStyle( quadric, GLU_FILL );
      gluSphere( quadric, flag->size/2.0, 4,2  );
		glDisable(GL_POLYGON_OFFSET_FILL);

      // draw the edges darker version of the same color
      double r,g,b,a;
      stg_color_unpack( flag->color, &r, &g, &b, &a );
      PushColor( stg_color_pack( r/2.0, g/2.0, b/2.0, a ));
		
      gluQuadricDrawStyle( quadric, GLU_LINE );
      gluSphere( quadric, flag->size/2.0, 4,2 );
		
      PopColor();
      PopColor();
		
      glTranslatef( 0, 0, flag->size/2.0 );
    }
  
  g_list_free( list );
  
  gluDeleteQuadric( quadric );
  
  PopCoords();
}


void Model::DrawBlinkenlights()
{
  PushLocalCoords();

  GLUquadric* quadric = gluNewQuadric();
  //glTranslatef(0,0,1); // jump up
  //Pose gpose = GetGlobalPose();
  //glRotatef( 180 + rtod(-gpose.a),0,0,1 );

  for( unsigned int i=0; i<blinkenlights->len; i++ )
    {
      stg_blinkenlight_t* b = 
	(stg_blinkenlight_t*)g_ptr_array_index( blinkenlights, i );
      assert(b);

      glTranslatef( b->pose.x, b->pose.y, b->pose.z );

      PushColor( b->color );

      if( b->enabled )
	gluQuadricDrawStyle( quadric, GLU_FILL );
      else
	gluQuadricDrawStyle( quadric, GLU_LINE );

      gluSphere( quadric, b->size/2.0, 8,8  );

      PopColor();
    }

  gluDeleteQuadric( quadric );

  PopCoords();
}

void Model::DrawPicker( void )
{
  //PRINT_DEBUG1( "Drawing %s", token );
  PushLocalCoords();

  // draw the boxes
  blockgroup.DrawSolid( geom );

  // recursively draw the tree below this model 
  LISTMETHOD( this->children, Model*, DrawPicker );

  PopCoords();
}

void Model::DataVisualize( Camera* cam )
{  
}

void Model::DataVisualizeTree( Camera* cam )
{
  PushLocalCoords();
  DataVisualize( cam ); // virtual function overridden by most model types  

  Visualizer* vis;
  for( GList* item = custom_visual_list; item; item = item->next ) {
    vis = static_cast<Visualizer* >( item->data );
	if( world_gui->GetCanvas()->_custom_options[ vis->GetMenuName() ]->isEnabled() )
	  vis->Visualize( this, cam );
  }

  // and draw the children
  LISTMETHODARG( children, Model*, DataVisualizeTree, cam );

  PopCoords();
}

void Model::DrawGrid( void )
{
  if ( gui.grid ) 
    {
      PushLocalCoords();
		
      stg_bounds3d_t vol;
      vol.x.min = -geom.size.x/2.0;
      vol.x.max =  geom.size.x/2.0;
      vol.y.min = -geom.size.y/2.0;
      vol.y.max =  geom.size.y/2.0;
      vol.z.min = 0;
      vol.z.max = geom.size.z;
		 
      PushColor( 0,0,1,0.4 );
		Gl::draw_grid(vol);
      PopColor();		 
      PopCoords();
    }
}

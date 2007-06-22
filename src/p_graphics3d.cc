/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2004, 2005 Richard Vaughan
 *                      
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * Desc: A plugin driver for Player that gives access to Stage devices.
 * Author: Richard Vaughan
 * Date: 10 December 2004
 * CVS: $Id: p_graphics3d.cc,v 1.1.2.3 2007-06-22 20:48:27 rtv Exp $
 */

#include "p_driver.h"

#include <iostream>
using namespace std;

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

extern GList* dl_list;

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player 
@par Graphics3d interface
- PLAYER_GRAPHICS3D_CMD_CLEAR
- PLAYER_GRAPHICS3D_CMD_DRAW
- PLAYER_GRAPHICS3D_CMD_TRANSLATE
- PLAYER_GRAPHICS3D_CMD_ROTATE
*/

typedef struct
{
  int type;
  void* data;
} cmd_record_t;


InterfaceGraphics3d::InterfaceGraphics3d( player_devaddr_t addr, 
			    StgDriver* driver,
			    ConfigFile* cf,
			    int section )
  : InterfaceModel( addr, driver, cf, section, NULL )
{ 
  // data members
  commands = NULL;
  displaylist = glGenLists( 1 );

  //printf( "allocated dl %d\n", displaylist );
 
  rebuild_displaylist = FALSE;

  dl_list = g_list_append( dl_list, (void*)displaylist );
}

InterfaceGraphics3d::~InterfaceGraphics3d( void )
{ 
  Clear();
  glDeleteLists( displaylist, 1 );
}

void InterfaceGraphics3d::Clear( void )
{ 
  // empty the list of drawing commands
  GList* it;
  for( it=commands; it; it=it->next )
    {
      cmd_record_t* rec = (cmd_record_t*)it->data;

      assert(rec);

      if( rec->data )
	g_free( rec->data );
      
      g_free( rec );
    }

  g_list_free( commands );
  commands = NULL;

  // empty the display list
  //puts( "CLEARING DISPLAYLIST" );
  glNewList( displaylist, GL_COMPILE );
  glEndList();
}

void InterfaceGraphics3d::StoreCommand( int type, void* cmd )
{
  cmd_record_t* rec = g_new( cmd_record_t, 1 );
  rec->type = type;
  rec->data = cmd;

  // store this command
  commands = g_list_append( commands, rec );
  
  // trigger display list update on next Publish()
  rebuild_displaylist = TRUE;
}

int InterfaceGraphics3d::ProcessMessage(MessageQueue* resp_queue,
				 player_msghdr_t* hdr,
				 void* data)
{

  // TODO queue *all* commands so that tranformations and clearing are
  // done synchronously

  // test for drawing commands first because they are the most common
  // by far and we avoid lots of tests

  // THESE COMMANDS HAVE A DATA PAYLOAD

  if( Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
			    PLAYER_GRAPHICS3D_CMD_DRAW, 
			    this->addr) )
    {
      StoreCommand( PLAYER_GRAPHICS3D_CMD_DRAW,
		    g_memdup( data, sizeof(player_graphics3d_cmd_draw_t)));;
      return 0; //ok
    }
  
  if( Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
			    PLAYER_GRAPHICS3D_CMD_ROTATE, 
			    this->addr) )
    {
      StoreCommand( PLAYER_GRAPHICS3D_CMD_ROTATE,
		    g_memdup( data, sizeof(player_graphics3d_cmd_rotate_t)));;
      return 0; //ok
    }
  
  if( Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
			    PLAYER_GRAPHICS3D_CMD_TRANSLATE, 
			    this->addr) )
    {
      StoreCommand( PLAYER_GRAPHICS3D_CMD_TRANSLATE,
		    g_memdup( data, sizeof(player_graphics3d_cmd_translate_t)));;
      return 0; //ok
    }

  // THESE COMMANDS HAVE NO DATA ATTACHED

  if( Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
			    PLAYER_GRAPHICS3D_CMD_PUSH, 
			    this->addr) )
    {
      StoreCommand( PLAYER_GRAPHICS3D_CMD_PUSH, NULL );
      return 0; //ok
    }
  
  if( Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
			    PLAYER_GRAPHICS3D_CMD_POP, 
			    this->addr) )
    {
      StoreCommand( PLAYER_GRAPHICS3D_CMD_POP, NULL );
      return 0; //ok
    }
  
  if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
                           PLAYER_GRAPHICS3D_CMD_CLEAR, 
                           this->addr) )
    {
      Clear(); // empties the display list immediately
      return 0; //ok
    }
      
  PLAYER_WARN2("stage graphics3d  doesn't support message %d:%d.", 
	       hdr->type, hdr->subtype );
  return -1;
}

void InterfaceGraphics3d::Publish( void )
{
  // rebuild this device's OpenGL display list
 
  if( rebuild_displaylist )
    {      
      //printf( "rebuilding display list %d commands\n", 
      //      g_list_length(commands) );
      
      // execute the commands
      glNewList( displaylist, GL_COMPILE );

      // move into the relevant model's coordinate frame
      stg_pose_t gpose;
      stg_model_get_global_pose( mod, &gpose );
      
      glPushMatrix();
      glTranslatef( gpose.x, gpose.y, 0 ); 
      glRotatef( RTOD(gpose.a), 0,0,1 ); // z-axis rotation 
      
      // as long as there's anything on the queue
      GList* it;
      for( it=commands; it; it=it->next )
	{
	  cmd_record_t* rec = (cmd_record_t*)it->data;
	  
	  switch( rec->type )
	    {
	    case PLAYER_GRAPHICS3D_CMD_CLEAR:
	      PRINT_ERR( "clear commands are not supposed to get here" );
	      break;
	      
	    case PLAYER_GRAPHICS3D_CMD_PUSH:
	    case PLAYER_GRAPHICS3D_CMD_POP:
	      puts( "pushing and popping not yet implemented" );
	      break;
	      
	    case PLAYER_GRAPHICS3D_CMD_TRANSLATE:
	      {
		player_graphics3d_cmd_translate_t* cmd = 
		  (player_graphics3d_cmd_translate_t*)rec->data;	      
		//printf( "TRANSLATE [%.2f,%.2f,%.2f]n", cmd->x, cmd->y, cmd->z );
		glTranslatef( cmd->x, cmd->y, cmd->z );
		g_free( cmd );
	      }
	      break;
	      
	    case PLAYER_GRAPHICS3D_CMD_ROTATE:
	      {	  
		player_graphics3d_cmd_rotate_t* cmd = 
		  (player_graphics3d_cmd_rotate_t*)rec->data;	      
		//printf( "ROTATE %.2f radians about [%.2f,%.2f,%.2f]n", 
		//cmd->a, cmd->x, cmd->y, cmd->z );
		glRotatef( RTOD(cmd->a), cmd->x, cmd->y, cmd->z );
		g_free( cmd );
	      }
	      break;
	      
	    case PLAYER_GRAPHICS3D_CMD_DRAW:
	      {

		player_graphics3d_cmd_draw_t* cmd = 
		  (player_graphics3d_cmd_draw_t*)rec->data;
		
		//printf( "DRAW mode %d with %d points\n", 
		//cmd->draw_mode, cmd->points_count );

		// player color elements are uint8_t type
		glColor4ub( cmd->color.red, 
			    cmd->color.green, 
			    cmd->color.blue, 
			    cmd->color.alpha );
		
		switch( cmd->draw_mode )
		  {
		  case PLAYER_DRAW_POINTS:
		    glPointSize( 3 );
		    glBegin( GL_POINTS );
		    break;	      
		  case PLAYER_DRAW_LINES:
		    glBegin( GL_LINES );
		    break;
		  case PLAYER_DRAW_LINE_STRIP:
		    glBegin( GL_LINE_STRIP );
		    break;
		  case PLAYER_DRAW_LINE_LOOP:
		    glBegin( GL_LINE_LOOP );
		    break;
		  case PLAYER_DRAW_TRIANGLES:
		    glBegin( GL_TRIANGLES );
		    break;
		  case PLAYER_DRAW_TRIANGLE_STRIP:
		    glBegin( GL_TRIANGLE_STRIP );
		    break;
		  case PLAYER_DRAW_TRIANGLE_FAN:
		    glBegin( GL_TRIANGLE_FAN );
		    break;
		  case PLAYER_DRAW_QUADS:
		    glBegin( GL_QUADS );
		    break;
		  case PLAYER_DRAW_QUAD_STRIP:
		    glBegin( GL_QUAD_STRIP );
		    break;
		  case PLAYER_DRAW_POLYGON:
		    glBegin( GL_POLYGON );
		    break;
		    
		  default:
		    PRINT_WARN1( "drawing mode %d not handled", 
				 cmd->draw_mode );	 
		  }
		
		unsigned int c;
		for( c=0; c<cmd->points_count; c++ )
		  glVertex3f( cmd->points[c].px,
				cmd->points[c].py,
				cmd->points[c].pz );
		
		glEnd();
	      }
	      break;
	      
	    default:
	      PRINT_WARN1( "Unrecognized graphics command type %d\n", 
			   rec->type );
	    }
	}
      
      glPopMatrix(); // drop out of the model's CS
      glEndList();
      
      //puts("");
      
      rebuild_displaylist = FALSE;
    }
}

/*
 *  Stage
 *  Copyright (C) Richard Vaughan
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
 * CVS: $Id$
 */

// DOCUMENTATION ------------------------------------------------------------

/** @addtogroup player
@par Graphics2D interface
-	PLAYER_GRAPHICS2D_CMD_POINTS
-	PLAYER_GRAPHICS2D_CMD_POLYGON
-	PLAYER_GRAPHICS2D_CMD_MULTILINE
-	PLAYER_GRAPHICS2D_CMD_POLYLINE
-	PLAYER_GRAPHICS2D_CMD_CLEAR

@par Graphics3D interface
-	PLAYER_GRAPHICS3D_CMD_DRAW
-	PLAYER_GRAPHICS3D_CMD_PUSH
-	PLAYER_GRAPHICS3D_CMD_POP
-	PLAYER_GRAPHICS3D_CMD_CLEAR
-	PLAYER_GRAPHICS3D_CMD_TRANSLATE
-	PLAYER_GRAPHICS3D_CMD_ROTATE

*/

#include "p_driver.h"

#include <iostream>
using namespace std;

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

struct clientDisplaylist {
  int DisplayList;
  vector<Message> RenderItems;
};

class PlayerGraphicsVis : public Stg::Visualizer {
private:
public:
  PlayerGraphicsVis() : Stg::Visualizer("Graphics", "custom_vis"){};
  virtual ~PlayerGraphicsVis(void)
  {
    for (queuemap::iterator itr = ClientDisplayLists.begin(); itr != ClientDisplayLists.end();
         ++itr)
      RemoveDisplayList(itr->second);
  };
  virtual void Visualize(Stg::Model *mod, Stg::Camera *cam)
  {
    GLint OldDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFunc);
    glDepthFunc(GL_LEQUAL);
    for (queuemap::iterator itr = ClientDisplayLists.begin(); itr != ClientDisplayLists.end();
         ++itr) {
      glPushMatrix();
      glTranslatef(0, 0, 0.01); // tiny Z offset raises rect above grid
      glCallList(itr->second.DisplayList);
      glPopMatrix();
    }
    glDepthFunc(OldDepthFunc);
  }

  void Clear(MessageQueue *client)
  {
    struct clientDisplaylist &list = GetDisplayList(client);
    list.RenderItems.clear();
    glNewList(list.DisplayList, GL_COMPILE);
    glEndList();
  };

  bool HasActiveDisplayList(MessageQueue *client)
  {
    queuemap::iterator found = ClientDisplayLists.find(client);
    if (found == ClientDisplayLists.end())
      return false;
    else if (found->second.DisplayList == -1)
      return false;
    return true;
  }

  struct clientDisplaylist &GetDisplayList(MessageQueue *client)
  {
    queuemap::iterator found = ClientDisplayLists.find(client);
    if (found == ClientDisplayLists.end()) // Display list not created yet
    {
      clientDisplaylist &clientDisp = ClientDisplayLists[client];
      clientDisp.DisplayList = glGenLists(1);
      return clientDisp;
    } else if (found->second.DisplayList == -1)
      found->second.DisplayList = glGenLists(1);
    return found->second;
  }

  void BuildDisplayList(MessageQueue *client)
  {
    struct clientDisplaylist &list = GetDisplayList(client);
    glNewList(list.DisplayList, GL_COMPILE);
    glPushMatrix();
    for (vector<Message>::iterator itr = list.RenderItems.begin(); itr != list.RenderItems.end();
         ++itr)
      RenderItem(*itr);
    glPopMatrix();
    glEndList();
  }

  void RemoveDisplayList(struct clientDisplaylist &list)
  {
    if (list.DisplayList > 0)
      glDeleteLists(list.DisplayList, 1);
  }

  virtual void AppendItem(MessageQueue *client, Message &item)
  {
    struct clientDisplaylist &list = GetDisplayList(client);
    list.RenderItems.push_back(item);
  };

  void Subscribe(QueuePointer &queue)
  {
    if (queue == NULL)
      return;
    clientDisplaylist &clientDisp = ClientDisplayLists[queue.get()];
    // delay creation of display list so we don't get race conditions on internal subscriptions.
    clientDisp.DisplayList = -1;
  }

  void Unsubscribe(QueuePointer &queue)
  {
    if (queue == NULL)
      return;
    if (HasActiveDisplayList(queue.get())) {
      struct clientDisplaylist &list = GetDisplayList(queue.get());
      RemoveDisplayList(list);
    }
    ClientDisplayLists.erase(queue.get());
  }

  void glPlayerColour(const player_color_t &colour)
  {
    glColor4f(static_cast<float>(colour.red) / 255.0, static_cast<float>(colour.green) / 255.0,
              static_cast<float>(colour.blue) / 255.0,
              1 - static_cast<float>(colour.alpha) / 255.0);
  }

  virtual void RenderItem(Message &item) = 0;

private:
  typedef map<MessageQueue *, struct clientDisplaylist> queuemap;
  queuemap ClientDisplayLists;
};

class PlayerGraphics2dVis : public PlayerGraphicsVis {
public:
  PlayerGraphics2dVis() : PlayerGraphicsVis(){};
  ~PlayerGraphics2dVis(){};

  void RenderItem(Message &item);
};

class PlayerGraphics3dVis : public PlayerGraphicsVis {
public:
  PlayerGraphics3dVis() : PlayerGraphicsVis(){};
  ~PlayerGraphics3dVis(){};

  void RenderItem(Message &item);
};

InterfaceGraphics2d::InterfaceGraphics2d(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf,
                                         int section)
    : InterfaceModel(addr, driver, cf, section, "")
{
  vis = new PlayerGraphics2dVis;
  mod->AddVisualizer(vis, true);
}

InterfaceGraphics2d::~InterfaceGraphics2d()
{
  mod->RemoveVisualizer(vis);
  delete vis;
}

void InterfaceGraphics2d::Subscribe(QueuePointer &queue)
{
  vis->Subscribe(queue);
}

void InterfaceGraphics2d::Unsubscribe(QueuePointer &queue)
{
  vis->Unsubscribe(queue);
}

int InterfaceGraphics2d::ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data)
{
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS2D_CMD_CLEAR, this->addr)) {
    vis->Clear(resp_queue.get());
    return 0; // ok
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS2D_CMD_POINTS, this->addr)
      || Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS2D_CMD_POLYLINE, this->addr)
#ifdef PLAYER_GRAPHICS2D_CMD_MULTILINE
      || Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS2D_CMD_MULTILINE, this->addr)
#endif
      || Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS2D_CMD_POLYGON,
                               this->addr)) {
    Message msg(*hdr, data);
    vis->AppendItem(resp_queue.get(), msg);
    vis->BuildDisplayList(resp_queue.get());
    return 0;
  }

  PLAYER_WARN2("stage graphics2d doesn't support message %d:%d.", hdr->type, hdr->subtype);
  return -1;
}

void PlayerGraphics2dVis::RenderItem(Message &item)
{
  glDepthMask(GL_FALSE);
  int type = item.GetHeader()->subtype;
  switch (type) {
  case PLAYER_GRAPHICS2D_CMD_POINTS: {
    player_graphics2d_cmd_points_t &data =
        *reinterpret_cast<player_graphics2d_cmd_points_t *>(item.GetPayload());
    glPlayerColour(data.color);
    glBegin(GL_POINTS);
    for (unsigned ii = 0; ii < data.points_count; ++ii)
      glVertex3f(data.points[ii].px, data.points[ii].py, 0);
    glEnd();
  } break;
  case PLAYER_GRAPHICS2D_CMD_POLYLINE: {
    player_graphics2d_cmd_polyline_t &data =
        *reinterpret_cast<player_graphics2d_cmd_polyline_t *>(item.GetPayload());
    glPlayerColour(data.color);
    glBegin(GL_LINE_STRIP);
    for (unsigned ii = 0; ii < data.points_count; ++ii)
      glVertex3f(data.points[ii].px, data.points[ii].py, 0);
    glEnd();
  } break;
#ifdef PLAYER_GRAPHICS2D_CMD_MULTILINE
  case PLAYER_GRAPHICS2D_CMD_MULTILINE: {
    player_graphics2d_cmd_multiline_t &data =
        *reinterpret_cast<player_graphics2d_cmd_multiline_t *>(item.GetPayload());
    glPlayerColour(data.color);
    glBegin(GL_LINES);
    for (unsigned ii = 0; ii < data.points_count; ++ii)
      glVertex3f(data.points[ii].px, data.points[ii].py, 0);
    glEnd();
  } break;

#endif

  case PLAYER_GRAPHICS2D_CMD_POLYGON: {
    player_graphics2d_cmd_polygon_t &data =
        *reinterpret_cast<player_graphics2d_cmd_polygon_t *>(item.GetPayload());
    if (data.filled) {
      glPlayerColour(data.fill_color);
      glBegin(GL_POLYGON);
      for (unsigned ii = 0; ii < data.points_count; ++ii)
        glVertex3f(data.points[ii].px, data.points[ii].py, 0);
      glEnd();
    }
    glPlayerColour(data.color);
    glBegin(GL_LINE_LOOP);
    for (unsigned ii = 0; ii < data.points_count; ++ii)
      glVertex3f(data.points[ii].px, data.points[ii].py, 0);
    glEnd();
  } break;
  }
  glDepthMask(GL_TRUE);
}

InterfaceGraphics3d::InterfaceGraphics3d(player_devaddr_t addr, StgDriver *driver, ConfigFile *cf,
                                         int section)
    : InterfaceModel(addr, driver, cf, section, "")
{
  vis = new PlayerGraphics3dVis;
  mod->AddVisualizer(vis, true);
}

InterfaceGraphics3d::~InterfaceGraphics3d()
{
  mod->RemoveVisualizer(vis);
  delete vis;
}

void InterfaceGraphics3d::Subscribe(QueuePointer &queue)
{
  vis->Subscribe(queue);
}

void InterfaceGraphics3d::Unsubscribe(QueuePointer &queue)
{
  vis->Unsubscribe(queue);
}

int InterfaceGraphics3d::ProcessMessage(QueuePointer &resp_queue, player_msghdr_t *hdr, void *data)
{
  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS3D_CMD_CLEAR, this->addr)) {
    vis->Clear(resp_queue.get());
    return 0; // ok
  }

  if (Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS3D_CMD_PUSH, this->addr)
      || Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS3D_CMD_POP, this->addr)
      || Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS3D_CMD_DRAW, this->addr)
      || Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS3D_CMD_TRANSLATE, this->addr)
      || Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_GRAPHICS3D_CMD_ROTATE, this->addr)) {
    Message msg(*hdr, data);
    vis->AppendItem(resp_queue.get(), msg);
    vis->BuildDisplayList(resp_queue.get());
    return 0;
  }

  PLAYER_WARN2("stage graphics2d doesn't support message %d:%d.", hdr->type, hdr->subtype);
  return -1;
}

void PlayerGraphics3dVis::RenderItem(Message &item)
{
  int type = item.GetHeader()->subtype;
  switch (type) {
  case PLAYER_GRAPHICS3D_CMD_DRAW: {
    player_graphics3d_cmd_draw_t &data =
        *reinterpret_cast<player_graphics3d_cmd_draw_t *>(item.GetPayload());
    glPlayerColour(data.color);
    switch (data.draw_mode) {
    case PLAYER_DRAW_POINTS: glBegin(GL_POINTS); break;
    case PLAYER_DRAW_LINES: glBegin(GL_LINES); break;
    case PLAYER_DRAW_LINE_STRIP: glBegin(GL_LINE_STRIP); break;
    case PLAYER_DRAW_LINE_LOOP: glBegin(GL_LINE_LOOP); break;
    case PLAYER_DRAW_TRIANGLES: glBegin(GL_TRIANGLES); break;
    case PLAYER_DRAW_TRIANGLE_STRIP: glBegin(GL_TRIANGLE_STRIP); break;
    case PLAYER_DRAW_TRIANGLE_FAN: glBegin(GL_TRIANGLE_FAN); break;
    case PLAYER_DRAW_QUADS: glBegin(GL_QUADS); break;
    case PLAYER_DRAW_QUAD_STRIP: glBegin(GL_QUAD_STRIP); break;
    case PLAYER_DRAW_POLYGON: glBegin(GL_POLYGON); break;
    default: fprintf(stderr, "Unknown graphics 3d draw mode\n"); return;
    }
    for (unsigned ii = 0; ii < data.points_count; ++ii)
      glVertex3f(data.points[ii].px, data.points[ii].py, data.points[ii].pz);
    glEnd();
  } break;
  case PLAYER_GRAPHICS3D_CMD_TRANSLATE: {
    player_graphics3d_cmd_translate_t &data =
        *reinterpret_cast<player_graphics3d_cmd_translate_t *>(item.GetPayload());
    glTranslatef(data.x, data.y, data.z);
  } break;
  case PLAYER_GRAPHICS3D_CMD_ROTATE: {
    player_graphics3d_cmd_rotate_t &data =
        *reinterpret_cast<player_graphics3d_cmd_rotate_t *>(item.GetPayload());
    glRotatef(data.a, data.x, data.y, data.z);
  } break;
  case PLAYER_GRAPHICS3D_CMD_PUSH: {
    glPushMatrix();
  } break;
  case PLAYER_GRAPHICS3D_CMD_POP: {
    glPopMatrix();
  } break;
  }
}

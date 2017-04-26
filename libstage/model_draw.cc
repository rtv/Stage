#include "canvas.hh"
#include "stage.hh"
#include "texture_manager.hh"
#include "worldfile.hh"
using namespace Stg;

// speech bubble colors
static const Color BUBBLE_FILL(1.0, 0.8, 0.8); // light blue/grey
static const Color BUBBLE_BORDER(0, 0, 0); // black
static const Color BUBBLE_TEXT(0, 0, 0); // black

void Model::DrawSelected()
{
  glPushMatrix();

  glTranslatef(pose.x, pose.y, pose.z + 0.01); // tiny Z offset raises rect above grid

  Pose gp = GetGlobalPose();

  char buf[64];
  snprintf(buf, 63, "%s [%.2f %.2f %.2f %.2f]", Token(), gp.x, gp.y, gp.z, rtod(gp.a));

  PushColor(0, 0, 0, 1); // text color black
  Gl::draw_string(0.5, 0.5, 0.5, buf);

  glRotatef(rtod(pose.a), 0, 0, 1);

  Gl::pose_shift(geom.pose);

  double dx = geom.size.x / 2.0 * 1.6;
  double dy = geom.size.y / 2.0 * 1.6;

  PopColor();

  PushColor(0, 1, 0, 0.4); // highlight color blue
  glRectf(-dx, -dy, dx, dy);
  PopColor();

  PushColor(0, 1, 0, 0.8); // highlight color blue
  glLineWidth(1);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glRectf(-dx, -dy, dx, dy);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  PopColor();

  glPopMatrix();
}

void Model::DrawTrailFootprint()
{
  double darkness = 0;
  double fade = 0.5 / (double)(trail.size() + 1);

  PushColor(0, 0, 0, 1); // dummy push just saving the color

  // this loop could be faster, but optimzing vis is not a priority
  
  for (unsigned int i = 0; i < trail.size(); i++) {
    
    // find correct offset inside ring buffer
    TrailItem &checkpoint = trail[(i + trail_index) % trail.size()];
    
    // ignore invalid items
    if (checkpoint.time == 0)
      continue;
    
    glPushMatrix();
    Pose pz = checkpoint.pose;
    
    Gl::pose_shift(pz);
    Gl::pose_shift(geom.pose);
    
    darkness += fade;
    Color c = checkpoint.color;
    c.a = darkness;
    glColor4f(c.r, c.g, c.b, c.a);

    blockgroup.DrawFootPrint(geom);

    glPopMatrix();
  }

  PopColor();
}

void Model::DrawTrailBlocks()
{
  double timescale = 0.0000001;

  FOR_EACH (it, trail) {
    TrailItem &checkpoint = *it;

    glPushMatrix();
    Pose pz = checkpoint.pose;
    pz.z = (world->sim_time - checkpoint.time) * timescale;

    Gl::pose_shift(pz);
    Gl::pose_shift(geom.pose);

    DrawBlocks();

    glPopMatrix();
  }
}

void Model::DrawTrailArrows()
{
  double dx = 0.2;
  double dy = 0.07;
  double timescale = 1e-7;

  PushColor(0, 0, 0, 1); // dummy push

  FOR_EACH (it, trail) {
    TrailItem &checkpoint = *it;

    glPushMatrix();
    Pose pz = checkpoint.pose;
    // set the height proportional to age
    pz.z = (world->sim_time - checkpoint.time) * timescale;

    Gl::pose_shift(pz);
    Gl::pose_shift(geom.pose);

    Color &c = checkpoint.color;
    glColor4f(c.r, c.g, c.b, c.a);

    glBegin(GL_TRIANGLES);
    glVertex3f(0, -dy, 0);
    glVertex3f(dx, 0, 0);
    glVertex3f(0, +dy, 0);
    glEnd();

    glPopMatrix();
  }

  PopColor();
}

void Model::DrawOriginTree()
{
  DrawPose(GetGlobalPose());

  FOR_EACH (it, children)
    (*it)->DrawOriginTree();
}

void Model::DrawBlocksTree()
{
  PushLocalCoords();

  FOR_EACH (it, children)
    (*it)->DrawBlocksTree();

  DrawBlocks();
  PopCoords();
}

void Model::DrawPose(Pose pose)
{
  PushColor(0, 0, 0, 1);
  glPointSize(4);

  glBegin(GL_POINTS);
  glVertex3f(pose.x, pose.y, pose.z);
  glEnd();

  PopColor();
}

void Model::DrawBlocks()
{
  blockgroup.CallDisplayList();
}

void Model::DrawBoundingBoxTree()
{
  PushLocalCoords();

  FOR_EACH (it, children)
    (*it)->DrawBoundingBoxTree();

  DrawBoundingBox();
  PopCoords();
}

void Model::DrawBoundingBox()
{
  Gl::pose_shift(geom.pose);

  PushColor(color);

  glBegin(GL_QUAD_STRIP);

  glVertex3f(-geom.size.x / 2.0, -geom.size.y / 2.0, geom.size.z);
  glVertex3f(-geom.size.x / 2.0, -geom.size.y / 2.0, 0);

  glVertex3f(+geom.size.x / 2.0, -geom.size.y / 2.0, geom.size.z);
  glVertex3f(+geom.size.x / 2.0, -geom.size.y / 2.0, 0);

  glVertex3f(+geom.size.x / 2.0, +geom.size.y / 2.0, geom.size.z);
  glVertex3f(+geom.size.x / 2.0, +geom.size.y / 2.0, 0);

  glVertex3f(+geom.size.x / 2.0, +geom.size.y / 2.0, geom.size.z);
  glVertex3f(+geom.size.x / 2.0, +geom.size.y / 2.0, 0);

  glVertex3f(-geom.size.x / 2.0, +geom.size.y / 2.0, geom.size.z);
  glVertex3f(-geom.size.x / 2.0, +geom.size.y / 2.0, 0);

  glVertex3f(-geom.size.x / 2.0, -geom.size.y / 2.0, geom.size.z);
  glVertex3f(-geom.size.x / 2.0, -geom.size.y / 2.0, 0);

  glEnd();

  glBegin(GL_LINES);
  glVertex2f(-0.02, 0);
  glVertex2f(+0.02, 0);

  glVertex2f(0, -0.02);
  glVertex2f(0, +0.02);
  glEnd();

  PopColor();
}

// move into this model's local coordinate frame
void Model::PushLocalCoords()
{
  glPushMatrix();

  if (parent && parent->stack_children)
    glTranslatef(0, 0, parent->geom.size.z);

  Gl::pose_shift(pose);
}

void Model::PopCoords()
{
  glPopMatrix();
}

void Model::AddVisualizer(Visualizer *cv, bool on_by_default)
{
  assert(cv);

  // If there's no GUI, ignore this request
  if (!world_gui)
    return;

  // save visualizer instance
  cv_list.push_back(cv);

  // register option for all instances which share the same name
  Canvas *canvas = world_gui->GetCanvas();
  std::map<std::string, Option *>::iterator i = canvas->_custom_options.find(cv->GetMenuName());
  if (i == canvas->_custom_options.end()) {
    Option *op =
        new Option(cv->GetMenuName(), cv->GetWorldfileName(), "", on_by_default, world_gui);
    canvas->_custom_options[cv->GetMenuName()] = op;
    RegisterOption(op);
  }
}

void Model::RemoveVisualizer(Visualizer *cv)
{
  if (cv)
    EraseAll(cv, cv_list);

  // TODO unregister option - tricky because there might still be instances
  // attached to different models which have the same name
}

void Model::DrawStatusTree(Camera *cam)
{
  PushLocalCoords();
  DrawStatus(cam);
  FOR_EACH (it, children)
    (*it)->DrawStatusTree(cam);
  PopCoords();
}

void Model::DrawStatus(Camera *cam)
{
  if (power_pack || !say_string.empty()) {
    float pitch = -cam->pitch();
    float yaw = -cam->yaw();

    Pose gpz = GetGlobalPose();

    float robotAngle = -rtod(gpz.a);
    glPushMatrix();

    // move above the robot
    glTranslatef(0, 0, 0.5);

    // rotate to face screen
    glRotatef(robotAngle - yaw, 0, 0, 1);
    glRotatef(-pitch, 1, 0, 0);

    //      if( power_pack )
    // power_pack->Visualize( cam );

    if (!say_string.empty()) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      // get raster positition, add gl_width, then project back to world coords
      glRasterPos3f(0, 0, 0);
      GLfloat pos[4];
      glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);

      GLboolean valid;
      glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &valid);

      if (valid) {
        // fl_font( FL_HELVETICA, 12 );
        float w = gl_width(this->say_string.c_str()); // scaled text width
        float h = gl_height(); // scaled text height

        GLdouble wx, wy, wz;
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        GLdouble modelview[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

        GLdouble projection[16];
        glGetDoublev(GL_PROJECTION_MATRIX, projection);

        // get width and height in world coords
        gluUnProject(pos[0] + w, pos[1], pos[2], modelview, projection, viewport, &wx, &wy, &wz);
        w = wx;
        gluUnProject(pos[0], pos[1] + h, pos[2], modelview, projection, viewport, &wx, &wy, &wz);
        h = wy;

        // calculate speech bubble margin
        const float m = h / 10;

        // draw inside of bubble
        PushColor(BUBBLE_FILL);
        glPushAttrib(GL_POLYGON_BIT | GL_LINE_BIT);
        glPolygonMode(GL_FRONT, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);
        Gl::draw_octagon(w, h, m);
        glDisable(GL_POLYGON_OFFSET_FILL);
        PopColor();

        // draw outline of bubble
        PushColor(BUBBLE_BORDER);
        glLineWidth(1);
        glEnable(GL_LINE_SMOOTH);
        glPolygonMode(GL_FRONT, GL_LINE);
        Gl::draw_octagon(w, h, m);
        glPopAttrib();
        PopColor();

        PushColor(BUBBLE_TEXT);
        // draw text inside the bubble
        Gl::draw_string(m, 2.5 * m, 0, this->say_string.c_str());
        PopColor();
      }
    }
    glPopMatrix();
  }

  if (stall) {
    DrawImage(TextureManager::getInstance()._stall_texture_id, cam, 0.85);
  }

  //   extern GLuint glowTex;
  //   extern GLuint checkTex;

  //   if( parent == NULL )
  //  	 {
  //  		glBlendFunc(GL_SRC_COLOR, GL_ONE );
  //  		DrawImage( glowTex, cam, 1.0 );
  //  		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  //  	 }
}

void Model::DrawImage(uint32_t texture_id, Camera *cam, float alpha, double width, double height)
{
  (void)alpha; // avoid warning about unused var

  float yaw, pitch;
  pitch = -cam->pitch();
  yaw = -cam->yaw();

  float robotAngle = -rtod(GetGlobalPose().a);

  glPolygonMode(GL_FRONT, GL_FILL);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glColor4f(1.0, 1.0, 1.0, 1.0);
  glPushMatrix();

  // position image above the robot
  // TODO
  glTranslatef(0.0, 0.0, ModelHeight() + 0.3);

  // rotate to face screen
  glRotatef(robotAngle - yaw, 0, 0, 1);
  glRotatef(-pitch - 90, 1, 0, 0);

  // draw a square, with the textured image
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-0.25f, 0, -0.25f);
  glTexCoord2f(width, 0.0f);
  glVertex3f(0.25f, 0, -0.25f);
  glTexCoord2f(width, height);
  glVertex3f(0.25f, 0, 0.25f);
  glTexCoord2f(0.0f, height);
  glVertex3f(-0.25f, 0, 0.25f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);

  glPopMatrix();
}

void Model::DrawFlagList(void)
{
  if (flag_list.size() < 1)
    return;

  Pose gp = GetGlobalPose();
  GLfloat z = 1.0;

  for (std::list<Flag *>::reverse_iterator it(flag_list.rbegin()); it != flag_list.rend(); ++it) {
    double sz = (*it)->GetSize();
    double d = sz / 2.0;

    (*it)->GetColor().GLSet();

    glVertex3f(gp.x + d, gp.y + 0, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y + d, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y + 0, gp.z + d + z);

    glVertex3f(gp.x + d, gp.y + 0, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y + d, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y + 0, gp.z - d + z);

    glVertex3f(gp.x - d, gp.y + 0, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y - d, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y + 0, gp.z + d + z);

    glVertex3f(gp.x - d, gp.y + 0, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y + d, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y + 0, gp.z - d + z);

    glVertex3f(gp.x + d, gp.y + 0, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y - d, gp.z + 0 + z);
    glVertex3f(gp.x + 0, gp.y + 0, gp.z - d + z);

    // for wire-frame we only need half of the 8 triangles

    // glVertex3f( gp.x+d, gp.y+0, gp.z+0 +z);
    // glVertex3f( gp.x+0, gp.y+d, gp.z+0 +z);
    // glVertex3f( gp.x+0, gp.y+0, gp.z-d +z);

    // glVertex3f( gp.x-d, gp.y+0, gp.z+0 +z);
    // glVertex3f( gp.x+0, gp.y-d, gp.z+0 +z);
    // glVertex3f( gp.x+0, gp.y+0, gp.z-d +z);

    // and two more...

    z += sz;
  }
}

// void Model::DrawBlinkenlights()
// {
//   PushLocalCoords();

//   GLUquadric* quadric = gluNewQuadric();
//   //glTranslatef(0,0,1); // jump up
//   //Pose gpose = GetGlobalPose();
//   //glRotatef( 180 + rtod(-gpose.a),0,0,1 );

//   for( unsigned int i=0; i<blinkenlights->len; i++ )
//     {
//       blinkenlight_t* b =
// 	(blinkenlight_t*)g_ptr_array_index( blinkenlights, i );
//       assert(b);

//       glTranslatef( b->pose.x, b->pose.y, b->pose.z );

//       PushColor( b->color );

//       if( b->enabled )
// 		  gluQuadricDrawStyle( quadric, GLU_FILL );
//       else
// 		  gluQuadricDrawStyle( quadric, GLU_LINE );

//       gluSphere( quadric, b->size/2.0, 8,8  );

//       PopColor();
//     }

//   gluDeleteQuadric( quadric );

//   PopCoords();
// }

void Model::DrawPicker(void)
{
  // PRINT_DEBUG1( "Drawing %s", token );
  PushLocalCoords();

  // draw the boxes
  blockgroup.DrawSolid(geom);

  // recursively draw the tree below this model
  FOR_EACH (it, children)
    (*it)->DrawPicker();

  PopCoords();
}

void Model::DataVisualize(Camera *cam)
{
  (void)cam; // avoid warning about unused var
}

void Model::DataVisualizeTree(Camera *cam)
{
  PushLocalCoords();

  if (subs > 0) {
    DataVisualize(cam); // virtual function overridden by some model types

    FOR_EACH (it, cv_list) {
      Visualizer *vis = *it;
      if (world_gui->GetCanvas()->_custom_options[vis->GetMenuName()]->isEnabled())
        vis->Visualize(this, cam);
    }
  }

  // and draw the children
  FOR_EACH (it, children)
    (*it)->DataVisualizeTree(cam);

  PopCoords();
}

void Model::DrawGrid(void)
{
  if (gui.grid) {
    PushLocalCoords();

    bounds3d_t vol;
    vol.x.min = -geom.size.x / 2.0;
    vol.x.max = geom.size.x / 2.0;
    vol.y.min = -geom.size.y / 2.0;
    vol.y.max = geom.size.y / 2.0;
    vol.z.min = 0;
    vol.z.max = geom.size.z;

    PushColor(0, 0, 1, 0.4);
    Gl::draw_grid(vol);
    PopColor();
    PopCoords();
  }
}

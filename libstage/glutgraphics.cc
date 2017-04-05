/*
 *  Player - One Hell of a Robot Server
 *  glutgraphics.cc: GLUT-based graphics3d + 2d driver
 *  Copyright (C) 2007
 *     Brian Gerkey, Richard Vaughan
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
 * A simple example of how to write a driver that will be built as a
 * shared object.
 */

// ONLY if you need something that was #define'd as a result of configure
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#if HAVE_CONFIG_H
#include <config.h>
#endif
 */

#include <string.h>
#include <unistd.h>

#include <libplayercore/playercore.h>

#include <GLUT/glut.h>
#include <math.h>
#include <stdio.h>

GLfloat light0_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat light0_diffuse[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat light1_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
GLfloat light1_position[] = { 1.0, 1.0, 1.0, 0.0 };
GLfloat light2_diffuse[] = { 0.0, 1.0, 0.0, 1.0 };
GLfloat light2_position[] = { -1.0, -1.0, 1.0, 0.0 };
float s = 0.0;
GLfloat angle1 = 0.0, angle2 = 0.0;

void output(GLfloat x, GLfloat y, char *text)
{
  char *p;

  glPushMatrix();
  glTranslatef(x, y, 0);
  for (p = text; *p; p++)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
  glPopMatrix();
}

void display(void)
{
  static GLfloat amb[] = { 0.4, 0.4, 0.4, 0.0 };
  static GLfloat dif[] = { 1.0, 1.0, 1.0, 0.0 };

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHT1);
  glDisable(GL_LIGHT2);
  amb[3] = dif[3] = cos(s) / 2.0 + 0.5;
  glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

  glPushMatrix();
  glTranslatef(-0.3, -0.3, 0.0);
  glRotatef(angle1, 1.0, 5.0, 0.0);
  glCallList(1); /* render ico display list */
  glPopMatrix();

  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHT2);
  glDisable(GL_LIGHT1);
  amb[3] = dif[3] = 0.5 - cos(s * .95) / 2.0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

  glPushMatrix();
  glTranslatef(0.3, 0.3, 0.0);
  glRotatef(angle2, 1.0, 0.0, 5.0);
  glCallList(1); /* render ico display list */
  glPopMatrix();

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 1500, 0, 1500);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  /* Rotate text slightly to help show jaggies. */
  glRotatef(4, 0.0, 0.0, 1.0);
  output(200, 225, "This is antialiased.");
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);
  output(160, 100, "This text is not.");
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glMatrixMode(GL_MODELVIEW);

  glutSwapBuffers();
}

void idle(void)
{
  angle1 = (GLfloat)fmod(angle1 + 0.8, 360.0);
  angle2 = (GLfloat)fmod(angle2 + 1.1, 360.0);
  s += 0.05;

  // usleep( 100000 );

  static int g = 1;
  printf("cycle %d\n", g++);

  glutPostRedisplay();
}

void redraw(int val)
{
  angle1 = (GLfloat)fmod(angle1 + 0.8, 360.0);
  angle2 = (GLfloat)fmod(angle2 + 1.1, 360.0);
  s += 0.05;

  // usleep( 100000 );

  static int g = 1;
  printf("cycle %d\n", g++);

  glutPostRedisplay();

  glutTimerFunc(100, redraw, 0);
}

void visible(int vis)
{
  if (vis == GLUT_VISIBLE)
    // glutTimerFunc( 100, redraw, 0 );
    glutIdleFunc(idle);
  else
    glutIdleFunc(NULL);
}

////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class ExampleDriver : public Driver {
public:
  // Constructor; need that
  ExampleDriver(ConfigFile *cf, int section);

  // Must implement the following methods.
  virtual int Setup();
  virtual int Shutdown();

  // This method will be invoked on each incoming message
  virtual int ProcessMessage(MessageQueue *resp_queue, player_msghdr *hdr, void *data);

private:
  // Main function for device thread.
  virtual void Main();

  virtual void Update();

  int foop;
};

// A factory creation function, declared outside of the class so that it
// can be invoked without any object context (alternatively, you can
// declare it static in the class).  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver *ExampleDriver_Init(ConfigFile *cf, int section)
{
  puts("my init");
  // Create and return a new instance of this driver
  return ((Driver *)(new ExampleDriver(cf, section)));
}

// A driver registration function, again declared outside of the class so
// that it can be invoked without object context.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void ExampleDriver_Register(DriverTable *table)
{
  table->AddDriver("glutgraphics", ExampleDriver_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
ExampleDriver::ExampleDriver(ConfigFile *cf, int section)
    : Driver(cf, section, false, PLAYER_MSGQUEUE_DEFAULT_MAXLEN, PLAYER_GRAPHICS3D_CODE), foop(0)
{
  // Read an option from the configuration file
  // this->foop = cf->ReadInt(section, "foo", 0);

  return;
}

static int argc = 1;
static char *argv = "fake";

////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
int ExampleDriver::Setup()
{
  puts("Example driver initialising");

  // Here you do whatever is necessary to setup the device, like open and
  // configure a serial port.

  // printf("Was foo option given in config file? %d\n", this->foop);

  glutInit(&argc, &argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("blender");
  glutDisplayFunc(display);
  glutVisibilityFunc(visible);

  glNewList(1, GL_COMPILE); /* create ico display list */
  glutSolidIcosahedron();
  glEndList();

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
  glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(2.0);

  glMatrixMode(GL_PROJECTION);
  gluPerspective(/* field of view in degree */ 40.0,
                 /* aspect ratio */ 1.0,
                 /* Z near */ 1.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0, /* eye is at (0,0,5) */
            0.0, 0.0, 0.0, /* center is at (0,0,0) */
            0.0, 1.0, 0.); /* up is in positive Y direction */
  glTranslatef(0.0, 0.6, -1.0);

  puts("Example driver ready");

  // Start the device thread; spawns a new thread and executes
  // ExampleDriver::Main(), which contains the main loop for the driver.
  // StartThread();

  return (0);
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
int ExampleDriver::Shutdown()
{
  puts("Shutting example driver down");

  // Stop and join the driver thread
  StopThread();

  // Here you would shut the device down by, for example, closing a
  // serial port.

  puts("Example driver has been shutdown");

  return (0);
}

int ExampleDriver::ProcessMessage(MessageQueue *resp_queue, player_msghdr *hdr, void *data)
{
  // Process messages here.  Send a response if necessary, using Publish().
  // If you handle the message successfully, return 0.  Otherwise,
  // return -1, and a NACK will be sent for you, if a response is required.

  // call this from the driver's private thread - on a timer?
  // receive messages and do drawing

  return (0);
}

void ExampleDriver::Update()
{
  ProcessMessages();
  glutCheckLoop();
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void ExampleDriver::Main()
{
  printf("entering main loop");
  // glutMainLoop();

  // return 0;             /* ANSI C requires main to return int. */

  // The main loop; interact with the device here
  for (;;) {
  }
  //{
  // test if we are supposed to cancel
  // pthread_testcancel();

  // Process incoming messages.  ExampleDriver::ProcessMessage() is
  // called on each message.
  // ProcessMessages();

  // Interact with the device, and push out the resulting data, using
  // Driver::Publish()

  // Sleep (you might, for example, block on a read() instead)
  // usleep(100000);
}

////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
extern "C" {
int player_driver_init(DriverTable *table)
{
  puts("Example driver initializing");
  ExampleDriver_Register(table);
  puts("Example driver done");
  return (0);
}
}

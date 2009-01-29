
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

Puck::Puck() : 
  color( stg_color_pack( 1,0,0,0) ),
  displaylist( 0 ),
  height( 0.2 ),
  pose( 0,0,0,0 ),
  radius( 0.1 )
{ /* empty */ }

void Puck::Load( Worldfile* wf, int section )
{
  pose.Load( wf, section, "pose" );
  //color.Load( wf, section, "color" );
  
  radius = wf->ReadLength( section, "radius", radius );
  height = wf->ReadLength( section, "height", height );
  
  BuildDisplayList();
}

void Puck::Save( Worldfile* wf, int section )
{
  pose.Save( wf, section, "pose" );
  //color.Save( wf, section, "color" );
  
  wf->WriteLength( section, "radius", radius );
  wf->WriteLength( section, "height", height );  
}

void Puck::BuildDisplayList()
{ 
  if( displaylist == 0 )
	 displaylist = glGenLists(1);
  
  glNewList( displaylist, GL_COMPILE );	  

  GLUquadricObj *q = gluNewQuadric();
  glColor3f( 1,0,0  );  

  // draw a filled body
  glPushMatrix();  
  glTranslatef( pose.x, pose.y, pose.z );
  gluCylinder(q, radius, radius, height, 16, 1 );
  glTranslatef( 0,0, height);
  gluDisk(q, 0, radius, 16, 16 );
  glPopMatrix();
  
  // outline the sides in a darker shade
  double r,g,b,a;
  stg_color_unpack( color, &r, &g, &b, &a );
  glColor3f( r/2.0, g/2.0, b/2.0 );
  
  gluQuadricDrawStyle( q, GLU_LINE );
  glTranslatef( pose.x, pose.y, pose.z );
  gluCylinder(q, radius+0.001, radius+0.001, height+0.001, 16, 1 );

  // clean up
  gluDeleteQuadric( q );  
  glEndList();
}


void Puck::Draw()
{
  if( displaylist == 0 )
	 BuildDisplayList();

  glPushMatrix();
  glCallList( displaylist );
  glPopMatrix();
}

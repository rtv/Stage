///////////////////////////////////////////////////////////////////////////
//
// File: fixedobstacle.cc
// Author: Andrew Howard
// Date: 29 Dec 2001
// Desc: Simulates fixed obstacles
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/fixedobstacle.cc,v $
//  $Author: inspectorg $
//  $Revision: 1.15 $
//
///////////////////////////////////////////////////////////////////////////

#include <float.h>
#include "image.hh"
#include "world.hh"
#include "fixedobstacle.hh"


///////////////////////////////////////////////////////////////////////////
// Default constructor
CFixedObstacle::CFixedObstacle(CWorld *world, CEntity *parent)
    : CEntity(world, parent)
{
  // set the Player IO sizes correctly for this type of Entity
  m_data_len    = 0;
  m_command_len = 0;
  m_config_len  = 0;
  m_reply_len  = 0;
  
  this->stage_type = WallType;
  this->color = ::LookupColor(WALL_COLOR);

  vision_return = true;
  laser_return = LaserVisible;
  sonar_return = true;
  obstacle_return = true;
  puck_return = false; // we trade velocities with pucks
  idar_return = IDARReflect;

  this->crop_ax = -DBL_MAX;
  this->crop_ay = -DBL_MAX;
  this->crop_bx = +DBL_MAX;
  this->crop_by = +DBL_MAX;
 
  this->filename = NULL;
  this->scale = 1.0 / m_world->ppm;
  this->image = NULL;

#ifdef INCLUDE_RTK2
  // We cant move fixed obstacles
  this->movemask = 0; 
#endif
}

///////////////////////////////////////////////////////////////////////////
// Load the entity from the worldfile
bool CFixedObstacle::Load(CWorldFile *worldfile, int section)
{
  if (!CEntity::Load(worldfile, section))
    return false;

  // Get the name of the image file to load.
  this->filename = worldfile->ReadFilename(section, "file", NULL);
  if (!this->filename || strlen(this->filename) == 0)
  {
    PRINT_ERR("empty filename");
    return false;
  }

  // Get the scale of the image;
  // i.e. the width/length of each pixel in m.
  // If no scale is specified, use the world resolution.
  this->scale = worldfile->ReadLength(section, "scale", 0);
  if (this->scale == 0)
  {
    PRINT_WARN1("no scale specified for image [%s]; using default",
                this->filename);
    this->scale = 1 / m_world->ppm;
  }

  // Get the crop region;
  // i.e. the bit of the image we are interested in
  this->crop_ax = worldfile->ReadTupleLength(section, "crop", 0, -DBL_MAX);
  this->crop_ay = worldfile->ReadTupleLength(section, "crop", 1, -DBL_MAX);
  this->crop_bx = worldfile->ReadTupleLength(section, "crop", 2, +DBL_MAX);
  this->crop_by = worldfile->ReadTupleLength(section, "crop", 3, +DBL_MAX);

  // Create and load the image here (we need to know its size)
  // Try to guess the file type from the extension.
  this->image = new Nimage;
  int len = strlen(this->filename);
  if (strcmp(&(this->filename[len - 4]), ".fig") == 0)
  {
    /* REINSTATE someday
       if (!img.load_fig(this->filename, this->ppm, this->scale))
       return false;
    */
    return false;
  } 
  else if( strcmp( &(this->filename[ len - 7 ]), ".pnm.gz" ) == 0 )
  {
    if (!this->image->load_pnm_gz(this->filename))
      return false;
  }
  else 
  {
    if (!this->image->load_pnm(this->filename))
      return false;
  }

  // Compute the object size based on the image
  this->size_x = this->scale * this->image->width;
  this->size_y = this->scale * this->image->height;

  // draw a border around the image
  this->image->draw_box(0,0,this->image->width-1, this->image->height-1, 255 );

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Initialise object
// Create a new wall object from the pnm file
// If we cant find it under the given name,
// we look for a zipped version.
bool CFixedObstacle::Startup()
{
  if (!CEntity::Startup())
    return false;

  assert(this->image);
  
  // Copy image into matrix
  double ox, oy, oth;
  this->GetGlobalPose(ox, oy, oth);
  double sx = this->scale;
  double sy = this->scale;
  for (int y = 0; y < this->image->height; y++)
  {
    for (int x = 0; x < this->image->width; x++)
    {
      if (this->image->get_pixel(x, y) == 0)
        continue;
      double lx = x * sx;
      double ly = (this->image->height - y) * sy;
      double px = ox + lx * cos(oth) - ly * sin(oth);
      double py = oy + lx * sin(oth) + ly * cos(oth);
      if (px < this->crop_ax || px > this->crop_bx)
        continue;
      if (py < this->crop_ay || py > this->crop_by)
        continue;
      m_world->SetRectangle(px, py, oth, sx, sy, this, true);
    }
  }

#ifdef INCLUDE_RTK2
  // Draw the image into the GUI
  rtk_fig_clear(this->fig);
  rtk_fig_layer(this->fig, -48);
  rtk_fig_color_rgb32(this->fig, this->color);
  for (int y = 0; y < this->image->height; y++)
  {
    for (int x = 0; x < this->image->width; x++)
    {
      if (this->image->get_pixel(x, y) == 0)
        continue;
      double px = x * sx;
      double py = (this->image->height - y) * sy;
      if (px < this->crop_ax || px > this->crop_bx)
        continue;
      if (py < this->crop_ay || py > this->crop_by)
        continue;
      rtk_fig_rectangle(this->fig, px, py, oth, sx, sy, true);
    }
  }
#endif

  // new: don't delete the image so we can download it to clients - rtv

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Finalize object
void CFixedObstacle::Shutdown()
{
  CEntity::Shutdown();
}






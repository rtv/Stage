///////////////////////////////////////////////////////////////////////////
//
// File: model_blobfinder.c
// Author: Richard Vaughan
// Date: 10 June 2004
//
// CVS info:
//  $Source:
//  /home/tcollett/stagecvs/playerstage-cvs/code/stage/libstage/model_blobfinder.cc,v
//  $
//  $Author: rtv $
//  $Revision$
//
///////////////////////////////////////////////////////////////////////////

//#define DEBUG

#include <sys/time.h>

#include "option.hh"
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

static const watts_t DEFAULT_BLOBFINDERWATTS = 2.0;
static const meters_t DEFAULT_BLOBFINDERRANGE = 12.0;
static const radians_t DEFAULT_BLOBFINDERFOV = M_PI / 3.0;
static const radians_t DEFAULT_BLOBFINDERPAN = 0.0;
static const unsigned int DEFAULT_BLOBFINDERSCANWIDTH = 80;
static const unsigned int DEFAULT_BLOBFINDERSCANHEIGHT = 60;

// todo?
// static const unsigned int DEFAULT_BLOBFINDERINTERVAL_MS = 100;
// static const unsigned int DEFAULT_BLOBFINDERRESOLUTION = 1;

/**
   @ingroup model
   @defgroup model_blobfinder Blobfinder model

   The blobfinder model simulates a color-blob-finding vision device,
   like a CMUCAM2, or the ACTS image processing software. It can track
   areas of color in a simulated 2D image, giving the location and size
   of the color 'blobs'. Multiple colors can be tracked at once; they are
   separated into channels, so that e.g. all red objects are tracked as
   channel one, blue objects in channel two, etc. The color associated
   with each channel is configurable.

   API: Stg::ModelBlobfinder

   <h2>Worldfile properties</h2>

   @par Summary and default values

   @verbatim
   blobfinder
   (
   # blobfinder properties
   colors_count 0
   colors [ ]
   image[ 80 60 ]
   range 12.0
   fov 3.14159/3.0
   pan 0.0

   # model properties
   size [ 0.0 0.0 0.0 ]
   )
   @endverbatim

   @par Details

   - colors_count <int>\n
   number of colors being tracked
   - colors [ col1:<string> col2:<string> ... ]\n
   A list of quoted strings defining the colors detected in each channel, using
   color names from the X11-style color database (ex. "black", "red").
   The number of strings should match colors_count.
   - image [ width:<int> height:<int> ]\n
   dimensions of the image in pixels. This determines the blobfinder's
   resolution
   - range <float>\n
   maximum range of the sensor in meters.

*/

ModelBlobfinder::ModelBlobfinder(World *world, Model *parent, const std::string &type)
    : Model(world, parent, type), vis(world), blobs(), colors(), fov(DEFAULT_BLOBFINDERFOV),
      pan(DEFAULT_BLOBFINDERPAN), range(DEFAULT_BLOBFINDERRANGE),
      scan_height(DEFAULT_BLOBFINDERSCANHEIGHT), scan_width(DEFAULT_BLOBFINDERSCANWIDTH)
{
  PRINT_DEBUG2("Constructing ModelBlobfinder %u (%s)\n", id, type.c_str());

  ClearBlocks();

  AddVisualizer(&this->vis, true);
}

ModelBlobfinder::~ModelBlobfinder(void)
{
}

static bool blob_match(Model *candidate, const Model *finder, const void *dummy)
{
  (void)dummy; // avoid warning about unused var

  return (!finder->IsRelated(candidate));
}

static bool ColorMatchIgnoreAlpha(Color a, Color b)
{
  double epsilon = 1e-5; // small
  return (fabs(a.r - b.r) < epsilon && fabs(a.g - b.g) < epsilon && fabs(a.b - b.b) < epsilon);
}

void ModelBlobfinder::ModelBlobfinder::AddColor(Color col)
{
  colors.push_back(col);
}

/** Stop tracking blobs with this color */
void ModelBlobfinder::RemoveColor(Color col)
{
  FOR_EACH (it, colors) {
    if ((*it) == col)
      it = colors.erase(it);
  }
}

/** Stop tracking all colors. Call this to clear the defaults, then
    add colors individually with AddColor(); */
void ModelBlobfinder::RemoveAllColors()
{
  colors.clear();
}

void ModelBlobfinder::Load(void)
{
  Model::Load();

  Worldfile *wf = world->GetWorldFile();

  wf->ReadTuple(wf_entity, "image", 0, 2, "uu", &scan_width, &scan_height);

  range = wf->ReadFloat(wf_entity, "range", range);
  fov = wf->ReadAngle(wf_entity, "fov", fov);
  pan = wf->ReadAngle(wf_entity, "pan", pan);

  if (wf->PropertyExists(wf_entity, "colors")) {
    RemoveAllColors(); // empty the color list to start from scratch

    unsigned int count = wf->ReadInt(wf_entity, "colors_count", 0);

    for (unsigned int c = 0; c < count; c++) {
      char *colorstr = NULL;
      wf->ReadTuple(wf_entity, "colors", c, 1, "s", &colorstr);

      if (!colorstr)
        break;
      else
        AddColor(Color(colorstr));
    }
  }
}

void ModelBlobfinder::Update(void)
{
  // generate a scan for post-processing into a blob image
  std::vector<RaytraceResult> samples(scan_width);

  Raytrace(Pose(0, 0, 0, pan), range, fov, blob_match, NULL, false, samples);

  // now the colors and ranges are filled in - time to do blob detection
  double yRadsPerPixel = fov / scan_height;

  blobs.clear();

  // scan through the samples looking for color blobs
  for (unsigned int s = 0; s < scan_width; s++) {
    if (samples[s].mod == NULL)
      continue; // we saw nothing

    unsigned int right = s;
    Color blobcol = samples[s].color;

    // printf( "blob start %d color %X\n", blobleft, blobcol );

    // loop until we hit the end of the blob
    // there has to be a gap of >1 pixel to end a blob
    // this avoids getting lots of crappy little blobs
    while (s < scan_width && samples[s].mod && ColorMatchIgnoreAlpha(samples[s].color, blobcol)) {
      // printf( "%u blobcol %X block %p %s color %X\n", s, blobcol,
      // samples[s].block, samples[s].block->Model()->Token(),
      // samples[s].block->Color() );
      s++;
    }

    unsigned int left = s - 1;

    // if we have color filters in place, check to see if we're looking for this
    // color
    if (colors.size()) {
      bool found = false;

      for (unsigned int c = 0; c < colors.size(); c++)
        if (ColorMatchIgnoreAlpha(blobcol, colors[c])) {
          found = true;
          break;
        }
      if (!found)
        continue; // continue scanning array for next blob
    }

    // printf( "blob end %d %X\n", blobright, blobcol );

    double robotHeight = 0.6; // meters

    // find the average range to the blob;
    meters_t range = 0;
    for (unsigned int t = right; t <= left; t++)
      range += samples[t].range;
    range /= left - right + 1;

    double startyangle = atan2(robotHeight / 2.0, range);
    double endyangle = -startyangle;
    int blobtop = scan_height / 2 - (int)(startyangle / yRadsPerPixel);
    int blobbottom = scan_height / 2 - (int)(endyangle / yRadsPerPixel);

    blobtop = std::max(blobtop, 0);
    blobbottom = std::min(blobbottom, (int)scan_height);

    // fill in an array entry for this blob
    Blob blob;
    blob.color = blobcol;
    blob.left = scan_width - left - 1;
    blob.top = blobtop;
    blob.right = scan_width - right - 1;
    ;
    blob.bottom = blobbottom;
    blob.range = range;

    // printf( "Robot %p sees %d xpos %d ypos %d\n",
    //  mod, blob.color, blob.xpos, blob.ypos );

    // add the blob to our stash
    // g_array_append_val( blobs, blob );
    blobs.push_back(blob);
  }

  Model::Update();
}

void ModelBlobfinder::Startup(void)
{
  Model::Startup();

  PRINT_DEBUG("blobfinder startup");

  // start consuming power
  SetWatts(DEFAULT_BLOBFINDERWATTS);
}

void ModelBlobfinder::Shutdown(void)
{
  PRINT_DEBUG("blobfinder shutdown");

  // stop consuming power
  SetWatts(0);

  // clear the data - this will unrender it too
  blobs.clear();

  Model::Shutdown();
}

//******************************************************************************
// visualization

// TODO make instance attempt to register an option (as customvisualizations do)
// Option ModelBlobfinder::showBlobData( "Show Blobfinder", "show_blob", "",
// true, NULL );

ModelBlobfinder::Vis::Vis(World *) : Visualizer("Blobfinder", "blobfinder_vis")
{
  // world->RegisterOption( &showArea );
  // world->RegisterOption( &showStrikes );
  // world->RegisterOption( &showFov );
  // world->RegisterOption( &showBeams );
}

void ModelBlobfinder::Vis::Visualize(Model *mod, Camera *cam)
{
  ModelBlobfinder *bf(dynamic_cast<ModelBlobfinder *>(mod));

  if (bf->debug) {
    // draw the FOV
    GLUquadric *quadric = gluNewQuadric();

    bf->PushColor(0, 0, 0, 0.2);

    gluQuadricDrawStyle(quadric, GLU_SILHOUETTE);
    gluPartialDisk(quadric, 0, bf->range,
                   20, // slices
                   1, // loops
                   rtod(M_PI / 2.0 + bf->fov / 2.0 - bf->pan), // start angle
                   rtod(-bf->fov)); // sweep angle

    gluDeleteQuadric(quadric);
    bf->PopColor();
  }

  if (bf->subs < 1)
    return;

  glPushMatrix();

  // return to global rotation frame
  Pose gpose(bf->GetGlobalPose());
  glRotatef(rtod(-gpose.a), 0, 0, 1);

  // place the "screen" a little away from the robot
  glTranslatef(-2.5, -1.5, 0.5);

  // rotate to face screen
  float yaw, pitch;
  pitch = -cam->pitch();
  yaw = -cam->yaw();
  float robotAngle = -rtod(bf->pose.a);
  glRotatef(robotAngle - yaw, 0, 0, 1);
  glRotatef(-pitch, 1, 0, 0);

  // convert blob pixels to meters scale - arbitrary
  glScalef(0.025, 0.025, 1);

  // draw a white screen with a black border
  bf->PushColor(1, 1, 1, 1);
  glRectf(0, 0, bf->scan_width, bf->scan_height);
  bf->PopColor();

  glTranslatef(0, 0, 0.01);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  bf->PushColor(1, 0, 0, 1);
  glRectf(0, 0, bf->scan_width, bf->scan_height);
  bf->PopColor();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // draw the blobs on the screen
  for (unsigned int s = 0; s < bf->blobs.size(); s++) {
    Blob *b = &bf->blobs[s];
    // blobfinder_blob_t* b =
    //&g_array_index( blobs, blobfinder_blob_t, s);

    bf->PushColor(b->color);
    glRectf(b->left, b->top, b->right, b->bottom);

    // printf( "%u l %u t%u r %u b %u\n", s, b->left, b->top, b->right,
    // b->bottom );
    bf->PopColor();
  }

  glPopMatrix();
}

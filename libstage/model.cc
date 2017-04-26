/**
    $Rev$
**/

/** @defgroup model

    The basic model simulates an object with basic properties; position,
    size,  color, visibility to various sensors, etc. The basic
    model also has a body made up of a list of lines. Internally, the
    basic model is used as the base class for all other model types. You
    can use the basic model to simulate environmental objects.

    API: Stg::Model

    <h2>Worldfile properties</h2>

    @par Summary and default values

    @verbatim
    model
    (
    pose [ 0.0 0.0 0.0 0.0 ]
    size [ 0.1 0.1 0.1 ]
    origin [ 0.0 0.0 0.0 0.0 ]

    update_interval 100

    color "red"
    bitmap ""
    ctrl ""

    # determine how the model appears in various sensors
    fiducial_return 0
    fiducial_key 0
    obstacle_return 1
    ranger_return 1.0
    blob_return 1
    gripper_return 0

    # GUI properties
    gui_nose 0
    gui_grid 0
    gui_outline 1
    gui_move 0 (1 if the model has no parents);

    boundary 0
    mass 10.0
    map_resolution 0.1
    say ""
    alwayson 0

    stack_children 1
    )
    @endverbatim

    @par Details

    - pose [ x:<float> y:<float> z:<float> heading:<float> ] \n
    specify the pose of the model in its parent's coordinate system

    - size [ x:<float> y:<float> z:<float> ]\n specify the size of the
    model in each dimension

    - origin [ x:<float> y:<float> z:<float> heading:<float> ]\n
    specify the position of the object's center, relative to its pose

    - update_interval int (defaults to 100) The amount of simulated
    time in milliseconds between calls to Model::Update(). Controls
    the frequency with which this model's data is generated.

    - color <string>\n specify the color of the object using a color
    name from the X11 database (rgb.txt)

    - bitmap filename:<string>\n Draw the model by interpreting the
    lines in a bitmap (bmp, jpeg, gif, png supported). The file is
    opened and parsed into a set of lines.  The lines are scaled to
    fit inside the rectangle defined by the model's current size.

    - ctrl <string>\n Specify the controller module for the model, and
    its argument string. For example, the string "foo bar bash" will
    load libfoo.so, which will have its Init() function called with
    the entire string as an argument (including the library name). It
    is up to the controller to parse the string if it needs
    arguments."

    - fiducial_return fiducial_id:<int>\n if non-zero, this model is
    detected by fiducialfinder sensors. The value is used as the
    fiducial ID.

    - fiducial_key <int> models are only detected by fiducialfinders
    if the fiducial_key values of model and fiducialfinder match. This
    allows you to have several independent types of fiducial in the
    same environment, each type only showing up in fiducialfinders
    that are "tuned" for it.

    - obstacle_return <int>\n if 1, this model can collide with other
    models that have this property set

    - blob_return <int>\n if 1, this model can be detected in the
    blob_finder (depending on its color)

    - ranger_return <float>\n This model is detected with this
    reflectance value by ranger_model sensors. If negative, this model
    is invisible to ranger sensors, and does not block propogation of
    range-sensing rays. This models an idealized reflectance sensor,
    and replaces the normal/bright reflectance of deprecated laser
    model. Defaults to 1.0.

    - gripper_return <int>\n iff 1, this model can be gripped by a
    gripper and can be pushed around by collisions with anything that
    has a non-zero obstacle_return.

    - gui_nose <int>\n if 1, draw a nose on the model showing its
    heading (positive X axis)

    - gui_grid <int>\n if 1, draw a scaling grid over the model

    - gui_outline <int>\n if 1, draw a bounding box around the model,
    indicating its size

    - gui_move <int>\n if 1, the model can be moved by the mouse in
    the GUI window

    - stack_children <int>\n If non-zero (the default), the coordinate
    system of child models is offset in z so that its origin is on
    _top_ of this model, making it easy to stack models together. If
    zero, the child coordinate system is not offset in z, making it
    easy to define objects in a single local coordinate system.
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <ltdl.h> // for library module loading
#include <map>
#include <sstream> // for converting values to strings

#include "config.h" // for build-time config
#include "file_manager.hh"
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

// static members
uint32_t Model::count(0);
std::map<Stg::id_t, Model *> Model::modelsbyid;
std::map<std::string, creator_t> Model::name_map;

// static const members
static const double DEFAULT_FRICTION = 0.0;

Bounds &Bounds::Load(Worldfile *wf, const int section, const char *keyword)
{
  wf->ReadTuple(section, keyword, 0, 2, "ll", &min, &max);
  return *this;
}

double Bounds::Constrain(double value)
{
  return Stg::constrain(value, min, max);
}

Stg::Size &Stg::Size::Load(Worldfile *wf, const int section, const char *keyword)
{
  wf->ReadTuple(section, keyword, 0, 3, "lll", &x, &y, &z);
  return *this;
}

void Stg::Size::Save(Worldfile *wf, int section, const char *keyword) const
{
  wf->WriteTuple(section, keyword, 0, 3, "lll", x, y, z);
}

Pose &Pose::Load(Worldfile *wf, const int section, const char *keyword)
{
  wf->ReadTuple(section, keyword, 0, 4, "llla", &x, &y, &z, &a);
  normalize(a);
  return *this;
}

void Pose::Save(Worldfile *wf, const int section, const char *keyword)
{
  wf->WriteTuple(section, keyword, 0, 4, "llla", x, y, z, a);
}

Model::Visibility::Visibility()
    : blob_return(true), fiducial_key(0), fiducial_return(0), gripper_return(false),
      obstacle_return(true), ranger_return(1.0)
{ /* nothing to do */
}

Model::Visibility &Model::Visibility::Load(Worldfile *wf, int wf_entity)
{
  blob_return = wf->ReadInt(wf_entity, "blob_return", blob_return);
  fiducial_key = wf->ReadInt(wf_entity, "fiducial_key", fiducial_key);
  fiducial_return = wf->ReadInt(wf_entity, "fiducial_return", fiducial_return);
  gripper_return = wf->ReadInt(wf_entity, "gripper_return", gripper_return);
  obstacle_return = wf->ReadInt(wf_entity, "obstacle_return", obstacle_return);
  ranger_return = wf->ReadFloat(wf_entity, "ranger_return", ranger_return);

  return *this;
}

void Model::Visibility::Save(Worldfile *wf, int wf_entity)
{
  wf->WriteInt(wf_entity, "blob_return", blob_return);
  wf->WriteInt(wf_entity, "fiducial_key", fiducial_key);
  wf->WriteInt(wf_entity, "fiducial_return", fiducial_return);
  wf->WriteInt(wf_entity, "gripper_return", gripper_return);
  wf->WriteInt(wf_entity, "obstacle_return", obstacle_return);
  wf->WriteFloat(wf_entity, "ranger_return", ranger_return);
}

Model::GuiState::GuiState() : grid(false), move(false), nose(false), outline(false)
{ /* nothing to do */
}

Model::GuiState &Model::GuiState::Load(Worldfile *wf, int wf_entity)
{
  nose = wf->ReadInt(wf_entity, "gui_nose", nose);
  grid = wf->ReadInt(wf_entity, "gui_grid", grid);
  outline = wf->ReadInt(wf_entity, "gui_outline", outline);
  move = wf->ReadInt(wf_entity, "gui_move", move);

  return *this;
}

// constructor
Model::Model(World *world, Model *parent, const std::string &type, const std::string &name)
    : Ancestor(), mapped(false), drawOptions(), alwayson(false), blockgroup(*this), boundary(false),
      callbacks(__CB_TYPE_COUNT), // one slot in the vector for each type
      color(1, 0, 0), // red
      data_fresh(false), disabled(false), cv_list(), flag_list(), friction(DEFAULT_FRICTION),
      geom(), has_default_block(true), id(Model::count++), interval((usec_t)1e5), // 100msec
      interval_energy((usec_t)1e5), // 100msec
      last_update(0), log_state(false), map_resolution(0.1), mass(0), parent(parent), pose(),
      power_pack(NULL), pps_charging(), rastervis(), rebuild_displaylist(true), say_string(),
      stack_children(true), stall(false), subs(0), thread_safe(false), trail(20),
      trail_index(0),  trail_interval(10), type(type), event_queue_num(0), used(false), watts(0.0), watts_give(0.0),
      watts_take(0.0), wf(NULL), wf_entity(0), world(world),
      world_gui(dynamic_cast<WorldGui *>(world))
{
  assert(world);

  PRINT_DEBUG3("Constructing model world: %s parent: %s type: %s \n", world->Token(),
               parent ? parent->Token() : "(null)", type.c_str());

  modelsbyid[id] = this;

  if (name.size()) // use a name if specified
  {
    // printf( "name set %s\n", name.c_str() );
    SetToken(name);
  } else // if a name was not specified make up a name based on the parent's
  // name,  model type and the number of instances so far
  {
    char buf[2048];
    //  printf( "adding child of type %d token %s\n", mod->type, mod->Token() );

    // prefix with parent name if any, followed by the type name
    // with a suffix of a colon and the parent's number of children
    // of this type
    if (parent) {
      snprintf(buf, 2048, "%s.%s:%u", parent->Token(), type.c_str(),
               parent->child_type_counts[type]);
    } else // no parent, so use the count of this type in the world
    {
      snprintf(buf, 2048, "%s:%u", type.c_str(), world->child_type_counts[type]);
    }

    // printf( "generated name %s\n", buf );
    SetToken(buf);
  }

  //  printf( "%s generated a name for my child %s\n", Token(),
  //  name.str().c_str() );

  world->AddModel(this);

  if (parent)
    parent->AddChild(this);
  else {
    world->AddChild(this);
    // top level models are draggable in the GUI by default
    gui.move = true;
  }

  //static size_t count=0;
  //printf( "basic %lu\n", ++count );

  // now we can add the basic square shape
  AddBlockRect(-0.5, -0.5, 1.0, 1.0, 1.0);

  AddVisualizer(&rastervis, false);

  PRINT_DEBUG2("finished model %s @ %p", this->Token(), this);
}

Model::~Model(void)
{
  // children are removed in ancestor class

  if (world) // if I'm not a worldless dummy model
  {
    UnMap(); // remove from all layers

    // remove myself from my parent's child list, or the world's child
    // list if I have no parent
    EraseAll(this, parent ? parent->children : world->children);
    // erase from the static map of all models
    modelsbyid.erase(id);

    world->RemoveModel(this);
  }
}

void Model::InitControllers()
{
  CallCallbacks(CB_INIT);
}

void Model::AddFlag(Flag *flag)
{
  if (flag) {
    flag_list.push_back(flag);
    CallCallbacks(CB_FLAGINCR);
  }
}

void Model::RemoveFlag(Flag *flag)
{
  if (flag) {
    EraseAll(flag, flag_list);
    CallCallbacks(CB_FLAGDECR);
  }
}

void Model::PushFlag(Flag *flag)
{
  if (flag) {
    flag_list.push_front(flag);
    CallCallbacks(CB_FLAGINCR);
  }
}

Model::Flag *Model::PopFlag()
{
  if (flag_list.size() == 0)
    return NULL;

  Flag *flag = flag_list.front();
  flag_list.pop_front();

  CallCallbacks(CB_FLAGDECR);

  return flag;
}

void Model::ClearBlocks()
{
  UnMap();
  blockgroup.Clear();
  // no need to Map() -  we have no blocks
  NeedRedraw();
}

void Model::LoadBlock(Worldfile *wf, int entity)
{
  if (has_default_block) {
    blockgroup.Clear();
    has_default_block = false;
  }

  blockgroup.LoadBlock(wf, entity);
}

void Model::AddBlockRect(meters_t x, meters_t y, meters_t dx, meters_t dy, meters_t dz)
{
  UnMap();
    
  std::vector<point_t> pts(4);
  pts[0].x = x;
  pts[0].y = y;
  pts[1].x = x + dx;
  pts[1].y = y;
  pts[2].x = x + dx;
  pts[2].y = y + dy;
  pts[3].x = x;
  pts[3].y = y + dy;

  blockgroup.AppendBlock(Block(&blockgroup, pts, Bounds(0, dz)));

  Map();
}

// convert a global pose into the model's local coordinate system
Pose Model::GlobalToLocal(const Pose &pose) const
{
  // get model's global pose
  const Pose org(GetGlobalPose());
  const double cosa(cos(org.a));
  const double sina(sin(org.a));

  // compute global pose in local coords
  return Pose((pose.x - org.x) * cosa + (pose.y - org.y) * sina,
              -(pose.x - org.x) * sina + (pose.y - org.y) * cosa, pose.z - org.z, pose.a - org.a);
}

void Model::Say(const std::string &str)
{
  say_string = str;
}

// returns true iff model [testmod] is an antecedent of this model
bool Model::IsAntecedent(const Model *testmod) const
{
  if (parent == NULL)
    return false;

  if (parent == testmod)
    return true;

  return parent->IsAntecedent(testmod);
}

// returns true iff model [testmod] is a descendent of this model
bool Model::IsDescendent(const Model *testmod) const
{
  if (this == testmod)
    return true;

  FOR_EACH (it, children)
    if ((*it)->IsDescendent(testmod))
      return true;

  // neither mod nor a child of this matches testmod
  return false;
}

bool Model::IsRelated(const Model *that) const
{
  // is it me?
  if (this == that)
    return true;

  // wind up to top-level object
  const Model *candidate = this;
  while (candidate->parent) {
    // shortcut out if we found it on the way up the tree
    if (candidate->parent == that)
      return true;

    candidate = candidate->parent;
  }

  // we got to our root, so recurse down the tree
  return candidate->IsDescendent(that);
  // TODO: recursive solution is costing us 3% of all compute time! try an
  // iterative solution?
}

point_t Model::LocalToGlobal(const point_t &pt) const
{
  const Pose gpose = LocalToGlobal(Pose(pt.x, pt.y, 0, 0));
  return point_t(gpose.x, gpose.y);
}

std::vector<point_int_t> Model::LocalToPixels(const std::vector<point_t> &local) const
{
  const size_t sz = local.size();

  std::vector<point_int_t> global(sz);

  const Pose gpose(GetGlobalPose() + geom.pose);
  Pose ptpose;

  for (size_t i = 0; i < sz; i++) {
    ptpose = gpose + Pose(local[i].x, local[i].y, 0, 0);

    global[i].x = (int32_t)floor(ptpose.x * world->ppm);
    global[i].y = (int32_t)floor(ptpose.y * world->ppm);
  }

  return global;
}

void Model::MapWithChildren(unsigned int layer)
{
  Map(layer);

  // recursive call for all the model's children
  FOR_EACH (it, children)
    (*it)->MapWithChildren(layer);
}

void Model::MapFromRoot(unsigned int layer)
{
  Root()->MapWithChildren(layer);
}

void Model::UnMapWithChildren(unsigned int layer)
{
  UnMap(layer);

  // recursive call for all the model's children
  FOR_EACH (it, children)
    (*it)->UnMapWithChildren(layer);
}

void Model::UnMapFromRoot(unsigned int layer)
{
  Root()->UnMapWithChildren(layer);
}

void Model::Subscribe(void)
{
  subs++;
  world->total_subs++;
  world->dirty = true; // need redraw

  // printf( "subscribe to %s %d\n", Token(), subs );

  // if this is the first sub, call startup
  if (subs == 1)
    Startup();
}

void Model::Unsubscribe(void)
{
  subs--;
  world->total_subs--;
  world->dirty = true; // need redraw

  // printf( "unsubscribe from %s %d\n", Token(), subs );

  // if this is the last remaining subscriber, shutdown
  if (subs == 0)
    Shutdown();
}

void Model::Print(char *prefix) const
{
  if (prefix)
    printf("%s model ", prefix);
  else
    printf("Model ");

  printf("%s:%s\n", world->Token(), Token());

  FOR_EACH (it, children)
    (*it)->Print(prefix);
}

const char *Model::PrintWithPose() const
{
  const Pose gpose = GetGlobalPose();

  static char txt[256];
  snprintf(txt, sizeof(txt), "%s @ [%.2f,%.2f,%.2f,%.2f]", Token(), gpose.x, gpose.y, gpose.z,
           gpose.a);

  return txt;
}

void Model::Startup(void)
{
  // printf( "Startup model %s\n", this->token );
  // printf( "model %s using queue %d\n", token, event_queue_num );

  // iff we're thread safe, we can use an event queue >0, else 0
  event_queue_num = thread_safe ? world->GetEventQueue(this) : 0;

  world->Enqueue(event_queue_num, interval, this, UpdateWrapper, NULL);

  if (FindPowerPack())
    world->EnableEnergy(this);

  CallCallbacks(CB_STARTUP);
}

void Model::Shutdown(void)
{
  // printf( "Shutdown model %s\n", this->token );
  CallCallbacks(CB_SHUTDOWN);

  world->DisableEnergy(this);

  // allows data visualizations to be cleared.
  NeedRedraw();
}

void Model::Update(void)
{
  // printf( "Q%d model %p %s update\n", event_queue_num, this, Token() );

  last_update = world->sim_time;

  if (subs > 0) // no subscriptions means we don't need to be updated
    world->Enqueue(event_queue_num, interval, this, UpdateWrapper, NULL);

  // if we updated the model then it needs to have its update
  // callback called in series back in the main thread. It's
  // not safe to run user callbacks in a worker thread, as
  // they may make OpenGL calls or unsafe Stage API calls,
  // etc. We queue up the callback into a queue specific to

  if (!callbacks[Model::CB_UPDATE].empty())
    world->pending_update_callbacks[event_queue_num].push(this);
}

void Model::CallUpdateCallbacks(void)
{
  CallCallbacks(CB_UPDATE);
}

meters_t Model::ModelHeight() const
{
  meters_t m_child = 0; // max size of any child
  FOR_EACH (it, children)
    m_child = std::max(m_child, (*it)->ModelHeight());

  // height of model + max( child height )
  return geom.size.z + m_child;
}

void Model::AddToPose(double dx, double dy, double dz, double da)
{
  Pose p(pose);
  p.x += dx;
  p.y += dy;
  p.z += dz;
  p.a += da;

  SetPose(p);
}

void Model::AddToPose(const Pose &pose)
{
  AddToPose(pose.x, pose.y, pose.z, pose.a);
}


bool Model::RandomPoseInFreeSpace(meters_t xmin, meters_t xmax,
				  meters_t ymin, meters_t ymax,
                                  size_t max_iter)
{
  SetPose(Pose::Random(xmin, xmax, ymin, ymax));

  size_t i = 0;
  while (TestCollision() && (max_iter <= 0 || i++ < max_iter))
    SetPose(Pose::Random(xmin, xmax, ymin, ymax));
  return i <= max_iter; // return true if a free pose was found within max iterations
}

void Model::AppendTouchingModels(std::set<Model *> &touchers)
{
  blockgroup.AppendTouchingModels(touchers);
}

Model *Model::TestCollision()
{
  Model *hitmod(blockgroup.TestCollision());

  if (hitmod == NULL)
    FOR_EACH (it, children) {
      hitmod = (*it)->TestCollision();
      if (hitmod)
        break;
    }

  // printf( "mod %s test collision done.\n", token );
  return hitmod;
}

void Model::UpdateCharge()
{
  PowerPack *mypp = FindPowerPack();
  assert(mypp);

  if (watts > 0) // dissipation rate
  {
    // consume  energy stored in the power pack
    mypp->Dissipate(watts * (interval_energy * 1e-6), GetGlobalPose());
  }

  if (watts_give > 0) // transmission to other powerpacks max rate
  {
    // detach charger from all the packs charged last time
    FOR_EACH (it, pps_charging)
      (*it)->ChargeStop();
    pps_charging.clear();

    // run through and update all appropriate touchers
    std::set<Model *> touchers;
    AppendTouchingModels(touchers);

    FOR_EACH (it, touchers) {
      Model *toucher = (*it);
      PowerPack *hispp = toucher->FindPowerPack();

      if (hispp && toucher->watts_take > 0.0) {
        // printf( "   toucher %s can take up to %.2f wats\n",
        //		toucher->Token(), toucher->watts_take );

        const watts_t rate = std::min(watts_give, toucher->watts_take);
        const joules_t amount = rate * interval_energy * 1e-6;

        // printf ( "moving %.2f joules from %s to %s\n",
        //		 amount, token, toucher->token );

        // set his charging flag
        hispp->ChargeStart();

        // move some joules from me to him
        mypp->TransferTo(hispp, amount);

        // remember who we are charging so we can detatch next time
        pps_charging.push_front(hispp);
      }
    }
  }
}

void Model::UpdateTrail()
{
  // get the current item and increment the counter
  TrailItem *item = &trail[trail_index++];

  // record the current info
  item->time = world->sim_time;
  item->pose = GetGlobalPose();
  item->color = color;

  // wrap around ring buffer
  trail_index %= trail.size();
}

Model *Model::GetUnsubscribedModelOfType(const std::string &type) const
{
  if ((this->type == type) && (this->subs == 0))
    return const_cast<Model *>(this); // discard const

  // this model is no use. try children recursively
  FOR_EACH (it, children) {
    Model *found = (*it)->GetUnsubscribedModelOfType(type);
    if (found)
      return found;
  }

  // nothing matching below this model
  return NULL;
}

void Model::NeedRedraw(void)
{
  rebuild_displaylist = true;

  if (parent)
    parent->NeedRedraw();
  else
    world->NeedRedraw();
}

void Model::Redraw(void)
{
  world->Redraw();
}

Model *Model::GetUnusedModelOfType(const std::string &type)
{
  // printf( "searching for type %d in model %s type %d\n", type, token,
  // this->type );

  if ((this->type == type) && (!this->used)) {
    this->used = true;
    return this;
  }

  // this model is no use. try children recursively
  FOR_EACH (it, children) {
    Model *found = (*it)->GetUnusedModelOfType(type);
    if (found)
      return found;
  }

  // nothing matching below this model
  if (!parent)
    PRINT_WARN1("Request for unused model of type %s failed", type.c_str());
  return NULL;
}

kg_t Model::GetTotalMass() const
{
  kg_t sum = mass;

  FOR_EACH (it, children)
    sum += (*it)->GetTotalMass();

  return sum;
}

kg_t Model::GetMassOfChildren() const
{
  return (GetTotalMass() - mass);
}

// render all blocks in the group at my global pose and size
void Model::Map(unsigned int layer)
{
  blockgroup.Map(layer);
}

void Model::UnMap(unsigned int layer)
{
  blockgroup.UnMap(layer);
}

void Model::BecomeParentOf(Model *child)
{
  if (child->parent)
    child->parent->RemoveChild(child);
  else
    world->RemoveChild(child);

  child->parent = this;

  this->AddChild(child);

  world->dirty = true;
}

PowerPack *Model::FindPowerPack() const
{
  if (power_pack)
    return power_pack;

  if (parent)
    return parent->FindPowerPack();

  return NULL;
}

void Model::RegisterOption(Option *opt)
{
  world->RegisterOption(opt);
}

void Model::Rasterize(uint8_t *data, unsigned int width, unsigned int height, meters_t cellwidth,
                      meters_t cellheight)
{
  rastervis.ClearPts();
  blockgroup.Rasterize(data, width, height, cellwidth, cellheight);
  rastervis.SetData(data, width, height, cellwidth, cellheight);
}

void Model::SetFriction(double friction)
{
  this->friction = friction;
}

Model *Model::GetChild(const std::string &modelname) const
{
  // construct the full model name and look it up

  const std::string fullname = token + "." + modelname;

  Model *mod = world->GetModel(fullname);

  if (mod == NULL)
    PRINT_WARN1("Model %s not found", fullname.c_str());

  return mod;
}

//***************************************************************
// Raster data visualizer
//
Model::RasterVis::RasterVis()
    : Visualizer("Rasterization", "raster_vis"), data(NULL), width(0), height(0), cellwidth(0),
      cellheight(0), pts(), subs(0), used(0)
{
}

void Model::RasterVis::Visualize(Model *mod, Camera *cam)
{
  (void)cam; // avoid warning about unused var

  if (data == NULL)
    return;

  // go into world coordinates
  glPushMatrix();
  mod->PushColor(1, 0, 0, 0.5);

  Gl::pose_inverse_shift(mod->GetGlobalPose());

  if (pts.size() > 0) {
    glPushMatrix();
    // Size sz = mod->blockgroup.GetSize();
    // glTranslatef( -mod->geom.size.x / 2.0, -mod->geom.size.y/2.0, 0 );
    // glScalef( mod->geom.size.x / sz.x, mod->geom.size.y / sz.y, 1 );

    // now we're in world meters coordinates
    glPointSize(4);
    glBegin(GL_POINTS);

    FOR_EACH (it, pts) {
      point_t &pt = *it;
      glVertex2f(pt.x, pt.y);

      char buf[128];
      snprintf(buf, 127, "[%.2f x %.2f]", pt.x, pt.y);
      Gl::draw_string(pt.x, pt.y, 0, buf);
    }
    glEnd();

    mod->PopColor();

    glPopMatrix();
  }

  // go into bitmap pixel coords
  glTranslatef(-mod->geom.size.x / 2.0, -mod->geom.size.y / 2.0, 0);
  // glScalef( mod->geom.size.x / width, mod->geom.size.y / height, 1 );

  glScalef(cellwidth, cellheight, 1);

  mod->PushColor(0, 0, 0, 0.5);
  glPolygonMode(GL_FRONT, GL_FILL);
  for (unsigned int y = 0; y < height; ++y)
    for (unsigned int x = 0; x < width; ++x) {
      // printf( "[%u %u] ", x, y );
      if (data[x + y * width])
        glRectf(x, y, x + 1, y + 1);
    }

  glTranslatef(0, 0, 0.01);

  mod->PushColor(0, 0, 0, 1);
  glPolygonMode(GL_FRONT, GL_LINE);
  for (unsigned int y = 0; y < height; ++y)
    for (unsigned int x = 0; x < width; ++x) {
      if (data[x + y * width])
        glRectf(x, y, x + 1, y + 1);

      // 		  char buf[128];
      // 		  snprintf( buf, 127, "[%u x %u]", x, y );
      // 		  Gl::draw_string( x, y, 0, buf );
    }

  glPolygonMode(GL_FRONT, GL_FILL);

  mod->PopColor();
  mod->PopColor();

  mod->PushColor(0, 0, 0, 1);
  char buf[128];
  snprintf(buf, 127, "[%u x %u]", width, height);
  glTranslatef(0, 0, 0.01);
  Gl::draw_string(1, height - 1, 0, buf);

  mod->PopColor();

  glPopMatrix();
}

void Model::RasterVis::SetData(uint8_t *data, const unsigned int width, const unsigned int height,
                               const meters_t cellwidth, const meters_t cellheight)
{
  // copy the raster for test visualization
  if (this->data)
    delete[] this->data;
  size_t len = sizeof(uint8_t) * width * height;
  // printf( "allocating %lu bytes\n", len );
  this->data = new uint8_t[len];
  memcpy(this->data, data, len);
  this->width = width;
  this->height = height;
  this->cellwidth = cellwidth;
  this->cellheight = cellheight;
}

void Model::RasterVis::AddPoint(const meters_t x, const meters_t y)
{
  pts.push_back(point_t(x, y));
}

void Model::RasterVis::ClearPts()
{
  pts.clear();
}

Model::Flag::Flag(const Color &color, const double size) : color(color), size(size), displaylist(0)
{
}

Model::Flag *Model::Flag::Nibble(double chunk)
{
  Flag *piece = NULL;

  if (size > 0) {
    chunk = std::min(chunk, this->size);
    piece = new Flag(this->color, chunk);
    this->size -= chunk;
  }

  return piece;
}

void Model::Flag::SetColor(const Color &c)
{
  color = c;

  if (displaylist) {
    // force recreation of list
    glDeleteLists(displaylist, 1);
    displaylist = 0;
  }
}

void Model::Flag::SetSize(double sz)
{
  size = sz;

  if (displaylist) {
    // force recreation of list
    glDeleteLists(displaylist, 1);
    displaylist = 0;
  }
}

void Model::Flag::Draw(GLUquadric *quadric)
{
  if (displaylist == 0) {
    displaylist = glGenLists(1);
    assert(displaylist > 0);

    glNewList(displaylist, GL_COMPILE);

    glColor4f(color.r, color.g, color.b, color.a);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluSphere(quadric, size / 2.0, 4, 2);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // draw the edges darker version of the same color
    glColor4f(color.r / 2.0, color.g / 2.0, color.b / 2.0, color.a / 2.0);

    gluQuadricDrawStyle(quadric, GLU_LINE);
    gluSphere(quadric, size / 2.0, 4, 2);

    glEndList();
  }

  glCallList(displaylist);
}

void Model::SetGeom(const Geom &val)
{
  UnMapWithChildren(0);
  UnMapWithChildren(1);

  geom = val;

  blockgroup.CalcSize();

  // printf( "model %s SetGeom size [%.3f %.3f %.3f]\n", Token(), geom.size.x,
  // geom.size.y, geom.size.z );

  NeedRedraw();

  MapWithChildren(0);
  MapWithChildren(1);

  CallCallbacks(CB_GEOM);
}

void Model::SetColor(Color val)
{
  color = val;
  NeedRedraw();
}

void Model::SetMass(kg_t val)
{
  mass = val;
}

void Model::SetStall(bool val)
{
  stall = val;
}

void Model::SetGripperReturn(bool val)
{
  vis.gripper_return = val;
}

void Model::SetFiducialReturn(int val)
{
  vis.fiducial_return = val;

  // non-zero values mean we need to be in the world's set of
  // detectable models
  if (val == 0)
    world->FiducialErase(this);
  else
    world->FiducialInsert(this);
}

void Model::SetFiducialKey(int val)
{
  vis.fiducial_key = val;
}

void Model::SetObstacleReturn(bool val)
{
  vis.obstacle_return = val;
}

void Model::SetBlobReturn(bool val)
{
  vis.blob_return = val;
}

void Model::SetRangerReturn(double val)
{
  vis.ranger_return = val;
}

void Model::SetBoundary(bool val)
{
  boundary = val;
}

void Model::SetGuiNose(bool val)
{
  gui.nose = val;
}

void Model::SetGuiMove(bool val)
{
  gui.move = val;
}

void Model::SetGuiGrid(bool val)
{
  gui.grid = val;
}

void Model::SetGuiOutline(bool val)
{
  gui.outline = val;
}

void Model::SetWatts(watts_t val)
{
  watts = val;
}

void Model::SetMapResolution(meters_t val)
{
  map_resolution = val;
}

// set the pose of model in global coordinates
void Model::SetGlobalPose(const Pose &gpose)
{
  SetPose(parent ? parent->GlobalToLocal(gpose) : gpose);
}

int Model::SetParent(Model *newparent)
{
  Pose oldPose = GetGlobalPose();

  // remove the model from its old parent (if it has one)
  if (parent)
    parent->RemoveChild(this);
  else
    world->RemoveChild(this);
  // link from the model to its new parent
  this->parent = newparent;

  if (newparent)
    newparent->AddChild(this);
  else
    world->AddModel(this);

  CallCallbacks(CB_PARENT);

  SetGlobalPose(oldPose); // Needs to recalculate position due to change in parent

  return 0; // ok
}

// get the model's position in the global frame
Pose Model::GetGlobalPose() const
{
  // if I'm a top level model, my global pose is my local pose
  if (parent == NULL)
    return pose;

  // otherwise
  Pose global_pose = parent->GetGlobalPose() + pose;

  if (parent->stack_children) // should we be on top of our parent?
    global_pose.z += parent->geom.size.z;

  return global_pose;
}

// set the model's pose in the local frame
void Model::SetPose(const Pose &newpose)
{
  // if the pose has changed, we need to do some work
  if (pose != newpose) {
    pose = newpose;
    pose.a = normalize(pose.a);

    //       if( isnan( pose.a ) )
    // 		  printf( "SetPose bad angle %s [%.2f %.2f %.2f %.2f]\n",
    // 					 token, pose.x, pose.y, pose.z, pose.a
    // );

    NeedRedraw();

    UnMapWithChildren(0);
    UnMapWithChildren(1);

    MapWithChildren(0);
    MapWithChildren(1);

    world->dirty = true;
  }

  CallCallbacks(CB_POSE);
}

void Model::Load()
{
  assert(wf);
  assert(wf_entity);

  PRINT_DEBUG1("Model \"%s\" loading...", token.c_str());

  // choose the thread to run in, if thread_safe > 0
  event_queue_num = wf->ReadInt(wf_entity, "event_queue", event_queue_num);

  if (wf->PropertyExists(wf_entity, "joules")) {
    if (!power_pack)
      power_pack = new PowerPack(this);

    joules_t j = wf->ReadFloat(wf_entity, "joules", power_pack->GetStored());

    /* assume that the store is full, so the capacity is the same as
     the charge */
    power_pack->SetStored(j);
    power_pack->SetCapacity(j);
  }

  if (wf->PropertyExists(wf_entity, "joules_capacity")) {
    if (!power_pack)
      power_pack = new PowerPack(this);

    power_pack->SetCapacity(wf->ReadFloat(wf_entity, "joules_capacity", power_pack->GetCapacity()));
  }

  if (wf->PropertyExists(wf_entity, "kjoules")) {
    if (!power_pack)
      power_pack = new PowerPack(this);

    joules_t j = 1000.0 * wf->ReadFloat(wf_entity, "kjoules", power_pack->GetStored());

    /* assume that the store is full, so the capacity is the same as
     the charge */
    power_pack->SetStored(j);
    power_pack->SetCapacity(j);
  }

  if (wf->PropertyExists(wf_entity, "kjoules_capacity")) {
    if (!power_pack)
      power_pack = new PowerPack(this);

    power_pack->SetCapacity(
        1000.0 * wf->ReadFloat(wf_entity, "kjoules_capacity", power_pack->GetCapacity()));
  }

  watts = wf->ReadFloat(wf_entity, "watts", watts);
  watts_give = wf->ReadFloat(wf_entity, "give_watts", watts_give);
  watts_take = wf->ReadFloat(wf_entity, "take_watts", watts_take);

  debug = wf->ReadInt(wf_entity, "debug", debug);

  const std::string &name = wf->ReadString(wf_entity, "name", token);
  if (name != token)
    SetToken(name);

  // PRINT_WARN1( "%s::Load", token );

  Geom g(GetGeom());

  if (wf->PropertyExists(wf_entity, "origin"))
    g.pose.Load(wf, wf_entity, "origin");

  if (wf->PropertyExists(wf_entity, "size"))
    g.size.Load(wf, wf_entity, "size");

  SetGeom(g);

  if (wf->PropertyExists(wf_entity, "pose"))
    SetPose(GetPose().Load(wf, wf_entity, "pose"));

  if (wf->PropertyExists(wf_entity, "color")) {
    Color col(1, 0, 0); // red;
    const std::string &colorstr = wf->ReadString(wf_entity, "color", "");
    if (colorstr != "") {
      if (colorstr == "random")
        col = Color(drand48(), drand48(), drand48());
      else
        col = Color(colorstr);
    }
    this->SetColor(col);
  }

  this->SetColor(GetColor().Load(wf, wf_entity));

  if (wf->ReadInt(wf_entity, "noblocks", 0)) {
    if (has_default_block) {
      blockgroup.Clear();
      has_default_block = false;
      blockgroup.CalcSize();
    }
  }

  if (wf->PropertyExists(wf_entity, "bitmap")) {
    const std::string bitmapfile = wf->ReadString(wf_entity, "bitmap", "");
    if (bitmapfile == "")
      PRINT_WARN1("model %s specified empty bitmap filename\n", Token());

    if (has_default_block) {
      blockgroup.Clear();
      has_default_block = false;
    }

    blockgroup.LoadBitmap(bitmapfile, wf);
  }

  if (wf->PropertyExists(wf_entity, "boundary")) {
    this->SetBoundary(wf->ReadInt(wf_entity, "boundary", this->boundary));

    if (boundary) {
      // PRINT_WARN1( "setting boundary for %s\n", token );

      blockgroup.CalcSize();

      const double epsilon = 0.01;
      const bounds3d_t b = blockgroup.BoundingBox();

      const Size size(b.x.max - b.x.min, b.y.max - b.y.min, b.z.max - b.z.min);

      //static size_t count=0;
      //printf( "boundaries %lu\n", ++count );
      AddBlockRect(b.x.min, b.y.min, epsilon, size.y, size.z);
      AddBlockRect(b.x.min, b.y.min, size.x, epsilon, size.z);
      AddBlockRect(b.x.min, b.y.max - epsilon, size.x, epsilon, size.z);
      AddBlockRect(b.x.max - epsilon, b.y.min, epsilon, size.y, size.z);
    }
  }

  this->stack_children = wf->ReadInt(wf_entity, "stack_children", this->stack_children);

  kg_t m = wf->ReadFloat(wf_entity, "mass", this->mass);
  if (m != this->mass)
    SetMass(m);

  vis.Load(wf, wf_entity);
  SetFiducialReturn(vis.fiducial_return); // may have some work to do

  gui.Load(wf, wf_entity);

  double res = wf->ReadFloat(wf_entity, "map_resolution", this->map_resolution);
  if (res != this->map_resolution)
    SetMapResolution(res);

  if (wf->PropertyExists(wf_entity, "friction")) {
    this->SetFriction(wf->ReadFloat(wf_entity, "friction", this->friction));
  }

  if (CProperty *ctrlp = wf->GetProperty(wf_entity, "ctrl")) {
    for (unsigned int index = 0; index < ctrlp->values.size(); index++) {
      const char *lib = wf->GetPropertyValue(ctrlp, index);

      if (!lib)
        printf("Error - NULL library name specified for model %s\n", Token());
      else
        LoadControllerModule(lib);
    }
  }

  // internally interval is in usec, but we use msec in worldfiles
  interval = 1000 * wf->ReadInt(wf_entity, "update_interval", interval / 1000);

  Say(wf->ReadString(wf_entity, "say", ""));

  int trail_length = wf->ReadInt(wf_entity, "trail_length", (int)trail.size() );
  trail.resize(trail_length);
  trail_interval = wf->ReadInt(wf_entity, "trail_interval", trail_interval);

  this->alwayson = wf->ReadInt(wf_entity, "alwayson", alwayson);
  if (alwayson)
    Subscribe();

  // call any type-specific load callbacks
  this->CallCallbacks(CB_LOAD);

  // we may well have changed blocks or geometry
  blockgroup.CalcSize();

  // remove and re-add to both layers
  UnMapWithChildren(0);
  MapWithChildren(0);

  UnMapWithChildren(1);
  MapWithChildren(1);

  if (this->debug)
    printf("Model \"%s\" is in debug mode\n", Token());

  PRINT_DEBUG1("Model \"%s\" loading complete", Token());
}

void Model::Save(void)
{
  // printf( "Model \"%s\" saving...\n", Token() );

  // some models were not loaded, so have no worldfile. Just bail here.
  if (wf == NULL)
    return;

  assert(wf_entity);

  PRINT_DEBUG5("saving model %s pose [ %.2f, %.2f, %.2f, %.2f]", token.c_str(), pose.x, pose.y,
               pose.z, pose.a);

  // just in case
  pose.a = normalize(pose.a);
  geom.pose.a = normalize(geom.pose.a);

  if (wf->PropertyExists(wf_entity, "pose"))
    pose.Save(wf, wf_entity, "pose");

  if (wf->PropertyExists(wf_entity, "size"))
    geom.size.Save(wf, wf_entity, "size");

  if (wf->PropertyExists(wf_entity, "origin"))
    geom.pose.Save(wf, wf_entity, "origin");

  vis.Save(wf, wf_entity);

  // call any type-specific save callbacks
  CallCallbacks(CB_SAVE);

  PRINT_DEBUG1("Model \"%s\" saving complete.", token.c_str());
}

void Model::LoadControllerModule(const char *lib)
{
  // printf( "[Ctrl \"%s\"", lib );
  // fflush(stdout);

  /* Initialise libltdl. */
  int errors = lt_dlinit();
  if (errors) {
    printf("Libtool error: %s. Failed to init libtool. Quitting\n",
           lt_dlerror()); // report the error from libtool
    puts("libtool error #1");
    fflush(stdout);
    exit(-1);
  }

  lt_dlsetsearchpath(FileManager::stagePath().c_str());

  // printf( "STAGEPATH: %s\n",  FileManager::stagePath().c_str());
  //  printf( "ltdl search path: %s\n", lt_dlgetsearchpath() );

  // PLUGIN_PATH now defined in config.h
  lt_dladdsearchdir(PLUGIN_PATH);

  // printf( "ltdl search path: %s\n", lt_dlgetsearchpath() );

  lt_dlhandle handle = NULL;

  // the library name is the first word in the string
  char libname[256];
  sscanf(lib, "%255s %*s", libname);

  if ((handle = lt_dlopenext(libname))) {
// printf( "]" );
// Refer to:
// http://stackoverflow.com/questions/14543801/iso-c-forbids-casting-between-pointer-to-function-and-pointer-to-object
#ifdef __GNUC__
    __extension__
#endif
        model_callback_t initfunc = (model_callback_t)lt_dlsym(handle, "Init");
    if (initfunc == NULL) {
      printf("(Libtool error: %s.) Something is wrong with your plugin.\n",
             lt_dlerror()); // report the error from libtool
      puts("libtool error #1");
      fflush(stdout);
      exit(-1);
    }

    AddCallback(CB_INIT, initfunc,
                new CtrlArgs(lib,
                             World::ctrlargs)); // pass complete string into initfunc
  } else {
    printf("(Libtool error: %s.) Can't open your plugin.\n",
           lt_dlerror()); // report the error from libtool

    PRINT_ERR1("Failed to open \"%s\". Check that it can be found by searching "
               "the directories in your STAGEPATH environment variable, or the "
               "current directory if STAGEPATH is not set.]\n",
               libname);

    printf("ctrl \"%s\" STAGEPATH \"%s\"\n", libname, PLUGIN_PATH);

    puts("libtool error #2");
    fflush(stdout);
    exit(-1);
  }

  fflush(stdout);
}

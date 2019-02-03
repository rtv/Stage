
/**
   $Id$
**/

/** @defgroup world World

    Stage simulates a 'world' composed of `models', defined in a `world
    file'.

    API: Stg::World

    <h2>Worldfile properties</h2>

    @par Summary and default values

    @verbatim

    name                     <worldfile name>
    interval_sim            100
    quit_time                 0
    resolution                0.02

    show_clock                0
    show_clock_interval     100
    threads                   1

    @endverbatim

    @par Details

    - name <string>\n
    An identifying name for the world, used e.g. in the title bar of
    the GUI.

    - interval_sim <float>\n
    The amount of simulation time run for each call of
    World::Update(). Each model has its own configurable update
    interval, which can be greater or less than this, but intervals
    shorter than this are not visible in the GUI or in World update
    callbacks. You are not likely to need to change the default of 100
    msec: this is used internally by clients such as Player and WebSim.

    - quit_time <float>\n
    Stop the simulation after this many simulated seconds have
    elapsed. In libstage, World::Update() returns true. In Stage with
    a GUI, the simulation is paused.wo In Stage without a GUI, Stage
    quits.

    - resolution <float>\n
    The resolution (in meters) of the underlying bitmap model. Larger
    values speed up raytracing at the expense of fidelity in collision
    detection and sensing. The default is often a reasonable choice.

    - show_clock <int>\n
    If non-zero, print the simulation time on stdout every
    $show_clock_interval updates. Useful to watch the progress of
    non-GUI simulations.

    - show_clock_interval <int>\n
    Sets the number of updates between printing the time on stdoutm,
    if $show_clock is enabled. The default is once every 10 simulated
    seconds. Smaller values slow the simulation down a little.

    - threads <int>\n The number of worker threads to spawn. Some
    models can be updated in parallel (e.g. laser, ranger), and
    running 2 or more threads here may make the simulation run faster,
    depending on the number of CPU cores available and the
    worldfile. As a guideline, use one thread per core if you have
    parallel-enabled high-resolution models, e.g. a laser with
    hundreds or thousands of samples, or lots of models. Defaults to
    1. Values of less than 1 will be forced to 1.

    @par More examples
    The Stage source distribution contains several example world files in
    <tt>(stage src)/worlds</tt> along with the worldfile properties
    described on the manual page for each model type.
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

//#define DEBUG

#include <cmath>
using std::abs;

#include <cstdlib>

#include <stdlib.h>
#include <assert.h>
#include <libgen.h> // for dirname(3)
#include <limits.h>
#include <locale.h>
#include <string.h> // for strdup(3)

#include "file_manager.hh"
#include "option.hh"
#include "region.hh"
#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

// // function objects for comparing model positions
bool World::ltx::operator()(const Model *a, const Model *b) const
{
  const meters_t ax(a->GetGlobalPose().x);
  const meters_t bx(b->GetGlobalPose().x);
  // break ties using the pointer value to give a unique ordering
  return (ax == bx ? a < b : ax < bx);
}
bool World::lty::operator()(const Model *a, const Model *b) const
{
  const meters_t ay(a->GetGlobalPose().y);
  const meters_t by(b->GetGlobalPose().y);
  // break ties using the pointer value ro give a unique ordering
  return (ay == by ? a < b : ay < by);
}

// static data members
unsigned int World::next_id(0);
bool World::quit_all(false);
std::set<World *> World::world_set;
std::string World::ctrlargs;
std::vector<std::string> World::args;

World::World(const std::string &,
             double ppm)
    : // private
      destroy(false),
      dirty(true), models(), models_by_name(), models_with_fiducials(), models_with_fiducials_byx(),
      models_with_fiducials_byy(), ppm(ppm), // raytrace resolution
      quit(false), show_clock(false),
      show_clock_interval(100), // 10 simulated seconds using defaults
      sync_mutex(), threads_working(0), threads_start_cond(), threads_done_cond(), total_subs(0),
      worker_threads(1),

      // protected
      cb_list(), extent(), graphics(false), option_table(), powerpack_list(), quit_time(0),
      ray_list(), sim_time(0), superregions(), updates(0), wf(NULL), paused(false),
      event_queues(1), // use 1 thread by default
      pending_update_callbacks(), active_energy(), active_velocity(),
      sim_interval(1e5), // 100 msec has proved a good default
      update_cb_count(0)
{
  if (!Stg::InitDone()) {
    PRINT_WARN("Stg::Init() must be called before a World is created.");
    exit(-1);
  }

  pthread_mutex_init(&sync_mutex, NULL);
  pthread_cond_init(&threads_start_cond, NULL);
  pthread_cond_init(&threads_done_cond, NULL);

  World::world_set.insert(this);

  ground = new Model(this, NULL, "model");
  assert(ground);
  ground->SetToken("_ground_model"); // allow users to identify this unique model
  AddModelName(ground, ground->Token()); // add this name to the world's table
  ground->ClearBlocks();
  ground->SetGuiMove(false);
}

World::~World(void)
{
  PRINT_DEBUG1("destroying world %s", Token());
  if (ground)
    delete ground;
  if (wf)
    delete wf;
  World::world_set.erase(this);
}

SuperRegion *World::CreateSuperRegion(point_int_t origin)
{
  SuperRegion *sr(new SuperRegion(this, origin));
  superregions[origin] = sr;
  dirty = true; // force redraw
  return sr;
}

void World::DestroySuperRegion(SuperRegion *sr)
{
  superregions.erase(sr->GetOrigin());
  delete sr;
}

void World::Run()
{
  // first check whether there is a single gui world
  bool found_gui = false;
  FOR_EACH (world_it, world_set) {
    found_gui |= (*world_it)->IsGUI();
  }
  if (found_gui && (world_set.size() != 1)) {
    PRINT_WARN("When using the GUI only a single world can be simulated.");
    exit(-1);
  }

  if (found_gui) {
    // roughly equals Fl::run() (see also
    // https://wiki.orfeo-toolbox.org/index.php/How_to_exit_every_fltk_window_in_the_world,
    // FLTK
    // is a piece of crap):
    while (Fl::first_window() && !World::quit_all) {
      Fl::wait();
    }
  } else {
    while (!UpdateAll())
      ;
  }
}

bool World::UpdateAll()
{
  bool quit(true);

  FOR_EACH (world_it, World::world_set) {
    if ((*world_it)->Update() == false)
      quit = false;
  }

  return quit;
}

void *World::update_thread_entry(std::pair<World *, int> *thread_info)
{
  World *world(thread_info->first);
  const int thread_instance(thread_info->second);

  // printf( "thread ID %d waiting for mutex\n", thread_instance );

  pthread_mutex_lock(&world->sync_mutex);

  while (1) {
    // printf( "thread ID %d waiting for start\n", thread_instance );
    // wait until the main thread signals us
    // puts( "worker waiting for start signal" );

    pthread_cond_wait(&world->threads_start_cond, &world->sync_mutex);
    pthread_mutex_unlock(&world->sync_mutex);

    // printf( "worker %u thread awakes for task %u\n", thread_instance, task );
    world->ConsumeQueue(thread_instance);
    // printf( "thread %d done\n", thread_instance );

    // done working, so increment the counter. If this was the last
    // thread to finish working, signal the main thread, which is
    // blocked waiting for this to happen

    pthread_mutex_lock(&world->sync_mutex);
    if (--world->threads_working == 0) {
      // puts( "last worker signalling main thread" );
      pthread_cond_signal(&world->threads_done_cond);
    }
    // keep lock going round the loop
  }

  return NULL;
}

void World::AddModel(Model *mod)
{
  models.insert(mod);
  models_by_name[mod->token] = mod;
}

void World::AddModelName(Model *mod, const std::string &name)
{
  models_by_name[name] = mod;
}

void World::RemoveModel(Model *mod)
{
  // remove all this model's names to the table
  models_by_name.erase(mod->token);

  models.erase(mod);
}

void World::LoadBlock(Worldfile *wf, int entity)
{
  // lookup the group in which this was defined
  Model *mod(models_by_wfentity[wf->GetEntityParent(entity)]);

  if (!mod)
    PRINT_ERR("block has no model for a parent");

  mod->LoadBlock(wf, entity);
}

void World::LoadSensor(Worldfile *wf, int entity)
{
  // lookup the group in which this was defined
  ModelRanger *rgr(dynamic_cast<ModelRanger *>(models_by_wfentity[wf->GetEntityParent(entity)]));

  // todo verify that the parent is indeed a ranger

  if (!rgr)
    PRINT_ERR("block has no ranger model for a parent");

  rgr->LoadSensor(wf, entity);
}

Model *World::CreateModel(Model *parent, const std::string &typestr)
{
  Model *mod(NULL); // new model to return

  // find the creator function pointer in the map. use the
  // vanilla model if the type is NULL.
  creator_t creator = NULL;

  // printf( "creating model of type %s\n", typestr );

  std::map<std::string, creator_t>::iterator it = Model::name_map.find(typestr);

  if (it == Model::name_map.end()) {
    puts("");
    PRINT_ERR1("Model type %s not found in model typetable", typestr.c_str());
  } else
    creator = it->second;

  // if we found a creator function, call it
  if (creator) {
    // printf( "creator fn: %p\n", creator );
    mod = (*creator)(this, parent, typestr);
  } else {
    PRINT_ERR1("Unknown model type %s in world file.", typestr.c_str());
    exit(1);
  }

  // printf( "created model %s\n", mod->Token() );

  return mod;
}

void World::LoadModel(Worldfile *wf, int entity)
{
  const int parent_entity(wf->GetEntityParent(entity));

  PRINT_DEBUG2("wf entity %d parent entity %d\n", entity, parent_entity);

  Model *parent(models_by_wfentity[parent_entity]);

  const char *typestr((char *)wf->GetEntityType(entity));
  assert(typestr);

  Model *mod(CreateModel(parent, typestr));

  // configure the model with properties from the world file
  mod->Load(wf, entity);

  // record the model we created for this worldfile entry
  models_by_wfentity[entity] = mod;
}

bool World::Load(std::istream &world_content, const std::string &worldfile_path)
{
  // note: must call Unload() before calling Load() if a world already
  // exists TODO: unload doesn't clean up enough right now

  printf(" [Loading from stream]");
  fflush(stdout);

  this->wf = new Worldfile();
  if (!wf->Load(world_content, worldfile_path))
    return false;

  // nothing gets added if the string is empty
  std::string tmp = wf->ReadString(0, "name", worldfile_path);
  if (!tmp.empty())
    this->SetToken(tmp);

  LoadWorldPostHook();
  return true;
}

bool World::Load(const std::string &worldfile_path)
{
  // note: must call Unload() before calling Load() if a world already
  // exists TODO: unload doesn't clean up enough right now

  printf(" [Loading %s]", worldfile_path.c_str());
  fflush(stdout);

  this->wf = new Worldfile();
  if (!wf->Load(worldfile_path)) {
    PRINT_ERR1(" Failed to open file %s", worldfile_path.c_str());
    return false;
  }

  PRINT_DEBUG1("wf has %d entitys", wf->GetEntityCount());

  // nothing gets added if the string is empty
  this->SetToken(wf->ReadString(0, "name", worldfile_path));
  LoadWorldPostHook();
  return true;
}

void World::LoadWorldPostHook()
{
  this->quit_time = (usec_t)(million * wf->ReadFloat(0, "quit_time", this->quit_time));

  this->ppm = 1.0 / wf->ReadFloat(0, "resolution", 1.0 / this->ppm);

  this->show_clock = wf->ReadInt(0, "show_clock", this->show_clock);

  this->show_clock_interval = wf->ReadInt(0, "show_clock_interval", this->show_clock_interval);

  // read msec instead of usec: easier for user
  this->sim_interval = 1e3 * wf->ReadFloat(0, "interval_sim", this->sim_interval / 1e3);

  this->worker_threads = wf->ReadInt(0, "threads", this->worker_threads);
  if (this->worker_threads < 1) {
    PRINT_WARN("threads set to <1. Forcing to 1");
    this->worker_threads = 1;
  }

  pending_update_callbacks.resize(worker_threads + 1);
  event_queues.resize(worker_threads + 1);

  // printf( "worker threads %d\n", worker_threads );

  // kick off the threads
  for (unsigned int t(0); t < worker_threads; ++t) {
    // normal posix pthread C function pointer
    typedef void *(*func_ptr)(void *);

    // the pair<World*,int> is the configuration for each thread. it can't be a
    // local
    // stack var, since it's accssed in the threads

    pthread_t pt;
    pthread_create(&pt, NULL, (func_ptr)World::update_thread_entry,
                   new std::pair<World *, int>(this, t + 1));
  }

  if (worker_threads > 1)
    printf("[threads %u]", worker_threads);

  // Iterate through entitys and create objects of the appropriate type
  for (int entity(1); entity < wf->GetEntityCount(); ++entity) {
    const char *typestr = (char *)wf->GetEntityType(entity);

    // don't load window entries here
    if (strcmp(typestr, "window") == 0) {
      /* do nothing here */
    } else if (strcmp(typestr, "block") == 0)
      LoadBlock(wf, entity);
    else if (strcmp(typestr, "sensor") == 0)
      LoadSensor(wf, entity);
    else
      LoadModel(wf, entity);
  }

  // call all controller init functions
  FOR_EACH (it, models) {
    (*it)->blockgroup.CalcSize();
    (*it)->UnMap(); // clears both layers
    (*it)->Map(); // maps both layers

    // to here
  }

  // the world is all done - run any init code for user's controllers
  FOR_EACH (it, models)
    (*it)->InitControllers();

  putchar('\n');
}

void World::UnLoad()
{
  if (wf)
    delete wf;

  FOR_EACH (it, children)
    delete (*it);
  children.clear();

  models_by_name.clear();
  models_by_wfentity.clear();

  ray_list.clear();

  // todo - clean up regions & superregions?

  token = "[unloaded]";
}

bool World::PastQuitTime()
{
  return ((quit_time > 0) && (sim_time >= quit_time));
}

std::string World::ClockString() const
{
  const uint32_t usec_per_hour(3600000000U);
  const uint32_t usec_per_minute(60000000U);
  const uint32_t usec_per_second(1000000U);
  const uint32_t usec_per_msec(1000U);

  const uint32_t hours(sim_time / usec_per_hour);
  const uint32_t minutes((sim_time % usec_per_hour) / usec_per_minute);
  const uint32_t seconds((sim_time % usec_per_minute) / usec_per_second);
  const uint32_t msec((sim_time % usec_per_second) / usec_per_msec);

  std::string str;
  char buf[256];

  if (hours > 0) {
    snprintf(buf, 255, "%uh", hours);
    str += buf;
  }

  snprintf(buf, 255, " %um %02us %03umsec", minutes, seconds, msec);
  str += buf;

  return str;
}

void World::AddUpdateCallback(world_callback_t cb, void *user)
{
  // add the callback & argument to the list
  cb_list.push_back(std::pair<world_callback_t, void *>(cb, user));
}

int World::RemoveUpdateCallback(world_callback_t cb, void *user)
{
  std::pair<world_callback_t, void *> p(cb, user);

  FOR_EACH (it, cb_list) {
    if ((*it) == p) {
      cb_list.erase(it);
      break;
    }
  }

  // return the number of callbacks now in the list. Useful for
  // detecting when the list is empty.
  return cb_list.size();
}

void World::CallUpdateCallbacks()
{
  // call model CB_UPDATE callbacks queued up by worker threads
  size_t threads(pending_update_callbacks.size());
  int cbcount(0);

  for (size_t t(0); t < threads; ++t) {
    std::queue<Model *> &q(pending_update_callbacks[t]);

    // 			printf( "pending callbacks for thread %u: %u\n",
    // 							(unsigned int)t,
    // 							(unsigned int)q.size()
    // );

    cbcount += q.size();

    while (!q.empty()) {
      q.front()->CallUpdateCallbacks();
      q.pop();
    }
  }
  //	printf( "cb total %u (global %d)\n\n", (unsigned
  // int)cbcount,update_cb_count );

  assert(update_cb_count >= cbcount);

  // world callbacks
  FOR_EACH (it, cb_list) {
    if (((*it).first)(this, (*it).second))
      it = cb_list.erase(it);
  }
}

void World::ConsumeQueue(unsigned int queue_num)
{
  std::priority_queue<Event> &queue(event_queues[queue_num]);

  if (queue.empty())
    return;

  // printf( "event queue len %d\n", (int)queue.size() );

  // update everything on the event queue that happens at this time or earlier
  do {
    Event ev(queue.top());
    if (ev.time > sim_time)
      break;
    queue.pop();

    // printf( "Q%d @ %llu next event ptr %p cb %p\n", queue_num, sim_time,
    // ev.mod, ev.cb );
    // std::string modelType = ev.mod->GetModelType();
    // printf( "@ %llu next event <%s %llu %s>\n",  sim_time, modelType.c_str(),
    // ev.time, ev.mod->Token() );

    ev.cb(ev.mod, ev.arg); // call the event's callback on the model
  } while (!queue.empty());
}

bool World::Update()
{
  // printf( "cells: %u blocks %u\n", Cell::count, Block::count );

  // puts( "World::Update()" );

  // if we've run long enough, exit
  if (PastQuitTime() || World::quit_all || this->quit)
    return true;

  if (show_clock && ((this->updates % show_clock_interval) == 0)) {
    printf("\r[Stage: %s]", ClockString().c_str());
    fflush(stdout);
  }

  sim_time += sim_interval;

  // rebuild the sets sorted by position on x,y axis
  models_with_fiducials_byx.clear();
  models_with_fiducials_byy.clear();

  FOR_EACH (it, models_with_fiducials) {
    models_with_fiducials_byx.insert(*it);
    models_with_fiducials_byy.insert(*it);
  }

  // printf( "x %lu y %lu\n", models_with_fiducials_byy.size(),
  //			models_with_fiducials_byx.size() );

  // handle the zeroth queue synchronously in the main thread
  ConsumeQueue(0);

  // handle all the remaining queues asynchronously in worker threads
  pthread_mutex_lock(&sync_mutex);
  threads_working = worker_threads;
  // unblock the workers - they are waiting on this condition var
  // puts( "main thread signalling workers" );
  pthread_cond_broadcast(&threads_start_cond);
  pthread_mutex_unlock(&sync_mutex);

  // update the position of all position models based on their velocity
  // while sensor models are running in other threads
  FOR_EACH (it, active_velocity)
    (*it)->Move();

  pthread_mutex_lock(&sync_mutex);
  // wait for all the last update job to complete - it will
  // signal the worker_threads_done condition var
  while (threads_working > 0) {
    // puts( "main thread waiting for workers to finish" );
    pthread_cond_wait(&threads_done_cond, &sync_mutex);
  }
  pthread_mutex_unlock(&sync_mutex);
  // puts( "main thread awakes" );

  // TODO: allow threadsafe callbacks to be called in worker
  // threads

  dirty = true; // need redraw

  // this stuff must be done in series here

  // world callbacks
  CallUpdateCallbacks();

  FOR_EACH (it, active_energy)
    (*it)->UpdateCharge();

  ++updates;

  return false;
}

unsigned int World::GetEventQueue(Model *) const
{
  // todo: there should be a policy that works faster than random, but
  // random should do a good core load balancing.

  if (worker_threads < 1)
    return 0;
  return ((random() % worker_threads) + 1);
}

Model *World::GetModel(const std::string &name) const
{
  PRINT_DEBUG1("looking up model name %s in models_by_name", name.c_str());

  std::map<std::string, Model *>::const_iterator it(models_by_name.find(name));

  if (it == models_by_name.end()) {
    PRINT_WARN1("lookup of model name %s: no matching name", name.c_str());
    return NULL;
  } else
    return it->second; // the Model*
}

void World::RecordRay(double x1, double y1, double x2, double y2)
{
  float *drawpts(new float[4]);
  drawpts[0] = x1;
  drawpts[1] = y1;
  drawpts[2] = x2;
  drawpts[3] = y2;
  ray_list.push_back(drawpts);
}

void World::ClearRays()
{
  // destroy the C arrays first
  FOR_EACH (it, ray_list)
    delete[] * it;

  ray_list.clear();
}

// Perform multiple raytraces evenly spaced over an angular field of view
void World::Raytrace(const Pose &gpose, // global pose
                     const meters_t range,
		     const radians_t fov,
		     const ray_test_func_t func,
                     const Model *mod,
		     const void *arg,
		     const bool ztest,
                     std::vector<RaytraceResult> &results)
{
  // find the direction of the first ray
  Pose raypose(gpose);
  const double starta(fov / 2.0 - raypose.a);

  // set up a ray to trace
  Ray ray(mod, gpose, range, func, arg, ztest);

  const size_t sample_count = results.size();

  for (size_t s(0); s < sample_count; ++s) {
    // aim the ray in the right direction before tracing
    ray.origin.a = (s * fov / (double)(sample_count - 1)) - starta;
    results[s] = Raytrace(ray);
  }
}

RaytraceResult World::Raytrace(const Pose &gpose,
			       const meters_t range,
			       const ray_test_func_t func,
                               const Model *mod,
			       const void *arg,
			       const bool ztest)
{
  return Raytrace(Ray(mod, gpose, range, func, arg, ztest));
}

RaytraceResult World::Raytrace(const Ray &r)
{
  // rt_cells.clear();
  // rt_candidate_cells.clear();

  // initialize result for return
  RaytraceResult result(r.origin, NULL, Color(), r.range);

  // our global position in (floating point) cell coordinates
  double globx(r.origin.x * ppm);
  double globy(r.origin.y * ppm);

  // record our starting position
  const double startx(globx);
  const double starty(globy);

  // eliminate a potential divide by zero
  const double angle(r.origin.a == 0.0 ? 1e-12 : r.origin.a);
  const double sina(sin(angle));
  const double cosa(cos(angle));
  const double tana(sina / cosa); // approximately tan(angle) but faster

  // the x and y components of the ray (these need to be doubles, or a
  // very weird and rare bug is produced)
  const double dx(ppm * r.range * cosa);
  const double dy(ppm * r.range * sina);

  // fast integer line 3d algorithm adapted from Cohen's code from
  // Graphics Gems IV
  const int32_t sx(sgn(dx));
  const int32_t sy(sgn(dy));
  const int32_t ax(std::abs(dx));
  const int32_t ay(std::abs(dy));
  const int32_t bx(2 * ax);
  const int32_t by(2 * ay);
  int32_t exy(ay - ax); // difference between x and y distances
  int32_t n(ax + ay); // the manhattan distance to the goal cell

  // the distances between region crossings in X and Y
  const double xjumpx(sx * REGIONWIDTH);
  const double xjumpy(sx * REGIONWIDTH * tana);
  const double yjumpx(sy * REGIONWIDTH / tana);
  const double yjumpy(sy * REGIONWIDTH);

  // manhattan distance between region crossings in X and Y
  const double xjumpdist(fabs(xjumpx) + fabs(xjumpy));
  const double yjumpdist(fabs(yjumpx) + fabs(yjumpy));

  const unsigned int layer((updates + 1) % 2);

  // these are updated as we go along the ray
  double xcrossx(0), xcrossy(0);
  double ycrossx(0), ycrossy(0);
  double distX(0), distY(0);
  bool calculatecrossings(true);

  // Stage spends up to 95% of its time in this loop! It would be
  // neater with more function calls encapsulating things, but even
  // inline calls have a noticeable (2-3%) effect on performance.

  // several useful asserts are commented out so that Stage is not too
  // slow in debug builds. Add them in if chasing a suspected raytrace bug
  while (n > 0) // while we are still not at the ray end
  {
    SuperRegion *sr(GetSuperRegion(point_int_t(GETSREG(globx), GETSREG(globy))));
    Region *reg(sr ? sr->GetRegion(GETREG(globx), GETREG(globy)) : NULL);

    if (reg && reg->count) // if the region contains any objects
    {
      // assert( reg->cells.size() );

      // invalidate the region crossing points used to jump over
      // empty regions
      calculatecrossings = true;

      // convert from global cell to local cell coords
      int32_t cx(GETCELL(globx));
      int32_t cy(GETCELL(globy));

      // since reg->count was non-zero, we expect this pointer to be good
      Cell *c(&reg->cells[cx + cy * REGIONWIDTH]);

      // while within the bounds of this region and while some ray remains
      // we'll tweak the cell pointer directly to move around quickly
      while ((cx >= 0) && (cx < REGIONWIDTH) && (cy >= 0) && (cy < REGIONWIDTH) && n > 0) {
        FOR_EACH (it, c->blocks[layer]) {
          Block *block(*it);
          assert(block);

          // skip if not in the right z range
          if (r.ztest && (r.origin.z < block->global_z.min || r.origin.z > block->global_z.max))
            continue;

          // test the predicate we were passed
          if ((*r.func)(&block->group->mod, r.mod, r.arg)) {
            // a hit!
            result.pose = r.origin;
            result.mod = &block->group->mod;
            result.color = result.mod->GetColor();

            if (ax > ay) // faster than the equivalent hypot() call
              result.range = fabs((globx - startx) / cosa) / ppm;
            else
              result.range = fabs((globy - starty) / sina) / ppm;

            return result;
          }
        }

        // increment our cell in the correct direction
        if (exy < 0) // we're iterating along X
        {
          globx += sx; // global coordinate
          exy += by;
          c += sx; // move the cell left or right
          cx += sx; // cell coordinate for bounds checking
        } else // we're iterating along Y
        {
          globy += sy; // global coordinate
          exy -= bx;
          c += sy * REGIONWIDTH; // move the cell up or down
          cy += sy; // cell coordinate for bounds checking
        }
        --n; // decrement the manhattan distance remaining

        // rt_cells.push_back( point_int_t( globx, globy ));
      }
      // printf( "leaving populated region\n" );
    } else // jump over the empty region
    {
      // on the first run, and when we've been iterating over
      // cells, we need to calculate the next crossing of a region
      // boundary along each axis
      if (calculatecrossings) {
        calculatecrossings = false;

        // find the coordinate in cells of the bottom left corner of
        // the current region
        const int32_t ix(globx);
        const int32_t iy(globy);
        double regionx(ix / REGIONWIDTH * REGIONWIDTH);
        double regiony(iy / REGIONWIDTH * REGIONWIDTH);
        if ((globx < 0) && (ix % REGIONWIDTH))
          regionx -= REGIONWIDTH;
        if ((globy < 0) && (iy % REGIONWIDTH))
          regiony -= REGIONWIDTH;

        // calculate the distance to the edge of the current region
        const double xdx(sx < 0 ? regionx - globx - 1.0 : // going left
                             regionx + REGIONWIDTH - globx); // going right
        const double xdy(xdx * tana);

        const double ydy(sy < 0 ? regiony - globy - 1.0 : // going down
                             regiony + REGIONWIDTH - globy); // going up
        const double ydx(ydy / tana);

        // these stored hit points are updated as we go along
        xcrossx = globx + xdx;
        xcrossy = globy + xdy;

        ycrossx = globx + ydx;
        ycrossy = globy + ydy;

        // find the distances to the region crossing points
        // manhattan distance is faster than using hypot()
        distX = fabs(xdx) + fabs(xdy);
        distY = fabs(ydx) + fabs(ydy);
      }

      if (distX < distY) // crossing a region boundary left or right
      {
        // move to the X crossing
        globx = xcrossx;
        globy = xcrossy;

        n -= distX; // decrement remaining manhattan distance

        // calculate the next region crossing
        xcrossx += xjumpx;
        xcrossy += xjumpy;

        distY -= distX;
        distX = xjumpdist;

        // rt_candidate_cells.push_back( point_int_t( xcrossx, xcrossy ));
      } else // crossing a region boundary up or down
      {
        // move to the X crossing
        globx = ycrossx;
        globy = ycrossy;

        n -= distY; // decrement remaining manhattan distance

        // calculate the next region crossing
        ycrossx += yjumpx;
        ycrossy += yjumpy;

        distX -= distY;
        distY = yjumpdist;

        // rt_candidate_cells.push_back( point_int_t( ycrossx, ycrossy ));
      }
    }
    // rt_cells.push_back( point_int_t( globx, globy ));
  }

  return result;
}

static int _save_cb(Model *mod, void *)
{
  mod->Save();
  return 0;
}

bool World::Save(const char *filename)
{
  ForEachDescendant(_save_cb, NULL);
  return this->wf->Save(filename ? filename : wf->filename);
}

static int _reload_cb(Model *mod, void *)
{
  mod->Load();
  return 0;
}

// reload the current worldfile
void World::Reload(void)
{
  ForEachDescendant(_reload_cb, NULL);
}

// add a block to each cell described by a polygon in world coordinates
void World::MapPoly(const std::vector<point_int_t> &pts, Block *block, unsigned int layer)
{
  const size_t pt_count(pts.size());

  for (size_t i(0); i < pt_count; ++i) {
    const point_int_t &start(pts[i]);
    const point_int_t &end(pts[(i + 1) % pt_count]);

    // line rasterization adapted from Cohen's 3D version in
    // Graphics Gems II. Should be very fast.
    const int32_t dx(end.x - start.x);
    const int32_t dy(end.y - start.y);
    const int32_t sx(sgn(dx));
    const int32_t sy(sgn(dy));
    const int32_t ax(std::abs(dx));
    const int32_t ay(std::abs(dy));
    const int32_t bx(2 * ax);
    const int32_t by(2 * ay);

    int32_t exy(ay - ax);
    int32_t n(ax + ay);

    int32_t globx(start.x);
    int32_t globy(start.y);

    while (n) {
      Region *reg(GetSuperRegionCreate(point_int_t(GETSREG(globx), GETSREG(globy)))
                      ->GetRegion(GETREG(globx), GETREG(globy)));
      assert(reg);

      // add all the required cells in this region before looking up
      // another region
      int32_t cx(GETCELL(globx));
      int32_t cy(GETCELL(globy));

      // need to call Region::GetCell() before using a Cell pointer
      // directly, because the region allocates cells lazily, waiting
      // for a call of this method
      Cell *c(reg->GetCell(cx, cy));

      // while inside the region, manipulate the Cell pointer directly
      while ((cx >= 0) && (cx < REGIONWIDTH) && (cy >= 0) && (cy < REGIONWIDTH) && n > 0) {
	assert( c != NULL );
	
        // if the block is not already rendered in the cell
        // if( find (block->rendered_cells[layer].begin(),
        // block->rendered_cells[layer].end(), c )
        // == block->rendered_cells[layer].end() )
        c->AddBlock(block, layer);

        // compute the next cell index inside the region 
        if (exy < 0) {
          globx += sx;
          exy += by;
          c += sx;
          cx += sx;
        } else {
          globy += sy;
          exy -= bx;
          c += sy * REGIONWIDTH;
          cy += sy;
        }
        --n;
      }
    }
  }
}

SuperRegion *World::AddSuperRegion(const point_int_t &sup)
{
  SuperRegion *sr(CreateSuperRegion(sup));

  // set the lower left corner of the new superregion
  Extend(point3_t((sup.x << SRBITS) / ppm, (sup.y << SRBITS) / ppm, 0));

  // top right corner of the new superregion
  Extend(point3_t(((sup.x + 1) << SRBITS) / ppm, ((sup.y + 1) << SRBITS) / ppm, 0));
  return sr;
}

inline SuperRegion *World::GetSuperRegion(const point_int_t &org)
{
  SuperRegion *sr(NULL);

  // I despise some STL syntax sometimes...
  std::map<point_int_t, SuperRegion *>::iterator it(superregions.find(org));

  if (it != superregions.end())
    sr = it->second;

  return sr;
}

inline SuperRegion *World::GetSuperRegionCreate(const point_int_t &org)
{
  SuperRegion *sr(GetSuperRegion(org));

  if (sr == NULL) // no superregion exists! make a new one
  {
    sr = AddSuperRegion(org);
    assert(sr);
  }
  return sr;
}

void World::Extend(point3_t pt)
{
  extent.x.min = std::min(extent.x.min, pt.x);
  extent.x.max = std::max(extent.x.max, pt.x);
  extent.y.min = std::min(extent.y.min, pt.y);
  extent.y.max = std::max(extent.y.max, pt.y);
  extent.z.min = std::min(extent.z.min, pt.z);
  extent.z.max = std::max(extent.z.max, pt.z);
}

void World::AddPowerPack(PowerPack *pp)
{
  powerpack_list.push_back(pp);
}

void World::RemovePowerPack(PowerPack *pp)
{
  EraseAll(pp, powerpack_list);
}

/// Register an Option for pickup by the GUI
void World::RegisterOption(Option *opt)
{  
  assert(opt);
  option_table.insert(opt);
}

void World::Log(Model *)
{
  // LogEntry( sim_time, mod);
  // printf( "log entry count %u\n", (unsigned int)LogEntry::Count() );
  // LogEntry::Print();
}

bool World::Event::operator<(const Event &other) const
{
  return (time > other.time);
}

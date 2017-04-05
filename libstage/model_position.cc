///////////////////////////////////////////////////////////////////////////
//
// File: model_position.cc
// Author: Richard Vaughan
// Date: 10 June 2004
//
// SVN info:
//  $Author: rtv $
//  $Revision$
//
///////////////////////////////////////////////////////////////////////////

#include <sys/time.h>

#include "stage.hh"
#include "worldfile.hh"
using namespace Stg;

/**
    @ingroup model
    @defgroup model_position Position model

    The position model simulates a
    mobile robot base. It can drive in one of two modes; either
    <i>differential</i>, i.e. able to control its speed and turn rate by
    driving left and roght wheels like a Pioneer robot, or
    <i>omnidirectional</i>, i.e. able to control each of its three axes
    independently.

    API: Stg::ModelPosition

    <h2>Worldfile properties</h2>

    @par Summary and default values

    @verbatim
    position
    (
    # position properties
    drive "diff"

    velocity [ 0.0 0.0 0.0 0.0 ]

    localization "gps"
    localization_origin [ <defaults to model's start pose> ]

    # odometry error model parameters,
    # only used if localization is set to "odom"
    odom_error [0.03 0.03 0.00 0.05]

    # only used if drive is set to "car"
    wheelbase 1.0

    # [ xmin xmax ymin ymax zmin zmax amin amax ]
    velocity_bounds [-1 1 -1 1 -1 1 -90 90 ]
    acceleration_bounds [-1 1 -1 1 -1 1 -90 90]

    # model properties

    )
    @endverbatim

    @par Note
    Since Stage-1.6.5 the odom property has been removed. Stage will generate a
warning if odom is defined in your worldfile. See localization_origin instead.

    @par Details

  - acceleration_bounds [ xmin xmax ymin ymax zmin zmax amin amax ] x,y,z in
meters per second squared, a in degrees per second squared
- drive "diff", "omni" or "car"\n
    select differential-steer model(like a Pioneer), omnidirectional mode or
carlike (velocity and steering angle).
    omega:<float> ]\n Specify the initial velocity of the model. Note
    that if the model hits an obstacle, its velocity will be set to
    zero.
    - localization "gps" or "odom"\n
    if "gps" the position model reports its position with perfect accuracy. If
"odom", a simple odometry model is used and position data drifts from the ground
truth over time. The odometry model is parameterized by the odom_error property.
    - localization_origin [x y z theta]
    - set the origin of the localization coordinate system. By default, this is
copied from the model's initial pose, so the robot reports its position relative
to the place it started out. Tip: If localization_origin is set to [0 0 0 0] and
localization is "gps", the model will return its true global position. This is
unrealistic, but useful if you want to abstract away the details of
localization. Be prepared to justify the use of this mode in your research!
    - odom_error [x y z theta]
    - parameters for the odometry error model used when specifying localization
"odom". Each value is the maximum proportion of error in intergrating x, y, and
theta velocities to compute odometric position estimate. For each axis, if the
the value specified here is E, the actual proportion is chosen at startup at
random in the range -E/2 to +E/2. Note that due to rounding errors, setting
these values to zero does NOT give you perfect localization - for that you need
to choose localization "gps".
    - velocity [ x:<float> y:<float> z:<float> heading:<float>
    - velocity_bounds [ xmin xmax ymin ymax zmin zmax amin amax ] x,y,z in
meters per second, a in degrees per second
    - wheelbase <float,meters>
    The wheelbase used for the car steering model. Only used if drive is set to
"car". Defaults to 1.0m\*/

static const double WATTS_KGMS = 10.0; // current per kg per meter per second
static const double WATTS = 1.0; // base cost of position device

// simple odometry error model parameters. the error is selected at
// random in the interval -MAX/2 to +MAX/2 at startup
static const double INTEGRATION_ERROR_MAX_X = 0.03;
static const double INTEGRATION_ERROR_MAX_Y = 0.03;
static const double INTEGRATION_ERROR_MAX_Z = 0.00; // note zero!
static const double INTEGRATION_ERROR_MAX_A = 0.05;

ModelPosition::ModelPosition(World *world, Model *parent, const std::string &type)
    : Model(world, parent, type),
      // private
      velocity(), goal(0, 0, 0, 0), control_mode(CONTROL_VELOCITY), drive_mode(DRIVE_DIFFERENTIAL),
      localization_mode(LOCALIZATION_GPS),
      integration_error(drand48() * INTEGRATION_ERROR_MAX_X - INTEGRATION_ERROR_MAX_X / 2.0,
                        drand48() * INTEGRATION_ERROR_MAX_Y - INTEGRATION_ERROR_MAX_Y / 2.0,
                        drand48() * INTEGRATION_ERROR_MAX_Z - INTEGRATION_ERROR_MAX_Z / 2.0,
                        drand48() * INTEGRATION_ERROR_MAX_A - INTEGRATION_ERROR_MAX_A / 2.0),
      wheelbase(1.0), acceleration_bounds(), velocity_bounds(),
      // public
      waypoints(), wpvis(), posevis()
{
  PRINT_DEBUG2("Constructing ModelPosition %u (%s)\n", id, type.c_str());

  // assert that Update() is reentrant for this derived model
  thread_safe = false;

  // install sensible velocity and acceleration bounds
  for (int i = 0; i < 3; i++) {
    velocity_bounds[i].min = -1.0;
    velocity_bounds[i].max = 1.0;

    acceleration_bounds[i].min = -1.0;
    acceleration_bounds[i].max = 1.0;
  }

  velocity_bounds[3].min = -M_PI / 2.0;
  velocity_bounds[3].max = M_PI / 2.0;

  acceleration_bounds[3].min = -M_PI / 2.0;
  acceleration_bounds[3].max = M_PI / 2.0;

  this->SetBlobReturn(true);

  AddVisualizer(&wpvis, true);
  AddVisualizer(&posevis, false);
}

ModelPosition::~ModelPosition(void)
{
  // nothing to do
}

void ModelPosition::SetVelocity(const Velocity &val)
{
  velocity = val;
  CallCallbacks(CB_VELOCITY);
}

// get the model's velocity in the global frame
Velocity ModelPosition::GetGlobalVelocity() const
{
  Pose gpose = GetGlobalPose();

  double cosa = cos(gpose.a);
  double sina = sin(gpose.a);

  Velocity gv;
  gv.x = velocity.x * cosa - velocity.y * sina;
  gv.y = velocity.x * sina + velocity.y * cosa;
  gv.a = velocity.a;

  return gv;
}

// set the model's velocity in the global frame
void ModelPosition::SetGlobalVelocity(const Velocity &gv)
{
  Pose gpose = GetGlobalPose();

  double cosa = cos(gpose.a);
  double sina = sin(gpose.a);

  Velocity lv;
  lv.x = gv.x * cosa + gv.y * sina;
  lv.y = -gv.x * sina + gv.y * cosa;
  lv.a = gv.a;

  this->SetVelocity(lv);
}

void ModelPosition::Load(void)
{
  Model::Load();

  if (wf->PropertyExists(wf_entity, "velocity"))
    SetVelocity(GetVelocity().Load(wf, wf_entity, "velocity"));

  // load steering mode
  if (wf->PropertyExists(wf_entity, "drive")) {
    const std::string &mode_str = wf->ReadString(wf_entity, "drive", "diff");

    if (mode_str == "diff")
      drive_mode = DRIVE_DIFFERENTIAL;
    else if (mode_str == "omni")
      drive_mode = DRIVE_OMNI;
    else if (mode_str == "car")
      drive_mode = DRIVE_CAR;
    else
      PRINT_ERR1("invalid position drive mode specified: \"%s\" - should be "
                 "one of: \"diff\", \"omni\" or \"car\". Using \"diff\" as "
                 "default.",
                 mode_str.c_str());
  }

  // choose a wheelbase
  this->wheelbase = wf->ReadLength(wf_entity, "wheelbase", this->wheelbase);

  // load odometry if specified
  if (wf->PropertyExists(wf_entity, "odom")) {
    PRINT_WARN1("the odom property is specified for model \"%s\","
                " but this property is no longer available."
                " Use localization_origin instead. See the position"
                " entry in the manual or src/model_position.c for details.",
                this->Token());
  }

  // set the starting pose as my initial odom position. This could be
  // overwritten below if the localization_origin property is
  // specified
  est_origin = this->GetGlobalPose();
  est_origin.Load(wf, wf_entity, "localization_origin");

  // compute our localization pose based on the origin and true pose
  const Pose gpose = this->GetGlobalPose();

  est_pose.a = normalize(gpose.a - est_origin.a);
  const double cosa = cos(est_origin.a);
  const double sina = sin(est_origin.a);
  const double dx = gpose.x - est_origin.x;
  const double dy = gpose.y - est_origin.y;
  est_pose.x = dx * cosa + dy * sina;
  est_pose.y = dy * cosa - dx * sina;

  // zero position error: assume we know exactly where we are on startup
  est_pose_error.Zero(); // memset( &est_pose_error, 0, sizeof(est_pose_error));

  // odometry model parameters
  integration_error.Load(wf, wf_entity, "odom_error");

  // choose a localization model
  if (wf->PropertyExists(wf_entity, "localization")) {
    const std::string &loc_str = wf->ReadString(wf_entity, "localization", "gps");

    if (loc_str == "gps")
      localization_mode = LOCALIZATION_GPS;
    else if (loc_str == "odom")
      localization_mode = LOCALIZATION_ODOM;
    else
      PRINT_ERR2("unrecognized localization mode \"%s\" for model \"%s\"."
                 " Valid choices are \"gps\" and \"odom\".",
                 loc_str.c_str(), this->Token());
  }

  // if the property does not exist, these have no effect on the argument list

  wf->ReadTuple(wf_entity, "acceleration_bounds", 0, 8, "llllllaa", &acceleration_bounds[0].min,
                &acceleration_bounds[0].max, &acceleration_bounds[1].min,
                &acceleration_bounds[1].max, &acceleration_bounds[2].min,
                &acceleration_bounds[2].max, &acceleration_bounds[3].min,
                &acceleration_bounds[3].max);

  wf->ReadTuple(wf_entity, "velocity_bounds", 0, 8, "llllllaa", &velocity_bounds[0].min,
                &velocity_bounds[0].max, &velocity_bounds[1].min, &velocity_bounds[1].max,
                &velocity_bounds[2].min, &velocity_bounds[2].max, &velocity_bounds[3].min,
                &velocity_bounds[3].max);
}


void ModelPosition::Update(void)
{
  PRINT_DEBUG1("[%lu] position update", this->world->SimTimeNow());

  // stop by default
  Velocity vel(0, 0, 0, 0);

  if (this->subs) // no driving if noone is subscribed
  {
    switch (control_mode) {
    case CONTROL_ACCELERATION: {
      // respect the accel bounds;
      goal.x = std::min(goal.x, acceleration_bounds[0].max);
      goal.x = std::max(goal.x, acceleration_bounds[0].min);

      goal.y = std::min(goal.y, acceleration_bounds[1].max);
      goal.y = std::max(goal.y, acceleration_bounds[1].min);

      goal.z = std::min(goal.z, acceleration_bounds[2].max);
      goal.z = std::max(goal.z, acceleration_bounds[2].min);

      goal.a = std::min(goal.a, acceleration_bounds[3].max);
      goal.a = std::max(goal.a, acceleration_bounds[3].min);

      vel = this->velocity; // we're modifying the current velocity

      // convert usec to sec
      const double interval((double)world->sim_interval / 1e6);

      PRINT_DEBUG("acceleration control mode");
      PRINT_DEBUG4("model %s command(%.2f %.2f %.2f)", this->Token(), this->goal.x, this->goal.y,
                   // this->goal.z,
                   this->goal.a);

      switch (drive_mode) {
      case DRIVE_DIFFERENTIAL:
        // differential-steering model, like a Pioneer
        vel.x += goal.x * interval;
        vel.y = 0;
        vel.a += goal.a * interval;
        break;

      case DRIVE_OMNI:
        // direct steering model, like an omnidirectional robot
        vel.x += goal.x * interval;
        vel.y += goal.y * interval;
        vel.a += goal.a * interval;
        break;

      case DRIVE_CAR:
        PRINT_ERR("car drive not supported in accelerartion control [to do]");
        // // car like steering model based on speed and turning angle
        // vel.x = goal.x * cos(goal.a);
        // vel.y = 0;
        // vel.a = goal.x * sin(goal.a)/wheelbase;
        break;

      default: PRINT_ERR1("unknown steering mode %d", drive_mode);
      }

      // printf( "interval %.3f vel: %.2f %.2f %.2f\taccel: %.2f %.2f %.2f\n",
      // 	    interval,
      // 	    vel.x, vel.y, vel.a,
      // 	    goal.x, goal.y, goal.a );

    } break;

    case CONTROL_VELOCITY: {
      PRINT_DEBUG("velocity control mode");
      PRINT_DEBUG4("model %s command(%.2f %.2f %.2f)", this->Token(), this->goal.x, this->goal.y,
                   this->goal.a);

      switch (drive_mode) {
      case DRIVE_DIFFERENTIAL:
        // differential-steering model, like a Pioneer
        vel.x = goal.x;
        vel.y = 0;
        vel.a = goal.a;
        break;

      case DRIVE_OMNI:
        // direct steering model, like an omnidirectional robot
        vel.x = goal.x;
        vel.y = goal.y;
        vel.a = goal.a;
        break;

      case DRIVE_CAR:
        // car like steering model based on speed and turning angle
        vel.x = goal.x * cos(goal.a);
        vel.y = 0;
        vel.a = goal.x * sin(goal.a) / wheelbase;
        break;

      default: PRINT_ERR1("unknown steering mode %d", drive_mode);
      }
    } break;

    case CONTROL_POSITION: {
      PRINT_DEBUG("position control mode");

      const double x_error = goal.x - est_pose.x;
      const double y_error = goal.y - est_pose.y;
      double a_error = normalize(goal.a - est_pose.a);

      PRINT_DEBUG3("errors: %.2f %.2f %.2f\n", x_error, y_error, a_error);

      // speed limits for controllers
      // TODO - have these configurable
      const double max_speed_x = 0.4;
      const double max_speed_y = 0.4;
      const double max_speed_a = 1.0;

      switch (drive_mode) {
      case DRIVE_OMNI: {
        // this is easy - we just reduce the errors in each axis
        // independently with a proportional controller, speed
        // limited
        vel.x = std::min(x_error, max_speed_x);
        vel.y = std::min(y_error, max_speed_y);
        vel.a = std::min(a_error, max_speed_a);
      } break;

      case DRIVE_DIFFERENTIAL: {
        // axes can not be controlled independently. We have to
        // turn towards the desired x,y position, drive there,
        // then turn to face the desired angle.  this is a
        // simple controller that works ok. Could easily be
        // improved if anyone needs it better. Who really does
        // position control anyhoo?

        // start out with no velocity
        Velocity calc;
        double close_enough = 0.02; // fudge factor

        // if we're at the right spot
        if (fabs(x_error) < close_enough && fabs(y_error) < close_enough) {
          PRINT_DEBUG("TURNING ON THE SPOT");
          // turn on the spot to minimize the error
          calc.a = std::min(a_error, max_speed_a);
          calc.a = std::max(a_error, -max_speed_a);
        } else {
          PRINT_DEBUG("TURNING TO FACE THE GOAL POINT");
          // turn to face the goal point
          double goal_angle = atan2(y_error, x_error);
          double goal_distance = hypot(y_error, x_error);

          a_error = normalize(goal_angle - est_pose.a);
          calc.a = std::min(a_error, max_speed_a);
          calc.a = std::max(a_error, -max_speed_a);

          PRINT_DEBUG2("steer errors: %.2f %.2f \n", a_error, goal_distance);

          // if we're pointing about the right direction, move
          // forward
          if (fabs(a_error) < M_PI / 16) {
            PRINT_DEBUG("DRIVING TOWARDS THE GOAL");
            calc.x = std::min(goal_distance, max_speed_x);
          }
        }

        // now set the underlying velocities using the normal
        // diff-steer model
        vel.x = calc.x;
        vel.y = 0;
        vel.a = calc.a;
      } break;

      default: PRINT_ERR1("unknown steering mode %d", (int)drive_mode);
      }
    } break;

    default: PRINT_ERR1("unrecognized position command mode %d", control_mode);
    }

    // simple model of power consumption
    watts = WATTS + fabs(vel.x) * WATTS_KGMS * mass + fabs(vel.y) * WATTS_KGMS * mass
            + fabs(vel.a) * WATTS_KGMS * mass;

    // PRINT_DEBUG4( "model %s velocity (%.2f %.2f %.2f)",
    //	    this->token,
    //	    this->velocity.x,
    //	    this->velocity.y,
    //	    this->velocity.a );

    // respect velocity bounds
    vel.x = velocity_bounds[0].Constrain(vel.x);
    vel.y = velocity_bounds[1].Constrain(vel.y);
    vel.z = velocity_bounds[2].Constrain(vel.z);
    vel.a = velocity_bounds[3].Constrain(vel.a);

    // printf( "final vel: %.2f %.2f %.2f\n",
    // vel.x, vel.y, vel.a );

    this->SetVelocity(vel);
  }

  switch (localization_mode) {
  case LOCALIZATION_GPS: {
    est_pose = this->GetGlobalPose();

    /*
    // report our compute our localization pose based on the
    // origin and true pose Issue #36 reports that this gives
    // inaccurate poses. I can't see what's wrong so I'm disabling
    // the local origin transform
    const Pose gpose = this->GetGlobalPose();

    est_pose.a = normalize( gpose.a - est_origin.a );
    const double cosa = cos(est_origin.a);
    const double sina = sin(est_origin.a);
    const double dx = gpose.x - est_origin.x;
    const double dy = gpose.y - est_origin.y;
    est_pose.x = dx * cosa + dy * sina;
    est_pose.y = dy * cosa - dx * sina;
    */
  } break;

  case LOCALIZATION_ODOM: {
    // integrate our velocities to get an 'odometry' position estimate.
    const double dt = world->sim_interval / 1e6; // update interval convert to seconds

    est_pose.a = normalize(est_pose.a + (vel.a * dt) * (1.0 + integration_error.a));

    const double cosa = cos(est_pose.a);
    const double sina = sin(est_pose.a);
    const double dx = (vel.x * dt) * (1.0 + integration_error.x);
    const double dy = (vel.y * dt) * (1.0 + integration_error.y);

    est_pose.x += dx * cosa + dy * sina;
    est_pose.y -= dy * cosa - dx * sina;
  } break;

  default:
    PRINT_ERR2("unknown localization mode %d for model %s\n", localization_mode, Token());
    break;
  }

  PRINT_DEBUG3(" READING POSITION: [ %.4f %.4f %.4f ]\n", est_pose.x, est_pose.y, est_pose.a);

  Model::Update();
}

void ModelPosition::Move(void)
{
  if (velocity.IsZero())
    return;

  if (disabled)
    return;

  // convert usec to sec
  const double interval((double)world->sim_interval / 1e6);

  // find the change of pose due to our velocity vector
  const Pose dp(velocity.x * interval, velocity.y * interval, velocity.z * interval,
                normalize(velocity.a * interval));

  // the pose we're trying to achieve (unless something stops us)
  const Pose newpose(pose + dp);

  // stash the original pose so we can put things back if we hit
  const Pose startpose(pose);

  pose = newpose; // do the move provisionally - we might undo it below

  const unsigned int layer(world->UpdateCount() % 2);
  // @todo th
  UnMapWithChildren(layer); // remove from all blocks
  MapWithChildren(layer); // render into new blocks

  if (TestCollision()) // crunch!
  {
    // put things back the way they were
    // this is expensive, but it happens _very_ rarely for most people
    pose = startpose;
    UnMapWithChildren(layer);
    MapWithChildren(layer);

    SetStall(true);
  } else {
    SetStall(false);
  }
}

void ModelPosition::Startup(void)
{
  world->active_velocity.insert(this);

  Model::Startup();

  PRINT_DEBUG("position startup");
}

void ModelPosition::Shutdown(void)
{
  PRINT_DEBUG("position shutdown");

  // safety features!
  goal.Zero();
  velocity.Zero();

  world->active_velocity.erase(this);

  Model::Shutdown();
}

void ModelPosition::Stop()
{
  SetSpeed(0, 0, 0);
}

void ModelPosition::SetSpeed(double x, double y, double a)
{
  control_mode = CONTROL_VELOCITY;
  goal.x = x;
  goal.y = y;
  goal.z = 0;
  goal.a = a;
}

void ModelPosition::SetXSpeed(double x)
{
  control_mode = CONTROL_VELOCITY;
  goal.x = x;
}

void ModelPosition::SetYSpeed(double y)
{
  control_mode = CONTROL_VELOCITY;
  goal.y = y;
}

void ModelPosition::SetZSpeed(double z)
{
  control_mode = CONTROL_VELOCITY;
  goal.z = z;
}

void ModelPosition::SetTurnSpeed(double a)
{
  control_mode = CONTROL_VELOCITY;
  goal.a = a;
}

void ModelPosition::SetSpeed(Velocity vel)
{
  control_mode = CONTROL_VELOCITY;
  goal.x = vel.x;
  goal.y = vel.y;
  goal.z = vel.z;
  goal.a = vel.a;
}

void ModelPosition::GoTo(double x, double y, double a)
{
  control_mode = CONTROL_POSITION;
  goal.x = x;
  goal.y = y;
  goal.z = 0;
  goal.a = a;
}

void ModelPosition::GoTo(Pose pose)
{
  control_mode = CONTROL_POSITION;
  goal = pose;
}

void ModelPosition::SetAcceleration(double x, double y, double a)
{
  control_mode = CONTROL_ACCELERATION;
  goal.x = x;
  goal.y = y;
  goal.z = 0;
  goal.a = a;
}

/**
    Set the current odometry estimate
*/
void ModelPosition::SetOdom(Pose odom)
{
  est_pose = odom;

  // figure out where the implied origin is in global coords
  const Pose gp = GetGlobalPose();
  const double da = normalize(-odom.a + gp.a);
  const double dx = -odom.x * cos(da) + odom.y * sin(da);
  const double dy = -odom.y * cos(da) - odom.x * sin(da);

  // origin of our estimated pose
  est_origin.x = gp.x + dx;
  est_origin.y = gp.y + dy;
  est_origin.a = da;
}

ModelPosition::PoseVis::PoseVis() : Visualizer("Position coordinates", "show_position_coords")
{
}

void ModelPosition::PoseVis::Visualize(Model *mod, Camera *cam)
{
  (void)cam; // avoid warning about unused var

  ModelPosition *pos = dynamic_cast<ModelPosition *>(mod);

  // vizualize my estimated pose
  glPushMatrix();

  // back into global coords
  Gl::pose_inverse_shift(pos->GetGlobalPose());

  Gl::pose_shift(pos->est_origin);
  pos->PushColor(1, 0, 0, 1); // origin in red
  Gl::draw_origin(0.5);

  glEnable(GL_LINE_STIPPLE);
  glLineStipple(3, 0xAAAA);

  pos->PushColor(1, 0, 0, 0.5);
  glBegin(GL_LINE_STRIP);
  glVertex2f(0, 0);
  glVertex2f(pos->est_pose.x, 0);
  glVertex2f(pos->est_pose.x, pos->est_pose.y);
  glEnd();

  glDisable(GL_LINE_STIPPLE);

  char label[64];
  snprintf(label, 64, "x:%.3f", pos->est_pose.x);
  Gl::draw_string(pos->est_pose.x / 2.0, -0.5, 0, label);

  snprintf(label, 64, "y:%.3f", pos->est_pose.y);
  Gl::draw_string(pos->est_pose.x + 0.5, pos->est_pose.y / 2.0, 0, (const char *)label);

  pos->PopColor();

  Gl::pose_shift(pos->est_pose);
  pos->PushColor(0, 1, 0, 1); // pose in green
  Gl::draw_origin(0.5);
  pos->PopColor();

  Gl::pose_shift(pos->geom.pose);
  pos->PushColor(0, 0, 1, 1); // offset in blue
  Gl::draw_origin(0.5);
  pos->PopColor();

  Color c = pos->color;
  c.a = 0.5;
  pos->PushColor(c);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  pos->blockgroup.DrawFootPrint(pos->geom);
  pos->PopColor();

  glPopMatrix();
}

ModelPosition::WaypointVis::WaypointVis()
    : Visualizer("Position waypoints", "show_position_waypoints")
{
}

void ModelPosition::WaypointVis::Visualize(Model *mod, Camera *cam)
{
  (void)cam; // avoid warning about unused var

  ModelPosition *pos = dynamic_cast<ModelPosition *>(mod);
  const std::vector<Waypoint> &waypoints = pos->waypoints;

  if (waypoints.empty())
    return;

  glPointSize(5);
  glPushMatrix();
  pos->PushColor(pos->color);

  Gl::pose_inverse_shift(pos->pose);
  Gl::pose_shift(pos->est_origin);

  glTranslatef(0, 0, 0.02);

  // draw waypoints
  glLineWidth(3);
  FOR_EACH (it, waypoints)
    it->Draw();
  glLineWidth(1);

  // draw lines connecting the waypoints
  const size_t num(waypoints.size());
  if (num > 1) {
    pos->PushColor(1, 0, 0, 0.3);
    glBegin(GL_LINES);

    for (size_t i(1); i < num; i++) {
      Pose p = waypoints[i].pose;
      Pose o = waypoints[i - 1].pose;

      glVertex2f(p.x, p.y);
      glVertex2f(o.x, o.y);
    }

    glEnd();

    pos->PopColor();
  }

  pos->PopColor();
  glPopMatrix();
}

ModelPosition::Waypoint::Waypoint(const Pose &pose, Color color) : pose(pose), color(color)
{
}

ModelPosition::Waypoint::Waypoint(meters_t x, meters_t y, meters_t z, radians_t a, Color color)
    : pose(x, y, z, a), color(color)
{
}

ModelPosition::Waypoint::Waypoint() : pose(), color()
{
}

void ModelPosition::Waypoint::Draw() const
{
  GLdouble d[4];

  d[0] = color.r;
  d[1] = color.g;
  d[2] = color.b;
  d[3] = color.a;

  glColor4dv(d);

  glBegin(GL_POINTS);
  glVertex3f(pose.x, pose.y, pose.z);
  glEnd();

  meters_t quiver_length = 0.15;

  double dx = cos(pose.a) * quiver_length;
  double dy = sin(pose.a) * quiver_length;

  glBegin(GL_LINES);
  glVertex3f(pose.x, pose.y, pose.z);
  glVertex3f(pose.x + dx, pose.y + dy, pose.z);
  glEnd();
}

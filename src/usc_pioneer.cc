///////////////////////////////////////////////////////////////////////////
//
// File: usc_pioneer.cc
// Author: Andrew Howard
// Date: 5 Mar 2000
// Desc: Class representing a standard USC pioneer robot
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/src/usc_pioneer.cc,v $
//  $Author: vaughan $
//  $Revision: 1.1.2.18 $
//
// Usage:
//  (empty)
//
// Theory of operation:
//  (empty)
//
// Known bugs:
//  (empty)
//
// Possible enhancements:
//  (empty)
//
///////////////////////////////////////////////////////////////////////////

#include "world.hh"
#include "usc_pioneer.hh"
#include "playerserver.hh"
#include "pioneermobiledevice.hh"
#include "miscdevice.hh"
#include "sonardevice.hh"
#include "laserdevice.hh"
#include "laserbeacondevice.hh"
#include "ptzdevice.hh"
#include "visiondevice.hh"


///////////////////////////////////////////////////////////////////////////
// Minimal constructor
//
CUscPioneer::CUscPioneer(CWorld *world, CEntity *parent)
        : CEntity(world, parent)
{
    m_player = new CPlayerServer(world, this);
    m_pioneer = new CPioneerMobileDevice(world, parent, m_player);
    m_misc = new CMiscDevice(world, m_pioneer, m_player);
    m_sonar = new CSonarDevice(world, m_pioneer, m_player);
    m_laser = new CLaserDevice(world, m_pioneer, m_player);
    m_laser_beacon = new CLaserBeaconDevice(world, m_pioneer, m_player, m_laser);
    m_ptz = new CPtzDevice(world, m_pioneer, m_player);
    m_vision = new CVisionDevice(world, m_pioneer, m_player, m_ptz);
    
    // Make the pioneer base the default parent of any would be children
    //
    m_default_object = m_pioneer;

    // Add all these objects to the world
    //
    world->AddObject(m_player);
    world->AddObject(m_pioneer);
    world->AddObject(m_misc);
    world->AddObject(m_sonar);
    world->AddObject(m_laser);
    world->AddObject(m_laser_beacon);
    world->AddObject(m_ptz);
    world->AddObject(m_vision);

    // Set the default pose of some key devices
    //
    m_laser->SetPose(0.09, 0, 0);
    
#ifdef INCLUDE_XGUI
    exp.objectType = uscpioneer_o;
    exp.width = 0.99; //m
    exp.height = 0.99; //m   
    strcpy( exp.label, "USC Pioneer" );
#endif

    m_ptz->SetPose(0.09, 0, 0);
}


///////////////////////////////////////////////////////////////////////////
// Destructor
//
CUscPioneer::~CUscPioneer()
{
}


///////////////////////////////////////////////////////////////////////////
// Load the object from an argument list
//
bool CUscPioneer::Load(int argc, char **argv)
{
    // Note that we dont call the underlying CEntity load;
    // this will load the wrong pose from the file.
    
    for (int i = 0; i < argc;)
    {
        // Extract pose
        // This as a bit of a hack -- we want the pioneer base to get the pose
        //
        if (strcmp(argv[i], "pose") == 0 && i + 3 < argc)
        {
            double px = atof(argv[i + 1]);
            double py = atof(argv[i + 2]);
            double pth = DTOR(atof(argv[i + 3]));
            m_pioneer->SetPose(px, py, pth);
            i += 4;
        }

        // Extract the robot name
        //
        else if (strcmp(argv[i], "name") == 0 && i + 1 < argc)
        {
            strcpy(m_pioneer->m_id, argv[i + 1]);
            i += 2;
        }

        // Extract player server port number
        //
        else if (strcmp(argv[i], "port") == 0 && i + 1 < argc)
        {
            m_player->m_port = atoi(argv[i + 1]);
            i += 2;
        }
        else
            i++;
    }
    return true;
} 


///////////////////////////////////////////////////////////////////////////
// Save the object
//
bool CUscPioneer::Save(int &argc, char **argv)
{
    // Note that we dont call the underlying CEntity save;
    // this will insert the wrong pose in the file

    // Get the pioneer base pose
    //
    double px, py, pa;
    m_pioneer->GetPose(px, py, pa);

    // Convert to strings
    //
    char sx[32];
    snprintf(sx, sizeof(sx), "%.2f", px);
    char sy[32];
    snprintf(sy, sizeof(sy), "%.2f", py);
    char sa[32];
    snprintf(sa, sizeof(sa), "%.0f", RTOD(pa));

    // Save pose
    //
    argv[argc++] = strdup("pose");
    argv[argc++] = strdup(sx);
    argv[argc++] = strdup(sy);
    argv[argc++] = strdup(sa);

    // Save name
    //
    argv[argc++] = strdup("name");
    argv[argc++] = strdup(m_pioneer->m_id);

    // Save port
    //
    char port[32];
    snprintf(port, sizeof(port), "%d", m_player->m_port);
    argv[argc++] = strdup("port");
    argv[argc++] = strdup(port);

    return true;
}


///////////////////////////////////////////////////////////////////////////
// Default startup -- doesnt do much
//
bool CUscPioneer::Startup()
{
    if (!CEntity::Startup())
        return false;
    return true;
}


///////////////////////////////////////////////////////////////////////////
// Default shutdown -- doesnt do much
//
void CUscPioneer::Shutdown()
{
    CEntity::Shutdown();
}


///////////////////////////////////////////////////////////////////////////
// Update robot and all its devices
//
void CUscPioneer::Update()
{
    CEntity::Update();
}


#ifdef INCLUDE_RTK

///////////////////////////////////////////////////////////////////////////
// Process GUI update messages
//
void CUscPioneer::OnUiUpdate(RtkUiDrawData *data)
{
    CEntity::OnUiUpdate(data);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI mouse messages
//
void CUscPioneer::OnUiMouse(RtkUiMouseData *data)
{
    CEntity::OnUiMouse(data);
}


///////////////////////////////////////////////////////////////////////////
// Process GUI property messages
//
void CUscPioneer::OnUiProperty(RtkUiPropertyData *data)
{
    CEntity::OnUiProperty(data);
}

#endif

#ifdef INCLUDE_XGUI

////////////////////////////////////////////////////////////////////////////
// compose and return the export data structure for external rendering
// return null if we're not exporting data right now.
ExportData* CUscPioneer::ImportExportData( ImportData* imp )
{ 

  if( imp ) // if there is some imported data
   SetGlobalPose( imp->x, imp->y, imp->th ); // move to the suggested place

  if( !exporting ) return 0;

  // fill in the exp structure
  GetGlobalPose( exp.x, exp.y, exp.th );

  // fill in the exp structure  
  //exp.width = m_size_x;
  //exp.height = m_size_y;

  return &exp;
}

#endif

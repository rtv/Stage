///////////////////////////////////////////////////////////////////////////
//
// File: object.hh
// Author: Andrew Howard
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/object.hh,v $
//  $Author: ahoward $
//  $Revision: 1.1.2.5 $
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


#ifndef OBJECT_HH
#define OBJECT_HH

#ifdef INCLUDE_RTK
#include "rtk-ui.hh"
#endif

#include "rtk-types.hh"

// We need run-time type info
//
#include <typeinfo>

// Forward declare the world class
//
class CWorld;


class CObject
{
    // Minimal constructor
    // Requires a pointer to the parent and a pointer to the world.
    //
    public: CObject(CWorld *world, CObject *parent);

    // Destructor
    // Will delete children
    //
    public: virtual ~CObject();

    // Startup routine
    //
    public: virtual bool Startup(RtkCfgFile *cfg);

    // Shutdown routine
    //
    public: virtual void Shutdown();
    
    // Update the object's representation
    //
    public: virtual void Update();

    // Creation routine
    // Creates child objects by reading them from a file.
    // Objects are created recursively
    //
    public: bool CreateChildren(RtkCfgFile *cfg);
    
    // Recursive versions for standard functions
    // These will call the function for all children, grand-children, etc,
    // and will normally be called from the root object.
    //
    public: bool StartupChildren(RtkCfgFile *cfg);
    public: void ShutdownChildren();
    public: void UpdateChildren();

    // Add a child object
    //
    public: void AddChild(CObject *child);

    // Get the first ancestor of the given run-time type
    // Note: will return ourself if the type matches.
    //
    public: CObject* FindAncestor(const type_info &type); 

    // Convert local to global coords
    //
    public: void LocalToGlobal(double &px, double &py, double &pth);

    // Convert global to local coords
    //
    public: void GlobalToLocal(double &px, double &py, double &pth);

    // Set the objects pose in the parent cs
    //
    public: void SetPose(double px, double py, double pth);

    // Get the objects pose in the parent cs
    //
    public: void GetPose(double &px, double &py, double &pth);

    // Get the objects pose in the global cs
    //
    public: void SetGlobalPose(double px, double py, double pth);

    // Get the objects pose in the global cs
    //
    public: void GetGlobalPose(double &px, double &py, double &pth);

    // Id of this object
    //
    public: char m_id[64];

    // Pointer to world
    // *** HACK -- get rid of the device class, then make this protected
    //
    public: CWorld *m_world;
    
    // Pointer to parent object
    //
    protected: CObject *m_parent;

    // Object pose in local cs (ie relative to parent)
    //
    private: double m_lx, m_ly, m_lth;
    
    // Current object pose in global cs
    //
    private: double m_gx, m_gy, m_gth;

    // Flag set if global pose is out-of-date
    //
    private: bool m_global_dirty;

    // List of child objects
    //
    private: int m_child_count;
    private: CObject *m_child[64];
    
#ifdef INCLUDE_RTK
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

    // Flag set if object is being dragged
    //
    private: bool m_dragging;

    // Drag radius for object
    //
    protected: double m_drag_radius;

#endif
};


#endif


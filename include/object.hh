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
//  $Revision: 1.1.2.1 $
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

    // Startup routine -- creates objects in the world
    //
    public: virtual bool Startup();

    // Shutdown routine -- deletes objects in the world
    //
    public: virtual void Shutdown();
    
    // Update the object's representation
    //
    public: virtual void Update();

    // Add a child object
    //
    public: void AddChild(CObject *child);

    // Convert local to global coords
    //
    public: void LocalToGlobal(double &px, double &py, double &pth);

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

    // Mark ourselves and all our children as dirty
    //
    private: void SetGlobalDirty();

    // Pointer to world
    //
    protected: CWorld *m_world;
    
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


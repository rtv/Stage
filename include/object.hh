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
//  $Revision: 1.1.2.8 $
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
    //
    public: virtual ~CObject();

    // Initialise the object from an argument list
    //
    public: virtual bool Init(int argc, char **argv);

    // Save the object to a file
    //
    public: virtual bool Save(char *buffer, size_t bufflen);

    // Startup routine
    //
    public: virtual bool Startup();

    // Shutdown routine
    //
    public: virtual void Shutdown();
    
    // Update the object's representation
    //
    public: virtual void Update();

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
    //
    public: CWorld *m_world;
    
    // Pointer to parent object
    //
    public: CObject *m_parent;

    // Number of parents this object has
    //
    public: int m_depth;

    // Object pose in local cs (ie relative to parent)
    //
    private: double m_lx, m_ly, m_lth;
 
    
#ifdef INCLUDE_RTK

    // UI property message handler
    //
    public: virtual void OnUiProperty(RtkUiPropertyData* pData);
    
    // Process GUI update messages
    //
    public: virtual void OnUiUpdate(RtkUiDrawData *pData);

    // Process GUI mouse messages
    //
    public: virtual void OnUiMouse(RtkUiMouseData *pData);

    // Return true if mouse is over object
    //
    protected: bool IsMouseReady() {return m_mouse_ready;};

    // Move object with the mouse
    //
    public: bool MouseMove(RtkUiMouseData *pData);
    
    // Mouse must be withing this radius for interaction
    //
    protected: double m_mouse_radius;

    // Flag set of object is draggable
    //
    protected: bool m_draggable;

    // Flag set of mouse is within radius
    //
    private: bool m_mouse_ready;

    // Flag set if object is being dragged
    //
    private: bool m_dragging;
    
#endif
};


#endif


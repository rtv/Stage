///////////////////////////////////////////////////////////////////////////
//
// File: entity.hh
// Author: Andrew Howard
// Date: 04 Dec 2000
// Desc: Base class for movable objects
//
// CVS info:
//  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/include/entity.hh,v $
//  $Author: gerkey $
//  $Revision: 1.5 $
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


#ifndef ENTITY_HH
#define ENTITY_HH

#include "stage_types.hh"
#include "guiexport.hh"

#ifdef INCLUDE_XGUI
#include "xgui.hh"
#endif

#ifdef INCLUDE_RTK
#include "rtk_ui.hh"
#endif

// Forward declare the world class
//
class CWorld;


///////////////////////////////////////////////////////////////////////////
// The basic object class
//
class CEntity
{
    // Minimal constructor
    // Requires a pointer to the parent and a pointer to the world.
    //
    public: CEntity(CWorld *world, CEntity *parent_object);

    // Destructor
    //
    public: virtual ~CEntity();

    // Load the object from an argument list
    //
    public: virtual bool Load(int argc, char **argv);

    // Save the object to an argument list
    //
    public: virtual bool Save(int &argc, char **argv);

    // Initialise object
    //
    public: virtual bool Startup();

    // Finalize object
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

    // Line in the world description file
    //
    public: int m_line;
    public: int m_column;

    // Type of this object
    //
    public: char m_type[64];
    
    // Id of this object
    //
    public: char m_id[64];

    // Pointer to world
    //
    public: CWorld *m_world;
    
    // Pointer to parent object
    // i.e. the object this object is attached to.
    //
    public: CEntity *m_parent_object;

    // Pointer the default object
    // i.e. the object that would-be children of this object should attach to.
    // This will usually be the object itself.
    //
    public: CEntity *m_default_object;

    // Object pose in local cs (ie relative to parent)
    //
    private: double m_lx, m_ly, m_lth;

    // Commanded speed
    //
    protected: double m_com_vr, m_com_vth;
    protected: double m_mass;

    public: double GetSpeed() { return(m_com_vr); }
    public: double GetMass() { return(m_mass); }
    
    // shouldn't really have this, but...
    public: void SetSpeed(double speed) { m_com_vr=speed; }

    // struct that holds data for external GUI rendering
    //
    // i made this public so that an object can get another object's
    // type - BPG
    //protected: ExportData exp;
    public: ExportData exp;
   
    // compose and return the export data structure
    //
    public: virtual ExportData* ImportExportData( const ImportData* imp ); 
  //public: virtual void ImportData( ImportData* data ); 

    // enable/disable export subscription
    protected: bool exporting;
    public: void EnableGuiExport( void ){ exporting = true; };
    public: void DisableGuiExport( void ){ exporting = false; };
    public: void ToggleGuiExport( void ){ exporting = !exporting; };

    // Object color description (for display)
    //
    private: char m_color_desc[128];
    
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

    // Object color
    //
    public: RTK_COLOR m_color;
    
#endif
};


#endif


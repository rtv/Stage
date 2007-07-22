#include <stage_internal.h>

class StgWorld
{
public:
    stg_id_t id; ///< Stage's unique identifier for this world
    
    GHashTable* models; ///< the models that make up the world, indexed by id
    GHashTable* models_by_name; ///< the models that make up the world, indexed by name
    /** a list of models that are currently selected by the user */
    GList* selected_models;

    CWorldFile* wf; ///< If set, points to the worldfile used to create this world

    /** a list of top-level models, i.e. models who have the world as
    their parent and whose position is therefore specified in world
    coordinates */
    GList* children;

    /** a list of models that should be updated with a call to
	stg_model_udpate(); */
    GList* update_list;

    stg_meters_t width; ///< x size of the world 
    stg_meters_t height; ///< y size of the world

    /** the number of models of each type is counted so we can
	automatically generate names for them
    */
    int child_type_count[256];
    
    struct _stg_matrix* matrix; ///< occupancy quadtree for model raytracing

    char* token; ///< the name of this world

    stg_msec_t sim_time; ///< the current time in this world
    stg_msec_t sim_interval; ///< this much simulated time elapses each step.
    long unsigned int updates; ///< the number of simulaticuted
    
    /** real-time interval between updates - set this to zero for 'as fast as possible' 
     */
    stg_msec_t wall_interval;
    stg_msec_t wall_last_update; ///< the wall-clock time of the last world update

    stg_msec_t gui_interval; ///< real-time interval between GUI canvas updates
    stg_msec_t gui_last_update; ///< the wall-clock time of the last gui canvas update

    /** the wallclock-time interval elapsed between the last two
	updates - compare this with sim_interval to see the ratio of
	sim to real time 
    */
    stg_msec_t real_interval_measured;

    double ppm; ///< the resolution of the world model in pixels per meter

    gboolean paused; ///< the world only updates when this is zero
   
    gboolean destroy; ///< this world should be destroyed ASAP

    gui_window_t* win; ///< the gui window associated with this world

    int subs; ///< the total number of subscriptions to all models

    int section_count;
};

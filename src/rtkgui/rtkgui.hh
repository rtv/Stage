
class CEntity;

// these funcs are the external interface to the rtkgui library
int RtkGuiInit( int argc, char** argv );
int RtkGuiLoad( stage_gui_config_t* cfg );
int RtkGuiUpdate( void );
int RtkGuiEntityStartup( CEntity* ent );
int RtkGuiEntityShutdown( CEntity* ent );
int RtkGuiEntityPropertyChange( CEntity* ent, stage_prop_id_t prop );

// mouse handling callback
void RtkOnMouse(rtk_fig_t *fig, int event, int mode);


// this data was previously in the entity class; now attached to the 
// void* CEntity::gui_data
typedef struct 
{
  rtk_fig_t *fig_rects, *fig_cmd, *fig_cfg, *fig_data, *fig_label, *fig_grid;
  bool grid_enable;
  double grid_major, grid_minor;
  int movemask;
  int type; // the model type
} rtk_entity_t;


class CEntity;

int RtkGuiInit( int argc, char** argv );
int RtkGuiLoad( stage_gui_config_t* cfg );
int RtkGuiUpdate( void );
int RtkGuiEntityStartup( CEntity* ent );
int RtkGuiEntityShutdown( CEntity* ent );
int RtkGuiEntityPropertyChange( CEntity* ent, stage_prop_id_t prop );

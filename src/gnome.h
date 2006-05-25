#ifndef STG_GNOME_H
#define STG_GNOME_H

#include <libgnomecanvas/libgnomecanvas.h>

gint background_event_callback(GnomeCanvasItem* item, 
				      GdkEvent* event, 
				      gpointer data);

gint acetate_event_callback(GnomeCanvasItem* item, 
				      GdkEvent* event, 
				      gpointer data);

gint model_event_callback(GnomeCanvasItem* item, 
				 GdkEvent* event, 
			  gpointer data);

int gc_model_polygons(  stg_model_t* mod, void* userp );
int gc_model_move( stg_model_t* mod, void* userp );
int gc_laser_render_data( stg_model_t* mod, void* userp );
int gc_laser_unrender_data( stg_model_t* mod, void* userp );

#endif

#ifndef STG_GNOME_H
#define STG_GNOME_H

#include <libgnomecanvas/libgnomecanvas.h>

gint background_event_callback(GnomeCanvasItem* item, 
				      GdkEvent* event, 
				      gpointer data);

gint model_event_callback(GnomeCanvasItem* item, 
				 GdkEvent* event, 
				 gpointer data);

int model_render_polygons_gc(  stg_model_t* mod, char* name, 
				void* data, size_t len, void* userp );

int laser_render_data_gc( stg_model_t* mod, char* name, 
			  void* data, size_t len, void* userp );

int laser_unrender_data_gc( stg_model_t* mod, char* name, 
			    void* data, size_t len, void* userp );


#endif

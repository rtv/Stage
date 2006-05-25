/* "Acetate" item type for GnomeCanvas widget. This item is invisible
 * but is there when you touch it.
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@acm.org>
 */

#ifndef GNOME_CANVAS_ACETATE_H
#define GNOME_CANVAS_ACETATE_H

//BEGIN_GNOME_DECLS


/* Acetate item for the canvas.
 *
 * The following object arguments are available:
 *
 * name			type			read/write	description
 * ------------------------------------------------------------------------------------------
 */

#define GNOME_TYPE_CANVAS_ACETATE            (gnome_canvas_acetate_get_type ())
#define GNOME_CANVAS_ACETATE(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_CANVAS_ACETATE, GnomeCanvasAcetate))
#define GNOME_CANVAS_ACETATE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_ACETATE, GnomeCanvasAcetateClass))
#define GNOME_IS_CANVAS_ACETATE(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CANVAS_ACETATE))
#define GNOME_IS_CANVAS_ACETATE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_ACETATE))


typedef struct _GnomeCanvasAcetate GnomeCanvasAcetate;
typedef struct _GnomeCanvasAcetateClass GnomeCanvasAcetateClass;

struct _GnomeCanvasAcetate {
	GnomeCanvasItem item;
};

struct _GnomeCanvasAcetateClass {
	GnomeCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType gnome_canvas_acetate_get_type (void);


//END_GNOME_DECLS

#endif

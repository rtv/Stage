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

/* These includes are set up for standalone compile. If/when this codebase
   is integrated into libgnomeui, the includes will need to change. */
#include <math.h>
#include <string.h>
#include <libgnomecanvas/libgnomecanvas.h>
//#include <gnome.h>
#include "gnome-canvas-acetate.h"


enum {
	ARG_0
};


static void gnome_canvas_acetate_class_init (GnomeCanvasAcetateClass *class);
static void gnome_canvas_acetate_init       (GnomeCanvasAcetate      *acetate);
static void gnome_canvas_acetate_destroy    (GtkObject               *object);
static void gnome_canvas_acetate_set_arg    (GtkObject               *object,
					     GtkArg                  *arg,
					     guint                    arg_id);
static void gnome_canvas_acetate_get_arg    (GtkObject               *object,
					     GtkArg                  *arg,
					     guint                    arg_id);

static void   gnome_canvas_acetate_update      (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
static void   gnome_canvas_acetate_realize     (GnomeCanvasItem *item);
static void   gnome_canvas_acetate_unrealize   (GnomeCanvasItem *item);
static void   gnome_canvas_acetate_draw        (GnomeCanvasItem *item, GdkDrawable *drawable,
						int x, int y, int width, int height);
static double gnome_canvas_acetate_point       (GnomeCanvasItem *item, double x, double y,
						int cx, int cy, GnomeCanvasItem **actual_item);
static void   gnome_canvas_acetate_bounds      (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2);
static void   gnome_canvas_acetate_render      (GnomeCanvasItem *item, GnomeCanvasBuf *buf);


static GnomeCanvasItemClass *parent_class;


GtkType
gnome_canvas_acetate_get_type (void)
{
	static GtkType acetate_type = 0;

	if (!acetate_type) {
		GtkTypeInfo acetate_info = {
			"GnomeCanvasAcetate",
			sizeof (GnomeCanvasAcetate),
			sizeof (GnomeCanvasAcetateClass),
			(GtkClassInitFunc) gnome_canvas_acetate_class_init,
			(GtkObjectInitFunc) gnome_canvas_acetate_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		acetate_type = gtk_type_unique (gnome_canvas_item_get_type (), &acetate_info);
	}

	return acetate_type;
}

static void
gnome_canvas_acetate_class_init (GnomeCanvasAcetateClass *class)
{
	GtkObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (GnomeCanvasItemClass *) class;

	parent_class = gtk_type_class (gnome_canvas_item_get_type ());

	object_class->destroy = gnome_canvas_acetate_destroy;
	object_class->set_arg = gnome_canvas_acetate_set_arg;
	object_class->get_arg = gnome_canvas_acetate_get_arg;

	item_class->update = gnome_canvas_acetate_update;
	item_class->realize = gnome_canvas_acetate_realize;
	item_class->unrealize = gnome_canvas_acetate_unrealize;
	item_class->draw = gnome_canvas_acetate_draw;
	item_class->point = gnome_canvas_acetate_point;
	item_class->bounds = gnome_canvas_acetate_bounds;
	item_class->render = gnome_canvas_acetate_render;
}

static void
gnome_canvas_acetate_init (GnomeCanvasAcetate *acetate)
{
}

static void
gnome_canvas_acetate_destroy (GtkObject *object)
{
	GnomeCanvasAcetate *acetate;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_ACETATE (object));

	acetate = GNOME_CANVAS_ACETATE (object);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/* Computes the bounding box of the acetate.
 */
static void
get_bounds (GnomeCanvasAcetate *acetate, double *bx1, double *by1, double *bx2, double *by2)
{
	/* Compute bounds of acetate */

	*bx1 = -G_MAXINT;
	*by1 = -G_MAXINT;
	*bx2 = G_MAXINT;
	*by2 = G_MAXINT;
}

static void
gnome_canvas_acetate_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{

	switch (arg_id) {
	default:
		break;
	}
}

static void
gnome_canvas_acetate_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{

	switch (arg_id) {
	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
gnome_canvas_acetate_render (GnomeCanvasItem *item,
			     GnomeCanvasBuf *buf)
{

}

static void
gnome_canvas_acetate_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
  item->x1 = -G_MAXINT;
  item->y1 = -G_MAXINT;
  item->x2 = G_MAXINT;
  item->y2 = G_MAXINT;
}

static void
gnome_canvas_acetate_realize (GnomeCanvasItem *item)
{

	if (parent_class->realize)
		(* parent_class->realize) (item);
}

static void
gnome_canvas_acetate_unrealize (GnomeCanvasItem *item)
{

	if (parent_class->unrealize)
		(* parent_class->unrealize) (item);
}


static void
gnome_canvas_acetate_draw (GnomeCanvasItem *item, GdkDrawable *drawable,
			   int x, int y, int width, int height)
{
}

static double
gnome_canvas_acetate_point (GnomeCanvasItem *item, double x, double y,
			    int cx, int cy, GnomeCanvasItem **actual_item)
{
	*actual_item = item;
	return 0.0;
}

static void
gnome_canvas_acetate_bounds (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GnomeCanvasAcetate *acetate;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_ACETATE (item));

	acetate = GNOME_CANVAS_ACETATE (item);

	get_bounds (acetate, x1, y1, x2, y2);
}


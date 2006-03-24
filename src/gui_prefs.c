#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

//#include "callbacks.h"
//#include "interface.h"
//#include "support.h"

#define _(X) (X)

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_prefsdialog (void)
{
  GtkWidget *window1;
  GtkWidget *vbox4;
  GtkWidget *notebook1;
  GtkWidget *hbox6;
  GtkWidget *vbox5;
  GtkWidget *label9;
  GtkWidget *label8;
  GtkWidget *label10;
  GtkWidget *vbox6;
  GtkObject *spinbutton5_adj;
  GtkWidget *spinbutton5;
  GtkObject *spinbutton6_adj;
  GtkWidget *spinbutton6;
  GtkObject *spinbutton7_adj;
  GtkWidget *spinbutton7;
  GtkWidget *label6;
  GtkWidget *hbox5;
  GtkWidget *button5;
  GtkWidget *button4;

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request (window1, 300, 200);
  gtk_window_set_title (GTK_WINDOW (window1), _("Stage Preferences"));

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (window1), vbox4);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (vbox4), notebook1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (notebook1), 1);

  hbox6 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox6);
  gtk_container_add (GTK_CONTAINER (notebook1), hbox6);
  gtk_notebook_set_tab_label_packing (GTK_NOTEBOOK (notebook1), hbox6,
                                      TRUE, TRUE, GTK_PACK_START);

  vbox5 = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (vbox5);
  gtk_box_pack_start (GTK_BOX (hbox6), vbox5, TRUE, TRUE, 0);

  label9 = gtk_label_new (_("Real world step (ms)"));
  gtk_widget_show (label9);
  gtk_box_pack_start (GTK_BOX (vbox5), label9, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_RIGHT);

  label8 = gtk_label_new (_("Simulation step (ms)"));
  gtk_widget_show (label8);
  gtk_box_pack_start (GTK_BOX (vbox5), label8, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_RIGHT);

  label10 = gtk_label_new (_("GUI window (ms)"));
  gtk_widget_show (label10);
  gtk_box_pack_start (GTK_BOX (vbox5), label10, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (label10), GTK_JUSTIFY_RIGHT);

  vbox6 = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (vbox6);
  gtk_box_pack_start (GTK_BOX (hbox6), vbox6, FALSE, TRUE, 12);

  spinbutton5_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
  spinbutton5 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton5_adj), 1, 0);
  gtk_widget_show (spinbutton5);
  gtk_box_pack_start (GTK_BOX (vbox6), spinbutton5, FALSE, FALSE, 0);

  spinbutton6_adj = gtk_adjustment_new (0, 0, 100, 1, 10, 10);
  spinbutton6 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton6_adj), 1, 0);
  gtk_widget_show (spinbutton6);
  gtk_box_pack_start (GTK_BOX (vbox6), spinbutton6, FALSE, FALSE, 0);

  spinbutton7_adj = gtk_adjustment_new (0, 0, 100, 1, 10, 10);
  spinbutton7 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton7_adj), 1, 0);
  gtk_widget_show (spinbutton7);
  gtk_box_pack_start (GTK_BOX (vbox6), spinbutton7, FALSE, FALSE, 0);

  label6 = gtk_label_new (_("TODO: Update intervals *NOT WORKING YET*"));
  gtk_widget_show (label6);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label6);

  hbox5 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox5, FALSE, FALSE, 7);

  button5 = gtk_button_new_from_stock ("gtk-save");
  gtk_widget_show (button5);
  gtk_box_pack_start (GTK_BOX (hbox5), button5, FALSE, FALSE, 0);

  button4 = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (button4);
  gtk_box_pack_start (GTK_BOX (hbox5), button4, FALSE, FALSE, 0);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (window1, window1, "window1");
  GLADE_HOOKUP_OBJECT (window1, vbox4, "vbox4");
  GLADE_HOOKUP_OBJECT (window1, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (window1, hbox6, "hbox6");
  GLADE_HOOKUP_OBJECT (window1, vbox5, "vbox5");
  GLADE_HOOKUP_OBJECT (window1, label9, "label9");
  GLADE_HOOKUP_OBJECT (window1, label8, "label8");
  GLADE_HOOKUP_OBJECT (window1, label10, "label10");
  GLADE_HOOKUP_OBJECT (window1, vbox6, "vbox6");
  GLADE_HOOKUP_OBJECT (window1, spinbutton5, "spinbutton5");
  GLADE_HOOKUP_OBJECT (window1, spinbutton6, "spinbutton6");
  GLADE_HOOKUP_OBJECT (window1, spinbutton7, "spinbutton7");
  GLADE_HOOKUP_OBJECT (window1, label6, "label6");
  GLADE_HOOKUP_OBJECT (window1, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (window1, button5, "button5");
  GLADE_HOOKUP_OBJECT (window1, button4, "button4");

  return window1;
}


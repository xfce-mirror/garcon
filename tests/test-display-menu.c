/*-
 * vi:set et ai sts=2 sw=2 cindent:
 *
 * Copyright (c) 2007 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <garcon/garcon.h>
#include <garcon-gtk/garcon-gtk.h>



/* Default icon size */
#define ICON_SIZE 22



/* Root menu */
static GarconMenu *root = NULL;
static GtkWidget  *gtk_root = NULL;




static void
show_menu (GtkButton *button,
           GtkWidget *menu)
{
  /* Display the menu */
  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time ());
}



static void
main_window_destroy (GtkWidget *window)
{
  gtk_main_quit ();
}



static void
create_main_window (void)
{
  GtkWidget *window;
  GtkWidget *button;

  /* Create main window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "GarconMenu: Display Menu Test");
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);
  gtk_widget_show (window);

  /* Exit main loop when when the window is closed */
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (main_window_destroy), NULL);

  /* Create button */
  button = gtk_button_new_with_mnemonic ("_Show menu");
  gtk_container_add (GTK_CONTAINER (window), button);
  gtk_widget_show (button);

  /* Create Garcon's root menu to show the applications */
  gtk_root = garcon_gtk_menu_new (NULL);
  garcon_gtk_menu_set_menu (GARCON_GTK_MENU (gtk_root), root);

  /* Display root menu when the button is clicked */
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (show_menu), gtk_root);
}



gint
main (gint    argc,
      gchar **argv)
{
  gint    exit_code = EXIT_SUCCESS;

  /* Initialize the menu library */
  garcon_set_environment ("XFCE");

  /* Initialize GTK+ */
  gtk_init (&argc, &argv);

  /* Try to load the menu */
  if (G_UNLIKELY (g_strv_length (argv) > 1))
    root = garcon_menu_new_for_path (argv[1]);
  else
    root = garcon_menu_new_applications ();

  /* create the main window */
  create_main_window ();

  /* Enter main loop */
  gtk_main ();

  /* Destroy the root menu */
  g_object_unref (root);

  return exit_code;
}

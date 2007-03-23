/* $Id$ */
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <gtk/gtk.h>

#include <glib.h>
#include <glib-object.h>
#include <glib/gprintf.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4menu/libxfce4menu.h>



/* Default icon size */
#define ICON_SIZE 22



/* Root menu */
static XfceMenu *root = NULL;



static void
execute_item_command (GtkWidget    *widget,
                      XfceMenuItem *item)
{
#if 0
  GError      *error = NULL;
#endif
  const gchar *command;
  gboolean     terminal;
  gboolean     startup_notification;

  command = xfce_menu_item_get_command (item);
  terminal = xfce_menu_item_requires_terminal (item);
  startup_notification = xfce_menu_item_supports_startup_notification (item);

  if (G_UNLIKELY (command == NULL))
    return;

#if 0
  if (!xfce_exec_on_screen (gdk_screen_get_default (), command, terminal, startup_notification, &error))
    {
      xfce_err (error->message);
      g_error_free (error);
    }
#endif
}



static GdkPixbuf*
create_item_icon (XfceMenuItem *item)
{
  GdkPixbuf    *icon = NULL;
  GtkIconTheme *icon_theme;
  const gchar  *icon_name;
  const gchar  *item_name;
  gchar        *basename;
  gchar        *extension;
  gchar        *new_item_name;
  gchar         new_icon_name[1024];

  /* Get current icon theme */
  icon_theme = gtk_icon_theme_get_default ();

  icon_name = xfce_menu_item_get_icon_name (item);
  item_name = xfce_menu_item_get_name (item);

  if (icon_name == NULL)
    return NULL;

  /* Check if we have an absolute filename */
  if (g_path_is_absolute (icon_name) && g_file_test (icon_name, G_FILE_TEST_EXISTS))
    icon = gdk_pixbuf_new_from_file_at_scale (icon_name, ICON_SIZE, ICON_SIZE, TRUE, NULL);
  else
    {
      /* Try to load the icon name directly using the icon theme */
      icon = gtk_icon_theme_load_icon (icon_theme, icon_name, ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);

      /* If that didn't work, try to remove the filename extension if there is one */
      if (icon == NULL)
        {
          /* Get basename (just to be sure) */
          basename = g_path_get_basename (icon_name);

          /* Determine position of the extension */
          extension = g_utf8_strrchr (basename, -1, '.');

          /* Make sure we found an extension */
          if (extension != NULL)
            {
              /* Remove extension */
              g_utf8_strncpy (new_icon_name, basename, g_utf8_strlen (basename, -1) - g_utf8_strlen (extension, -1));

              /* Try to load the pixbuf using the new icon name */
              icon = gtk_icon_theme_load_icon (icon_theme, new_icon_name, ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
            }

          /* Free basename */
          g_free (basename);

          /* As a last fallback, we try to load the icon by lowercase item name */
          if (icon == NULL && item_name != NULL)
            {
              new_item_name = g_utf8_strdown (item_name, -1);
              icon = gtk_icon_theme_load_icon (icon_theme, new_item_name, ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
              g_free (new_item_name);
            }
        }
    }

  /* Scale icon (if needed) */
  if (icon != NULL)
    {
      GdkPixbuf *old_icon = icon;
      icon = gdk_pixbuf_scale_simple (old_icon, ICON_SIZE, ICON_SIZE, GDK_INTERP_BILINEAR);
      g_object_unref (old_icon);
    }

  return icon;
}



static void
create_item_widgets (XfceMenuItem *item, 
                     GtkWidget    *parent_menu)
{
  GtkWidget *gtk_item;
  GtkWidget *image;
  GdkPixbuf *icon;

  /* Try to load the icon */
  icon = create_item_icon (item);

  if (icon != NULL)
    image = gtk_image_new_from_pixbuf (icon);
  else
    image = gtk_image_new_from_icon_name ("applications-other", ICON_SIZE);

  gtk_item = gtk_image_menu_item_new_with_label (xfce_menu_item_get_name (item));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (gtk_item), image);
  gtk_menu_shell_append (GTK_MENU_SHELL (parent_menu), gtk_item);
  gtk_widget_show (gtk_item);

  /* Execute command if item is clicked */
  g_signal_connect (gtk_item, "activate", G_CALLBACK (execute_item_command), item);
}



static void
create_menu_widgets (GtkWidget *gtk_menu, 
                     XfceMenu  *menu)
{
  XfceMenuDirectory *directory;
  XfceMenu          *submenu;
  GtkIconTheme      *icon_theme;
  GtkWidget         *gtk_item;
  GtkWidget         *gtk_submenu;
  GtkWidget         *image;
  GdkPixbuf         *icon;
  GSList            *iter;
  GSList            *items;
  const gchar       *display_name;
  const gchar       *icon_name;

  /* Get current icon theme */
  icon_theme = gtk_icon_theme_get_default ();

  /* Get submenus and items based on the menu layout */
  if (G_UNLIKELY (xfce_menu_has_layout (menu)))
    items = xfce_menu_get_layout_items (menu);
  else
    {
      items = xfce_menu_get_menus (menu);
      items = g_slist_concat (items, xfce_menu_get_items (menu));
    }

  /* Iterate over menu items */
  for (iter = items; iter != NULL; iter = g_slist_next (iter))
    {
      if (XFCE_IS_MENU_ITEM (iter->data))
        {
          /* Add menu item to the menu */
          create_item_widgets (XFCE_MENU_ITEM (iter->data), gtk_menu);
        }
      else if (XFCE_IS_MENU_SEPARATOR (iter->data))
        {
          /* Add separator to the menu */
          gtk_item = gtk_separator_menu_item_new ();
          gtk_menu_shell_append (GTK_MENU_SHELL (gtk_menu), gtk_item);
          gtk_widget_show (gtk_item);
        }
      else if (XFCE_IS_MENU (iter->data))
        {
          submenu = XFCE_MENU (iter->data);
          directory = xfce_menu_get_directory (submenu);

          /* Determine display name */
          display_name = directory != NULL ? xfce_menu_directory_get_name (directory) : xfce_menu_get_name (submenu);

          /* Determine icon name */
          icon_name = directory != NULL ? xfce_menu_directory_get_icon (directory) : "applications-other";

          /* Load menu icon */
          icon = gtk_icon_theme_load_icon (icon_theme, icon_name, ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
          if (G_UNLIKELY (icon == NULL))
            icon = gtk_icon_theme_load_icon (icon_theme, "applications-other", ICON_SIZE, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);

          /* Create image widget */
          image = gtk_image_new_from_pixbuf (icon);

          /* Create menu item */
          gtk_item = gtk_image_menu_item_new_with_label (display_name);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (gtk_item), image);
          gtk_menu_shell_append (GTK_MENU_SHELL (gtk_menu), gtk_item);
          gtk_widget_show (gtk_item);

          /* Create submenu */
          gtk_submenu = gtk_menu_new ();
          gtk_menu_item_set_submenu (GTK_MENU_ITEM (gtk_item), gtk_submenu);

          /* Create widgets for submenu */
          create_menu_widgets (gtk_submenu, submenu);

          /* Destroy submenu if it is empty */
          if (G_UNLIKELY (gtk_container_get_children (GTK_CONTAINER (gtk_submenu)) == NULL))
            gtk_widget_destroy (gtk_item);
        }
    }

  /* Free menu item list */
  g_slist_free (items);
}



static void 
show_menu (GtkButton *button, 
           GtkWidget *menu)
{
  /* Create menu widgets if not already done */
  if (g_list_length (gtk_container_get_children (GTK_CONTAINER (menu))) == 0)
    create_menu_widgets (menu, root);

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
  GtkWidget *menu;

  /* Create main window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), _("XfceMenu: Display Menu Test"));
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);
  gtk_widget_show (window);

  /* Exit main loop when when the window is closed */
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (main_window_destroy), NULL);

  /* Create button */
  button = gtk_button_new_with_mnemonic (_("_Show menu"));
  gtk_container_add (GTK_CONTAINER (window), button);
  gtk_widget_show (button);

  /* Create GTK+ root menu */
  menu = gtk_menu_new ();

  /* Display root menu when the button is clicked */
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (show_menu), menu);
}



int 
main (int    argc,
      char **argv)
{
  GError *error     = NULL;
  gint    exit_code = EXIT_SUCCESS;

  /* Initialize the menu library */
  xfce_menu_init ("XFCE");

  /* Initialize GTK+ */
  gtk_init (&argc, &argv);

  /* Try to load the menu */
  if (G_UNLIKELY (g_strv_length (argv) > 1))
    root = xfce_menu_new (argv[1], &error);
  else
    root = xfce_menu_get_root (&error);

  /* Check if the menu was loaded */
  if (G_LIKELY (root != NULL))
    {
      /* Create main window */
      create_main_window ();

      /* Enter main loop */
      gtk_main ();

      /* Destroy the root menu */
      g_object_unref (root);
    }
  else
    {
      g_error (error->message);
      exit_code = EXIT_FAILURE;
    }

  /* Shut down the menu library */
  xfce_menu_shutdown ();

  return exit_code;
}

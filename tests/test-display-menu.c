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

#include <stdlib.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <garcon/garcon.h>



/* Default icon size */
#define ICON_SIZE 22



/* Root menu */
static GarconMenu *root = NULL;
static GtkWidget  *gtk_root = NULL;



static void
execute_item_command (GtkWidget    *widget,
                      GarconMenuItem *item)
{
#if 0
  GError      *error = NULL;
#endif
  const gchar *command;
  gboolean     terminal;
  gboolean     startup_notification;

  command = garcon_menu_item_get_command (item);
  terminal = garcon_menu_item_requires_terminal (item);
  startup_notification = garcon_menu_item_supports_startup_notification (item);

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
create_item_icon (GarconMenuItem *item)
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

  item_name = garcon_menu_element_get_name (GARCON_MENU_ELEMENT (item));
  icon_name = garcon_menu_element_get_icon_name (GARCON_MENU_ELEMENT (item));

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
item_changed (GarconMenuItem *item,
              GtkWidget      *gtk_item)
{
  GdkPixbuf *icon;
  GtkWidget *image;

  /* Try reloading the icon */
  icon = create_item_icon (item);

  if (icon != NULL)
    image = gtk_image_new_from_pixbuf (icon);
  else
    image = gtk_image_new_from_icon_name ("applications-other", ICON_SIZE);

  gtk_menu_item_set_label (GTK_MENU_ITEM (gtk_item), garcon_menu_element_get_name (GARCON_MENU_ELEMENT (item)));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (gtk_item), image);
}



static void
create_item_widgets (GarconMenuItem *item,
                     GtkWidget      *parent_menu)
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

  gtk_item = gtk_image_menu_item_new_with_label (garcon_menu_element_get_name (GARCON_MENU_ELEMENT (item)));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (gtk_item), image);
  gtk_menu_shell_append (GTK_MENU_SHELL (parent_menu), gtk_item);
  gtk_widget_show (gtk_item);

  g_object_set_data_full (G_OBJECT (gtk_item), "garcon-menu-item", g_object_ref (item), g_object_unref);

  /* React to changes made to the item on disk */
  g_signal_connect (item, "changed", G_CALLBACK (item_changed), gtk_item);

  /* Execute command if item is clicked */
  g_signal_connect (gtk_item, "activate", G_CALLBACK (execute_item_command), item);
}



static void
directory_changed (GarconMenu          *menu,
                   GarconMenuDirectory *old_directory,
                   GarconMenuDirectory *new_directory,
                   GtkWidget           *item)
{
  const gchar *display_name;
  const gchar *icon_name;
  GtkWidget   *image;

  g_debug ("directory changed from %s to %s", 
           old_directory != NULL ? garcon_menu_directory_get_name (old_directory) : NULL,
           new_directory != NULL ? garcon_menu_directory_get_name (new_directory) : NULL);

  display_name = garcon_menu_element_get_name (GARCON_MENU_ELEMENT (menu));
  gtk_menu_item_set_label (GTK_MENU_ITEM (item), display_name);
  
  icon_name = garcon_menu_element_get_icon_name (GARCON_MENU_ELEMENT (menu));
  if (icon_name == NULL)
    icon_name = "applications-other";
  image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (item));
  gtk_image_set_from_icon_name (GTK_IMAGE (image), icon_name, ICON_SIZE);
}



static void
item_added (GarconMenu     *menu,
            GarconMenuItem *item,
            guint           position,
            GtkWidget      *gtk_menu)
{
  /* Add menu item to the menu */
  create_item_widgets (item, gtk_menu);
}



static void
item_removed (GarconMenu     *menu,
              GarconMenuItem *item,
              GtkWidget      *gtk_menu)
{
  GarconMenuItem *corresponding_item;
  GList          *children;
  GList          *lp;

  children = gtk_container_get_children (GTK_CONTAINER (gtk_menu));

  for (lp = children; lp != NULL; lp = lp->next)
    {
      corresponding_item = g_object_get_data (G_OBJECT (lp->data), "garcon-menu-item");
      if (corresponding_item != NULL)
        {
          if (garcon_menu_element_equal (GARCON_MENU_ELEMENT (item),
                                         GARCON_MENU_ELEMENT (corresponding_item)))
            {
              gtk_container_remove (GTK_CONTAINER (gtk_menu), lp->data);
            }
        }
    }
}



static void
create_menu_widgets (GtkWidget   *gtk_menu,
                     GarconMenu  *menu)
{
  GarconMenuDirectory *directory;
  GarconMenu          *submenu;
  GtkIconTheme        *icon_theme;
  GtkWidget           *gtk_item;
  GtkWidget           *gtk_submenu;
  GtkWidget           *image;
  GdkPixbuf           *icon;
  GList               *iter;
  GList               *items;
  const gchar         *display_name;
  const gchar         *icon_name;

  /* Get current icon theme */
  icon_theme = gtk_icon_theme_get_default ();

  /* Get submenus and items based on the menu layout */
  items = garcon_menu_get_elements (menu);

  /* Iterate over menu items */
  for (iter = items; iter != NULL; iter = g_list_next (iter))
    {
      if (GARCON_IS_MENU_ITEM (iter->data))
        {
          /* Add menu item to the menu */
          create_item_widgets (GARCON_MENU_ITEM (iter->data), gtk_menu);
        }
      else if (GARCON_IS_MENU_SEPARATOR (iter->data))
        {
          /* Add separator to the menu */
          gtk_item = gtk_separator_menu_item_new ();
          gtk_menu_shell_append (GTK_MENU_SHELL (gtk_menu), gtk_item);
          gtk_widget_show (gtk_item);
        }
      else if (GARCON_IS_MENU (iter->data))
        {
          submenu = GARCON_MENU (iter->data);
          directory = garcon_menu_get_directory (submenu);

          /* Determine display name */
          display_name = garcon_menu_element_get_name (GARCON_MENU_ELEMENT (submenu));

          /* Determine icon name */
          icon_name = garcon_menu_element_get_icon_name (GARCON_MENU_ELEMENT (submenu));
          if (icon_name == NULL)
            icon_name = "applications-other";

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

#if 0
          /* Update the menu item when the menu directory changes */
          g_signal_connect (submenu, "directory-changed", G_CALLBACK (directory_changed), gtk_item);
#endif

          /* Remvoe the menu item and submenu if there are no menu items left in it */
          /* g_signal_connect (submenu, "destroy", G_CALLBACK (menu_destroyed), gtk_item); */

#if 0
          /* Remove menu items if they are removed on disk */
          g_signal_connect (submenu, "item-added", G_CALLBACK (item_added), gtk_submenu);
          g_signal_connect (submenu, "item-removed", G_CALLBACK (item_removed), gtk_submenu);
#endif

          /* Create widgets for submenu */
          create_menu_widgets (gtk_submenu, submenu);

          /* Destroy submenu if it is empty */
          /* FIXME destroying menus will not work with monitoring. if a menu is destroyed and
           * an item is added to it at runtime, where should we add it...? instead, we should
           * just hide the empty menu */
#if 0
          if (G_UNLIKELY (gtk_container_get_children (GTK_CONTAINER (gtk_submenu)) == NULL))
            gtk_widget_destroy (gtk_item);
#endif
          if (G_UNLIKELY (gtk_container_get_children (GTK_CONTAINER (gtk_submenu)) == NULL))
            gtk_widget_hide (gtk_item);
        }
    }

  /* Free menu item list */
  g_list_free (items);
}



static void
remove_child (GtkWidget *child,
              GtkMenu   *menu)
{
  gtk_container_remove (GTK_CONTAINER (menu), child);
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

  /* Create GTK+ root menu */
  gtk_root = gtk_menu_new ();

  /* Display root menu when the button is clicked */
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (show_menu), gtk_root);
}



static gboolean
reload_menu (GError **error)
{
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  return garcon_menu_load (root, NULL, error);
}



static void
reload_required (GarconMenu *menu)
{
  GError *error = NULL;

  g_return_if_fail (GARCON_IS_MENU (menu));

  g_debug ("reload required");

  if (!reload_menu (&error)) 
    {
      g_warning ("Failed to reload the menu: %s", error != NULL ? error->message : "No error");
      g_error_free (error);
    }
  
  gtk_container_foreach (GTK_CONTAINER (gtk_root), (GtkCallback) remove_child, gtk_root);
}



gint
main (gint    argc,
      gchar **argv)
{
  gint    exit_code = EXIT_SUCCESS;
  GError *error = NULL;

  /* Initialize the menu library */
  garcon_set_environment ("XFCE");

  /* Initialize GTK+ */
  gtk_init (&argc, &argv);

  /* Try to load the menu */
  if (G_UNLIKELY (g_strv_length (argv) > 1))
    root = garcon_menu_new_for_path (argv[1]);
  else
    root = garcon_menu_new_applications ();

  /* Check if the menu was loaded */
  if (reload_menu (&error))
    {
      /* create the main window */
      create_main_window ();

      /* be notified when a menu rebuild is required */
      g_signal_connect (root, "reload-required", G_CALLBACK (reload_required), NULL);

      /* Enter main loop */
      gtk_main ();
    }
  else
    {
      g_error ("Failed to load the menu: %s", error != NULL ? error->message : "No error");
      g_error_free (error);
      exit_code = EXIT_FAILURE;
    }

  /* Destroy the root menu */
  g_object_unref (root);

  return exit_code;
}

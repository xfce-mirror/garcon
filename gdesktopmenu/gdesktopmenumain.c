/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General 
 * Public License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>

#include <gdesktopmenu/gdesktopmenu.h>
#include <gdesktopmenu/gdesktopmenuenvironment.h>
#include <gdesktopmenu/gdesktopmenuitemcache.h>
#include <gdesktopmenu/gdesktopmenudirectory.h>
#include <gdesktopmenu/gdesktopmenumonitor.h>
#include <gdesktopmenu/gdesktopmenuseparator.h>



/**
 * SECTION:gdesktopmenumain
 * @title: Library Initialization and Shutdown
 *
 * Library Initialization and Shutdown.
 **/



static gint g_desktop_menu_ref_count = 0;



/**
 * g_desktop_menu_init:
 * @env : name of the desktop environment (e.g. XFCE, GNOME or KDE) 
 *        or %NULL.
 *
 * Initializes the gdesktopmenu library. @env optionally defines the 
 * name of the desktop environment for which menus will be generated. 
 * This means that items belonging only to other desktop environments 
 * will be ignored.
 **/
void
g_desktop_menu_init (const gchar *env)
{
  if (g_atomic_int_exchange_and_add (&g_desktop_menu_ref_count, 1) == 0)
    {
      /* Initialize the GThread system */
      if (!g_thread_supported ())
        g_thread_init (NULL);

      /* Initialize the GObject type system */
      g_type_init ();

      /* Set desktop environment */
      g_desktop_menu_set_environment (env);

      /* Initialize the menu item cache */
      _g_desktop_menu_item_cache_init ();

      /* Initialize the directory module */
      _g_desktop_menu_directory_init ();

      /* Initialize monitoring system */
      _g_desktop_menu_monitor_init ();

      /* Creates the menu separator */
      _g_desktop_menu_separator_init ();
    }
}



/**
 * g_desktop_menu_shutdown:
 *
 * Shuts the gdesktopmenu library down.
 **/
void
g_desktop_menu_shutdown (void)
{
  if (g_atomic_int_dec_and_test (&g_desktop_menu_ref_count))
    {
      /* Unset desktop environment */
      g_desktop_menu_set_environment (NULL);

      /* Destroys the menu separator */
      _g_desktop_menu_separator_shutdown ();

      /* Shutdown monitoring system */
      _g_desktop_menu_monitor_shutdown ();

      /* Shutdown the directory module */
      _g_desktop_menu_directory_shutdown ();

      /* Shutdown the menu item cache */
      _g_desktop_menu_item_cache_shutdown ();
    }
}



gchar *
g_desktop_menu_config_lookup (const gchar *filename)
{
  const gchar * const *dirs;
  gchar               *path;
  gint                 i;

  g_return_val_if_fail (filename != NULL && g_utf8_strlen (filename, -1) > 0, NULL);

  path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_config_dir (), filename, NULL);

  if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
      g_free (path);
      path = NULL;

      dirs = g_get_system_config_dirs ();

      for (i = 0; path == NULL && dirs[i] != NULL; ++i)
        {
          if (g_path_is_absolute (dirs[i]))
            {
              path = g_build_path (G_DIR_SEPARATOR_S, dirs[i], filename, NULL);

              if (!g_file_test (path, G_FILE_TEST_IS_REGULAR))
                {
                  g_free (path);
                  path = NULL;
                }
            }
        }
    }
  
  return path;
}

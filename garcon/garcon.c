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

#include <garcon/garcon.h>
#include <garcon/garcon-environment.h>



/**
 * SECTION:garcon
 * @title: Library Initialization and Shutdown
 *
 * Library Initialization and Shutdown.
 **/



static gint garcon_ref_count = 0;



/**
 * garcon_init:
 * @env : name of the desktop environment (e.g. XFCE, GNOME or KDE)
 *        or %NULL.
 *
 * Initializes the garcon library. @env optionally defines the
 * name of the desktop environment for which menus will be generated.
 * This means that items belonging only to other desktop environments
 * will be ignored.
 **/
void
garcon_init (const gchar *env)
{
  if (g_atomic_int_exchange_and_add (&garcon_ref_count, 1) == 0)
    {
      /* Initialize the GThread system */
      if (!g_thread_supported ())
        g_thread_init (NULL);

      /* Initialize the GObject type system */
      g_type_init ();

      /* Set desktop environment */
      garcon_set_environment (env);
   }
}



/**
 * garcon_shutdown:
 *
 * Shuts the garcon library down.
 **/
void
garcon_shutdown (void)
{
  if (g_atomic_int_dec_and_test (&garcon_ref_count))
    {
      /* Unset desktop environment */
      garcon_set_environment (NULL);
    }
}



gchar *
garcon_config_lookup (const gchar *filename)
{
  const gchar * const *dirs;
  gchar               *path;
  guint                i;

  g_return_val_if_fail (filename != NULL && *filename != '\0', NULL);

  /* Look for the file in the user's config directory */
  path = g_build_filename (g_get_user_config_dir (), filename, NULL);
  if (g_path_is_absolute (path)
      && g_file_test (path, G_FILE_TEST_IS_REGULAR))
    return path;
  g_free (path);

  /* Look for the file in the system config directories */
  dirs = g_get_system_config_dirs ();
  for (i = 0; dirs[i] != NULL; ++i)
    {
      /* Build the filename, if the file exists return the path */
      path = g_build_filename (dirs[i], filename, NULL);
      if (g_path_is_absolute (path)
          && g_file_test (path, G_FILE_TEST_IS_REGULAR))
        return path;
      g_free (path);
    }

  /* Nothing found */
  return NULL;
}

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

#include <garcon/garcon-config.h>



const guint garcon_major_version = GARCON_MAJOR_VERSION;
const guint garcon_minor_version = GARCON_MINOR_VERSION;
const guint garcon_micro_version = GARCON_MICRO_VERSION;




/**
 * garcon_check_version:
 * @required_major : the required major version.
 * @required_minor : the required minor version.
 * @required_micro : the required micro version.
 *
 * Checks that the <systemitem class="library">garcon</systemitem>
 * library in use is compatible with the given version. Generally you
 * would pass in the constants #GARCON_MAJOR_VERSION,
 * #GARCON_MINOR_VERSION and #GARCON_MICRO_VERSION as the three
 * arguments to this function; that produces a check that the library
 * in use is compatible with the version of
 * <systemitem class="library">garcon</systemitem> the application was
 * compiled against.
 *
 * <example>
 * <title>Checking the runtime version of the garcon library</title>
 * <programlisting>
 * const gchar *mismatch;
 * mismatch = garcon_check_version (GARCON_VERSION_MAJOR,
 *                                  GARCON_VERSION_MINOR,
 *                                  GARCON_VERSION_MICRO);
 * if (G_UNLIKELY (mismatch != NULL))
 *   g_error ("Version mismatch: %<!---->s", mismatch);
 * </programlisting>
 * </example>
 *
 * Return value: %NULL if the library is compatible with the given version,
 *               or a string describing the version mismatch. The returned
 *               string is owned by the library and must not be freed or
 *               modified by the caller.
 *
 * Since: 0.3.1
 **/
const gchar*
garcon_check_version (guint required_major,
                      guint required_minor,
                      guint required_micro)
{
  return NULL;
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

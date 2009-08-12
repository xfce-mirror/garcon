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

#include <glib/gprintf.h>

#include <gdesktopmenu/gdesktopmenu.h>



void
print_menu (GDesktopMenu *menu, 
            const gchar  *path)
{
  GDesktopMenuDirectory *directory;
  GList                 *menus;
  GList                 *items;
  GList                 *iter;
  gchar                 *name;

  if (!g_desktop_menu_element_get_visible (G_DESKTOP_MENU_ELEMENT (menu)))
    return;

  /* Determine menu name */
  directory = g_desktop_menu_get_directory (menu);

  if (G_UNLIKELY (path == NULL))
    name = g_strdup ("");
  else
    {
      name = g_strdup_printf ("%s%s/", path, 
                              (directory == NULL 
                               ? g_desktop_menu_element_get_name (G_DESKTOP_MENU_ELEMENT (menu)) 
                               : g_desktop_menu_directory_get_name (directory)));
    }

  /* Fetch submenus */
  menus = g_desktop_menu_get_menus (menu);

  /* Print child menus */
  for (iter = menus; iter != NULL; iter = g_list_next (iter)) 
    {
      /* Only display menus which are not hidden or excluded from this environment */
      if (G_LIKELY (g_desktop_menu_element_get_visible (iter->data)))
        print_menu (G_DESKTOP_MENU (iter->data), name);
    }

  /* Free submenu list */
  g_list_free (menus);

  /* Fetch menu items */
  items = g_desktop_menu_get_elements (menu);

  /* Print menu items */
  for (iter = items; iter != NULL; iter = g_list_next (iter)) 
    {
      if (G_IS_DESKTOP_MENU_ITEM (iter->data) 
          && g_desktop_menu_element_get_visible (iter->data))
        {
          g_printf ("%s\t%s\t%s\n", name, g_desktop_menu_item_get_desktop_id (iter->data), 
                    g_desktop_menu_item_get_filename (iter->data));
        }
    }

  /* Free menu item list */
  g_list_free (items);

  /* Free name */
  g_free (name);
}



int
main (int    argc,
      char **argv)
{
  GDesktopMenu *menu;
  GError       *error = NULL;
#ifdef HAVE_STDLIB_H
  int           exit_code = EXIT_SUCCESS;
#else
  int           exit_code = 0;
#endif

  g_set_prgname ("test-menu-spec");

  /* Initialize menu library */
  g_desktop_menu_init (NULL);

  /* Try to get the root menu */
  menu = g_desktop_menu_new_applications ();

  if (G_LIKELY (g_desktop_menu_load (menu, NULL, &error)))
    {
      /* Print menu contents according to the test suite criteria */
      print_menu (menu, NULL);
    }
  else
    {
      gchar *uri;

      uri = g_file_get_uri (g_desktop_menu_get_file (menu));
      g_error ("Could not load menu from %s: %s", uri, error->message);
      g_free (uri);

      g_error_free (error);
#ifdef HAVE_STDLIB_H
      exit_code = EXIT_FAILURE;
#else
      exit_code = -1;
#endif
    }

  /* Shut down the menu library */
  g_desktop_menu_shutdown ();

  return exit_code;
}

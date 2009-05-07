/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2007-2009 Jannis Pohlmann <jannis@xfce.org>
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

#include <gdesktopmenu/gdesktopmenuenvironment.h>



/**
 * SECTION:gdesktopmenuenvironment
 * @title: Desktop Environment Configuration
 *
 * Desktop Environment Configuration
 **/



static gchar *environment = NULL;



/**
 * g_desktop_menu_set_environment:
 * @env : Name of the desktop environment for which menus will
 *        be generated (e.g. XFCE, KDE, GNOME or %NULL).
 *
 * Sets (or unsets) the desktop environment for which menus will generated.
 * Menus and menu items belonging to other desktop environments will be
 * ignored. If set to %NULL, all menu items are used.
 */
void
g_desktop_menu_set_environment (const gchar *env)
{
  if (G_LIKELY (environment != NULL))
    g_free (environment);

  environment = g_strdup (env);
}



/**
 * g_desktop_menu_get_environment:
 * 
 * Return value: Name of the desktop environment (e.g. XFCE, KDE, GNOME) 
 * which is used or %NULL.
 */
const gchar *
g_desktop_menu_get_environment (void)
{
  return environment;
}

/* $Id$ */
/* vi:set expandtab sw=2 sts=2: */
/*-
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

#include <libxfce4menu/xfce-menu-environment.h>



static gchar *environment = NULL;



/**
 * xfce_menu_set_environment:
 * @env : Name of the desktop environment for which menus will
 *        be generated (e.g. XFCE, KDE, GNOME or %NULL).
 *
 * Sets (or unsets) the desktop environment for which menus will generated.
 * Menus and menu items belonging to other desktop environments will be
 * ignored. If set to %NULL, all menu items are used.
 */
void
xfce_menu_set_environment (const gchar *env)
{
  if (G_LIKELY (environment != NULL))
    g_free (environment);

  environment = g_strdup (env);
}



/**
 * xfce_menu_get_environment:
 * 
 * Return value: Name of the desktop environment (e.g. XFCE, KDE, GNOME) 
 * which is used or %NULL.
 */
G_CONST_RETURN gchar*
xfce_menu_get_environment (void)
{
  return environment;
}

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006-2009 Jannis Pohlmann <jannis@xfce.org>
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

#include <gdesktopmenu/gdesktopmenuelement.h>



static void g_desktop_menu_element_class_init (GDesktopMenuElementIface *klass);



GType
g_desktop_menu_element_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            "GDesktopMenuElement",
                                            sizeof (GDesktopMenuElementIface),
                                            (GClassInitFunc) g_desktop_menu_element_class_init,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
    }

  return type;
}



static void
g_desktop_menu_element_class_init (GDesktopMenuElementIface *klass)
{
}



const gchar*
g_desktop_menu_element_get_name (GDesktopMenuElement *element)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ELEMENT (element), NULL);
  return (*G_DESKTOP_MENU_ELEMENT_GET_IFACE (element)->get_name) (element);
}



const gchar*
g_desktop_menu_element_get_comment (GDesktopMenuElement *element)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ELEMENT (element), NULL);
  return (*G_DESKTOP_MENU_ELEMENT_GET_IFACE (element)->get_comment) (element);
}



const gchar*
g_desktop_menu_element_get_icon_name (GDesktopMenuElement *element)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ELEMENT (element), NULL);
  return (*G_DESKTOP_MENU_ELEMENT_GET_IFACE (element)->get_icon_name) (element);
}



gboolean
g_desktop_menu_element_get_visible (GDesktopMenuElement *element)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ELEMENT (element), FALSE);
  return (*G_DESKTOP_MENU_ELEMENT_GET_IFACE (element)->get_visible) (element);
}



gboolean
g_desktop_menu_element_get_show_in_environment (GDesktopMenuElement *element)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ELEMENT (element), FALSE);
  return (*G_DESKTOP_MENU_ELEMENT_GET_IFACE (element)->get_show_in_environment) (element);
}



gboolean
g_desktop_menu_element_get_no_display (GDesktopMenuElement *element)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ELEMENT (element), FALSE);
  return (*G_DESKTOP_MENU_ELEMENT_GET_IFACE (element)->get_no_display) (element);
}


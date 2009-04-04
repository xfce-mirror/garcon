/* $Id$ */
/*-
 * vi:set sw=2 sts=2 et ai cindent:
 *
 * Copyright (c) 2006 Jannis Pohlmann <jannis@xfce.org>
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

#include <libxfce4menu/xfce-menu-element.h>



static void xfce_menu_element_class_init (XfceMenuElementIface *klass);



GType
xfce_menu_element_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfceMenuElementIface),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_element_class_init,
        NULL,
        NULL,
        0,
        0,
        NULL,
      };

      type = g_type_register_static (G_TYPE_INTERFACE, "XfceMenuElement", &info, 0);
      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
    }

  return type;
}



static void
xfce_menu_element_class_init (XfceMenuElementIface *klass)
{
}



const gchar*
xfce_menu_element_get_name (XfceMenuElement *element)
{
  g_return_val_if_fail (XFCE_IS_MENU_ELEMENT (element), NULL);
  return (*XFCE_MENU_ELEMENT_GET_IFACE (element)->get_name) (element);
}



const gchar*
xfce_menu_element_get_icon_name (XfceMenuElement *element)
{
  g_return_val_if_fail (XFCE_IS_MENU_ELEMENT (element), NULL);
  return (*XFCE_MENU_ELEMENT_GET_IFACE (element)->get_icon_name) (element);
}



gboolean
xfce_menu_element_get_visible (XfceMenuElement *element)
{
  g_return_val_if_fail (XFCE_IS_MENU_ELEMENT (element), FALSE);
  return (*XFCE_MENU_ELEMENT_GET_IFACE (element)->get_visible) (element);
}


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

#if !defined (GDESKTOPMENU_INSIDE_GDESKTOPMENU_H) && !defined (GDESKTOPMENU_COMPILATION)
#error "Only <gdesktopmenu/gdesktopmenu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __G_DESKTOP_MENU_ELEMENT_H__
#define __G_DESKTOP_MENU_ELEMENT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define G_TYPE_DESKTOP_MENU_ELEMENT           (g_desktop_menu_element_get_type ())
#define G_DESKTOP_MENU_ELEMENT(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU_ELEMENT, GDesktopMenuElement))
#define G_IS_DESKTOP_MENU_ELEMENT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU_ELEMENT))
#define G_DESKTOP_MENU_ELEMENT_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), G_TYPE_DESKTOP_MENU_ELEMENT, GDesktopMenuElementIface))

typedef struct _GDesktopMenuElement      GDesktopMenuElement;
typedef struct _GDesktopMenuElementIface GDesktopMenuElementIface;

struct _GDesktopMenuElementIface
{
  GTypeInterface __parent__;

  /* Virtual methods */
  const gchar *(*get_name)                (GDesktopMenuElement *element);
  const gchar *(*get_comment)             (GDesktopMenuElement *element);
  const gchar *(*get_icon_name)           (GDesktopMenuElement *element);
  gboolean     (*get_visible)             (GDesktopMenuElement *element);
  gboolean     (*get_show_in_environment) (GDesktopMenuElement *element);
  gboolean     (*get_no_display)          (GDesktopMenuElement *element);
};

GType        g_desktop_menu_element_get_type                (void) G_GNUC_CONST;

const gchar *g_desktop_menu_element_get_name                (GDesktopMenuElement *element);
const gchar *g_desktop_menu_element_get_comment             (GDesktopMenuElement *element);
const gchar *g_desktop_menu_element_get_icon_name           (GDesktopMenuElement *element);
gboolean     g_desktop_menu_element_get_visible             (GDesktopMenuElement *element);
gboolean     g_desktop_menu_element_get_show_in_environment (GDesktopMenuElement *element);
gboolean     g_desktop_menu_element_get_no_display          (GDesktopMenuElement *element);

G_END_DECLS

#endif /* !__G_DESKTOP_MENU_ELEMENT_H__ */

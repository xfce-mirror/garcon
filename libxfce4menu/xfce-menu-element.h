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

#if !defined (LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H) && !defined (LIBXFCE4MENU_COMPILATION)
#error "Only <libxfce4menu/libxfce4menu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __XFCE_MENU_ELEMENT_H__
#define __XFCE_MENU_ELEMENT_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _XfceMenuElement      XfceMenuElement;
typedef struct _XfceMenuElementIface XfceMenuElementIface;

#define XFCE_TYPE_MENU_ELEMENT           (xfce_menu_element_get_type ())
#define XFCE_MENU_ELEMENT(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_ELEMENT, XfceMenuElement))
#define XFCE_IS_MENU_ELEMENT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_ELEMENT))
#define XFCE_MENU_ELEMENT_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), XFCE_TYPE_MENU_ELEMENT, XfceMenuElementIface))

struct _XfceMenuElementIface
{
  GTypeInterface __parent__;

  /* Virtual methods */
  const gchar *(*get_name)      (XfceMenuElement *element);
  const gchar *(*get_icon_name) (XfceMenuElement *element);
  gboolean     (*get_visible)   (XfceMenuElement *element);
};

GType        xfce_menu_element_get_type      (void) G_GNUC_CONST;

const gchar *xfce_menu_element_get_name      (XfceMenuElement *element);
const gchar *xfce_menu_element_get_icon_name (XfceMenuElement *element);
gboolean     xfce_menu_element_get_visible   (XfceMenuElement *element);

G_END_DECLS

#endif /* !__XFCE_MENU_ELEMENT_H__ */

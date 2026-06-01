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

#if !defined(_GARCON_INSIDE_GARCON_H) && !defined(GARCON_COMPILATION)
#error "Only <garcon/garcon.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __GARCON_MENU_ELEMENT_H__
#define __GARCON_MENU_ELEMENT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GARCON_TYPE_MENU_ELEMENT (garcon_menu_element_get_type ())
G_DECLARE_INTERFACE (GarconMenuElement, garcon_menu_element, GARCON, MENU_ELEMENT, GObject)

typedef GarconMenuElementInterface GarconMenuElementIface;

struct _GarconMenuElementInterface
{
  GTypeInterface __parent__;

  /* Virtual methods */
  const gchar *(*get_name) (GarconMenuElement *element);
  const gchar *(*get_comment) (GarconMenuElement *element);
  const gchar *(*get_icon_name) (GarconMenuElement *element);
  gboolean (*get_visible) (GarconMenuElement *element);
  gboolean (*get_show_in_environment) (GarconMenuElement *element);
  gboolean (*get_no_display) (GarconMenuElement *element);
  gboolean (*equal) (GarconMenuElement *element,
                     GarconMenuElement *other);
};

const gchar *
garcon_menu_element_get_name (GarconMenuElement *element);
const gchar *
garcon_menu_element_get_comment (GarconMenuElement *element);
const gchar *
garcon_menu_element_get_icon_name (GarconMenuElement *element);
gboolean
garcon_menu_element_get_visible (GarconMenuElement *element);
gboolean
garcon_menu_element_get_show_in_environment (GarconMenuElement *element);
gboolean
garcon_menu_element_get_no_display (GarconMenuElement *element);
gboolean
garcon_menu_element_equal (GarconMenuElement *a,
                           GarconMenuElement *b);

G_END_DECLS

#endif /* !__GARCON_MENU_ELEMENT_H__ */

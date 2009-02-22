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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#if !defined (LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H) && !defined (LIBXFCE4MENU_COMPILATION)
#error "Only <libxfce4menu/libxfce4menu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __XFCE_MENU_TREE_PROVIDER_H__
#define __XFCE_MENU_TREE_PROVIDER_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS;

typedef struct _XfceMenuTreeProviderIface   XfceMenuTreeProviderIface;
typedef struct _XfceMenuTreeProvider        XfceMenuTreeProvider;

#define XFCE_TYPE_MENU_TREE_PROVIDER            (xfce_menu_tree_provider_get_type ())
#define XFCE_MENU_TREE_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_TREE_PROVIDER, XfceMenuTreeProvider))
#define XFCE_IS_MENU_TREE_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_TREE_PROVIDER))
#define XFCE_MENU_TREE_PROVIDER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), XFCE_TYPE_MENU_TREE_PROVIDER, XfceMenuTreeProviderIface))

GType       xfce_menu_tree_provider_get_type (void) G_GNUC_CONST;

GNode      *xfce_menu_tree_provider_get_tree (XfceMenuTreeProvider *provider);
GFile      *xfce_menu_tree_provider_get_file (XfceMenuTreeProvider *provider);

struct _XfceMenuTreeProviderIface
{
  GTypeInterface __parent__;

  /* Virtual methods */
  GNode *(*get_tree) (XfceMenuTreeProvider *provider);
  GFile *(*get_file) (XfceMenuTreeProvider *provider);
};

G_END_DECLS;

#endif /* !__XFCE_MENU_TREE_PROVIDER_H__ */

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

#if !defined(LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H) && !defined(LIBXFCE4MENU_COMPILATION)
#error "Only <libxfce4menu/libxfce4menu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __XFCE_MENU_LAYOUT_H__
#define __XFCE_MENU_LAYOUT_H__

#include <glib-object.h>

#include "xfce-menu-node.h"

G_BEGIN_DECLS;

typedef enum
{
  XFCE_MENU_LAYOUT_NODE_INVALID,
  XFCE_MENU_LAYOUT_NODE_FILENAME,
  XFCE_MENU_LAYOUT_NODE_MENUNAME,
  XFCE_MENU_LAYOUT_NODE_SEPARATOR,
  XFCE_MENU_LAYOUT_NODE_MERGE,
} XfceMenuLayoutNodeType;

typedef struct _XfceMenuLayoutNode    XfceMenuLayoutNode;

typedef struct _XfceMenuLayoutPrivate XfceMenuLayoutPrivate;
typedef struct _XfceMenuLayoutClass   XfceMenuLayoutClass;
typedef struct _XfceMenuLayout        XfceMenuLayout;

#define XFCE_TYPE_MENU_LAYOUT            (xfce_menu_layout_get_type())
#define XFCE_MENU_LAYOUT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_LAYOUT, XfceMenuLayout))
#define XFCE_MENU_LAYOUT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU_LAYOUT, XfceMenuLayoutClass))
#define XFCE_IS_MENU_LAYOUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_LAYOUT))
#define XFCE_IS_MENU_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_MENU_LAYOUT))
#define XFCE_MENU_LAYOUT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU_LAYOUT, XfceMenuLayoutClass))

GType                   xfce_menu_layout_get_type            (void) G_GNUC_CONST;

XfceMenuLayout         *xfce_menu_layout_new                 (void) G_GNUC_MALLOC;
void                    xfce_menu_layout_add_filename        (XfceMenuLayout          *layout,
                                                              const gchar             *filename);
void                    xfce_menu_layout_add_menuname        (XfceMenuLayout          *layout,
                                                              const gchar             *menuname);
void                    xfce_menu_layout_add_separator       (XfceMenuLayout          *layout);
void                    xfce_menu_layout_add_merge           (XfceMenuLayout          *layout,
                                                              XfceMenuLayoutMergeType  type);
GSList                 *xfce_menu_layout_get_nodes           (XfceMenuLayout          *layout);
gboolean                xfce_menu_layout_get_filename_used   (XfceMenuLayout          *layout,
                                                              const gchar             *filename);
gboolean                xfce_menu_layout_get_menuname_used   (XfceMenuLayout          *layout,
                                                              const gchar             *menuname);

XfceMenuLayoutNodeType  xfce_menu_layout_node_get_type       (XfceMenuLayoutNode       *node);
const gchar            *xfce_menu_layout_node_get_filename   (XfceMenuLayoutNode       *node);
const gchar            *xfce_menu_layout_node_get_menuname   (XfceMenuLayoutNode       *node);
XfceMenuLayoutMergeType xfce_menu_layout_node_get_merge_type (XfceMenuLayoutNode       *node);

G_END_DECLS;

#endif /* !__XFCE_MENU_LAYOUT_H__ */


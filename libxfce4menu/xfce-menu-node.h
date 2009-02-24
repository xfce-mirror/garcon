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

#ifndef __XFCE_MENU_NODE_H__
#define __XFCE_MENU_NODE_H__

#include <glib-object.h>

#include <libxfce4menu/xfce-menu-item.h>

G_BEGIN_DECLS;

/* Types for the menu nodes */
typedef enum
{
  XFCE_MENU_NODE_TYPE_INVALID,
  XFCE_MENU_NODE_TYPE_MENU,
  XFCE_MENU_NODE_TYPE_NAME,
  XFCE_MENU_NODE_TYPE_DIRECTORY,
  XFCE_MENU_NODE_TYPE_DIRECTORY_DIR,
  XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS,
  XFCE_MENU_NODE_TYPE_APP_DIR,
  XFCE_MENU_NODE_TYPE_DEFAULT_APP_DIRS,
  XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED,
  XFCE_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED,
  XFCE_MENU_NODE_TYPE_DELETED,
  XFCE_MENU_NODE_TYPE_NOT_DELETED,
  XFCE_MENU_NODE_TYPE_INCLUDE,
  XFCE_MENU_NODE_TYPE_EXCLUDE,
  XFCE_MENU_NODE_TYPE_ALL,
  XFCE_MENU_NODE_TYPE_FILENAME,
  XFCE_MENU_NODE_TYPE_CATEGORY,
  XFCE_MENU_NODE_TYPE_OR,
  XFCE_MENU_NODE_TYPE_AND,
  XFCE_MENU_NODE_TYPE_NOT,
  XFCE_MENU_NODE_TYPE_MOVE,
  XFCE_MENU_NODE_TYPE_OLD,
  XFCE_MENU_NODE_TYPE_NEW,
  XFCE_MENU_NODE_TYPE_LAYOUT,
  XFCE_MENU_NODE_TYPE_MENUNAME,
  XFCE_MENU_NODE_TYPE_SEPARATOR,
  XFCE_MENU_NODE_TYPE_MERGE,
  XFCE_MENU_NODE_TYPE_MERGE_FILE,
  XFCE_MENU_NODE_TYPE_MERGE_DIR,
  XFCE_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS,
} XfceMenuNodeType;

#define XFCE_MENU_NODE_TYPE            (xfce_menu_node_type_get_type ())

typedef enum
{
  XFCE_MENU_LAYOUT_MERGE_MENUS,
  XFCE_MENU_LAYOUT_MERGE_FILES,
  XFCE_MENU_LAYOUT_MERGE_ALL,
} XfceMenuLayoutMergeType;

typedef enum
{
  XFCE_MENU_MERGE_FILE_PATH,
  XFCE_MENU_MERGE_FILE_PARENT,
} XfceMenuMergeFileType;

typedef union  _XfceMenuNodeData    XfceMenuNodeData;
typedef struct _XfceMenuNodeClass   XfceMenuNodeClass;
typedef struct _XfceMenuNode        XfceMenuNode;

#define XFCE_TYPE_MENU_NODE            (xfce_menu_node_get_type ())
#define XFCE_MENU_NODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_NODE, XfceMenuNode))
#define XFCE_MENU_NODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU_NODE, XfceMenuNodeClass))
#define XFCE_IS_MENU_NODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_NODE))
#define XFCE_IS_MENU_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_MENU_NODE)
#define XFCE_MENU_NODE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU_NODE, XfceMenuNodeClass))

GType                   xfce_menu_node_get_type                     (void) G_GNUC_CONST;

XfceMenuNode           *xfce_menu_node_new                          (XfceMenuNodeType      node_type) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
XfceMenuNodeType        xfce_menu_node_get_node_type                (XfceMenuNode         *node);
XfceMenuNode           *xfce_menu_node_create                       (XfceMenuNodeType      node_type,
                                                                     gpointer              first_value,
                                                                     ...) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
XfceMenuNode           *xfce_menu_node_copy                         (XfceMenuNode         *node) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
const gchar            *xfce_menu_node_get_string                   (XfceMenuNode         *node);
void                    xfce_menu_node_set_string                   (XfceMenuNode         *node,
                                                                     const gchar          *string);
XfceMenuMergeFileType   xfce_menu_node_get_merge_file_type          (XfceMenuNode         *node);
void                    xfce_menu_node_set_merge_file_type          (XfceMenuNode         *node,
                                                                     XfceMenuMergeFileType type);
const gchar            *xfce_menu_node_get_merge_file_filename      (XfceMenuNode         *node);
void                    xfce_menu_node_set_merge_file_filename      (XfceMenuNode         *node,
                                                                     const gchar          *filename);

gboolean                xfce_menu_node_tree_rule_matches            (GNode                *tree,
                                                                     XfceMenuItem         *item);
XfceMenuNodeType        xfce_menu_node_tree_get_node_type           (GNode                *tree);
const gchar            *xfce_menu_node_tree_get_string              (GNode                *tree);
XfceMenuLayoutMergeType xfce_menu_node_tree_get_layout_merge_type   (GNode                *tree);
XfceMenuMergeFileType   xfce_menu_node_tree_get_merge_file_type     (GNode                *tree);
const gchar            *xfce_menu_node_tree_get_merge_file_filename (GNode                *tree);
gint                    xfce_menu_node_tree_compare                 (GNode                *tree,
                                                                     GNode                *other_tree);
GNode                  *xfce_menu_node_tree_copy                    (GNode                *tree);
void                    xfce_menu_node_tree_free                    (GNode                *tree);
void                    xfce_menu_node_tree_free_data               (GNode                *tree);


G_END_DECLS;

#endif /* !__XFCE_MENU_NODE_H__ */

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

#if !defined (GDESKTOPMENU_INSIDE_GDESKTOPMENU_H) && !defined (GDESKTOPMENU_COMPILATION)
#error "Only <gdesktopmenu/gdesktopmenu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __G_DESKTOP_MENU_NODE_H__
#define __G_DESKTOP_MENU_NODE_H__

#include <gdesktopmenu/gdesktopmenu.h>

G_BEGIN_DECLS

/* Types for the menu nodes */
typedef enum
{
  G_DESKTOP_MENU_NODE_TYPE_INVALID,
  G_DESKTOP_MENU_NODE_TYPE_MENU,
  G_DESKTOP_MENU_NODE_TYPE_NAME,
  G_DESKTOP_MENU_NODE_TYPE_DIRECTORY,
  G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR,
  G_DESKTOP_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS,
  G_DESKTOP_MENU_NODE_TYPE_APP_DIR,
  G_DESKTOP_MENU_NODE_TYPE_DEFAULT_APP_DIRS,
  G_DESKTOP_MENU_NODE_TYPE_ONLY_UNALLOCATED,
  G_DESKTOP_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED,
  G_DESKTOP_MENU_NODE_TYPE_DELETED,
  G_DESKTOP_MENU_NODE_TYPE_NOT_DELETED,
  G_DESKTOP_MENU_NODE_TYPE_INCLUDE,
  G_DESKTOP_MENU_NODE_TYPE_EXCLUDE,
  G_DESKTOP_MENU_NODE_TYPE_ALL,
  G_DESKTOP_MENU_NODE_TYPE_FILENAME,
  G_DESKTOP_MENU_NODE_TYPE_CATEGORY,
  G_DESKTOP_MENU_NODE_TYPE_OR,
  G_DESKTOP_MENU_NODE_TYPE_AND,
  G_DESKTOP_MENU_NODE_TYPE_NOT,
  G_DESKTOP_MENU_NODE_TYPE_MOVE,
  G_DESKTOP_MENU_NODE_TYPE_OLD,
  G_DESKTOP_MENU_NODE_TYPE_NEW,
  G_DESKTOP_MENU_NODE_TYPE_DEFAULT_LAYOUT,
  G_DESKTOP_MENU_NODE_TYPE_LAYOUT,
  G_DESKTOP_MENU_NODE_TYPE_MENUNAME,
  G_DESKTOP_MENU_NODE_TYPE_SEPARATOR,
  G_DESKTOP_MENU_NODE_TYPE_MERGE,
  G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE,
  G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR,
  G_DESKTOP_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS,
} GDesktopMenuNodeType;



typedef enum
{
  G_DESKTOP_MENU_LAYOUT_MERGE_MENUS,
  G_DESKTOP_MENU_LAYOUT_MERGE_FILES,
  G_DESKTOP_MENU_LAYOUT_MERGE_ALL,
} GDesktopMenuLayoutMergeType;

typedef enum
{
  G_DESKTOP_MENU_MERGE_FILE_PATH,
  G_DESKTOP_MENU_MERGE_FILE_PARENT,
} GDesktopMenuMergeFileType;



typedef union  _GDesktopMenuNodeData  GDesktopMenuNodeData;
typedef struct _GDesktopMenuNodeClass GDesktopMenuNodeClass;
typedef struct _GDesktopMenuNode      GDesktopMenuNode;

#define G_TYPE_DESKTOP_MENU_NODE            (g_desktop_menu_node_get_type ())
#define G_DESKTOP_MENU_NODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU_NODE, GDesktopMenuNode))
#define G_DESKTOP_MENU_NODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DESKTOP_MENU_NODE, GDesktopMenuNodeClass))
#define G_IS_DESKTOP_MENU_NODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU_NODE))
#define G_IS_DESKTOP_MENU_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DESKTOP_MENU_NODE)
#define G_DESKTOP_MENU_NODE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DESKTOP_MENU_NODE, GDesktopMenuNodeClass))

GType                       g_desktop_menu_node_get_type                     (void) G_GNUC_CONST;

GDesktopMenuNode           *g_desktop_menu_node_new                          (GDesktopMenuNodeType      node_type) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GDesktopMenuNodeType        g_desktop_menu_node_get_node_type                (GDesktopMenuNode         *node);
GDesktopMenuNode           *g_desktop_menu_node_create                       (GDesktopMenuNodeType      node_type,
                                                                              gpointer                  first_value,
                                                                              ...) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GDesktopMenuNode           *g_desktop_menu_node_copy                         (GDesktopMenuNode         *node) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
const gchar                *g_desktop_menu_node_get_string                   (GDesktopMenuNode         *node);
void                        g_desktop_menu_node_set_string                   (GDesktopMenuNode         *node,
                                                                              const gchar              *value);
GDesktopMenuMergeFileType   g_desktop_menu_node_get_merge_file_type          (GDesktopMenuNode         *node);
void                        g_desktop_menu_node_set_merge_file_type          (GDesktopMenuNode         *node,
                                                                              GDesktopMenuMergeFileType type);
const gchar                *g_desktop_menu_node_get_merge_file_filename      (GDesktopMenuNode         *node);
void                        g_desktop_menu_node_set_merge_file_filename      (GDesktopMenuNode         *node,
                                                                              const gchar              *filename);

GNode                      *g_desktop_menu_node_tree_get_child_node          (GNode                    *tree,
                                                                              GDesktopMenuNodeType      type,
                                                                              gboolean                  reverse);
GList                      *g_desktop_menu_node_tree_get_child_nodes         (GNode                    *tree,
                                                                              GDesktopMenuNodeType      type,
                                                                              gboolean                  reverse);
GList                      *g_desktop_menu_node_tree_get_string_children     (GNode                    *tree,
                                                                              GDesktopMenuNodeType      type,
                                                                              gboolean                  reverse);
gboolean                    g_desktop_menu_node_tree_get_boolean_child       (GNode                    *tree,
                                                                              GDesktopMenuNodeType      type);
const gchar                *g_desktop_menu_node_tree_get_string_child        (GNode                    *tree,
                                                                              GDesktopMenuNodeType      type);
gboolean                    g_desktop_menu_node_tree_rule_matches            (GNode                    *tree,
                                                                              GDesktopMenuItem         *item);
GDesktopMenuNodeType        g_desktop_menu_node_tree_get_node_type           (GNode                    *tree);
const gchar                *g_desktop_menu_node_tree_get_string              (GNode                    *tree);
void                        g_desktop_menu_node_tree_set_string              (GNode                    *tree,
                                                                              const gchar              *value);
GDesktopMenuLayoutMergeType g_desktop_menu_node_tree_get_layout_merge_type   (GNode                    *tree);
GDesktopMenuMergeFileType   g_desktop_menu_node_tree_get_merge_file_type     (GNode                    *tree);
const gchar                *g_desktop_menu_node_tree_get_merge_file_filename (GNode                    *tree);
void                        g_desktop_menu_node_tree_set_merge_file_filename (GNode                    *tree,
                                                                              const gchar              *filename);
gint                        g_desktop_menu_node_tree_compare                 (GNode                    *tree,
                                                                              GNode                    *other_tree);
GNode                      *g_desktop_menu_node_tree_copy                    (GNode                    *tree);
void                        g_desktop_menu_node_tree_free                    (GNode                    *tree);
void                        g_desktop_menu_node_tree_free_data               (GNode                    *tree);


G_END_DECLS

#endif /* !__G_DESKTOP_MENU_NODE_H__ */

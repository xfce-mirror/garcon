/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include <gdesktopmenu/gdesktopmenuitem.h>
#include <gdesktopmenu/gdesktopmenunode.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_NODE_TYPE,
};



static void g_desktop_menu_node_class_init   (GDesktopMenuNodeClass *klass);
static void g_desktop_menu_node_init         (GDesktopMenuNode      *node);
static void g_desktop_menu_node_finalize     (GObject               *object);
static void g_desktop_menu_node_get_property (GObject               *object,
                                              guint                  prop_id,
                                              GValue                *value,
                                              GParamSpec            *pspec);
static void g_desktop_menu_node_set_property (GObject               *object,
                                              guint                  prop_id,
                                              const GValue          *value,
                                              GParamSpec            *pspec);
static void g_desktop_menu_node_free_data    (GDesktopMenuNode      *node);



struct _GDesktopMenuNodeClass
{
  GObjectClass __parent__;
};

union _GDesktopMenuNodeData
{
  GDesktopMenuLayoutMergeType layout_merge_type;
  struct 
  {
    GDesktopMenuMergeFileType type;
    gchar                    *filename;
  } merge_file;
  gchar                      *string;
};

struct _GDesktopMenuNode
{
  GObject              __parent__;

  GDesktopMenuNodeType node_type;
  GDesktopMenuNodeData data;
};



static GObjectClass *g_desktop_menu_node_parent_class = NULL;



GType
g_desktop_menu_node_type_get_type (void)
{
  static GType      type = G_TYPE_INVALID;
  static GEnumValue values[] = 
  {
    { G_DESKTOP_MENU_NODE_TYPE_INVALID, "G_DESKTOP_MENU_NODE_TYPE_INVALID", "Invalid" },
    { G_DESKTOP_MENU_NODE_TYPE_MENU, "G_DESKTOP_MENU_NODE_TYPE_MENU", "Menu" },
    { G_DESKTOP_MENU_NODE_TYPE_NAME, "G_DESKTOP_MENU_NODE_TYPE_NAME", "Name" },
    { G_DESKTOP_MENU_NODE_TYPE_DIRECTORY, "G_DESKTOP_MENU_NODE_TYPE_DIRECTORY", "Directory" },
    { G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR, "G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR", "DirectoryDir" },
    { G_DESKTOP_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS, "G_DESKTOP_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS", "DefaultDirectoryDirs" },
    { G_DESKTOP_MENU_NODE_TYPE_APP_DIR, "G_DESKTOP_MENU_NODE_TYPE_APP_DIR", "AppDir" },
    { G_DESKTOP_MENU_NODE_TYPE_DEFAULT_APP_DIRS, "G_DESKTOP_MENU_NODE_TYPE_DEFAULT_APP_DIRS", "DefaultAppDirs" },
    { G_DESKTOP_MENU_NODE_TYPE_ONLY_UNALLOCATED, "G_DESKTOP_MENU_NODE_TYPE_ONLY_UNALLOCATED", "OnlyUnallocated" },
    { G_DESKTOP_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED, "G_DESKTOP_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED", "NotOnlyUnallocated" },
    { G_DESKTOP_MENU_NODE_TYPE_DELETED, "G_DESKTOP_MENU_NODE_TYPE_DELETED", "Deleted" },
    { G_DESKTOP_MENU_NODE_TYPE_NOT_DELETED, "G_DESKTOP_MENU_NODE_TYPE_NOT_DELETED", "NotDeleted" },
    { G_DESKTOP_MENU_NODE_TYPE_INCLUDE, "G_DESKTOP_MENU_NODE_TYPE_INCLUDE", "Include" },
    { G_DESKTOP_MENU_NODE_TYPE_EXCLUDE, "G_DESKTOP_MENU_NODE_TYPE_EXCLUDE", "Exclude" },
    { G_DESKTOP_MENU_NODE_TYPE_ALL, "G_DESKTOP_MENU_NODE_TYPE_ALL", "All" },
    { G_DESKTOP_MENU_NODE_TYPE_FILENAME, "G_DESKTOP_MENU_NODE_TYPE_FILENAME", "Filename" },
    { G_DESKTOP_MENU_NODE_TYPE_CATEGORY, "G_DESKTOP_MENU_NODE_TYPE_CATEGORY", "Category" },
    { G_DESKTOP_MENU_NODE_TYPE_OR, "G_DESKTOP_MENU_NODE_TYPE_OR", "Or" },
    { G_DESKTOP_MENU_NODE_TYPE_AND, "G_DESKTOP_MENU_NODE_TYPE_AND", "And" },
    { G_DESKTOP_MENU_NODE_TYPE_NOT, "G_DESKTOP_MENU_NODE_TYPE_NOT", "Not" },
    { G_DESKTOP_MENU_NODE_TYPE_MOVE, "G_DESKTOP_MENU_NODE_TYPE_MOVE", "Move" },
    { G_DESKTOP_MENU_NODE_TYPE_OLD, "G_DESKTOP_MENU_NODE_TYPE_OLD", "Old" },
    { G_DESKTOP_MENU_NODE_TYPE_NEW, "G_DESKTOP_MENU_NODE_TYPE_NEW", "New" },
    { G_DESKTOP_MENU_NODE_TYPE_DEFAULT_LAYOUT, "G_DESKTOP_MENU_NODE_TYPE_DEFAULT_LAYOUT", "DefaultLayout" },
    { G_DESKTOP_MENU_NODE_TYPE_LAYOUT, "G_DESKTOP_MENU_NODE_TYPE_LAYOUT", "Layout" },
    { G_DESKTOP_MENU_NODE_TYPE_MENUNAME, "G_DESKTOP_MENU_NODE_TYPE_MENUNAME", "Menuname" },
    { G_DESKTOP_MENU_NODE_TYPE_SEPARATOR, "G_DESKTOP_MENU_NODE_TYPE_SEPARATOR", "Separator" },
    { G_DESKTOP_MENU_NODE_TYPE_MERGE, "G_DESKTOP_MENU_NODE_TYPE_MERGE", "Merge" },
    { G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE, "G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE", "MergeFile" },
    { G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR, "G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR", "MergeDir" },
    { G_DESKTOP_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS, "G_DESKTOP_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS", "MergeDirs" },
    { 0, NULL, NULL },
  };

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    type = g_enum_register_static ("GDesktopMenuNodeType", values);

  return type;
}



GType
g_desktop_menu_node_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      type = g_type_register_static_simple (G_TYPE_OBJECT, 
                                            "GDesktopMenuNode",
                                            sizeof (GDesktopMenuNodeClass),
                                            (GClassInitFunc) g_desktop_menu_node_class_init,
                                            sizeof (GDesktopMenuNode),
                                            (GInstanceInitFunc) g_desktop_menu_node_init,
                                            0);
    }

  return type;
}



static void
g_desktop_menu_node_class_init (GDesktopMenuNodeClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine the parent type class */
  g_desktop_menu_node_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = g_desktop_menu_node_finalize; 
  gobject_class->get_property = g_desktop_menu_node_get_property;
  gobject_class->set_property = g_desktop_menu_node_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_NODE_TYPE,
                                   g_param_spec_enum ("node-type",
                                                      "node-type",
                                                      "node-type",
                                                      g_desktop_menu_node_type_get_type (),
                                                      G_DESKTOP_MENU_NODE_TYPE_MENU,
                                                      G_PARAM_READABLE | G_PARAM_WRITABLE));
}



static void
g_desktop_menu_node_init (GDesktopMenuNode *node)
{
}



static void
g_desktop_menu_node_finalize (GObject *object)
{
  GDesktopMenuNode *node = G_DESKTOP_MENU_NODE (object);

  g_desktop_menu_node_free_data (node);

  (*G_OBJECT_CLASS (g_desktop_menu_node_parent_class)->finalize) (object);
}



static void
g_desktop_menu_node_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  GDesktopMenuNode *node = G_DESKTOP_MENU_NODE (object);

  switch (prop_id)
    {
    case PROP_NODE_TYPE:
      g_value_set_enum (value, node->node_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
g_desktop_menu_node_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  GDesktopMenuNode *node = G_DESKTOP_MENU_NODE (object);

  switch (prop_id)
    {
    case PROP_NODE_TYPE:
      node->node_type = g_value_get_enum (value);
      g_object_notify (G_OBJECT (node), "node-type");
      break;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



GDesktopMenuNode *
g_desktop_menu_node_new (GDesktopMenuNodeType node_type)
{
  return g_object_new (G_TYPE_DESKTOP_MENU_NODE, "node-type", node_type, NULL);
}



GDesktopMenuNodeType g_desktop_menu_node_get_node_type (GDesktopMenuNode *node)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_NODE (node), 0);
  return node->node_type;
}



GDesktopMenuNode *
g_desktop_menu_node_create (GDesktopMenuNodeType node_type,
                            gpointer             first_value,
                            ...)
{
  GDesktopMenuNode *node;

  node = g_desktop_menu_node_new (node_type);

  switch (node_type)
    {
    case G_DESKTOP_MENU_NODE_TYPE_NAME:
    case G_DESKTOP_MENU_NODE_TYPE_DIRECTORY:
    case G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR:
    case G_DESKTOP_MENU_NODE_TYPE_APP_DIR:
    case G_DESKTOP_MENU_NODE_TYPE_FILENAME:
    case G_DESKTOP_MENU_NODE_TYPE_CATEGORY:
    case G_DESKTOP_MENU_NODE_TYPE_OLD:
    case G_DESKTOP_MENU_NODE_TYPE_NEW:
    case G_DESKTOP_MENU_NODE_TYPE_MENUNAME:
    case G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR:
      node->data.string = g_strdup (first_value);
      break;

    case G_DESKTOP_MENU_NODE_TYPE_MERGE:
      node->data.layout_merge_type = GPOINTER_TO_UINT (first_value);
      break;

    case G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE:
      node->data.merge_file.type = GPOINTER_TO_UINT (first_value);
      node->data.merge_file.filename = NULL;
      break;

    default:
      break;
    }

  return node;
}



GDesktopMenuNode *
g_desktop_menu_node_copy (GDesktopMenuNode *node)
{
  GDesktopMenuNode *copy;

  if (node == NULL || !G_IS_DESKTOP_MENU_NODE (node))
    return NULL;

  copy = g_desktop_menu_node_new (node->node_type);

  switch (copy->node_type)
    {
    case G_DESKTOP_MENU_NODE_TYPE_NAME:
    case G_DESKTOP_MENU_NODE_TYPE_DIRECTORY:
    case G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR:
    case G_DESKTOP_MENU_NODE_TYPE_APP_DIR:
    case G_DESKTOP_MENU_NODE_TYPE_FILENAME:
    case G_DESKTOP_MENU_NODE_TYPE_CATEGORY:
    case G_DESKTOP_MENU_NODE_TYPE_OLD:
    case G_DESKTOP_MENU_NODE_TYPE_NEW:
    case G_DESKTOP_MENU_NODE_TYPE_MENUNAME:
    case G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR:
      copy->data.string = g_strdup (node->data.string);
      break;

    case G_DESKTOP_MENU_NODE_TYPE_MERGE:
      copy->data.layout_merge_type = node->data.layout_merge_type;
      break;

    case G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE:
      copy->data.merge_file.type = node->data.merge_file.type;
      copy->data.merge_file.filename = g_strdup (node->data.merge_file.filename);
      break;

    default:
      break;
    }

  return copy;
}



static void
g_desktop_menu_node_free_data (GDesktopMenuNode *node)
{
  g_return_if_fail (G_IS_DESKTOP_MENU_NODE (node));

  switch (node->node_type)
    {
    case G_DESKTOP_MENU_NODE_TYPE_NAME:
    case G_DESKTOP_MENU_NODE_TYPE_DIRECTORY:
    case G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR:
    case G_DESKTOP_MENU_NODE_TYPE_APP_DIR:
    case G_DESKTOP_MENU_NODE_TYPE_FILENAME:
    case G_DESKTOP_MENU_NODE_TYPE_CATEGORY:
    case G_DESKTOP_MENU_NODE_TYPE_OLD:
    case G_DESKTOP_MENU_NODE_TYPE_NEW:
    case G_DESKTOP_MENU_NODE_TYPE_MENUNAME:
    case G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR:
      g_free (node->data.string);
      break;

    default:
      break;
    }
}



const gchar *
g_desktop_menu_node_get_string (GDesktopMenuNode *node)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_NODE (node), NULL);
  return node->data.string;
}



void
g_desktop_menu_node_set_string (GDesktopMenuNode *node,
                                const gchar      *value)
{
  g_return_if_fail (G_IS_DESKTOP_MENU_NODE (node));
  g_return_if_fail (value != NULL);

  g_free (node->data.string);
  node->data.string = g_strdup (value);
}



GDesktopMenuMergeFileType
g_desktop_menu_node_get_merge_file_type (GDesktopMenuNode *node)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_NODE (node), 0);
  g_return_val_if_fail (node->node_type == G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE, 0);
  return node->data.merge_file.type;
}



void
g_desktop_menu_node_set_merge_file_type (GDesktopMenuNode         *node,
                                         GDesktopMenuMergeFileType type)
{
  g_return_if_fail (G_IS_DESKTOP_MENU_NODE (node));
  g_return_if_fail (node->node_type == G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE);
  node->data.merge_file.type = type;
}



const gchar *
g_desktop_menu_node_get_merge_file_filename (GDesktopMenuNode *node)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_NODE (node), NULL);
  g_return_val_if_fail (node->node_type == G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE, NULL);
  return node->data.merge_file.filename;
}



void
g_desktop_menu_node_set_merge_file_filename (GDesktopMenuNode *node,
                                             const gchar      *filename)
{
  g_return_if_fail (G_IS_DESKTOP_MENU_NODE (node));
  g_return_if_fail (filename != NULL);
  g_return_if_fail (node->node_type == G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE);

  g_free (node->data.merge_file.filename);
  node->data.merge_file.filename = g_strdup (filename);
}



typedef struct
{
  GDesktopMenuNodeType type;
  GNode               *self;
  gboolean             reverse;
  gpointer             value;
} Pair;



gboolean
collect_children (GNode *node,
                  Pair  *pair)
{
  if (node == pair->self)
    return FALSE;

  if (g_desktop_menu_node_tree_get_node_type (node) == pair->type)
    {
      if (pair->reverse)
        pair->value = g_list_prepend (pair->value, node);
      else
        pair->value = g_list_append (pair->value, node);
    }

  return FALSE;
}



GNode *
g_desktop_menu_node_tree_get_child_node (GNode               *tree,
                                         GDesktopMenuNodeType type,
                                         gboolean             reverse)
{
  GNode *node = NULL;
  GNode *child;

  for (child = reverse ? g_node_last_child (tree) : g_node_first_child (tree); 
       node == NULL && child != NULL; 
       child = reverse ? g_node_prev_sibling (child) : g_node_next_sibling (child))
    {
      if (g_desktop_menu_node_tree_get_node_type (child) == type)
        node = child;
    }

  return node;
}



GList *
g_desktop_menu_node_tree_get_child_nodes (GNode               *tree,
                                          GDesktopMenuNodeType type,
                                          gboolean             reverse)
{
  Pair pair;

  pair.type = type;
  pair.reverse = reverse;
  pair.value = NULL;
  pair.self = tree;

  g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_children, &pair);

  return pair.value;
}



gboolean
collect_strings (GNode *node,
                 Pair  *pair)
{
  gpointer string;

  if (node == pair->self)
    return FALSE;

  if (g_desktop_menu_node_tree_get_node_type (node) == pair->type)
    {
      string = (gpointer) g_desktop_menu_node_tree_get_string (node);

      if (pair->reverse)
        pair->value = g_list_prepend (pair->value, string);
      else
        pair->value = g_list_append (pair->value, string);
    }

  return FALSE;
}



GList *
g_desktop_menu_node_tree_get_string_children (GNode               *tree,
                                              GDesktopMenuNodeType type,
                                              gboolean             reverse)
{
  Pair pair;

  pair.type = type;
  pair.reverse = reverse;
  pair.value = NULL;
  pair.self = tree;

  g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_strings, &pair);

  return pair.value;
}



static gboolean
collect_boolean (GNode *node,
                 Pair  *pair)
{
  if (node == pair->self)
    return FALSE;

  if (g_desktop_menu_node_tree_get_node_type (node) == pair->type)
    {
      pair->value = GUINT_TO_POINTER (1);
      return TRUE;
    }

  return FALSE;
}



gboolean 
g_desktop_menu_node_tree_get_boolean_child (GNode               *tree,
                                            GDesktopMenuNodeType type)
{
  Pair pair;

  pair.value = GUINT_TO_POINTER (0);
  pair.self = tree;
  pair.type = type;

  g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_boolean, &pair);

  return !!GPOINTER_TO_UINT (pair.value);
}



static gboolean
collect_string (GNode *node,
                Pair  *pair)
{
  const gchar **string = pair->value;

  if (node == pair->self)
    return FALSE;

  if (g_desktop_menu_node_tree_get_node_type (node) == pair->type)
    {
      *string = g_desktop_menu_node_tree_get_string (node);
      return TRUE;
    }

  return FALSE;
}



const gchar *
g_desktop_menu_node_tree_get_string_child (GNode               *tree,
                                           GDesktopMenuNodeType type)
{
  Pair         pair;
  const gchar *string = NULL;

  pair.type = type;
  pair.value = &string;
  pair.self = tree;

  g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_string, &pair);

  return string;
}



gboolean
g_desktop_menu_node_tree_rule_matches (GNode            *node,
                                       GDesktopMenuItem *item)
{
  GNode   *child;
  gboolean matches = FALSE;
  gboolean child_matches = FALSE;

  switch (g_desktop_menu_node_tree_get_node_type (node))
    {
    case G_DESKTOP_MENU_NODE_TYPE_INCLUDE:
    case G_DESKTOP_MENU_NODE_TYPE_EXCLUDE:
    case G_DESKTOP_MENU_NODE_TYPE_OR:
      for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
        matches = matches || g_desktop_menu_node_tree_rule_matches (child, item);
      break;

    case G_DESKTOP_MENU_NODE_TYPE_AND:
      matches = TRUE;
      for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
        matches = matches && g_desktop_menu_node_tree_rule_matches (child, item);
      break;

    case G_DESKTOP_MENU_NODE_TYPE_NOT:
      for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
        child_matches = child_matches || g_desktop_menu_node_tree_rule_matches (child, item);
      matches = !child_matches;
      break;

    case G_DESKTOP_MENU_NODE_TYPE_FILENAME:
      matches = g_str_equal (g_desktop_menu_node_tree_get_string (node),
                             g_desktop_menu_item_get_desktop_id (item));
      break;

    case G_DESKTOP_MENU_NODE_TYPE_CATEGORY:
      matches = g_desktop_menu_item_has_category (item, g_desktop_menu_node_tree_get_string (node));
      break;

    case G_DESKTOP_MENU_NODE_TYPE_ALL:
      matches = TRUE;
      break;

    default:
      break;
    }

  return matches;
}






GDesktopMenuNodeType 
g_desktop_menu_node_tree_get_node_type (GNode *tree)
{
  if (tree == NULL)
    return G_DESKTOP_MENU_NODE_TYPE_INVALID;

  if (tree->data == NULL)
    return G_DESKTOP_MENU_NODE_TYPE_MENU;

  return g_desktop_menu_node_get_node_type (tree->data);
}



const gchar * 
g_desktop_menu_node_tree_get_string (GNode *tree)
{
  if (tree == NULL || tree->data == NULL)
    return NULL;
  else
    return g_desktop_menu_node_get_string (tree->data);
}



void
g_desktop_menu_node_tree_set_string (GNode       *tree,
                                     const gchar *value)
{
  GDesktopMenuNodeType type;

  type = g_desktop_menu_node_tree_get_node_type (tree);

  g_return_if_fail (type == G_DESKTOP_MENU_NODE_TYPE_NAME ||
                    type == G_DESKTOP_MENU_NODE_TYPE_DIRECTORY ||
                    type == G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR ||
                    type == G_DESKTOP_MENU_NODE_TYPE_APP_DIR ||
                    type == G_DESKTOP_MENU_NODE_TYPE_FILENAME ||
                    type == G_DESKTOP_MENU_NODE_TYPE_CATEGORY ||
                    type == G_DESKTOP_MENU_NODE_TYPE_OLD ||
                    type == G_DESKTOP_MENU_NODE_TYPE_NEW ||
                    type == G_DESKTOP_MENU_NODE_TYPE_MENUNAME ||
                    type == G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR);

  g_desktop_menu_node_set_string (tree->data, value);
}


GDesktopMenuLayoutMergeType
g_desktop_menu_node_tree_get_layout_merge_type (GNode *tree)
{
  g_return_val_if_fail (g_desktop_menu_node_tree_get_node_type (tree) == G_DESKTOP_MENU_NODE_TYPE_MERGE, 0);
  return ((GDesktopMenuNode *)tree->data)->data.layout_merge_type;
}



GDesktopMenuMergeFileType 
g_desktop_menu_node_tree_get_merge_file_type (GNode *tree)
{
  g_return_val_if_fail (g_desktop_menu_node_tree_get_node_type (tree) == G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE, 0);
  return g_desktop_menu_node_get_merge_file_type (tree->data);
}



const gchar *
g_desktop_menu_node_tree_get_merge_file_filename (GNode *tree)
{
  g_return_val_if_fail (g_desktop_menu_node_tree_get_node_type (tree) == G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE, NULL);
  return g_desktop_menu_node_get_merge_file_filename (tree->data);
}



void
g_desktop_menu_node_tree_set_merge_file_filename (GNode       *tree,
                                                  const gchar *filename)
{
  g_return_if_fail (g_desktop_menu_node_tree_get_node_type (tree) == G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE);
  g_desktop_menu_node_set_merge_file_filename (tree->data, filename);
}



gint
g_desktop_menu_node_tree_compare (GNode *tree,
                                  GNode *other_tree)
{
  GDesktopMenuNode *node;
  GDesktopMenuNode *other_node;

  if (tree == NULL || other_tree == NULL)
    return 0;

  node = tree->data;
  other_node = other_tree->data;

  if (node->node_type != other_node->node_type)
    return 0;

  switch (node->node_type)
    {
    case G_DESKTOP_MENU_NODE_TYPE_NAME:
    case G_DESKTOP_MENU_NODE_TYPE_DIRECTORY:
    case G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR:
    case G_DESKTOP_MENU_NODE_TYPE_APP_DIR:
    case G_DESKTOP_MENU_NODE_TYPE_FILENAME:
    case G_DESKTOP_MENU_NODE_TYPE_CATEGORY:
    case G_DESKTOP_MENU_NODE_TYPE_OLD:
    case G_DESKTOP_MENU_NODE_TYPE_NEW:
    case G_DESKTOP_MENU_NODE_TYPE_MENUNAME:
    case G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR:
      return g_utf8_collate (node->data.string, other_node->data.string);
      break;

    case G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE:
      return g_utf8_collate (node->data.merge_file.filename, 
                             other_node->data.merge_file.filename);
      break;

    default:
      return 0;
      break;
    }

  return 0;
}



GNode *
g_desktop_menu_node_tree_copy (GNode *tree)
{
  return g_node_copy_deep (tree, (GCopyFunc) g_desktop_menu_node_copy, NULL);
}



void
g_desktop_menu_node_tree_free (GNode *tree)
{
  if (tree != NULL)
    {
      g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                       (GNodeTraverseFunc) g_desktop_menu_node_tree_free_data, NULL);

      g_node_destroy (tree);
    }
}



void
g_desktop_menu_node_tree_free_data (GNode *tree)
{
  if (tree != NULL && tree->data != NULL)
    g_object_unref (tree->data);
}

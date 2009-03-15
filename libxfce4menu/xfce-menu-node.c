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

#include <libxfce4menu/xfce-menu-item.h>
#include <libxfce4menu/xfce-menu-node.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_NODE_TYPE,
};



static void xfce_menu_node_class_init   (XfceMenuNodeClass *klass);
static void xfce_menu_node_init         (XfceMenuNode      *node);
static void xfce_menu_node_finalize     (GObject           *object);
static void xfce_menu_node_get_property (GObject           *object,
                                         guint              prop_id,
                                         GValue            *value,
                                         GParamSpec        *pspec);
static void xfce_menu_node_set_property (GObject           *object,
                                         guint              prop_id,
                                         const GValue      *value,
                                         GParamSpec        *pspec);
static void xfce_menu_node_free_data    (XfceMenuNode      *node);



struct _XfceMenuNodeClass
{
  GObjectClass __parent__;
};

union _XfceMenuNodeData
{
  XfceMenuLayoutMergeType layout_merge_type;
  struct 
  {
    XfceMenuMergeFileType type;
    gchar                *filename;
  } merge_file;
  gchar                  *string;
};

struct _XfceMenuNode
{
  GObject          __parent__;

  XfceMenuNodeType node_type;
  XfceMenuNodeData data;
};



static GObjectClass *xfce_menu_node_parent_class = NULL;



GType
xfce_menu_node_type_get_type (void)
{
  static GType      type = G_TYPE_INVALID;
  static GEnumValue values[] = 
  {
    { XFCE_MENU_NODE_TYPE_INVALID, "XFCE_MENU_NODE_TYPE_INVALID", "Invalid" },
    { XFCE_MENU_NODE_TYPE_MENU, "XFCE_MENU_NODE_TYPE_MENU", "Menu" },
    { XFCE_MENU_NODE_TYPE_NAME, "XFCE_MENU_NODE_TYPE_NAME", "Name" },
    { XFCE_MENU_NODE_TYPE_DIRECTORY, "XFCE_MENU_NODE_TYPE_DIRECTORY", "Directory" },
    { XFCE_MENU_NODE_TYPE_DIRECTORY_DIR, "XFCE_MENU_NODE_TYPE_DIRECTORY_DIR", "DirectoryDir" },
    { XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS, "XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS", "DefaultDirectoryDirs" },
    { XFCE_MENU_NODE_TYPE_APP_DIR, "XFCE_MENU_NODE_TYPE_APP_DIR", "AppDir" },
    { XFCE_MENU_NODE_TYPE_DEFAULT_APP_DIRS, "XFCE_MENU_NODE_TYPE_DEFAULT_APP_DIRS", "DefaultAppDirs" },
    { XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED, "XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED", "OnlyUnallocated" },
    { XFCE_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED, "XFCE_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED", "NotOnlyUnallocated" },
    { XFCE_MENU_NODE_TYPE_DELETED, "XFCE_MENU_NODE_TYPE_DELETED", "Deleted" },
    { XFCE_MENU_NODE_TYPE_NOT_DELETED, "XFCE_MENU_NODE_TYPE_NOT_DELETED", "NotDeleted" },
    { XFCE_MENU_NODE_TYPE_INCLUDE, "XFCE_MENU_NODE_TYPE_INCLUDE", "Include" },
    { XFCE_MENU_NODE_TYPE_EXCLUDE, "XFCE_MENU_NODE_TYPE_EXCLUDE", "Exclude" },
    { XFCE_MENU_NODE_TYPE_ALL, "XFCE_MENU_NODE_TYPE_ALL", "All" },
    { XFCE_MENU_NODE_TYPE_FILENAME, "XFCE_MENU_NODE_TYPE_FILENAME", "Filename" },
    { XFCE_MENU_NODE_TYPE_CATEGORY, "XFCE_MENU_NODE_TYPE_CATEGORY", "Category" },
    { XFCE_MENU_NODE_TYPE_OR, "XFCE_MENU_NODE_TYPE_OR", "Or" },
    { XFCE_MENU_NODE_TYPE_AND, "XFCE_MENU_NODE_TYPE_AND", "And" },
    { XFCE_MENU_NODE_TYPE_NOT, "XFCE_MENU_NODE_TYPE_NOT", "Not" },
    { XFCE_MENU_NODE_TYPE_MOVE, "XFCE_MENU_NODE_TYPE_MOVE", "Move" },
    { XFCE_MENU_NODE_TYPE_OLD, "XFCE_MENU_NODE_TYPE_OLD", "Old" },
    { XFCE_MENU_NODE_TYPE_NEW, "XFCE_MENU_NODE_TYPE_NEW", "New" },
    { XFCE_MENU_NODE_TYPE_LAYOUT, "XFCE_MENU_NODE_TYPE_LAYOUT", "Layout" },
    { XFCE_MENU_NODE_TYPE_MENUNAME, "XFCE_MENU_NODE_TYPE_MENUNAME", "Menuname" },
    { XFCE_MENU_NODE_TYPE_SEPARATOR, "XFCE_MENU_NODE_TYPE_SEPARATOR", "Separator" },
    { XFCE_MENU_NODE_TYPE_MERGE, "XFCE_MENU_NODE_TYPE_MERGE", "Merge" },
    { XFCE_MENU_NODE_TYPE_MERGE_FILE, "XFCE_MENU_NODE_TYPE_MERGE_FILE", "MergeFile" },
    { XFCE_MENU_NODE_TYPE_MERGE_DIR, "XFCE_MENU_NODE_TYPE_MERGE_DIR", "MergeDir" },
    { XFCE_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS, "XFCE_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS", "MergeDirs" },
    { 0, NULL, NULL },
  };

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    type = g_enum_register_static ("XfceMenuNodeType", values);

  return type;
}



GType
xfce_menu_node_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      type = g_type_register_static_simple (G_TYPE_OBJECT, 
                                            "XfceMenuNode",
                                            sizeof (XfceMenuNodeClass),
                                            (GClassInitFunc) xfce_menu_node_class_init,
                                            sizeof (XfceMenuNode),
                                            (GInstanceInitFunc) xfce_menu_node_init,
                                            0);
    }

  return type;
}



static void
xfce_menu_node_class_init (XfceMenuNodeClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine the parent type class */
  xfce_menu_node_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_node_finalize; 
  gobject_class->get_property = xfce_menu_node_get_property;
  gobject_class->set_property = xfce_menu_node_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_NODE_TYPE,
                                   g_param_spec_enum ("node-type",
                                                      "node-type",
                                                      "node-type",
                                                      xfce_menu_node_type_get_type (),
                                                      XFCE_MENU_NODE_TYPE_MENU,
                                                      G_PARAM_READABLE | G_PARAM_WRITABLE));
}



static void
xfce_menu_node_init (XfceMenuNode *node)
{
}



static void
xfce_menu_node_finalize (GObject *object)
{
  XfceMenuNode *node = XFCE_MENU_NODE (object);

  xfce_menu_node_free_data (node);

  (*G_OBJECT_CLASS (xfce_menu_node_parent_class)->finalize) (object);
}



static void
xfce_menu_node_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  XfceMenuNode *node = XFCE_MENU_NODE (object);

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
xfce_menu_node_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  XfceMenuNode *node = XFCE_MENU_NODE (object);

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



XfceMenuNode *
xfce_menu_node_new (XfceMenuNodeType node_type)
{
  return g_object_new (XFCE_TYPE_MENU_NODE, "node-type", node_type, NULL);
}



XfceMenuNodeType xfce_menu_node_get_node_type (XfceMenuNode *node)
{
  g_return_val_if_fail (XFCE_IS_MENU_NODE (node), 0);
  return node->node_type;
}



XfceMenuNode *
xfce_menu_node_create (XfceMenuNodeType node_type,
                       gpointer         first_value,
                       ...)
{
  XfceMenuNode *node;

  node = xfce_menu_node_new (node_type);

  switch (node_type)
    {
    case XFCE_MENU_NODE_TYPE_NAME:
    case XFCE_MENU_NODE_TYPE_DIRECTORY:
    case XFCE_MENU_NODE_TYPE_DIRECTORY_DIR:
    case XFCE_MENU_NODE_TYPE_APP_DIR:
    case XFCE_MENU_NODE_TYPE_FILENAME:
    case XFCE_MENU_NODE_TYPE_CATEGORY:
    case XFCE_MENU_NODE_TYPE_OLD:
    case XFCE_MENU_NODE_TYPE_NEW:
    case XFCE_MENU_NODE_TYPE_MENUNAME:
    case XFCE_MENU_NODE_TYPE_MERGE_DIR:
      node->data.string = g_strdup (first_value);
      break;

    case XFCE_MENU_NODE_TYPE_MERGE:
      node->data.layout_merge_type = GPOINTER_TO_UINT (first_value);
      break;

    case XFCE_MENU_NODE_TYPE_MERGE_FILE:
      node->data.merge_file.type = GPOINTER_TO_UINT (first_value);
      node->data.merge_file.filename = NULL;
      break;

    default:
      break;
    }

  return node;
}



XfceMenuNode *
xfce_menu_node_copy (XfceMenuNode *node)
{
  XfceMenuNode *copy;

  if (node == NULL || !XFCE_IS_MENU_NODE (node))
    return NULL;

  copy = xfce_menu_node_new (node->node_type);

  switch (copy->node_type)
    {
    case XFCE_MENU_NODE_TYPE_NAME:
    case XFCE_MENU_NODE_TYPE_DIRECTORY:
    case XFCE_MENU_NODE_TYPE_DIRECTORY_DIR:
    case XFCE_MENU_NODE_TYPE_APP_DIR:
    case XFCE_MENU_NODE_TYPE_FILENAME:
    case XFCE_MENU_NODE_TYPE_CATEGORY:
    case XFCE_MENU_NODE_TYPE_OLD:
    case XFCE_MENU_NODE_TYPE_NEW:
    case XFCE_MENU_NODE_TYPE_MENUNAME:
    case XFCE_MENU_NODE_TYPE_MERGE_DIR:
      copy->data.string = g_strdup (node->data.string);
      break;

    case XFCE_MENU_NODE_TYPE_MERGE:
      copy->data.layout_merge_type = node->data.layout_merge_type;
      break;

    case XFCE_MENU_NODE_TYPE_MERGE_FILE:
      copy->data.merge_file.type = node->data.merge_file.type;
      copy->data.merge_file.filename = g_strdup (node->data.merge_file.filename);
      break;

    default:
      break;
    }

  return copy;
}



static void
xfce_menu_node_free_data (XfceMenuNode *node)
{
  g_return_if_fail (XFCE_IS_MENU_NODE (node));

  switch (node->node_type)
    {
    case XFCE_MENU_NODE_TYPE_NAME:
    case XFCE_MENU_NODE_TYPE_DIRECTORY:
    case XFCE_MENU_NODE_TYPE_DIRECTORY_DIR:
    case XFCE_MENU_NODE_TYPE_APP_DIR:
    case XFCE_MENU_NODE_TYPE_FILENAME:
    case XFCE_MENU_NODE_TYPE_CATEGORY:
    case XFCE_MENU_NODE_TYPE_OLD:
    case XFCE_MENU_NODE_TYPE_NEW:
    case XFCE_MENU_NODE_TYPE_MENUNAME:
    case XFCE_MENU_NODE_TYPE_MERGE_DIR:
      g_free (node->data.string);
      break;

    default:
      break;
    }
}



const gchar *
xfce_menu_node_get_string (XfceMenuNode *node)
{
  g_return_val_if_fail (XFCE_IS_MENU_NODE (node), NULL);
  return node->data.string;
}



void
xfce_menu_node_set_string (XfceMenuNode *node,
                           const gchar  *value)
{
  g_return_if_fail (XFCE_IS_MENU_NODE (node));
  g_return_if_fail (value != NULL);

  g_free (node->data.string);
  node->data.string = g_strdup (value);
}



XfceMenuMergeFileType
xfce_menu_node_get_merge_file_type (XfceMenuNode *node)
{
  g_return_val_if_fail (XFCE_IS_MENU_NODE (node), 0);
  g_return_val_if_fail (node->node_type == XFCE_MENU_NODE_TYPE_MERGE_FILE, 0);
  return node->data.merge_file.type;
}



void
xfce_menu_node_set_merge_file_type (XfceMenuNode         *node,
                                    XfceMenuMergeFileType type)
{
  g_return_if_fail (XFCE_IS_MENU_NODE (node));
  g_return_if_fail (node->node_type == XFCE_MENU_NODE_TYPE_MERGE_FILE);
  node->data.merge_file.type = type;
}



const gchar *
xfce_menu_node_get_merge_file_filename (XfceMenuNode *node)
{
  g_return_val_if_fail (XFCE_IS_MENU_NODE (node), NULL);
  g_return_val_if_fail (node->node_type == XFCE_MENU_NODE_TYPE_MERGE_FILE, NULL);
  return node->data.merge_file.filename;
}



void
xfce_menu_node_set_merge_file_filename (XfceMenuNode *node,
                                        const gchar  *filename)
{
  g_return_if_fail (XFCE_IS_MENU_NODE (node));
  g_return_if_fail (filename != NULL);
  g_return_if_fail (node->node_type == XFCE_MENU_NODE_TYPE_MERGE_FILE);

  g_free (node->data.merge_file.filename);
  node->data.merge_file.filename = g_strdup (filename);
}



typedef struct
{
  XfceMenuNodeType type;
  GNode           *self;
  gboolean         reverse;
  gpointer         value;
} Pair;



gboolean
collect_children (GNode *node,
                  Pair  *pair)
{
  if (node == pair->self)
    return FALSE;

  if (xfce_menu_node_tree_get_node_type (node) == pair->type)
    {
      if (pair->reverse)
        pair->value = g_list_prepend (pair->value, node);
      else
        pair->value = g_list_append (pair->value, node);
    }

  return FALSE;
}



GList *
xfce_menu_node_tree_get_child_nodes (GNode            *tree,
                                     XfceMenuNodeType  type,
                                     gboolean          reverse)
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

  if (xfce_menu_node_tree_get_node_type (node) == pair->type)
    {
      string = (gpointer) xfce_menu_node_tree_get_string (node);

      if (pair->reverse)
        pair->value = g_list_prepend (pair->value, string);
      else
        pair->value = g_list_append (pair->value, string);
    }

  return FALSE;
}



GList *
xfce_menu_node_tree_get_string_children (GNode            *tree,
                                         XfceMenuNodeType  type,
                                         gboolean          reverse)
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

  if (xfce_menu_node_tree_get_node_type (node) == pair->type)
    {
      pair->value = GUINT_TO_POINTER (1);
      return TRUE;
    }

  return FALSE;
}



gboolean 
xfce_menu_node_tree_get_boolean_child (GNode            *tree,
                                       XfceMenuNodeType  type)
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

  if (xfce_menu_node_tree_get_node_type (node) == pair->type)
    {
      *string = xfce_menu_node_tree_get_string (node);
      return TRUE;
    }

  return FALSE;
}



const gchar *
xfce_menu_node_tree_get_string_child (GNode            *tree,
                                      XfceMenuNodeType  type)
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
xfce_menu_node_tree_rule_matches (GNode        *node,
                                  XfceMenuItem *item)
{
  GNode   *child;
  gboolean matches = FALSE;
  gboolean child_matches = FALSE;

  switch (xfce_menu_node_tree_get_node_type (node))
    {
    case XFCE_MENU_NODE_TYPE_INCLUDE:
    case XFCE_MENU_NODE_TYPE_EXCLUDE:
    case XFCE_MENU_NODE_TYPE_OR:
      for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
        matches = matches || xfce_menu_node_tree_rule_matches (child, item);
      break;

    case XFCE_MENU_NODE_TYPE_AND:
      matches = TRUE;
      for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
        matches = matches && xfce_menu_node_tree_rule_matches (child, item);
      break;

    case XFCE_MENU_NODE_TYPE_NOT:
      for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
        child_matches = child_matches || xfce_menu_node_tree_rule_matches (child, item);
      matches = !child_matches;
      break;

    case XFCE_MENU_NODE_TYPE_FILENAME:
      matches = g_str_equal (xfce_menu_node_tree_get_string (node),
                             xfce_menu_item_get_desktop_id (item));
      break;

    case XFCE_MENU_NODE_TYPE_CATEGORY:
      matches = xfce_menu_item_has_category (item, xfce_menu_node_tree_get_string (node));
      break;

    case XFCE_MENU_NODE_TYPE_ALL:
      matches = TRUE;
      break;

    default:
      break;
    }

  return matches;
}






XfceMenuNodeType 
xfce_menu_node_tree_get_node_type (GNode *tree)
{
  if (tree == NULL)
    return XFCE_MENU_NODE_TYPE_INVALID;

  if (tree->data == NULL)
    return XFCE_MENU_NODE_TYPE_MENU;

  return xfce_menu_node_get_node_type (tree->data);
}



const gchar * 
xfce_menu_node_tree_get_string (GNode *tree)
{
  if (tree == NULL || tree->data == NULL)
    return NULL;
  else
    return xfce_menu_node_get_string (tree->data);
}



void
xfce_menu_node_tree_set_string (GNode       *tree,
                                const gchar *value)
{
  XfceMenuNodeType type;

  type = xfce_menu_node_tree_get_node_type (tree);

  g_return_if_fail (type == XFCE_MENU_NODE_TYPE_NAME ||
                    type == XFCE_MENU_NODE_TYPE_DIRECTORY ||
                    type == XFCE_MENU_NODE_TYPE_DIRECTORY_DIR ||
                    type == XFCE_MENU_NODE_TYPE_APP_DIR ||
                    type == XFCE_MENU_NODE_TYPE_FILENAME ||
                    type == XFCE_MENU_NODE_TYPE_CATEGORY ||
                    type == XFCE_MENU_NODE_TYPE_OLD ||
                    type == XFCE_MENU_NODE_TYPE_NEW ||
                    type == XFCE_MENU_NODE_TYPE_MENUNAME ||
                    type == XFCE_MENU_NODE_TYPE_MERGE_DIR);

  xfce_menu_node_set_string (tree->data, value);
}


XfceMenuLayoutMergeType
xfce_menu_node_tree_get_layout_merge_type (GNode *tree)
{
  g_return_val_if_fail (xfce_menu_node_tree_get_node_type (tree) != XFCE_MENU_NODE_TYPE_MERGE, 0);
  return ((XfceMenuNode *)tree->data)->data.layout_merge_type;
}



XfceMenuMergeFileType 
xfce_menu_node_tree_get_merge_file_type (GNode *tree)
{
  g_return_val_if_fail (xfce_menu_node_tree_get_node_type (tree) == XFCE_MENU_NODE_TYPE_MERGE_FILE, 0);
  return xfce_menu_node_get_merge_file_type (tree->data);
}



const gchar *
xfce_menu_node_tree_get_merge_file_filename (GNode *tree)
{
  g_return_val_if_fail (xfce_menu_node_tree_get_node_type (tree) == XFCE_MENU_NODE_TYPE_MERGE_FILE, NULL);
  return xfce_menu_node_get_merge_file_filename (tree->data);
}



void
xfce_menu_node_tree_set_merge_file_filename (GNode       *tree,
                                             const gchar *filename)
{
  g_return_if_fail (xfce_menu_node_tree_get_node_type (tree) == XFCE_MENU_NODE_TYPE_MERGE_FILE);
  xfce_menu_node_set_merge_file_filename (tree->data, filename);
}



gint
xfce_menu_node_tree_compare (GNode *tree,
                             GNode *other_tree)
{
  XfceMenuNode *node;
  XfceMenuNode *other_node;

  if (tree == NULL || other_tree == NULL)
    return 0;

  node = tree->data;
  other_node = other_tree->data;

  if (node->node_type != other_node->node_type)
    return 0;

  switch (node->node_type)
    {
    case XFCE_MENU_NODE_TYPE_NAME:
    case XFCE_MENU_NODE_TYPE_DIRECTORY:
    case XFCE_MENU_NODE_TYPE_DIRECTORY_DIR:
    case XFCE_MENU_NODE_TYPE_APP_DIR:
    case XFCE_MENU_NODE_TYPE_FILENAME:
    case XFCE_MENU_NODE_TYPE_CATEGORY:
    case XFCE_MENU_NODE_TYPE_OLD:
    case XFCE_MENU_NODE_TYPE_NEW:
    case XFCE_MENU_NODE_TYPE_MENUNAME:
    case XFCE_MENU_NODE_TYPE_MERGE_DIR:
      return g_utf8_collate (node->data.string, other_node->data.string);
      break;

    case XFCE_MENU_NODE_TYPE_MERGE_FILE:
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
xfce_menu_node_tree_copy (GNode *tree)
{
  return g_node_copy_deep (tree, (GCopyFunc) xfce_menu_node_copy, NULL);
}



void
xfce_menu_node_tree_free (GNode *tree)
{
  if (tree != NULL)
    {
      g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                       (GNodeTraverseFunc) xfce_menu_node_tree_free_data, NULL);

      g_node_destroy (tree);
    }
}



void
xfce_menu_node_tree_free_data (GNode *tree)
{
  if (tree != NULL && tree->data != NULL)
    g_object_unref (tree->data);
}

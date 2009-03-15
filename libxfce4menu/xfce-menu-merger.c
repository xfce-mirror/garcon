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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include <libxfce4menu/xfce-menu-node.h>
#include <libxfce4menu/xfce-menu-tree-provider.h>
#include <libxfce4menu/xfce-menu-parser.h>
#include <libxfce4menu/xfce-menu-merger.h>
#include <libxfce4menu/xfce-menu-gio.h>



typedef struct _XfceMenuMergerContext XfceMenuMergerContext;

struct _XfceMenuMergerContext
{
  XfceMenuNodeType node_type;
  XfceMenuMerger  *merger;
  GCancellable    *cancellable;
  GError         **error;
  gboolean         success;
  GList           *file_stack;
};



#define XFCE_MENU_MERGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), XFCE_TYPE_MENU_MERGER, XfceMenuMergerPrivate))



/* Property identifiers */
enum
{
  PROP_0,
  PROP_TREE_PROVIDER,
};



static void     xfce_menu_merger_class_init                 (XfceMenuMergerClass       *klass);
static void     xfce_menu_merger_provider_init              (XfceMenuTreeProviderIface *iface);
static void     xfce_menu_merger_init                       (XfceMenuMerger            *merger);
static void     xfce_menu_merger_constructed                (GObject                   *object);
static void     xfce_menu_merger_finalize                   (GObject                   *object);
static void     xfce_menu_merger_get_property               (GObject                   *object,
                                                             guint                      prop_id,
                                                             GValue                    *value,
                                                             GParamSpec                *pspec);
static void     xfce_menu_merger_set_property               (GObject                   *object,
                                                             guint                      prop_id,
                                                             const GValue              *value,
                                                             GParamSpec                *pspec);
static GNode   *xfce_menu_merger_get_tree                   (XfceMenuTreeProvider      *provider);
static GFile   *xfce_menu_merger_get_file                   (XfceMenuTreeProvider      *provider);
static gboolean xfce_menu_merger_resolve_default_dirs       (GNode                     *node,
                                                             XfceMenuMergerContext     *context);
static gboolean xfce_menu_merger_resolve_relative_paths     (GNode                     *node,
                                                             XfceMenuMergerContext     *context);
static void     xfce_menu_merger_remove_duplicate_paths     (GNode                     *node,
                                                             XfceMenuNodeType           type);
static void     xfce_menu_merger_consolidate_child_menus    (GNode                     *node);
static gboolean xfce_menu_merger_resolve_merge_dirs         (GNode                     *node,
                                                             XfceMenuMergerContext     *context);
static gboolean xfce_menu_merger_process_merge_files        (GNode                     *node,
                                                             XfceMenuMergerContext     *context);
static void     xfce_menu_merger_clean_up_elements          (GNode                     *node,
                                                             XfceMenuNodeType           type);
static void     xfce_menu_merger_resolve_moves              (GNode                     *node);



struct _XfceMenuMergerPrivate
{
  XfceMenuTreeProvider *tree_provider;
  GNode                *menu;
  GList                *file_stack;
};



static GObjectClass *xfce_menu_merger_parent_class = NULL;



GType
xfce_menu_merger_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GInterfaceInfo provider_info = 
      {
        (GInterfaceInitFunc) xfce_menu_merger_provider_init,
        NULL,
        NULL,
      };

      type = g_type_register_static_simple (G_TYPE_OBJECT, 
                                            "XfceMenuMerger",
                                            sizeof (XfceMenuMergerClass),
                                            (GClassInitFunc) xfce_menu_merger_class_init,
                                            sizeof (XfceMenuMerger),
                                            (GInstanceInitFunc) xfce_menu_merger_init,
                                            0);

      g_type_add_interface_static (type, XFCE_TYPE_MENU_TREE_PROVIDER, &provider_info);
    }

  return type;
}



static void
xfce_menu_merger_class_init (XfceMenuMergerClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (XfceMenuMergerPrivate));

  /* Determine the parent type class */
  xfce_menu_merger_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_merger_finalize; 
  gobject_class->constructed = xfce_menu_merger_constructed;
  gobject_class->get_property = xfce_menu_merger_get_property;
  gobject_class->set_property = xfce_menu_merger_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_TREE_PROVIDER,
                                   g_param_spec_object ("tree-provider",
                                                        "tree-provider",
                                                        "tree-provider",
                                                        XFCE_TYPE_MENU_TREE_PROVIDER,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}



static void 
xfce_menu_merger_provider_init (XfceMenuTreeProviderIface *iface)
{
  iface->get_tree = xfce_menu_merger_get_tree;
  iface->get_file = xfce_menu_merger_get_file;
}



static void
xfce_menu_merger_init (XfceMenuMerger *merger)
{
  merger->priv = XFCE_MENU_MERGER_GET_PRIVATE (merger);
  merger->priv->tree_provider = NULL;
  merger->priv->menu = NULL;
  merger->priv->file_stack = NULL;
}



static void
xfce_menu_merger_constructed (GObject *object)
{
  XfceMenuMerger *merger = XFCE_MENU_MERGER (object);

  merger->priv->menu = xfce_menu_tree_provider_get_tree (merger->priv->tree_provider);
}



static void
xfce_menu_merger_finalize (GObject *object)
{
  XfceMenuMerger *merger = XFCE_MENU_MERGER (object);

  xfce_menu_node_tree_free (merger->priv->menu);

  g_object_unref (merger->priv->tree_provider);

  (*G_OBJECT_CLASS (xfce_menu_merger_parent_class)->finalize) (object);
}



static void
xfce_menu_merger_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  XfceMenuMerger *merger = XFCE_MENU_MERGER (object);

  switch (prop_id)
    {
    case PROP_TREE_PROVIDER:
      g_value_set_object (value, g_object_ref (merger->priv->tree_provider));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_menu_merger_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  XfceMenuMerger *merger = XFCE_MENU_MERGER (object);

  switch (prop_id)
    {
    case PROP_TREE_PROVIDER:
      merger->priv->tree_provider = g_object_ref (g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



XfceMenuMerger *
xfce_menu_merger_new (XfceMenuTreeProvider *provider)
{
  g_return_val_if_fail (XFCE_IS_MENU_TREE_PROVIDER (provider), NULL);
  return g_object_new (XFCE_TYPE_MENU_MERGER, "tree-provider", provider, NULL);
}



gboolean
xfce_menu_merger_prepare_merging (XfceMenuMerger        *merger,
                                  GNode                 *tree,
                                  XfceMenuMergerContext *context)
{
  g_return_val_if_fail (XFCE_IS_MENU_MERGER (merger), FALSE);
  g_return_val_if_fail (context != NULL, FALSE);

  context->node_type = XFCE_MENU_NODE_TYPE_INVALID;
  g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_resolve_default_dirs, 
                   context);

  context->node_type = XFCE_MENU_NODE_TYPE_MERGE_DIR;
  g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_resolve_relative_paths,
                   context);

  context->node_type = XFCE_MENU_NODE_TYPE_MERGE_FILE;
  g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_resolve_relative_paths,
                   context);

  xfce_menu_merger_remove_duplicate_paths (tree,
                                           XFCE_MENU_NODE_TYPE_MERGE_DIR);

  g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_resolve_merge_dirs,
                   context);

  xfce_menu_merger_remove_duplicate_paths (tree,
                                           XFCE_MENU_NODE_TYPE_MERGE_FILE);

  return context->success;
}



gboolean
xfce_menu_merger_run (XfceMenuMerger *merger,
                      GCancellable   *cancellable,
                      GError        **error)
{
  XfceMenuMergerContext context;
  GFile                *file;

  g_return_val_if_fail (XFCE_IS_MENU_MERGER (merger), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  context.merger = merger;
  context.cancellable = cancellable;
  context.error = error;
  context.success = TRUE;
  context.file_stack = NULL;

  file = xfce_menu_tree_provider_get_file (XFCE_MENU_TREE_PROVIDER (merger));
  context.file_stack = g_list_concat (context.file_stack, merger->priv->file_stack);
  context.file_stack = g_list_prepend (context.file_stack, file);

  xfce_menu_merger_prepare_merging (merger, merger->priv->menu, &context);

  g_node_traverse (merger->priv->menu, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_process_merge_files,
                   &context);

  xfce_menu_merger_consolidate_child_menus (merger->priv->menu);

  context.node_type = XFCE_MENU_NODE_TYPE_DEFAULT_APP_DIRS;
  g_node_traverse (merger->priv->menu, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_resolve_default_dirs,
                   &context);

  context.node_type = XFCE_MENU_NODE_TYPE_APP_DIR;
  g_node_traverse (merger->priv->menu, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_resolve_relative_paths,
                   &context);

  xfce_menu_merger_remove_duplicate_paths (merger->priv->menu, XFCE_MENU_NODE_TYPE_APP_DIR);

  context.node_type = XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS;
  g_node_traverse (merger->priv->menu, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_resolve_default_dirs,
                   &context);

  context.node_type = XFCE_MENU_NODE_TYPE_DIRECTORY_DIR;
  g_node_traverse (merger->priv->menu, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_resolve_relative_paths,
                   &context);

  xfce_menu_merger_remove_duplicate_paths (merger->priv->menu,XFCE_MENU_NODE_TYPE_DIRECTORY_DIR);
  xfce_menu_merger_remove_duplicate_paths (merger->priv->menu, XFCE_MENU_NODE_TYPE_DIRECTORY);

  xfce_menu_merger_resolve_moves (merger->priv->menu);

  xfce_menu_merger_consolidate_child_menus (merger->priv->menu);

  xfce_menu_merger_clean_up_elements (merger->priv->menu, XFCE_MENU_NODE_TYPE_DELETED);
  xfce_menu_merger_clean_up_elements (merger->priv->menu, XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED);
  /* TODO Resolve <DefaultLayout> and empty <Layout> elements */
  xfce_menu_merger_clean_up_elements (merger->priv->menu, XFCE_MENU_NODE_TYPE_LAYOUT);

  g_list_foreach (context.file_stack, (GFunc) g_object_unref, NULL);
  g_list_free (context.file_stack);

  return context.success;
}



static GNode *
xfce_menu_merger_get_tree (XfceMenuTreeProvider *provider)
{
  g_return_val_if_fail (XFCE_IS_MENU_MERGER (provider), NULL);
  return xfce_menu_node_tree_copy (XFCE_MENU_MERGER (provider)->priv->menu);
}



static GFile *
xfce_menu_merger_get_file (XfceMenuTreeProvider *provider)
{
  XfceMenuTreeProvider *provider_;

  g_return_val_if_fail (XFCE_IS_MENU_MERGER (provider), NULL);

  provider_ = XFCE_MENU_MERGER (provider)->priv->tree_provider;
  return xfce_menu_tree_provider_get_file (provider_);
}



static void
xfce_menu_merger_insert_default_dirs (GNode *parent,
                                      GNode *defaults_node)
{
  XfceMenuNodeType     type;
  GNode               *node;
  GNode               *prev_node;
  const gchar * const *dirs;
  const gchar         *kde_dir;
  int                  i;
  gchar               *path;
  gchar               *kde_data_dir;
  const gchar         *basename;

  g_return_if_fail (parent != NULL);
  g_return_if_fail (defaults_node != NULL);

  prev_node = defaults_node;

  if  (xfce_menu_node_tree_get_node_type (defaults_node) == XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS)
    {
      basename = "desktop-directories";
      type = XFCE_MENU_NODE_TYPE_DIRECTORY_DIR;
    }
  else
    {
      basename = "applications";
      type = XFCE_MENU_NODE_TYPE_APP_DIR;
    }

  /* Append $KDEDIR/share/desktop-directories and $KDEDIR/share/applications 
   * as a workaround for distributions not installing KDE menu files 
   * properly into $XDG_DATA_DIRS */

  /* Get KDEDIR environment variable */
  kde_dir = g_getenv ("KDEDIR");

  /* Check if this variable is set */
  if (G_UNLIKELY (kde_dir != NULL))
    {
      /* Build KDE data dir */
      kde_data_dir = g_build_filename (kde_dir, "share", basename, NULL);

      /* Add it as a directory dir if it exists */
      if (G_LIKELY (g_file_test (kde_data_dir, G_FILE_TEST_IS_DIR)))
        {
          node = g_node_new (xfce_menu_node_create (type, kde_data_dir));
          prev_node = g_node_insert_after (parent, prev_node, node);
        }

      /* Free the KDE data dir */
      g_free (kde_data_dir);
    }

  /* The $KDEDIR workaround ends here */

  /* Append system-wide data dirs */
  dirs = g_get_system_data_dirs ();
  for (i = 0; dirs[i] != NULL; i++)
    {
      path = g_build_path (G_DIR_SEPARATOR_S, dirs[i], basename, NULL);
      if (G_LIKELY (g_file_test (path, G_FILE_TEST_IS_DIR)))
        {
          node = g_node_new (xfce_menu_node_create (type, path));
          prev_node = g_node_insert_after (parent, prev_node, node);
        }
      g_free (path);
    }

  /* Append user data dir */
  path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), basename, NULL);
  if (G_LIKELY (g_file_test (path, G_FILE_TEST_IS_DIR)))
    {
      node = g_node_new (xfce_menu_node_create (type, path));
      prev_node = g_node_insert_after (parent, prev_node, node);
    }
  g_free (path);
}



static void
xfce_menu_merger_insert_default_merge_dirs (GNode *parent,
                                            GNode *defaults_node)
{
  GNode               *node;
  GNode               *prev_node;
  const gchar * const *dirs;
  int                  i;
  gchar               *path;

  g_return_if_fail (parent != NULL);
  g_return_if_fail (defaults_node != NULL);

  prev_node = defaults_node;

  /* Append system-wide config dirs */
  dirs = g_get_system_config_dirs ();
  for (i = 0; dirs[i] != NULL; i++)
    {
      path = g_build_path (G_DIR_SEPARATOR_S, dirs[i], "menus", "applications-merged", NULL);
      if (G_LIKELY (g_file_test (path, G_FILE_TEST_IS_DIR)))
        {
          node = g_node_new (xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE_DIR, path));
          prev_node = g_node_insert_after (parent, prev_node, node);
        }
      g_free (path);
    }

  /* Append user config dir */
  path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_config_dir (), "menus", 
                       "applications-merged", NULL);
  if (G_LIKELY (g_file_test (path, G_FILE_TEST_IS_DIR)))
    {
      node = g_node_new (xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE_DIR, path));
      prev_node = g_node_insert_after (parent, prev_node, node);
    }
  g_free (path);
}



static gboolean 
xfce_menu_merger_resolve_default_dirs (GNode                 *node,
                                       XfceMenuMergerContext *context)
{
  g_return_val_if_fail (context != NULL, FALSE);

  if (xfce_menu_node_tree_get_node_type (node) == context->node_type)
    {
      xfce_menu_merger_insert_default_dirs (node->parent, node);
      xfce_menu_node_tree_free (node);
    }
  else if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS)
    {
      xfce_menu_merger_insert_default_merge_dirs (node->parent, node);
      xfce_menu_node_tree_free (node);
    }

  return FALSE;
}



static gboolean 
xfce_menu_merger_resolve_relative_paths (GNode                 *node,
                                         XfceMenuMergerContext *context)
{
  GFile               *source_file;
  const gchar * const *system_config_dirs;
  const gchar        **config_dirs;
  gchar               *absolute_path = NULL;
  gchar               *relative_path = NULL;
  gint                 i;

  g_return_val_if_fail (context != NULL, FALSE);

  if (xfce_menu_node_tree_get_node_type (node) != context->node_type)
    return FALSE;

  source_file = g_list_first (context->file_stack)->data;

  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_APP_DIR ||
      xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_DIRECTORY_DIR ||
      xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_MERGE_DIR)
    {
      relative_path = (gchar *)xfce_menu_node_tree_get_string (node);
      absolute_path = g_file_get_uri_relative_to_file (relative_path, source_file);
      xfce_menu_node_tree_set_string (node, absolute_path);
      g_free (absolute_path);
    }
  else if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_MERGE_FILE)
    {
      if (xfce_menu_node_tree_get_merge_file_type (node) == XFCE_MENU_MERGE_FILE_PATH)
        {
          relative_path = (gchar *)xfce_menu_node_tree_get_merge_file_filename (node);
          absolute_path = g_file_get_uri_relative_to_file (relative_path, source_file);
          xfce_menu_node_tree_set_merge_file_filename (node, absolute_path);
          g_free (absolute_path);
        }
      else
        {
          system_config_dirs = g_get_system_config_dirs ();

          config_dirs = g_new0 (const gchar *, 2 + g_strv_length ((gchar **)system_config_dirs));

          config_dirs[0] = g_get_user_config_dir ();
          config_dirs[1 + g_strv_length ((gchar **)system_config_dirs)] = NULL;

          for (i = 0; system_config_dirs[i] != NULL; ++i)
            config_dirs[i+1] = system_config_dirs[i];

          /* Find the parent XDG_CONFIG_DIRS entry for the current menu file */
          for (i = 0; relative_path == NULL && config_dirs[i] != NULL; ++i)
            {
              GFile *config_dir = g_file_new_for_unknown_input (config_dirs[i], NULL);
              relative_path = g_file_get_relative_path (config_dir, source_file);
              g_object_unref (config_dir);
            }

          /* Look for the same relative path in the XDG_CONFIG_DIRS entries after the parent 
           * of the current menu file */
          for (; relative_path != NULL && config_dirs[i] != NULL; ++i)
            {
              GFile *config_dir = g_file_new_for_unknown_input (config_dirs[i], NULL);
              GFile *absolute = g_file_resolve_relative_path (config_dir, relative_path);

              if (G_LIKELY (absolute != NULL))
                {
                  if (G_UNLIKELY (g_file_query_exists (absolute, NULL)))
                    {
                      absolute_path = g_file_get_uri (absolute);

                      /* Destroy the MenuFile type="parent" information */
                      xfce_menu_node_tree_free_data (node);

                      /* Replace it with a MergeFile type="path" element */
                      node->data = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE_FILE,
                                                          GUINT_TO_POINTER (XFCE_MENU_MERGE_FILE_PATH));
                      xfce_menu_node_tree_set_merge_file_filename (node, absolute_path);
                    }
                  g_object_unref (absolute);
                  break;
                }
              g_object_unref (config_dir);
            }

          /* No file with the same relative filename found in XDG_CONFIG_DIRS */
          if (absolute_path == NULL || i >= g_strv_length ((gchar **)config_dirs))
            {
              /* Remove the MenuFile type="parent" node */
              xfce_menu_node_tree_free (node);
            }

          g_free (absolute_path);
          g_free (relative_path);
          g_free (config_dirs);
        }
    }

  return FALSE;
}



static void
xfce_menu_merger_remove_duplicate_paths (GNode            *node,
                                         XfceMenuNodeType  type)
{
  GList        *destroy_nodes = NULL;
  GList        *remaining_nodes = NULL;
  GNode        *child;

  g_return_if_fail (node != NULL);

  if (xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_MENU)
    return;

  for (child = g_node_last_child (node); child != NULL; child = g_node_prev_sibling (child))
    {
      if (xfce_menu_node_tree_get_node_type (child) == XFCE_MENU_NODE_TYPE_MENU)
        {
          xfce_menu_merger_remove_duplicate_paths (child, type);
          continue;
        }

      if (xfce_menu_node_tree_get_node_type (child) != type)
        continue;

      if (G_LIKELY (g_list_find_custom (remaining_nodes, child, 
                                        (GCompareFunc) xfce_menu_node_tree_compare) == NULL))
        {
          remaining_nodes = g_list_prepend (remaining_nodes, child);
        }
      else
        destroy_nodes = g_list_prepend (destroy_nodes, child);
    }

  g_list_foreach (destroy_nodes, (GFunc) xfce_menu_node_tree_free, NULL);
  g_list_free (destroy_nodes);
  g_list_free (remaining_nodes);
}



static gboolean
collect_name (GNode        *node,
              const gchar **name)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_NAME)
    {
      *name = xfce_menu_node_tree_get_string (node);
      return TRUE;
    }
  else
    return FALSE;
}



static void
xfce_menu_merger_move_nodes (GNode *source,
                             GNode *target,
                             GNode *position)
{
  GNode *child;

  for (child = g_node_first_child (source); child != NULL; child = g_node_next_sibling (child))
    if (xfce_menu_node_tree_get_node_type (child) != XFCE_MENU_NODE_TYPE_NAME)
      g_node_insert_before (target, position, g_node_copy (child));
}



static void
xfce_menu_merger_consolidate_child_menus (GNode *node)
{
  GHashTable  *table;
  GNode       *child;
  GNode       *next_child;
  GNode       *target;
  const gchar *name;

  if (xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_MENU)
    return;

  table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  /* Determine the last child menu for each child menu name */
  for (child = g_node_last_child (node); child != NULL; child = g_node_prev_sibling (child))
    {
      if (xfce_menu_node_tree_get_node_type (child) != XFCE_MENU_NODE_TYPE_MENU)
        continue;

      name = NULL;
      g_node_traverse (child, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                       (GNodeTraverseFunc) collect_name, &name);

      if (G_UNLIKELY (g_hash_table_lookup (table, name) == NULL))
        g_hash_table_insert (table, g_strdup (name), child);
    }

  for (child = g_node_first_child (node); child != NULL; )
    {
      if (xfce_menu_node_tree_get_node_type (child) == XFCE_MENU_NODE_TYPE_MENU)
        {
          name = NULL;
          g_node_traverse (child, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                           (GNodeTraverseFunc) collect_name, &name);

          target = g_hash_table_lookup (table, name);

          if (G_LIKELY (target != NULL && child != target))
            {
              xfce_menu_merger_move_nodes (child, target, g_node_first_child (target));

              next_child = g_node_next_sibling (child);
              g_node_destroy (child);
              child = g_node_prev_sibling (next_child);
            }
        }

        child = g_node_next_sibling (child);
    }

  for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
    xfce_menu_merger_consolidate_child_menus (child);

  g_hash_table_unref (table);
}



static gboolean
xfce_menu_merger_resolve_merge_dirs (GNode                 *node,
                                     XfceMenuMergerContext *context)
{
  GFileEnumerator *enumerator;
  GFileInfo       *file_info;
  GFile           *file;
  GFile           *dir;
  GNode           *file_node;
  gchar           *uri;

  g_return_val_if_fail (context != NULL, FALSE);

  /* Skip elements that are not MergeDirs */
  if (xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_MERGE_DIR)
    return FALSE;

  dir = g_file_new_for_unknown_input (xfce_menu_node_tree_get_string (node), NULL);

  enumerator = g_file_enumerate_children (dir, G_FILE_ATTRIBUTE_STANDARD_NAME, 
                                          G_FILE_QUERY_INFO_NONE, NULL, NULL);

  if (G_UNLIKELY (enumerator != NULL))
    {
      while (TRUE)
        {
          file_info = g_file_enumerator_next_file (enumerator, NULL, NULL);

          if (G_UNLIKELY (file_info == NULL))
            break;

          if (G_LIKELY (g_str_has_suffix (g_file_info_get_name (file_info), ".menu")))
            {
              file_node = g_node_new (xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE_FILE,
                                                             XFCE_MENU_MERGE_FILE_PATH));

              file = g_file_resolve_relative_path (dir, g_file_info_get_name (file_info));
              uri = g_file_get_uri (file);

              xfce_menu_node_tree_set_merge_file_filename (file_node, uri);

              g_free (uri);
              g_object_unref (file);

              g_node_insert_after (node->parent, node, file_node);
            }
          
          g_object_unref (file_info);
        }

      g_object_unref (enumerator);
    }

  xfce_menu_node_tree_free (node);

  g_object_unref (dir);

  return FALSE;
}



static gboolean
xfce_menu_parser_insert_elements (GNode *node,
                                  GNode *origin)
{
  if (node == origin)
    return FALSE;

  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_NAME)
    {
      xfce_menu_node_tree_free (node);
      return FALSE;
    }

  g_node_insert_before (origin->parent, origin, g_node_copy (node));

  return FALSE;
}



static gint
compare_files (GFile *file,
               GFile *other_file)
{
  return g_file_equal (file, other_file) ? 0 : 1;
}




static gboolean
xfce_menu_merger_process_merge_files (GNode                 *node,
                                      XfceMenuMergerContext *context)
{
  XfceMenuMerger *merger;
  XfceMenuParser *parser;
  GFile          *file;
  GNode          *tree;

  g_return_val_if_fail (context != NULL, FALSE);

  if (xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_MERGE_FILE ||
      xfce_menu_node_tree_get_merge_file_type (node) != XFCE_MENU_MERGE_FILE_PATH)
    {
      return FALSE;
    }

  file = g_file_new_for_uri (xfce_menu_node_tree_get_merge_file_filename (node));

  if (G_UNLIKELY (g_list_find_custom (context->file_stack, file, 
                                      (GCompareFunc) compare_files) != NULL))
    {
      g_object_unref (file);
      return FALSE;
    }

  parser = xfce_menu_parser_new (file);
  g_object_unref (file);

  if (G_LIKELY (xfce_menu_parser_run (parser, NULL, NULL)))
    {
      merger = xfce_menu_merger_new (XFCE_MENU_TREE_PROVIDER (parser));
      g_object_unref (parser);

      merger->priv->file_stack = g_list_copy (context->file_stack);
      g_list_foreach (merger->priv->file_stack, (GFunc) g_object_ref, NULL);

      if (G_LIKELY (xfce_menu_merger_run (merger, NULL, NULL)))
        {
          tree = xfce_menu_tree_provider_get_tree (XFCE_MENU_TREE_PROVIDER (merger));
          g_object_unref (merger);

          g_node_insert_after (node->parent, node, tree);
          g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                           (GNodeTraverseFunc) xfce_menu_parser_insert_elements, tree);
          g_node_destroy (tree);
        }
    }
  
  xfce_menu_node_tree_free (node);

  return FALSE;
}



static void 
xfce_menu_merger_clean_up_elements (GNode            *node,
                                    XfceMenuNodeType  type)
{
  GNode *child;
  GNode *remaining_node = NULL;
  GList *destroy_list = NULL;

  for (child = g_node_last_child (node); child != NULL; child = g_node_prev_sibling (child))
    {
      if (xfce_menu_node_tree_get_node_type (child) == XFCE_MENU_NODE_TYPE_MENU)
        {
          xfce_menu_merger_clean_up_elements (child, type);
          continue;
        }

      if (type == XFCE_MENU_NODE_TYPE_DELETED 
          && xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_DELETED
          && xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_NOT_DELETED)
        {
          continue;
        }

      if (type == XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED
          && xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED
          && xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED)
        {
          continue;
        }
      
      if (type == XFCE_MENU_NODE_TYPE_LAYOUT 
          && xfce_menu_node_tree_get_node_type (child) != XFCE_MENU_NODE_TYPE_LAYOUT)
        {
          continue;
        }
      
      if (remaining_node != NULL)
        destroy_list = g_list_prepend (destroy_list, child);
      else
        remaining_node = child;
    }

  g_list_foreach (destroy_list, (GFunc) xfce_menu_node_tree_free, NULL);
  g_list_free (destroy_list);
}



static gboolean
collect_moves (GNode  *node,
               GList **list)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_MOVE)
    *list = g_list_append (*list, node);

  return FALSE;
}



static gboolean
remove_moves (GNode *node)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_MOVE)
    xfce_menu_node_tree_free (node);

  return FALSE;
}



static gboolean
collect_old_new (GNode  *node,
                 GList **list)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_OLD)
    *list = g_list_append (*list, g_strdup (xfce_menu_node_tree_get_string (node)));
  else if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_NEW)
    *list = g_list_append (*list, g_strdup (xfce_menu_node_tree_get_string (node)));

  return FALSE;
}



static GNode *
xfce_menu_merger_find_menu_with_name (GNode       *node,
                                      const gchar *name)
{
  GNode       *result = NULL;
  GNode       *child;
  const gchar *child_name;

  for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
    if (xfce_menu_node_tree_get_node_type (child) == XFCE_MENU_NODE_TYPE_MENU)
      {
        child_name = NULL;
        g_node_traverse (child, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                         (GNodeTraverseFunc) collect_name, &child_name);

        if (g_str_equal (child_name, name))
          {
            result = child;
            break;
          }
      }

  return result;
}



static GNode *
xfce_menu_merger_find_menu (GNode  *node,
                            gchar **path,
                            gint    position,
                            gint    depth,
                            GNode **parent)
{
  GNode *result = NULL;
  GNode *child;

  g_return_val_if_fail (position <= depth, NULL);
  g_return_val_if_fail (node != NULL, NULL);
  g_return_val_if_fail (path != NULL, NULL);

  /* Make sure to set parent to NULL unless the path is valid */
  if (parent != NULL)
    *parent = NULL;

  /* Search for the direct child which has the name equal to the current
   * path componenent */
  child = xfce_menu_merger_find_menu_with_name (node, path[position]);

  if (G_UNLIKELY (child == NULL))
    return NULL;

  if (G_LIKELY (position == depth))
    {
      result = child;

      if (parent != NULL)
        *parent = node;
    }
  else
    result = xfce_menu_merger_find_menu (child, path, position+1, depth, parent);

  return result;
}



static GNode *
xfce_menu_merger_create_menu (GNode  *node,
                              gchar **path,
                              gint    position,
                              gint    depth)
{
  GNode *result = NULL;
  GNode *child;

  g_return_val_if_fail (position <= depth, NULL);
  g_return_val_if_fail (node != NULL, NULL);
  g_return_val_if_fail (path != NULL, NULL);

  /* Search for the direct child which has the name equal to the current
   * path componenent */
  child = xfce_menu_merger_find_menu_with_name (node, path[position]);

  /* Create it if it doesn't exist */
  if (G_LIKELY (child == NULL))
    {
      child = g_node_append_data (node, NULL);
      g_node_append_data (child, xfce_menu_node_create (XFCE_MENU_NODE_TYPE_NAME, path[position]));
    }

  if (G_LIKELY (position == depth))
    result = child;
  else
    result = xfce_menu_merger_create_menu (child, path, position+1, depth);

  return result;
}



static void
xfce_menu_merger_resolve_moves (GNode *node)
{
  GNode  *child;
  GNode  *old_node;
  GNode  *new_node;
  GList  *moves = NULL;
  GList  *pairs = NULL;
  GList  *iter;
  gchar **old_path;
  gchar **new_path;

  if (xfce_menu_node_tree_get_node_type (node) != XFCE_MENU_NODE_TYPE_MENU)
    return;

  for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
    xfce_menu_merger_resolve_moves (child);

  g_node_traverse (node, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_moves, &moves);

  for (iter = moves; iter != NULL; iter = g_list_next (iter))
    {
      g_node_traverse (iter->data, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                       (GNodeTraverseFunc) collect_old_new, &pairs);
    }

  g_node_traverse (node, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) remove_moves, &moves);

  for (iter = pairs; iter != NULL; iter = g_list_next (iter))
    {
      if (g_list_length (iter) < 2)
        break;

      old_path = g_strsplit (iter->data, "/", -1);
      iter = g_list_next (iter);
      new_path = g_strsplit (iter->data, "/", -1);

      old_node = xfce_menu_merger_find_menu (node, old_path, 0, 
                                             g_strv_length (old_path)-1, NULL);
      new_node = xfce_menu_merger_find_menu (node, new_path, 0, 
                                             g_strv_length (new_path)-1, NULL);

      if (G_LIKELY (old_node != NULL && old_node != new_node))
        {
          if (G_LIKELY (new_node == NULL))
            {
              new_node = xfce_menu_merger_create_menu (node, new_path, 0,
                                                       g_strv_length (new_path)-1);

              xfce_menu_merger_move_nodes (old_node, new_node, NULL);
            }
          else
            xfce_menu_merger_move_nodes (old_node, new_node, g_node_first_child (new_node));

          g_node_destroy (old_node);
        }

      g_strfreev (old_path);
      g_strfreev (new_path);
    }

  g_list_foreach (pairs, (GFunc) g_free, NULL);
  g_list_free (pairs);
}

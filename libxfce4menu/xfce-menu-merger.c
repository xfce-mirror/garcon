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

#include <libxfce4util/libxfce4util.h>

#include <libxfce4menu/xfce-menu-node.h>
#include <libxfce4menu/xfce-menu-tree-provider.h>
#include <libxfce4menu/xfce-menu-parser.h>
#include <libxfce4menu/xfce-menu-merger.h>



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



static void     xfce_menu_merger_class_init             (XfceMenuMergerClass       *klass);
static void     xfce_menu_merger_provider_init          (XfceMenuTreeProviderIface *iface);
static void     xfce_menu_merger_init                   (XfceMenuMerger            *merger);
static void     xfce_menu_merger_constructed            (GObject                   *object);
static void     xfce_menu_merger_finalize               (GObject                   *object);
static void     xfce_menu_merger_get_property           (GObject                   *object,
                                                         guint                      prop_id,
                                                         GValue                    *value,
                                                         GParamSpec                *pspec);
static void     xfce_menu_merger_set_property           (GObject                   *object,
                                                         guint                      prop_id,
                                                         const GValue              *value,
                                                         GParamSpec                *pspec);
static GNode   *xfce_menu_merger_get_tree               (XfceMenuTreeProvider      *provider);
static GFile   *xfce_menu_merger_get_file               (XfceMenuTreeProvider      *provider);
static gboolean xfce_menu_merger_resolve_default_dirs   (GNode                     *node,
                                                         XfceMenuMergerContext     *context);
static gboolean xfce_menu_merger_resolve_relative_paths (GNode                     *node,
                                                         XfceMenuMergerContext     *context);
static void     xfce_menu_merger_remove_duplicate_paths (GNode                     *node,
                                                         XfceMenuNodeType           type);
static gboolean xfce_menu_merger_resolve_merge_dirs     (GNode                     *node,
                                                         XfceMenuMergerContext     *context);
static gboolean xfce_menu_merger_process_merge_files    (GNode                     *node,
                                                         XfceMenuMergerContext     *context);
static void     xfce_menu_merger_clean_up_elements      (GNode                     *node,
                                                         XfceMenuNodeType           type);
static GNode   *xfce_menu_merger_remove_deleted_menus   (GNode                     *node);



struct _XfceMenuMergerPrivate
{
  XfceMenuTreeProvider *tree_provider;
  GNode                *menu;
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

  context->node_type = XFCE_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS;
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
  context.file_stack = g_list_prepend (context.file_stack, file);

  xfce_menu_merger_prepare_merging (merger, merger->priv->menu, &context);

  g_node_traverse (merger->priv->menu, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
                   (GNodeTraverseFunc) xfce_menu_merger_process_merge_files,
                   &context);

#if 0
  xfce_menu_merger_consolidate_child_menus (merger->priv->menu, &context);
#endif

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

#if 0
  xfce_menu_merger_resolve_moves (merger->priv->menu);
#endif

  xfce_menu_merger_clean_up_elements (merger->priv->menu, XFCE_MENU_NODE_TYPE_DELETED);
  xfce_menu_merger_clean_up_elements (merger->priv->menu, XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED);
  
  merger->priv->menu = xfce_menu_merger_remove_deleted_menus (merger->priv->menu);

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
  XfceMenuNode        *node_;
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

  node_ = defaults_node->data;
  prev_node = defaults_node;

  switch (xfce_menu_node_get_node_type (node_))
    {
    case XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS:
      basename = "desktop-directories";
      type = XFCE_MENU_NODE_TYPE_DIRECTORY_DIR;
      break;
    case XFCE_MENU_NODE_TYPE_DEFAULT_APP_DIRS:
      basename = "applications";
      type = XFCE_MENU_NODE_TYPE_APP_DIR;
      break;
    default:
      basename = "applications-merged";
      type = XFCE_MENU_NODE_TYPE_MERGE_DIR;
      break;
    }

  /* Append $KDEDIR/share/desktop-directories, $KDEDIR/share/applications or
   * $KDEDIR/share/applications-merged as a workaround for distributions not 
   * installing KDE menu files properly into $XDG_DATA_DIRS */

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
          node_ = xfce_menu_node_create (type, kde_data_dir);
          node = g_node_new (node_);
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
          node_ = xfce_menu_node_create (type, path);
          node = g_node_new (node_);
          prev_node = g_node_insert_after (parent, prev_node, node);
        }
      g_free (path);
    }

  /* Append user data dir */
  path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), basename, NULL);
  if (G_LIKELY (g_file_test (path, G_FILE_TEST_IS_DIR)))
    {
      node_ = xfce_menu_node_create (type, path);
      node = g_node_new (node_);
      prev_node = g_node_insert_after (parent, prev_node, node);
    }
  g_free (path);
}



static gboolean 
xfce_menu_merger_resolve_default_dirs (GNode                 *node,
                                       XfceMenuMergerContext *context)
{
  XfceMenuNode *node_ = node->data; 

  g_return_val_if_fail (context != NULL, FALSE);

  if (node_ == NULL)
    return FALSE;

  if (xfce_menu_node_get_node_type (node_) == context->node_type)
    {
      xfce_menu_merger_insert_default_dirs (node->parent, node);
      xfce_menu_node_tree_free (node);
    }

  return FALSE;
}



static gboolean
is_valid_scheme_character (char c)
{
  return g_ascii_isalnum (c) || c == '+' || c == '-' || c == '.';
}



/* Following RFC 2396, valid schemes are built like:
 *       scheme        = alpha *( alpha | digit | "+" | "-" | "." )
 */
static gboolean
has_valid_scheme (const char *uri)
{
  const char *p;

  p = uri;

  if (!g_ascii_isalpha (*p))
    return FALSE;

  do 
    {
      p++;
    } 
  while (is_valid_scheme_character (*p));

  return *p == ':';
}



static GFile *
g_file_new_for_unknown_input (const gchar *path,
                              GFile       *parent)
{
  g_return_val_if_fail (path != NULL, NULL);

  if (g_path_is_absolute (path))
    return g_file_new_for_path (path);

  if (has_valid_scheme (path))
    return g_file_new_for_uri (path);

  if (G_LIKELY (parent != NULL))
    return g_file_resolve_relative_path (parent, path);
  else
    return g_file_new_for_path (path);
}



static gchar *
xfce_menu_merger_build_absolute_path (GFile       *source_file,
                                      const gchar *path)
{
  GFile *file;
  GFile *parent;
  gchar *absolute_path = NULL;

  g_return_val_if_fail (G_IS_FILE (source_file), NULL);
  g_return_val_if_fail (path != NULL, NULL);

  parent = g_file_get_parent (source_file);
  file = g_file_new_for_unknown_input (path, parent);
  absolute_path = g_file_get_uri (file);

  g_object_unref (file);
  g_object_unref (parent);
  
  return absolute_path;
}



static gboolean 
xfce_menu_merger_resolve_relative_paths (GNode                 *node,
                                         XfceMenuMergerContext *context)
{
  XfceMenuNode *node_ = node->data;
  GFile        *source_file;
  gchar       **config_dirs;
  gchar        *absolute_path = NULL;
  gchar        *relative_path = NULL;
  gint          i;

  g_return_val_if_fail (context != NULL, FALSE);

  if (node_ == NULL)
    return FALSE;

  if (xfce_menu_node_get_node_type (node_) != context->node_type)
    return FALSE;

  source_file = g_list_first (context->file_stack)->data;

  if (xfce_menu_node_get_node_type (node_) == XFCE_MENU_NODE_TYPE_APP_DIR ||
      xfce_menu_node_get_node_type (node_) == XFCE_MENU_NODE_TYPE_DIRECTORY_DIR ||
      xfce_menu_node_get_node_type (node_) == XFCE_MENU_NODE_TYPE_MERGE_DIR)
    {
      relative_path = (gchar *)xfce_menu_node_get_string (node_);
      absolute_path = xfce_menu_merger_build_absolute_path (source_file, relative_path);
      xfce_menu_node_set_string (node_, absolute_path);
      g_free (absolute_path);
    }
  else if (xfce_menu_node_get_node_type (node_) == XFCE_MENU_NODE_TYPE_MERGE_FILE)
    {
      if (xfce_menu_node_get_merge_file_type (node_) == XFCE_MENU_MERGE_FILE_PATH)
        {
          relative_path = (gchar *)xfce_menu_node_get_merge_file_filename (node_);
          absolute_path = xfce_menu_merger_build_absolute_path (source_file, relative_path);
          xfce_menu_node_set_merge_file_filename (node_, absolute_path);
          g_free (absolute_path);
        }
      else
        {
          config_dirs = xfce_resource_dirs (XFCE_RESOURCE_CONFIG);

          /* Find the parent XDG_CONFIG_DIRS entry for the current menu file */
          for (i = 0; relative_path == NULL && i < g_strv_length ((gchar **)config_dirs); ++i)
            {
              GFile *config_dir = g_file_new_for_unknown_input (config_dirs[i], NULL);
              relative_path = g_file_get_relative_path (config_dir, source_file);
              g_object_unref (config_dir);
            }

          /* Look for the same relative path in the XDG_CONFIG_DIRS entries after the parent 
           * of the current menu file */
          for (++i; relative_path != NULL && i < g_strv_length ((gchar **)config_dirs); ++i)
            {
              GFile *config_dir = g_file_new_for_unknown_input (config_dirs[i], NULL);
              GFile *absolute = g_file_resolve_relative_path (config_dir, relative_path);
              if (G_LIKELY (absolute != NULL))
                {
                  if (G_UNLIKELY (g_file_query_exists (absolute, NULL)))
                    {
                      absolute_path = g_file_get_uri (absolute);

                      /* Replace the MenuFile type="parent" node */
                      g_object_unref (node_);
                      node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE_FILE,
                                                     GUINT_TO_POINTER (XFCE_MENU_MERGE_FILE_PATH));
                      xfce_menu_node_set_merge_file_filename (node_, absolute_path);
                      node->data = node_;
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
          g_strfreev (config_dirs);
        }
    }

  return FALSE;
}



static void
xfce_menu_merger_remove_duplicate_paths (GNode            *node,
                                         XfceMenuNodeType  type)
{
  XfceMenuNode *node_ = node->data;
  GList        *destroy_nodes = NULL;
  GList        *remaining_nodes = NULL;
  GNode        *child;

  if (node_ != NULL)
    return;

  g_return_if_fail (node != NULL);

  for (child = g_node_last_child (node); child != NULL; child = g_node_prev_sibling (child))
    {
      node_ = child->data;

      if (node_ == NULL)
        {
          xfce_menu_merger_remove_duplicate_paths (child, type);
          continue;
        }

      if (xfce_menu_node_get_node_type (node_) != type)
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
xfce_menu_merger_resolve_merge_dirs (GNode                 *node,
                                     XfceMenuMergerContext *context)
{
  XfceMenuNode    *node_ = node->data;
  XfceMenuNode    *file_node_;
  GFileEnumerator *enumerator;
  GFileInfo       *file_info;
  GFile           *file;
  GFile           *dir;
  GNode           *file_node;
  gchar           *uri;

  g_return_val_if_fail (context != NULL, FALSE);

  /* Skip elements that are not MergeDirs */
  if (node_ == NULL || xfce_menu_node_get_node_type (node_) != XFCE_MENU_NODE_TYPE_MERGE_DIR)
    return FALSE;

  dir = g_file_new_for_unknown_input (xfce_menu_node_get_string (node_), NULL);
  enumerator = g_file_enumerate_children (dir, G_FILE_ATTRIBUTE_STANDARD_NAME, 
                                          G_FILE_QUERY_INFO_NONE, NULL, NULL);

  if (G_UNLIKELY (enumerator != NULL))
    {
      while (TRUE)
        {
          file_info = g_file_enumerator_next_file (enumerator, NULL, NULL);

          if (G_UNLIKELY (file_info == NULL))
            break;

          file = g_file_resolve_relative_path (dir, g_file_info_get_name (file_info));

          file_node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE_FILE, 
                                              XFCE_MENU_MERGE_FILE_PATH);

          uri = g_file_get_uri (file);
          xfce_menu_node_set_merge_file_filename (file_node_, uri);
          g_free (uri);

          file_node = g_node_new (file_node_);
          g_node_insert_after (node->parent, node, file_node);

          g_object_unref (file);
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
  XfceMenuNode *node_ = node->data;

  if (node == origin)
    return FALSE;

  if (node_ != NULL && xfce_menu_node_get_node_type (node_) == XFCE_MENU_NODE_TYPE_NAME)
    {
      xfce_menu_node_tree_free (node);
      return FALSE;
    }

  g_node_insert_before (origin->parent, origin, g_node_copy (node));

  return FALSE;
}



static gboolean
xfce_menu_merger_process_merge_files (GNode                 *node,
                                      XfceMenuMergerContext *context)
{
  XfceMenuParser *parser;
  XfceMenuNode   *node_ = node->data;
  GFile          *source_file;
  GFile          *file;
  GNode          *tree;

  g_return_val_if_fail (context != NULL, FALSE);
  
  if (node_ == NULL || xfce_menu_node_get_node_type (node_) != XFCE_MENU_NODE_TYPE_MERGE_FILE)
    return FALSE;

  if (xfce_menu_node_get_merge_file_type (node_) != XFCE_MENU_MERGE_FILE_PATH)
    return FALSE;

  file = g_file_new_for_uri (xfce_menu_node_get_merge_file_filename (node_));
  parser = xfce_menu_parser_new (file);
  g_object_unref (file);

  if (G_LIKELY (xfce_menu_parser_run (parser, NULL, NULL)))
    {
      tree = xfce_menu_tree_provider_get_tree (XFCE_MENU_TREE_PROVIDER (parser));
      source_file = xfce_menu_tree_provider_get_file (XFCE_MENU_TREE_PROVIDER (parser));

      context->file_stack = g_list_prepend (context->file_stack, source_file);
      xfce_menu_merger_prepare_merging (context->merger, tree, context);
      context->file_stack = g_list_remove (context->file_stack, source_file);

      g_object_unref (source_file);

      g_node_insert_after (node->parent, node, tree);
      g_node_traverse (tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                       (GNodeTraverseFunc) xfce_menu_parser_insert_elements, tree);

      g_node_destroy (tree);
    }
  
  xfce_menu_node_tree_free (node);

  g_object_unref (parser);

  return FALSE;
}



static void 
xfce_menu_merger_clean_up_elements (GNode            *node,
                                    XfceMenuNodeType  type)
{
  XfceMenuNode *node_;
  GNode        *child;
  GNode        *remaining_node = NULL;
  GList        *destroy_list = NULL;

  for (child = g_node_last_child (node); child != NULL; child = g_node_prev_sibling (child))
    {
      node_ = child->data;

      if (node_ == NULL)
        {
          xfce_menu_merger_clean_up_elements (child, type);
          continue;
        }

      if (type == XFCE_MENU_NODE_TYPE_DELETED 
          && xfce_menu_node_get_node_type (node_) != XFCE_MENU_NODE_TYPE_DELETED
          && xfce_menu_node_get_node_type (node_) != XFCE_MENU_NODE_TYPE_NOT_DELETED)
        {
          continue;
        }

      if (type == XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED
          && xfce_menu_node_get_node_type (node_) != XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED
          && xfce_menu_node_get_node_type (node_) != XFCE_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED)
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



static GNode *
xfce_menu_merger_remove_deleted_menus (GNode *node)
{
  XfceMenuNode *node_;
  GNode        *next_child;
  GNode        *child;
  gboolean      is_deleted = FALSE;

  g_return_val_if_fail (node != NULL, FALSE);

  if (node->data != NULL)
    return node;

  for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
    {
      node_ = child->data;

      if (node_ == NULL)
        continue;

      if (xfce_menu_node_get_node_type (node_) == XFCE_MENU_NODE_TYPE_DELETED)
        is_deleted = TRUE;
      else if (xfce_menu_node_get_node_type (node_) == XFCE_MENU_NODE_TYPE_NOT_DELETED)
        is_deleted = FALSE;
    }

  if (is_deleted)
    {
      xfce_menu_node_tree_free (node);
      return NULL;
    }
      
  for (child = g_node_first_child (node); child != NULL; )
    {
      next_child = g_node_next_sibling (child);

      if (child->data == NULL)
        xfce_menu_merger_remove_deleted_menus (child);

      child = next_child;
    }

  return node;
}

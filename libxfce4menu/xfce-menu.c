/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2007-2009 Jannis Pohlmann <jannis@xfce.org>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <libxfce4util/libxfce4util.h>

#include <libxfce4menu/xfce-menu-environment.h>
#include <libxfce4menu/xfce-menu-element.h>
#include <libxfce4menu/xfce-menu-item.h>
#include <libxfce4menu/xfce-menu-directory.h>
#include <libxfce4menu/xfce-menu-item-pool.h>
#include <libxfce4menu/xfce-menu-item-cache.h>
#include <libxfce4menu/xfce-menu-layout.h>
#include <libxfce4menu/xfce-menu-separator.h>
#include <libxfce4menu/xfce-menu-monitor.h>
#include <libxfce4menu/xfce-menu-node.h>
#include <libxfce4menu/xfce-menu-parser.h>
#include <libxfce4menu/xfce-menu-merger.h>
#include <libxfce4menu/xfce-menu-gio.h>
#include <libxfce4menu/xfce-menu.h>



/* Use g_access() on win32 */
#if defined(G_OS_WIN32)
#include <glib/gstdio.h>
#else
#define g_access(filename, mode) (access ((filename), (mode)))
#endif



/**
 * SECTION:xfce-menu
 * @title: XfceMenu
 * @short_description: Menu loading and library initialization/shutdown
 **/



/* Potential root menu files */
static const gchar XFCE_MENU_ROOT_SPECS[][30] = 
{
  "menus/applications.menu",
  "menus/xfce-applications.menu",
  "menus/gnome-applications.menu",
  "menus/kde-applications.menu",
};



#define XFCE_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), XFCE_TYPE_MENU, XfceMenuPrivate))



typedef struct _XfceMenuPair
{
  gpointer first;
  gpointer second;
} XfceMenuPair;



/* Property identifiers */
enum
{
  PROP_0,
  PROP_ENVIRONMENT,
  PROP_FILE,
  PROP_DIRECTORY,
  PROP_PARENT, /* TODO */
};



static void               xfce_menu_class_init                             (XfceMenuClass         *klass);
static void               xfce_menu_element_init                           (XfceMenuElementIface  *iface);
static void               xfce_menu_instance_init                          (XfceMenu              *menu);
static void               xfce_menu_finalize                               (GObject               *object);
static void               xfce_menu_get_property                           (GObject               *object,
                                                                            guint                  prop_id,
                                                                            GValue                *value,
                                                                            GParamSpec            *pspec);
static void               xfce_menu_set_property                           (GObject               *object,
                                                                            guint                  prop_id,
                                                                            const GValue          *value,
                                                                            GParamSpec            *pspec);

static gboolean           xfce_menu_load                                   (XfceMenu              *menu,
                                                                            GError               **error);
#if 0
static void               xfce_menu_consolidate_child_menus                (XfceMenu              *menu);
#endif
static void               xfce_menu_resolve_menus                          (XfceMenu              *menu);
static void               xfce_menu_resolve_directory                      (XfceMenu              *menu);
static XfceMenuDirectory *xfce_menu_lookup_directory                       (XfceMenu              *menu,
                                                                            const gchar           *filename);
static void               xfce_menu_collect_files                          (XfceMenu              *menu,
                                                                            GHashTable            *desktop_id_table);
static void               xfce_menu_collect_files_from_path                (XfceMenu              *menu,
                                                                            GHashTable            *desktop_id_table,
                                                                            GFile                 *path,
                                                                            const gchar           *id_prefix);
static void               xfce_menu_resolve_items                          (XfceMenu              *menu,
                                                                            GHashTable            *desktop_id_table,
                                                                            gboolean               only_unallocated);
static void               xfce_menu_resolve_items_by_rule                  (XfceMenu              *menu,
                                                                            GHashTable            *desktop_id_table,
                                                                            GNode                 *node);
static void               xfce_menu_resolve_item_by_rule                   (const gchar           *desktop_id,
                                                                            const gchar           *uri,
                                                                            XfceMenuPair          *data);
static void               xfce_menu_remove_deleted_menus                   (XfceMenu              *menu);
static gint               xfce_menu_compare_items                          (gconstpointer         *a,
                                                                            gconstpointer         *b);
static const gchar       *xfce_menu_get_element_name                       (XfceMenuElement       *element);
static const gchar       *xfce_menu_get_element_icon_name                  (XfceMenuElement       *element);
static void               xfce_menu_monitor_start                          (XfceMenu              *menu);
static void               xfce_menu_monitor_stop                           (XfceMenu              *menu);



struct _XfceMenuPrivate
{
  /* Menu file */
  GFile             *file;

  /* DOM tree */
  GNode             *tree;

  /* Directory */
  XfceMenuDirectory *directory;

  /* Submenus */
  GSList            *submenus;

  /* Parent menu */
  XfceMenu          *parent;

  /* Menu item pool */
  XfceMenuItemPool  *pool;

  /* Shared menu item cache */
  XfceMenuItemCache *cache;

  /* Menu layout */
  XfceMenuLayout    *layout;
};

struct _XfceMenuClass
{
  GObjectClass __parent__;
};

struct _XfceMenu
{
  GObject          __parent__;

  /* < private > */
  XfceMenuPrivate *priv;
};



static GObjectClass *xfce_menu_parent_class = NULL;



GType
xfce_menu_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfceMenuClass),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_class_init,
        NULL,
        NULL,
        sizeof (XfceMenu),
        0,
        (GInstanceInitFunc) xfce_menu_instance_init,
        NULL,
      };

      static const GInterfaceInfo element_info =
      {
        (GInterfaceInitFunc) xfce_menu_element_init,
        NULL,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMenu", &info, 0);
      g_type_add_interface_static (type, XFCE_TYPE_MENU_ELEMENT, &element_info);
    }

  return type;
}



static void
xfce_menu_class_init (XfceMenuClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (XfceMenuPrivate));

  /* Determine the parent type class */
  xfce_menu_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_finalize; 
  gobject_class->get_property = xfce_menu_get_property;
  gobject_class->set_property = xfce_menu_set_property;

  /**
   * XfceMenu:file:
   *
   * The #GFile from which the %XfceMenu was loaded. 
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file",
                                                        "file",
                                                        "file",
                                                        G_TYPE_FILE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  /**
   * XfceMenu:directory:
   *
   * The directory entry associated with this menu. 
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_DIRECTORY,
                                   g_param_spec_object ("directory",
                                                        "Directory",
                                                        "Directory entry associated with this menu",
                                                        XFCE_TYPE_MENU_DIRECTORY,
                                                        G_PARAM_READWRITE));
}



static void
xfce_menu_element_init (XfceMenuElementIface *iface)
{
  iface->get_name = xfce_menu_get_element_name;
  iface->get_icon_name = xfce_menu_get_element_icon_name;
}



static void
xfce_menu_instance_init (XfceMenu *menu)
{
  menu->priv = XFCE_MENU_GET_PRIVATE (menu);
  menu->priv->file = NULL;
  menu->priv->tree = NULL;
  menu->priv->directory = NULL;
  menu->priv->submenus = NULL;
  menu->priv->parent = NULL;
  menu->priv->pool = xfce_menu_item_pool_new ();
  menu->priv->layout = xfce_menu_layout_new ();

  /* Take reference on the menu item cache */
  menu->priv->cache = xfce_menu_item_cache_get_default ();
}



static void
xfce_menu_finalize (GObject *object)
{
  XfceMenu *menu = XFCE_MENU (object);

  /* Stop monitoring */
  xfce_menu_monitor_stop (menu);

  /* Destroy the menu tree */
  if (menu->priv->parent == NULL)
    xfce_menu_node_tree_free (menu->priv->tree);

  /* Free file */
  g_object_unref (menu->priv->file);

  /* Free directory */
  if (G_LIKELY (menu->priv->directory != NULL))
    g_object_unref (menu->priv->directory);

  /* Free submenus */
  g_slist_foreach (menu->priv->submenus, (GFunc) g_object_unref, NULL);
  g_slist_free (menu->priv->submenus);

  /* Free item pool */
  g_object_unref (menu->priv->pool);

  /* Free menu layout */
  g_object_unref (menu->priv->layout);

  /* Release item cache reference */
  g_object_unref (menu->priv->cache);

  (*G_OBJECT_CLASS (xfce_menu_parent_class)->finalize) (object);
}



static void
xfce_menu_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  XfceMenu *menu = XFCE_MENU (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, xfce_menu_get_file (menu));
      break;

    case PROP_DIRECTORY:
      g_value_set_object (value, xfce_menu_get_directory (menu));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_menu_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  XfceMenu *menu = XFCE_MENU (object);

  switch (prop_id)
    {
    case PROP_FILE:
      menu->priv->file = g_object_ref (g_value_get_object (value));
      break;

    case PROP_DIRECTORY:
      xfce_menu_set_directory (menu, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/**
 * xfce_menu_get_root:
 * @error : Return location for errors or %NULL.
 *
 * Loads the system's root menu. This may take some time as it involves
 * parsing and merging a lot of files. So if you call this function from a GUI
 * program it should be done in a way that won't block the user interface (e.g.
 * by using a worker thread).
 * The returned pointer needs to be released using
 * <informalexample><programlisting>
 * g_object_unref (menu);
 * </programlisting></informalexample>
 * when no longer needed.
 *
 * Return value: The system root menu. The menu has to be released when no 
 *               longer needed.
 **/
XfceMenu*
xfce_menu_get_root (GError **error)
{
  static XfceMenu *root_menu = NULL;
  GFile           *file;
  gchar           *filename;
  guint            n;

  if (G_UNLIKELY (root_menu == NULL))
    {
      /* Search for a usable root menu file */
      for (n = 0; n < G_N_ELEMENTS (XFCE_MENU_ROOT_SPECS) && root_menu == NULL; ++n)
        {
          /* Search for the root menu file */
          filename = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, XFCE_MENU_ROOT_SPECS[n]);
          if (G_UNLIKELY (filename == NULL))
            continue;

          /* Try to load the root menu from this file */
          file = g_file_new_for_unknown_input (filename, NULL);
          root_menu = xfce_menu_new (file, NULL);

          if (G_LIKELY (root_menu != NULL))
            {
              /* Add weak pointer on the menu */
              g_object_add_weak_pointer (G_OBJECT (root_menu), (gpointer) &root_menu);
            }

          g_object_unref (file);
          g_free (filename);
        }

      /* Check if we failed to load the root menu */
      if (G_UNLIKELY (root_menu == NULL))
        {
          /* Let the caller know there was no suitable file */
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, _("Failed to locate the system menu"));
        }
    }
  else
    g_object_ref (G_OBJECT (root_menu));
  
  return root_menu;
}



/**
 * xfce_menu_new:
 * @file  : A GFile containing the menu you want to load.
 * @error : return location for errors or %NULL.
 *
 * Parses a file and returns the menu structure found in this file. This
 * may involve parsing and merging of a lot of other files. So if you call this
 * function from a GUI program it should be done in a way that won't block the
 * user interface (e.g. by using a worker thread).
 * The returned pointer needs to be released using
 * <informalexample><programlisting>
 * g_object_unref (menu);
 * </programlisting></informalexample>
 * when it is not used anymore.
 *
 * Return value: Menu found in @file.
 **/
XfceMenu*
xfce_menu_new (GFile   *file,
               GError **error)
{
  XfceMenu *menu;

  g_return_val_if_fail (G_IS_FILE (file), NULL);

  /* Create new menu */
  menu = g_object_new (XFCE_TYPE_MENU, "file", file, NULL);

  /* Try to load the menu structure */
  if (!xfce_menu_load (menu, error))
    {
      g_object_unref (G_OBJECT (menu));
      return NULL;
    }

  return menu;
}



/**
 * xfce_menu_get_file:
 * @menu : a #XfceMenu.
 *
 * Returns the @GFile from which @menu was loaded. The caller is responsible
 * to decrease the reference on the returned @GFile with 
 * <informalexample><programlisting>
 * g_object_unref (file);
 * </programlisting></informalexample>
 * 
 * Return value: The @GFile from which @menu was loaded.
 */
GFile *
xfce_menu_get_file (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return g_object_ref (menu->priv->file);
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



/**
 * xfce_menu_get_name:
 * @menu : a #XfceMenu.
 *
 * Returns the name of @menu. In most cases this will be the 
 * contents of the &lt;Menu&gt; element. It may be useful for providing
 * a display name for the menu if it does not have a menu
 * directory.
 *
 * Return value: name of @menu.
 */
const gchar*
xfce_menu_get_name (XfceMenu *menu)
{
  const gchar *name = NULL;
  
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  g_node_traverse (menu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_name, &name);

  return name;
}



/**
 * xfce_menu_get_directory:
 * @menu : a #XfceMenu.
 *
 * Returns the #XfceMenuDirectory of @menu or %NULL. The menu
 * directory may contain a lot of useful information about 
 * the menu, like display name, desktop environments it should
 * show up in etc.
 *
 * Return value: #XfceMenuDirectory of @menu or %NULL.
 */
XfceMenuDirectory*
xfce_menu_get_directory (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return menu->priv->directory;
}



/**
 * xfce_menu_set_directory:
 * @menu      : a #XfceMenu.
 * @directory : a #XfceMenuDirectory.
 *
 * Replaces the #XfceMenuDirectory of @menu with @directory. This
 * may be useful if @menu has no directory or if you want to 
 * define your own directory for menus. Usually, there's no need
 * to call this function.
 */
void
xfce_menu_set_directory (XfceMenu          *menu,
                         XfceMenuDirectory *directory)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (XFCE_IS_MENU_DIRECTORY (directory));

  /* Abort if directories (TODO: and their locations) are equal */
  if (G_UNLIKELY (directory == menu->priv->directory))
    return;
  
  /* Destroy old directory */
  if (G_UNLIKELY (menu->priv->directory != NULL))
    g_object_unref (menu->priv->directory);

  /* Remove the floating reference and acquire a normal one */
  g_object_ref_sink (G_OBJECT (directory));

  /* Set the new directory */
  menu->priv->directory = directory;

  /* Notify listeners */
  g_object_notify (G_OBJECT (menu), "directory");
}



static gboolean
xfce_menu_load (XfceMenu *menu, 
                GError  **error)
{
  XfceMenuParser *parser;
  XfceMenuMerger *merger;
  GHashTable     *desktop_id_table;
  gboolean        success = TRUE;

  g_return_val_if_fail (XFCE_IS_MENU (menu), FALSE);

  parser = xfce_menu_parser_new (menu->priv->file);

  if (G_LIKELY (xfce_menu_parser_run (parser, NULL, error)))
    {
      merger = xfce_menu_merger_new (XFCE_MENU_TREE_PROVIDER (parser));

      if (G_UNLIKELY (xfce_menu_merger_run (merger, NULL, error)))
        menu->priv->tree = xfce_menu_tree_provider_get_tree (XFCE_MENU_TREE_PROVIDER (merger));
      else
        success = FALSE;

      g_object_unref (merger);
    }
  else
    success = FALSE;

  g_object_unref (parser);

  if (G_LIKELY (!success))
    return FALSE;

  /* Generate submenus */
  xfce_menu_resolve_menus (menu);

  /* Resolve the menu directory */
  xfce_menu_resolve_directory (menu);

  desktop_id_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  /* Load menu items */
  xfce_menu_collect_files (menu, desktop_id_table);
  xfce_menu_resolve_items (menu, desktop_id_table, FALSE);
  xfce_menu_resolve_items (menu, desktop_id_table, TRUE);

  /* Remove deleted menus */
  xfce_menu_remove_deleted_menus (menu);

  g_hash_table_unref (desktop_id_table);

  /* Start monitoring */
  xfce_menu_monitor_start (menu);

  return TRUE;
}



GSList*
xfce_menu_get_menus (XfceMenu *menu)
{
  GSList *menus = NULL;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  
  /* Copy submenu list */
  menus = g_slist_copy (menu->priv->submenus);

  /* Sort submenus */
  menus = g_slist_sort (menus, (GCompareFunc) xfce_menu_compare_items);

  return menus;
}



void
xfce_menu_add_menu (XfceMenu *menu,
                    XfceMenu *submenu)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (XFCE_IS_MENU (submenu));

  /* Remove floating reference and acquire a 'real' one */
  g_object_ref_sink (G_OBJECT (submenu));

  /* Append menu to the list */
  menu->priv->submenus = g_slist_append (menu->priv->submenus, submenu);

  /* TODO: Use property method here */
  submenu->priv->parent = menu;
}



XfceMenu*
xfce_menu_get_menu_with_name (XfceMenu    *menu,
                              const gchar *name)
{
  XfceMenu *result = NULL;
  XfceMenu *submenu;
  GSList   *iter;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  /* Iterate over the submenu list */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      submenu = XFCE_MENU (iter->data);

      /* End loop when a matching submenu is found */
      if (G_UNLIKELY (g_utf8_collate (xfce_menu_get_name (submenu), name) == 0))
        {
          result = submenu;
          break;
        }
    }

  return result;
}



XfceMenu *
xfce_menu_get_parent (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return menu->priv->parent;
}



static gboolean
collect_menus (GNode        *node,
               XfceMenuPair *pair)
{
  GNode   *self;
  GSList **list;

  self = pair->first;
  list = pair->second;

  if (node != self && xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_MENU)
    *list = g_slist_append (*list, node);

  return FALSE;
}



static void
xfce_menu_resolve_menus (XfceMenu *menu)
{
  XfceMenuPair pair;
  XfceMenu    *submenu;
  GSList      *menus = NULL;
  GSList      *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  pair.first = menu->priv->tree;
  pair.second = &menus;

  g_node_traverse (menu->priv->tree, G_LEVEL_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_menus, &pair);

  for (iter = menus; iter != NULL; iter = g_slist_next (iter))
    {
      submenu = g_object_new (XFCE_TYPE_MENU, "file", menu->priv->file, NULL);
      submenu->priv->tree = iter->data;

      xfce_menu_add_menu (menu, submenu);
      g_object_unref (submenu);
    }

  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_resolve_menus (iter->data);
}



static gboolean
collect_directories (GNode   *node,
                     GSList **list)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_DIRECTORY)
    *list = g_slist_prepend (*list, (gpointer) xfce_menu_node_tree_get_string (node));

  return FALSE;
}



static GSList *
xfce_menu_get_directories (XfceMenu *menu)
{
  GSList *dirs = NULL;

  /* Fetch all application directories */
  g_node_traverse (menu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_directories, &dirs);

  if (menu->priv->parent != NULL)
    dirs = g_slist_concat (dirs, xfce_menu_get_directories (menu->priv->parent));

  return dirs;
}



static void
xfce_menu_resolve_directory (XfceMenu *menu)
{
  GSList            *directories = NULL;
  GSList            *iter;
  XfceMenuDirectory *directory = NULL;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Determine all directories for this menu */
  directories = xfce_menu_get_directories (menu);

  /* Try to load one directory name after another */
  for (iter = directories; iter != NULL; iter = g_slist_next (iter))
    {
      /* Try to load the directory with this name */
      directory = xfce_menu_lookup_directory (menu, iter->data);

      if (directory != NULL)
        break;
    }

  if (G_LIKELY (directory != NULL)) 
    {
      /* Set the directory (assuming that we found at least one valid name) */
      menu->priv->directory = directory;
    }

  /* Free reverse list copy */
  g_slist_free (directories);

  /* ... and all submenus (recursively) */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_resolve_directory (iter->data);
}



static gboolean
collect_directory_dirs (GNode   *node,
                        GSList **list)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_DIRECTORY_DIR)
    *list = g_slist_prepend (*list, (gpointer) xfce_menu_node_tree_get_string (node));

  return FALSE;
}



static GSList *
xfce_menu_get_directory_dirs (XfceMenu *menu)
{
  GSList *dirs = NULL;

  /* Fetch all application directories */
  g_node_traverse (menu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_directory_dirs, &dirs);

  if (menu->priv->parent != NULL)
    dirs = g_slist_concat (dirs, xfce_menu_get_directory_dirs (menu->priv->parent));

  return dirs;
}



static XfceMenuDirectory *
xfce_menu_lookup_directory (XfceMenu    *menu,
                            const gchar *filename)
{
  XfceMenuDirectory *directory = NULL;
  GSList            *dirs = NULL;
  GSList            *iter;
  GFile             *file;
  GFile             *dir;
  gboolean           found = FALSE;
  
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  g_return_val_if_fail (filename != NULL, NULL);

  dirs = xfce_menu_get_directory_dirs (menu);

  /* Iterate through all directories */
  for (iter = dirs; iter != NULL && !found; iter = g_slist_next (iter))
    {
      dir = g_file_new_relative_to_file (iter->data, menu->priv->file);
      file = g_file_new_relative_to_file (filename, dir);

      /* Check if the file exists and is readable */
      if (G_LIKELY (g_file_query_exists (file, NULL)))
        {
          /* Load menu directory */
          directory = g_object_new (XFCE_TYPE_MENU_DIRECTORY, "file", file, NULL);

          /* Update search status */
          found = TRUE;
        }
      
      /* Destroy the file objects */
      g_object_unref (file);
      g_object_unref (dir);
    }

  /* Free reverse copy */
  g_slist_free (dirs);
  

  return directory;
}



static gboolean
collect_app_dirs (GNode   *node,
                  GSList **list)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_APP_DIR)
    *list = g_slist_prepend (*list, (gpointer) xfce_menu_node_tree_get_string (node));

  return FALSE;
}



static GSList *
xfce_menu_get_app_dirs (XfceMenu *menu)
{
  GSList *dirs = NULL;

  /* Fetch all application directories */
  g_node_traverse (menu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_app_dirs, &dirs);

  if (menu->priv->parent != NULL)
    dirs = g_slist_concat (dirs, xfce_menu_get_app_dirs (menu->priv->parent));

  return dirs;
}



static void
xfce_menu_collect_files (XfceMenu   *menu,
                         GHashTable *desktop_id_table)
{
  GSList *app_dirs = NULL;
  GSList *iter;
  GFile  *file;

  g_return_if_fail (XFCE_IS_MENU (menu));

  app_dirs = xfce_menu_get_app_dirs (menu);

  /* Collect desktop entry filenames */
  for (iter = app_dirs; iter != NULL; iter = g_slist_next (iter))
    {
      file = g_file_new_for_uri (iter->data);
      xfce_menu_collect_files_from_path (menu, desktop_id_table, file, NULL);
      g_object_unref (file);
    }

  /* Free directory list */
  g_slist_free (app_dirs);

  /* Collect filenames for submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_collect_files (iter->data, desktop_id_table);
}



static void
xfce_menu_collect_files_from_path (XfceMenu    *menu,
                                   GHashTable  *desktop_id_table,
                                   GFile       *dir,
                                   const gchar *id_prefix)
{
  GFileEnumerator *enumerator;
  GFileInfo       *file_info;
  GFile           *file;
  gchar           *basename;
  gchar           *new_id_prefix;
  gchar           *desktop_id;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Skip directory if it doesn't exist */
  if (G_UNLIKELY (!g_file_query_exists (dir, NULL)))
    return;

  /* Skip directory if it's not a directory */
  if (G_UNLIKELY (g_file_query_file_type (dir, 
                                          G_FILE_QUERY_INFO_NONE, 
                                          NULL) != G_FILE_TYPE_DIRECTORY))
    {
      return;
    }

  /* Open directory for reading */
  enumerator = g_file_enumerate_children (dir, "standard::name,standard::type",
                                          G_FILE_QUERY_INFO_NONE, NULL, NULL);

  /* Abort if directory cannot be opened */
  if (G_UNLIKELY (enumerator == NULL))
    return;

  /* Read file by file */
  while (TRUE)
    {
      file_info = g_file_enumerator_next_file (enumerator, NULL, NULL);

      if (G_UNLIKELY (file_info == NULL))
        break;

      file = g_file_resolve_relative_path (dir, g_file_info_get_name (file_info));
      basename = g_file_get_basename (file);

      /* Treat files and directories differently */
      if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY)
        {
          /* Create new desktop-file id prefix */
          if (G_LIKELY (id_prefix == NULL))
            new_id_prefix = g_strdup (basename);
          else
            new_id_prefix = g_strjoin ("-", id_prefix, basename, NULL);

          /* Collect files in the directory */
          xfce_menu_collect_files_from_path (menu, desktop_id_table, file, new_id_prefix);

          /* Free id prefix */
          g_free (new_id_prefix);
        }
      else
        {
          /* Skip all filenames which do not end with .desktop */
          if (G_LIKELY (g_str_has_suffix (basename, ".desktop")))
            {
              /* Create desktop-file id */
              if (G_LIKELY (id_prefix == NULL))
                desktop_id = g_strdup (basename);
              else
                desktop_id = g_strjoin ("-", id_prefix, basename, NULL);

              /* Insert into the files hash table if the desktop-file id does not exist in there yet */
              if (G_LIKELY (g_hash_table_lookup (desktop_id_table, desktop_id) == NULL))
                g_hash_table_insert (desktop_id_table, desktop_id, g_file_get_uri (file));
              else
                g_free (desktop_id);
            }
        }

      /* Free absolute path */
      g_free (basename);

      /* Destroy file */
      g_object_unref (file);
    }

  g_object_unref (enumerator);
}



static gboolean
collect_only_unallocated (GNode    *node,
                          gboolean *only_unallocated)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED)
    {
      *only_unallocated = TRUE;
      return TRUE;
    }
  else
    return FALSE;
}



static gboolean
collect_rules (GNode   *node,
               GSList **list)
{
  XfceMenuNodeType type;

  type = xfce_menu_node_tree_get_node_type (node);

  if (type == XFCE_MENU_NODE_TYPE_INCLUDE ||
      type == XFCE_MENU_NODE_TYPE_EXCLUDE)
    {
      *list = g_slist_append (*list, node);
    }

  return FALSE;
}



static void
xfce_menu_resolve_items (XfceMenu   *menu,
                         GHashTable *desktop_id_table,
                         gboolean    only_unallocated)
{
  GSList  *rules = NULL;
  GSList  *iter;
  gboolean menu_only_unallocated = FALSE;

  g_return_if_fail (menu != NULL && XFCE_IS_MENU (menu));

  g_node_traverse (menu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                   (GNodeTraverseFunc) collect_only_unallocated, &menu_only_unallocated);

  /* Resolve items in this menu (if it matches the only_unallocated argument.
   * This means that in the first pass, all items of menus without 
   * <OnlyUnallocated /> are resolved and in the second pass, only items of 
   * menus with <OnlyUnallocated /> are resolved */
  if (menu_only_unallocated == only_unallocated)
    {
      g_node_traverse (menu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                       (GNodeTraverseFunc) collect_rules, &rules);

      /* Iterate over all rules */
      for (iter = rules; iter != NULL; iter = g_slist_next (iter))
        {
          if (G_LIKELY (xfce_menu_node_tree_get_node_type (iter->data) == XFCE_MENU_NODE_TYPE_INCLUDE))
            {
              /* Resolve available items and match them against this rule */
              xfce_menu_resolve_items_by_rule (menu, desktop_id_table, iter->data);
            }
          else
            {
              /* Remove all items matching this exclude rule from the item pool */
              xfce_menu_item_pool_apply_exclude_rule (menu->priv->pool, iter->data);
            }
        }
    }

  /* Iterate over all submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      /* Resolve items of the submenu */
      xfce_menu_resolve_items (XFCE_MENU (iter->data), desktop_id_table, only_unallocated);
    }
}



static void
xfce_menu_resolve_items_by_rule (XfceMenu   *menu,
                                 GHashTable *desktop_id_table,
                                 GNode      *node)
{
  XfceMenuPair pair;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Store menu and rule pointer in the pair */
  pair.first = menu;
  pair.second = node;

  /* Try to insert each of the collected desktop entry filenames into the menu */
  g_hash_table_foreach (desktop_id_table, (GHFunc) xfce_menu_resolve_item_by_rule, &pair);
}



static void
xfce_menu_resolve_item_by_rule (const gchar  *desktop_id,
                                const gchar  *uri,
                                XfceMenuPair *data)
{
  XfceMenuItem *item = NULL;
  XfceMenu     *menu = NULL;
  GNode        *node = NULL;
  gboolean      only_unallocated = FALSE;

  g_return_if_fail (XFCE_IS_MENU (data->first));
  g_return_if_fail (data->second != NULL);

  /* Restore menu and rule from the data pair */
  menu = data->first;
  node = data->second;

  /* Try to load the menu item from the cache */
  item = xfce_menu_item_cache_lookup (menu->priv->cache, uri, desktop_id);

  if (G_LIKELY (item != NULL))
    {
      g_node_traverse (menu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                       (GNodeTraverseFunc) collect_only_unallocated, &only_unallocated);

      /* Only include item if menu not only includes unallocated items
       * or if the item is not allocated yet */
      if (!only_unallocated || xfce_menu_item_get_allocated (item) == 0)
        {
          /* Add item to the pool if it matches the include rule */
          if (G_LIKELY (xfce_menu_node_tree_rule_matches (node, item)))
            xfce_menu_item_pool_insert (menu->priv->pool, item);
        }
    }
}



static gboolean
collect_deleted (GNode    *node,
                 gboolean *deleted)
{
  if (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_DELETED)
    {
      *deleted = TRUE;
      return TRUE;
    }
  else
    return FALSE;
}



static void
xfce_menu_remove_deleted_menus (XfceMenu *menu)
{
  XfceMenu *submenu;
  GSList   *iter;
  gboolean  deleted;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Note: There's a limitation: if the root menu has a <Deleted/> we
   * can't just free the pointer here. Therefor we only check child menus. */

  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      submenu = iter->data;
      deleted = FALSE;

      /* Check whether there is a <Deleted/> element */
      g_node_traverse (submenu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                       (GNodeTraverseFunc) collect_deleted, &deleted);

      /* Determine whether this submenu was deleted */
      if (G_LIKELY (submenu->priv->directory != NULL))
        deleted = deleted || xfce_menu_directory_get_hidden (submenu->priv->directory);

      /* Remove submenu if it is deleted, otherwise check submenus of the submenu */
      if (G_UNLIKELY (deleted))
        {
          /* Remove submenu from the list ... */
          menu->priv->submenus = g_slist_remove_link (menu->priv->submenus, iter);

          /* ... and destroy it */
          g_object_unref (submenu);
        }
      else
        xfce_menu_remove_deleted_menus (submenu);
    }
}



XfceMenuItemPool*
xfce_menu_get_item_pool (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  return menu->priv->pool;
}



static void
items_collect (const gchar  *desktop_id,
               XfceMenuItem *item,
               GSList      **listp)
{
  *listp = g_slist_prepend (*listp, item);
}



/**
 * xfce_menu_get_items:
 * @menu : a #XfceMenu.
 *
 * Convenience wrapper around xfce_menu_get_item_pool(), which simply returns the
 * #XfceMenuItem<!---->s contained within the associated item pool as singly linked
 * list.
 *
 * The caller is responsible to free the returned list using
 * <informalexample><programlisting>
 * g_slist_free (list);
 * </programlisting></informalexample>
 * when no longer needed.
 * 
 * Return value: the list of #XfceMenuItem<!---->s within this menu.
 **/
GSList*
xfce_menu_get_items (XfceMenu *menu)
{
  GSList *items = NULL;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  /* Collect the items in the pool */
  xfce_menu_item_pool_foreach (menu->priv->pool, (GHFunc) items_collect, &items);

  /* Sort items */
  items = g_slist_sort (items, (GCompareFunc) xfce_menu_compare_items);

  return items;
}



gboolean
xfce_menu_has_layout (XfceMenu *menu)
{
  GSList *nodes;

  g_return_val_if_fail (XFCE_IS_MENU (menu), FALSE);
  g_return_val_if_fail (XFCE_IS_MENU_LAYOUT (menu->priv->layout), FALSE);

  /* Fetch layout nodes */
  nodes = xfce_menu_layout_get_nodes (menu->priv->layout);

  /* Menu is supposed to have no layout when the nodes list is empty */
  return g_slist_length (nodes) > 0;
}



static void
layout_elements_collect (GSList        **dest_list,
                         GSList         *src_list,
                         XfceMenuLayout *layout)
{
  XfceMenuItem *item;
  XfceMenu     *menu;
  GSList       *iter;

  for (iter = src_list; iter != NULL; iter = g_slist_next (iter))
    {
      if (XFCE_IS_MENU (iter->data))
        {
          menu = XFCE_MENU (iter->data);

          if (G_LIKELY (!xfce_menu_layout_get_menuname_used (layout, xfce_menu_get_name (menu))))
            *dest_list = g_slist_append (*dest_list, iter->data);
        }
      else if (XFCE_IS_MENU_ITEM (iter->data))
        {
          item = XFCE_MENU_ITEM (iter->data);

          if (G_LIKELY (!xfce_menu_layout_get_filename_used (layout, xfce_menu_item_get_desktop_id (item))))
            *dest_list = g_slist_append (*dest_list, iter->data);
        }
    }
}



GSList*
xfce_menu_get_layout_elements (XfceMenu *menu)
{
  GSList *items = NULL;
  GSList *menu_items;
  GSList *nodes;
  GSList *iter;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  /* Return NULL if there is no layout */
  if (G_UNLIKELY (!xfce_menu_has_layout (menu)))
    return NULL;

  /* Fetch layout nodes */
  nodes = xfce_menu_layout_get_nodes (menu->priv->layout);

  /* Process layout nodes in order */
  for (iter = nodes; iter != NULL; iter = g_slist_next (iter))
    {
      XfceMenuLayoutNode     *node = (XfceMenuLayoutNode *)iter->data;
      XfceMenuLayoutNodeType  type;
      XfceMenuLayoutMergeType merge_type;
      XfceMenuItem           *item;
      XfceMenu               *submenu;

      /* Determine layout node type */
      type = xfce_menu_layout_node_get_type (node);

      if (type == XFCE_MENU_LAYOUT_NODE_FILENAME)
        {
          /* Search for desktop ID in the item pool */
          item = xfce_menu_item_pool_lookup (menu->priv->pool, xfce_menu_layout_node_get_filename (node));

          /* If the item with this desktop ID is included in the menu, append it to the list */
          if (G_LIKELY (item != NULL))
            items = g_slist_append (items, item);
        }
      if (type == XFCE_MENU_LAYOUT_NODE_MENUNAME)
        {
          /* Search submenu with this name */
          submenu = xfce_menu_get_menu_with_name (menu, xfce_menu_layout_node_get_menuname (node));

          /* If there is such a menu, append it to the list */
          if (G_LIKELY (submenu != NULL))
            items = g_slist_append (items, submenu);
        }
      else if (type == XFCE_MENU_LAYOUT_NODE_SEPARATOR)
        {
          /* Append separator to the list */
          items = g_slist_append (items, xfce_menu_separator_get_default ());
        }
      else if (type == XFCE_MENU_LAYOUT_NODE_MERGE)
        {
          /* Determine merge type */
          merge_type = xfce_menu_layout_node_get_merge_type (node);

          if (merge_type == XFCE_MENU_LAYOUT_MERGE_ALL)
            {
              /* Get all menu items of this menu */
              menu_items = xfce_menu_get_items (menu);
              
              /* Append submenus */
              menu_items = g_slist_concat (menu_items, xfce_menu_get_menus (menu));

              /* Sort menu items */
              menu_items = g_slist_sort (menu_items, (GCompareFunc) xfce_menu_compare_items);

              /* Append menu items to the returned item list */
              layout_elements_collect (&items, menu_items, menu->priv->layout);
            }
          else if (merge_type == XFCE_MENU_LAYOUT_MERGE_FILES)
            {
              /* Get all menu items of this menu */
              menu_items = xfce_menu_get_items (menu);

              /* Append menu items to the returned item list */
              layout_elements_collect (&items, menu_items, menu->priv->layout);
            }
          else if (merge_type == XFCE_MENU_LAYOUT_MERGE_MENUS)
            {
              /* Get all submenus */
              menu_items = xfce_menu_get_menus (menu);

              /* Append submenus to the returned item list */
              layout_elements_collect (&items, menu_items, menu->priv->layout);
            }
        }
    }
  
  return items;
}



static gint
xfce_menu_compare_items (gconstpointer *a,
                         gconstpointer *b)
{
  return g_utf8_collate (xfce_menu_element_get_name (XFCE_MENU_ELEMENT (a)), 
                         xfce_menu_element_get_name (XFCE_MENU_ELEMENT (b)));
}



static const gchar*
xfce_menu_get_element_name (XfceMenuElement *element)
{
  XfceMenu    *menu;
  const gchar *name = NULL;

  g_return_val_if_fail (XFCE_IS_MENU (element), NULL);

  menu = XFCE_MENU (element);

  /* Try directory name first */
  if (menu->priv->directory != NULL)
    name = xfce_menu_directory_get_name (menu->priv->directory);

  /* Otherwise use the menu name as a fallback */
  if (name == NULL)
    name = xfce_menu_get_name (menu);

  return name;
}



static const gchar*
xfce_menu_get_element_icon_name (XfceMenuElement *element)
{
  XfceMenu *menu;
  
  g_return_val_if_fail (XFCE_IS_MENU (element), NULL);

  menu = XFCE_MENU (element);

  if (menu->priv->directory == NULL)
    return NULL;
  else
    return xfce_menu_directory_get_icon (menu->priv->directory);
}



static void
item_monitor_start (const gchar  *desktop_id,
                    XfceMenuItem *item,
                    XfceMenu     *menu)
{
  xfce_menu_monitor_add_item (menu, item);
}



static void
xfce_menu_monitor_start (XfceMenu *menu)
{
  GSList *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

#if 0
  /* Monitor the menu file */
  if (G_LIKELY (xfce_menu_monitor_has_flags (XFCE_MENU_MONITOR_MENU_FILES)))
    xfce_menu_monitor_add_file (menu, menu->priv->filename);

  /* Monitor the menu directory file */
  if (G_LIKELY (XFCE_IS_MENU_DIRECTORY (menu->priv->directory) && xfce_menu_monitor_has_flags (XFCE_MENU_MONITOR_DIRECTORY_FILES)))
    xfce_menu_monitor_add_file (menu, xfce_menu_directory_get_filename (menu->priv->directory));

  /* Monitor the application directories */
  if (G_LIKELY (xfce_menu_monitor_has_flags (XFCE_MENU_MONITOR_DIRECTORIES)))
    for (iter = menu->priv->app_dirs; iter != NULL; iter = g_slist_next (iter))
      xfce_menu_monitor_add_directory (menu, (const gchar *)iter->data);
#endif

  /* Monitor items in the menu pool */
  if (G_LIKELY (xfce_menu_monitor_has_flags (XFCE_MENU_MONITOR_DESKTOP_FILES)))
    xfce_menu_item_pool_foreach (menu->priv->pool, (GHFunc) item_monitor_start, menu);

  /* Monitor items in submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_monitor_start (XFCE_MENU (iter->data));
}



static void
item_monitor_stop (const gchar  *desktop_id,
                   XfceMenuItem *item,
                   XfceMenu     *menu)
{
  xfce_menu_monitor_remove_item (menu, item);
}



static void
xfce_menu_monitor_stop (XfceMenu *menu)
{
  GSList *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Stop monitoring items in submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_monitor_stop (XFCE_MENU (iter->data));

  /* Stop monitoring the items */
  xfce_menu_item_pool_foreach (menu->priv->pool, (GHFunc) item_monitor_stop, menu);

#if 0
  /* Stop monitoring the application directories */
  for (iter = menu->priv->app_dirs; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_monitor_remove_directory (menu, (const gchar *)iter->data);

  /* Stop monitoring the menu directory file */
  if (XFCE_IS_MENU_DIRECTORY (menu->priv->directory))
    xfce_menu_monitor_remove_file (menu, xfce_menu_directory_get_filename (menu->priv->directory));

  /* Stop monitoring the menu file */
  xfce_menu_monitor_remove_file (menu, menu->priv->filename);
#endif
}

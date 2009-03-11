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
static void               xfce_menu_set_directory                          (XfceMenu              *menu,
                                                                            XfceMenuDirectory     *directory);
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
  GList             *submenus;

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
  g_list_foreach (menu->priv->submenus, (GFunc) g_object_unref, NULL);
  g_list_free (menu->priv->submenus);

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
 * xfce_menu_new:
 * @filename  : Path/URI of the .menu file you want to load.
 *
 * Creates a new #XfceMenu for the .menu file referred to by @filename.
 * This operation only fails if the filename is NULL. To load the menu 
 * tree from the file, you need to call xfce_menu_load() with the
 * returned #XfceMenu. 
 *
 * <informalexample><programlisting>
 * XfceMenu *menu = xfce_menu_new (filename);
 * 
 * if (xfce_menu_load (menu, &error))
 *   ...
 * else
 *   ...
 *
 * g_object_unref (menu);
 * </programlisting></informalexample>
 *
 * The caller is responsible to destroy the returned #XfceMenu
 * using g_object_unref().
 *
 * Return value: a new #XfceMenu for @filename.
 **/
XfceMenu*
xfce_menu_new (const gchar *filename)
{
  XfceMenu *menu;
  GFile    *file;

  g_return_val_if_fail (file != NULL, NULL);

  /* Create new menu */
  file = g_file_new_for_unknown_input (filename, NULL);
  menu = g_object_new (XFCE_TYPE_MENU, "file", file, NULL);
  g_object_unref (file);

  return menu;
}



/**
 * xfce_menu_new_for_file:
 * @file  : #GFile for the .menu file you want to load.
 *
 * Creates a new #XfceMenu for the .menu file referred to by @file.
 * This operation only fails @file is invalid. To load the menu 
 * tree from the file, you need to call xfce_menu_load() with the
 * returned #XfceMenu. 
 *
 * The caller is responsible to destroy the returned #XfceMenu
 * using g_object_unref().
 *
 * For more information about the usage @see xfce_menu_new().
 *
 * Return value: a new #XfceMenu for @file.
 **/
XfceMenu*
xfce_menu_new_for_file (GFile *file)
{
  g_return_val_if_fail (G_IS_FILE (file), NULL);
  return g_object_new (XFCE_TYPE_MENU, "file", file, NULL);
}



/**
 * xfce_menu_new_applications:
 *
 * Creates a new #XfceMenu for the applications.menu file
 * which is being used to display installed applications.
 *
 * For more information about the usage @see xfce_menu_new().
 *
 * Return value: a new #XfceMenu for applications.menu.
 **/
XfceMenu*
xfce_menu_new_applications (void)
{
  XfceMenu *menu = NULL;
  GFile    *file;
  gchar    *filename;
  guint     n;

  /* Search for a usable applications menu file */
  for (n = 0; menu == NULL && n < G_N_ELEMENTS (XFCE_MENU_ROOT_SPECS); ++n)
    {
      /* Search for the applications menu file */
      filename = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, XFCE_MENU_ROOT_SPECS[n]);

      /* Create menu if the file exists */
      if (G_UNLIKELY (filename != NULL))
        {
          file = g_file_new_for_unknown_input (filename, NULL);
          menu = xfce_menu_new_for_file (file);
          g_object_unref (file);
        }

      g_free (filename);
    }
  
  return menu;
}



/**
 * xfce_menu_get_file:
 * @menu : a #XfceMenu.
 *
 * Returns the #GFile of @menu. It refers to the .menu file from which 
 * @menu was or will be loaded.
 * 
 * Return value: the @GFile of @menu.
 */
GFile *
xfce_menu_get_file (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return g_object_ref (menu->priv->file);
}



static const gchar *
xfce_menu_get_name (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return xfce_menu_node_tree_get_string_child (menu->priv->tree,
                                               XFCE_MENU_NODE_TYPE_NAME);
}



/**
 * xfce_menu_get_directory:
 * @menu : a #XfceMenu.
 *
 * Returns the #XfceMenuDirectory of @menu or %NULL if the &lt;Menu&gt;
 * element that corresponds to @menu has no valid &lt;Directory&gt; element.
 * The menu directory may contain a lot of useful information about 
 * the menu like the display and icon name, desktop environments it 
 * should show up in etc.
 *
 * Return value: #XfceMenuDirectory of @menu or %NULL if
 *               @menu has no valid directory element.
 */
XfceMenuDirectory*
xfce_menu_get_directory (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return menu->priv->directory;
}



static void
xfce_menu_set_directory (XfceMenu          *menu,
                         XfceMenuDirectory *directory)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (XFCE_IS_MENU_DIRECTORY (directory));

  /* Abort if directories are equal */
  if (G_UNLIKELY (xfce_menu_directory_equal (directory, menu->priv->directory)))
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



/**
 * xfce_menu_load:
 * @menu        : a #XfceMenu
 * @cancellable : a #GCancellable
 * @error       : #GError return location
 *
 * This function loads the entire menu tree from the file referred to 
 * by @menu. It resolves merges, moves and everything else defined
 * in the menu specification. The resulting tree information is
 * stored within @menu and can be accessed using the public #XfceMenu 
 * API afterwards.
 *
 * @cancellable can be used to handle blocking I/O when reading data
 * from files during the loading process. 
 *
 * @error should either be NULL or point to a #GError return location
 * where errors should be stored in.
 *
 * Return value: %TRUE if the menu was loaded successfully or
 *               %FALSE if there was an error or the process was 
 *               cancelled.
 **/
gboolean
xfce_menu_load (XfceMenu     *menu, 
                GCancellable *cancellable,
                GError      **error)
{
  XfceMenuParser *parser;
  XfceMenuMerger *merger;
  GHashTable     *desktop_id_table;
  gboolean        success = TRUE;

  g_return_val_if_fail (XFCE_IS_MENU (menu), FALSE);

  parser = xfce_menu_parser_new (menu->priv->file);

  if (G_LIKELY (xfce_menu_parser_run (parser, cancellable, error)))
    {
      merger = xfce_menu_merger_new (XFCE_MENU_TREE_PROVIDER (parser));

      if (G_UNLIKELY (xfce_menu_merger_run (merger, cancellable, error)))
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



GList *
xfce_menu_get_menus (XfceMenu *menu)
{
  GList *menus = NULL;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  
  /* Copy submenu list */
  menus = g_list_copy (menu->priv->submenus);

  /* Sort submenus */
  menus = g_list_sort (menus, (GCompareFunc) xfce_menu_compare_items);

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
  menu->priv->submenus = g_list_append (menu->priv->submenus, submenu);

  /* TODO: Use property method here */
  submenu->priv->parent = menu;
}



XfceMenu*
xfce_menu_get_menu_with_name (XfceMenu    *menu,
                              const gchar *name)
{
  XfceMenu *result = NULL;
  GList   *iter;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  /* Iterate over the submenu list */
  for (iter = menu->priv->submenus; result == NULL && iter != NULL; iter = g_list_next (iter))
    {
      /* End loop when a matching submenu is found */
      if (G_UNLIKELY (g_utf8_collate (xfce_menu_get_name (iter->data), name) == 0))
        result = iter->data;
    }

  return result;
}



XfceMenu *
xfce_menu_get_parent (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return menu->priv->parent;
}



static void
xfce_menu_resolve_menus (XfceMenu *menu)
{
  XfceMenu *submenu;
  GList    *menus = NULL;
  GList    *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  menus = xfce_menu_node_tree_get_child_nodes (menu->priv->tree, XFCE_MENU_NODE_TYPE_MENU, FALSE);

  for (iter = menus; iter != NULL; iter = g_list_next (iter))
    {
      submenu = g_object_new (XFCE_TYPE_MENU, "file", menu->priv->file, NULL);
      submenu->priv->tree = iter->data;
      xfce_menu_add_menu (menu, submenu);
      g_object_unref (submenu);
    }

  g_list_free (menus);

  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    xfce_menu_resolve_menus (iter->data);
}



static GList *
xfce_menu_get_directories (XfceMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = xfce_menu_node_tree_get_string_children (menu->priv->tree, 
                                                  XFCE_MENU_NODE_TYPE_DIRECTORY, 
                                                  TRUE);

  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, xfce_menu_get_directories (menu->priv->parent));

  return dirs;
}



static void
xfce_menu_resolve_directory (XfceMenu *menu)
{
  XfceMenuDirectory *directory = NULL;
  GList             *directories = NULL;
  GList             *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Determine all directories for this menu */
  directories = xfce_menu_get_directories (menu);

  /* Try to load one directory name after another */
  for (iter = directories; directory == NULL && iter != NULL; iter = g_list_next (iter))
    {
      /* Try to load the directory with this name */
      directory = xfce_menu_lookup_directory (menu, iter->data);
    }

  if (G_LIKELY (directory != NULL)) 
    {
      /* Set the directory (assuming that we found at least one valid name) */
      menu->priv->directory = directory;
    }

  /* Free reverse list copy */
  g_list_free (directories);

  /* Resolve directories of submenus recursively */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    xfce_menu_resolve_directory (iter->data);
}



static GList *
xfce_menu_get_directory_dirs (XfceMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = xfce_menu_node_tree_get_string_children (menu->priv->tree, 
                                                  XFCE_MENU_NODE_TYPE_DIRECTORY_DIR, 
                                                  TRUE);

  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, xfce_menu_get_directory_dirs (menu->priv->parent));

  return dirs;
}



static XfceMenuDirectory *
xfce_menu_lookup_directory (XfceMenu    *menu,
                            const gchar *filename)
{
  XfceMenuDirectory *directory = NULL;
  GList             *dirs = NULL;
  GList             *iter;
  GFile             *file;
  GFile             *dir;
  gboolean           found = FALSE;
  
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  g_return_val_if_fail (filename != NULL, NULL);

  dirs = xfce_menu_get_directory_dirs (menu);

  /* Iterate through all directories */
  for (iter = dirs; !found && iter != NULL; iter = g_list_next (iter))
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
  g_list_free (dirs);

  return directory;
}



static GList *
xfce_menu_get_app_dirs (XfceMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = xfce_menu_node_tree_get_string_children (menu->priv->tree, 
                                                  XFCE_MENU_NODE_TYPE_APP_DIR,
                                                  TRUE);

  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, xfce_menu_get_app_dirs (menu->priv->parent));

  return dirs;
}



static void
xfce_menu_collect_files (XfceMenu   *menu,
                         GHashTable *desktop_id_table)
{
  GList *app_dirs = NULL;
  GList *iter;
  GFile  *file;

  g_return_if_fail (XFCE_IS_MENU (menu));

  app_dirs = xfce_menu_get_app_dirs (menu);

  /* Collect desktop entry filenames */
  for (iter = app_dirs; iter != NULL; iter = g_list_next (iter))
    {
      file = g_file_new_for_uri (iter->data);
      xfce_menu_collect_files_from_path (menu, desktop_id_table, file, NULL);
      g_object_unref (file);
    }

  /* Free directory list */
  g_list_free (app_dirs);

  /* Collect filenames for submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
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

              /* Insert into the files hash table if the desktop-file id does not exist there yet */
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
collect_rules (GNode   *node,
               GList **list)
{
  XfceMenuNodeType type;

  type = xfce_menu_node_tree_get_node_type (node);

  if (type == XFCE_MENU_NODE_TYPE_INCLUDE ||
      type == XFCE_MENU_NODE_TYPE_EXCLUDE)
    {
      *list = g_list_append (*list, node);
    }

  return FALSE;
}



static void
xfce_menu_resolve_items (XfceMenu   *menu,
                         GHashTable *desktop_id_table,
                         gboolean    only_unallocated)
{
  GList  *rules = NULL;
  GList  *iter;
  gboolean menu_only_unallocated = FALSE;

  g_return_if_fail (menu != NULL && XFCE_IS_MENU (menu));

  menu_only_unallocated = xfce_menu_node_tree_get_boolean_child (menu->priv->tree, 
                                                                 XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED);

  /* Resolve items in this menu (if it matches the only_unallocated argument.
   * This means that in the first pass, all items of menus without 
   * <OnlyUnallocated /> are resolved and in the second pass, only items of 
   * menus with <OnlyUnallocated /> are resolved */
  if (menu_only_unallocated == only_unallocated)
    {
      g_node_traverse (menu->priv->tree, G_IN_ORDER, G_TRAVERSE_ALL, 2,
                       (GNodeTraverseFunc) collect_rules, &rules);

      /* Iterate over all rules */
      for (iter = rules; iter != NULL; iter = g_list_next (iter))
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
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
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
      only_unallocated = xfce_menu_node_tree_get_boolean_child (menu->priv->tree,
                                                                XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED);

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



static void
xfce_menu_remove_deleted_menus (XfceMenu *menu)
{
  XfceMenu *submenu;
  GList   *iter;
  gboolean  deleted;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Note: There's a limitation: if the root menu has a <Deleted/> we
   * can't just free the pointer here. Therefor we only check child menus. */

  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    {
      submenu = iter->data;

      /* Check whether there is a <Deleted/> element */
      deleted = xfce_menu_node_tree_get_boolean_child (submenu->priv->tree, 
                                                       XFCE_MENU_NODE_TYPE_DELETED);

      /* Determine whether this submenu was deleted */
      if (G_LIKELY (submenu->priv->directory != NULL))
        deleted = deleted || xfce_menu_directory_get_hidden (submenu->priv->directory);

      /* Remove submenu if it is deleted, otherwise check submenus of the submenu */
      if (G_UNLIKELY (deleted))
        {
          /* Remove submenu from the list ... */
          menu->priv->submenus = g_list_remove_link (menu->priv->submenus, iter);

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
               GList       **listp)
{
  *listp = g_list_prepend (*listp, item);
}



/**
 * xfce_menu_get_items:
 * @menu : a #XfceMenu.
 *
 * Returns all #XfceMenuItem<!---->s included in @menu. The items are 
 * sorted by their display names in ascending order.
 *
 * The caller is responsible to free the returned list using
 * <informalexample><programlisting>
 * g_list_free (list);
 * </programlisting></informalexample>
 * when no longer needed.
 * 
 * Return value: list of #XfceMenuItem<!---->s included in @menu.
 **/
GList *
xfce_menu_get_items (XfceMenu *menu)
{
  GList *items = NULL;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  /* Collect the items in the pool */
  xfce_menu_item_pool_foreach (menu->priv->pool, (GHFunc) items_collect, &items);

  /* Sort items */
  items = g_list_sort (items, (GCompareFunc) xfce_menu_compare_items);

  return items;
}



/**
 * xfce_menu_has_layout:
 * @menu : a #XfceMenu
 *
 * Checks whether @menu has a <Layout> or <DefaultLayout> element. If
 * this is the case, you can call xfce_menu_get_layout_elements() to get
 * the submenus, separators and items arranged according to the layout.
 *
 * Return value: %TRUE if @menu has a layout, %FALSE if not.
 **/
gboolean
xfce_menu_has_layout (XfceMenu *menu)
{
  GList *nodes;

  g_return_val_if_fail (XFCE_IS_MENU (menu), FALSE);
  g_return_val_if_fail (XFCE_IS_MENU_LAYOUT (menu->priv->layout), FALSE);

  /* Fetch layout nodes */
  nodes = xfce_menu_layout_get_nodes (menu->priv->layout);

  /* Menu is supposed to have no layout when the nodes list is empty */
  return g_list_length (nodes) > 0;
}



static void
layout_elements_collect (GList        **dest_list,
                         GList         *src_list,
                         XfceMenuLayout *layout)
{
  XfceMenuItem *item;
  XfceMenu     *menu;
  GList       *iter;

  for (iter = src_list; iter != NULL; iter = g_list_next (iter))
    {
      if (XFCE_IS_MENU (iter->data))
        {
          menu = XFCE_MENU (iter->data);

          if (G_LIKELY (!xfce_menu_layout_get_menuname_used (layout, xfce_menu_get_name (menu))))
            *dest_list = g_list_append (*dest_list, iter->data);
        }
      else if (XFCE_IS_MENU_ITEM (iter->data))
        {
          item = XFCE_MENU_ITEM (iter->data);

          if (G_LIKELY (!xfce_menu_layout_get_filename_used (layout, xfce_menu_item_get_desktop_id (item))))
            *dest_list = g_list_append (*dest_list, iter->data);
        }
    }
}



GList *
xfce_menu_get_layout_elements (XfceMenu *menu)
{
  GList *items = NULL;
  GList *menu_items;
  GList *nodes;
  GList *iter;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  /* Return NULL if there is no layout */
  if (G_UNLIKELY (!xfce_menu_has_layout (menu)))
    return NULL;

  /* Fetch layout nodes */
  nodes = xfce_menu_layout_get_nodes (menu->priv->layout);

  /* Process layout nodes in order */
  for (iter = nodes; iter != NULL; iter = g_list_next (iter))
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
            items = g_list_append (items, item);
        }
      if (type == XFCE_MENU_LAYOUT_NODE_MENUNAME)
        {
          /* Search submenu with this name */
          submenu = xfce_menu_get_menu_with_name (menu, xfce_menu_layout_node_get_menuname (node));

          /* If there is such a menu, append it to the list */
          if (G_LIKELY (submenu != NULL))
            items = g_list_append (items, submenu);
        }
      else if (type == XFCE_MENU_LAYOUT_NODE_SEPARATOR)
        {
          /* Append separator to the list */
          items = g_list_append (items, xfce_menu_separator_get_default ());
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
              menu_items = g_list_concat (menu_items, xfce_menu_get_menus (menu));

              /* Sort menu items */
              menu_items = g_list_sort (menu_items, (GCompareFunc) xfce_menu_compare_items);

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
  GList *iter;

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
    for (iter = menu->priv->app_dirs; iter != NULL; iter = g_list_next (iter))
      xfce_menu_monitor_add_directory (menu, (const gchar *)iter->data);
#endif

  /* Monitor items in the menu pool */
  if (G_LIKELY (xfce_menu_monitor_has_flags (XFCE_MENU_MONITOR_DESKTOP_FILES)))
    xfce_menu_item_pool_foreach (menu->priv->pool, (GHFunc) item_monitor_start, menu);

  /* Monitor items in submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
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
  GList *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Stop monitoring items in submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    xfce_menu_monitor_stop (XFCE_MENU (iter->data));

  /* Stop monitoring the items */
  xfce_menu_item_pool_foreach (menu->priv->pool, (GHFunc) item_monitor_stop, menu);

#if 0
  /* Stop monitoring the application directories */
  for (iter = menu->priv->app_dirs; iter != NULL; iter = g_list_next (iter))
    xfce_menu_monitor_remove_directory (menu, (const gchar *)iter->data);

  /* Stop monitoring the menu directory file */
  if (XFCE_IS_MENU_DIRECTORY (menu->priv->directory))
    xfce_menu_monitor_remove_file (menu, xfce_menu_directory_get_filename (menu->priv->directory));

  /* Stop monitoring the menu file */
  xfce_menu_monitor_remove_file (menu, menu->priv->filename);
#endif
}

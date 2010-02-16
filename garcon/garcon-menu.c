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

#include <glib/gi18n.h>

#include <garcon/garcon-environment.h>
#include <garcon/garcon-menu-element.h>
#include <garcon/garcon-menu-item.h>
#include <garcon/garcon-menu-directory.h>
#include <garcon/garcon-menu-item-cache.h>
#include <garcon/garcon-menu-separator.h>
#include <garcon/garcon-menu-node.h>
#include <garcon/garcon-menu-parser.h>
#include <garcon/garcon-menu-merger.h>
#include <garcon/garcon-private.h>
#include <garcon/garcon.h>



/* Use g_access() on win32 */
#if defined(G_OS_WIN32)
#include <glib/gstdio.h>
#else
#define g_access(filename, mode) (access ((filename), (mode)))
#endif



/**
 * SECTION:garcon
 * @title: GarconMenu
 **/



/* Potential root menu files */
static const gchar GARCON_MENU_ROOT_SPECS[][30] =
{
  "menus/applications.menu",
  "menus/xfce-applications.menu",
  "menus/gnome-applications.menu",
  "menus/kde-applications.menu",
};



#define GARCON_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GARCON_TYPE_MENU, GarconMenuPrivate))



typedef struct _GarconMenuPair
{
  gpointer first;
  gpointer second;
} GarconMenuPair;



/* Property identifiers */
enum
{
  PROP_0,
  PROP_ENVIRONMENT,
  PROP_FILE,
  PROP_DIRECTORY,
  PROP_PARENT, /* TODO */
};



static void                 garcon_menu_element_init                    (GarconMenuElementIface  *iface);
static void                 garcon_menu_finalize                        (GObject                 *object);
static void                 garcon_menu_get_property                    (GObject                 *object,
                                                                         guint                    prop_id,
                                                                         GValue                  *value,
                                                                         GParamSpec              *pspec);
static void                 garcon_menu_set_property                    (GObject                 *object,
                                                                         guint                    prop_id,
                                                                         const GValue            *value,
                                                                         GParamSpec              *pspec);
static void                 garcon_menu_set_directory                   (GarconMenu              *menu,
                                                                         GarconMenuDirectory     *directory);
static void                 garcon_menu_resolve_menus                   (GarconMenu              *menu);
static void                 garcon_menu_resolve_directory               (GarconMenu              *menu);
static GarconMenuDirectory *garcon_menu_lookup_directory                (GarconMenu              *menu,
                                                                         const gchar             *filename);
static void                 garcon_menu_collect_files                   (GarconMenu              *menu,
                                                                         GHashTable              *desktop_id_table);
static void                 garcon_menu_collect_files_from_path         (GarconMenu              *menu,
                                                                         GHashTable              *desktop_id_table,
                                                                         GFile                   *path,
                                                                         const gchar             *id_prefix);
static void                 garcon_menu_resolve_items                   (GarconMenu              *menu,
                                                                         GHashTable              *desktop_id_table,
                                                                         gboolean                 only_unallocated);
static void                 garcon_menu_resolve_items_by_rule           (GarconMenu              *menu,
                                                                         GHashTable              *desktop_id_table,
                                                                         GNode                   *node);
static void                 garcon_menu_resolve_item_by_rule            (const gchar             *desktop_id,
                                                                         const gchar             *uri,
                                                                         GarconMenuPair          *data);
static void                 garcon_menu_remove_deleted_menus            (GarconMenu              *menu);
static gint                 garcon_menu_compare_items                   (gconstpointer           *a,
                                                                         gconstpointer           *b);
static const gchar         *garcon_menu_get_element_name                (GarconMenuElement       *element);
static const gchar         *garcon_menu_get_element_comment             (GarconMenuElement       *element);
static const gchar         *garcon_menu_get_element_icon_name           (GarconMenuElement       *element);
static gboolean             garcon_menu_get_element_visible             (GarconMenuElement       *element);
static gboolean             garcon_menu_get_element_show_in_environment (GarconMenuElement       *element);
static gboolean             garcon_menu_get_element_no_display          (GarconMenuElement       *element);



struct _GarconMenuPrivate
{
  /* Menu file */
  GFile               *file;

  /* DOM tree */
  GNode               *tree;

  /* Directory */
  GarconMenuDirectory *directory;

  /* Submenus */
  GList               *submenus;

  /* Parent menu */
  GarconMenu          *parent;

  /* Menu item pool */
  GarconMenuItemPool  *pool;

  /* Shared menu item cache */
  GarconMenuItemCache *cache;
};



G_DEFINE_TYPE_WITH_CODE (GarconMenu, garcon_menu, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (GARCON_TYPE_MENU_ELEMENT, garcon_menu_element_init))



static void
garcon_menu_class_init (GarconMenuClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (GarconMenuPrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = garcon_menu_finalize;
  gobject_class->get_property = garcon_menu_get_property;
  gobject_class->set_property = garcon_menu_set_property;

  /**
   * GarconMenu:file:
   *
   * The #GFile from which the %GarconMenu was loaded.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file",
                                                        "file",
                                                        "file",
                                                        G_TYPE_FILE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS |
                                                        G_PARAM_CONSTRUCT_ONLY));

  /**
   * GarconMenu:directory:
   *
   * The directory entry associated with this menu.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_DIRECTORY,
                                   g_param_spec_object ("directory",
                                                        "Directory",
                                                        "Directory entry associated with this menu",
                                                        GARCON_TYPE_MENU_DIRECTORY,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
}



static void
garcon_menu_element_init (GarconMenuElementIface *iface)
{
  iface->get_name = garcon_menu_get_element_name;
  iface->get_comment = garcon_menu_get_element_comment;
  iface->get_icon_name = garcon_menu_get_element_icon_name;
  iface->get_visible = garcon_menu_get_element_visible;
  iface->get_show_in_environment = garcon_menu_get_element_show_in_environment;
  iface->get_no_display = garcon_menu_get_element_no_display;
}



static void
garcon_menu_init (GarconMenu *menu)
{
  menu->priv = GARCON_MENU_GET_PRIVATE (menu);
  menu->priv->file = NULL;
  menu->priv->tree = NULL;
  menu->priv->directory = NULL;
  menu->priv->submenus = NULL;
  menu->priv->parent = NULL;
  menu->priv->pool = garcon_menu_item_pool_new ();

  /* Take reference on the menu item cache */
  menu->priv->cache = garcon_menu_item_cache_get_default ();
}



static void
garcon_menu_finalize (GObject *object)
{
  GarconMenu *menu = GARCON_MENU (object);

  /* Destroy the menu tree */
  if (menu->priv->parent == NULL)
    garcon_menu_node_tree_free (menu->priv->tree);

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

  /* Release item cache reference */
  g_object_unref (menu->priv->cache);

  (*G_OBJECT_CLASS (garcon_menu_parent_class)->finalize) (object);
}



static void
garcon_menu_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GarconMenu *menu = GARCON_MENU (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, menu->priv->file);
      break;

    case PROP_DIRECTORY:
      g_value_set_object (value, menu->priv->directory);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
garcon_menu_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  GarconMenu *menu = GARCON_MENU (object);

  switch (prop_id)
    {
    case PROP_FILE:
      menu->priv->file = g_value_dup_object (value);
      break;

    case PROP_DIRECTORY:
      garcon_menu_set_directory (menu, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



/**
 * garcon_menu_new:
 * @file  : #GFile for the .menu file you want to load.
 *
 * Creates a new #GarconMenu for the .menu file referred to by @file.
 * This operation only fails @file is invalid. To load the menu
 * tree from the file, you need to call garcon_menu_load() with the
 * returned #GarconMenu.
 *
 * The caller is responsible to destroy the returned #GarconMenu
 * using g_object_unref().
 *
 * For more information about the usage @see garcon_menu_new().
 *
 * Return value: a new #GarconMenu for @file.
 **/
GarconMenu *
garcon_menu_new (GFile *file)
{
  g_return_val_if_fail (G_IS_FILE (file), NULL);
  return g_object_new (GARCON_TYPE_MENU, "file", file, NULL);
}



/**
 * garcon_menu_new_for_path:
 * @filename : Path/URI of the .menu file you want to load.
 *
 * Creates a new #GarconMenu for the .menu file referred to by @filename.
 * This operation only fails if the filename is NULL. To load the menu
 * tree from the file, you need to call garcon_menu_load() with the
 * returned #GarconMenu.
 *
 * <informalexample><programlisting>
 * GarconMenu *menu = garcon_menu_new (filename);
 *
 * if (garcon_menu_load (menu, &error))
 *   ...
 * else
 *   ...
 *
 * g_object_unref (menu);
 * </programlisting></informalexample>
 *
 * The caller is responsible to destroy the returned #GarconMenu
 * using g_object_unref().
 *
 * Return value: a new #GarconMenu for @filename.
 **/
GarconMenu *
garcon_menu_new_for_path (const gchar *filename)
{
  GarconMenu *menu;
  GFile      *file;

  g_return_val_if_fail (filename != NULL, NULL);

  /* Create new menu */
  file = _garcon_file_new_for_unknown_input (filename, NULL);
  menu = g_object_new (GARCON_TYPE_MENU, "file", file, NULL);
  g_object_unref (file);

  return menu;
}



/**
 * garcon_menu_new_applications:
 *
 * Creates a new #GarconMenu for the applications.menu file
 * which is being used to display installed applications.
 *
 * For more information about the usage @see garcon_menu_new().
 *
 * Return value: a new #GarconMenu for applications.menu.
 **/
GarconMenu *
garcon_menu_new_applications (void)
{
  GarconMenu *menu = NULL;
  GFile      *file;
  gchar      *filename;
  guint       n;

  /* Search for a usable applications menu file */
  for (n = 0; menu == NULL && n < G_N_ELEMENTS (GARCON_MENU_ROOT_SPECS); ++n)
    {
      /* Search for the applications menu file */
      filename = garcon_config_lookup (GARCON_MENU_ROOT_SPECS[n]);

      /* Create menu if the file exists */
      if (G_UNLIKELY (filename != NULL))
        {
          file = _garcon_file_new_for_unknown_input (filename, NULL);
          menu = garcon_menu_new (file);
          g_object_unref (file);
        }

      g_free (filename);
    }

  return menu;
}



/**
 * garcon_menu_get_file:
 * @menu : a #GarconMenu.
 *
 * Get the file for @menu. It refers to the .menu file from which
 * @menu was or will be loaded.
 *
 * Return value: a #GFile. The returned object
 * should be unreffed with g_object_unref() when no longer needed.
 */
GFile *
garcon_menu_get_file (GarconMenu *menu)
{
  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  return g_object_ref (menu->priv->file);
}



static const gchar *
garcon_menu_get_name (GarconMenu *menu)
{
  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  return garcon_menu_node_tree_get_string_child (menu->priv->tree,
                                                 GARCON_MENU_NODE_TYPE_NAME);
}



/**
 * garcon_menu_get_directory:
 * @menu : a #GarconMenu.
 *
 * Returns the #GarconMenuDirectory of @menu or %NULL if the &lt;Menu&gt;
 * element that corresponds to @menu has no valid &lt;Directory&gt; element.
 * The menu directory may contain a lot of useful information about
 * the menu like the display and icon name, desktop environments it
 * should show up in etc.
 *
 * Return value: #GarconMenuDirectory of @menu or %NULL if
 *               @menu has no valid directory element. The returned object
 *               should be unreffed with g_object_unref() when no longer needed.
 */
GarconMenuDirectory*
garcon_menu_get_directory (GarconMenu *menu)
{
  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  return menu->priv->directory;
}



static void
garcon_menu_set_directory (GarconMenu          *menu,
                           GarconMenuDirectory *directory)
{
  g_return_if_fail (GARCON_IS_MENU (menu));
  g_return_if_fail (GARCON_IS_MENU_DIRECTORY (directory));

  /* Abort if directories are equal */
  if (garcon_menu_directory_equal (directory, menu->priv->directory))
    return;

  /* Destroy old directory */
  if (menu->priv->directory != NULL)
    g_object_unref (menu->priv->directory);

  /* Remove the floating reference and acquire a normal one */
  g_object_ref_sink (directory);

  /* Set the new directory */
  menu->priv->directory = directory;

  /* Notify listeners */
  g_object_notify (G_OBJECT (menu), "directory");
}



/**
 * garcon_menu_load:
 * @menu        : a #GarconMenu
 * @cancellable : a #GCancellable
 * @error       : #GError return location
 *
 * This function loads the entire menu tree from the file referred to
 * by @menu. It resolves merges, moves and everything else defined
 * in the menu specification. The resulting tree information is
 * stored within @menu and can be accessed using the public #GarconMenu
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
garcon_menu_load (GarconMenu   *menu,
                  GCancellable *cancellable,
                  GError      **error)
{
  GarconMenuParser *parser;
  GarconMenuMerger *merger;
  GHashTable       *desktop_id_table;
  gboolean          success = TRUE;

  g_return_val_if_fail (GARCON_IS_MENU (menu), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  parser = garcon_menu_parser_new (menu->priv->file);

  if (G_LIKELY (garcon_menu_parser_run (parser, cancellable, error)))
    {
      merger = garcon_menu_merger_new (GARCON_MENU_TREE_PROVIDER (parser));

      if (garcon_menu_merger_run (merger, cancellable, error))
        menu->priv->tree = garcon_menu_tree_provider_get_tree (GARCON_MENU_TREE_PROVIDER (merger));
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
  garcon_menu_resolve_menus (menu);

  /* Resolve the menu directory */
  garcon_menu_resolve_directory (menu);

  desktop_id_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  /* Load menu items */
  garcon_menu_collect_files (menu, desktop_id_table);
  garcon_menu_resolve_items (menu, desktop_id_table, FALSE);
  garcon_menu_resolve_items (menu, desktop_id_table, TRUE);

  /* Remove deleted menus */
  garcon_menu_remove_deleted_menus (menu);

  g_hash_table_unref (desktop_id_table);

  return TRUE;
}



GList *
garcon_menu_get_menus (GarconMenu *menu)
{
  GList *menus = NULL;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);

  /* Copy submenu list */
  menus = g_list_copy (menu->priv->submenus);

  /* Sort submenus */
  menus = g_list_sort (menus, (GCompareFunc) garcon_menu_compare_items);

  return menus;
}



void
garcon_menu_add_menu (GarconMenu *menu,
                      GarconMenu *submenu)
{
  g_return_if_fail (GARCON_IS_MENU (menu));
  g_return_if_fail (GARCON_IS_MENU (submenu));

  /* Remove floating reference and acquire a 'real' one */
  g_object_ref_sink (G_OBJECT (submenu));

  /* Append menu to the list */
  menu->priv->submenus = g_list_append (menu->priv->submenus, submenu);

  /* TODO: Use property method here */
  submenu->priv->parent = menu;
}



GarconMenu *
garcon_menu_get_menu_with_name (GarconMenu  *menu,
                                const gchar *name)
{
  GarconMenu *result = NULL;
  GList      *iter;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  /* Iterate over the submenu list */
  for (iter = menu->priv->submenus; result == NULL && iter != NULL; iter = g_list_next (iter))
    {
      /* End loop when a matching submenu is found */
      if (G_UNLIKELY (g_strcmp0 (garcon_menu_get_name (iter->data), name) == 0))
        result = iter->data;
    }

  return result;
}



GarconMenu *
garcon_menu_get_parent (GarconMenu *menu)
{
  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  return menu->priv->parent;
}



static void
garcon_menu_resolve_menus (GarconMenu *menu)
{
  GarconMenu *submenu;
  GList      *menus = NULL;
  GList      *iter;

  g_return_if_fail (GARCON_IS_MENU (menu));

  menus = garcon_menu_node_tree_get_child_nodes (menu->priv->tree,
                                                 GARCON_MENU_NODE_TYPE_MENU,
                                                 FALSE);

  for (iter = menus; iter != NULL; iter = g_list_next (iter))
    {
      submenu = g_object_new (GARCON_TYPE_MENU, "file", menu->priv->file, NULL);
      submenu->priv->tree = iter->data;
      garcon_menu_add_menu (menu, submenu);
      g_object_unref (submenu);
    }

  g_list_free (menus);

  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    garcon_menu_resolve_menus (iter->data);
}



static GList *
garcon_menu_get_directories (GarconMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = garcon_menu_node_tree_get_string_children (menu->priv->tree,
                                                    GARCON_MENU_NODE_TYPE_DIRECTORY,
                                                    TRUE);

  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, garcon_menu_get_directories (menu->priv->parent));

  return dirs;
}



static void
garcon_menu_resolve_directory (GarconMenu *menu)
{
  GarconMenuDirectory *directory = NULL;
  GList               *directories = NULL;
  GList               *iter;

  g_return_if_fail (GARCON_IS_MENU (menu));

  /* Determine all directories for this menu */
  directories = garcon_menu_get_directories (menu);

  /* Try to load one directory name after another */
  for (iter = directories; directory == NULL && iter != NULL; iter = g_list_next (iter))
    {
      /* Try to load the directory with this name */
      directory = garcon_menu_lookup_directory (menu, iter->data);
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
    garcon_menu_resolve_directory (iter->data);
}



static GList *
garcon_menu_get_directory_dirs (GarconMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = garcon_menu_node_tree_get_string_children (menu->priv->tree,
                                                    GARCON_MENU_NODE_TYPE_DIRECTORY_DIR,
                                                    TRUE);

  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, garcon_menu_get_directory_dirs (menu->priv->parent));

  return dirs;
}



static GarconMenuDirectory *
garcon_menu_lookup_directory (GarconMenu  *menu,
                              const gchar *filename)
{
  GarconMenuDirectory *directory = NULL;
  GList               *dirs = NULL;
  GList               *iter;
  GFile               *file;
  GFile               *dir;
  gboolean             found = FALSE;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  g_return_val_if_fail (filename != NULL, NULL);

  dirs = garcon_menu_get_directory_dirs (menu);

  /* Iterate through all directories */
  for (iter = dirs; !found && iter != NULL; iter = g_list_next (iter))
    {
      dir = _garcon_file_new_relative_to_file (iter->data, menu->priv->file);
      file = _garcon_file_new_relative_to_file (filename, dir);

      /* Check if the file exists and is readable */
      if (G_LIKELY (g_file_query_exists (file, NULL)))
        {
          /* Load menu directory */
          directory = garcon_menu_directory_new (file);

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
garcon_menu_get_app_dirs (GarconMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = garcon_menu_node_tree_get_string_children (menu->priv->tree,
                                                    GARCON_MENU_NODE_TYPE_APP_DIR,
                                                    TRUE);

#if 0
  /* A submenu always inherits the application directories of its parent,
   * that is the reason the call below was added.
   * It only turned out we were looking in that same directories for
   * .desktop files multiple times.
   *
   * This was caused by the combination of the parent call below and
   * traversing the children in garcon_menu_collect_files(). For each
   * submenu the appdirs of the root were added and traversed again.
   *
   * This is not needed because we always start at the root and traverse
   * in "pre-order", so all the desktop files are added in the hash-table.
   */
  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, garcon_menu_get_app_dirs (menu->priv->parent));
#endif

  return dirs;
}



static void
garcon_menu_collect_files (GarconMenu *menu,
                           GHashTable *desktop_id_table)
{
  GList *app_dirs = NULL;
  GList *iter;
  GFile *file;

  g_return_if_fail (GARCON_IS_MENU (menu));

  app_dirs = garcon_menu_get_app_dirs (menu);

  /* Collect desktop entry filenames */
  for (iter = app_dirs; iter != NULL; iter = g_list_next (iter))
    {
      file = g_file_new_for_uri (iter->data);
      garcon_menu_collect_files_from_path (menu, desktop_id_table, file, NULL);
      g_object_unref (file);
    }

  /* Free directory list */
  g_list_free (app_dirs);

  /* Collect filenames for submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    garcon_menu_collect_files (iter->data, desktop_id_table);
}



static void
garcon_menu_collect_files_from_path (GarconMenu  *menu,
                                     GHashTable  *desktop_id_table,
                                     GFile       *dir,
                                     const gchar *id_prefix)
{
  GFileEnumerator *enumerator;
  GFileInfo       *file_info;
  GFile           *file;
  gchar           *base_name;
  gchar           *new_id_prefix;
  gchar           *desktop_id;

  g_return_if_fail (GARCON_IS_MENU (menu));

  /* Skip directory if it doesn't exist */
  if (G_UNLIKELY (!g_file_query_exists (dir, NULL)))
    return;

  /* Skip directory if it's not a directory */
  if (G_UNLIKELY (g_file_query_file_type (dir, G_FILE_QUERY_INFO_NONE,
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
      base_name = g_file_get_basename (file);

      /* Treat files and directories differently */
      if (g_file_info_get_file_type (file_info) == G_FILE_TYPE_DIRECTORY)
        {
          /* Create new desktop-file id prefix */
          if (G_LIKELY (id_prefix == NULL))
            new_id_prefix = g_strdup (base_name);
          else
            new_id_prefix = g_strjoin ("-", id_prefix, base_name, NULL);

          /* Collect files in the directory */
          garcon_menu_collect_files_from_path (menu, desktop_id_table, file, new_id_prefix);

          /* Free id prefix */
          g_free (new_id_prefix);
        }
      else if (G_LIKELY (g_str_has_suffix (base_name, ".desktop")))
        {
          /* Create desktop-file id */
          if (G_LIKELY (id_prefix == NULL))
            desktop_id = g_strdup (base_name);
          else
            desktop_id = g_strjoin ("-", id_prefix, base_name, NULL);

          /* Insert into the files hash table if the desktop-file id does not exist there yet */
          if (G_LIKELY (g_hash_table_lookup (desktop_id_table, desktop_id) == NULL))
            g_hash_table_insert (desktop_id_table, desktop_id, g_file_get_uri (file));
          else
            g_free (desktop_id);
        }

      /* Free absolute path */
      g_free (base_name);

      /* Destroy file */
      g_object_unref (file);

      /* Destroy info */
      g_object_unref (file_info);
    }

  g_object_unref (enumerator);
}



static gboolean
collect_rules (GNode  *node,
               GList **list)
{
  GarconMenuNodeType type;

  type = garcon_menu_node_tree_get_node_type (node);

  if (type == GARCON_MENU_NODE_TYPE_INCLUDE ||
      type == GARCON_MENU_NODE_TYPE_EXCLUDE)
    {
      *list = g_list_append (*list, node);
    }

  return FALSE;
}



static void
garcon_menu_resolve_items (GarconMenu *menu,
                           GHashTable *desktop_id_table,
                           gboolean    only_unallocated)
{
  GList  *rules = NULL;
  GList  *iter;
  gboolean menu_only_unallocated = FALSE;

  g_return_if_fail (menu != NULL && GARCON_IS_MENU (menu));

  menu_only_unallocated = garcon_menu_node_tree_get_boolean_child (menu->priv->tree,
                                                                   GARCON_MENU_NODE_TYPE_ONLY_UNALLOCATED);

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
          if (G_LIKELY (garcon_menu_node_tree_get_node_type (iter->data) == GARCON_MENU_NODE_TYPE_INCLUDE))
            {
              /* Resolve available items and match them against this rule */
              garcon_menu_resolve_items_by_rule (menu, desktop_id_table, iter->data);
            }
          else
            {
              /* Remove all items matching this exclude rule from the item pool */
              garcon_menu_item_pool_apply_exclude_rule (menu->priv->pool, iter->data);
            }
        }
    }

  /* Iterate over all submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    {
      /* Resolve items of the submenu */
      garcon_menu_resolve_items (GARCON_MENU (iter->data), desktop_id_table, only_unallocated);
    }
}



static void
garcon_menu_resolve_items_by_rule (GarconMenu *menu,
                                   GHashTable *desktop_id_table,
                                   GNode      *node)
{
  GarconMenuPair pair;

  g_return_if_fail (GARCON_IS_MENU (menu));

  /* Store menu and rule pointer in the pair */
  pair.first = menu;
  pair.second = node;

  /* Try to insert each of the collected desktop entry filenames into the menu */
  g_hash_table_foreach (desktop_id_table, (GHFunc) garcon_menu_resolve_item_by_rule, &pair);
}



static void
garcon_menu_resolve_item_by_rule (const gchar    *desktop_id,
                                  const gchar    *uri,
                                  GarconMenuPair *data)
{
  GarconMenuItem *item = NULL;
  GarconMenu     *menu = NULL;
  GNode          *node = NULL;
  gboolean        only_unallocated = FALSE;

  g_return_if_fail (GARCON_IS_MENU (data->first));
  g_return_if_fail (data->second != NULL);

  /* Restore menu and rule from the data pair */
  menu = data->first;
  node = data->second;

  /* Try to load the menu item from the cache */
  item = garcon_menu_item_cache_lookup (menu->priv->cache, uri, desktop_id);

  if (G_LIKELY (item != NULL))
    {
      only_unallocated = garcon_menu_node_tree_get_boolean_child (menu->priv->tree,
                                                                  GARCON_MENU_NODE_TYPE_ONLY_UNALLOCATED);

      /* Only include item if menu not only includes unallocated items
       * or if the item is not allocated yet */
      if (!only_unallocated || garcon_menu_item_get_allocated (item) == 0)
        {
          /* Add item to the pool if it matches the include rule */
          if (G_LIKELY (garcon_menu_node_tree_rule_matches (node, item)))
            garcon_menu_item_pool_insert (menu->priv->pool, item);
        }
    }
}



static void
garcon_menu_remove_deleted_menus (GarconMenu *menu)
{
  GarconMenu *submenu;
  GList      *iter;
  gboolean    deleted;

  g_return_if_fail (GARCON_IS_MENU (menu));

  /* Note: There's a limitation: if the root menu has a <Deleted/> we
   * can't just free the pointer here. Therefor we only check child menus. */

  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    {
      submenu = iter->data;

      /* Check whether there is a <Deleted/> element */
      deleted = garcon_menu_node_tree_get_boolean_child (submenu->priv->tree,
                                                         GARCON_MENU_NODE_TYPE_DELETED);

      /* Determine whether this submenu was deleted */
      if (G_LIKELY (submenu->priv->directory != NULL))
        deleted = deleted || garcon_menu_directory_get_hidden (submenu->priv->directory);

      /* Remove submenu if it is deleted, otherwise check submenus of the submenu */
      if (G_UNLIKELY (deleted))
        {
          /* Remove submenu from the list ... */
          menu->priv->submenus = g_list_remove_link (menu->priv->submenus, iter);

          /* ... and destroy it */
          g_object_unref (submenu);
        }
      else
        garcon_menu_remove_deleted_menus (submenu);
    }
}



GarconMenuItemPool*
garcon_menu_get_item_pool (GarconMenu *menu)
{
  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);

  return menu->priv->pool;
}



static void
items_collect (const gchar    *desktop_id,
               GarconMenuItem *item,
               GList         **listp)
{
  *listp = g_list_prepend (*listp, item);
}



/**
 * garcon_menu_get_items:
 * @menu : a #GarconMenu.
 *
 * Returns all #GarconMenuItem<!---->s included in @menu. The items are
 * sorted by their display names in ascending order.
 *
 * The caller is responsible to free the returned list using
 * <informalexample><programlisting>
 * g_list_free (list);
 * </programlisting></informalexample>
 * when no longer needed.
 *
 * Return value: list of #GarconMenuItem<!---->s included in @menu.
 **/
GList *
garcon_menu_get_items (GarconMenu *menu)
{
  GList *items = NULL;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);

  /* Collect the items in the pool */
  garcon_menu_item_pool_foreach (menu->priv->pool, (GHFunc) items_collect, &items);

  /* Sort items */
  items = g_list_sort (items, (GCompareFunc) garcon_menu_compare_items);

  return items;
}



static GNode *
garcon_menu_get_layout (GarconMenu *menu,
                        gboolean    default_only)
{
  GNode *layout = NULL;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);

  if (G_LIKELY (!default_only))
    {
      layout = garcon_menu_node_tree_get_child_node (menu->priv->tree,
                                                     GARCON_MENU_NODE_TYPE_LAYOUT,
                                                     TRUE);
    }

  if (layout == NULL)
    {
      layout = garcon_menu_node_tree_get_child_node (menu->priv->tree,
                                                     GARCON_MENU_NODE_TYPE_DEFAULT_LAYOUT,
                                                     TRUE);

      if (layout == NULL && menu->priv->parent != NULL)
        layout = garcon_menu_get_layout (menu->priv->parent, TRUE);
    }

  return layout;
}



static gboolean
layout_has_menuname (GNode       *layout,
                     const gchar *name)
{
  GList   *nodes;
  GList   *iter;
  gboolean has_menuname = FALSE;

  nodes = garcon_menu_node_tree_get_child_nodes (layout, GARCON_MENU_NODE_TYPE_MENUNAME,
                                                 FALSE);

  for (iter = g_list_first (nodes); !has_menuname && iter != NULL; iter = g_list_next (iter))
    if (g_str_equal (garcon_menu_node_tree_get_string (iter->data), name))
      has_menuname = TRUE;

  g_list_free (nodes);

  return has_menuname;
}



static gboolean
layout_has_filename (GNode       *layout,
                     const gchar *desktop_id)
{
  GList   *nodes;
  GList   *iter;
  gboolean has_filename = FALSE;

  nodes = garcon_menu_node_tree_get_child_nodes (layout, GARCON_MENU_NODE_TYPE_FILENAME,
                                                 FALSE);

  for (iter = g_list_first (nodes); !has_filename && iter != NULL; iter = g_list_next (iter))
    if (g_str_equal (garcon_menu_node_tree_get_string (iter->data), desktop_id))
      has_filename = TRUE;

  g_list_free (nodes);

  return has_filename;
}



static void
layout_elements_collect (GList **dest_list,
                         GList  *src_list,
                         GNode  *layout)
{
  GList *iter;

  for (iter = src_list; iter != NULL; iter = g_list_next (iter))
    {
      if (GARCON_IS_MENU (iter->data))
        {
          if (G_LIKELY (!layout_has_menuname (layout, garcon_menu_get_name (iter->data))))
            *dest_list = g_list_append (*dest_list, iter->data);
        }
      else if (GARCON_IS_MENU_ITEM (iter->data))
        {
          if (G_LIKELY (!layout_has_filename (layout, garcon_menu_item_get_desktop_id (iter->data))))
            *dest_list = g_list_append (*dest_list, iter->data);
        }
    }
}



GList *
garcon_menu_get_elements (GarconMenu *menu)
{
  GarconMenuLayoutMergeType merge_type;
  GarconMenuNodeType        type;
  GarconMenuItem           *item;
  GarconMenu               *submenu;
  GList                    *items = NULL;
  GList                    *menu_items;
  GNode                    *layout = NULL;
  GNode                    *node;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);

  /* Determine layout node */
  layout = garcon_menu_get_layout (menu, FALSE);

  /* There should always be a layout, otherwise GarconMenuMerger is broken */
  g_return_val_if_fail (layout != NULL, NULL);

  /* Process layout nodes in order */
  for (node = g_node_first_child (layout); node != NULL; node = g_node_next_sibling (node))
    {
      /* Determine layout node type */
      type = garcon_menu_node_tree_get_node_type (node);

      if (type == GARCON_MENU_NODE_TYPE_FILENAME)
        {
          /* Search for desktop ID in the item pool */
          item = garcon_menu_item_pool_lookup (menu->priv->pool,
                                               garcon_menu_node_tree_get_string (node));

          /* If the item with this desktop ID is included in the menu, append it to the list */
          if (G_LIKELY (item != NULL))
            items = g_list_append (items, item);
        }
      else if (type == GARCON_MENU_NODE_TYPE_MENUNAME)
        {
          /* Search submenu with this name */
          submenu = garcon_menu_get_menu_with_name (menu,
                                                    garcon_menu_node_tree_get_string (node));

          /* If there is such a menu, append it to the list */
          if (G_LIKELY (submenu != NULL))
            items = g_list_append (items, submenu);
        }
      else if (type == GARCON_MENU_NODE_TYPE_SEPARATOR)
        {
          /* Append separator to the list */
          items = g_list_append (items, garcon_menu_separator_get_default ());
        }
      else if (type == GARCON_MENU_NODE_TYPE_MERGE)
        {
          /* Determine merge type */
          merge_type = garcon_menu_node_tree_get_layout_merge_type (node);

          if (merge_type == GARCON_MENU_LAYOUT_MERGE_ALL)
            {
              /* Get all menu items of this menu */
              menu_items = garcon_menu_get_items (menu);

              /* Append submenus */
              menu_items = g_list_concat (menu_items, garcon_menu_get_menus (menu));

              /* Sort menu items */
              menu_items = g_list_sort (menu_items, (GCompareFunc) garcon_menu_compare_items);

              /* Append menu items to the returned item list */
              layout_elements_collect (&items, menu_items, layout);
            }
          else if (merge_type == GARCON_MENU_LAYOUT_MERGE_FILES)
            {
              /* Get all menu items of this menu */
              menu_items = garcon_menu_get_items (menu);

              /* Append menu items to the returned item list */
              layout_elements_collect (&items, menu_items, layout);
            }
          else if (merge_type == GARCON_MENU_LAYOUT_MERGE_MENUS)
            {
              /* Get all submenus */
              menu_items = garcon_menu_get_menus (menu);

              /* Append submenus to the returned item list */
              layout_elements_collect (&items, menu_items, layout);
            }
        }
    }

  return items;
}



static gint
garcon_menu_compare_items (gconstpointer *a,
                           gconstpointer *b)
{
  return g_strcmp0 (garcon_menu_element_get_name (GARCON_MENU_ELEMENT (a)),
                    garcon_menu_element_get_name (GARCON_MENU_ELEMENT (b)));
}



static const gchar*
garcon_menu_get_element_name (GarconMenuElement *element)
{
  GarconMenu    *menu;
  const gchar   *name = NULL;

  g_return_val_if_fail (GARCON_IS_MENU (element), NULL);

  menu = GARCON_MENU (element);

  /* Try directory name first */
  if (menu->priv->directory != NULL)
    name = garcon_menu_directory_get_name (menu->priv->directory);

  /* Otherwise use the menu name as a fallback */
  if (name == NULL)
    name = garcon_menu_get_name (menu);

  return name;
}



static const gchar*
garcon_menu_get_element_comment (GarconMenuElement *element)
{
  GarconMenu *menu;

  g_return_val_if_fail (GARCON_IS_MENU (element), NULL);

  menu = GARCON_MENU (element);

  if (menu->priv->directory == NULL)
    return NULL;
  else
    return garcon_menu_directory_get_comment (menu->priv->directory);
}



static const gchar*
garcon_menu_get_element_icon_name (GarconMenuElement *element)
{
  GarconMenu *menu;

  g_return_val_if_fail (GARCON_IS_MENU (element), NULL);

  menu = GARCON_MENU (element);

  if (menu->priv->directory == NULL)
    return NULL;
  else
    return garcon_menu_directory_get_icon_name (menu->priv->directory);
}



static gboolean
garcon_menu_get_element_visible (GarconMenuElement *element)
{
  GarconMenu *menu;
  GList      *items;
  GList      *iter;
  gboolean    visible = FALSE;

  g_return_val_if_fail (GARCON_IS_MENU (element), FALSE);

  menu = GARCON_MENU (element);

  if (menu->priv->directory != NULL)
    {
      if (!garcon_menu_directory_get_visible (menu->priv->directory))
        return FALSE;
    }

  /* if a menu has no visible children it shouldn't be visible */
  items = garcon_menu_get_elements (menu);
  for (iter = items; visible != TRUE && iter != NULL; iter = g_list_next (iter))
    {
      if (garcon_menu_element_get_visible (GARCON_MENU_ELEMENT (iter->data)))
        visible = TRUE;
    }

  g_list_free (items);
  return visible;
}



static gboolean
garcon_menu_get_element_show_in_environment (GarconMenuElement *element)
{
  GarconMenu *menu;

  g_return_val_if_fail (GARCON_IS_MENU (element), FALSE);

  menu = GARCON_MENU (element);

  if (menu->priv->directory == NULL)
    return FALSE;
  else
    return garcon_menu_directory_get_show_in_environment (menu->priv->directory);
}



static gboolean
garcon_menu_get_element_no_display (GarconMenuElement *element)
{
  GarconMenu *menu;

  g_return_val_if_fail (GARCON_IS_MENU (element), FALSE);

  menu = GARCON_MENU (element);

  if (menu->priv->directory == NULL)
    return FALSE;
  else
    return garcon_menu_directory_get_no_display (menu->priv->directory);
}

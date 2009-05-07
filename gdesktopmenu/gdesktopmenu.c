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

#include <gdesktopmenu/gdesktopmenuenvironment.h>
#include <gdesktopmenu/gdesktopmenuelement.h>
#include <gdesktopmenu/gdesktopmenuitem.h>
#include <gdesktopmenu/gdesktopmenudirectory.h>
#include <gdesktopmenu/gdesktopmenuitem-pool.h>
#include <gdesktopmenu/gdesktopmenuitem-cache.h>
#include <gdesktopmenu/gdesktopmenuseparator.h>
#include <gdesktopmenu/gdesktopmenumonitor.h>
#include <gdesktopmenu/gdesktopmenunode.h>
#include <gdesktopmenu/gdesktopmenuparser.h>
#include <gdesktopmenu/gdesktopmenumerger.h>
#include <gdesktopmenu/gdesktopmenugio.h>
#include <gdesktopmenu/gdesktopmenuh>



/* Use g_access() on win32 */
#if defined(G_OS_WIN32)
#include <glib/gstdio.h>
#else
#define g_access(filename, mode) (access ((filename), (mode)))
#endif



/**
 * SECTION:gdesktopmenu-library
 * @title: Library Initialization and Shutdown
 *
 * Library Initialization and Shutdown.
 **/



static gint g_desktop_menu_ref_count = 0;



/**
 * gdesktopmenu_init:
 * @env : name of the desktop environment (e.g. XFCE, GNOME or KDE) 
 *        or %NULL.
 *
 * Initializes the gdesktopmenu library. @env optionally defines the 
 * name of the desktop environment for which menus will be generated. 
 * This means that items belonging only to other desktop environments 
 * will be ignored.
 **/
void
gdesktopmenu_init (const gchar *env)
{
  if (g_atomic_int_exchange_and_add (&g_desktop_menu_ref_count, 1) == 0)
    {
      /* Initialize the GThread system */
      if (!g_thread_supported ())
        g_thread_init (NULL);

      /* Initialize the GObject type system */
      g_type_init ();

      /* Set desktop environment */
      g_desktop_menu_set_environment (env);

      /* Initialize the menu item cache */
      _g_desktop_menu_item_cache_init ();

      /* Initialize the directory module */
      _g_desktop_menu_directory_init ();

      /* Initialize monitoring system */
      _g_desktop_menu_monitor_init ();

      /* Creates the menu separator */
      _g_desktop_menu_separator_init ();
    }
}



/**
 * gdesktopmenu_shutdown:
 *
 * Shuts the gdesktopmenu library down.
 **/
void
gdesktopmenu_shutdown (void)
{
  if (g_atomic_int_dec_and_test (&g_desktop_menu_ref_count))
    {
      /* Unset desktop environment */
      g_desktop_menu_set_environment (NULL);

      /* Destroys the menu separator */
      _g_desktop_menu_separator_shutdown ();

      /* Shutdown monitoring system */
      _g_desktop_menu_monitor_shutdown ();

      /* Shutdown the directory module */
      _g_desktop_menu_directory_shutdown ();

      /* Shutdown the menu item cache */
      _g_desktop_menu_item_cache_shutdown ();
    }
}



gchar *
g_desktop_menu_config_lookup (const gchar *filename)
{
  const gchar * const *dirs;
  gchar               *path;
  gint                 i;

  g_return_val_if_fail (filename != NULL && g_utf8_strlen (filename, -1) > 0, NULL);

  path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_config_dir (), filename, NULL);

  if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
      g_free (path);
      path = NULL;

      dirs = g_get_system_config_dirs ();

      for (i = 0; path == NULL && dirs[i] != NULL; ++i)
        {
          if (g_path_is_absolute (dirs[i]))
            {
              path = g_build_path (G_DIR_SEPARATOR_S, dirs[i], filename, NULL);

              if (!g_file_test (path, G_FILE_TEST_IS_REGULAR))
                {
                  g_free (path);
                  path = NULL;
                }
            }
        }
    }
  
  return path;
}



/**
 * SECTION:gdesktopmenu
 * @title: GDesktopMenu
 * @short_description: Menu loading and library initialization/shutdown
 **/



/* Potential root menu files */
static const gchar G_DESKTOP_MENU_ROOT_SPECS[][30] = 
{
  "menus/applications.menu",
  "menus/xfce-applications.menu",
  "menus/gnome-applications.menu",
  "menus/kde-applications.menu",
};



#define G_DESKTOP_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), G_TYPE_DESKTOP_MENU, GDesktopMenuPrivate))



typedef struct _GDesktopMenuPair
{
  gpointer first;
  gpointer second;
} GDesktopMenuPair;



/* Property identifiers */
enum
{
  PROP_0,
  PROP_ENVIRONMENT,
  PROP_FILE,
  PROP_DIRECTORY,
  PROP_PARENT, /* TODO */
};



static void                   g_desktop_menu_class_init                      (GDesktopMenuClass         *klass);
static void                   g_desktop_menu_element_init                    (GDesktopMenuElementIface  *iface);
static void                   g_desktop_menu_instance_init                   (GDesktopMenu              *menu);
static void                   g_desktop_menu_finalize                        (GObject                   *object);
static void                   g_desktop_menu_get_property                    (GObject                   *object,
                                                                              guint                      prop_id,
                                                                              GValue                    *value,
                                                                              GParamSpec                *pspec);
static void                   g_desktop_menu_set_property                    (GObject                   *object,
                                                                              guint                      prop_id,
                                                                              const GValue              *value,
                                                                              GParamSpec                *pspec);
static void                   g_desktop_menu_set_directory                   (GDesktopMenu              *menu,
                                                                              GDesktopMenuDirectory     *directory);
static void                   g_desktop_menu_resolve_menus                   (GDesktopMenu              *menu);
static void                   g_desktop_menu_resolve_directory               (GDesktopMenu              *menu);
static GDesktopMenuDirectory *g_desktop_menu_lookup_directory                (GDesktopMenu              *menu,
                                                                              const gchar               *filename);
static void                   g_desktop_menu_collect_files                   (GDesktopMenu              *menu,
                                                                              GHashTable                *desktop_id_table);
static void                   g_desktop_menu_collect_files_from_path         (GDesktopMenu              *menu,
                                                                              GHashTable                *desktop_id_table,
                                                                              GFile                     *path,
                                                                              const gchar               *id_prefix);
static void                   g_desktop_menu_resolve_items                   (GDesktopMenu              *menu,
                                                                              GHashTable                *desktop_id_table,
                                                                              gboolean                   only_unallocated);
static void                   g_desktop_menu_resolve_items_by_rule           (GDesktopMenu              *menu,
                                                                              GHashTable                *desktop_id_table,
                                                                              GNode                     *node);
static void                   g_desktop_menu_resolve_item_by_rule            (const gchar               *desktop_id,
                                                                              const gchar               *uri,
                                                                              GDesktopMenuPair          *data);
static void                   g_desktop_menu_remove_deleted_menus            (GDesktopMenu              *menu);
static gint                   g_desktop_menu_compare_items                   (gconstpointer             *a,
                                                                              gconstpointer             *b);
static const gchar           *g_desktop_menu_get_element_name                (GDesktopMenuElement       *element);
static const gchar           *g_desktop_menu_get_element_comment             (GDesktopMenuElement       *element);
static const gchar           *g_desktop_menu_get_element_icon_name           (GDesktopMenuElement       *element);
static gboolean               g_desktop_menu_get_element_visible             (GDesktopMenuElement       *element);
static gboolean               g_desktop_menu_get_element_show_in_environment (GDesktopMenuElement       *element);
static gboolean               g_desktop_menu_get_element_no_display          (GDesktopMenuElement       *element);
static void                   g_desktop_menu_monitor_start                   (GDesktopMenu              *menu);
static void                   g_desktop_menu_monitor_stop                    (GDesktopMenu              *menu);



struct _GDesktopMenuPrivate
{
  /* Menu file */
  GFile                 *file;

  /* DOM tree */
  GNode                 *tree;

  /* Directory */
  GDesktopMenuDirectory *directory;

  /* Submenus */
  GList                 *submenus;

  /* Parent menu */
  GDesktopMenu          *parent;

  /* Menu item pool */
  GDesktopMenuItemPool  *pool;

  /* Shared menu item cache */
  GDesktopMenuItemCache *cache;
};

struct _GDesktopMenuClass
{
  GObjectClass __parent__;
};

struct _GDesktopMenu
{
  GObject              __parent__;

  /* < private > */
  GDesktopMenuPrivate *priv;
};



static GObjectClass *g_desktop_menu_parent_class = NULL;



GType
g_desktop_menu_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GInterfaceInfo element_info =
      {
        (GInterfaceInitFunc) g_desktop_menu_element_init,
        NULL,
        NULL,
      };

      type = g_type_register_static_simple (G_TYPE_OBJECT,
                                            "GDesktopMenuClass",
                                            sizeof (GDesktopMenuClass),
                                            (GClassInitFunc) g_desktop_menu_class_init,
                                            sizeof (GDesktopMenu),
                                            (GInstanceInitFunc) g_desktop_menu_instance_init,
                                            0);

      g_type_add_interface_static (type, G_TYPE_DESKTOP_MENU_ELEMENT, &element_info);
    }

  return type;
}



static void
g_desktop_menu_class_init (GDesktopMenuClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (GDesktopMenuPrivate));

  /* Determine the parent type class */
  g_desktop_menu_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = g_desktop_menu_finalize; 
  gobject_class->get_property = g_desktop_menu_get_property;
  gobject_class->set_property = g_desktop_menu_set_property;

  /**
   * GDesktopMenu:file:
   *
   * The #GFile from which the %GDesktopMenu was loaded. 
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file",
                                                        "file",
                                                        "file",
                                                        G_TYPE_FILE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  /**
   * GDesktopMenu:directory:
   *
   * The directory entry associated with this menu. 
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_DIRECTORY,
                                   g_param_spec_object ("directory",
                                                        "Directory",
                                                        "Directory entry associated with this menu",
                                                        G_TYPE_DESKTOP_MENU_DIRECTORY,
                                                        G_PARAM_READWRITE));
}



static void
g_desktop_menu_element_init (GDesktopMenuElementIface *iface)
{
  iface->get_name = g_desktop_menu_get_element_name;
  iface->get_comment = g_desktop_menu_get_element_comment;
  iface->get_icon_name = g_desktop_menu_get_element_icon_name;
  iface->get_visible = g_desktop_menu_get_element_visible;
  iface->get_show_in_environment = g_desktop_menu_get_element_show_in_environment;
  iface->get_no_display = g_desktop_menu_get_element_no_display;
}



static void
g_desktop_menu_instance_init (GDesktopMenu *menu)
{
  menu->priv = G_DESKTOP_MENU_GET_PRIVATE (menu);
  menu->priv->file = NULL;
  menu->priv->tree = NULL;
  menu->priv->directory = NULL;
  menu->priv->submenus = NULL;
  menu->priv->parent = NULL;
  menu->priv->pool = g_desktop_menu_item_pool_new ();

  /* Take reference on the menu item cache */
  menu->priv->cache = g_desktop_menu_item_cache_get_default ();
}



static void
g_desktop_menu_finalize (GObject *object)
{
  GDesktopMenu *menu = G_DESKTOP_MENU (object);

  /* Stop monitoring */
  g_desktop_menu_monitor_stop (menu);

  /* Destroy the menu tree */
  if (menu->priv->parent == NULL)
    g_desktop_menu_node_tree_free (menu->priv->tree);

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

  (*G_OBJECT_CLASS (g_desktop_menu_parent_class)->finalize) (object);
}



static void
g_desktop_menu_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  GDesktopMenu *menu = G_DESKTOP_MENU (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, g_desktop_menu_get_file (menu));
      break;

    case PROP_DIRECTORY:
      g_value_set_object (value, g_desktop_menu_get_directory (menu));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
g_desktop_menu_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GDesktopMenu *menu = G_DESKTOP_MENU (object);

  switch (prop_id)
    {
    case PROP_FILE:
      menu->priv->file = g_object_ref (g_value_get_object (value));
      break;

    case PROP_DIRECTORY:
      g_desktop_menu_set_directory (menu, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/**
 * g_desktop_menu_new:
 * @filename : Path/URI of the .menu file you want to load.
 *
 * Creates a new #GDesktopMenu for the .menu file referred to by @filename.
 * This operation only fails if the filename is NULL. To load the menu 
 * tree from the file, you need to call g_desktop_menu_load() with the
 * returned #GDesktopMenu. 
 *
 * <informalexample><programlisting>
 * GDesktopMenu *menu = g_desktop_menu_new (filename);
 * 
 * if (g_desktop_menu_load (menu, &error))
 *   ...
 * else
 *   ...
 *
 * g_object_unref (menu);
 * </programlisting></informalexample>
 *
 * The caller is responsible to destroy the returned #GDesktopMenu
 * using g_object_unref().
 *
 * Return value: a new #GDesktopMenu for @filename.
 **/
GDesktopMenu *
g_desktop_menu_new (const gchar *filename)
{
  GDesktopMenu *menu;
  GFile    *file;

  g_return_val_if_fail (filename != NULL, NULL);

  /* Create new menu */
  file = g_file_new_for_unknown_input (filename, NULL);
  menu = g_object_new (G_TYPE_DESKTOP_MENU, "file", file, NULL);
  g_object_unref (file);

  return menu;
}



/**
 * g_desktop_menu_new_for_file:
 * @file  : #GFile for the .menu file you want to load.
 *
 * Creates a new #GDesktopMenu for the .menu file referred to by @file.
 * This operation only fails @file is invalid. To load the menu 
 * tree from the file, you need to call g_desktop_menu_load() with the
 * returned #GDesktopMenu. 
 *
 * The caller is responsible to destroy the returned #GDesktopMenu
 * using g_object_unref().
 *
 * For more information about the usage @see g_desktop_menu_new().
 *
 * Return value: a new #GDesktopMenu for @file.
 **/
GDesktopMenu *
g_desktop_menu_new_for_file (GFile *file)
{
  g_return_val_if_fail (G_IS_FILE (file), NULL);
  return g_object_new (G_TYPE_DESKTOP_MENU, "file", file, NULL);
}



/**
 * g_desktop_menu_new_applications:
 *
 * Creates a new #GDesktopMenu for the applications.menu file
 * which is being used to display installed applications.
 *
 * For more information about the usage @see g_desktop_menu_new().
 *
 * Return value: a new #GDesktopMenu for applications.menu.
 **/
GDesktopMenu *
g_desktop_menu_new_applications (void)
{
  GDesktopMenu *menu = NULL;
  GFile    *file;
  gchar    *filename;
  guint     n;

  /* Search for a usable applications menu file */
  for (n = 0; menu == NULL && n < G_N_ELEMENTS (G_DESKTOP_MENU_ROOT_SPECS); ++n)
    {
      /* Search for the applications menu file */
      filename = g_desktop_menu_config_lookup (G_DESKTOP_MENU_ROOT_SPECS[n]);

      /* Create menu if the file exists */
      if (G_UNLIKELY (filename != NULL))
        {
          file = g_file_new_for_unknown_input (filename, NULL);
          menu = g_desktop_menu_new_for_file (file);
          g_object_unref (file);
        }

      g_free (filename);
    }
  
  return menu;
}



/**
 * g_desktop_menu_get_file:
 * @menu : a #GDesktopMenu.
 *
 * Returns the #GFile of @menu. It refers to the .menu file from which 
 * @menu was or will be loaded.
 * 
 * Return value: the @GFile of @menu.
 */
GFile *
g_desktop_menu_get_file (GDesktopMenu *menu)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  return g_object_ref (menu->priv->file);
}



static const gchar *
g_desktop_menu_get_name (GDesktopMenu *menu)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  return g_desktop_menu_node_tree_get_string_child (menu->priv->tree,
                                                    G_DESKTOP_MENU_NODE_TYPE_NAME);
}



/**
 * g_desktop_menu_get_directory:
 * @menu : a #GDesktopMenu.
 *
 * Returns the #GDesktopMenuDirectory of @menu or %NULL if the &lt;Menu&gt;
 * element that corresponds to @menu has no valid &lt;Directory&gt; element.
 * The menu directory may contain a lot of useful information about 
 * the menu like the display and icon name, desktop environments it 
 * should show up in etc.
 *
 * Return value: #GDesktopMenuDirectory of @menu or %NULL if
 *               @menu has no valid directory element.
 */
GDesktopMenuDirectory*
g_desktop_menu_get_directory (GDesktopMenu *menu)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  return menu->priv->directory;
}



static void
g_desktop_menu_set_directory (GDesktopMenu          *menu,
                              GDesktopMenuDirectory *directory)
{
  g_return_if_fail (G_IS_DESKTOP_MENU (menu));
  g_return_if_fail (G_IS_DESKTOP_MENU_DIRECTORY (directory));

  /* Abort if directories are equal */
  if (g_desktop_menu_directory_equal (directory, menu->priv->directory))
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
 * g_desktop_menu_load:
 * @menu        : a #GDesktopMenu
 * @cancellable : a #GCancellable
 * @error       : #GError return location
 *
 * This function loads the entire menu tree from the file referred to 
 * by @menu. It resolves merges, moves and everything else defined
 * in the menu specification. The resulting tree information is
 * stored within @menu and can be accessed using the public #GDesktopMenu 
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
g_desktop_menu_load (GDesktopMenu *menu, 
                     GCancellable *cancellable,
                     GError      **error)
{
  GDesktopMenuParser *parser;
  GDesktopMenuMerger *merger;
  GHashTable         *desktop_id_table;
  gboolean            success = TRUE;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), FALSE);

  parser = g_desktop_menu_parser_new (menu->priv->file);

  if (G_LIKELY (g_desktop_menu_parser_run (parser, cancellable, error)))
    {
      merger = g_desktop_menu_merger_new (G_DESKTOP_MENU_TREE_PROVIDER (parser));

      if (g_desktop_menu_merger_run (merger, cancellable, error))
        menu->priv->tree = g_desktop_menu_tree_provider_get_tree (G_DESKTOP_MENU_TREE_PROVIDER (merger));
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
  g_desktop_menu_resolve_menus (menu);

  /* Resolve the menu directory */
  g_desktop_menu_resolve_directory (menu);

  desktop_id_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  /* Load menu items */
  g_desktop_menu_collect_files (menu, desktop_id_table);
  g_desktop_menu_resolve_items (menu, desktop_id_table, FALSE);
  g_desktop_menu_resolve_items (menu, desktop_id_table, TRUE);

  /* Remove deleted menus */
  g_desktop_menu_remove_deleted_menus (menu);

  g_hash_table_unref (desktop_id_table);

  /* Start monitoring */
  g_desktop_menu_monitor_start (menu);

  return TRUE;
}



GList *
g_desktop_menu_get_menus (GDesktopMenu *menu)
{
  GList *menus = NULL;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  
  /* Copy submenu list */
  menus = g_list_copy (menu->priv->submenus);

  /* Sort submenus */
  menus = g_list_sort (menus, (GCompareFunc) g_desktop_menu_compare_items);

  return menus;
}



void
g_desktop_menu_add_menu (GDesktopMenu *menu,
                         GDesktopMenu *submenu)
{
  g_return_if_fail (G_IS_DESKTOP_MENU (menu));
  g_return_if_fail (G_IS_DESKTOP_MENU (submenu));

  /* Remove floating reference and acquire a 'real' one */
  g_object_ref_sink (G_OBJECT (submenu));

  /* Append menu to the list */
  menu->priv->submenus = g_list_append (menu->priv->submenus, submenu);

  /* TODO: Use property method here */
  submenu->priv->parent = menu;
}



GDesktopMenu *
g_desktop_menu_get_menu_with_name (GDesktopMenu    *menu,
                                   const gchar *name)
{
  GDesktopMenu *result = NULL;
  GList   *iter;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  /* Iterate over the submenu list */
  for (iter = menu->priv->submenus; result == NULL && iter != NULL; iter = g_list_next (iter))
    {
      /* End loop when a matching submenu is found */
      if (G_UNLIKELY (g_utf8_collate (g_desktop_menu_get_name (iter->data), name) == 0))
        result = iter->data;
    }

  return result;
}



GDesktopMenu *
g_desktop_menu_get_parent (GDesktopMenu *menu)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  return menu->priv->parent;
}



static void
g_desktop_menu_resolve_menus (GDesktopMenu *menu)
{
  GDesktopMenu *submenu;
  GList    *menus = NULL;
  GList    *iter;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));

  menus = g_desktop_menu_node_tree_get_child_nodes (menu->priv->tree, 
                                                    G_DESKTOP_MENU_NODE_TYPE_MENU, 
                                                    FALSE);

  for (iter = menus; iter != NULL; iter = g_list_next (iter))
    {
      submenu = g_object_new (G_TYPE_DESKTOP_MENU, "file", menu->priv->file, NULL);
      submenu->priv->tree = iter->data;
      g_desktop_menu_add_menu (menu, submenu);
      g_object_unref (submenu);
    }

  g_list_free (menus);

  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    g_desktop_menu_resolve_menus (iter->data);
}



static GList *
g_desktop_menu_get_directories (GDesktopMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = g_desktop_menu_node_tree_get_string_children (menu->priv->tree, 
                                                       G_DESKTOP_MENU_NODE_TYPE_DIRECTORY, 
                                                       TRUE);

  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, g_desktop_menu_get_directories (menu->priv->parent));

  return dirs;
}



static void
g_desktop_menu_resolve_directory (GDesktopMenu *menu)
{
  GDesktopMenuDirectory *directory = NULL;
  GList             *directories = NULL;
  GList             *iter;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));

  /* Determine all directories for this menu */
  directories = g_desktop_menu_get_directories (menu);

  /* Try to load one directory name after another */
  for (iter = directories; directory == NULL && iter != NULL; iter = g_list_next (iter))
    {
      /* Try to load the directory with this name */
      directory = g_desktop_menu_lookup_directory (menu, iter->data);
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
    g_desktop_menu_resolve_directory (iter->data);
}



static GList *
g_desktop_menu_get_directory_dirs (GDesktopMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = g_desktop_menu_node_tree_get_string_children (menu->priv->tree, 
                                                       G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR, 
                                                       TRUE);

  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, g_desktop_menu_get_directory_dirs (menu->priv->parent));

  return dirs;
}



static GDesktopMenuDirectory *
g_desktop_menu_lookup_directory (GDesktopMenu    *menu,
                                 const gchar *filename)
{
  GDesktopMenuDirectory *directory = NULL;
  GList                 *dirs = NULL;
  GList                 *iter;
  GFile                 *file;
  GFile                 *dir;
  gboolean               found = FALSE;
  
  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  g_return_val_if_fail (filename != NULL, NULL);

  dirs = g_desktop_menu_get_directory_dirs (menu);

  /* Iterate through all directories */
  for (iter = dirs; !found && iter != NULL; iter = g_list_next (iter))
    {
      dir = g_file_new_relative_to_file (iter->data, menu->priv->file);
      file = g_file_new_relative_to_file (filename, dir);

      /* Check if the file exists and is readable */
      if (G_LIKELY (g_file_query_exists (file, NULL)))
        {
          /* Load menu directory */
          directory = g_object_new (G_TYPE_DESKTOP_MENU_DIRECTORY, "file", file, NULL);

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
g_desktop_menu_get_app_dirs (GDesktopMenu *menu)
{
  GList *dirs = NULL;

  /* Fetch all application directories */
  dirs = g_desktop_menu_node_tree_get_string_children (menu->priv->tree, 
                                                       G_DESKTOP_MENU_NODE_TYPE_APP_DIR,
                                                       TRUE);

  if (menu->priv->parent != NULL)
    dirs = g_list_concat (dirs, g_desktop_menu_get_app_dirs (menu->priv->parent));

  return dirs;
}



static void
g_desktop_menu_collect_files (GDesktopMenu *menu,
                              GHashTable   *desktop_id_table)
{
  GList *app_dirs = NULL;
  GList *iter;
  GFile *file;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));

  app_dirs = g_desktop_menu_get_app_dirs (menu);

  /* Collect desktop entry filenames */
  for (iter = app_dirs; iter != NULL; iter = g_list_next (iter))
    {
      file = g_file_new_for_uri (iter->data);
      g_desktop_menu_collect_files_from_path (menu, desktop_id_table, file, NULL);
      g_object_unref (file);
    }

  /* Free directory list */
  g_list_free (app_dirs);

  /* Collect filenames for submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    g_desktop_menu_collect_files (iter->data, desktop_id_table);
}



static void
g_desktop_menu_collect_files_from_path (GDesktopMenu *menu,
                                        GHashTable   *desktop_id_table,
                                        GFile        *dir,
                                        const gchar  *id_prefix)
{
  GFileEnumerator *enumerator;
  GFileInfo       *file_info;
  GFile           *file;
  gchar           *basename;
  gchar           *new_id_prefix;
  gchar           *desktop_id;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));

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
          g_desktop_menu_collect_files_from_path (menu, desktop_id_table, file, new_id_prefix);

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
collect_rules (GNode  *node,
               GList **list)
{
  GDesktopMenuNodeType type;

  type = g_desktop_menu_node_tree_get_node_type (node);

  if (type == G_DESKTOP_MENU_NODE_TYPE_INCLUDE ||
      type == G_DESKTOP_MENU_NODE_TYPE_EXCLUDE)
    {
      *list = g_list_append (*list, node);
    }

  return FALSE;
}



static void
g_desktop_menu_resolve_items (GDesktopMenu *menu,
                              GHashTable   *desktop_id_table,
                              gboolean      only_unallocated)
{
  GList  *rules = NULL;
  GList  *iter;
  gboolean menu_only_unallocated = FALSE;

  g_return_if_fail (menu != NULL && G_IS_DESKTOP_MENU (menu));

  menu_only_unallocated = g_desktop_menu_node_tree_get_boolean_child (menu->priv->tree, 
                                                                      G_DESKTOP_MENU_NODE_TYPE_ONLY_UNALLOCATED);

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
          if (G_LIKELY (g_desktop_menu_node_tree_get_node_type (iter->data) == G_DESKTOP_MENU_NODE_TYPE_INCLUDE))
            {
              /* Resolve available items and match them against this rule */
              g_desktop_menu_resolve_items_by_rule (menu, desktop_id_table, iter->data);
            }
          else
            {
              /* Remove all items matching this exclude rule from the item pool */
              g_desktop_menu_item_pool_apply_exclude_rule (menu->priv->pool, iter->data);
            }
        }
    }

  /* Iterate over all submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    {
      /* Resolve items of the submenu */
      g_desktop_menu_resolve_items (G_DESKTOP_MENU (iter->data), desktop_id_table, only_unallocated);
    }
}



static void
g_desktop_menu_resolve_items_by_rule (GDesktopMenu *menu,
                                      GHashTable   *desktop_id_table,
                                      GNode        *node)
{
  GDesktopMenuPair pair;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));

  /* Store menu and rule pointer in the pair */
  pair.first = menu;
  pair.second = node;

  /* Try to insert each of the collected desktop entry filenames into the menu */
  g_hash_table_foreach (desktop_id_table, (GHFunc) g_desktop_menu_resolve_item_by_rule, &pair);
}



static void
g_desktop_menu_resolve_item_by_rule (const gchar      *desktop_id,
                                     const gchar      *uri,
                                     GDesktopMenuPair *data)
{
  GDesktopMenuItem *item = NULL;
  GDesktopMenu     *menu = NULL;
  GNode            *node = NULL;
  gboolean          only_unallocated = FALSE;

  g_return_if_fail (G_IS_DESKTOP_MENU (data->first));
  g_return_if_fail (data->second != NULL);

  /* Restore menu and rule from the data pair */
  menu = data->first;
  node = data->second;

  /* Try to load the menu item from the cache */
  item = g_desktop_menu_item_cache_lookup (menu->priv->cache, uri, desktop_id);

  if (G_LIKELY (item != NULL))
    {
      only_unallocated = g_desktop_menu_node_tree_get_boolean_child (menu->priv->tree,
                                                                     G_DESKTOP_MENU_NODE_TYPE_ONLY_UNALLOCATED);

      /* Only include item if menu not only includes unallocated items
       * or if the item is not allocated yet */
      if (!only_unallocated || g_desktop_menu_item_get_allocated (item) == 0)
        {
          /* Add item to the pool if it matches the include rule */
          if (G_LIKELY (g_desktop_menu_node_tree_rule_matches (node, item)))
            g_desktop_menu_item_pool_insert (menu->priv->pool, item);
        }
    }
}



static void
g_desktop_menu_remove_deleted_menus (GDesktopMenu *menu)
{
  GDesktopMenu *submenu;
  GList   *iter;
  gboolean  deleted;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));

  /* Note: There's a limitation: if the root menu has a <Deleted/> we
   * can't just free the pointer here. Therefor we only check child menus. */

  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    {
      submenu = iter->data;

      /* Check whether there is a <Deleted/> element */
      deleted = g_desktop_menu_node_tree_get_boolean_child (submenu->priv->tree, 
                                                            G_DESKTOP_MENU_NODE_TYPE_DELETED);

      /* Determine whether this submenu was deleted */
      if (G_LIKELY (submenu->priv->directory != NULL))
        deleted = deleted || g_desktop_menu_directory_get_hidden (submenu->priv->directory);

      /* Remove submenu if it is deleted, otherwise check submenus of the submenu */
      if (G_UNLIKELY (deleted))
        {
          /* Remove submenu from the list ... */
          menu->priv->submenus = g_list_remove_link (menu->priv->submenus, iter);

          /* ... and destroy it */
          g_object_unref (submenu);
        }
      else
        g_desktop_menu_remove_deleted_menus (submenu);
    }
}



GDesktopMenuItemPool*
g_desktop_menu_get_item_pool (GDesktopMenu *menu)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);

  return menu->priv->pool;
}



static void
items_collect (const gchar      *desktop_id,
               GDesktopMenuItem *item,
               GList           **listp)
{
  *listp = g_list_prepend (*listp, item);
}



/**
 * g_desktop_menu_get_items:
 * @menu : a #GDesktopMenu.
 *
 * Returns all #GDesktopMenuItem<!---->s included in @menu. The items are 
 * sorted by their display names in ascending order.
 *
 * The caller is responsible to free the returned list using
 * <informalexample><programlisting>
 * g_list_free (list);
 * </programlisting></informalexample>
 * when no longer needed.
 * 
 * Return value: list of #GDesktopMenuItem<!---->s included in @menu.
 **/
GList *
g_desktop_menu_get_items (GDesktopMenu *menu)
{
  GList *items = NULL;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);

  /* Collect the items in the pool */
  g_desktop_menu_item_pool_foreach (menu->priv->pool, (GHFunc) items_collect, &items);

  /* Sort items */
  items = g_list_sort (items, (GCompareFunc) g_desktop_menu_compare_items);

  return items;
}



static GNode *
g_desktop_menu_get_layout (GDesktopMenu *menu,
                           gboolean      default_only)
{
  GNode *layout = NULL;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);

  if (G_LIKELY (!default_only))
    {
      layout = g_desktop_menu_node_tree_get_child_node (menu->priv->tree, 
                                                        G_DESKTOP_MENU_NODE_TYPE_LAYOUT,
                                                        TRUE);
    }

  if (layout == NULL)
    {
      layout = g_desktop_menu_node_tree_get_child_node (menu->priv->tree,
                                                        G_DESKTOP_MENU_NODE_TYPE_DEFAULT_LAYOUT,
                                                        TRUE);

      if (layout == NULL && menu->priv->parent != NULL)
        layout = g_desktop_menu_get_layout (menu->priv->parent, TRUE);
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

  nodes = g_desktop_menu_node_tree_get_child_nodes (layout, G_DESKTOP_MENU_NODE_TYPE_MENUNAME, 
                                                    FALSE);

  for (iter = g_list_first (nodes); !has_menuname && iter != NULL; iter = g_list_next (iter))
    if (g_str_equal (g_desktop_menu_node_tree_get_string (iter->data), name))
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

  nodes = g_desktop_menu_node_tree_get_child_nodes (layout, G_DESKTOP_MENU_NODE_TYPE_FILENAME, 
                                                    FALSE);

  for (iter = g_list_first (nodes); !has_filename && iter != NULL; iter = g_list_next (iter))
    if (g_str_equal (g_desktop_menu_node_tree_get_string (iter->data), desktop_id))
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
      if (G_IS_DESKTOP_MENU (iter->data))
        {
          if (G_LIKELY (!layout_has_menuname (layout, g_desktop_menu_get_name (iter->data))))
            *dest_list = g_list_append (*dest_list, iter->data);
        }
      else if (G_IS_DESKTOP_MENU_ITEM (iter->data))
        {
          if (G_LIKELY (!layout_has_filename (layout, g_desktop_menu_item_get_desktop_id (iter->data))))
            *dest_list = g_list_append (*dest_list, iter->data);
        }
    }
}



GList *
g_desktop_menu_get_elements (GDesktopMenu *menu)
{
  GDesktopMenuLayoutMergeType merge_type;
  GDesktopMenuNodeType        type;
  GDesktopMenuItem           *item;
  GDesktopMenu               *submenu;
  GList                      *items = NULL;
  GList                      *menu_items;
  GNode                      *layout = NULL;
  GNode                      *node;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);

  /* Determine layout node */
  layout = g_desktop_menu_get_layout (menu, FALSE);

  /* There should always be a layout, otherwise GDesktopMenuMerger is broken */
  g_return_val_if_fail (layout != NULL, NULL);

  /* Process layout nodes in order */
  for (node = g_node_first_child (layout); node != NULL; node = g_node_next_sibling (node))
    {
      /* Determine layout node type */
      type = g_desktop_menu_node_tree_get_node_type (node);

      if (type == G_DESKTOP_MENU_NODE_TYPE_FILENAME)
        {
          /* Search for desktop ID in the item pool */
          item = g_desktop_menu_item_pool_lookup (menu->priv->pool, 
                                                  g_desktop_menu_node_tree_get_string (node));

          /* If the item with this desktop ID is included in the menu, append it to the list */
          if (G_LIKELY (item != NULL))
            items = g_list_append (items, item);
        }
      if (type == G_DESKTOP_MENU_NODE_TYPE_MENUNAME)
        {
          /* Search submenu with this name */
          submenu = g_desktop_menu_get_menu_with_name (menu, 
                                                       g_desktop_menu_node_tree_get_string (node));

          /* If there is such a menu, append it to the list */
          if (G_LIKELY (submenu != NULL))
            items = g_list_append (items, submenu);
        }
      else if (type == G_DESKTOP_MENU_NODE_TYPE_SEPARATOR)
        {
          /* Append separator to the list */
          items = g_list_append (items, g_desktop_menu_separator_get_default ());
        }
      else if (type == G_DESKTOP_MENU_NODE_TYPE_MERGE)
        {
          /* Determine merge type */
          merge_type = g_desktop_menu_node_tree_get_layout_merge_type (node);

          if (merge_type == G_DESKTOP_MENU_LAYOUT_MERGE_ALL)
            {
              /* Get all menu items of this menu */
              menu_items = g_desktop_menu_get_items (menu);
              
              /* Append submenus */
              menu_items = g_list_concat (menu_items, g_desktop_menu_get_menus (menu));

              /* Sort menu items */
              menu_items = g_list_sort (menu_items, (GCompareFunc) g_desktop_menu_compare_items);

              /* Append menu items to the returned item list */
              layout_elements_collect (&items, menu_items, layout);
            }
          else if (merge_type == G_DESKTOP_MENU_LAYOUT_MERGE_FILES)
            {
              /* Get all menu items of this menu */
              menu_items = g_desktop_menu_get_items (menu);

              /* Append menu items to the returned item list */
              layout_elements_collect (&items, menu_items, layout);
            }
          else if (merge_type == G_DESKTOP_MENU_LAYOUT_MERGE_MENUS)
            {
              /* Get all submenus */
              menu_items = g_desktop_menu_get_menus (menu);

              /* Append submenus to the returned item list */
              layout_elements_collect (&items, menu_items, layout);
            }
        }
    }
  
  return items;
}



static gint
g_desktop_menu_compare_items (gconstpointer *a,
                              gconstpointer *b)
{
  return g_utf8_collate (g_desktop_menu_element_get_name (G_DESKTOP_MENU_ELEMENT (a)), 
                         g_desktop_menu_element_get_name (G_DESKTOP_MENU_ELEMENT (b)));
}



static const gchar*
g_desktop_menu_get_element_name (GDesktopMenuElement *element)
{
  GDesktopMenu    *menu;
  const gchar *name = NULL;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (element), NULL);

  menu = G_DESKTOP_MENU (element);

  /* Try directory name first */
  if (menu->priv->directory != NULL)
    name = g_desktop_menu_directory_get_name (menu->priv->directory);

  /* Otherwise use the menu name as a fallback */
  if (name == NULL)
    name = g_desktop_menu_get_name (menu);

  return name;
}



static const gchar*
g_desktop_menu_get_element_comment (GDesktopMenuElement *element)
{
  GDesktopMenu *menu;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (element), NULL);

  menu = G_DESKTOP_MENU (element);

  if (menu->priv->directory == NULL)
    return NULL;
  else
    return g_desktop_menu_directory_get_comment (menu->priv->directory);
}



static const gchar*
g_desktop_menu_get_element_icon_name (GDesktopMenuElement *element)
{
  GDesktopMenu *menu;
  
  g_return_val_if_fail (G_IS_DESKTOP_MENU (element), NULL);

  menu = G_DESKTOP_MENU (element);

  if (menu->priv->directory == NULL)
    return NULL;
  else
    return g_desktop_menu_directory_get_icon (menu->priv->directory);
}



static gboolean
g_desktop_menu_get_element_visible (GDesktopMenuElement *element)
{
  GDesktopMenu *menu;
  GList    *items;
  GList    *iter;
  gboolean  visible = FALSE;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (element), FALSE);

  menu = G_DESKTOP_MENU (element);

  if (menu->priv->directory != NULL)
    {
      if (!g_desktop_menu_directory_get_visible (menu->priv->directory))
        return FALSE;
    }

  items = g_desktop_menu_get_elements (menu);
  for (iter = items; visible != TRUE && iter != NULL; iter = g_list_next (iter))
    {
      if (g_desktop_menu_element_get_visible (G_DESKTOP_MENU_ELEMENT (iter->data)))
        visible = TRUE;
    }

  g_list_free (items);
  return visible;
}



static gboolean
g_desktop_menu_get_element_show_in_environment (GDesktopMenuElement *element)
{
  GDesktopMenu *menu;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (element), FALSE);

  menu = G_DESKTOP_MENU (element);

  if (menu->priv->directory == NULL)
    return FALSE;
  else
    return g_desktop_menu_directory_get_show_in_environment (menu->priv->directory);
}



static gboolean
g_desktop_menu_get_element_no_display (GDesktopMenuElement *element)
{
  GDesktopMenu *menu;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (element), FALSE);

  menu = G_DESKTOP_MENU (element);

  if (menu->priv->directory == NULL)
    return FALSE;
  else
    return g_desktop_menu_directory_get_no_display (menu->priv->directory);
}



static void
item_monitor_start (const gchar      *desktop_id,
                    GDesktopMenuItem *item,
                    GDesktopMenu     *menu)
{
  g_desktop_menu_monitor_add_item (menu, item);
}



static void
g_desktop_menu_monitor_start (GDesktopMenu *menu)
{
  GList *iter;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));

  /* TODO Make monitoring work properly again */
#if 0
  /* Monitor the menu file */
  if (G_LIKELY (g_desktop_menu_monitor_has_flags (G_DESKTOP_MENU_MONITOR_MENU_FILES)))
    g_desktop_menu_monitor_add_file (menu, menu->priv->filename);

  /* Monitor the menu directory file */
  if (G_LIKELY (G_IS_DESKTOP_MENU_DIRECTORY (menu->priv->directory) 
                && g_desktop_menu_monitor_has_flags (G_DESKTOP_MENU_MONITOR_DIRECTORY_FILES)))
    {
      g_desktop_menu_monitor_add_file (menu, g_desktop_menu_directory_get_filename (menu->priv->directory));
    }

  /* Monitor the application directories */
  if (G_LIKELY (g_desktop_menu_monitor_has_flags (G_DESKTOP_MENU_MONITOR_DIRECTORIES)))
    for (iter = menu->priv->app_dirs; iter != NULL; iter = g_list_next (iter))
      g_desktop_menu_monitor_add_directory (menu, (const gchar *)iter->data);
#endif

  /* Monitor items in the menu pool */
  if (G_LIKELY (g_desktop_menu_monitor_has_flags (G_DESKTOP_MENU_MONITOR_DESKTOP_FILES)))
    g_desktop_menu_item_pool_foreach (menu->priv->pool, (GHFunc) item_monitor_start, menu);

  /* Monitor items in submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    g_desktop_menu_monitor_start (G_DESKTOP_MENU (iter->data));
}



static void
item_monitor_stop (const gchar      *desktop_id,
                   GDesktopMenuItem *item,
                   GDesktopMenu     *menu)
{
  g_desktop_menu_monitor_remove_item (menu, item);
}



static void
g_desktop_menu_monitor_stop (GDesktopMenu *menu)
{
  GList *iter;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));

  /* Stop monitoring items in submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_list_next (iter))
    g_desktop_menu_monitor_stop (G_DESKTOP_MENU (iter->data));

  /* Stop monitoring the items */
  g_desktop_menu_item_pool_foreach (menu->priv->pool, (GHFunc) item_monitor_stop, menu);

  /* TODO Make monitoring work properly again */
#if 0
  /* Stop monitoring the application directories */
  for (iter = menu->priv->app_dirs; iter != NULL; iter = g_list_next (iter))
    g_desktop_menu_monitor_remove_directory (menu, (const gchar *)iter->data);

  /* Stop monitoring the menu directory file */
  if (G_IS_DESKTOP_MENU_DIRECTORY (menu->priv->directory))
    g_desktop_menu_monitor_remove_file (menu, g_desktop_menu_directory_get_filename (menu->priv->directory));

  /* Stop monitoring the menu file */
  g_desktop_menu_monitor_remove_file (menu, menu->priv->filename);
#endif
}

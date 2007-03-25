/* $Id$ */
/* vi:set expandtab sw=2 sts=2 et: */
/*-
 * Copyright (c) 2006-2007 Jannis Pohlmann <jannis@xfce.org>
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
#include <libxfce4menu/xfce-menu-rules.h>
#include <libxfce4menu/xfce-menu-standard-rules.h>
#include <libxfce4menu/xfce-menu-or-rules.h>
#include <libxfce4menu/xfce-menu-and-rules.h>
#include <libxfce4menu/xfce-menu-not-rules.h>
#include <libxfce4menu/xfce-menu-directory.h>
#include <libxfce4menu/xfce-menu-item-pool.h>
#include <libxfce4menu/xfce-menu-item-cache.h>
#include <libxfce4menu/xfce-menu-move.h>
#include <libxfce4menu/xfce-menu-layout.h>
#include <libxfce4menu/xfce-menu-separator.h>
#include <libxfce4menu/xfce-menu-monitor.h>
#include <libxfce4menu/xfce-menu.h>



/* Use g_access() on win32 */
#if defined(G_OS_WIN32)
#include <glib/gstdio.h>
#else
#define g_access(filename, mode) (access ((filename), (mode)))
#endif



/* Potential root menu files */
static const gchar XFCE_MENU_ROOT_SPECS[][30] = 
{
  "menus/applications.menu",
  "menus/xfce-applications.menu",
  "menus/gnome-applications.menu",
  "menus/kde-applications.menu",
};



static gint xfce_menu_ref_count = 0;



/**
 * xfce_menu_init:
 * @env : Name of the desktop environment (e.g. XFCE, GNOME, KDE) or %NULL.
 *
 * Initializes the libxfce4menu library and optionally defines the desktop 
 * environment for which menus will be generated. This means items belonging
 * only to other desktop environments will be ignored.
 **/
void
xfce_menu_init (const gchar *env)
{
  if (g_atomic_int_exchange_and_add (&xfce_menu_ref_count, 1) == 0)
    {
      /* Initialize the GThread system */
      if (!g_thread_supported ())
        g_thread_init (NULL);

      /* Initialize the GObject type system */
      g_type_init ();

      /* Set desktop environment */
      xfce_menu_set_environment (env);

      /* Initialize the menu item cache */
      _xfce_menu_item_cache_init ();

      /* Initialize the directory module */
      _xfce_menu_directory_init ();

      /* Initialize monitoring system */
      _xfce_menu_monitor_init ();

      /* Creates the menu separator */
      _xfce_menu_separator_init ();
    }
}



/**
 * xfce_menu_shutdown:
 *
 * Shuts down the libxfce4menu library.
 **/
void
xfce_menu_shutdown (void)
{
  if (g_atomic_int_dec_and_test (&xfce_menu_ref_count))
    {
      /* Unset desktop environment */
      xfce_menu_set_environment (NULL);

      /* Destroys the menu separator */
      _xfce_menu_separator_shutdown ();

      /* Shutdown monitoring system */
      _xfce_menu_monitor_shutdown ();

      /* Shutdown the directory module */
      _xfce_menu_directory_shutdown ();

      /* Shutdown the menu item cache */
      _xfce_menu_item_cache_shutdown ();
    }
}



#define XFCE_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), XFCE_TYPE_MENU, XfceMenuPrivate))



/* Menu file parser states */
typedef enum 
{
  XFCE_MENU_PARSE_STATE_START,
  XFCE_MENU_PARSE_STATE_ROOT,
  XFCE_MENU_PARSE_STATE_MENU,
  XFCE_MENU_PARSE_STATE_RULE,
  XFCE_MENU_PARSE_STATE_END,
  XFCE_MENU_PARSE_STATE_MOVE,
  XFCE_MENU_PARSE_STATE_LAYOUT,

} XfceMenuParseState;

/* Menu file node types */
typedef enum
{
  XFCE_MENU_PARSE_NODE_TYPE_NONE,
  XFCE_MENU_PARSE_NODE_TYPE_NAME,
  XFCE_MENU_PARSE_NODE_TYPE_DIRECTORY,
  XFCE_MENU_PARSE_NODE_TYPE_APP_DIR,
  XFCE_MENU_PARSE_NODE_TYPE_LEGACY_DIR,
  XFCE_MENU_PARSE_NODE_TYPE_DIRECTORY_DIR,
  XFCE_MENU_PARSE_NODE_TYPE_FILENAME,
  XFCE_MENU_PARSE_NODE_TYPE_CATEGORY,
  XFCE_MENU_PARSE_NODE_TYPE_OLD,
  XFCE_MENU_PARSE_NODE_TYPE_NEW,
  XFCE_MENU_PARSE_NODE_TYPE_MENUNAME,

} XfceMenuParseNodeType;

/* Menu file parse context */
typedef struct _XfceMenuParseContext
{
  /* Menu to be parsed */
  XfceMenu             *root_menu;

  /* Parser state (position in XML tree */
  XfceMenuParseState    state;

  /* Menu hierarchy "stack" */
  GList                *menu_stack;

  /* Include/exclude rules stack */
  GList                *rule_stack;

  /* Current move instruction */
  XfceMenuMove         *move;

  /* Current node type (for text handler) */
  XfceMenuParseNodeType node_type;

} XfceMenuParseContext;

typedef struct _XfceMenuPair
{
  gpointer first;
  gpointer second;
} XfceMenuPair;

typedef struct _XfceMenuParseInfo
{
  /* Directory names */
  GSList     *directory_names;

  /* Desktop entry files items (desktop-file id => absolute filename) used for
   * resolving the menu items */
  GHashTable *files;

} XfceMenuParseInfo;



/* Property identifiers */
enum
{
  PROP_0,
  PROP_ENVIRONMENT,
  PROP_FILENAME,
  PROP_NAME,
  PROP_DIRECTORY,
  PROP_DIRECTORY_DIRS, /* TODO */
  PROP_LEGACY_DIRS, /* TODO */
  PROP_APP_DIRS, /* TODO Implement methods for this! */
  PROP_PARENT, /* TODO */
  PROP_ONLY_UNALLOCATED,
  PROP_DELETED,
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
static void               xfce_menu_start_element                          (GMarkupParseContext   *context,
                                                                            const gchar           *element_name,
                                                                            const gchar          **attribute_names,
                                                                            const gchar          **attribute_values,
                                                                            gpointer               user_data,
                                                                            GError               **error);
static void               xfce_menu_end_element                            (GMarkupParseContext   *context,
                                                                            const gchar           *element_name,
                                                                            gpointer               user_data,
                                                                            GError               **error);
static void               xfce_menu_characters                             (GMarkupParseContext   *context,
                                                                            const gchar           *text,
                                                                            gsize                  text_len,
                                                                            gpointer               user_data,
                                                                            GError               **error);
static void               xfce_menu_parse_info_add_directory_name          (XfceMenuParseInfo     *parse_info,
                                                                            const gchar           *name);
static void               xfce_menu_parse_info_free                        (XfceMenuParseInfo     *menu);
static void               xfce_menu_parse_info_consolidate_directory_names (XfceMenuParseInfo *parse_info);

static void               xfce_menu_add_directory_dir                      (XfceMenu              *menu,
                                                                            const gchar           *dir);
static void               xfce_menu_add_default_directory_dirs             (XfceMenu              *menu);
static void               xfce_menu_add_app_dir                            (XfceMenu              *menu,
                                                                            const gchar           *dir);
static void               xfce_menu_add_legacy_dir                         (XfceMenu              *menu,
                                                                            const gchar           *dir);
static void               xfce_menu_add_kde_legacy_dirs                    (XfceMenu              *menu);
static void               xfce_menu_add_default_app_dirs                   (XfceMenu              *menu);

static void               xfce_menu_resolve_legacy_menus                   (XfceMenu              *menu);
static void               xfce_menu_resolve_legacy_menu                    (XfceMenu              *menu,
                                                                            const gchar           *path);
static void               xfce_menu_remove_duplicates                      (XfceMenu              *menu);
static void               xfce_menu_consolidate_child_menus                (XfceMenu              *menu);
#if 0
static void               xfce_menu_merge_directory_name                   (const gchar           *name,
                                                                            XfceMenu              *menu);
static void               xfce_menu_merge_directory_dir                    (const gchar           *dir,
                                                                            XfceMenu              *menu);
static void               xfce_menu_merge_app_dir                          (const gchar           *dir,
                                                                            XfceMenu              *menu);
static void               xfce_menu_merge_rule                             (XfceMenuRules         *rule,
                                                                            XfceMenu              *menu);
#endif
static void               xfce_menu_consolidate_directory_dirs             (XfceMenu              *menu);
static void               xfce_menu_consolidate_app_dirs                   (XfceMenu              *menu);
static void               xfce_menu_resolve_directory                      (XfceMenu              *menu);
static XfceMenuDirectory *xfce_menu_lookup_directory                       (XfceMenu              *menu,
                                                                            const gchar           *filename);
static void               xfce_menu_add_rule                               (XfceMenu              *menu,
                                                                            XfceMenuRules         *rules);
static void               xfce_menu_add_move                               (XfceMenu              *menu,
                                                                            XfceMenuMove          *move);
static void               xfce_menu_collect_files                          (XfceMenu              *menu);
static void               xfce_menu_collect_files_from_path                (XfceMenu              *menu,
                                                                            const gchar           *path,
                                                                            const gchar           *id_prefix);
static void               xfce_menu_resolve_items                          (XfceMenu              *menu,
                                                                            gboolean               only_unallocated);
static void               xfce_menu_resolve_items_by_rule                  (XfceMenu              *menu,
                                                                            XfceMenuStandardRules *rule);
static void               xfce_menu_resolve_item_by_rule                   (const gchar           *desktop_id,
                                                                            const gchar           *filename,
                                                                            XfceMenuPair          *data);
static void               xfce_menu_resolve_deleted                        (XfceMenu              *menu);
static void               xfce_menu_resolve_moves                          (XfceMenu              *menu);
static gint               xfce_menu_compare_items                          (gconstpointer         *a,
                                                                            gconstpointer         *b);
static const gchar       *xfce_menu_get_element_name                       (XfceMenuElement       *element);
static const gchar       *xfce_menu_get_element_icon_name                  (XfceMenuElement       *element);
static void               xfce_menu_monitor_start                          (XfceMenu              *menu);
static void               xfce_menu_monitor_stop                           (XfceMenu              *menu);



struct _XfceMenuPrivate
{
  /* Menu filename */
  gchar             *filename;

  /* Menu name */
  gchar             *name;

  /* Directory */
  XfceMenuDirectory *directory;

  /* Submenus */
  GSList            *submenus;

  /* Parent menu */
  XfceMenu          *parent;

  /* Directory dirs */
  GSList            *directory_dirs;

  /* Legacy dirs */
  GSList            *legacy_dirs;

  /* App dirs */
  GSList            *app_dirs;

  /* Only include desktop entries not used in other menus */
  guint              only_unallocated : 1;

  /* Whether this menu should be ignored or not */
  guint              deleted : 1;

  /* Include/exclude rules */
  GSList            *rules;

  /* Move instructions */
  GSList            *moves;

  /* Menu item pool */
  XfceMenuItemPool  *pool;

  /* Shared menu item cache */
  XfceMenuItemCache *cache;

  /* Menu layout */
  XfceMenuLayout    *layout;

  /* Parse information (used for resolving) */
  XfceMenuParseInfo *parse_info;
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
   * XfceMenu:filename:
   *
   * The filename of an %XfceMenu object. Whenever this is redefined, the
   * menu is reloaded.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILENAME,
                                   g_param_spec_string ("filename",
                                                        "Filename",
                                                        "XML menu filename",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  /**
   * XfceMenu:name:
   *
   * The name of the menu.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "Name",
                                                        "Menu name",
                                                        NULL,
                                                        G_PARAM_READWRITE));

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

  /**
   * XfceMenu:only-unallocated:
   *
   * Whether this menu should only contain desktop entries not used by other
   * menus.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_ONLY_UNALLOCATED,
                                   g_param_spec_boolean ("only-unallocated",
                                                         "Only unallocated",
                                                         "Whether this menu only contains unallocated entries",
                                                         FALSE,
                                                         G_PARAM_READWRITE));

  /**
   * XfceMenu:deleted:
   *
   * Whether this menu should be ignored.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_ONLY_UNALLOCATED,
                                   g_param_spec_boolean ("deleted",
                                                         "Deleted",
                                                         "Whether this menu should be ignored",
                                                         FALSE,
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
  menu->priv->filename = NULL;
  menu->priv->name = NULL;
  menu->priv->directory = NULL;
  menu->priv->submenus = NULL;
  menu->priv->parent = NULL;
  menu->priv->directory_dirs = NULL;
  menu->priv->legacy_dirs = NULL;
  menu->priv->app_dirs = NULL;
  menu->priv->only_unallocated = FALSE;
  menu->priv->rules = NULL;
  menu->priv->moves = NULL;
  menu->priv->pool = xfce_menu_item_pool_new ();
  menu->priv->layout = xfce_menu_layout_new ();

  /* Take reference on the menu item cache */
  menu->priv->cache = xfce_menu_item_cache_get_default ();

  menu->priv->parse_info = g_new (XfceMenuParseInfo, 1);
  menu->priv->parse_info->directory_names = NULL;
  menu->priv->parse_info->files = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}



static void
xfce_menu_finalize (GObject *object)
{
  XfceMenu *menu = XFCE_MENU (object);

  /* Stop monitoring */
  xfce_menu_monitor_stop (menu);

  /* Free filename */
  g_free (menu->priv->filename);

  /* Free name */
  g_free (menu->priv->name);

  /* Free directory */
  if (G_LIKELY (menu->priv->directory != NULL))
    g_object_unref (menu->priv->directory);

  /* Free directory dirs */
  g_slist_foreach (menu->priv->directory_dirs, (GFunc) g_free, NULL);
  g_slist_free (menu->priv->directory_dirs);

  /* Free legacy dirs (check if this is the best way to free the list) */
  g_slist_foreach (menu->priv->legacy_dirs, (GFunc) g_free, NULL);
  g_slist_free (menu->priv->legacy_dirs);

  /* Free app dirs */
  g_slist_foreach (menu->priv->app_dirs, (GFunc) g_free, NULL);
  g_slist_free (menu->priv->app_dirs);

  /* TODO Free submenus etc. */
  g_slist_foreach (menu->priv->submenus, (GFunc) g_object_unref, NULL);
  g_slist_free (menu->priv->submenus);

  /* Free rules */
  g_slist_foreach (menu->priv->rules, (GFunc) g_object_unref, NULL);
  g_slist_free (menu->priv->rules);

  /* Free move instructions */
  g_slist_foreach (menu->priv->moves, (GFunc) g_object_unref, NULL);
  g_slist_free (menu->priv->moves);

  /* Free item pool */
  g_object_unref (G_OBJECT (menu->priv->pool));

  /* Free menu layout */
  g_object_unref (G_OBJECT (menu->priv->layout));

  /* Release item cache reference */
  g_object_unref (G_OBJECT (menu->priv->cache));

  /* Free parse information */
  xfce_menu_parse_info_free (menu->priv->parse_info);

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
    case PROP_FILENAME:
      g_value_set_string (value, xfce_menu_get_filename (menu));
      break;

    case PROP_NAME:
      g_value_set_string (value, xfce_menu_get_name (menu));
      break;

    case PROP_DIRECTORY:
      g_value_set_object (value, xfce_menu_get_directory (menu));
      break;

    case PROP_ONLY_UNALLOCATED:
      g_value_set_boolean (value, xfce_menu_get_only_unallocated (menu));
      break;

    case PROP_DELETED:
      g_value_set_boolean (value, xfce_menu_get_deleted (menu));
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
    case PROP_FILENAME:
      xfce_menu_set_filename (menu, g_value_get_string (value));
      break;

    case PROP_NAME:
      xfce_menu_set_name (menu, g_value_get_string (value));
      break;

    case PROP_DIRECTORY:
      xfce_menu_set_directory (menu, g_value_get_object (value));
      break;

    case PROP_ONLY_UNALLOCATED:
      xfce_menu_set_only_unallocated (menu, g_value_get_boolean (value));
      break;

    case PROP_DELETED:
      xfce_menu_set_deleted (menu, g_value_get_boolean (value));
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
          root_menu = xfce_menu_new (filename, NULL);
          if (G_LIKELY (root_menu != NULL))
            {
              /* Add weak pointer on the menu */
              g_object_add_weak_pointer (G_OBJECT (root_menu), (gpointer) &root_menu);
            }

          /* Free filename string */
          g_free (filename);
        }

      /* Check if we failed to load the root menu */
      if (G_UNLIKELY (root_menu == NULL))
        {
          /* Let the caller know there was no suitable file */
          g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, _("Failed to locate the application root menu"));
        }
    }
  else
    g_object_ref (G_OBJECT (root_menu));
  
  return root_menu;
}



/**
 * xfce_menu_new:
 * @filename : filename containing the menu structure you want to load.
 * @error    : return location for errors or %NULL.
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
 * Return value: Menu structure found in @filename.
 **/
XfceMenu*
xfce_menu_new (const gchar *filename, 
               GError     **error)
{
  XfceMenu *menu;

  g_return_val_if_fail (filename != NULL && g_path_is_absolute (filename), NULL);

  /* Create new menu */
  menu = g_object_new (XFCE_TYPE_MENU, "filename", filename, NULL);

  /* Try to load the menu structure */
  if (!xfce_menu_load (menu, error))
    {
      g_object_unref (G_OBJECT (menu));
      return NULL;
    }

  return menu;
}



/**
 * xfce_menu_get_filename:
 * @menu : a #XfceMenu.
 *
 * Returns the filename from which @menu was loaded.
 * 
 * Return value: filename from which @menu was loaded.
 */
const gchar*
xfce_menu_get_filename (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return menu->priv->filename;
}



/**
 * xfce_menu_set_filename:
 * @menu     : a #XfceMenu.
 * @filename : new filename of the menu.
 *
 * Sets the menu filename. It should not be necessary to call this
 * function anywhere - it's only of internal use.
 */
void
xfce_menu_set_filename (XfceMenu *menu, const gchar *filename)
{
  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Abort if filenames are equal */
  if (G_UNLIKELY (menu->priv->filename != NULL))
    {
      if (G_UNLIKELY (filename != NULL && g_utf8_collate (filename, menu->priv->filename) == 0))
        return;

      /* Free old filename */
      g_free (menu->priv->filename);
    }

  /* Set the new filename */
  menu->priv->filename = g_strdup (filename);

  /* Notify listeners */
  g_object_notify (G_OBJECT (menu), "filename");
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
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  return menu->priv->name;
}



/**
 * xfce_menu_set_name:
 * @menu : a #XfceMenu
 * @name : new name of the menu.
 *
 * Sets the name of @menu. This might come in handy if you want
 * to replace certain menu names with your own names. However, in
 * most cases this function won't be useful.
 */
void
xfce_menu_set_name (XfceMenu    *menu, 
                    const gchar *name)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (name != NULL);

  /* Abort if names are equal */
  if (G_UNLIKELY (menu->priv->name != NULL))
    {
      if (G_UNLIKELY (g_utf8_collate (name, menu->priv->name) == 0))
        return;

      /* Free old name */
      g_free (menu->priv->name);
    }

  /* Set the new filename */
  menu->priv->name = g_strdup (name);

  /* Notify listeners */
  g_object_notify (G_OBJECT (menu), "name");
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
#if GLIB_CHECK_VERSION(2,10,0)
  g_object_ref_sink (G_OBJECT (directory));
#else
  g_object_ref (G_OBJECT (directory));
#endif

  /* Set the new directory */
  menu->priv->directory = directory;

  /* Notify listeners */
  g_object_notify (G_OBJECT (menu), "directory");
}



/**
 * xfce_menu_get_only_unallocated:
 * @menu : a #XfceMenu.
 *
 * Returns whether @menu only contains #XfceMenuItem<!---->s which
 * are not already included in other menus.
 *
 * Return value: Whether the menu contains only items not used already
 *               included in other menus.
 */
gboolean
xfce_menu_get_only_unallocated (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), FALSE);
  return menu->priv->only_unallocated;
}



/**
 * xfce_menu_set_only_unallocated:
 * @menu             : a #XfceMenu.
 * @only_unallocated : Whether to include only unused 
 *                     #XfceMenuItem<!---->s
 *
 * Since all items are resolved directly after parsing the
 * menu file, this won't be useful other than internally.
 */
void
xfce_menu_set_only_unallocated (XfceMenu *menu,
                                gboolean  only_unallocated)
{
  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Abort if values are equal */
  if (menu->priv->only_unallocated == only_unallocated)
    return;

  /* Set new value */
  menu->priv->only_unallocated = only_unallocated;

  /* Notify listeners */
  g_object_notify (G_OBJECT (menu), "only-unallocated");
}



gboolean
xfce_menu_get_deleted (XfceMenu *menu)
{
  g_return_val_if_fail (XFCE_IS_MENU (menu), FALSE);
  return menu->priv->deleted;
}



void
xfce_menu_set_deleted (XfceMenu *menu,
                       gboolean  deleted)
{
  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Abort if values are equal */
  if (menu->priv->deleted == deleted)
    return;

  /* Set new value */
  menu->priv->deleted = deleted;

  /* Notify listeners */
  g_object_notify (G_OBJECT (menu), "deleted");
}



/**
 * xfce_menu_get_directory_dirs:
 * @menu : a #XfceMenu
 *
 * Returns a list with all directory dirs of @menu. Collects directory 
 * dirs from @menu up to the root menu so that the root menu directory 
 * dirs come first.
 *
 * Return value: List with all relevant directory dirs of @menu.
 */
GSList*
xfce_menu_get_directory_dirs (XfceMenu *menu)
{
  XfceMenu *current_menu;
  GSList   *directories = NULL;
  GList    *menus = NULL;
  GList    *iter;
  GSList    *iter2;
  
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  /* Collect all menus from menu -> parent -> ... -> root */
  for (current_menu = menu; current_menu != NULL; current_menu = current_menu->priv->parent)
    menus = g_list_prepend (menus, current_menu);

  /* Iterate over all menus from root -> parent -> ... -> menu */
  for (iter = menus; iter != NULL; iter = g_list_next (iter))
    {
      /* Fetch current menu */
      current_menu = XFCE_MENU (iter->data);

      /* Iterate over all directory dirs */
      for (iter2 = current_menu->priv->directory_dirs; iter2 != NULL; iter2 = g_slist_next (iter2))
        {
          /* Append directory dir to the list */
          directories = g_slist_append (directories, iter2->data);
        }

      /* Free the directory dir list */
      g_slist_free (iter2);
    }

  /* Free menu list */
  g_list_free (menus);

  return directories;
}



GSList*
xfce_menu_get_legacy_dirs (XfceMenu *menu)
{
  /* FIXME: Collecting legacy dirs from the bottom up might be wrong. Perhaps
   *        only <Menu> items with <LegacyDir> elements are allowed to parse
   *        legacy menu hierarchies - verify this!
   */

  XfceMenu *current_menu;
  GSList   *directories = NULL;
  GList    *menus = NULL;
  GList    *iter;
  GSList    *iter2;
  
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  /* Collect all menus from menu -> parent -> ... -> root */
  for (current_menu = menu; current_menu != NULL; current_menu = current_menu->priv->parent)
    menus = g_list_prepend (menus, current_menu);

  /* Iterate over all menus from root -> parent -> ... -> menu */
  for (iter = menus; iter != NULL; iter = g_list_next (iter))
    {
      /* Fetch current menu */
      current_menu = XFCE_MENU (iter->data);

      /* Iterate over all legacy directories */
      for (iter2 = current_menu->priv->legacy_dirs; iter2 != NULL; iter2 = g_slist_next (iter2))
        {
          /* Append legacy dir to the list */
          directories = g_slist_append (directories, iter2->data);
        }

      /* Free the legacy dir list */
      g_slist_free (iter2);
    }

  /* Free menu list */
  g_list_free (menus);

  return directories;
}



GSList*
xfce_menu_get_app_dirs (XfceMenu *menu)
{
  XfceMenu *current_menu;
  GSList   *directories = NULL;
  GList    *menus = NULL;
  GList    *iter;
  GSList    *iter2;
  
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);

  /* Collect all menus from menu -> parent -> ... -> root */
  for (current_menu = menu; current_menu != NULL; current_menu = current_menu->priv->parent)
    menus = g_list_prepend (menus, current_menu);

  /* Iterate over all menus from root -> parent -> ... -> menu */
  for (iter = menus; iter != NULL; iter = g_list_next (iter))
    {
      /* Fetch current menu */
      current_menu = XFCE_MENU (iter->data);

      /* Iterate over all application directories */
      for (iter2 = current_menu->priv->app_dirs; iter2 != NULL; iter2 = g_slist_next (iter2))
        {
          /* Append app dir to the list */
          directories = g_slist_append (directories, iter2->data);
        }

      /* Free the app dir list */
      g_slist_free (iter2);
    }

  /* Free menu list */
  g_list_free (menus);

  return directories;
}



static gboolean
xfce_menu_load (XfceMenu *menu, GError **error)
{
  /* Parser structure (connect handlers etc.) */
  GMarkupParseContext *context;
  GMarkupParser parser = {
      xfce_menu_start_element,
      xfce_menu_end_element,
      xfce_menu_characters,
      NULL,
      NULL
  };
  XfceMenuParseContext menu_context;

  /* File content information */
  gchar *contents;
  gsize contents_length;
  GIOStatus status;
  GIOChannel *stream;

  g_return_val_if_fail (XFCE_IS_MENU (menu), FALSE);
  g_return_val_if_fail (menu->priv->filename != NULL, FALSE);

  /* Try to open the menu file */
  stream = g_io_channel_new_file (menu->priv->filename, "r", error);

  if (G_UNLIKELY (stream == NULL))
    return FALSE;

  /* Try to read the menu file */
  status = g_io_channel_read_to_end (stream, &contents, &contents_length, error);
  
  /* Free IO handle */
  g_io_channel_unref (stream);

  if (G_UNLIKELY (status != G_IO_STATUS_NORMAL))
    return FALSE;

  /* Define menu parse context */
  menu_context.root_menu = menu;
  menu_context.state = XFCE_MENU_PARSE_STATE_START;
  menu_context.node_type = XFCE_MENU_PARSE_NODE_TYPE_NONE;
  menu_context.menu_stack = NULL;
  menu_context.rule_stack = NULL;
  menu_context.move = NULL;

  /* Allocate parse context */
  context = g_markup_parse_context_new (&parser, 0, &menu_context, NULL);

  /* Try to parse the menu file */
  if (!g_markup_parse_context_parse (context, contents, contents_length, error) || !g_markup_parse_context_end_parse (context, error))
    {
      g_markup_parse_context_free (context);
      return FALSE;
    }
  
  /* Free file contents */
  g_free (contents);

  /* Free parse context */
  g_markup_parse_context_free (context);

  /* Free menu parse context */
  g_list_free (menu_context.menu_stack);
  g_list_free (menu_context.rule_stack);

#if 0
  xfce_menu_resolve_legacy_menus (menu);
#endif
  xfce_menu_remove_duplicates (menu);
  xfce_menu_resolve_directory (menu);
  xfce_menu_resolve_moves (menu);

  /* Collect all potential menu item filenames */
  xfce_menu_collect_files (menu);

  /* Resolve menu items in two steps to handle <OnlyUnallocated/> properly */
  xfce_menu_resolve_items (menu, FALSE);
  xfce_menu_resolve_items (menu, TRUE);
  
  /* Remove deleted menus */
  xfce_menu_resolve_deleted (menu);

  /* Start monitoring */
  xfce_menu_monitor_start (menu);

  return TRUE;
}



static void
xfce_menu_start_element (GMarkupParseContext *context,
                         const gchar         *element_name,
                         const gchar        **attribute_names,
                         const gchar        **attribute_values,
                         gpointer             user_data,
                         GError             **error)
{
  XfceMenuParseContext *menu_context = (XfceMenuParseContext *)user_data;
  XfceMenu             *current_menu;
  XfceMenuRules        *current_rule;

  switch (menu_context->state) 
    {
    case XFCE_MENU_PARSE_STATE_START:
      if (g_utf8_collate (element_name, "Menu") == 0)
        {
          menu_context->state = XFCE_MENU_PARSE_STATE_ROOT;
          menu_context->menu_stack = g_list_prepend (menu_context->menu_stack, menu_context->root_menu);
        }
      break;

    case XFCE_MENU_PARSE_STATE_ROOT:
    case XFCE_MENU_PARSE_STATE_MENU:
      /* Fetch current menu from stack */
      current_menu = g_list_first (menu_context->menu_stack)->data;

      if (g_utf8_collate (element_name, "Name") == 0)
        menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_NAME;

      else if (g_utf8_collate (element_name, "Directory") == 0)
        menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_DIRECTORY;
      else if (g_utf8_collate (element_name, "DirectoryDir") == 0)
        menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_DIRECTORY_DIR;
      else if (g_utf8_collate (element_name, "DefaultDirectoryDirs") == 0)
        xfce_menu_add_default_directory_dirs (current_menu);

      else if (g_utf8_collate (element_name, "DefaultAppDirs") == 0)
        xfce_menu_add_default_app_dirs (current_menu);
      else if (g_utf8_collate (element_name, "AppDir") == 0)
        menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_APP_DIR;

      else if (g_utf8_collate (element_name, "KDELegacyDirs") == 0)
        xfce_menu_add_kde_legacy_dirs (current_menu);
      else if (g_utf8_collate (element_name, "LegacyDir") == 0)
        menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_LEGACY_DIR;

      else if (g_utf8_collate (element_name, "OnlyUnallocated") == 0)
        xfce_menu_set_only_unallocated (current_menu, TRUE);
      else if (g_utf8_collate (element_name, "NotOnlyUnallocated") == 0)
        xfce_menu_set_only_unallocated (current_menu, FALSE);

      else if (g_utf8_collate (element_name, "Deleted") == 0)
        xfce_menu_set_deleted (current_menu, TRUE);
      else if (g_utf8_collate (element_name, "NotDeleted") == 0)
        xfce_menu_set_deleted (current_menu, FALSE);

      else if (g_utf8_collate (element_name, "Include") == 0)
        {
          /* Create include rule */
          XfceMenuOrRules *rule = xfce_menu_or_rules_new ();

          /* Set include property */
          xfce_menu_standard_rules_set_include (XFCE_MENU_STANDARD_RULES (rule), TRUE);

          /* Add rule to the menu */
          xfce_menu_add_rule (current_menu, XFCE_MENU_RULES (rule));

          /* Put rule to the stack */
          menu_context->rule_stack = g_list_prepend (menu_context->rule_stack, rule);

          /* Set parse state */
          menu_context->state = XFCE_MENU_PARSE_STATE_RULE;
        }
      else if (g_utf8_collate (element_name, "Exclude") == 0)
        {
          /* Create exclude rule */
          XfceMenuOrRules *rule = xfce_menu_or_rules_new ();

          /* Set include property */
          xfce_menu_standard_rules_set_include (XFCE_MENU_STANDARD_RULES (rule), FALSE);

          /* Add rule to the menu */
          xfce_menu_add_rule (current_menu, XFCE_MENU_RULES (rule));

          /* Put rule to the stack */
          menu_context->rule_stack = g_list_prepend (menu_context->rule_stack, rule);

          /* Set parse state */
          menu_context->state = XFCE_MENU_PARSE_STATE_RULE;
        }

      else if (g_utf8_collate (element_name, "Menu") == 0)
        {
          /* Create new menu */
          XfceMenu *menu = g_object_new (XFCE_TYPE_MENU, "filename", current_menu->priv->filename, NULL);

          /* Add menu as submenu to the current menu */
          xfce_menu_add_menu (current_menu, menu); 

          /* Menu is now owned by the current menu, so we can release it */
          g_object_unref (menu);

          /* Set parse state */
          menu_context->state = XFCE_MENU_PARSE_STATE_MENU;

          /* Push new menu to the stack */
          menu_context->menu_stack = g_list_prepend (menu_context->menu_stack, menu);
        }

      else if (g_utf8_collate (element_name, "Move") == 0)
        {
          /* Set parse state */
          menu_context->state = XFCE_MENU_PARSE_STATE_MOVE;
        }
      else if (g_utf8_collate (element_name, "Layout") == 0)
        {
          /* Set parse state */
          menu_context->state = XFCE_MENU_PARSE_STATE_LAYOUT;
        }
      
      break;

    case XFCE_MENU_PARSE_STATE_RULE:
      /* Fetch current rule from stack */
      current_rule = XFCE_MENU_RULES (g_list_first (menu_context->rule_stack)->data);

      if (g_utf8_collate (element_name, "All") == 0)
        {
          xfce_menu_rules_add_all (current_rule);
        }
      else if (g_utf8_collate (element_name, "Filename") == 0)
        {
          menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_FILENAME;
        }
      else if (g_utf8_collate (element_name, "Category") == 0)
        {
          menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_CATEGORY;
        }
      else if (g_utf8_collate (element_name, "And") == 0)
        {
          /* Create include rule */
          XfceMenuAndRules *rule = xfce_menu_and_rules_new ();

          /* Add rule to the current rule */
          xfce_menu_rules_add_rules (current_rule, XFCE_MENU_RULES (rule));

          /* Rule is now owned by current rule, so we can release it */
          g_object_unref (rule);

          /* Put rule to the stack */
          menu_context->rule_stack = g_list_prepend (menu_context->rule_stack, rule);
        }
      else if (g_utf8_collate (element_name, "Or") == 0)
        {
          /* Create include rule */
          XfceMenuOrRules *rule = xfce_menu_or_rules_new ();

          /* Add rule to the current rule */
          xfce_menu_rules_add_rules (current_rule, XFCE_MENU_RULES (rule));

          /* Rule is now owned by current rule, so we can release it */
          g_object_unref (rule);

          /* Put rule to the stack */
          menu_context->rule_stack = g_list_prepend (menu_context->rule_stack, rule);
        }
      else if (g_utf8_collate (element_name, "Not") == 0)
        {
          /* Create include rule */
          XfceMenuNotRules *rule = xfce_menu_not_rules_new ();

          /* Add rule to the current rule */
          xfce_menu_rules_add_rules (current_rule, XFCE_MENU_RULES (rule));

          /* Rule is now owned by current rule, so we can release it */
          g_object_unref (rule);

          /* Put rule to the stack */
          menu_context->rule_stack = g_list_prepend (menu_context->rule_stack, rule);
        }

      break;

    case XFCE_MENU_PARSE_STATE_MOVE:
      /* Fetch current menu from stack */
      current_menu = g_list_first (menu_context->menu_stack)->data;

      if (g_utf8_collate (element_name, "Old") == 0)
        {
          /* Create a new move instruction in the parse context */
          menu_context->move = xfce_menu_move_new ();

          /* Add move instruction to the menu */
          xfce_menu_add_move (current_menu, menu_context->move);

          menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_OLD;
        }
      else if (g_utf8_collate (element_name, "New") == 0)
        menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_NEW;

      break;

    case XFCE_MENU_PARSE_STATE_LAYOUT:
      /* Fetch current menu from stack */
      current_menu = g_list_first (menu_context->menu_stack)->data;

      if (g_utf8_collate (element_name, "Filename") == 0)
        {
          /* Set node type */
          menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_FILENAME;
        }
      else if (g_utf8_collate (element_name, "Menuname") == 0)
        {
          /* TODO Parse attributes */
          
          /* Set node type */
          menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_MENUNAME;
        }
      else if (g_utf8_collate (element_name, "Separator") == 0)
        {
          /* Add separator to the menu layout */
          xfce_menu_layout_add_separator (current_menu->priv->layout);
        }
      else if (g_utf8_collate (element_name, "Merge") == 0)
        {
          XfceMenuLayoutMergeType type = XFCE_MENU_LAYOUT_MERGE_ALL;

          gboolean type_found = FALSE;
          gint     i;

          /* Find 'type' attribute */
          for (i = 0; i < g_strv_length ((gchar **)attribute_names); ++i) 
            {
              if (g_utf8_collate (attribute_names[i], "type") == 0)
                {
                  /* Determine merge type */
                  if (g_utf8_collate (attribute_values[i], "menus") == 0)
                    type = XFCE_MENU_LAYOUT_MERGE_MENUS;
                  else if (g_utf8_collate (attribute_values[i], "files") == 0)
                    type = XFCE_MENU_LAYOUT_MERGE_FILES;
                  else if (g_utf8_collate (attribute_values[i], "all") == 0)
                    type = XFCE_MENU_LAYOUT_MERGE_ALL;

                  type_found = TRUE;
                }
            }

          /* Add merge to the menu layout */
          xfce_menu_layout_add_merge (current_menu->priv->layout, type);
        }
    default:
      break;
    }
}



static void 
xfce_menu_end_element (GMarkupParseContext *context,
                       const gchar         *element_name,
                       gpointer             user_data,
                       GError             **error)
{
  XfceMenuParseContext *menu_context = (XfceMenuParseContext *)user_data;

  switch (menu_context->state)
    {
    case XFCE_MENU_PARSE_STATE_ROOT:
      if (g_utf8_collate (element_name, "Menu") == 0)
        {
          /* Remove root menu from stack */
          menu_context->menu_stack = g_list_delete_link (menu_context->menu_stack, g_list_first (menu_context->menu_stack));

          /* Set parser state */
          menu_context->state = XFCE_MENU_PARSE_STATE_END;
        }
      break;

    case XFCE_MENU_PARSE_STATE_MENU:
      if (g_utf8_collate (element_name, "Menu") == 0)
        {
          /* Remove current menu from stack */
          menu_context->menu_stack = g_list_delete_link (menu_context->menu_stack, g_list_first (menu_context->menu_stack));

          /* Set parse state to _STATE_ROOT only if there are no other menus
           * left on the stack. Otherwise, we're still inside a <Menu> element. */
          if (G_LIKELY (g_list_length (menu_context->menu_stack) == 1))
            menu_context->state = XFCE_MENU_PARSE_STATE_ROOT;
        }
      break;

    case XFCE_MENU_PARSE_STATE_RULE:
      if (g_utf8_collate (element_name, "Include") == 0 || g_utf8_collate (element_name, "Exclude") == 0 || g_utf8_collate (element_name, "Or") == 0 || g_utf8_collate (element_name, "And") == 0 || g_utf8_collate (element_name, "Not") == 0)
        {
          /* Remove current rule from stack */
          menu_context->rule_stack = g_list_delete_link (menu_context->rule_stack, g_list_first (menu_context->rule_stack));

          /* Set parse state */
          if (g_list_length (menu_context->rule_stack) == 0)
            {
              if (g_list_length (menu_context->menu_stack) > 1) 
                menu_context->state = XFCE_MENU_PARSE_STATE_MENU;
              else
                menu_context->state = XFCE_MENU_PARSE_STATE_ROOT;
            }
        }
      break;

    case XFCE_MENU_PARSE_STATE_MOVE:
      if (g_utf8_collate (element_name, "Move") == 0)
        {
          /* Set menu parse state */
          menu_context->state = XFCE_MENU_PARSE_STATE_MENU;

          /* Handle incomplete move commands (those missing a <New> element) */
          if (G_UNLIKELY (menu_context->move != NULL && xfce_menu_move_get_new (menu_context->move) == NULL))
            {
              /* Determine current menu */
              XfceMenu *current_menu = XFCE_MENU (g_list_first (menu_context->menu_stack)->data);

              /* Remove move command from the menu */
              current_menu->priv->moves = g_slist_remove (current_menu->priv->moves, menu_context->move);

              /* Free the move command */
              g_object_unref (menu_context->move);
            }
        }
      else if (g_utf8_collate (element_name, "New") == 0)
        menu_context->move = NULL;
      break;

    case XFCE_MENU_PARSE_STATE_LAYOUT:
      if (g_utf8_collate (element_name, "Layout") == 0)
        {
          if (g_list_length (menu_context->menu_stack) > 1)
            menu_context->state = XFCE_MENU_PARSE_STATE_MENU;
          else
            menu_context->state = XFCE_MENU_PARSE_STATE_ROOT;
        }
      break;

    default:
      break;
    }
}



static void
xfce_menu_characters (GMarkupParseContext *context,
                      const gchar         *text,
                      gsize                text_len,
                      gpointer             user_data,
                      GError             **error)
{
  XfceMenuParseContext *menu_context = (XfceMenuParseContext *)user_data;
  XfceMenu             *current_menu = g_list_first (menu_context->menu_stack)->data;
  XfceMenuRules        *current_rule = NULL;

  /* Generate NULL-terminated string */
  gchar *content = g_strndup (text, text_len);

  /* Fetch current rule from stack (if possible) */
  if (g_list_length (menu_context->rule_stack) > 0)
    current_rule = g_list_first (menu_context->rule_stack)->data;

  switch (menu_context->node_type)
    {
    case XFCE_MENU_PARSE_NODE_TYPE_NAME:
      xfce_menu_set_name (current_menu, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_DIRECTORY:
      xfce_menu_parse_info_add_directory_name (current_menu->priv->parse_info, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_DIRECTORY_DIR:
      xfce_menu_add_directory_dir (current_menu, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_APP_DIR:
      xfce_menu_add_app_dir (current_menu, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_LEGACY_DIR:
      xfce_menu_add_legacy_dir (current_menu, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_FILENAME:
      if (menu_context->state == XFCE_MENU_PARSE_STATE_RULE) 
        {
          if (G_LIKELY (current_rule != NULL))
            xfce_menu_rules_add_filename (current_rule, content);
        }
      else if (menu_context->state == XFCE_MENU_PARSE_STATE_LAYOUT)
        xfce_menu_layout_add_filename (current_menu->priv->layout, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_CATEGORY:
      if (G_LIKELY (current_rule != NULL))
        xfce_menu_rules_add_category (current_rule, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_OLD:
      xfce_menu_move_set_old (menu_context->move, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_NEW:
      if (G_LIKELY (menu_context->move != NULL))
        xfce_menu_move_set_new (menu_context->move, content);
      break;

    case XFCE_MENU_PARSE_NODE_TYPE_MENUNAME:
      xfce_menu_layout_add_menuname (current_menu->priv->layout, content);
      break;

    default:
      break;
    }

  /* Free string */
  g_free (content);

  /* Invalidate node type information */
  menu_context->node_type = XFCE_MENU_PARSE_NODE_TYPE_NONE;
}



static void
xfce_menu_parse_info_add_directory_name (XfceMenuParseInfo *parse_info,
                                         const gchar       *name)
{
  g_return_if_fail (name != NULL);
  parse_info->directory_names = g_slist_append (parse_info->directory_names, g_strdup (name));
}



static void
xfce_menu_parse_info_consolidate_directory_names (XfceMenuParseInfo *parse_info)
{
  GSList *names = NULL;
  GSList *iter;

  g_return_if_fail (parse_info != NULL);

  /* Iterate over directory names in reverse order */
  for (iter = g_slist_reverse (parse_info->directory_names); iter != NULL; iter = g_slist_next (iter))
    {
      /* Prepend name to the new list unless it already exists */
      if (G_LIKELY (g_slist_find_custom (names, iter->data, (GCompareFunc) g_utf8_collate) == NULL))
        names = g_slist_prepend (names, g_strdup (iter->data));
    }

  /* Free old list */
  g_slist_foreach (parse_info->directory_names, (GFunc) g_free, NULL);
  g_slist_free (parse_info->directory_names);

  parse_info->directory_names = names;
}



static void
xfce_menu_parse_info_free (XfceMenuParseInfo *parse_info)
{
  g_return_if_fail (parse_info != NULL);

  /* Free directory names */
  g_slist_foreach (parse_info->directory_names, (GFunc) g_free, NULL);
  g_slist_free (parse_info->directory_names);

#if GLIB_CHECK_VERSION(2,12,0)
  g_hash_table_unref (parse_info->files);
#else
  g_hash_table_destroy (parse_info->files);
#endif

  /* Free parse info */
  g_free (parse_info);
}



static void
xfce_menu_add_directory_dir (XfceMenu    *menu,
                             const gchar *dir)
{
  /* Absolute path of the directory (free'd by the menu instance later) */
  gchar *path;

  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (dir != NULL);

  if (!g_path_is_absolute (dir))
    {
      /* Determine the absolute path (directory) of the menu file */
      gchar *dirname = g_path_get_dirname (menu->priv->filename);

      /* Construct absolute path */
      path = g_build_path (G_DIR_SEPARATOR_S, dirname, dir, NULL);

      /* Free absolute menu file directory path */
      g_free (dirname);
    }
  else
    path = g_strdup (dir);

  /* Append path */
  menu->priv->directory_dirs = g_slist_append (menu->priv->directory_dirs, path);
}



static void
xfce_menu_add_default_directory_dirs (XfceMenu *menu)
{
  int          i;
  gchar       *path;
  gchar       *kde_data_dir;
  const gchar *kde_dir;

  const gchar * const *dirs;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Append $KDEDIR/share/desktop-directories as a workaround for distributions 
   * not installing KDE menu files properly into $XDG_DATA_DIR */

  /* Get KDEDIR environment variable */
  kde_dir = g_getenv ("KDEDIR");

  /* Check if this variable is set */
  if (G_UNLIKELY (kde_dir != NULL))
    {
      /* Build KDE data dir */
      kde_data_dir = g_build_filename (kde_dir, "share", "desktop-directories", NULL);

      /* Add it as a directory dir if it exists */
      if (G_LIKELY (g_file_test (kde_data_dir, G_FILE_TEST_IS_DIR)))
        xfce_menu_add_directory_dir (menu, kde_data_dir);

      /* Free the KDE data dir */
      g_free (kde_data_dir);
    }

  /* The $KDEDIR workaround ends here */

  /* Append system-wide data dirs */
  dirs = g_get_system_data_dirs ();
  for (i = 0; dirs[i] != NULL; i++)
    {
      path = g_build_path (G_DIR_SEPARATOR_S, dirs[i], "desktop-directories", NULL);
      xfce_menu_add_directory_dir (menu, path);
      g_free (path);
    }

  /* Append user data dir */
  path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), "desktop-directories", NULL);
  xfce_menu_add_directory_dir (menu, path);
  g_free (path);
}



static void
xfce_menu_add_legacy_dir (XfceMenu    *menu,
                          const gchar *dir)
{
  /* Absolute path of the directory (free'd by the menu instance later) */
  gchar *path;

  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (menu->priv->filename != NULL);
  g_return_if_fail (dir != NULL);

  if (!g_path_is_absolute (dir))
    {
      /* Determine the absolute path (directory) of the menu file */
      gchar *dirname = g_path_get_dirname (menu->priv->filename);

      /* Construct absolute path */
      path = g_build_path (G_DIR_SEPARATOR_S, dirname, dir, NULL);

      /* Free absolute menu file directory path */
      g_free (dirname);
    }
  else
    path = g_strdup (dir);

  /* Check if there already are legacy dirs */
  if (G_LIKELY (menu->priv->legacy_dirs != NULL))
    {
      /* Remove all previous occurences of the directory from the list */
      /* TODO: This probably is rather dirty and should be replaced with a more
       * clean algorithm. */
      GSList *iter = menu->priv->legacy_dirs;
      while (iter != NULL) 
        {
          gchar *data = (gchar *)iter->data;
          if (g_utf8_collate (data, dir) == 0)
            {
              GSList *tmp = g_slist_next (iter);
              menu->priv->app_dirs = g_slist_remove_link (menu->priv->legacy_dirs, iter);
              iter = tmp;
            }
          else
            iter = iter->next;
        }
      
      /* Append directory */
      menu->priv->legacy_dirs = g_slist_append (menu->priv->legacy_dirs, path);
    }
  else
    {
      /* Create new GSList and append the absolute path of the directory */
      menu->priv->legacy_dirs = g_slist_append (menu->priv->legacy_dirs, path);
    }
}



static void
xfce_menu_add_kde_legacy_dirs (XfceMenu *menu)
{
  static gchar **kde_legacy_dirs = NULL;

  g_return_if_fail (XFCE_IS_MENU (menu));

  if (G_UNLIKELY (kde_legacy_dirs == NULL))
    {
      gchar       *std_out;
      gchar       *std_err;

      gint         status;
      GError      *error = NULL;
      const gchar *kde_dir = g_getenv ("KDEDIR");
      const gchar *path = g_getenv ("PATH");
      gchar       *kde_path;

      /* Determine value of KDEDIR */
      if (G_UNLIKELY (kde_dir != NULL))
        {
          /* Build KDEDIR/bin path */
          gchar *kde_bin_dir = g_build_path (G_DIR_SEPARATOR_S, kde_dir, "bin", NULL);
          
          /* Expand PATH to include KDEDIR/bin - if necessary */
          const gchar *occurence = g_strrstr (path, kde_bin_dir);
          if (G_LIKELY (occurence == NULL))
            {
              /* PATH = $PATH:$KDEDIR/bin */
              kde_path = g_strjoin (G_SEARCHPATH_SEPARATOR_S, path, kde_bin_dir, NULL);

              /* Set new $PATH value */
              g_setenv ("PATH", kde_path, TRUE);

              /* Free expanded PATH value */
              g_free (kde_path);
            }
              
          /* Free KDEDIR/bin */
          g_free (kde_bin_dir);
        }

      /* Parse output of kde-config */
      if (g_spawn_command_line_sync ("kde-config --path apps", &std_out, &std_err, &status, &error))
        kde_legacy_dirs = g_strsplit (g_strchomp (std_out), G_SEARCHPATH_SEPARATOR_S, 0);
      else
        g_error_free (error);

      /* Free output buffers */
      g_free (std_err);
      g_free (std_out);
    }

  if (kde_legacy_dirs != NULL) /* This is neither likely nor unlikely */
    {
      int i;

      /* Add all KDE legacy dirs to the list */
      for (i = 0; i < g_strv_length (kde_legacy_dirs); i++) 
        xfce_menu_add_legacy_dir (menu, kde_legacy_dirs[i]);
    }
}



static void
xfce_menu_add_app_dir (XfceMenu    *menu,
                       const gchar *dir)
{
  /* Absolute path of the directory (free'd by the menu instance later) */
  gchar *path;

  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (menu->priv->filename != NULL);
  g_return_if_fail (dir != NULL);

  if (!g_path_is_absolute (dir))
    {
      /* Determine the absolute path (directory) of the menu file */
      gchar *dirname = g_path_get_dirname (menu->priv->filename);

      /* Construct absolute path */
      path = g_build_path (G_DIR_SEPARATOR_S, dirname, dir, NULL);

      /* Free absolute menu file directory path */
      g_free (dirname);
    }
  else
    path = g_strdup (dir);

  /* Append path */
  menu->priv->app_dirs = g_slist_append (menu->priv->app_dirs, path);
}



static void 
xfce_menu_add_default_app_dirs (XfceMenu *menu)
{
  int    i;
  gchar *path;
  gchar *kde_data_dir;
  const gchar *kde_dir;

  const gchar * const *dirs;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Append $KDEDIR/share/applications as a workaround for distributions 
   * not installing KDE menu files properly into $XDG_DATA_DIR */

  /* Get KDEDIR environment variable */
  kde_dir = g_getenv ("KDEDIR");

  /* Check if this variable is set */
  if (G_UNLIKELY (kde_dir != NULL))
    {
      /* Build KDE data dir */
      kde_data_dir = g_build_filename (kde_dir, "share", "applications", NULL);

      /* Add it as an app dir if it exists */
      if (G_LIKELY (g_file_test (kde_data_dir, G_FILE_TEST_IS_DIR)))
        xfce_menu_add_app_dir (menu, kde_data_dir);

      /* Free the KDE data dir */
      g_free (kde_data_dir);
    }

  /* The $KDEDIR workaround ends here */

  /* Append system-wide data dirs */
  dirs = g_get_system_data_dirs ();
  for (i = 0; dirs[i] != NULL; i++)
    {
      path = g_build_path (G_DIR_SEPARATOR_S, dirs[i], "applications", NULL);
      xfce_menu_add_app_dir (menu, path);
      g_free (path);
    }

  /* Append user data dir */
  path = g_build_path (G_DIR_SEPARATOR_S, g_get_user_data_dir (), "applications", NULL);
  xfce_menu_add_app_dir (menu, path);
  g_free (path);
}



GSList*
xfce_menu_get_menus (XfceMenu *menu)
{
  GSList *menus;

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
#if GLIB_CHECK_VERSION (2,10,0)
  g_object_ref_sink (G_OBJECT (submenu));
#else
  g_object_ref (G_OBJECT (submenu));
#endif

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



static void
xfce_menu_resolve_legacy_menus (XfceMenu *menu)
{
  GSList      *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Iterate over list of legacy directories */
  for (iter = menu->priv->legacy_dirs; iter != NULL; iter = g_slist_next (iter))
    {
      /* Check if the directory exists */
      if (g_file_test (iter->data, G_FILE_TEST_IS_DIR))
        {
          /* Resolve legacy menu hierarchy found in this directory */
          xfce_menu_resolve_legacy_menu (menu, iter->data);
        }
    }

  /* Resolve legacy menus of all child menus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      xfce_menu_resolve_legacy_menus (XFCE_MENU (iter->data));
    }
}



static void
xfce_menu_resolve_legacy_menu (XfceMenu    *menu,
                               const gchar *path)
{
  XfceMenu          *legacy_menu;
  XfceMenuDirectory *directory = NULL;
  GDir              *dir;
  const gchar       *filename;
  gchar             *absolute_filename;

  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (path != NULL && g_file_test (path, G_FILE_TEST_IS_DIR));

  /* Open directory for reading */
  dir = g_dir_open (path, 0, NULL);

  /* Abort if directory could not be opened */
  if (G_UNLIKELY (dir == NULL))
    return;

  /* Create the legacy menu */
  legacy_menu = g_object_new (XFCE_TYPE_MENU, "filename", menu->priv->filename, NULL);

  /* Set legacy menu name to the directory path */
  xfce_menu_set_name (legacy_menu, path);

  /* Iterate over directory entries */
  while ((filename = g_dir_read_name (dir)) != NULL)
    {
      /* Build absolute filename for this entry */
      absolute_filename = g_build_filename (path, filename, NULL);

      if (g_file_test (absolute_filename, G_FILE_TEST_IS_DIR))
        {
          /* We have a subdir -> create another legacy menu for this subdirectory */
          xfce_menu_resolve_legacy_menu (legacy_menu, absolute_filename);
        }
      else if (g_utf8_collate (".directory", filename) == 0) 
        {
          /* We have a .directory file -> create the directory object for the legacy menu */
          directory = g_object_new (XFCE_TYPE_MENU_DIRECTORY, "filename", absolute_filename, NULL);
        }
    }

  /* Check if there was a .directory file in the directory. Otherwise, don't add
   * this legacy menu to its parent (-> it is ignored). */
  if (G_LIKELY (directory != NULL))
    {
      /* Set the legacy menu directory */
      xfce_menu_set_directory (legacy_menu, directory);

      /* Add legacy menu to its new parent */
      xfce_menu_add_menu (menu, legacy_menu);
    }
  else
    {
      /* Destroy the legacy menu again - no .directory file found */
      g_object_unref (legacy_menu);
    }

  /* Close directory handle */
  g_dir_close (dir);
}



static void
xfce_menu_remove_duplicates (XfceMenu *menu)
{
  GSList *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  xfce_menu_consolidate_child_menus (menu);
  xfce_menu_consolidate_app_dirs (menu);
  xfce_menu_consolidate_directory_dirs (menu);
  xfce_menu_parse_info_consolidate_directory_names (menu->priv->parse_info);

  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      XfceMenu *submenu = XFCE_MENU (iter->data);
      xfce_menu_remove_duplicates (submenu);
    }
}



static void
xfce_menu_consolidate_child_menus (XfceMenu *menu)
{
#if 0
  GSList      *iter;
  GSList      *merged_submenus = NULL;
  GHashTable  *groups;
  const gchar *name;
  XfceMenu    *submenu;
  XfceMenu    *merged_menu;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Setup the hash table */
  groups = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  /* Iterate over all submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      submenu = XFCE_MENU (iter->data);

      /* Get menu this submenu should be appended to */
      merged_menu = XFCE_MENU (g_hash_table_lookup (groups, xfce_menu_get_name (submenu)));

      if (G_LIKELY (merged_menu == NULL))
        {
          /* Create empty menu */
          merged_menu = g_object_new (XFCE_TYPE_MENU, NULL);

          /* Add new menu to the hash table */
          g_hash_table_insert (groups, (gpointer) xfce_menu_get_name (submenu), merged_menu);
        }

      /* Copy menu information */
      /* FIXME This introduces possible bugs. E.g. if merged_menu has one <Deleted> 
       * element and submenu has none, the lines below would set it to <NotDeleted> 
       * (which is the default value). This does not follow the spec! Same goes
       * for <OnlyUnallocated>. */
      xfce_menu_set_name (merged_menu, xfce_menu_get_name (submenu));
      xfce_menu_set_only_unallocated (merged_menu, xfce_menu_get_only_unallocated (submenu));
      xfce_menu_set_deleted (merged_menu, xfce_menu_get_deleted (submenu));
      xfce_menu_set_filename (merged_menu, xfce_menu_get_filename (submenu));

      /* Set parent menu */
      merged_menu->priv->parent = menu;

      /* Append directory names, directory and app dirs as well as rules to the merged menu */      
      g_slist_foreach (submenu->priv->parse_info->directory_names, (GFunc) xfce_menu_merge_directory_name, merged_menu);
      g_slist_foreach (submenu->priv->directory_dirs, (GFunc) xfce_menu_merge_directory_dir, merged_menu);
      g_slist_foreach (submenu->priv->app_dirs, (GFunc) xfce_menu_merge_app_dir, merged_menu);
      g_slist_foreach (submenu->priv->rules, (GFunc) xfce_menu_merge_rule, merged_menu);

      /* TODO Merge submenus of submenu and merged_menu! */

      /* Add merged menu to the new list of submenus if not included already */
      if (g_slist_find (merged_submenus, merged_menu) == NULL)
        merged_submenus = g_slist_append (merged_submenus, merged_menu);
    }

  /* Free old submenu list (and the submenus) */
  g_slist_foreach (menu->priv->submenus, (GFunc) g_object_unref, NULL);
  g_slist_free (menu->priv->submenus);

  /* Use list of merged submenus as new submenu list */
  menu->priv->submenus = merged_submenus;

  /* Free hash table */
#if GLIB_CHECK_VERSION(2,10,0)  
  g_hash_table_unref (groups);
#else
  g_hash_table_destroy (groups);
#endif

#endif
}



#if 0
static void
xfce_menu_merge_directory_name (const gchar *name,
                                XfceMenu    *menu)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  menu->priv->parse_info->directory_names = g_slist_append (menu->priv->parse_info->directory_names, (gpointer) name);
}



static void
xfce_menu_merge_directory_dir (const gchar *dir, 
                               XfceMenu    *menu)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  xfce_menu_add_directory_dir (menu, dir);
}



static void
xfce_menu_merge_app_dir (const gchar *dir,
                         XfceMenu    *menu)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  xfce_menu_add_app_dir (menu, dir);
}



static void
xfce_menu_merge_rule (XfceMenuRules *rules,
                      XfceMenu      *menu)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (XFCE_IS_MENU_RULES (rules));
  xfce_menu_add_rule (menu, rules);
}
#endif



static void
xfce_menu_consolidate_directory_dirs (XfceMenu *menu)
{
  GSList *iter;
  GSList *dirs = NULL;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Iterate over directory dirs in reverse order */
  for (iter = g_slist_reverse (menu->priv->directory_dirs); iter != NULL; iter = g_slist_next (iter))
    {
      /* Prepend directory dir to the new list unless it already exists */
      if (G_LIKELY (g_slist_find_custom (dirs, iter->data, (GCompareFunc) g_utf8_collate) == NULL))
        dirs = g_slist_prepend (dirs, g_strdup (iter->data));
    }

  /* Free old list */
  g_slist_foreach (menu->priv->directory_dirs, (GFunc) g_free, NULL);
  g_slist_free (menu->priv->directory_dirs);

  menu->priv->directory_dirs = dirs;
}



static void
xfce_menu_consolidate_app_dirs (XfceMenu *menu)
{
  GSList *iter;
  GSList *dirs = NULL;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Iterate over app dirs in reverse order */
  for (iter = menu->priv->app_dirs; iter != NULL; iter = g_slist_next (iter))
    {
      /* Append app dir to the new list unless it already exists */
      if (G_LIKELY (g_slist_find_custom (dirs, iter->data, (GCompareFunc) g_utf8_collate) == NULL))
        dirs = g_slist_append (dirs, g_strdup (iter->data));
    }

  /* Free old list */
  g_slist_foreach (menu->priv->app_dirs, (GFunc) g_free, NULL);
  g_slist_free (menu->priv->app_dirs);

  menu->priv->app_dirs = dirs;
}



static void
xfce_menu_resolve_directory (XfceMenu *menu)
{
  GSList            *directory_names;
  GSList            *iter;
  XfceMenuDirectory *directory = NULL;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Get reverse copy of all directory names */
  directory_names = g_slist_reverse (g_slist_copy (menu->priv->parse_info->directory_names));

  /* Try to load one directory name after another */
  for (iter = directory_names; iter != NULL; iter = g_slist_next (iter))
    {
      /* Try to load the directory with this name */
      directory = xfce_menu_lookup_directory (menu, iter->data);

      /* Abort search if the directory was loaded successfully */
      if (G_LIKELY (directory != NULL))
        break;
    }

  if (G_LIKELY (directory != NULL)) 
    {
      /* Set the directory (assuming that we found at least one valid name) */
      menu->priv->directory = directory;
    }

  /* Free reverse list copy */
  g_slist_free (directory_names);

  /* ... and all submenus (recursively) */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_resolve_directory (iter->data);
}



static XfceMenuDirectory*
xfce_menu_lookup_directory (XfceMenu    *menu,
                            const gchar *filename)
{
  XfceMenuDirectory *directory = NULL;
  XfceMenu          *current;
  GSList            *dirs;
  gchar             *dirname;
  gchar             *absolute_path;
  GSList            *iter;
  gboolean           found = FALSE;
  
  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  g_return_val_if_fail (filename != NULL, NULL);

  /* Iterate through all (including parent) menus from the bottom up */
  for (current = menu; current != NULL; current = xfce_menu_get_parent (current))
    {
      /* Allocate a reverse copy of the menu's directory dirs */
      dirs = g_slist_reverse (xfce_menu_get_directory_dirs (current));

      /* Iterate through all directories */
      for (iter = dirs; iter != NULL; iter = g_slist_next (iter))
        {
          /* Check if the path is absolute */
          if (G_UNLIKELY (!g_path_is_absolute (iter->data)))
            {
              /* Determine directory of the menu file */
              dirname = g_path_get_dirname (xfce_menu_get_filename (menu));
              
              /* Build absolute path */
              absolute_path = g_build_filename (dirname, iter->data, filename, NULL);

              /* Free directory name */
              g_free (dirname); 
            }
          else
            absolute_path = g_build_filename (iter->data, filename, NULL);

          /* Check if the file exists and is readable */
          if (G_UNLIKELY (g_file_test (absolute_path, G_FILE_TEST_EXISTS)))
            {
              if (G_LIKELY (g_access (absolute_path, R_OK) == 0))
                {
                  /* Load menu directory */
                  directory = g_object_new (XFCE_TYPE_MENU_DIRECTORY, "filename", absolute_path, NULL);

                  /* Update search status */
                  found = TRUE;
                }
            }
          
          /* Free the absolute path */
         g_free (absolute_path);

          /* Cancel search if we found the menu directory file */
          if (G_UNLIKELY (found))
            break;
        }

      /* Free reverse copy */
      g_slist_free (dirs);
    }

  return directory;
}



static void
xfce_menu_collect_files (XfceMenu *menu)
{
  GSList *app_dirs;
  GSList *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Fetch all application directories */
  app_dirs = g_slist_reverse (xfce_menu_get_app_dirs (menu));

  /* Collect desktop entry filenames */
  for (iter = app_dirs; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_collect_files_from_path (menu, iter->data, NULL);

  /* Free directory list */
  g_slist_free (app_dirs);

  /* Collect filenames for submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    xfce_menu_collect_files (XFCE_MENU (iter->data));
}



static void
xfce_menu_collect_files_from_path (XfceMenu    *menu,
                                   const gchar *path,
                                   const gchar *id_prefix)
{
  GDir        *dir;
  const gchar *filename;
  gchar       *absolute_path;
  gchar       *new_id_prefix;
  gchar       *desktop_id;

  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (path != NULL && g_path_is_absolute (path));

  /* Skip directory if it doesn't exist */
  if (G_UNLIKELY (!g_file_test (path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
    return;

  /* Open directory for reading */
  dir = g_dir_open (path, 0, NULL);

  /* Abort if directory cannot be opened */
  if (G_UNLIKELY (dir == NULL))
    return;

  /* Read file by file */
  while ((filename = g_dir_read_name (dir)) != NULL)
    {
      /* Build absolute path */
      absolute_path = g_build_filename (path, filename, NULL);

      /* Treat files and directories differently */
      if (g_file_test (absolute_path, G_FILE_TEST_IS_DIR))
        {
          /* Create new desktop-file id prefix */
          if (G_LIKELY (id_prefix == NULL))
            new_id_prefix = g_strdup (filename);
          else
            new_id_prefix = g_strjoin ("-", id_prefix, filename, NULL);

          /* Collect files in the directory */
          xfce_menu_collect_files_from_path (menu, absolute_path, new_id_prefix);

          /* Free id prefix */
          g_free (new_id_prefix);
        }
      else
        {
          /* Skip all filenames which do not end with .desktop */
          if (G_LIKELY (g_str_has_suffix (filename, ".desktop")))
            {
              /* Create desktop-file id */
              if (G_LIKELY (id_prefix == NULL))
                desktop_id = g_strdup (filename);
              else
                desktop_id = g_strjoin ("-", id_prefix, filename, NULL);

              /* Insert into the files hash table if the desktop-file id does not exist in there yet */
              if (G_LIKELY (g_hash_table_lookup (menu->priv->parse_info->files, desktop_id) == NULL))
                g_hash_table_insert (menu->priv->parse_info->files, g_strdup (desktop_id), g_strdup (absolute_path));

              /* Free desktop-file id */
              g_free (desktop_id);
            }
        }

      /* Free absolute path */
      g_free (absolute_path);
    }

  /* Close directory handle */
  g_dir_close (dir);
}



static void
xfce_menu_resolve_items (XfceMenu *menu,
                         gboolean  only_unallocated)
{
  XfceMenuStandardRules *rule;
  GSList                *iter;

  g_return_if_fail (menu != NULL && XFCE_IS_MENU (menu));

  /* Resolve items in this menu (if it matches the only_unallocated argument.
   * This means that in the first pass, all items of menus without 
   * <OnlyUnallocated /> are resolved and in the second pass, only items of 
   * menus with <OnlyUnallocated /> are resolved */
  if (menu->priv->only_unallocated == only_unallocated)
    {
      /* Iterate over all rules */
      for (iter = menu->priv->rules; iter != NULL; iter = g_slist_next (iter))
        {
          rule = XFCE_MENU_STANDARD_RULES (iter->data);

          if (G_LIKELY (xfce_menu_standard_rules_get_include (rule)))
            {
              /* Resolve available items and match them against this rule */
              xfce_menu_resolve_items_by_rule (menu, rule);
            }
          else
            {
              /* Remove all items matching this exclude rule from the item pool */
              xfce_menu_item_pool_apply_exclude_rule (menu->priv->pool, rule);
            }
        }
    }

  /* Iterate over all submenus */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      /* Resolve items of the submenu */
      xfce_menu_resolve_items (XFCE_MENU (iter->data), only_unallocated);
    }
}



static void
xfce_menu_resolve_items_by_rule (XfceMenu              *menu,
                                 XfceMenuStandardRules *rule)
{
  XfceMenuPair pair;

  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (XFCE_IS_MENU_STANDARD_RULES (rule));

  /* Store menu and rule pointer in the pair */
  pair.first = menu;
  pair.second = rule;

  /* Try to insert each of the collected desktop entry filenames into the menu */
  g_hash_table_foreach (menu->priv->parse_info->files, (GHFunc) xfce_menu_resolve_item_by_rule, &pair);
}



static void
xfce_menu_resolve_item_by_rule (const gchar  *desktop_id,
                                const gchar  *filename,
                                XfceMenuPair *data)
{
  XfceMenu              *menu;
  XfceMenuStandardRules *rule;
  XfceMenuItem          *item;

  g_return_if_fail (XFCE_IS_MENU (data->first));
  g_return_if_fail (XFCE_IS_MENU_STANDARD_RULES (data->second));

  /* Restore menu and rule from the data pair */
  menu = XFCE_MENU (data->first);
  rule = XFCE_MENU_STANDARD_RULES (data->second);

  /* Try to load the menu item from the cache */
  item = xfce_menu_item_cache_lookup (menu->priv->cache, filename, desktop_id);

  if (G_LIKELY (item != NULL))
    {
      /* Only include item if menu not only includes unallocated items
       * or if the item is not allocated yet */
      if (!menu->priv->only_unallocated || (xfce_menu_item_get_allocated (item) < 1))
        {
          /* Add item to the pool if it matches the include rule */
          if (G_LIKELY (xfce_menu_standard_rules_get_include (rule) && xfce_menu_rules_match (XFCE_MENU_RULES (rule), item)))
            xfce_menu_item_pool_insert (menu->priv->pool, item);
        }
    }
}



static void
xfce_menu_resolve_deleted (XfceMenu *menu)
{
  GSList  *iter;
  gboolean deleted;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Note: There's a limitation: if the root menu has a <Deleted /> we
   * can't just free the pointer here. Therefor we only check child menus. */

  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      XfceMenu *submenu = iter->data;

      /* Determine whether this submenu was deleted */
      deleted = submenu->priv->deleted;
      if (G_LIKELY (submenu->priv->directory != NULL))
        deleted = deleted || xfce_menu_directory_get_hidden (submenu->priv->directory);

      /* Remove submenu if it is deleted, otherwise check submenus of the submenu */
      if (G_UNLIKELY (deleted))
        {
          /* Remove submenu from the list ... */
          menu->priv->submenus = g_slist_remove_link (menu->priv->submenus, iter);

          /* ... and destroy it */
          g_object_unref (G_OBJECT (submenu));
        }
      else
        xfce_menu_resolve_deleted (submenu);
    }
}



static void
xfce_menu_resolve_moves (XfceMenu *menu)
{
  XfceMenuMove *move;
  XfceMenu     *source;
  XfceMenu     *destination;
  GSList       *iter;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Recurse into the submenus which need to perform move actions first */
  for (iter = menu->priv->submenus; iter != NULL; iter = g_slist_next (iter))
    {
      source = XFCE_MENU (iter->data);

      /* Resolve moves of the child menu */
      xfce_menu_resolve_moves (source);
    }

  /* Iterate over move instructions */
  for (iter = menu->priv->moves; iter != NULL; iter = g_slist_next (iter))
    {
      move = XFCE_MENU_MOVE (iter->data);
      
      /* Determine submenu with the old name */
      source = xfce_menu_get_menu_with_name (menu, xfce_menu_move_get_old (move));

      /* Skip if there is no such submenu */
      if (G_LIKELY (source == NULL))
        continue;

      /* Determine destination submenu */
      destination = xfce_menu_get_menu_with_name (menu, xfce_menu_move_get_new (move));

      /* If destination does not exist, simply reset the name of the source menu */
      if (G_LIKELY (destination == NULL))
        xfce_menu_set_name (source, xfce_menu_move_get_new (move));
      else
        {
          /* TODO: See section "Merging" in the menu specification. */
        }
    }

#if 0
  XfceMenu     *submenu;
  XfceMenu     *target_submenu;
  XfceMenuMove *move;
  GSList       *iter;
  GSList       *submenu_iter;
  GSList       *removed_menus;

  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Recurse into the submenus which need to perform move actions first */
  for (submenu_iter = menu->priv->submenus; submenu_iter != NULL; submenu_iter = g_slist_next (submenu_iter))
    {
      submenu = XFCE_MENU (submenu_iter->data);

      /* Resolve moves of the child menu */
      xfce_menu_resolve_moves (submenu);
    }

  /* Iterate over the move instructions */
  for (iter = menu->priv->moves; iter != NULL; iter = g_slist_next (iter))
    {
      move = XFCE_MENU_MOVE (iter->data);

      /* Fetch submenu with the old name */
      submenu = xfce_menu_get_menu_with_name (menu, xfce_menu_move_get_old (move));

      /* Only go into details if there actually is a submenu with this name */
      if (submenu != NULL)
        {
          /* Fetch the target submenu */
          target_submenu = xfce_menu_get_menu_with_name (menu, xfce_menu_move_get_new (move));

          /* If there is no target, just rename the submenu */
          if (target_submenu == NULL)
            xfce_menu_set_name (submenu, xfce_menu_move_get_new (move));
          else
            {
              /* TODO Set <Deleted>, <OnlyUnallocated>, etc. See FIXME in this file for what kind 
               * of bugs this may introduce. */

              /* Append directory names, directory and app dirs as well as rules to the merged menu */      
              g_slist_foreach (submenu->priv->parse_info->directory_names, (GFunc) xfce_menu_merge_directory_name, target_submenu);
              g_slist_foreach (submenu->priv->directory_dirs, (GFunc) xfce_menu_merge_directory_dir, target_submenu);
              g_slist_foreach (submenu->priv->app_dirs, (GFunc) xfce_menu_merge_app_dir, target_submenu);
              g_slist_foreach (submenu->priv->rules, (GFunc) xfce_menu_merge_rule, target_submenu);
              
              /* Remove submenu from the submenu list */
              menu->priv->submenus = g_slist_remove (menu->priv->submenus, submenu);

              /* TODO Merge submenus of submenu and target_submenu. Perhaps even
               * find a better way for merging menus as the current way is a)
               * duplicated (see consolidate_child_menus) and b) buggy */

              /* TODO Free the submenu - this introduces a strange item pool
               * error ... */
              /* g_object_unref (submenu); */
            }
        }
    }
#endif
}



static void
xfce_menu_add_rule (XfceMenu      *menu,
                    XfceMenuRules *rules)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (XFCE_IS_MENU_RULES (rules));

  menu->priv->rules = g_slist_append (menu->priv->rules, rules);
}



static void
xfce_menu_add_move (XfceMenu     *menu,
                    XfceMenuMove *move)
{
  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (XFCE_IS_MENU_MOVE (move));

  menu->priv->moves = g_slist_append (menu->priv->moves, move);
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
  const gchar *name1;
  const gchar *name2;

  /* Determining display name of a */
  if (XFCE_IS_MENU (a))
    {
      if (G_LIKELY (xfce_menu_get_directory (XFCE_MENU (a)) != NULL))
        name1 = xfce_menu_directory_get_name (xfce_menu_get_directory (XFCE_MENU (a)));
      else
        name1 = xfce_menu_get_name (XFCE_MENU (a));
    }
  else
    name1 = xfce_menu_item_get_name (XFCE_MENU_ITEM (a));

  /* Determining display name of b */
  if (XFCE_IS_MENU (b))
    {
      if (G_LIKELY (xfce_menu_get_directory (XFCE_MENU (b)) != NULL))
        name2 = xfce_menu_directory_get_name (xfce_menu_get_directory (XFCE_MENU (b)));
      else
        name2 = xfce_menu_get_name (XFCE_MENU (b));
    }
  else
    name2 = xfce_menu_item_get_name (XFCE_MENU_ITEM (b));

  /* Compare display names and return the result */
  return g_utf8_collate (name1, name2);
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
    name = menu->priv->name;

  return name;
}



static const gchar*
xfce_menu_get_element_icon_name (XfceMenuElement *element)
{
  XfceMenu *menu;
  
  g_return_val_if_fail (XFCE_IS_MENU (element), NULL);

  menu = XFCE_MENU (element);

  return menu->priv->directory != NULL ? xfce_menu_directory_get_icon (menu->priv->directory) : NULL;
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

  /* Monitor items in the menu pool */
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
  g_return_if_fail (XFCE_IS_MENU (menu));

  /* Stop monitoring the items */
  xfce_menu_item_pool_foreach (menu->priv->pool, (GHFunc) item_monitor_stop, menu);
}

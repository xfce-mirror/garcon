/* $Id$ */
/* vi:set expandtab sw=2 sts=2: */
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

#include <locale.h>
#include <glib.h>

#include <libxfce4menu/xfce-menu-environment.h>
#include <libxfce4menu/xfce-menu-directory.h>



void
_xfce_menu_directory_init (void)
{
}



void
_xfce_menu_directory_shutdown (void)
{
}



#define XFCE_MENU_DIRECTORY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), XFCE_TYPE_MENU_DIRECTORY, XfceMenuDirectoryPrivate))



/* Desktop entry keys */
#if 0
static const gchar *desktop_entry_keys[] = 
{
  "Name",
  "Comment",
  "Icon",
  "Categories",
  "OnlyShowIn",
  "NotShowIn",
  "NoDisplay",
  "Hidden",
  NULL
};
#endif



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILE,
  PROP_NAME,
  PROP_COMMENT,
  PROP_NO_DISPLAY,
  PROP_ICON,
};



static void       xfce_menu_directory_class_init       (XfceMenuDirectoryClass       *klass);
static void       xfce_menu_directory_init             (XfceMenuDirectory            *directory);
static void       xfce_menu_directory_constructed      (GObject                      *object);
static void       xfce_menu_directory_finalize         (GObject                      *object);
static void       xfce_menu_directory_get_property     (GObject                      *object,
                                                        guint                         prop_id,
                                                        GValue                       *value,
                                                        GParamSpec                   *pspec);
static void       xfce_menu_directory_set_property     (GObject                      *object,
                                                        guint                         prop_id,
                                                        const GValue                 *value,
                                                        GParamSpec                   *pspec);
static void       xfce_menu_directory_free_private     (XfceMenuDirectory            *directory);
static void       xfce_menu_directory_load             (XfceMenuDirectory            *directory);



struct _XfceMenuDirectoryPrivate
{
  /* Directory file */
  GFile             *file;

  /* Directory name */
  gchar             *name;

  /* Directory description (comment) */
  gchar             *comment;

  /* Icon */
  gchar             *icon;

  /* Environments in which the menu should be displayed only */
  gchar            **only_show_in;

  /* Environments in which the menu should be hidden */
  gchar            **not_show_in;

  /* Whether the menu should be ignored completely */
  guint              hidden : 1;

  /* Whether the menu should be hidden */
  guint              no_display : 1;
};

struct _XfceMenuDirectoryClass
{
  GObjectClass __parent__;
};

struct _XfceMenuDirectory
{
  GObject          __parent__;

  /* < private > */
  XfceMenuDirectoryPrivate *priv;
};



static GObjectClass *xfce_menu_directory_parent_class = NULL;



GType
xfce_menu_directory_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfceMenuDirectoryClass),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_directory_class_init,
        NULL,
        NULL,
        sizeof (XfceMenuDirectory),
        0,
        (GInstanceInitFunc) xfce_menu_directory_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMenuDirectory", &info, 0);
    }

  return type;
}



static void
xfce_menu_directory_class_init (XfceMenuDirectoryClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof(XfceMenuDirectoryPrivate));

  /* Determine the parent type class */
  xfce_menu_directory_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->constructed = xfce_menu_directory_constructed;
  gobject_class->finalize = xfce_menu_directory_finalize; 
  gobject_class->get_property = xfce_menu_directory_get_property;
  gobject_class->set_property = xfce_menu_directory_set_property;

  /**
   * XfceMenuDirectory:filename:
   *
   * The @GFile of an %XfceMenuDirectory. 
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file",
                                                        "File",
                                                        "File",
                                                        G_TYPE_FILE,
                                                        G_PARAM_READWRITE | 
                                                        G_PARAM_CONSTRUCT_ONLY));

  /**
   * XfceMenuDirectory:name:
   *
   * Name of the directory.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "Name",
                                                        "Directory name",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  /**
   * XfceMenuDirectory:comment:
   *
   * Directory description (comment).
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_COMMENT,
                                   g_param_spec_string ("comment",
                                                        "Description",
                                                        "Directory description",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  /**
   * XfceMenuDirectory:icon:
   *
   * Icon associated with this directory.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_ICON,
                                   g_param_spec_string ("icon",
                                                        "Icon",
                                                        "Directory icon",
                                                        NULL,
                                                        G_PARAM_READWRITE));

  /**
   * XfceMenuDirectory:no-display:
   *
   * Whether this menu item is hidden in menus.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_NO_DISPLAY,
                                   g_param_spec_boolean ("no-display",
                                                         "No Display",
                                                         "Visibility state of the related menu",
                                                         FALSE,
                                                         G_PARAM_READWRITE));

}



static void
xfce_menu_directory_init (XfceMenuDirectory *directory)
{
  directory->priv = XFCE_MENU_DIRECTORY_GET_PRIVATE (directory);
  directory->priv->file = NULL;
  directory->priv->name = NULL;
  directory->priv->icon = NULL;
  directory->priv->only_show_in = NULL;
  directory->priv->not_show_in = NULL;
  directory->priv->hidden = FALSE;
  directory->priv->no_display = FALSE;
}



static void 
xfce_menu_directory_constructed (GObject *object)
{
  XfceMenuDirectory *directory = XFCE_MENU_DIRECTORY (object);
  xfce_menu_directory_load (directory);
}



static void
xfce_menu_directory_finalize (GObject *object)
{
  XfceMenuDirectory *directory = XFCE_MENU_DIRECTORY (object);

  /* Free private data */
  xfce_menu_directory_free_private (directory);

  /* Free file */
  g_object_unref (directory->priv->file);

  (*G_OBJECT_CLASS (xfce_menu_directory_parent_class)->finalize) (object);
}



static void
xfce_menu_directory_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  XfceMenuDirectory *directory = XFCE_MENU_DIRECTORY (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, xfce_menu_directory_get_file (directory));
      break;

    case PROP_NAME:
      g_value_set_string (value, xfce_menu_directory_get_name (directory));
      break;

    case PROP_COMMENT:
      g_value_set_string (value, xfce_menu_directory_get_comment (directory));
      break;

    case PROP_ICON:
      g_value_set_string (value, xfce_menu_directory_get_icon (directory));
      break;

    case PROP_NO_DISPLAY:
      g_value_set_boolean (value, xfce_menu_directory_get_no_display (directory));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_menu_directory_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  XfceMenuDirectory *directory = XFCE_MENU_DIRECTORY (object);

  switch (prop_id)
    {
    case PROP_FILE:
      directory->priv->file = g_object_ref (g_value_get_object (value));
      break;

    case PROP_NAME:
      xfce_menu_directory_set_name (directory, g_value_get_string (value));
      break;

    case PROP_COMMENT:
      xfce_menu_directory_set_comment (directory, g_value_get_string (value));
      break;

    case PROP_ICON:
      xfce_menu_directory_set_icon (directory, g_value_get_string (value));
      break;

    case PROP_NO_DISPLAY:
      xfce_menu_directory_set_no_display (directory, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



GFile *
xfce_menu_directory_get_file (XfceMenuDirectory *directory)
{
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), NULL);
  return directory->priv->file;
}



const gchar*
xfce_menu_directory_get_name (XfceMenuDirectory *directory)
{
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), NULL);
  return directory->priv->name;
}



void
xfce_menu_directory_set_name (XfceMenuDirectory *directory, const gchar *name)
{
  g_return_if_fail (XFCE_IS_MENU_DIRECTORY (directory));
  g_return_if_fail (name != NULL);

  /* Free old name */
  if (G_UNLIKELY (directory->priv->name != NULL))
    g_free (directory->priv->name);

  /* Set the new filename */
  directory->priv->name = g_strdup (name);

  /* Notify listeners */
  g_object_notify (G_OBJECT (directory), "name");
}



const gchar*
xfce_menu_directory_get_comment (XfceMenuDirectory *directory)
{
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), NULL);
  return directory->priv->comment;
}



void
xfce_menu_directory_set_comment (XfceMenuDirectory *directory, const gchar *comment)
{
  g_return_if_fail (XFCE_IS_MENU_DIRECTORY (directory));

  /* Free old name */
  if (G_UNLIKELY (directory->priv->comment != NULL))
    g_free (directory->priv->comment);

  /* Set the new filename */
  directory->priv->comment = g_strdup (comment);

  /* Notify listeners */
  g_object_notify (G_OBJECT (directory), "comment");
}


const gchar*
xfce_menu_directory_get_icon (XfceMenuDirectory *directory)
{
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), NULL);
  return directory->priv->icon;
}



void
xfce_menu_directory_set_icon (XfceMenuDirectory *directory, const gchar *icon)
{
  g_return_if_fail (XFCE_IS_MENU_DIRECTORY (directory));

  /* Free old name */
  if (G_UNLIKELY (directory->priv->icon != NULL))
    g_free (directory->priv->icon);

  /* Set the new filename */
  directory->priv->icon = g_strdup (icon);

  /* Notify listeners */
  g_object_notify (G_OBJECT (directory), "icon");
}



gboolean
xfce_menu_directory_get_no_display (XfceMenuDirectory *directory)
{
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), FALSE);
  return directory->priv->no_display;
}



void        
xfce_menu_directory_set_no_display (XfceMenuDirectory *directory,
                                    gboolean           no_display)
{
  g_return_if_fail (XFCE_IS_MENU_DIRECTORY (directory));
  
  /* Abort if old and new value are equal */
  if (directory->priv->no_display == no_display)
    return;

  /* Assign new value */
  directory->priv->no_display = no_display;

  /* Notify listeners */
  g_object_notify (G_OBJECT (directory), "no-display");
}



static void
xfce_menu_directory_free_private (XfceMenuDirectory *directory)
{
  g_return_if_fail (XFCE_IS_MENU_DIRECTORY (directory));

  /* Free name */
  g_free (directory->priv->name);

  /* Free comment */
  g_free (directory->priv->comment);

  /* Free icon */
  g_free (directory->priv->icon);

  /* Free environment lists */
  g_strfreev (directory->priv->only_show_in);
  g_strfreev (directory->priv->not_show_in);
}



static void
xfce_menu_directory_load (XfceMenuDirectory *directory)
{
  GKeyFile    *entry;
  GError      *error = NULL;
  const gchar *name;
  const gchar *comment;
  const gchar *icon;
  gchar       *filename;

  g_return_if_fail (XFCE_IS_MENU_DIRECTORY (directory));

  /* TODO: Use get_uri() here, together with g_file_read() and
   * g_key_file_load_from_data() */

  filename = g_file_get_path (directory->priv->file);
  entry = g_key_file_new ();
  g_key_file_load_from_file (entry, filename, G_KEY_FILE_NONE, &error);
  g_free (filename);

  if (G_UNLIKELY (error != NULL))
    {
      g_error_free (error);
      return;
    }

  /* Read directory information */
  name = g_key_file_get_locale_string (entry, "Desktop Entry", "Name", NULL, NULL);
  comment = g_key_file_get_locale_string (entry, "Desktop Entry", "Comment", NULL, NULL);
  icon = g_key_file_get_locale_string (entry, "Desktop Entry", "Icon", NULL, NULL);

  /* Pass data to the directory */
  xfce_menu_directory_set_name (directory, name);
  xfce_menu_directory_set_comment (directory, comment);
  xfce_menu_directory_set_icon (directory, icon);
  xfce_menu_directory_set_no_display (directory, g_key_file_get_boolean (entry, "Desktop Entry", "NoDisplay", NULL));

  /* Set rest of the private data directly */
  directory->priv->only_show_in = g_key_file_get_string_list (entry, "Desktop Entry", "OnlyShowIn", NULL, NULL);
  directory->priv->not_show_in = g_key_file_get_string_list (entry, "Desktop Entry", "NotShowIn", NULL, NULL);
  directory->priv->hidden = g_key_file_get_boolean (entry, "Desktop Entry", "Hidden", NULL);

  g_key_file_free (entry);
}



gboolean
xfce_menu_directory_get_hidden (XfceMenuDirectory *directory)
{
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), FALSE);
  return directory->priv->hidden;
}



gboolean
xfce_menu_directory_show_in_environment (XfceMenuDirectory *directory)
{
  const gchar *env;
  gboolean     show = TRUE;
  gboolean     included;
  int          i;

  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), FALSE);
  
  /* Determine current environment */
  env = xfce_menu_get_environment ();

  /* If no environment has been set, the menu is displayed no matter what
   * OnlyShowIn or NotShowIn contain */
  if (G_UNLIKELY (env == NULL))
    return TRUE;

  /* Check if we have a OnlyShowIn OR a NotShowIn list (only one of them will be
   * there, according to the desktop entry specification) */
  if (G_UNLIKELY (directory->priv->only_show_in != NULL))
    {
      /* Determine whether our environment is included in this list */
      included = FALSE;
      for (i = 0; i < g_strv_length (directory->priv->only_show_in); ++i) 
        {
          if (G_UNLIKELY (g_utf8_collate (directory->priv->only_show_in[i], env) == 0))
            included = TRUE;
        }

      /* If it's not, don't show the menu */
      if (G_LIKELY (!included))
        show = FALSE;
    }
  else if (G_UNLIKELY (directory->priv->not_show_in != NULL))
    {
      /* Determine whether our environment is included in this list */
      included = FALSE;
      for (i = 0; i < g_strv_length (directory->priv->not_show_in); ++i)
        {
          if (G_UNLIKELY (g_utf8_collate (directory->priv->not_show_in[i], env) == 0))
            included = TRUE;
        }

      /* If it is, hide the menu */
      if (G_UNLIKELY (included))
        show = FALSE;
    }

  return show;
}



gboolean
xfce_menu_directory_get_visible (XfceMenuDirectory *directory)
{
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), FALSE);

  if (!xfce_menu_directory_show_in_environment (directory))
    return FALSE;
  else if (xfce_menu_directory_get_hidden (directory))
    return FALSE;
  else if (xfce_menu_directory_get_no_display (directory))
    return FALSE;

  return TRUE;
}



gboolean
xfce_menu_directory_equal (XfceMenuDirectory *directory,
                           XfceMenuDirectory *other)
{
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (directory), FALSE);
  g_return_val_if_fail (XFCE_IS_MENU_DIRECTORY (other), FALSE);
  return g_file_equal (directory->priv->file, other->priv->file);
}

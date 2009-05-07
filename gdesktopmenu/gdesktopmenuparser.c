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
#include <glib/gi18n.h>

#include <gio/gio.h>

#include <gdesktopmenu/gdesktopmenunode.h>
#include <gdesktopmenu/gdesktopmenutreeprovider.h>
#include <gdesktopmenu/gdesktopmenuparser.h>



/* Menu file parser states */
typedef enum
{
  G_DESKTOP_MENU_PARSER_STATE_START,
  G_DESKTOP_MENU_PARSER_STATE_ROOT,
  G_DESKTOP_MENU_PARSER_STATE_MENU,
  G_DESKTOP_MENU_PARSER_STATE_RULE,
  G_DESKTOP_MENU_PARSER_STATE_MOVE,
  G_DESKTOP_MENU_PARSER_STATE_LAYOUT,
  G_DESKTOP_MENU_PARSER_STATE_END,
} GDesktopMenuParserState;

/* Node types */
typedef enum
{
  G_DESKTOP_MENU_PARSER_NODE_TYPE_NONE,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_NAME,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_DIRECTORY,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_DIRECTORY_DIR,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_APP_DIR,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_FILENAME,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_CATEGORY,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_OLD,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_NEW,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_MENUNAME,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_MERGE_FILE,
  G_DESKTOP_MENU_PARSER_NODE_TYPE_MERGE_DIR,
} GDesktopMenuParserNodeType;

typedef struct _GDesktopMenuParserContext GDesktopMenuParserContext;

struct _GDesktopMenuParserContext
{
  GDesktopMenuParserNodeType node_type;
  GDesktopMenuParserState    state;
  GDesktopMenuParser        *parser;
  GNode                 *node;
};



#define G_DESKTOP_MENU_PARSER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), G_TYPE_DESKTOP_MENU_PARSER, GDesktopMenuParserPrivate))



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILE,
};



static void   g_desktop_menu_parser_class_init    (GDesktopMenuParserClass       *klass);
static void   g_desktop_menu_parser_provider_init (GDesktopMenuTreeProviderIface *iface);
static void   g_desktop_menu_parser_init          (GDesktopMenuParser            *parser);
static void   g_desktop_menu_parser_finalize      (GObject                       *object);
static void   g_desktop_menu_parser_get_property  (GObject                       *object,
                                                   guint                          prop_id,
                                                   GValue                        *value,
                                                   GParamSpec                    *pspec);
static void   g_desktop_menu_parser_set_property  (GObject                       *object,
                                                   guint                          prop_id,
                                                   const GValue                  *value,
                                                   GParamSpec                    *pspec);
static void   g_desktop_menu_parser_start_element (GMarkupParseContext           *context,
                                                   const gchar                   *element_name,
                                                   const gchar                  **attribute_names,
                                                   const gchar                  **attribute_values,
                                                   gpointer                       user_data,
                                                   GError                       **error);
static void   g_desktop_menu_parser_end_element   (GMarkupParseContext           *context,
                                                   const gchar                   *element_name,
                                                   gpointer                       user_data,
                                                   GError                       **error);
static void   g_desktop_menu_parser_characters    (GMarkupParseContext           *context,
                                                   const gchar                   *text,
                                                   gsize                          text_len,
                                                   gpointer                       user_data,
                                                   GError                       **error);
static GNode *g_desktop_menu_parser_get_tree      (GDesktopMenuTreeProvider      *provider);
static GFile *g_desktop_menu_parser_get_file      (GDesktopMenuTreeProvider      *provider);



struct _GDesktopMenuParserPrivate
{
  /* .menu file */
  GFile *file;

  /* Root menu node */
  GNode *menu;
};



static GObjectClass *g_desktop_menu_parser_parent_class = NULL;



GType
g_desktop_menu_parser_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GInterfaceInfo provider_info = 
      {
        (GInterfaceInitFunc) g_desktop_menu_parser_provider_init,
        NULL,
        NULL,
      };

      type = g_type_register_static_simple (G_TYPE_OBJECT,
                                            "GDesktopMenuParser",
                                            sizeof (GDesktopMenuParserClass),
                                            (GClassInitFunc) g_desktop_menu_parser_class_init,
                                            sizeof (GDesktopMenuParser),
                                            (GInstanceInitFunc) g_desktop_menu_parser_init,
                                            0);

      g_type_add_interface_static (type, G_TYPE_DESKTOP_MENU_TREE_PROVIDER, &provider_info);
    }

  return type;
}



static void
g_desktop_menu_parser_class_init (GDesktopMenuParserClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (GDesktopMenuParserPrivate));

  /* Determine the parent type class */
  g_desktop_menu_parser_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = g_desktop_menu_parser_finalize; 
  gobject_class->get_property = g_desktop_menu_parser_get_property;
  gobject_class->set_property = g_desktop_menu_parser_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file",
                                                        "file",
                                                        "file",
                                                        G_TYPE_FILE,
                                                        G_PARAM_READWRITE 
                                                        | G_PARAM_CONSTRUCT_ONLY));
}



static void 
g_desktop_menu_parser_provider_init (GDesktopMenuTreeProviderIface *iface)
{
  iface->get_tree = g_desktop_menu_parser_get_tree;
  iface->get_file = g_desktop_menu_parser_get_file;
}



static void
g_desktop_menu_parser_init (GDesktopMenuParser *parser)
{
  parser->priv = G_DESKTOP_MENU_PARSER_GET_PRIVATE (parser);
  parser->priv->file = NULL;
  parser->priv->menu = NULL;
}



static void
g_desktop_menu_parser_finalize (GObject *object)
{
  GDesktopMenuParser *parser = G_DESKTOP_MENU_PARSER (object);

  g_desktop_menu_node_tree_free (parser->priv->menu);

  g_object_unref (parser->priv->file);

  (*G_OBJECT_CLASS (g_desktop_menu_parser_parent_class)->finalize) (object);
}



static void
g_desktop_menu_parser_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  GDesktopMenuParser *parser = G_DESKTOP_MENU_PARSER (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, parser->priv->file);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
g_desktop_menu_parser_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  GDesktopMenuParser *parser = G_DESKTOP_MENU_PARSER (object);

  switch (prop_id)
    {
    case PROP_FILE:
      parser->priv->file = g_object_ref (g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



GDesktopMenuParser *
g_desktop_menu_parser_new (GFile *file)
{
  g_return_val_if_fail (G_IS_FILE (file), NULL);
  return g_object_new (G_TYPE_DESKTOP_MENU_PARSER, "file", file, NULL);
}



gboolean
g_desktop_menu_parser_run (GDesktopMenuParser *parser,
                           GCancellable       *cancellable,
                           GError            **error)
{
  GDesktopMenuParserContext parser_context;
  GMarkupParseContext      *context;
  GMarkupParser             markup_parser = {
    g_desktop_menu_parser_start_element,
    g_desktop_menu_parser_end_element,
    g_desktop_menu_parser_characters,
    NULL,
  };
  gboolean                  result = TRUE;
  gchar                    *data;
  gsize                     data_length;

  g_return_val_if_fail (G_IS_DESKTOP_MENU_PARSER (parser), FALSE);
  g_return_val_if_fail (G_IS_FILE (parser->priv->file), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Try to open and read the file */
  if (G_UNLIKELY (!g_file_load_contents (parser->priv->file, cancellable,
                                         &data, &data_length, NULL, error)))
    {
      gchar *uri = g_file_get_uri (parser->priv->file);
      
      if (error != NULL)
        {
          g_warning (_("Could not load menu file data from %s: %s"), 
                     uri, (*error)->message);
          g_error_free (*error);
        }
      else
        g_warning (_("Could not load menu file data from %s"), uri);

      g_free (uri);
      return FALSE;
    }

  /* Create parser context */
  parser_context.parser = parser;
  parser_context.node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_NONE;
  parser_context.state = G_DESKTOP_MENU_PARSER_STATE_START;
  parser_context.node = NULL;

  /* Create markup parse context */
  context = g_markup_parse_context_new (&markup_parser, 0, &parser_context, NULL); 

  /* Try to parse the menu file */
  if (!g_markup_parse_context_parse (context, data, data_length, error) ||
      !g_markup_parse_context_end_parse (context, error))
    {
      result = FALSE;
    }

  g_markup_parse_context_free (context);
  g_free (data);

  return result;
}



static GNode *
g_desktop_menu_parser_get_tree (GDesktopMenuTreeProvider *provider)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_PARSER (provider), NULL);
  return g_desktop_menu_node_tree_copy (G_DESKTOP_MENU_PARSER (provider)->priv->menu);
}



static GFile *
g_desktop_menu_parser_get_file (GDesktopMenuTreeProvider *provider)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_PARSER (provider), NULL);
  return g_object_ref (G_DESKTOP_MENU_PARSER (provider)->priv->file);
}



static void
g_desktop_menu_parser_start_element (GMarkupParseContext *context,
                                     const gchar         *element_name,
                                     const gchar        **attribute_names,
                                     const gchar        **attribute_values,
                                     gpointer             user_data,
                                     GError             **error)
{
  GDesktopMenuParserContext *parser_context = (GDesktopMenuParserContext *)user_data;
  GDesktopMenuNode          *node_;

  switch (parser_context->state)
    {
    case G_DESKTOP_MENU_PARSER_STATE_START:
      if (g_str_equal (element_name, "Menu"))
        {
          parser_context->parser->priv->menu = g_node_new (NULL);

          parser_context->state = G_DESKTOP_MENU_PARSER_STATE_ROOT;
          parser_context->node = parser_context->parser->priv->menu;
        }
      break;

    case G_DESKTOP_MENU_PARSER_STATE_ROOT:
    case G_DESKTOP_MENU_PARSER_STATE_MENU:
      if (g_str_equal (element_name, "Name"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_NAME;

      else if (g_str_equal (element_name, "Directory"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_DIRECTORY;
      else if (g_str_equal (element_name, "DirectoryDir"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_DIRECTORY_DIR;
      else if (g_str_equal (element_name, "DefaultDirectoryDirs"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS, NULL);
          g_node_append_data (parser_context->node, node_);
        }

      else if (g_str_equal (element_name, "AppDir"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_APP_DIR;
      else if (g_str_equal (element_name, "DefaultAppDirs"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_DEFAULT_APP_DIRS, NULL);
          g_node_append_data (parser_context->node, node_);
        }

      else if (g_str_equal (element_name, "Deleted"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_DELETED, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "NotDeleted"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_NOT_DELETED, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "OnlyUnallocated"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_ONLY_UNALLOCATED, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "NotOnlyUnallocated"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED, NULL);
          g_node_append_data (parser_context->node, node_);
        }

      else if (g_str_equal (element_name, "Include"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_INCLUDE, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = G_DESKTOP_MENU_PARSER_STATE_RULE;
        }
      else if (g_str_equal (element_name, "Exclude"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_EXCLUDE, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = G_DESKTOP_MENU_PARSER_STATE_RULE;
        }

      else if (g_str_equal (element_name, "Menu"))
        {
          parser_context->node = g_node_append_data (parser_context->node, NULL);
          parser_context->state = G_DESKTOP_MENU_PARSER_STATE_MENU;
        }

      else if (g_str_equal (element_name, "Move"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_MOVE, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = G_DESKTOP_MENU_PARSER_STATE_MOVE;
        }

      else if (g_str_equal (element_name, "DefaultLayout"))
        {
          /* TODO Parse attributes */
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_DEFAULT_LAYOUT, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = G_DESKTOP_MENU_PARSER_STATE_LAYOUT;
        }
      else if (g_str_equal (element_name, "Layout"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_LAYOUT, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = G_DESKTOP_MENU_PARSER_STATE_LAYOUT;
        }

      else if (g_str_equal (element_name, "MergeFile"))
        {
          GDesktopMenuMergeFileType type = G_DESKTOP_MENU_MERGE_FILE_PATH;

          if (g_strv_length ((gchar **)attribute_names) == 1 &&
              g_str_equal (attribute_names[0], "type"))
            {
              if (g_str_equal (attribute_values[0], "parent"))
                type = G_DESKTOP_MENU_MERGE_FILE_PARENT;
            }

          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE, GUINT_TO_POINTER (type));
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_MERGE_FILE;
        }
      else if (g_str_equal (element_name, "MergeDir"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_MERGE_DIR;
      else if (g_str_equal (element_name, "DefaultMergeDirs"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      break;

    case G_DESKTOP_MENU_PARSER_STATE_RULE:
      if (g_str_equal (element_name, "All"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_ALL, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "Filename"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_FILENAME;
      else if (g_str_equal (element_name, "Category"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_CATEGORY;
      else if (g_str_equal (element_name, "Or"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_OR, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "And"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_AND, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "Not"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_NOT, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
        }
      break;

    case G_DESKTOP_MENU_PARSER_STATE_MOVE:
      if (g_str_equal (element_name, "Old"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_OLD;
      else if (g_str_equal (element_name, "New"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_NEW;
      break;

    case G_DESKTOP_MENU_PARSER_STATE_LAYOUT:
      if (g_str_equal (element_name, "Filename"))
        parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_FILENAME;
      else if (g_str_equal (element_name, "Menuname"))
        {
          /* TODO Parse attributes */
          parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_MENUNAME;
        }
      else if (g_str_equal (element_name, "Separator"))
        {
          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_SEPARATOR, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "Merge"))
        {
          GDesktopMenuLayoutMergeType type = G_DESKTOP_MENU_LAYOUT_MERGE_ALL;

          if (g_strv_length ((gchar **)attribute_names) == 1 &&
              g_str_equal (attribute_names[0], "type"))
            {
              if (g_str_equal (attribute_values[0], "menus"))
                type = G_DESKTOP_MENU_LAYOUT_MERGE_MENUS;
              else if (g_str_equal (attribute_values[0], "files"))
                type = G_DESKTOP_MENU_LAYOUT_MERGE_FILES;
            }

          node_ = g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_MERGE, GUINT_TO_POINTER (type));
          g_node_append_data (parser_context->node, node_);
        }
      break;

    default: 
      break;
    }
}



static void 
g_desktop_menu_parser_end_element (GMarkupParseContext *context,
                                   const gchar         *element_name,
                                   gpointer             user_data,
                                   GError             **error)
{
  GDesktopMenuParserContext *parser_context = (GDesktopMenuParserContext *)user_data;

  switch (parser_context->state)
    {
    case G_DESKTOP_MENU_PARSER_STATE_ROOT:
    case G_DESKTOP_MENU_PARSER_STATE_MENU:
      if (g_str_equal (element_name, "Menu"))
        {
          /* We no longer have a menu on the stack */
          parser_context->node = parser_context->node->parent;

          if (parser_context->node == NULL)
            parser_context->state = G_DESKTOP_MENU_PARSER_STATE_END;
          else if (parser_context->node->parent == NULL)
            parser_context->state = G_DESKTOP_MENU_PARSER_STATE_ROOT;
        }
      else if (g_str_equal (element_name, "MergeFile"))
        {
          parser_context->node = parser_context->node->parent;

          if (parser_context->node->parent == NULL)
            parser_context->state = G_DESKTOP_MENU_PARSER_STATE_ROOT;
          else 
            parser_context->state = G_DESKTOP_MENU_PARSER_STATE_MENU;
        }
      break;

    case G_DESKTOP_MENU_PARSER_STATE_RULE:
      if (g_str_equal (element_name, "Include") ||
          g_str_equal (element_name, "Exclude") ||
          g_str_equal (element_name, "Or") ||
          g_str_equal (element_name, "And") ||
          g_str_equal (element_name, "Not"))
        {
          /* Switch to the parent rule or menu */
          parser_context->node = parser_context->node->parent;

          /* Set the parse state according to the parent type */
          if (parser_context->node->data == NULL)
            {
              if (parser_context->node->parent == NULL)
                parser_context->state = G_DESKTOP_MENU_PARSER_STATE_ROOT;
              else
                parser_context->state = G_DESKTOP_MENU_PARSER_STATE_MENU;
            }
        }
      break;

    case G_DESKTOP_MENU_PARSER_STATE_MOVE:
      if (g_str_equal (element_name, "Move"))
        {
          parser_context->node = parser_context->node->parent;

          /* Set the parse state according to the parent type */
          if (parser_context->node->data == NULL)
            {
              if (parser_context->node->parent == NULL)
                parser_context->state = G_DESKTOP_MENU_PARSER_STATE_ROOT;
              else
                parser_context->state = G_DESKTOP_MENU_PARSER_STATE_MENU;
            }
        }
      break;

    case G_DESKTOP_MENU_PARSER_STATE_LAYOUT:
      if (g_str_equal (element_name, "Layout") || g_str_equal (element_name, "DefaultLayout"))
        {
          parser_context->node = parser_context->node->parent;

          /* Set the parse state according to the parent type */
          if (parser_context->node->data == NULL)
            {
              if (parser_context->node->parent == NULL)
                parser_context->state = G_DESKTOP_MENU_PARSER_STATE_ROOT;
              else
                parser_context->state = G_DESKTOP_MENU_PARSER_STATE_MENU;
            }
        }
      break;

    default:
      break;
    }
}



static void
g_desktop_menu_parser_characters (GMarkupParseContext *context,
                                  const gchar         *text,
                                  gsize                text_len,
                                  gpointer             user_data,
                                  GError             **error)
{
  GDesktopMenuParserContext *parser_context = (GDesktopMenuParserContext *)user_data;
  gchar                 *data;

  /* Ignore characters outside the root <Menu> element */
  if (G_UNLIKELY (parser_context->node_type == G_DESKTOP_MENU_PARSER_NODE_TYPE_NONE))
    return;

  /* Generate NUL-terminated string */
  data = g_strndup (text, text_len);

  switch (parser_context->node_type)
    {
    case G_DESKTOP_MENU_PARSER_NODE_TYPE_NAME:
      g_node_append_data (parser_context->node, 
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_NAME, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_DIRECTORY:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_DIRECTORY, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_DIRECTORY_DIR:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_DIRECTORY_DIR, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_APP_DIR:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_APP_DIR, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_FILENAME:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_FILENAME, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_CATEGORY:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_CATEGORY, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_OLD:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_OLD, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_NEW:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_NEW, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_MENUNAME:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_MENUNAME, data));
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_MERGE_FILE:
      if (g_desktop_menu_node_tree_get_node_type (parser_context->node) == G_DESKTOP_MENU_NODE_TYPE_MERGE_FILE)
        g_desktop_menu_node_set_merge_file_filename (parser_context->node->data, data);
      break;

    case G_DESKTOP_MENU_PARSER_NODE_TYPE_MERGE_DIR:
      g_node_append_data (parser_context->node,
                          g_desktop_menu_node_create (G_DESKTOP_MENU_NODE_TYPE_MERGE_DIR, data));
      break;

    default:
      break;
    }

  /* Free NUL-terminated string */
  g_free (data);

  /* Invalidate node type information */
  parser_context->node_type = G_DESKTOP_MENU_PARSER_NODE_TYPE_NONE;
}

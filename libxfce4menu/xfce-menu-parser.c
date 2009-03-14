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

#include <libxfce4menu/xfce-menu-node.h>
#include <libxfce4menu/xfce-menu-tree-provider.h>
#include <libxfce4menu/xfce-menu-parser.h>



/* Menu file parser states */
typedef enum
{
  XFCE_MENU_PARSER_STATE_START,
  XFCE_MENU_PARSER_STATE_ROOT,
  XFCE_MENU_PARSER_STATE_MENU,
  XFCE_MENU_PARSER_STATE_RULE,
  XFCE_MENU_PARSER_STATE_MOVE,
  XFCE_MENU_PARSER_STATE_LAYOUT,
  XFCE_MENU_PARSER_STATE_END,
} XfceMenuParserState;

/* Node types */
typedef enum
{
  XFCE_MENU_PARSER_NODE_TYPE_NONE,
  XFCE_MENU_PARSER_NODE_TYPE_NAME,
  XFCE_MENU_PARSER_NODE_TYPE_DIRECTORY,
  XFCE_MENU_PARSER_NODE_TYPE_DIRECTORY_DIR,
  XFCE_MENU_PARSER_NODE_TYPE_APP_DIR,
  XFCE_MENU_PARSER_NODE_TYPE_FILENAME,
  XFCE_MENU_PARSER_NODE_TYPE_CATEGORY,
  XFCE_MENU_PARSER_NODE_TYPE_OLD,
  XFCE_MENU_PARSER_NODE_TYPE_NEW,
  XFCE_MENU_PARSER_NODE_TYPE_MENUNAME,
  XFCE_MENU_PARSER_NODE_TYPE_MERGE_FILE,
  XFCE_MENU_PARSER_NODE_TYPE_MERGE_DIR,
} XfceMenuParserNodeType;

typedef struct _XfceMenuParserContext XfceMenuParserContext;

struct _XfceMenuParserContext
{
  XfceMenuParserNodeType node_type;
  XfceMenuParserState    state;
  XfceMenuParser        *parser;
  GNode                 *node;
};



#define XFCE_MENU_PARSER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), XFCE_TYPE_MENU_PARSER, XfceMenuParserPrivate))



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILE,
};



static void    xfce_menu_parser_class_init    (XfceMenuParserClass       *klass);
static void    xfce_menu_parser_provider_init (XfceMenuTreeProviderIface *iface);
static void    xfce_menu_parser_init          (XfceMenuParser            *parser);
static void    xfce_menu_parser_finalize      (GObject                   *object);
static void    xfce_menu_parser_get_property  (GObject                   *object,
                                               guint                      prop_id,
                                               GValue                    *value,
                                               GParamSpec                *pspec);
static void    xfce_menu_parser_set_property  (GObject                   *object,
                                               guint                      prop_id,
                                               const GValue              *value,
                                               GParamSpec                *pspec);
static void    xfce_menu_parser_start_element (GMarkupParseContext       *context,
                                               const gchar               *element_name,
                                               const gchar              **attribute_names,
                                               const gchar              **attribute_values,
                                               gpointer                   user_data,
                                               GError                   **error);
static void    xfce_menu_parser_end_element   (GMarkupParseContext       *context,
                                               const gchar               *element_name,
                                               gpointer                   user_data,
                                               GError                   **error);
static void    xfce_menu_parser_characters    (GMarkupParseContext       *context,
                                               const gchar               *text,
                                               gsize                      text_len,
                                               gpointer                   user_data,
                                               GError                   **error);
static GNode  *xfce_menu_parser_get_tree      (XfceMenuTreeProvider      *provider);
static GFile  *xfce_menu_parser_get_file      (XfceMenuTreeProvider      *provider);



struct _XfceMenuParserPrivate
{
  /* .menu file */
  GFile *file;

  /* Root menu node */
  GNode *menu;
};



static GObjectClass *xfce_menu_parser_parent_class = NULL;



GType
xfce_menu_parser_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GInterfaceInfo provider_info = 
      {
        (GInterfaceInitFunc) xfce_menu_parser_provider_init,
        NULL,
        NULL,
      };

      type = g_type_register_static_simple (G_TYPE_OBJECT,
                                            "XfceMenuParser",
                                            sizeof (XfceMenuParserClass),
                                            (GClassInitFunc) xfce_menu_parser_class_init,
                                            sizeof (XfceMenuParser),
                                            (GInstanceInitFunc) xfce_menu_parser_init,
                                            0);

      g_type_add_interface_static (type, XFCE_TYPE_MENU_TREE_PROVIDER, &provider_info);
    }

  return type;
}



static void
xfce_menu_parser_class_init (XfceMenuParserClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (XfceMenuParserPrivate));

  /* Determine the parent type class */
  xfce_menu_parser_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_parser_finalize; 
  gobject_class->get_property = xfce_menu_parser_get_property;
  gobject_class->set_property = xfce_menu_parser_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file",
                                                        "file",
                                                        "file",
                                                        G_TYPE_FILE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}



static void 
xfce_menu_parser_provider_init (XfceMenuTreeProviderIface *iface)
{
  iface->get_tree = xfce_menu_parser_get_tree;
  iface->get_file = xfce_menu_parser_get_file;
}



static void
xfce_menu_parser_init (XfceMenuParser *parser)
{
  parser->priv = XFCE_MENU_PARSER_GET_PRIVATE (parser);
  parser->priv->file = NULL;
  parser->priv->menu = NULL;
}



static void
xfce_menu_parser_finalize (GObject *object)
{
  XfceMenuParser *parser = XFCE_MENU_PARSER (object);

  xfce_menu_node_tree_free (parser->priv->menu);

  g_object_unref (parser->priv->file);

  (*G_OBJECT_CLASS (xfce_menu_parser_parent_class)->finalize) (object);
}



static void
xfce_menu_parser_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  XfceMenuParser *parser = XFCE_MENU_PARSER (object);

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
xfce_menu_parser_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  XfceMenuParser *parser = XFCE_MENU_PARSER (object);

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



XfceMenuParser *
xfce_menu_parser_new (GFile *file)
{
  g_return_val_if_fail (G_IS_FILE (file), NULL);
  return g_object_new (XFCE_TYPE_MENU_PARSER, "file", file, NULL);
}



gboolean
xfce_menu_parser_run (XfceMenuParser *parser,
                      GCancellable   *cancellable,
                      GError        **error)
{
  XfceMenuParserContext parser_context;
  GMarkupParseContext  *context;
  GMarkupParser         markup_parser = {
    xfce_menu_parser_start_element,
    xfce_menu_parser_end_element,
    xfce_menu_parser_characters,
    NULL,
  };
  gboolean              result = TRUE;
  gchar                *data;
  gsize                 data_length;

  g_return_val_if_fail (XFCE_IS_MENU_PARSER (parser), FALSE);
  g_return_val_if_fail (G_IS_FILE (parser->priv->file), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* Try to open and read the file */
  if (G_UNLIKELY (!g_file_load_contents (parser->priv->file, cancellable,
                                         &data, &data_length, NULL, error)))
    {
      gchar *uri = g_file_get_uri (parser->priv->file);
      
      if (error != NULL)
        {
          g_warning (_("Could not load menu file data from %s: %s"), uri, (*error)->message);
          g_error_free (*error);
        }
      else
        g_warning (_("Could not load menu file data from %s"), uri);

      g_free (uri);
      return FALSE;
    }

  /* Create parser context */
  parser_context.parser = parser;
  parser_context.node_type = XFCE_MENU_PARSER_NODE_TYPE_NONE;
  parser_context.state = XFCE_MENU_PARSER_STATE_START;
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
xfce_menu_parser_get_tree (XfceMenuTreeProvider *provider)
{
  g_return_val_if_fail (XFCE_IS_MENU_PARSER (provider), NULL);
  return xfce_menu_node_tree_copy (XFCE_MENU_PARSER (provider)->priv->menu);
}



static GFile *
xfce_menu_parser_get_file (XfceMenuTreeProvider *provider)
{
  g_return_val_if_fail (XFCE_IS_MENU_PARSER (provider), NULL);
  return g_object_ref (XFCE_MENU_PARSER (provider)->priv->file);
}



static void
xfce_menu_parser_start_element (GMarkupParseContext *context,
                                const gchar         *element_name,
                                const gchar        **attribute_names,
                                const gchar        **attribute_values,
                                gpointer             user_data,
                                GError             **error)
{
  XfceMenuParserContext *parser_context = (XfceMenuParserContext *)user_data;
  XfceMenuNode          *node_;

  switch (parser_context->state)
    {
    case XFCE_MENU_PARSER_STATE_START:
      if (g_str_equal (element_name, "Menu"))
        {
          parser_context->parser->priv->menu = g_node_new (NULL);

          parser_context->state = XFCE_MENU_PARSER_STATE_ROOT;
          parser_context->node = parser_context->parser->priv->menu;
        }
      break;

    case XFCE_MENU_PARSER_STATE_ROOT:
    case XFCE_MENU_PARSER_STATE_MENU:
      if (g_str_equal (element_name, "Name"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_NAME;

      else if (g_str_equal (element_name, "Directory"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_DIRECTORY;
      else if (g_str_equal (element_name, "DirectoryDir"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_DIRECTORY_DIR;
      else if (g_str_equal (element_name, "DefaultDirectoryDirs"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS, NULL);
          g_node_append_data (parser_context->node, node_);
        }

      else if (g_str_equal (element_name, "AppDir"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_APP_DIR;
      else if (g_str_equal (element_name, "DefaultAppDirs"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_DEFAULT_APP_DIRS, NULL);
          g_node_append_data (parser_context->node, node_);
        }

      else if (g_str_equal (element_name, "Deleted"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_DELETED, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "NotDeleted"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_NOT_DELETED, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "OnlyUnallocated"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "NotOnlyUnallocated"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED, NULL);
          g_node_append_data (parser_context->node, node_);
        }

      else if (g_str_equal (element_name, "Include"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_INCLUDE, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = XFCE_MENU_PARSER_STATE_RULE;
        }
      else if (g_str_equal (element_name, "Exclude"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_EXCLUDE, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = XFCE_MENU_PARSER_STATE_RULE;
        }

      else if (g_str_equal (element_name, "Menu"))
        {
          parser_context->node = g_node_append_data (parser_context->node, NULL);
          parser_context->state = XFCE_MENU_PARSER_STATE_MENU;
        }

      else if (g_str_equal (element_name, "Move"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MOVE, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = XFCE_MENU_PARSER_STATE_MOVE;
        }
      
      else if (g_str_equal (element_name, "Layout"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_LAYOUT, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->state = XFCE_MENU_PARSER_STATE_LAYOUT;
        }

      else if (g_str_equal (element_name, "MergeFile"))
        {
          XfceMenuMergeFileType type = XFCE_MENU_MERGE_FILE_PATH;

          if (g_strv_length ((gchar **)attribute_names) == 1 &&
              g_str_equal (attribute_names[0], "type"))
            {
              if (g_str_equal (attribute_values[0], "parent"))
                type = XFCE_MENU_MERGE_FILE_PARENT;
            }

          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE_FILE, GUINT_TO_POINTER (type));
          parser_context->node = g_node_append_data (parser_context->node, node_);
          parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_MERGE_FILE;
        }
      else if (g_str_equal (element_name, "MergeDir"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_MERGE_DIR;
      else if (g_str_equal (element_name, "DefaultMergeDirs"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      break;

    case XFCE_MENU_PARSER_STATE_RULE:
      if (g_str_equal (element_name, "All"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_ALL, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "Filename"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_FILENAME;
      else if (g_str_equal (element_name, "Category"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_CATEGORY;
      else if (g_str_equal (element_name, "Or"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_OR, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "And"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_AND, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "Not"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_NOT, NULL);
          parser_context->node = g_node_append_data (parser_context->node, node_);
        }
      break;

    case XFCE_MENU_PARSER_STATE_MOVE:
      if (g_str_equal (element_name, "Old"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_OLD;
      else if (g_str_equal (element_name, "New"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_NEW;
      break;

    case XFCE_MENU_PARSER_STATE_LAYOUT:
      if (g_str_equal (element_name, "Filename"))
        parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_FILENAME;
      else if (g_str_equal (element_name, "Menuname"))
        {
          /* TODO Parse attributes */
          parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_MENUNAME;
        }
      else if (g_str_equal (element_name, "Separator"))
        {
          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_SEPARATOR, NULL);
          g_node_append_data (parser_context->node, node_);
        }
      else if (g_str_equal (element_name, "Merge"))
        {
          XfceMenuLayoutMergeType type = XFCE_MENU_LAYOUT_MERGE_ALL;

          if (g_strv_length ((gchar **)attribute_names) == 1 &&
              g_str_equal (attribute_names[0], "type"))
            {
              if (g_str_equal (attribute_values[0], "menus"))
                type = XFCE_MENU_LAYOUT_MERGE_MENUS;
              else if (g_str_equal (attribute_values[0], "files"))
                type = XFCE_MENU_LAYOUT_MERGE_FILES;

              break;
            }

          node_ = xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE, GUINT_TO_POINTER (type));
          g_node_append_data (parser_context->node, node_);
        }
      break;

    default: 
      break;
    }
}



static void 
xfce_menu_parser_end_element (GMarkupParseContext *context,
                              const gchar         *element_name,
                              gpointer             user_data,
                              GError             **error)
{
  XfceMenuParserContext *parser_context = (XfceMenuParserContext *)user_data;

  switch (parser_context->state)
    {
    case XFCE_MENU_PARSER_STATE_ROOT:
    case XFCE_MENU_PARSER_STATE_MENU:
      if (g_str_equal (element_name, "Menu"))
        {
          /* We no longer have a menu on the stack */
          parser_context->node = parser_context->node->parent;

          if (parser_context->node == NULL)
            parser_context->state = XFCE_MENU_PARSER_STATE_END;
          else if (parser_context->node->parent == NULL)
            parser_context->state = XFCE_MENU_PARSER_STATE_ROOT;
        }
      else if (g_str_equal (element_name, "MergeFile"))
        {
          parser_context->node = parser_context->node->parent;

          if (parser_context->node->parent == NULL)
            parser_context->state = XFCE_MENU_PARSER_STATE_ROOT;
          else 
            parser_context->state = XFCE_MENU_PARSER_STATE_MENU;
        }
      break;

    case XFCE_MENU_PARSER_STATE_RULE:
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
                parser_context->state = XFCE_MENU_PARSER_STATE_ROOT;
              else
                parser_context->state = XFCE_MENU_PARSER_STATE_MENU;
            }
        }
      break;

    case XFCE_MENU_PARSER_STATE_MOVE:
      if (g_str_equal (element_name, "Move"))
        {
          parser_context->node = parser_context->node->parent;

          /* Set the parse state according to the parent type */
          if (parser_context->node->data == NULL)
            {
              if (parser_context->node->parent == NULL)
                parser_context->state = XFCE_MENU_PARSER_STATE_ROOT;
              else
                parser_context->state = XFCE_MENU_PARSER_STATE_MENU;
            }
        }
      break;

    case XFCE_MENU_PARSER_STATE_LAYOUT:
      if (g_str_equal (element_name, "Layout"))
        {
          parser_context->node = parser_context->node->parent;

          /* Set the parse state according to the parent type */
          if (parser_context->node->data == NULL)
            {
              if (parser_context->node->parent == NULL)
                parser_context->state = XFCE_MENU_PARSER_STATE_ROOT;
              else
                parser_context->state = XFCE_MENU_PARSER_STATE_MENU;
            }
        }
      break;

    default:
      break;
    }
}



static void
xfce_menu_parser_characters (GMarkupParseContext *context,
                             const gchar         *text,
                             gsize                text_len,
                             gpointer             user_data,
                             GError             **error)
{
  XfceMenuParserContext *parser_context = (XfceMenuParserContext *)user_data;
  gchar                 *data;

  /* Ignore characters outside the root <Menu> element */
  if (G_UNLIKELY (parser_context->node_type == XFCE_MENU_PARSER_NODE_TYPE_NONE))
    return;

  /* Generate NUL-terminated string */
  data = g_strndup (text, text_len);

  switch (parser_context->node_type)
    {
    case XFCE_MENU_PARSER_NODE_TYPE_NAME:
      g_node_append_data (parser_context->node, 
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_NAME, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_DIRECTORY:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_DIRECTORY, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_DIRECTORY_DIR:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_DIRECTORY_DIR, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_APP_DIR:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_APP_DIR, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_FILENAME:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_FILENAME, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_CATEGORY:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_CATEGORY, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_OLD:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_OLD, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_NEW:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_NEW, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_MENUNAME:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MENUNAME, data));
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_MERGE_FILE:
      if (xfce_menu_node_tree_get_node_type (parser_context->node) == XFCE_MENU_NODE_TYPE_MERGE_FILE)
        xfce_menu_node_set_merge_file_filename (parser_context->node->data, data);
      break;

    case XFCE_MENU_PARSER_NODE_TYPE_MERGE_DIR:
      g_node_append_data (parser_context->node,
                          xfce_menu_node_create (XFCE_MENU_NODE_TYPE_MERGE_DIR, data));
      break;

    default:
      break;
    }

  /* Free NUL-terminated string */
  g_free (data);

  /* Invalidate node type information */
  parser_context->node_type = XFCE_MENU_PARSER_NODE_TYPE_NONE;
}

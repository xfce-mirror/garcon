/* $Id$ */
/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2007 Jannis Pohlmann <jannis@xfce.org>
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

#include <libxfce4menu/xfce-menu-layout.h>



#define XFCE_MENU_LAYOUT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), XFCE_TYPE_MENU_LAYOUT, XfceMenuLayoutPrivate))



/* Property identifiers */
enum
{
  PROP_0,
};



struct _XfceMenuLayoutNode
{
  XfceMenuLayoutNodeType   type;
  union
  {
    gchar                  *filename;
    gchar                  *menuname;
    XfceMenuLayoutMergeType merge_type;
  } data;
};



static void xfce_menu_layout_class_init   (XfceMenuLayoutClass *klass);
static void xfce_menu_layout_init         (XfceMenuLayout      *layout);
static void xfce_menu_layout_finalize     (GObject             *object);
static void xfce_menu_layout_get_property (GObject             *object,
                                           guint                prop_id,
                                           GValue              *value,
                                           GParamSpec          *pspec);
static void xfce_menu_layout_set_property (GObject             *object,
                                           guint                prop_id,
                                           const GValue        *value,
                                           GParamSpec          *pspec);
static void xfce_menu_layout_free_node    (XfceMenuLayoutNode  *node);



struct _XfceMenuLayoutClass
{
  GObjectClass __parent__;
};

struct _XfceMenuLayoutPrivate
{
  GSList *nodes;
};

struct _XfceMenuLayout
{
  GObject __parent__;

  /* < private > */
  XfceMenuLayoutPrivate *priv;
};



static GObjectClass *xfce_menu_layout_parent_class = NULL;



GType
xfce_menu_layout_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfceMenuLayoutClass),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_layout_class_init,
        NULL,
        NULL,
        sizeof (XfceMenuLayout),
        0,
        (GInstanceInitFunc) xfce_menu_layout_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMenuLayout", &info, 0);
    }

  return type;
}



static void
xfce_menu_layout_class_init (XfceMenuLayoutClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (XfceMenuLayoutPrivate));

  /* Determine parent type class */
  xfce_menu_layout_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_layout_finalize;
  gobject_class->get_property = xfce_menu_layout_get_property;
  gobject_class->set_property = xfce_menu_layout_set_property;
}



static void
xfce_menu_layout_init (XfceMenuLayout *layout)
{
  layout->priv = XFCE_MENU_LAYOUT_GET_PRIVATE (layout);
  layout->priv->nodes = NULL;
}



static void
xfce_menu_layout_finalize (GObject *object)
{
  XfceMenuLayout *layout = XFCE_MENU_LAYOUT (object);

  /* Free nodes */
  g_slist_foreach (layout->priv->nodes, (GFunc) xfce_menu_layout_free_node, NULL);
  g_slist_free (layout->priv->nodes);

  (*G_OBJECT_CLASS (xfce_menu_layout_parent_class)->finalize) (object);
}



static void
xfce_menu_layout_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  XfceMenuLayout *layout = XFCE_MENU_LAYOUT (layout);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_menu_layout_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  XfceMenuLayout *layout = XFCE_MENU_LAYOUT (layout);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



XfceMenuLayout*
xfce_menu_layout_new (void)
{
  return g_object_new (XFCE_TYPE_MENU_LAYOUT, NULL);
}



static void
xfce_menu_layout_free_node (XfceMenuLayoutNode *node)
{
  if (node->type == XFCE_MENU_LAYOUT_NODE_FILENAME)
    g_free (node->data.filename);
  else if (node->type == XFCE_MENU_LAYOUT_NODE_MENUNAME)
    g_free (node->data.menuname);

  g_free (node);
}



void
xfce_menu_layout_add_filename (XfceMenuLayout *layout,
                               const gchar    *filename)
{
  g_return_if_fail (XFCE_IS_MENU_LAYOUT (layout));
  g_return_if_fail (filename != NULL);

  /* Build filename node */
  XfceMenuLayoutNode *node = g_new0 (XfceMenuLayoutNode, 1);
  node->type = XFCE_MENU_LAYOUT_NODE_FILENAME;
  node->data.filename = g_strdup (filename);

  /* Append node to the list */
  layout->priv->nodes = g_slist_append (layout->priv->nodes, node);
}



void
xfce_menu_layout_add_menuname (XfceMenuLayout *layout,
                               const gchar    *menuname)
{
  g_return_if_fail (XFCE_IS_MENU_LAYOUT (layout));
  g_return_if_fail (menuname != NULL);

  /* Build menuname node */
  XfceMenuLayoutNode *node = g_new0 (XfceMenuLayoutNode, 1);
  node->type = XFCE_MENU_LAYOUT_NODE_MENUNAME;
  node->data.menuname = g_strdup (menuname);

  /* Append node to the list */
  layout->priv->nodes = g_slist_append (layout->priv->nodes, node);
}



void
xfce_menu_layout_add_separator (XfceMenuLayout *layout)
{
  g_return_if_fail (XFCE_IS_MENU_LAYOUT (layout));

  /* Build separator node */
  XfceMenuLayoutNode *node = g_new0 (XfceMenuLayoutNode, 1);
  node->type = XFCE_MENU_LAYOUT_NODE_SEPARATOR;

  /* Append node to the list */
  layout->priv->nodes = g_slist_append (layout->priv->nodes, node);
}



void
xfce_menu_layout_add_merge (XfceMenuLayout         *layout,
                            XfceMenuLayoutMergeType type)
{
  g_return_if_fail (XFCE_IS_MENU_LAYOUT (layout));

  /* Build merge node */
  XfceMenuLayoutNode *node = g_new0 (XfceMenuLayoutNode, 1);
  node->type = XFCE_MENU_LAYOUT_NODE_MERGE;
  node->data.merge_type = type;

  /* Append node to the list */
  layout->priv->nodes = g_slist_append (layout->priv->nodes, node);
}



GSList*
xfce_menu_layout_get_nodes (XfceMenuLayout *layout)
{
  g_return_val_if_fail (XFCE_IS_MENU_LAYOUT (layout), NULL);
  return layout->priv->nodes;
}



gboolean 
xfce_menu_layout_get_filename_used (XfceMenuLayout *layout,
                                    const gchar    *filename)
{
  XfceMenuLayoutNode *node;
  GSList             *iter;
  gboolean            found = FALSE;

  g_return_val_if_fail (XFCE_IS_MENU_LAYOUT (layout), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  for (iter = layout->priv->nodes; iter != NULL; iter = g_slist_next (iter))
    {
      node = (XfceMenuLayoutNode *)iter->data;

      if (G_UNLIKELY (node == NULL))
        continue;

      if (G_UNLIKELY (node->type == XFCE_MENU_LAYOUT_NODE_FILENAME && g_utf8_collate (node->data.filename, filename) == 0))
        {
          found = TRUE;
          break;
        }
    }

  return found;
}



gboolean 
xfce_menu_layout_get_menuname_used (XfceMenuLayout *layout,
                                    const gchar    *menuname)
{
  XfceMenuLayoutNode *node;
  GSList             *iter;
  gboolean            found = FALSE;

  g_return_val_if_fail (XFCE_IS_MENU_LAYOUT (layout), FALSE);
  g_return_val_if_fail (menuname != NULL, FALSE);

  for (iter = layout->priv->nodes; iter != NULL; iter = g_slist_next (iter))
    {
      node = (XfceMenuLayoutNode *)iter->data;

      if (G_UNLIKELY (node == NULL))
        continue;

      if (G_UNLIKELY (node->type == XFCE_MENU_LAYOUT_NODE_MENUNAME && g_utf8_collate (node->data.menuname, menuname) == 0))
        {
          found = TRUE;
          break;
        }
    }

  return found;
}



XfceMenuLayoutNodeType
xfce_menu_layout_node_get_type (XfceMenuLayoutNode *node)
{
  g_return_val_if_fail (node != NULL, XFCE_MENU_LAYOUT_NODE_INVALID);
  return node->type;
}



const gchar*
xfce_menu_layout_node_get_filename (XfceMenuLayoutNode *node)
{
  g_return_val_if_fail (node != NULL && node->type == XFCE_MENU_LAYOUT_NODE_FILENAME, NULL);
  return node->data.filename;
}




const gchar*
xfce_menu_layout_node_get_menuname (XfceMenuLayoutNode *node)
{
  g_return_val_if_fail (node != NULL && node->type == XFCE_MENU_LAYOUT_NODE_MENUNAME, NULL);
  return node->data.menuname;
}




XfceMenuLayoutMergeType
xfce_menu_layout_node_get_merge_type (XfceMenuLayoutNode *node)
{
  g_return_val_if_fail (node != NULL && node->type == XFCE_MENU_LAYOUT_NODE_MERGE, XFCE_MENU_LAYOUT_MERGE_ALL);
  return node->data.merge_type;
}

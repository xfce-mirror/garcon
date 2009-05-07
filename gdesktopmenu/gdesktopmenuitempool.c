/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006-2009 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General 
 * Public License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdesktopmenu/gdesktopmenuitem.h>
#include <gdesktopmenu/gdesktopmenunode.h>
#include <gdesktopmenu/gdesktopmenuitempool.h>



#define G_DESKTOP_MENU_ITEM_POOL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), G_TYPE_DESKTOP_MENU_ITEM_POOL, GDesktopMenuItemPoolPrivate))



static void     g_desktop_menu_item_pool_class_init     (GDesktopMenuItemPoolClass *klass);
static void     g_desktop_menu_item_pool_init           (GDesktopMenuItemPool      *pool);
static void     g_desktop_menu_item_pool_finalize       (GObject                   *object);
static gboolean g_desktop_menu_item_pool_filter_exclude (const gchar               *desktop_id,
                                                         GDesktopMenuItem          *item,
                                                         GNode                     *node);



struct _GDesktopMenuItemPoolClass
{
  GObjectClass __parent__;
};

struct _GDesktopMenuItemPoolPrivate
{
  /* Hash table for mapping desktop-file id's to GDesktopMenuItem's */
  GHashTable *items;
};

struct _GDesktopMenuItemPool
{
  GObject                      __parent__;

  /* < private > */
  GDesktopMenuItemPoolPrivate *priv;
};



static GObjectClass *g_desktop_menu_item_pool_parent_class = NULL;



GType
g_desktop_menu_item_pool_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      type = g_type_register_static_simple (G_TYPE_OBJECT,
                                            "GDesktopMenuItemPool",
                                            sizeof (GDesktopMenuItemPoolClass),
                                            (GClassInitFunc) g_desktop_menu_item_pool_class_init,
                                            sizeof (GDesktopMenuItemPool),
                                            (GInstanceInitFunc) g_desktop_menu_item_pool_init,
                                            0);
    }
  
  return type;
}



static void
g_desktop_menu_item_pool_class_init (GDesktopMenuItemPoolClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (GDesktopMenuItemPoolPrivate));

  /* Determine the parent type class */
  g_desktop_menu_item_pool_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = g_desktop_menu_item_pool_finalize;
}



static void
g_desktop_menu_item_pool_init (GDesktopMenuItemPool *pool)
{
  pool->priv = G_DESKTOP_MENU_ITEM_POOL_GET_PRIVATE (pool);
  pool->priv->items = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, 
                                             (GDestroyNotify) g_desktop_menu_item_unref);
}



static void
g_desktop_menu_item_pool_finalize (GObject *object)
{
  GDesktopMenuItemPool *pool = G_DESKTOP_MENU_ITEM_POOL (object);

  g_hash_table_unref (pool->priv->items);

  (*G_OBJECT_CLASS (g_desktop_menu_item_pool_parent_class)->finalize) (object);
}



GDesktopMenuItemPool*
g_desktop_menu_item_pool_new (void)
{
  return g_object_new (G_TYPE_DESKTOP_MENU_ITEM_POOL, NULL);
}



void
g_desktop_menu_item_pool_insert (GDesktopMenuItemPool *pool,
                                 GDesktopMenuItem     *item)
{
  g_return_if_fail (G_IS_DESKTOP_MENU_ITEM_POOL (pool));
  g_return_if_fail (G_IS_DESKTOP_MENU_ITEM (item));

  /* Insert into the hash table and remove old item (if any) */
  g_hash_table_replace (pool->priv->items, g_strdup (g_desktop_menu_item_get_desktop_id (item)), item);

  /* Grab a reference on the item */
  g_desktop_menu_item_ref (item);
}



GDesktopMenuItem*
g_desktop_menu_item_pool_lookup (GDesktopMenuItemPool *pool,
                                 const gchar          *desktop_id)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ITEM_POOL (pool), NULL);
  g_return_val_if_fail (desktop_id != NULL, NULL);

  return g_hash_table_lookup (pool->priv->items, desktop_id);
}



void 
g_desktop_menu_item_pool_foreach (GDesktopMenuItemPool *pool,
                                  GHFunc                func,
                                  gpointer              user_data)
{
  g_return_if_fail (G_IS_DESKTOP_MENU_ITEM_POOL (pool));

  g_hash_table_foreach (pool->priv->items, func, user_data);
}



void
g_desktop_menu_item_pool_apply_exclude_rule (GDesktopMenuItemPool *pool,
                                             GNode                *node)
{
  g_return_if_fail (G_IS_DESKTOP_MENU_ITEM_POOL (pool));
  g_return_if_fail (node != NULL);

  /* Remove all items which match this exclude rule */
  g_hash_table_foreach_remove (pool->priv->items, (GHRFunc) g_desktop_menu_item_pool_filter_exclude, node);
}



static gboolean
g_desktop_menu_item_pool_filter_exclude (const gchar      *desktop_id,
                                         GDesktopMenuItem *item,
                                         GNode            *node)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ITEM (item), FALSE);
  g_return_val_if_fail (node != NULL, FALSE);
  
  return g_desktop_menu_node_tree_rule_matches (node, item);
}



gboolean
g_desktop_menu_item_pool_get_empty (GDesktopMenuItemPool *pool)
{
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ITEM_POOL (pool), TRUE);
  return (g_hash_table_size (pool->priv->items) == 0);
}

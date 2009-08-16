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

#include <garcon/garcon-menu-node.h>
#include <garcon/garcon-menu-item-pool.h>



#define GARCON_MENU_ITEM_POOL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GARCON_TYPE_MENU_ITEM_POOL, GarconMenuItemPoolPrivate))



static void     garcon_menu_item_pool_finalize       (GObject                 *object);
static gboolean garcon_menu_item_pool_filter_exclude (const gchar             *desktop_id,
                                                      GarconMenuItem          *item,
                                                      GNode                   *node);



struct _GarconMenuItemPoolClass
{
  GObjectClass __parent__;
};

struct _GarconMenuItemPoolPrivate
{
  /* Hash table for mapping desktop-file id's to GarconMenuItem's */
  GHashTable *items;
};

struct _GarconMenuItemPool
{
  GObject                    __parent__;

  /* < private > */
  GarconMenuItemPoolPrivate *priv;
};



G_DEFINE_TYPE (GarconMenuItemPool, garcon_menu_item_pool, G_TYPE_OBJECT)



static void
garcon_menu_item_pool_class_init (GarconMenuItemPoolClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (GarconMenuItemPoolPrivate));

  /* Determine the parent type class */
  garcon_menu_item_pool_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = garcon_menu_item_pool_finalize;
}



static void
garcon_menu_item_pool_init (GarconMenuItemPool *pool)
{
  pool->priv = GARCON_MENU_ITEM_POOL_GET_PRIVATE (pool);
  pool->priv->items = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                             (GDestroyNotify) garcon_menu_item_unref);
}



static void
garcon_menu_item_pool_finalize (GObject *object)
{
  GarconMenuItemPool *pool = GARCON_MENU_ITEM_POOL (object);

  g_hash_table_unref (pool->priv->items);

  (*G_OBJECT_CLASS (garcon_menu_item_pool_parent_class)->finalize) (object);
}



GarconMenuItemPool*
garcon_menu_item_pool_new (void)
{
  return g_object_new (GARCON_TYPE_MENU_ITEM_POOL, NULL);
}



void
garcon_menu_item_pool_insert (GarconMenuItemPool *pool,
                              GarconMenuItem     *item)
{
  g_return_if_fail (GARCON_IS_MENU_ITEM_POOL (pool));
  g_return_if_fail (GARCON_IS_MENU_ITEM (item));

  /* Insert into the hash table and remove old item (if any) */
  g_hash_table_replace (pool->priv->items, g_strdup (garcon_menu_item_get_desktop_id (item)), item);

  /* Grab a reference on the item */
  garcon_menu_item_ref (item);
}



GarconMenuItem*
garcon_menu_item_pool_lookup (GarconMenuItemPool *pool,
                              const gchar        *desktop_id)
{
  g_return_val_if_fail (GARCON_IS_MENU_ITEM_POOL (pool), NULL);
  g_return_val_if_fail (desktop_id != NULL, NULL);

  return g_hash_table_lookup (pool->priv->items, desktop_id);
}



void
garcon_menu_item_pool_foreach (GarconMenuItemPool *pool,
                               GHFunc              func,
                               gpointer            user_data)
{
  g_return_if_fail (GARCON_IS_MENU_ITEM_POOL (pool));

  g_hash_table_foreach (pool->priv->items, func, user_data);
}



void
garcon_menu_item_pool_apply_exclude_rule (GarconMenuItemPool *pool,
                                          GNode              *node)
{
  g_return_if_fail (GARCON_IS_MENU_ITEM_POOL (pool));
  g_return_if_fail (node != NULL);

  /* Remove all items which match this exclude rule */
  g_hash_table_foreach_remove (pool->priv->items, (GHRFunc) garcon_menu_item_pool_filter_exclude, node);
}



static gboolean
garcon_menu_item_pool_filter_exclude (const gchar    *desktop_id,
                                      GarconMenuItem *item,
                                      GNode          *node)
{
  g_return_val_if_fail (GARCON_IS_MENU_ITEM (item), FALSE);
  g_return_val_if_fail (node != NULL, FALSE);

  return garcon_menu_node_tree_rule_matches (node, item);
}



gboolean
garcon_menu_item_pool_get_empty (GarconMenuItemPool *pool)
{
  g_return_val_if_fail (GARCON_IS_MENU_ITEM_POOL (pool), TRUE);
  return (g_hash_table_size (pool->priv->items) == 0);
}

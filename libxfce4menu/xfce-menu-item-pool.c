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

#include <libxfce4menu/xfce-menu-item.h>
#include <libxfce4menu/xfce-menu-item-pool.h>
#include <libxfce4menu/xfce-menu-rules.h>
#include <libxfce4menu/xfce-menu-standard-rules.h>



#define XFCE_MENU_ITEM_POOL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), XFCE_TYPE_MENU_ITEM_POOL, XfceMenuItemPoolPrivate))



static void     xfce_menu_item_pool_class_init       (XfceMenuItemPoolClass *klass);
static void     xfce_menu_item_pool_init             (XfceMenuItemPool      *pool);
static void     xfce_menu_item_pool_finalize         (GObject               *object);
static gboolean xfce_menu_item_pool_filter_exclude   (const gchar           *desktop_id,
                                                      XfceMenuItem          *item,
                                                      XfceMenuStandardRules *rules);



struct _XfceMenuItemPoolClass
{
  GObjectClass __parent__;
};

struct _XfceMenuItemPoolPrivate
{
  /* Hash table for mapping desktop-file id's to XfceMenuItem's */
  GHashTable *items;
};

struct _XfceMenuItemPool
{
  GObject __parent__;

  /* < private > */
  XfceMenuItemPoolPrivate *priv;
};



static GObjectClass *xfce_menu_item_pool_parent_class = NULL;



GType
xfce_menu_item_pool_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
      {
        sizeof (XfceMenuItemPoolClass),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_item_pool_class_init,
        NULL,
        NULL,
        sizeof (XfceMenuItemPool),
        0,
        (GInstanceInitFunc) xfce_menu_item_pool_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMenuItemPool", &info, 0);
    }
  
  return type;
}



static void
xfce_menu_item_pool_class_init (XfceMenuItemPoolClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (XfceMenuItemPoolPrivate));

  /* Determine the parent type class */
  xfce_menu_item_pool_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_item_pool_finalize;
}



static void
xfce_menu_item_pool_init (XfceMenuItemPool *pool)
{
  pool->priv = XFCE_MENU_ITEM_POOL_GET_PRIVATE (pool);
  pool->priv->items = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) xfce_menu_item_unref);
}



static void
xfce_menu_item_pool_finalize (GObject *object)
{
  XfceMenuItemPool *pool = XFCE_MENU_ITEM_POOL (object);

#if GLIB_CHECK_VERSION(2,10,0)
  g_hash_table_unref (pool->priv->items);
#else
  g_hash_table_destroy (pool->priv->items);
#endif

  (*G_OBJECT_CLASS (xfce_menu_item_pool_parent_class)->finalize) (object);
}



XfceMenuItemPool*
xfce_menu_item_pool_new (void)
{
  return g_object_new (XFCE_TYPE_MENU_ITEM_POOL, NULL);
}



void
xfce_menu_item_pool_insert (XfceMenuItemPool *pool,
                            XfceMenuItem     *item)
{
  g_return_if_fail (XFCE_IS_MENU_ITEM_POOL (pool));
  g_return_if_fail (XFCE_IS_MENU_ITEM (item));

  /* Insert into the hash table and remove old item (if any) */
  g_hash_table_replace (pool->priv->items, g_strdup (xfce_menu_item_get_desktop_id (item)), item);

  /* Grab a reference on the item */
  xfce_menu_item_ref (item);
}



XfceMenuItem*
xfce_menu_item_pool_lookup (XfceMenuItemPool *pool,
                            const gchar      *desktop_id)
{
  g_return_val_if_fail (XFCE_IS_MENU_ITEM_POOL (pool), NULL);
  g_return_val_if_fail (desktop_id != NULL, NULL);

  return g_hash_table_lookup (pool->priv->items, desktop_id);
}



void 
xfce_menu_item_pool_foreach (XfceMenuItemPool *pool,
                             GHFunc            func,
                             gpointer          user_data)
{
  g_return_if_fail (XFCE_IS_MENU_ITEM_POOL (pool));

  g_hash_table_foreach (pool->priv->items, func, user_data);
}



void
xfce_menu_item_pool_apply_exclude_rule (XfceMenuItemPool      *pool,
                                        XfceMenuStandardRules *rule)
{
  g_return_if_fail (XFCE_IS_MENU_ITEM_POOL (pool));
  g_return_if_fail (XFCE_IS_MENU_STANDARD_RULES (rule));

  /* Remove all items which match this exclude rule */
  g_hash_table_foreach_remove (pool->priv->items, (GHRFunc) xfce_menu_item_pool_filter_exclude, rule);
}



static gboolean
xfce_menu_item_pool_filter_exclude (const gchar           *desktop_id,
                                    XfceMenuItem          *item,
                                    XfceMenuStandardRules *rule)
{
  g_return_val_if_fail (XFCE_IS_MENU_STANDARD_RULES (rule), FALSE);
  g_return_val_if_fail (XFCE_IS_MENU_ITEM (item), FALSE);

  return xfce_menu_rules_match (XFCE_MENU_RULES (rule), item);
}



gboolean
xfce_menu_item_pool_get_empty (XfceMenuItemPool *pool)
{
  g_return_val_if_fail (XFCE_IS_MENU_ITEM_POOL (pool), TRUE);
  return (g_hash_table_size (pool->priv->items) == 0);
}

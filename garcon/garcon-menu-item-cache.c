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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <garcon/garcon-menu-item.h>
#include <garcon/garcon-menu-item-cache.h>



#define GARCON_MENU_ITEM_CACHE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GARCON_TYPE_MENU_ITEM_CACHE, GarconMenuItemCachePrivate))



static void garcon_menu_item_cache_class_init (GarconMenuItemCacheClass *klass);
static void garcon_menu_item_cache_init       (GarconMenuItemCache      *cache);
static void garcon_menu_item_cache_finalize   (GObject                  *object);




static GarconMenuItemCache *_garcon_menu_item_cache = NULL;



void
_garcon_menu_item_cache_init (void)
{
  if (G_LIKELY (_garcon_menu_item_cache == NULL))
    {
      _garcon_menu_item_cache = g_object_new (GARCON_TYPE_MENU_ITEM_CACHE, NULL);
      g_object_add_weak_pointer (G_OBJECT (_garcon_menu_item_cache), 
                                 (gpointer) &_garcon_menu_item_cache);
    }
}



void
_garcon_menu_item_cache_shutdown (void)
{
  if (G_LIKELY (_garcon_menu_item_cache != NULL))
    g_object_unref (G_OBJECT (_garcon_menu_item_cache));
      
}



struct _GarconMenuItemCacheClass
{
  GObjectClass __parent__;
};

struct _GarconMenuItemCachePrivate
{
  /* Hash table for mapping absolute filenames to GarconMenuItem's */
  GHashTable *items;

  /* Mutex lock */
  GMutex     *lock;
};

struct _GarconMenuItemCache
{
  GObject                     __parent__;

  /* Private data */
  GarconMenuItemCachePrivate *priv;
};



static GObjectClass *garcon_menu_item_cache_parent_class = NULL;



GType
garcon_menu_item_cache_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      type = g_type_register_static_simple (G_TYPE_OBJECT,
                                            "GarconMenuItemCache",
                                            sizeof (GarconMenuItemCacheClass),
                                            (GClassInitFunc) garcon_menu_item_cache_class_init,
                                            sizeof (GarconMenuItemCache),
                                            (GInstanceInitFunc) garcon_menu_item_cache_init,
                                            0);
    }
  
  return type;
}



static void
garcon_menu_item_cache_class_init (GarconMenuItemCacheClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof (GarconMenuItemCachePrivate));

  /* Determine the parent type class */
  garcon_menu_item_cache_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = garcon_menu_item_cache_finalize;
}



static void
garcon_menu_item_cache_init (GarconMenuItemCache *cache)
{
  cache->priv = GARCON_MENU_ITEM_CACHE_GET_PRIVATE (cache);

  /* Initialize the mutex lock */
  cache->priv->lock = g_mutex_new ();

  /* Create empty hash table */
  cache->priv->items = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, 
                                              (GDestroyNotify) garcon_menu_item_unref);
}



GarconMenuItemCache*
garcon_menu_item_cache_get_default (void)
{
  return g_object_ref (G_OBJECT (_garcon_menu_item_cache));
}



static void
garcon_menu_item_cache_finalize (GObject *object)
{
  GarconMenuItemCache *cache = GARCON_MENU_ITEM_CACHE (object);

  /* Free hash table */
  g_hash_table_unref (cache->priv->items);

  /* Destroy the mutex */
  g_mutex_free (cache->priv->lock);

  (*G_OBJECT_CLASS (garcon_menu_item_cache_parent_class)->finalize) (object);
}



GarconMenuItem*
garcon_menu_item_cache_lookup (GarconMenuItemCache *cache,
                               const gchar         *uri,
                               const gchar         *desktop_id)
{
  GarconMenuItem *item = NULL;

  g_return_val_if_fail (GARCON_IS_MENU_ITEM_CACHE (cache), NULL);
  g_return_val_if_fail (uri != NULL, NULL);
  g_return_val_if_fail (desktop_id != NULL, NULL);

  /* Acquire lock on the item cache as it's likely that we need to load 
   * items from the hard drive and store it in the hash table of the 
   * item cache */
  g_mutex_lock (cache->priv->lock);

  /* Search uri in the hash table */
  item = g_hash_table_lookup (cache->priv->items, uri);

  /* Return the item if we we found one */
  if (item != NULL)
    {
      /* Update desktop id, if necessary */
      garcon_menu_item_set_desktop_id (item, desktop_id);

      /* Release item cache lock */
      g_mutex_unlock (cache->priv->lock);

      return item;
    }

  /* Last chance is to load it directly from the file */
  item = garcon_menu_item_new (uri);

  if (G_LIKELY (item != NULL))
    {
      /* Update desktop id */
      garcon_menu_item_set_desktop_id (item, desktop_id);

      /* The file has been loaded, add the item to the hash table */
      g_hash_table_replace (cache->priv->items, g_strdup (uri), item);
    }

  /* Release item cache lock */
  g_mutex_unlock (cache->priv->lock);

  return item;
}



void 
garcon_menu_item_cache_foreach (GarconMenuItemCache *cache,
                                GHFunc               func,
                                gpointer             user_data)
{
  g_return_if_fail (GARCON_IS_MENU_ITEM_CACHE (cache));

  /* Acquire lock on the item cache */
  g_mutex_lock (cache->priv->lock);

  g_hash_table_foreach (cache->priv->items, func, user_data);

  /* Release item cache lock */
  g_mutex_unlock (cache->priv->lock);
}



void
garcon_menu_item_cache_invalidate (GarconMenuItemCache *cache)
{
  g_return_if_fail (GARCON_IS_MENU_ITEM_CACHE (cache));

  /* Destroy the hash table */
  g_hash_table_unref (cache->priv->items);
  
  /* Create a new, empty hash table */
  cache->priv->items = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, 
                                              (GDestroyNotify) garcon_menu_item_unref);
}

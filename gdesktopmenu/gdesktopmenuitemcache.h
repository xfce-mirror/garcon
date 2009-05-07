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

#if !defined(GDESKTOPMENU_INSIDE_GDESKTOPMENU_H) && !defined(GDESKTOPMENU_COMPILATION)
#error "Only <gdesktopmenu/gdesktopmenu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __G_DESKTOP_MENU_ITEM_CACHE_H__
#define __G_DESKTOP_MENU_ITEM_CACHE_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GDesktopMenuItemCachePrivate GDesktopMenuItemCachePrivate;
typedef struct _GDesktopMenuItemCacheClass   GDesktopMenuItemCacheClass;
typedef struct _GDesktopMenuItemCache        GDesktopMenuItemCache;

#define G_TYPE_DESKTOP_MENU_ITEM_CACHE            (g_desktop_menu_item_cache_get_type ())
#define G_DESKTOP_MENU_ITEM_CACHE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU_ITEM_CACHE, GDesktopMenuItemCache))
#define G_DESKTOP_MENU_ITEM_CACHE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DESKTOP_MENU_ITEM_CACHE, GDesktopMenuItemCacheClass))
#define G_IS_DESKTOP_MENU_ITEM_CACHE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU_ITEM_CACHE))
#define G_IS_DESKTOP_MENU_ITEM_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DESKTOP_MENU_ITEM_CACHE))
#define G_DESKTOP_MENU_ITEM_CACHE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DESKTOP_MENU_ITEM_CACHE, GDesktopMenuItemCacheClass))

GType                  g_desktop_menu_item_cache_get_type    (void) G_GNUC_CONST;

GDesktopMenuItemCache *g_desktop_menu_item_cache_get_default (void);

GDesktopMenuItem      *g_desktop_menu_item_cache_lookup      (GDesktopMenuItemCache *cache,
                                                              const gchar           *filename,
                                                              const gchar           *desktop_id);
void                   g_desktop_menu_item_cache_foreach     (GDesktopMenuItemCache *cache,
                                                              GHFunc                 func, 
                                                              gpointer               user_data);
void                   g_desktop_menu_item_cache_invalidate  (GDesktopMenuItemCache *cache);

#if defined(GDESKTOPMENU_COMPILATION)
void                   _g_desktop_menu_item_cache_init       (void) G_GNUC_INTERNAL;
void                   _g_desktop_menu_item_cache_shutdown   (void) G_GNUC_INTERNAL;
#endif

G_END_DECLS

#endif /* !__G_DESKTOP_MENU_ITEM_CACHE_H__ */

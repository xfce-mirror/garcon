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

#if !defined(LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H) && !defined(LIBXFCE4MENU_COMPILATION)
#error "Only <libxfce4menu/libxfce4menu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __XFCE_MENU_ITEM_POOL_H__
#define __XFCE_MENU_ITEM_POOL_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _XfceMenuItemPoolPrivate XfceMenuItemPoolPrivate;
typedef struct _XfceMenuItemPoolClass   XfceMenuItemPoolClass;
typedef struct _XfceMenuItemPool        XfceMenuItemPool;

#define XFCE_TYPE_MENU_ITEM_POOL            (xfce_menu_item_pool_get_type ())
#define XFCE_MENU_ITEM_POOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_ITEM_POOL, XfceMenuItemPool))
#define XFCE_MENU_ITEM_POOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU_ITEM_POOL, XfceMenuItemPoolClass))
#define XFCE_IS_MENU_ITEM_POOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_ITEM_POOL))
#define XFCE_IS_MENU_ITEM_POOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_MENU_ITEM_POOL))
#define XFCE_MENU_ITEM_POOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU_ITEM_POOL, XfceMenuItemPoolClass))

GType             xfce_menu_item_pool_get_type           (void) G_GNUC_CONST;

XfceMenuItemPool *xfce_menu_item_pool_new                (void);

void              xfce_menu_item_pool_insert             (XfceMenuItemPool      *pool,
                                                          XfceMenuItem          *item);
XfceMenuItem     *xfce_menu_item_pool_lookup             (XfceMenuItemPool      *pool,
                                                          const gchar           *desktop_id);
void              xfce_menu_item_pool_foreach            (XfceMenuItemPool      *pool,
                                                          GHFunc                 func, 
                                                          gpointer               user_data);
void              xfce_menu_item_pool_apply_exclude_rule (XfceMenuItemPool      *pool,
                                                          GNode                 *node);
gboolean          xfce_menu_item_pool_get_empty          (XfceMenuItemPool      *pool);

G_END_DECLS

#endif /* !__XFCE_MENU_ITEM_POOL_H__ */

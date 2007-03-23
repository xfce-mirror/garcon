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

#ifndef __XFCE_MENU_ITEM_H__
#define __XFCE_MENU_ITEM_H__

#include <glib-object.h>

G_BEGIN_DECLS;

typedef struct _XfceMenuItemPrivate XfceMenuItemPrivate;
typedef struct _XfceMenuItemClass   XfceMenuItemClass;
typedef struct _XfceMenuItem        XfceMenuItem;

#define XFCE_TYPE_MENU_ITEM            (xfce_menu_item_get_type())
#define XFCE_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_ITEM, XfceMenuItem))
#define XFCE_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU_ITEM, XfceMenuItemClass))
#define XFCE_IS_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_ITEM))
#define XFCE_IS_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_MENU_ITEM))
#define XFCE_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU_ITEM, XfceMenuItemClass))

GType         xfce_menu_item_get_type                          (void) G_GNUC_CONST;

XfceMenuItem *xfce_menu_item_new                               (const gchar  *filename);

const gchar  *xfce_menu_item_get_desktop_id                    (XfceMenuItem *item);
void          xfce_menu_item_set_desktop_id                    (XfceMenuItem *item,
                                                                const gchar  *desktop_id);

const gchar  *xfce_menu_item_get_filename                      (XfceMenuItem *item);
void          xfce_menu_item_set_filename                      (XfceMenuItem *item,
                                                                const gchar  *filename);
const gchar  *xfce_menu_item_get_command                       (XfceMenuItem *item);
void          xfce_menu_item_set_command                       (XfceMenuItem *item,
                                                                const gchar  *command);
const gchar  *xfce_menu_item_get_try_exec                      (XfceMenuItem *item);
void          xfce_menu_item_set_try_exec                      (XfceMenuItem *item,
                                                                const gchar  *try_exec);
const gchar  *xfce_menu_item_get_name                          (XfceMenuItem *item);
void          xfce_menu_item_set_name                          (XfceMenuItem *item,
                                                                const gchar  *name);
const gchar  *xfce_menu_item_get_icon_name                     (XfceMenuItem *item);
void          xfce_menu_item_set_icon_name                     (XfceMenuItem *item,
                                                                const gchar  *icon_name);
gboolean      xfce_menu_item_requires_terminal                 (XfceMenuItem *item);
void          xfce_menu_item_set_requires_terminal             (XfceMenuItem *item,
                                                                gboolean      requires_terminal);
gboolean      xfce_menu_item_get_no_display                    (XfceMenuItem *item);
void          xfce_menu_item_set_no_display                    (XfceMenuItem *item,
                                                                gboolean      no_display);
gboolean      xfce_menu_item_supports_startup_notification     (XfceMenuItem *item);
void          xfce_menu_item_set_supports_startup_notification (XfceMenuItem *item,
                                                                gboolean      supports_startup_notification);
GList        *xfce_menu_item_get_categories                    (XfceMenuItem *item);
void          xfce_menu_item_set_categories                    (XfceMenuItem *item,
                                                                GList        *categories);
gboolean      xfce_menu_item_show_in_environment               (XfceMenuItem *item);
void          xfce_menu_item_ref                               (XfceMenuItem *item);
void          xfce_menu_item_unref                             (XfceMenuItem *item);
gint          xfce_menu_item_get_allocated                     (XfceMenuItem *item);
void          xfce_menu_item_increment_allocated               (XfceMenuItem *item);
void          xfce_menu_item_decrement_allocated               (XfceMenuItem *item);

G_END_DECLS;

#endif /* !__XFCE_MENU_ITEM_H__ */

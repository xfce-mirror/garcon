/* $Id$ */
/*-
 * vi:set et ai sts=2 sw=2 cindent:
 *
 * Copyright (c) 2006-2009 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __G_DESKTOP_MENU_ITEM_H__
#define __G_DESKTOP_MENU_ITEM_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define G_TYPE_DESKTOP_MENU_ITEM            (g_desktop_menu_item_get_type())
#define G_DESKTOP_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU_ITEM, GDesktopMenuItem))
#define G_DESKTOP_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DESKTOP_MENU_ITEM, GDesktopMenuItemClass))
#define G_IS_DESKTOP_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU_ITEM))
#define G_IS_DESKTOP_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DESKTOP_MENU_ITEM))
#define G_DESKTOP_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DESKTOP_MENU_ITEM, GDesktopMenuItemClass))

typedef struct _GDesktopMenuItemPrivate GDesktopMenuItemPrivate;
typedef struct _GDesktopMenuItemClass   GDesktopMenuItemClass;
typedef struct _GDesktopMenuItem        GDesktopMenuItem;

GType             g_desktop_menu_item_get_type                          (void) G_GNUC_CONST;

GDesktopMenuItem *g_desktop_menu_item_new                               (const gchar  *filename) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

const gchar      *g_desktop_menu_item_get_desktop_id                    (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_desktop_id                    (GDesktopMenuItem *item,
                                                                         const gchar      *desktop_id);

const gchar      *g_desktop_menu_item_get_filename                      (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_filename                      (GDesktopMenuItem *item,
                                                                         const gchar       *filename);
const gchar      *g_desktop_menu_item_get_command                       (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_command                       (GDesktopMenuItem *item,
                                                                         const gchar       *command);
const gchar      *g_desktop_menu_item_get_try_exec                      (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_try_exec                      (GDesktopMenuItem *item,
                                                                         const gchar       *try_exec);
const gchar      *g_desktop_menu_item_get_name                          (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_name                          (GDesktopMenuItem *item,
                                                                         const gchar      *name);
const gchar      *g_desktop_menu_item_get_generic_name                  (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_generic_name                  (GDesktopMenuItem *item,
                                                                         const gchar      *generic_name);
const gchar      *g_desktop_menu_item_get_comment                       (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_comment                       (GDesktopMenuItem *item,
                                                                         const gchar      *comment);
const gchar      *g_desktop_menu_item_get_icon_name                     (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_icon_name                     (GDesktopMenuItem *item,
                                                                         const gchar      *icon_name);
const gchar      *g_desktop_menu_item_get_path                          (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_path                          (GDesktopMenuItem *item,
                                                                         const gchar      *path);
gboolean          g_desktop_menu_item_requires_terminal                 (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_requires_terminal             (GDesktopMenuItem *item,
                                                                         gboolean          requires_terminal);
gboolean          g_desktop_menu_item_get_no_display                    (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_no_display                    (GDesktopMenuItem *item,
                                                                         gboolean           no_display);
gboolean          g_desktop_menu_item_supports_startup_notification     (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_supports_startup_notification (GDesktopMenuItem *item,
                                                                         gboolean          supports_startup_notification);
GList            *g_desktop_menu_item_get_categories                    (GDesktopMenuItem *item);
void              g_desktop_menu_item_set_categories                    (GDesktopMenuItem *item,
                                                                         GList            *categories);
gboolean          g_desktop_menu_item_has_category                      (GDesktopMenuItem *item,
                                                                         const gchar      *category);
gboolean          g_desktop_menu_item_get_show_in_environment           (GDesktopMenuItem *item);
gboolean          g_desktop_menu_item_only_show_in_environment          (GDesktopMenuItem *item);
void              g_desktop_menu_item_ref                               (GDesktopMenuItem *item);
void              g_desktop_menu_item_unref                             (GDesktopMenuItem *item);
gint              g_desktop_menu_item_get_allocated                     (GDesktopMenuItem *item);
void              g_desktop_menu_item_increment_allocated               (GDesktopMenuItem *item);
void              g_desktop_menu_item_decrement_allocated               (GDesktopMenuItem *item);

G_END_DECLS

#endif /* !__G_DESKTOP_MENU_ITEM_H__ */

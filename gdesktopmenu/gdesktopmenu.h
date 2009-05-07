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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __G_DESKTOP_MENU_H__
#define __G_DESKTOP_MENU_H__

#include <gio/gio.h>

#define GDESKTOPMENU_INSIDE_GDESKTOPMENU_H
#include <gdesktopmenu/gdesktopmenu-config.h>
#include <gdesktopmenu/gdesktopmenudirectory.h>
#include <gdesktopmenu/gdesktopmenuelement.h>
#include <gdesktopmenu/gdesktopmenuenvironment.h>
#include <gdesktopmenu/gdesktopmenuitem.h>
#include <gdesktopmenu/gdesktopmenuitemcache.h>
#include <gdesktopmenu/gdesktopmenumain.h>
#include <gdesktopmenu/gdesktopmenutreeprovider.h>
#include <gdesktopmenu/gdesktopmenumerger.h>
#include <gdesktopmenu/gdesktopmenunode.h>
#include <gdesktopmenu/gdesktopmenuparser.h>
#include <gdesktopmenu/gdesktopmenuseparator.h>
#undef GDESKTOPMENU_INSIDE_GDESKTOPMENU_H

G_BEGIN_DECLS

#define G_DESKTOP_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU, GDesktopMenu))
#define G_DESKTOP_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DESKTOP_MENU, GDesktopMenuClass))
#define G_IS_DESKTOP_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU))
#define G_IS_DESKTOP_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DESKTOP_MENU))
#define G_DESKTOP_MENU_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DESKTOP_MENU, GDesktopMenuClass))

typedef struct _GDesktopMenuPrivate GDesktopMenuPrivate;
typedef struct _GDesktopMenuClass   GDesktopMenuClass;
typedef struct _GDesktopMenu        GDesktopMenu;

GType                  g_desktop_menu_get_type           (void) G_GNUC_CONST;

GDesktopMenu          *g_desktop_menu_new                (const gchar  *filename) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GDesktopMenu          *g_desktop_menu_new_for_file       (GFile        *file) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GDesktopMenu          *g_desktop_menu_new_applications   (void) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gboolean               g_desktop_menu_load               (GDesktopMenu *menu, 
                                                          GCancellable *cancellable,
                                                          GError      **error);
GFile                 *g_desktop_menu_get_file           (GDesktopMenu *menu);
GDesktopMenuDirectory *g_desktop_menu_get_directory      (GDesktopMenu *menu);
GList                 *g_desktop_menu_get_menus          (GDesktopMenu *menu);
GDesktopMenu          *g_desktop_menu_get_menu_with_name (GDesktopMenu *menu,
                                                          const gchar  *name);
GDesktopMenu          *g_desktop_menu_get_parent         (GDesktopMenu *menu);
GList                 *g_desktop_menu_get_elements       (GDesktopMenu *menu);

G_END_DECLS

#define GDESKTOPMENU_INSIDE_GDESKTOPMENU_H
#include <gdesktopmenu/gdesktopmenuitempool.h>
#include <gdesktopmenu/gdesktopmenumonitor.h>
#undef GDESKTOPMENU_INSIDE_GDESKTOPMENU_H

#endif /* !__G_DESKTOP_MENU_H__ */

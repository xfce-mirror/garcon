/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2007-2009 Jannis Pohlmann <jannis@xfce.org>
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

#if !defined(GDESKTOPMENU_INSIDE_GDESKTOPMENU_H) && !defined(GDESKTOPMENU_COMPILATION)
#error "Only <gdesktopmenu/gdesktopmenu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __G_DESKTOP_MENU_DIRECTORY_H__
#define __G_DESKTOP_MENU_DIRECTORY_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define G_TYPE_DESKTOP_MENU_DIRECTORY            (g_desktop_menu_directory_get_type ())
#define G_DESKTOP_MENU_DIRECTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU_DIRECTORY, GDesktopMenuDirectory))
#define G_DESKTOP_MENU_DIRECTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DESKTOP_MENU_DIRECTORY, GDesktopMenuDirectoryClass))
#define G_IS_DESKTOP_MENU_DIRECTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU_DIRECTORY))
#define G_IS_DESKTOP_MENU_DIRECTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DESKTOP_MENU_DIRECTORY))
#define G_DESKTOP_MENU_DIRECTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DESKTOP_MENU_DIRECTORY, GDesktopMenuDirectoryClass))

typedef struct _GDesktopMenuDirectoryPrivate GDesktopMenuDirectoryPrivate;
typedef struct _GDesktopMenuDirectoryClass   GDesktopMenuDirectoryClass;
typedef struct _GDesktopMenuDirectory        GDesktopMenuDirectory;

GType        g_desktop_menu_directory_get_type                (void) G_GNUC_CONST;

GFile       *g_desktop_menu_directory_get_file                (GDesktopMenuDirectory *directory);
const gchar *g_desktop_menu_directory_get_name                (GDesktopMenuDirectory *directory);
void         g_desktop_menu_directory_set_name                (GDesktopMenuDirectory *directory,
                                                               const gchar           *name);
const gchar *g_desktop_menu_directory_get_comment             (GDesktopMenuDirectory *directory);
void         g_desktop_menu_directory_set_comment             (GDesktopMenuDirectory *directory,
                                                               const gchar           *comment);
const gchar *g_desktop_menu_directory_get_icon                (GDesktopMenuDirectory *directory);
void         g_desktop_menu_directory_set_icon                (GDesktopMenuDirectory *directory,
                                                               const gchar           *icon);
gboolean     g_desktop_menu_directory_get_no_display          (GDesktopMenuDirectory *directory);
void         g_desktop_menu_directory_set_no_display          (GDesktopMenuDirectory *directory,
                                                               gboolean               no_display);
gboolean     g_desktop_menu_directory_get_hidden              (GDesktopMenuDirectory *directory);
gboolean     g_desktop_menu_directory_get_show_in_environment (GDesktopMenuDirectory *directory);
gboolean     g_desktop_menu_directory_get_visible             (GDesktopMenuDirectory *directory);
gboolean     g_desktop_menu_directory_equal                   (GDesktopMenuDirectory *directory,
                                                               GDesktopMenuDirectory *other);

#if defined(GDESKTOPMENU_COMPILATION)
void _g_desktop_menu_directory_init     (void) G_GNUC_INTERNAL;
void _g_desktop_menu_directory_shutdown (void) G_GNUC_INTERNAL;
#endif

G_END_DECLS

#endif /* !__G_DESKTOP_MENU_DIRECTORY_H__ */

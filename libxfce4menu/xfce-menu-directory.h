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

#ifndef __XFCE_MENU_DIRECTORY_H__
#define __XFCE_MENU_DIRECTORY_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS;

typedef struct _XfceMenuDirectoryPrivate XfceMenuDirectoryPrivate;
typedef struct _XfceMenuDirectoryClass   XfceMenuDirectoryClass;
typedef struct _XfceMenuDirectory        XfceMenuDirectory;

#define XFCE_TYPE_MENU_DIRECTORY            (xfce_menu_directory_get_type ())
#define XFCE_MENU_DIRECTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_DIRECTORY, XfceMenuDirectory))
#define XFCE_MENU_DIRECTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU_DIRECTORY, XfceMenuDirectoryClass))
#define XFCE_IS_MENU_DIRECTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_DIRECTORY))
#define XFCE_IS_MENU_DIRECTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_MENU_DIRECTORY))
#define XFCE_MENU_DIRECTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU_DIRECTORY, XfceMenuDirectoryClass))

GType                    xfce_menu_directory_get_type            (void) G_GNUC_CONST;

GFile                   *xfce_menu_directory_get_file            (XfceMenuDirectory *directory);
const gchar             *xfce_menu_directory_get_name            (XfceMenuDirectory *directory);
void                     xfce_menu_directory_set_name            (XfceMenuDirectory *directory,
                                                                  const gchar       *name);
const gchar             *xfce_menu_directory_get_comment         (XfceMenuDirectory *directory);
void                     xfce_menu_directory_set_comment         (XfceMenuDirectory *directory,
                                                                  const gchar       *comment);
const gchar             *xfce_menu_directory_get_icon            (XfceMenuDirectory *directory);
void                     xfce_menu_directory_set_icon            (XfceMenuDirectory *directory,
                                                                  const gchar       *icon);
gboolean                 xfce_menu_directory_get_no_display      (XfceMenuDirectory *directory);
void                     xfce_menu_directory_set_no_display      (XfceMenuDirectory *directory,
                                                                  gboolean           no_display);
gboolean                 xfce_menu_directory_get_hidden          (XfceMenuDirectory *directory);
gboolean                 xfce_menu_directory_show_in_environment (XfceMenuDirectory *directory);
gboolean                 xfce_menu_directory_equal               (XfceMenuDirectory *directory,
                                                                  XfceMenuDirectory *other);

#if defined(LIBXFCE4MENU_COMPILATION)
void _xfce_menu_directory_init     (void) G_GNUC_INTERNAL;
void _xfce_menu_directory_shutdown (void) G_GNUC_INTERNAL;
#endif

G_END_DECLS;

#endif /* !__XFCE_MENU_DIRECTORY_H__ */

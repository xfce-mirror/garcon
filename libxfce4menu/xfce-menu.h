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

#if !defined(LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H) && !defined(LIBXFCE4MENU_COMPILATION)
#error "Only <libxfce4menu/libxfce4menu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __XFCE_MENU_H__
#define __XFCE_MENU_H__

#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _XfceMenuPrivate XfceMenuPrivate;
typedef struct _XfceMenuClass   XfceMenuClass;
typedef struct _XfceMenu        XfceMenu;

#define XFCE_TYPE_MENU            (xfce_menu_get_type ())
#define XFCE_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU, XfceMenu))
#define XFCE_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU, XfceMenuClass))
#define XFCE_IS_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU))
#define XFCE_IS_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_MENU))
#define XFCE_MENU_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU, XfceMenuClass))

GType              xfce_menu_get_type              (void) G_GNUC_CONST;

XfceMenu          *xfce_menu_new                   (const gchar       *filename) G_GNUC_MALLOC;
XfceMenu          *xfce_menu_new_for_file          (GFile             *file) G_GNUC_MALLOC;
XfceMenu          *xfce_menu_new_applications      (void) G_GNUC_MALLOC;
gboolean           xfce_menu_load                  (XfceMenu          *menu, 
                                                    GCancellable      *cancellable,
                                                    GError           **error);
GFile             *xfce_menu_get_file              (XfceMenu          *menu);
XfceMenuDirectory *xfce_menu_get_directory         (XfceMenu          *menu);
GList             *xfce_menu_get_menus             (XfceMenu          *menu);
XfceMenu          *xfce_menu_get_menu_with_name    (XfceMenu          *menu,
                                                    const gchar       *name);
XfceMenu          *xfce_menu_get_parent            (XfceMenu          *menu);
GList             *xfce_menu_get_elements          (XfceMenu          *menu);

G_END_DECLS

#endif /* !__XFCE_MENU_H__ */

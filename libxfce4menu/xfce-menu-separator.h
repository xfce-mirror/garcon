/* $Id$ */
/* vi:set et ai sw=2 sts=2: */
/*-
 * Copyright (c) 2007 Jannis Pohlmann <jannis@xfce.org>
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

#ifndef __XFCE_MENU_SEPARATOR_H__
#define __XFCE_MENU_SEPARATOR_H__

#include <glib-object.h>

G_BEGIN_DECLS;

typedef struct _XfceMenuSeparatorClass XfceMenuSeparatorClass;
typedef struct _XfceMenuSeparator      XfceMenuSeparator;

#define XFCE_TYPE_MENU_SEPARATOR            (xfce_menu_separator_get_type())
#define XFCE_MENU_SEPARATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_SEPARATOR, XfceMenuSeparator))
#define XFCE_MENU_SEPARATOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU_SEPARATOR, XfceMenuSeparatorClass))
#define XFCE_IS_MENU_SEPARATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_SEPARATOR))
#define XFCE_IS_MENU_SEPARATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_MENU_SEPARATOR))
#define XFCE_MENU_SEPARATOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU_SEPARATOR, XfceMenuSeparatorClass))


GType              xfce_menu_separator_get_type    (void) G_GNUC_CONST;

XfceMenuSeparator *xfce_menu_separator_get_default (void);

#if defined(LIBXFCE4MENU_COMPILATION)
void               _xfce_menu_separator_init       (void) G_GNUC_INTERNAL;
void               _xfce_menu_separator_shutdown   (void) G_GNUC_INTERNAL;
#endif

G_END_DECLS;

#endif /* !__XFCE_MENU_SEPARATOR_H__ */

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

#if !defined(GDESKTOPMENU_INSIDE_GDESKTOPMENU_H) && !defined(GDESKTOPMENU_COMPILATION)
#error "Only <gdesktopmenu/gdesktopmenu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __G_DESKTOP_MENU_SEPARATOR_H__
#define __G_DESKTOP_MENU_SEPARATOR_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _GDesktopMenuSeparatorClass GDesktopMenuSeparatorClass;
typedef struct _GDesktopMenuSeparator      GDesktopMenuSeparator;

#define G_TYPE_DESKTOP_MENU_SEPARATOR            (g_desktop_menu_separator_get_type())
#define G_DESKTOP_MENU_SEPARATOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU_SEPARATOR, GDesktopMenuSeparator))
#define G_DESKTOP_MENU_SEPARATOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DESKTOP_MENU_SEPARATOR, GDesktopMenuSeparatorClass))
#define G_IS_DESKTOP_MENU_SEPARATOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU_SEPARATOR))
#define G_IS_DESKTOP_MENU_SEPARATOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DESKTOP_MENU_SEPARATOR))
#define G_DESKTOP_MENU_SEPARATOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DESKTOP_MENU_SEPARATOR, GDesktopMenuSeparatorClass))


GType                  g_desktop_menu_separator_get_type    (void) G_GNUC_CONST;

GDesktopMenuSeparator *g_desktop_menu_separator_get_default (void);

#if defined(GDESKTOPMENU_COMPILATION)
void                   _g_desktop_menu_separator_init       (void) G_GNUC_INTERNAL;
void                   _g_desktop_menu_separator_shutdown   (void) G_GNUC_INTERNAL;
#endif

G_END_DECLS

#endif /* !__G_DESKTOP_MENU_SEPARATOR_H__ */

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

#if !defined (GDESKTOPMENU_INSIDE_GDESKTOPMENU_H) && !defined (GDESKTOPMENU_COMPILATION)
#error "Only <gdesktopmenu/gdesktopmenu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __G_DESKTOP_MENU_MAIN_H__
#define __G_DESKTOP_MENU_MAIN_H__

#include <gdesktopmenu/gdesktopmenu.h>

G_BEGIN_DECLS

void   g_desktop_menu_init          (const gchar *env);
void   g_desktop_menu_shutdown      (void);

gchar *g_desktop_menu_config_lookup (const gchar *filename) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#define G_TYPE_DESKTOP_MENU            (g_desktop_menu_get_type ())

#endif /* !__G_DESKTOP_MENU_MAIN_H__ */

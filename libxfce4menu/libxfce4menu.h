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

#ifndef __LIBXFCE4MENU_LIBXFCE4MENU_H__
#define __LIBXFCE4MENU_LIBXFCE4MENU_H__

#define LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H

#include <glib.h>

#include <libxfce4menu/libxfce4menu-config.h>
#include <libxfce4menu/xfce-menu-environment.h>
#include <libxfce4menu/xfce-menu-element.h>
#include <libxfce4menu/xfce-menu-item.h>
#include <libxfce4menu/xfce-menu-item-pool.h>
#include <libxfce4menu/xfce-menu-item-cache.h>
#include <libxfce4menu/xfce-menu-directory.h>
#include <libxfce4menu/xfce-menu-layout.h>
#include <libxfce4menu/xfce-menu-separator.h>
#include <libxfce4menu/xfce-menu-node.h>
#include <libxfce4menu/xfce-menu-tree-provider.h>
#include <libxfce4menu/xfce-menu-merger.h>
#include <libxfce4menu/xfce-menu-parser.h>
#include <libxfce4menu/xfce-menu.h>
#include <libxfce4menu/xfce-menu-monitor.h>

#undef LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H

G_BEGIN_DECLS

void   libxfce4menu_init       (const gchar *env);
void   libxfce4menu_shutdown   (void);

gchar *xfce_menu_config_lookup (const gchar *filename) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#endif /* !__LIBXFCE4MENU_LIBXFCE4MENU_H__ */

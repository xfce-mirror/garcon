/* $Id$ */
/*-
 * vi:set sw=2 sts=2 et ai cindent:
 *
 * Copyright (c) 2006 Jannis Pohlmann <jannis@xfce.org>
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

#if !defined (LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H) && !defined (LIBXFCE4MENU_COMPILATION)
#error "Only <libxfce4menu/libxfce4menu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __XFCE_MENU_MONITOR_H__
#define __XFCE_MENU_MONITOR_H__

#include <glib.h>
#include <libxfce4menu/libxfce4menu.h>

G_BEGIN_DECLS;

typedef struct _XfceMenuMonitorVTable XfceMenuMonitorVTable;

void     xfce_menu_monitor_set_vtable  (XfceMenuMonitorVTable *vtable, 
                                        gpointer               user_data);
gpointer xfce_menu_monitor_add_item    (XfceMenu              *menu,
                                        XfceMenuItem          *item);
void     xfce_menu_monitor_remove_item (XfceMenu              *menu,
                                        XfceMenuItem          *item);

/**
 * XfceMenuMonitorVTable:
 *
 * This structure can be used by clients of the libxfce4menu API to 
 * register functions which will be called whenever monitoring a
 * certain file or directory becomes necessary. This way libxfce4menu
 * only has to manage the monitor handles and leaves the monitoring
 * implementations to the client.
 *
 * This mechanism was invented because the two main API clients, 
 * Thunar and xfdesktop are already linked to ThunarVFS which has
 * monitoring capabilities.
 */
struct _XfceMenuMonitorVTable
{
  gpointer (*monitor_file)      (XfceMenu    *menu,
                                 const gchar *filename,
                                 gpointer     user_data);

  gpointer (*monitor_directory) (XfceMenu    *menu,
                                 const gchar *filename,
                                 gpointer     user_data);

  void     (*remove_monitor)    (XfceMenu    *menu,
                                 gpointer     monitor_handle);
};

#if defined(LIBXFCE4MENU_COMPILATION)
void _xfce_menu_monitor_init     (void) G_GNUC_INTERNAL;
void _xfce_menu_monitor_shutdown (void) G_GNUC_INTERNAL;
#endif

G_END_DECLS;

#endif /* !__XFCE_MENU_MONITOR_H__ */

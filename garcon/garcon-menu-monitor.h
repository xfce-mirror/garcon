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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General 
 * Public License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#if !defined (GARCON_INSIDE_GARCON_H) && !defined (GARCON_COMPILATION)
#error "Only <garcon/garcon.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __GARCON_MENU_MONITOR_H__
#define __GARCON_MENU_MONITOR_H__

#include <garcon/garcon.h>

G_BEGIN_DECLS

typedef enum
{
  GARCON_MENU_MONITOR_DIRECTORIES     = 1 << 0,
  GARCON_MENU_MONITOR_MENU_FILES      = 1 << 1,
  GARCON_MENU_MONITOR_DIRECTORY_FILES = 1 << 2,
  GARCON_MENU_MONITOR_DESKTOP_FILES   = 1 << 3
} GarconMenuMonitorFlags;

typedef struct _GarconMenuMonitorVTable GarconMenuMonitorVTable;

void                   garcon_menu_monitor_set_vtable       (GarconMenuMonitorVTable *vtable, 
                                                             gpointer                 user_data);
gpointer               garcon_menu_monitor_add_item         (GarconMenu              *menu,
                                                             GarconMenuItem          *item);
void                   garcon_menu_monitor_remove_item      (GarconMenu              *menu,
                                                             GarconMenuItem          *item);
gpointer               garcon_menu_monitor_add_directory    (GarconMenu              *menu,
                                                             const gchar             *directory);
void                   garcon_menu_monitor_remove_directory (GarconMenu              *menu,
                                                             const gchar             *directory);
gpointer               garcon_menu_monitor_add_file         (GarconMenu              *menu,
                                                             const gchar             *filename);
void                   garcon_menu_monitor_remove_file      (GarconMenu              *menu,
                                                             const gchar             *filename);
void                   garcon_menu_monitor_set_flags        (GarconMenuMonitorFlags   flags);
GarconMenuMonitorFlags garcon_menu_monitor_get_flags        (void);
gboolean               garcon_menu_monitor_has_flags        (GarconMenuMonitorFlags   flags);

/**
 * GarconMenuMonitorVTable:
 * @monitor_file      : Function called by garcon to request that
 *                      a file should be monitored.
 * @monitor_directory : Function called by garcon to request that
 *                      a directory should be monitored.
 * @remove_monitor    : Function called by garcon to request that
 *                      a file or directory monitor should be 
 *                      cancelled/removed.
 *
 * This structure can be used by clients of the garcon API to 
 * register functions which will be called whenever monitoring a
 * certain file or directory becomes necessary. This way garcon
 * only has to manage the monitor handles and leaves the monitoring
 * implementations to the client.
 *
 * This mechanism was invented because the two main API clients, 
 * Thunar and xfdesktop are already linked to ThunarVFS which has
 * monitoring capabilities.
 **/
struct _GarconMenuMonitorVTable
{
  gpointer (*monitor_file)      (GarconMenu    *menu,
                                 const gchar *filename,
                                 gpointer     user_data);

  gpointer (*monitor_directory) (GarconMenu    *menu,
                                 const gchar *filename,
                                 gpointer     user_data);

  void     (*remove_monitor)    (GarconMenu    *menu,
                                 gpointer     monitor_handle);
};

#if defined(GARCON_COMPILATION)
void _garcon_menu_monitor_init     (void) G_GNUC_INTERNAL;
void _garcon_menu_monitor_shutdown (void) G_GNUC_INTERNAL;
#endif

G_END_DECLS

#endif /* !__GARCON_MENU_MONITOR_H__ */

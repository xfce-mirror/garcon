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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxfce4menu/xfce-menu-monitor.h>
#include <libxfce4menu/xfce-menu.h>
#include <libxfce4menu/xfce-menu-item.h>



/* Whether the vtable has been set once */
static gboolean xfce_menu_monitor_vtable_set = FALSE;

/* Initial vtable configuration */
static XfceMenuMonitorVTable xfce_menu_monitor_vtable = {
  NULL,
  NULL,
  NULL,
};

/* User data as provided by the client */
static gpointer xfce_menu_monitor_user_data = NULL;

/* Hash table with (XfceMenuElement => gpointer) pairs */
static GHashTable *xfce_menu_monitor_handles;



void 
_xfce_menu_monitor_init (void)
{
  /* Initialize hash table */
  xfce_menu_monitor_handles = g_hash_table_new (NULL, NULL);
}



void
_xfce_menu_monitor_shutdown (void)
{
#if GLIB_CHECK_VERSION(2,10,0)
  g_hash_table_unref (xfce_menu_monitor_handles);
#else
  g_hash_table_destroy (xfce_menu_monitor_handles);
#endif
}



/**
 * xfce_menu_monitor_set_vtable:
 * @vtable    : a #XfceMenuMonitorVTable
 * @user_data : custom pointer to be passed to the vtable functions.
 *
 * Sets the functions to be called when monitoring a file or directory
 * becomes necessary. See #XfceMenuMonitorVTable for more detailled
 * information.
 */
void
xfce_menu_monitor_set_vtable (XfceMenuMonitorVTable *vtable,
                              gpointer               user_data)
{
  if (G_LIKELY (!xfce_menu_monitor_vtable_set))
    {
      if (G_UNLIKELY (vtable->monitor_file && vtable->monitor_directory && vtable->remove_monitor))
        {
          xfce_menu_monitor_vtable.monitor_file = vtable->monitor_file;
          xfce_menu_monitor_vtable.monitor_directory = vtable->monitor_directory;
          xfce_menu_monitor_vtable.remove_monitor = vtable->remove_monitor;
          xfce_menu_monitor_user_data = user_data;
        }
    }
}



gpointer
xfce_menu_monitor_add_item (XfceMenu     *menu,
                            XfceMenuItem *item)
{
  gpointer monitor_handle;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  g_return_val_if_fail (XFCE_IS_MENU_ITEM (item), NULL);
  g_return_val_if_fail (xfce_menu_monitor_vtable.monitor_file != NULL, NULL);

  /* Request monitor handle from the library client */
  monitor_handle = xfce_menu_monitor_vtable.monitor_file (menu, xfce_menu_item_get_filename (item), xfce_menu_monitor_user_data);

  if (G_LIKELY (monitor_handle != NULL))
    {
      /* Store the item => handle pair in the hash table */
      g_hash_table_insert (xfce_menu_monitor_handles, item, monitor_handle);
    }

  return monitor_handle;
}



void
xfce_menu_monitor_remove_item (XfceMenu     *menu,
                               XfceMenuItem *item)
{
  gpointer monitor_handle;

  g_return_if_fail (XFCE_IS_MENU_ITEM (item));
  g_return_if_fail (xfce_menu_monitor_vtable.remove_monitor != NULL);

  /* Lookup the monitor handle for this item */
  monitor_handle = g_hash_table_lookup (xfce_menu_monitor_handles, item);

  if (G_LIKELY (monitor_handle != NULL))
    {
      /* Remove monitor handle from the library client */
      xfce_menu_monitor_vtable.remove_monitor (menu, monitor_handle);
    }
}

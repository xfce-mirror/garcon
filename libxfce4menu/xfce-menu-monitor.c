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

/* Hash table with (XfceMenuItem => gpointer) pairs */
static GHashTable *xfce_menu_monitor_item_handles;

/* Hash table with (Directory => gpointer) pairs */
static GHashTable *xfce_menu_monitor_shared_handles;

/* Structure for directory handles */
typedef struct _SharedHandle SharedHandle;
struct _SharedHandle  
{
  gpointer monitor_handle;
  int      references;
};



void 
_xfce_menu_monitor_init (void)
{
  /* Initialize hash tables */
  xfce_menu_monitor_item_handles = g_hash_table_new (NULL, NULL);
  xfce_menu_monitor_shared_handles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}



void
_xfce_menu_monitor_shutdown (void)
{
#if GLIB_CHECK_VERSION(2,10,0)
  g_hash_table_unref (xfce_menu_monitor_item_handles);
  g_hash_table_unref (xfce_menu_monitor_shared_handles);
#else
  g_hash_table_destroy (xfce_menu_monitor_item_handles);
  g_hash_table_destroy (xfce_menu_monitor_shared_handles);
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
  
  if (G_UNLIKELY (xfce_menu_monitor_vtable.monitor_file == NULL))
    return NULL;

  /* Request monitor handle from the library client */
  monitor_handle = xfce_menu_monitor_vtable.monitor_file (menu, xfce_menu_item_get_filename (item), xfce_menu_monitor_user_data);

  if (G_LIKELY (monitor_handle != NULL))
    {
      /* Store the item => handle pair in the hash table */
      g_hash_table_insert (xfce_menu_monitor_item_handles, item, monitor_handle);
    }

  return monitor_handle;
}



void
xfce_menu_monitor_remove_item (XfceMenu     *menu,
                               XfceMenuItem *item)
{
  gpointer monitor_handle;

  g_return_if_fail (XFCE_IS_MENU_ITEM (item));
  
  if (G_UNLIKELY (xfce_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the monitor handle for this item */
  monitor_handle = g_hash_table_lookup (xfce_menu_monitor_item_handles, item);

  if (G_LIKELY (monitor_handle != NULL))
    {
      /* Remove monitor handle from the library client */
      xfce_menu_monitor_vtable.remove_monitor (menu, monitor_handle);

      /* ... and remove the item from the hash table */
      g_hash_table_remove (xfce_menu_monitor_item_handles, item);
    }
}



gpointer
xfce_menu_monitor_add_directory (XfceMenu    *menu,
                                 const gchar *directory)
{
  SharedHandle *shared_handle;
  gpointer         monitor_handle;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  g_return_val_if_fail (directory != NULL, NULL);
  
  if (G_UNLIKELY (xfce_menu_monitor_vtable.monitor_directory == NULL))
    return NULL;

  /* Load directory handle from the hash table */
  shared_handle = (SharedHandle *)g_hash_table_lookup (xfce_menu_monitor_shared_handles, directory);

  /* Check if the directory is already monitored */
  if (G_LIKELY (shared_handle != NULL))
    {
      /* Increment reference counter */
      shared_handle->references++;

      /* Return the monitor handle */
      monitor_handle = shared_handle->monitor_handle;
    }
  else
    {
      /* Request monitor handle from the library client */
      monitor_handle = xfce_menu_monitor_vtable.monitor_directory (menu, directory, xfce_menu_monitor_user_data);

      if (G_LIKELY (monitor_handle != NULL))
        {
          /* Allocate new directory handle */
          shared_handle = g_new0 (SharedHandle, 1);

          /* Set values (reference counter starts with 1) */
          shared_handle->references = 1;
          shared_handle->monitor_handle = monitor_handle;

          /* Store the item => handle pair in the hash table */
          g_hash_table_insert (xfce_menu_monitor_shared_handles, g_strdup (directory), shared_handle);
        }
    }

  return monitor_handle;
}



void
xfce_menu_monitor_remove_directory (XfceMenu    *menu,
                                    const gchar *directory)
{
  SharedHandle *shared_handle;

  g_return_if_fail (directory != NULL);
  
  if (G_UNLIKELY (xfce_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the directory handle for this directory */
  shared_handle = g_hash_table_lookup (xfce_menu_monitor_shared_handles, directory);

  if (G_LIKELY (shared_handle != NULL))
    {
      /* Decrement the reference counter */
      shared_handle->references--;

      /* Check if there are no references left */
      if (G_UNLIKELY (shared_handle->references == 0)) 
        {
          /* Remove monitor handle from the library client */
          xfce_menu_monitor_vtable.remove_monitor (menu, shared_handle->monitor_handle);

          /* Remove directory handle from the hash table and destroy it */
          g_hash_table_remove (xfce_menu_monitor_shared_handles, shared_handle);
        }
    }
}



gpointer
xfce_menu_monitor_add_file (XfceMenu    *menu,
                            const gchar *filename)
{
  SharedHandle *shared_handle;
  gpointer      monitor_handle;

  g_return_val_if_fail (XFCE_IS_MENU (menu), NULL);
  g_return_val_if_fail (filename != NULL, NULL);
  
  if (G_UNLIKELY (xfce_menu_monitor_vtable.monitor_file == NULL))
    return NULL;

  /* Load filename handle from the hash table */
  shared_handle = (SharedHandle *)g_hash_table_lookup (xfce_menu_monitor_shared_handles, filename);

  /* Check if the file is already monitored */
  if (G_LIKELY (shared_handle != NULL))
    {
      /* Increment reference counter */
      shared_handle->references++;

      /* Return the monitor handle */
      monitor_handle = shared_handle->monitor_handle;
    }
  else
    {
      /* Request monitor handle from the library client */
      monitor_handle = xfce_menu_monitor_vtable.monitor_file (menu, filename, xfce_menu_monitor_user_data);

      if (G_LIKELY (monitor_handle != NULL))
        {
          /* Allocate new filename handle */
          shared_handle = g_new0 (SharedHandle, 1);

          /* Set values (reference counter starts with 1) */
          shared_handle->references = 1;
          shared_handle->monitor_handle = monitor_handle;

          /* Store the item => handle pair in the hash table */
          g_hash_table_insert (xfce_menu_monitor_shared_handles, g_strdup (filename), shared_handle);
        }
    }

  return monitor_handle;
}



void
xfce_menu_monitor_remove_file (XfceMenu    *menu,
                               const gchar *filename)
{
  SharedHandle *shared_handle;

  g_return_if_fail (XFCE_IS_MENU (menu));
  g_return_if_fail (filename != NULL);
  
  if (G_UNLIKELY (xfce_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the filename handle for this file */
  shared_handle = g_hash_table_lookup (xfce_menu_monitor_shared_handles, filename);

  if (G_LIKELY (shared_handle != NULL))
    {
      /* Decrement the reference counter */
      shared_handle->references--;

      /* Check if there are no references left */
      if (G_UNLIKELY (shared_handle->references == 0)) 
        {
          /* Remove monitor handle from the library client */
          xfce_menu_monitor_vtable.remove_monitor (menu, shared_handle->monitor_handle);

          /* Remove filename handle from the hash table and destroy it */
          g_hash_table_remove (xfce_menu_monitor_shared_handles, shared_handle);
        }
    }
}

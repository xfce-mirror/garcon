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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdesktopmenu/gdesktopmenumonitor.h>
#include <gdesktopmenu/gdesktopmenu.h>
#include <gdesktopmenu/gdesktopmenuitem.h>



/* Initial vtable configuration */
static GDesktopMenuMonitorVTable g_desktop_menu_monitor_vtable = {
  NULL,
  NULL,
  NULL,
};

/* Monitor flags */
static GDesktopMenuMonitorFlags g_desktop_menu_monitor_flags;

/* User data as provided by the client */
static gpointer                 g_desktop_menu_monitor_user_data = NULL;

/* Hash table with (GDesktopMenuItem => gpointer) pairs */
static GHashTable              *g_desktop_menu_monitor_item_handles;

/* Hash table with (Directory => gpointer) pairs */
static GHashTable              *g_desktop_menu_monitor_shared_handles;

/* Structure for directory handles */
typedef struct _SharedHandle  
{
  gpointer monitor_handle;
  int      references;
} SharedHandle;



void 
_g_desktop_menu_monitor_init (void)
{
  /* Initialize hash tables */
  g_desktop_menu_monitor_item_handles = g_hash_table_new (NULL, NULL);
  g_desktop_menu_monitor_shared_handles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  g_desktop_menu_monitor_flags = G_DESKTOP_MENU_MONITOR_DIRECTORIES
                                 | G_DESKTOP_MENU_MONITOR_MENU_FILES
                                 | G_DESKTOP_MENU_MONITOR_DIRECTORY_FILES
                                 | G_DESKTOP_MENU_MONITOR_DESKTOP_FILES;
}



void
_g_desktop_menu_monitor_shutdown (void)
{
  g_hash_table_unref (g_desktop_menu_monitor_item_handles);
  g_hash_table_unref (g_desktop_menu_monitor_shared_handles);
}



/**
 * g_desktop_menu_monitor_set_vtable:
 * @vtable    : a #GDesktopMenuMonitorVTable
 * @user_data : custom pointer to be passed to the vtable functions.
 *
 * Sets the functions to be called when monitoring a file or directory
 * becomes necessary. See #GDesktopMenuMonitorVTable for more detailled
 * information. Be careful with redefining the #GDesktopMenuMonitorVTable 
 * (e.g. don't redefine it while there are any #GDesktopMenu's around).
 *
 * In order to change the user data, just pass the same 
 * #GDesktopMenuMonitorVTable as you did before. Pass NULL to clear the
 * vtable (e.g. if you want to disable monitoring support).
 */
void
g_desktop_menu_monitor_set_vtable (GDesktopMenuMonitorVTable *vtable,
                                   gpointer                   user_data)
{
  if (G_UNLIKELY (vtable == NULL))
    {
      g_desktop_menu_monitor_vtable.monitor_file = NULL;
      g_desktop_menu_monitor_vtable.monitor_directory = NULL;
      g_desktop_menu_monitor_vtable.remove_monitor = NULL;
    }
  else
    {
      g_desktop_menu_monitor_vtable.monitor_file = vtable->monitor_file;
      g_desktop_menu_monitor_vtable.monitor_directory = vtable->monitor_directory;
      g_desktop_menu_monitor_vtable.remove_monitor = vtable->remove_monitor;
    }

  g_desktop_menu_monitor_user_data = user_data;
}



gpointer
g_desktop_menu_monitor_add_item (GDesktopMenu     *menu,
                                 GDesktopMenuItem *item)
{
  gpointer monitor_handle;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  g_return_val_if_fail (G_IS_DESKTOP_MENU_ITEM (item), NULL);
  
  if (G_UNLIKELY (g_desktop_menu_monitor_vtable.monitor_file == NULL))
    return NULL;

  /* Request monitor handle from the library client */
  monitor_handle = g_desktop_menu_monitor_vtable.monitor_file (menu, g_desktop_menu_item_get_filename (item), 
                                                               g_desktop_menu_monitor_user_data);

  if (G_LIKELY (monitor_handle != NULL))
    {
      /* Store the item => handle pair in the hash table */
      g_hash_table_insert (g_desktop_menu_monitor_item_handles, item, monitor_handle);
    }

  return monitor_handle;
}



void
g_desktop_menu_monitor_remove_item (GDesktopMenu     *menu,
                                    GDesktopMenuItem *item)
{
  gpointer monitor_handle;

  g_return_if_fail (G_IS_DESKTOP_MENU_ITEM (item));
  
  if (G_UNLIKELY (g_desktop_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the monitor handle for this item */
  monitor_handle = g_hash_table_lookup (g_desktop_menu_monitor_item_handles, item);

  if (G_LIKELY (monitor_handle != NULL))
    {
      /* Remove monitor handle from the library client */
      g_desktop_menu_monitor_vtable.remove_monitor (menu, monitor_handle);

      /* ... and remove the item from the hash table */
      g_hash_table_remove (g_desktop_menu_monitor_item_handles, item);
    }
}



gpointer
g_desktop_menu_monitor_add_directory (GDesktopMenu *menu,
                                      const gchar  *directory)
{
  SharedHandle *shared_handle;
  gpointer         monitor_handle;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  g_return_val_if_fail (directory != NULL, NULL);
  
  if (G_UNLIKELY (g_desktop_menu_monitor_vtable.monitor_directory == NULL))
    return NULL;

  /* Load directory handle from the hash table */
  shared_handle = (SharedHandle *)g_hash_table_lookup (g_desktop_menu_monitor_shared_handles, 
                                                       directory);

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
      monitor_handle = g_desktop_menu_monitor_vtable.monitor_directory (menu, directory, 
                                                                        g_desktop_menu_monitor_user_data);

      if (G_LIKELY (monitor_handle != NULL))
        {
          /* Allocate new directory handle */
          shared_handle = g_new0 (SharedHandle, 1);

          /* Set values (reference counter starts with 1) */
          shared_handle->references = 1;
          shared_handle->monitor_handle = monitor_handle;

          /* Store the item => handle pair in the hash table */
          g_hash_table_insert (g_desktop_menu_monitor_shared_handles, 
                               g_strdup (directory), shared_handle);
        }
    }

  return monitor_handle;
}



void
g_desktop_menu_monitor_remove_directory (GDesktopMenu *menu,
                                         const gchar  *directory)
{
  SharedHandle *shared_handle;

  g_return_if_fail (directory != NULL);
  
  if (G_UNLIKELY (g_desktop_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the directory handle for this directory */
  shared_handle = g_hash_table_lookup (g_desktop_menu_monitor_shared_handles, directory);

  if (G_LIKELY (shared_handle != NULL))
    {
      /* Decrement the reference counter */
      shared_handle->references--;

      /* Check if there are no references left */
      if (G_UNLIKELY (shared_handle->references == 0)) 
        {
          /* Remove monitor handle from the library client */
          g_desktop_menu_monitor_vtable.remove_monitor (menu, shared_handle->monitor_handle);

          /* Remove directory handle from the hash table and destroy it */
          g_hash_table_remove (g_desktop_menu_monitor_shared_handles, directory);
        }
    }
}



gpointer
g_desktop_menu_monitor_add_file (GDesktopMenu *menu,
                                 const gchar  *filename)
{
  SharedHandle *shared_handle;
  gpointer      monitor_handle;

  g_return_val_if_fail (G_IS_DESKTOP_MENU (menu), NULL);
  g_return_val_if_fail (filename != NULL, NULL);
  
  if (G_UNLIKELY (g_desktop_menu_monitor_vtable.monitor_file == NULL))
    return NULL;

  /* Load filename handle from the hash table */
  shared_handle = (SharedHandle *)g_hash_table_lookup (g_desktop_menu_monitor_shared_handles, 
                                                       filename);

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
      monitor_handle = g_desktop_menu_monitor_vtable.monitor_file (menu, filename, 
                                                                   g_desktop_menu_monitor_user_data);

      if (G_LIKELY (monitor_handle != NULL))
        {
          /* Allocate new filename handle */
          shared_handle = g_new0 (SharedHandle, 1);

          /* Set values (reference counter starts with 1) */
          shared_handle->references = 1;
          shared_handle->monitor_handle = monitor_handle;

          /* Store the item => handle pair in the hash table */
          g_hash_table_insert (g_desktop_menu_monitor_shared_handles, 
                               g_strdup (filename), shared_handle);
        }
    }

  return monitor_handle;
}



void
g_desktop_menu_monitor_remove_file (GDesktopMenu *menu,
                                    const gchar  *filename)
{
  SharedHandle *shared_handle;

  g_return_if_fail (G_IS_DESKTOP_MENU (menu));
  g_return_if_fail (filename != NULL);
  
  if (G_UNLIKELY (g_desktop_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the filename handle for this file */
  shared_handle = g_hash_table_lookup (g_desktop_menu_monitor_shared_handles, filename);

  if (G_LIKELY (shared_handle != NULL))
    {
      /* Decrement the reference counter */
      shared_handle->references--;

      /* Check if there are no references left */
      if (G_UNLIKELY (shared_handle->references == 0)) 
        {
          /* Remove monitor handle from the library client */
          g_desktop_menu_monitor_vtable.remove_monitor (menu, shared_handle->monitor_handle);

          /* Remove filename handle from the hash table and destroy it */
          g_hash_table_remove (g_desktop_menu_monitor_shared_handles, shared_handle);
        }
    }
}



void 
g_desktop_menu_monitor_set_flags (GDesktopMenuMonitorFlags flags)
{
  g_desktop_menu_monitor_flags = flags;
}



GDesktopMenuMonitorFlags 
g_desktop_menu_monitor_get_flags (void)
{
  return g_desktop_menu_monitor_flags;
}



gboolean
g_desktop_menu_monitor_has_flags (GDesktopMenuMonitorFlags flags)
{
  return (g_desktop_menu_monitor_flags & flags) != 0;
}

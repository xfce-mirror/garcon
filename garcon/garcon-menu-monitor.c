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

#include <garcon/garcon-menu-monitor.h>
#include <garcon/garcon-menu-item.h>
#include <garcon/garcon.h>



/* Initial vtable configuration */
static GarconMenuMonitorVTable garcon_menu_monitor_vtable = {
  NULL,
  NULL,
  NULL,
};

/* Monitor flags */
static GarconMenuMonitorFlags garcon_menu_monitor_flags;

/* User data as provided by the client */
static gpointer               garcon_menu_monitor_user_data = NULL;

/* Hash table with (GarconMenuItem => gpointer) pairs */
static GHashTable            *garcon_menu_monitor_item_handles;

/* Hash table with (Directory => gpointer) pairs */
static GHashTable            *garcon_menu_monitor_shared_handles;

/* Structure for directory handles */
typedef struct _SharedHandle  
{
  gpointer monitor_handle;
  int      references;
} SharedHandle;



void 
_garcon_menu_monitor_init (void)
{
  /* Initialize hash tables */
  garcon_menu_monitor_item_handles = g_hash_table_new (NULL, NULL);
  garcon_menu_monitor_shared_handles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  garcon_menu_monitor_flags = GARCON_MENU_MONITOR_DIRECTORIES
                              | GARCON_MENU_MONITOR_MENU_FILES
                              | GARCON_MENU_MONITOR_DIRECTORY_FILES
                              | GARCON_MENU_MONITOR_DESKTOP_FILES;
}



void
_garcon_menu_monitor_shutdown (void)
{
  g_hash_table_unref (garcon_menu_monitor_item_handles);
  g_hash_table_unref (garcon_menu_monitor_shared_handles);
}



/**
 * garcon_menu_monitor_set_vtable:
 * @vtable    : a #GarconMenuMonitorVTable
 * @user_data : custom pointer to be passed to the vtable functions.
 *
 * Sets the functions to be called when monitoring a file or directory
 * becomes necessary. See #GarconMenuMonitorVTable for more detailled
 * information. Be careful with redefining the #GarconMenuMonitorVTable 
 * (e.g. don't redefine it while there are any #GarconMenu's around).
 *
 * In order to change the user data, just pass the same 
 * #GarconMenuMonitorVTable as you did before. Pass NULL to clear the
 * vtable (e.g. if you want to disable monitoring support).
 */
void
garcon_menu_monitor_set_vtable (GarconMenuMonitorVTable *vtable,
                                gpointer                 user_data)
{
  if (G_UNLIKELY (vtable == NULL))
    {
      garcon_menu_monitor_vtable.monitor_file = NULL;
      garcon_menu_monitor_vtable.monitor_directory = NULL;
      garcon_menu_monitor_vtable.remove_monitor = NULL;
    }
  else
    {
      garcon_menu_monitor_vtable.monitor_file = vtable->monitor_file;
      garcon_menu_monitor_vtable.monitor_directory = vtable->monitor_directory;
      garcon_menu_monitor_vtable.remove_monitor = vtable->remove_monitor;
    }

  garcon_menu_monitor_user_data = user_data;
}



gpointer
garcon_menu_monitor_add_item (GarconMenu     *menu,
                              GarconMenuItem *item)
{
  gpointer monitor_handle;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  g_return_val_if_fail (GARCON_IS_MENU_ITEM (item), NULL);
  
  if (G_UNLIKELY (garcon_menu_monitor_vtable.monitor_file == NULL))
    return NULL;

  /* Request monitor handle from the library client */
  monitor_handle = garcon_menu_monitor_vtable.monitor_file (menu, garcon_menu_item_get_filename (item), 
                                                            garcon_menu_monitor_user_data);

  if (G_LIKELY (monitor_handle != NULL))
    {
      /* Store the item => handle pair in the hash table */
      g_hash_table_insert (garcon_menu_monitor_item_handles, item, monitor_handle);
    }

  return monitor_handle;
}



void
garcon_menu_monitor_remove_item (GarconMenu     *menu,
                                 GarconMenuItem *item)
{
  gpointer monitor_handle;

  g_return_if_fail (GARCON_IS_MENU_ITEM (item));
  
  if (G_UNLIKELY (garcon_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the monitor handle for this item */
  monitor_handle = g_hash_table_lookup (garcon_menu_monitor_item_handles, item);

  if (G_LIKELY (monitor_handle != NULL))
    {
      /* Remove monitor handle from the library client */
      garcon_menu_monitor_vtable.remove_monitor (menu, monitor_handle);

      /* ... and remove the item from the hash table */
      g_hash_table_remove (garcon_menu_monitor_item_handles, item);
    }
}



gpointer
garcon_menu_monitor_add_directory (GarconMenu  *menu,
                                   const gchar *directory)
{
  SharedHandle *shared_handle;
  gpointer         monitor_handle;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  g_return_val_if_fail (directory != NULL, NULL);
  
  if (G_UNLIKELY (garcon_menu_monitor_vtable.monitor_directory == NULL))
    return NULL;

  /* Load directory handle from the hash table */
  shared_handle = (SharedHandle *)g_hash_table_lookup (garcon_menu_monitor_shared_handles, 
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
      monitor_handle = garcon_menu_monitor_vtable.monitor_directory (menu, directory, 
                                                                     garcon_menu_monitor_user_data);

      if (G_LIKELY (monitor_handle != NULL))
        {
          /* Allocate new directory handle */
          shared_handle = g_new0 (SharedHandle, 1);

          /* Set values (reference counter starts with 1) */
          shared_handle->references = 1;
          shared_handle->monitor_handle = monitor_handle;

          /* Store the item => handle pair in the hash table */
          g_hash_table_insert (garcon_menu_monitor_shared_handles, 
                               g_strdup (directory), shared_handle);
        }
    }

  return monitor_handle;
}



void
garcon_menu_monitor_remove_directory (GarconMenu  *menu,
                                      const gchar *directory)
{
  SharedHandle *shared_handle;

  g_return_if_fail (directory != NULL);
  
  if (G_UNLIKELY (garcon_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the directory handle for this directory */
  shared_handle = g_hash_table_lookup (garcon_menu_monitor_shared_handles, directory);

  if (G_LIKELY (shared_handle != NULL))
    {
      /* Decrement the reference counter */
      shared_handle->references--;

      /* Check if there are no references left */
      if (G_UNLIKELY (shared_handle->references == 0)) 
        {
          /* Remove monitor handle from the library client */
          garcon_menu_monitor_vtable.remove_monitor (menu, shared_handle->monitor_handle);

          /* Remove directory handle from the hash table and destroy it */
          g_hash_table_remove (garcon_menu_monitor_shared_handles, directory);
        }
    }
}



gpointer
garcon_menu_monitor_add_file (GarconMenu  *menu,
                              const gchar *filename)
{
  SharedHandle *shared_handle;
  gpointer      monitor_handle;

  g_return_val_if_fail (GARCON_IS_MENU (menu), NULL);
  g_return_val_if_fail (filename != NULL, NULL);
  
  if (G_UNLIKELY (garcon_menu_monitor_vtable.monitor_file == NULL))
    return NULL;

  /* Load filename handle from the hash table */
  shared_handle = (SharedHandle *)g_hash_table_lookup (garcon_menu_monitor_shared_handles, 
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
      monitor_handle = garcon_menu_monitor_vtable.monitor_file (menu, filename, 
                                                                garcon_menu_monitor_user_data);

      if (G_LIKELY (monitor_handle != NULL))
        {
          /* Allocate new filename handle */
          shared_handle = g_new0 (SharedHandle, 1);

          /* Set values (reference counter starts with 1) */
          shared_handle->references = 1;
          shared_handle->monitor_handle = monitor_handle;

          /* Store the item => handle pair in the hash table */
          g_hash_table_insert (garcon_menu_monitor_shared_handles, 
                               g_strdup (filename), shared_handle);
        }
    }

  return monitor_handle;
}



void
garcon_menu_monitor_remove_file (GarconMenu  *menu,
                                 const gchar *filename)
{
  SharedHandle *shared_handle;

  g_return_if_fail (GARCON_IS_MENU (menu));
  g_return_if_fail (filename != NULL);
  
  if (G_UNLIKELY (garcon_menu_monitor_vtable.remove_monitor == NULL))
    return;

  /* Lookup the filename handle for this file */
  shared_handle = g_hash_table_lookup (garcon_menu_monitor_shared_handles, filename);

  if (G_LIKELY (shared_handle != NULL))
    {
      /* Decrement the reference counter */
      shared_handle->references--;

      /* Check if there are no references left */
      if (G_UNLIKELY (shared_handle->references == 0)) 
        {
          /* Remove monitor handle from the library client */
          garcon_menu_monitor_vtable.remove_monitor (menu, shared_handle->monitor_handle);

          /* Remove filename handle from the hash table and destroy it */
          g_hash_table_remove (garcon_menu_monitor_shared_handles, shared_handle);
        }
    }
}



void 
garcon_menu_monitor_set_flags (GarconMenuMonitorFlags flags)
{
  garcon_menu_monitor_flags = flags;
}



GarconMenuMonitorFlags 
garcon_menu_monitor_get_flags (void)
{
  return garcon_menu_monitor_flags;
}



gboolean
garcon_menu_monitor_has_flags (GarconMenuMonitorFlags flags)
{
  return (garcon_menu_monitor_flags & flags) != 0;
}


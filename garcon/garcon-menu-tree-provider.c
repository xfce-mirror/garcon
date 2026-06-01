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

#include "garcon-menu-tree-provider.h"
#include "garcon-visibility.h"



G_DEFINE_INTERFACE (GarconMenuTreeProvider, garcon_menu_tree_provider, G_TYPE_OBJECT)



static void
garcon_menu_tree_provider_default_init (GarconMenuTreeProviderInterface *iface)
{
}



/**
 * garcon_menu_tree_provider_get_tree: (skip)
 * @provider: a #GarconMenuTreeProvider
 *
 * Returns: a #GNode
 */
GNode *
garcon_menu_tree_provider_get_tree (GarconMenuTreeProvider *provider)
{
  g_return_val_if_fail (GARCON_IS_MENU_TREE_PROVIDER (provider), NULL);
  return (*GARCON_MENU_TREE_PROVIDER_GET_IFACE (provider)->get_tree) (provider);
}


/**
 * garcon_menu_tree_provider_get_file:
 * @provider: a #GarconMenuTreeProvider
 *
 * Returns: (transfer full):
 */
GFile *
garcon_menu_tree_provider_get_file (GarconMenuTreeProvider *provider)
{
  g_return_val_if_fail (GARCON_IS_MENU_TREE_PROVIDER (provider), NULL);
  return (*GARCON_MENU_TREE_PROVIDER_GET_IFACE (provider)->get_file) (provider);
}

#define __GARCON_MENU_TREE_PROVIDER_C__
#include "garcon-visibility.c"

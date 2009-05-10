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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GARCON_H__
#define __GARCON_H__

#include <gio/gio.h>

#define GARCON_INSIDE_GARCON_H

#include <garcon/garcon-config.h>
#include <garcon/garconmenudirectory.h>
#include <garcon/garconmenuelement.h>
#include <garcon/garconenvironment.h>
#include <garcon/garconmenu.h>
#include <garcon/garconmenuitem.h>
#include <garcon/garconmenuitemcache.h>
#include <garcon/garconmenuitempool.h>
#include <garcon/garconmenunode.h>
#include <garcon/garconmenumerger.h>
#include <garcon/garconmenumonitor.h>
#include <garcon/garconmenuparser.h>
#include <garcon/garconmenuseparator.h>
#include <garcon/garconmenutreeprovider.h>

#undef GARCON_INSIDE_GARCON_H

G_BEGIN_DECLS

void   garcon_init          (const gchar *env);
void   garcon_shutdown      (void);

gchar *garcon_config_lookup (const gchar *filename) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#endif /* !__GARCON_H__ */

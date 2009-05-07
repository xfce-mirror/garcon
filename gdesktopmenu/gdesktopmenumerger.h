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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#if !defined (GDESKTOPMENU_INSIDE_GDESKTOPMENU_H) && !defined (GDESKTOPMENU_COMPILATION)
#error "Only <gdesktopmenu/gdesktopmenu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __G_DESKTOP_MENU_MERGER_H__
#define __G_DESKTOP_MENU_MERGER_H__

#include <glib-object.h>

#include "gdesktopmenutree-provider.h"

G_BEGIN_DECLS

#define G_TYPE_DESKTOP_MENU_MERGER            (g_desktop_menu_merger_get_type ())
#define G_DESKTOP_MENU_MERGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU_MERGER, GDesktopMenuMerger))
#define G_DESKTOP_MENU_MERGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DESKTOP_MENU_MERGER, GDesktopMenuMergerClass))
#define G_IS_DESKTOP_MENU_MERGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU_MERGER))
#define G_IS_DESKTOP_MENU_MERGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DESKTOP_MENU_MERGER)
#define G_DESKTOP_MENU_MERGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DESKTOP_MENU_MERGER, GDesktopMenuMergerClass))

typedef struct _GDesktopMenuMergerPrivate GDesktopMenuMergerPrivate;
typedef struct _GDesktopMenuMergerClass   GDesktopMenuMergerClass;
typedef struct _GDesktopMenuMerger        GDesktopMenuMerger;

GType               g_desktop_menu_merger_get_type (void) G_GNUC_CONST;

GDesktopMenuMerger *g_desktop_menu_merger_new      (GDesktopMenuTreeProvider *provider) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gboolean            g_desktop_menu_merger_run      (GDesktopMenuMerger       *merger,
                                                    GCancellable             *cancellable,
                                                    GError                  **error);



struct _GDesktopMenuMergerClass
{
  GObjectClass __parent__;
};

struct _GDesktopMenuMerger
{
  GObject                    __parent__;

  GDesktopMenuMergerPrivate *priv;
};

G_END_DECLS

#endif /* !__G_DESKTOP_MENU_MERGER_H__ */

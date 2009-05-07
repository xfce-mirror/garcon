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

#ifndef __G_DESKTOP_MENU_PARSER_H__
#define __G_DESKTOP_MENU_PARSER_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _GDesktopMenuParserPrivate GDesktopMenuParserPrivate;
typedef struct _GDesktopMenuParserClass   GDesktopMenuParserClass;
typedef struct _GDesktopMenuParser        GDesktopMenuParser;

#define G_TYPE_DESKTOP_MENU_PARSER            (g_desktop_menu_parser_get_type ())
#define G_DESKTOP_MENU_PARSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_DESKTOP_MENU_PARSER, GDesktopMenuParser))
#define G_DESKTOP_MENU_PARSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_TYPE_DESKTOP_MENU_PARSER, GDesktopMenuParserClass))
#define G_IS_DESKTOP_MENU_PARSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_DESKTOP_MENU_PARSER))
#define G_IS_DESKTOP_MENU_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_DESKTOP_MENU_PARSER)
#define G_DESKTOP_MENU_PARSER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_DESKTOP_MENU_PARSER, GDesktopMenuParserClass))

GType               g_desktop_menu_parser_get_type (void) G_GNUC_CONST;

GDesktopMenuParser *g_desktop_menu_parser_new      (GFile              *file) G_GNUC_MALLOC;
gboolean            g_desktop_menu_parser_run      (GDesktopMenuParser *parser,
                                                    GCancellable       *cancellable,
                                                    GError            **error);



struct _GDesktopMenuParserClass
{
  GObjectClass __parent__;
};

struct _GDesktopMenuParser
{
  GObject                    __parent__;

  GDesktopMenuParserPrivate *priv;
};

G_END_DECLS

#endif /* !__G_DESKTOP_MENU_PARSER_H__ */

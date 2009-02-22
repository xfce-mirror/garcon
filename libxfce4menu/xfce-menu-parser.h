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

#if !defined (LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H) && !defined (LIBXFCE4MENU_COMPILATION)
#error "Only <libxfce4menu/libxfce4menu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __XFCE_MENU_PARSER_H__
#define __XFCE_MENU_PARSER_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS;

typedef struct _XfceMenuParserPrivate XfceMenuParserPrivate;
typedef struct _XfceMenuParserClass   XfceMenuParserClass;
typedef struct _XfceMenuParser        XfceMenuParser;

#define XFCE_TYPE_MENU_PARSER            (xfce_menu_parser_get_type ())
#define XFCE_MENU_PARSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_PARSER, XfceMenuParser))
#define XFCE_MENU_PARSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU_PARSER, XfceMenuParserClass))
#define XFCE_IS_MENU_PARSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_PARSER))
#define XFCE_IS_MENU_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_MENU_PARSER)
#define XFCE_MENU_PARSER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU_PARSER, XfceMenuParserClass))

GType                  xfce_menu_parser_get_type         (void) G_GNUC_CONST;

XfceMenuParser        *xfce_menu_parser_new              (GFile                 *file) G_GNUC_MALLOC;
gboolean               xfce_menu_parser_run              (XfceMenuParser        *parser,
                                                          GCancellable          *cancellable,
                                                          GError               **error);



struct _XfceMenuParserClass
{
  GObjectClass __parent__;
};

struct _XfceMenuParser
{
  GObject __parent__;

  XfceMenuParserPrivate *priv;
};

G_END_DECLS;

#endif /* !__XFCE_MENU_PARSER_H__ */

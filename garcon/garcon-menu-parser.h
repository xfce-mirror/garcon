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

#if !defined(_GARCON_INSIDE_GARCON_H) && !defined(GARCON_COMPILATION)
#error "Only <garcon/garcon.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __GARCON_MENU_PARSER_H__
#define __GARCON_MENU_PARSER_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define GARCON_TYPE_MENU_PARSER (garcon_menu_parser_get_type ())
#define GARCON_MENU_PARSER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GARCON_TYPE_MENU_PARSER, GarconMenuParser))
#define GARCON_MENU_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GARCON_TYPE_MENU_PARSER, GarconMenuParserClass))
#define GARCON_IS_MENU_PARSER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GARCON_TYPE_MENU_PARSER))
#define GARCON_IS_MENU_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GARCON_TYPE_MENU_PARSER)
#define GARCON_MENU_PARSER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GARCON_TYPE_MENU_PARSER, GarconMenuParserClass))

typedef struct _GarconMenuParserPrivate GarconMenuParserPrivate;
typedef struct _GarconMenuParserClass GarconMenuParserClass;
typedef struct _GarconMenuParser GarconMenuParser;

GType
garcon_menu_parser_get_type (void) G_GNUC_CONST;

GarconMenuParser *
garcon_menu_parser_new (GFile *file) G_GNUC_MALLOC;
gboolean
garcon_menu_parser_run (GarconMenuParser *parser,
                        GCancellable *cancellable,
                        GError **error);



struct _GarconMenuParserClass
{
  GObjectClass __parent__;
};

struct _GarconMenuParser
{
  GObject __parent__;

  GarconMenuParserPrivate *priv;
};

G_END_DECLS

#endif /* !__GARCON_MENU_PARSER_H__ */

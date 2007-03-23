/* $Id$ */
/* vim:set et ai sw=2 sts=2: */
/*-
 * Copyright (c) 2007 Jannis Pohlmann <jannis@xfce.org>
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

#include <libxfce4menu/xfce-menu-separator.h>



static void               xfce_menu_separator_class_init (XfceMenuSeparatorClass *klass);
static void               xfce_menu_separator_init       (XfceMenuSeparator      *separator);
static void               xfce_menu_separator_finalize   (GObject                *object);



static XfceMenuSeparator *_xfce_menu_separator = NULL;



void
_xfce_menu_separator_init (void)
{
  if (G_LIKELY (_xfce_menu_separator == NULL))
    _xfce_menu_separator = g_object_new (XFCE_TYPE_MENU_SEPARATOR, NULL);
}



void
_xfce_menu_separator_shutdown (void)
{
  if (G_LIKELY (_xfce_menu_separator != NULL))
    g_object_unref (G_OBJECT (_xfce_menu_separator));
}



struct _XfceMenuSeparatorClass
{
  GObjectClass __parent__;
};

struct _XfceMenuSeparator
{
  GObject __parent__;
};



static GObjectClass *xfce_menu_separator_parent_class = NULL;



GType
xfce_menu_separator_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfceMenuSeparatorClass),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_separator_class_init,
        NULL,
        NULL,
        sizeof (XfceMenuSeparator),
        0,
        (GInstanceInitFunc) xfce_menu_separator_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMenuSeparator", &info, 0);
    }

  return type;
}



static void
xfce_menu_separator_class_init (XfceMenuSeparatorClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_menu_separator_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_separator_finalize;
}



static void
xfce_menu_separator_init (XfceMenuSeparator *separator)
{
}



static void
xfce_menu_separator_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_menu_separator_parent_class)->finalize) (object);
}



XfceMenuSeparator*
xfce_menu_separator_get_default (void)
{
  return _xfce_menu_separator;
}

/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2007-2009 Jannis Pohlmann <jannis@xfce.org>
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

#include <gdesktopmenu/gdesktopmenuelement.h>
#include <gdesktopmenu/gdesktopmenuseparator.h>



static void         g_desktop_menu_separator_class_init                     (GDesktopMenuSeparatorClass *klass);
static void         g_desktop_menu_separator_element_init                   (GDesktopMenuElementIface   *iface);
static void         g_desktop_menu_separator_init                           (GDesktopMenuSeparator      *separator);
static void         g_desktop_menu_separator_finalize                       (GObject                    *object);

static const gchar *g_desktop_menu_separator_get_element_name                (GDesktopMenuElement       *element);
static const gchar *g_desktop_menu_separator_get_element_comment             (GDesktopMenuElement       *element);
static const gchar *g_desktop_menu_separator_get_element_icon_name           (GDesktopMenuElement       *element);
static gboolean     g_desktop_menu_separator_get_element_visible             (GDesktopMenuElement       *element);
static gboolean     g_desktop_menu_separator_get_element_show_in_environment (GDesktopMenuElement       *element);
static gboolean     g_desktop_menu_separator_get_element_no_display          (GDesktopMenuElement       *element);


static GDesktopMenuSeparator *_g_desktop_menu_separator = NULL;



void
_g_desktop_menu_separator_init (void)
{
  if (G_LIKELY (_g_desktop_menu_separator == NULL))
    {
      _g_desktop_menu_separator = g_object_new (G_TYPE_DESKTOP_MENU_SEPARATOR, NULL);
      g_object_add_weak_pointer (G_OBJECT (_g_desktop_menu_separator), 
                                 (gpointer) &_g_desktop_menu_separator);
    }
}



void
_g_desktop_menu_separator_shutdown (void)
{
  if (G_LIKELY (_g_desktop_menu_separator != NULL))
    g_object_unref (G_OBJECT (_g_desktop_menu_separator));
}



struct _GDesktopMenuSeparatorClass
{
  GObjectClass __parent__;
};

struct _GDesktopMenuSeparator
{
  GObject __parent__;


};



static GObjectClass *g_desktop_menu_separator_parent_class = NULL;



GType
g_desktop_menu_separator_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GInterfaceInfo element_info =
      {
        (GInterfaceInitFunc) g_desktop_menu_separator_element_init,
        NULL,
        NULL,
      };

      type = g_type_register_static_simple (G_TYPE_OBJECT,
                                            "GDesktopMenuSeparator",
                                            sizeof (GDesktopMenuSeparatorClass),
                                            (GClassInitFunc) g_desktop_menu_separator_class_init,
                                            sizeof (GDesktopMenuSeparator),
                                            (GInstanceInitFunc) g_desktop_menu_separator_init,
                                            0);

      g_type_add_interface_static (type, G_TYPE_DESKTOP_MENU_ELEMENT, &element_info);
    }

  return type;
}



static void
g_desktop_menu_separator_class_init (GDesktopMenuSeparatorClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  g_desktop_menu_separator_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = g_desktop_menu_separator_finalize;
}



static void
g_desktop_menu_separator_element_init (GDesktopMenuElementIface *iface)
{
  iface->get_name = g_desktop_menu_separator_get_element_name;
  iface->get_comment = g_desktop_menu_separator_get_element_comment;
  iface->get_icon_name = g_desktop_menu_separator_get_element_icon_name;
  iface->get_visible = g_desktop_menu_separator_get_element_visible;
  iface->get_show_in_environment = g_desktop_menu_separator_get_element_show_in_environment;
  iface->get_no_display = g_desktop_menu_separator_get_element_no_display;
}



static void
g_desktop_menu_separator_init (GDesktopMenuSeparator *separator)
{
}



static void
g_desktop_menu_separator_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (g_desktop_menu_separator_parent_class)->finalize) (object);
}



GDesktopMenuSeparator*
g_desktop_menu_separator_get_default (void)
{
  return _g_desktop_menu_separator;
}



static const gchar*
g_desktop_menu_separator_get_element_name (GDesktopMenuElement *element)
{
  return NULL;
}



static const gchar*
g_desktop_menu_separator_get_element_comment (GDesktopMenuElement *element)
{
  return NULL;
}



static const gchar*
g_desktop_menu_separator_get_element_icon_name (GDesktopMenuElement *element)
{
  return NULL;
}



static gboolean
g_desktop_menu_separator_get_element_visible (GDesktopMenuElement *element)
{
  return TRUE;
}



static gboolean
g_desktop_menu_separator_get_element_show_in_environment (GDesktopMenuElement *element)
{
  return TRUE;
}



static gboolean
g_desktop_menu_separator_get_element_no_display (GDesktopMenuElement *element)
{
  return FALSE;
}


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

#include <garcon/garcon-menu-element.h>
#include <garcon/garcon-menu-separator.h>



static void         garcon_menu_separator_element_init                    (GarconMenuElementIface   *iface);
static void         garcon_menu_separator_finalize                        (GObject                  *object);

static const gchar *garcon_menu_separator_get_element_name                (GarconMenuElement        *element);
static const gchar *garcon_menu_separator_get_element_comment             (GarconMenuElement        *element);
static const gchar *garcon_menu_separator_get_element_icon_name           (GarconMenuElement        *element);
static gboolean     garcon_menu_separator_get_element_visible             (GarconMenuElement        *element);
static gboolean     garcon_menu_separator_get_element_show_in_environment (GarconMenuElement        *element);
static gboolean     garcon_menu_separator_get_element_no_display          (GarconMenuElement        *element);


static GarconMenuSeparator *_garcon_menu_separator = NULL;



void
_garcon_menu_separator_init (void)
{
  if (G_LIKELY (_garcon_menu_separator == NULL))
    {
      _garcon_menu_separator = g_object_new (GARCON_TYPE_MENU_SEPARATOR, NULL);
      g_object_add_weak_pointer (G_OBJECT (_garcon_menu_separator), 
                                 (gpointer) &_garcon_menu_separator);
    }
}



void
_garcon_menu_separator_shutdown (void)
{
  if (G_LIKELY (_garcon_menu_separator != NULL))
    g_object_unref (G_OBJECT (_garcon_menu_separator));
}



struct _GarconMenuSeparatorClass
{
  GObjectClass __parent__;
};

struct _GarconMenuSeparator
{
  GObject __parent__;


};



G_DEFINE_TYPE_WITH_CODE (GarconMenuSeparator, garcon_menu_separator, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (GARCON_TYPE_MENU_ELEMENT, garcon_menu_separator_element_init))



static void
garcon_menu_separator_class_init (GarconMenuSeparatorClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  garcon_menu_separator_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = garcon_menu_separator_finalize;
}



static void
garcon_menu_separator_element_init (GarconMenuElementIface *iface)
{
  iface->get_name = garcon_menu_separator_get_element_name;
  iface->get_comment = garcon_menu_separator_get_element_comment;
  iface->get_icon_name = garcon_menu_separator_get_element_icon_name;
  iface->get_visible = garcon_menu_separator_get_element_visible;
  iface->get_show_in_environment = garcon_menu_separator_get_element_show_in_environment;
  iface->get_no_display = garcon_menu_separator_get_element_no_display;
}



static void
garcon_menu_separator_init (GarconMenuSeparator *separator)
{
}



static void
garcon_menu_separator_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (garcon_menu_separator_parent_class)->finalize) (object);
}



GarconMenuSeparator*
garcon_menu_separator_get_default (void)
{
  return _garcon_menu_separator;
}



static const gchar*
garcon_menu_separator_get_element_name (GarconMenuElement *element)
{
  return NULL;
}



static const gchar*
garcon_menu_separator_get_element_comment (GarconMenuElement *element)
{
  return NULL;
}



static const gchar*
garcon_menu_separator_get_element_icon_name (GarconMenuElement *element)
{
  return NULL;
}



static gboolean
garcon_menu_separator_get_element_visible (GarconMenuElement *element)
{
  return TRUE;
}



static gboolean
garcon_menu_separator_get_element_show_in_environment (GarconMenuElement *element)
{
  return TRUE;
}



static gboolean
garcon_menu_separator_get_element_no_display (GarconMenuElement *element)
{
  return FALSE;
}


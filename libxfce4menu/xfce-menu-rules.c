/* $Id$ */
/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2006-2007 Jannis Pohlmann <jannis@xfce.org>
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

#include <libxfce4menu/xfce-menu-item.h>
#include <libxfce4menu/xfce-menu-rules.h>



static void xfce_menu_rules_class_init (gpointer klass);



GType
xfce_menu_rules_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfceMenuRulesIface),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_rules_class_init,
        NULL,
        NULL,
        0,
        0,
        NULL,
      };

      type = g_type_register_static (G_TYPE_INTERFACE, "XfceMenuRules", &info, 0);
      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
    }
  
  return type;
}



static void
xfce_menu_rules_class_init (gpointer klass)
{
}



gboolean
xfce_menu_rules_match (XfceMenuRules *rules,
                       XfceMenuItem  *item)
{
  g_return_val_if_fail (XFCE_IS_MENU_RULES (rules), FALSE);
  return (*XFCE_MENU_RULES_GET_IFACE (rules)->match) (rules, item);
}



void
xfce_menu_rules_add_rules (XfceMenuRules *rules,
                           XfceMenuRules *additional_rules)
{
  g_return_if_fail (XFCE_IS_MENU_RULES (rules));
  g_return_if_fail (XFCE_IS_MENU_RULES (additional_rules));
  (*XFCE_MENU_RULES_GET_IFACE (rules)->add_rules) (rules, additional_rules);
}



void xfce_menu_rules_add_all (XfceMenuRules *rules)
{
  g_return_if_fail (XFCE_IS_MENU_RULES (rules));
  (*XFCE_MENU_RULES_GET_IFACE (rules)->add_all) (rules);
}



void
xfce_menu_rules_add_filename (XfceMenuRules *rules,
                              const gchar   *filename)
{
  g_return_if_fail (XFCE_IS_MENU_RULES (rules));
  (*XFCE_MENU_RULES_GET_IFACE (rules)->add_filename) (rules, filename);
}


void
xfce_menu_rules_add_category (XfceMenuRules *rules,
                              const gchar   *category)
{
  g_return_if_fail (XFCE_IS_MENU_RULES (rules));
  (*XFCE_MENU_RULES_GET_IFACE (rules)->add_category) (rules, category);
}

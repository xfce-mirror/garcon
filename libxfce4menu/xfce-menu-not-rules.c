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
#include <libxfce4menu/xfce-menu-standard-rules.h>
#include <libxfce4menu/xfce-menu-not-rules.h>



static void     xfce_menu_not_rules_class_init     (XfceMenuNotRulesClass  *klass);
static void     xfce_menu_not_rules_init           (XfceMenuNotRules       *rules);
static void     xfce_menu_not_rules_finalize       (GObject               *object);
static gboolean xfce_menu_not_rules_match          (XfceMenuStandardRules *rules,
                                                    XfceMenuItem          *item);



struct _XfceMenuNotRulesClass
{
  XfceMenuStandardRulesClass __parent__;
};

struct _XfceMenuNotRules
{
  XfceMenuStandardRules __parent__;
};



static GObjectClass *xfce_menu_not_rules_parent_class = NULL;



GType
xfce_menu_not_rules_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfceMenuNotRulesClass),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_not_rules_class_init,
        NULL,
        NULL,
        sizeof (XfceMenuNotRules),
        0,
        (GInstanceInitFunc) xfce_menu_not_rules_init,
        NULL,
      };

      type = g_type_register_static (XFCE_TYPE_MENU_STANDARD_RULES, "XfceMenuNotRules", &info, 0);
    }

  return type;
}



static void
xfce_menu_not_rules_class_init (XfceMenuNotRulesClass *klass)
{
  GObjectClass *gobject_class;
  XfceMenuStandardRulesClass *std_rules_class;

  /* Determine the parent type class */
  xfce_menu_not_rules_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_not_rules_finalize;

  std_rules_class = XFCE_MENU_STANDARD_RULES_CLASS (klass);
  std_rules_class->match_item = xfce_menu_not_rules_match;
}



static void
xfce_menu_not_rules_init (XfceMenuNotRules *rules)
{
}



static void
xfce_menu_not_rules_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_menu_not_rules_parent_class)->finalize) (object); 
}



XfceMenuNotRules*
xfce_menu_not_rules_new (void)
{
  return g_object_new (XFCE_TYPE_MENU_NOT_RULES, NULL);
}



static gboolean
xfce_menu_not_rules_match (XfceMenuStandardRules *rules,
                           XfceMenuItem          *item)
{
  GList *iter;

  g_return_val_if_fail (XFCE_IS_MENU_STANDARD_RULES (rules), FALSE);
  g_return_val_if_fail (XFCE_IS_MENU_ITEM (item), FALSE);

  if (rules->all)
    return FALSE;

  /* Compare desktop id's */
  for (iter = rules->filenames; iter != NULL; iter = g_list_next (iter))
    {
      if (g_utf8_collate (xfce_menu_item_get_desktop_id (item), iter->data) == 0)
        return FALSE;
    }

  for (iter = rules->categories; iter != NULL; iter = g_list_next (iter))
    {
      if (g_list_find_custom (xfce_menu_item_get_categories (item), iter->data, (GCompareFunc) g_utf8_collate))
        return FALSE;
    }

  /* Match item against nested rules */
  for (iter = g_list_first (rules->rules); iter != NULL; iter = g_list_next (iter))
    {
      if (xfce_menu_rules_match (XFCE_MENU_RULES (iter->data), item))
        return FALSE;
    }

  return TRUE;
}

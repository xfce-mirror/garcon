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

#include <libxfce4util/libxfce4util.h>

#include <libxfce4menu/xfce-menu-item.h>
#include <libxfce4menu/xfce-menu-rules.h>
#include <libxfce4menu/xfce-menu-standard-rules.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_INCLUDE,
};



static void     xfce_menu_standard_rules_class_init     (XfceMenuStandardRulesClass *klass);
static void     xfce_menu_standard_rules_rules_init     (XfceMenuRulesIface         *iface);
static void     xfce_menu_standard_rules_init           (XfceMenuStandardRules      *rules);
static void     xfce_menu_standard_rules_finalize       (GObject                    *object);
static void     xfce_menu_standard_rules_get_property   (GObject                    *object,
                                                         guint                       prop_id,
                                                         GValue                     *value,
                                                         GParamSpec                 *pspec);
static void     xfce_menu_standard_rules_set_property   (GObject                    *object,
                                                         guint                       prop_id,
                                                         const GValue               *value,
                                                         GParamSpec                 *pspec);
static gboolean xfce_menu_standard_rules_match          (XfceMenuRules              *rules,
                                                         XfceMenuItem               *item);
static gboolean xfce_menu_standard_rules_match_item     (XfceMenuStandardRules      *rules,
                                                         XfceMenuItem               *item);
static void     xfce_menu_standard_rules_add_rules      (XfceMenuRules              *rules,
                                                         XfceMenuRules              *additional_rules);
static void     xfce_menu_standard_rules_add_all        (XfceMenuRules              *rules);
static void     xfce_menu_standard_rules_add_filename   (XfceMenuRules              *rules,
                                                         const gchar                *filename);
static void     xfce_menu_standard_rules_add_category   (XfceMenuRules              *rules,
                                                         const gchar                *category);


static GObjectClass *xfce_menu_standard_rules_parent_class = NULL;



GType
xfce_menu_standard_rules_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (XfceMenuStandardRulesClass),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_standard_rules_class_init,
        NULL,
        NULL,
        sizeof (XfceMenuStandardRules),
        0,
        (GInstanceInitFunc) xfce_menu_standard_rules_init,
        NULL,
      };

      static const GInterfaceInfo rules_info = 
      {
        (GInterfaceInitFunc) xfce_menu_standard_rules_rules_init,
        NULL,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMenuStandardRules", &info, G_TYPE_FLAG_ABSTRACT);
      g_type_add_interface_static (type, XFCE_TYPE_MENU_RULES, &rules_info);
    }

  return type;
}



static void
xfce_menu_standard_rules_class_init (XfceMenuStandardRulesClass *klass)
{
  GObjectClass *gobject_class;

  xfce_menu_standard_rules_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_standard_rules_finalize;
  gobject_class->get_property = xfce_menu_standard_rules_get_property;
  gobject_class->set_property = xfce_menu_standard_rules_set_property;

  klass->match_item = xfce_menu_standard_rules_match_item;

  /**
   * XfceMenuStandardRules:include:
   *
   * Whether this rule set shall be treated as an include or exclude element.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_INCLUDE,
                                   g_param_spec_boolean ("include",
                                                         _("Include"),
                                                         _("Treat element as include or exclude element"),
                                                         TRUE,
                                                         G_PARAM_READWRITE));

  /* Overwrite XfceMenuRules's properties */
#if 0
  g_object_class_override_property (gobject_class, PROP_INCLUDE, "include");
#endif
}



static void
xfce_menu_standard_rules_rules_init (XfceMenuRulesIface *iface)
{
  iface->add_rules = xfce_menu_standard_rules_add_rules;
  iface->add_all = xfce_menu_standard_rules_add_all;
  iface->add_filename = xfce_menu_standard_rules_add_filename;
  iface->add_category = xfce_menu_standard_rules_add_category;
  iface->match = xfce_menu_standard_rules_match;
}
  


static void
xfce_menu_standard_rules_init (XfceMenuStandardRules *rules)
{
  rules->all = FALSE;
  rules->rules = NULL;
  rules->filenames = NULL;
  rules->categories = NULL; 
  rules->include = TRUE;
}



static void
xfce_menu_standard_rules_finalize (GObject *object)
{
  XfceMenuStandardRules *rules = XFCE_MENU_STANDARD_RULES (object);

  g_list_foreach (rules->rules, (GFunc) g_object_unref, NULL);
  g_list_free (rules->rules);

  g_list_foreach (rules->filenames, (GFunc) g_free, NULL);
  g_list_free (rules->filenames);

  g_list_foreach (rules->categories, (GFunc) g_free, NULL);
  g_list_free (rules->categories);

  (*G_OBJECT_CLASS (xfce_menu_standard_rules_parent_class)->finalize) (object); 
}



static void
xfce_menu_standard_rules_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  XfceMenuStandardRules *rules = XFCE_MENU_STANDARD_RULES (object);

  switch (prop_id)
    {
    case PROP_INCLUDE:
      g_value_set_boolean (value, xfce_menu_standard_rules_get_include (rules));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_menu_standard_rules_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  XfceMenuStandardRules *rules = XFCE_MENU_STANDARD_RULES (object);

  switch (prop_id)
    {
    case PROP_INCLUDE:
      xfce_menu_standard_rules_set_include (rules, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_menu_standard_rules_add_rules (XfceMenuRules *rules,
                                    XfceMenuRules *additional_rules)
{
  XfceMenuStandardRules *std_rules = XFCE_MENU_STANDARD_RULES (rules);

  g_return_if_fail (XFCE_IS_MENU_RULES (rules));
  g_return_if_fail (XFCE_IS_MENU_RULES (additional_rules));

  /* Remove floating reference (if any) and request a normal one */
#if GLIB_CHECK_VERSION(2,10,0)
  g_object_ref_sink (G_OBJECT (additional_rules));
#else
  g_object_ref (G_OBJECT (additional_rules));
#endif

  /* Append rules to the list */
  std_rules->rules = g_list_append (std_rules->rules, additional_rules);
}



static void
xfce_menu_standard_rules_add_all (XfceMenuRules *rules)
{
  XfceMenuStandardRules *std_rules = XFCE_MENU_STANDARD_RULES (rules);

  g_return_if_fail (XFCE_IS_MENU_RULES (rules));

  std_rules->all = TRUE;
}



static void
xfce_menu_standard_rules_add_filename (XfceMenuRules *rules,
                                       const gchar   *filename)
{
  XfceMenuStandardRules *std_rules = XFCE_MENU_STANDARD_RULES (rules);

  g_return_if_fail (XFCE_IS_MENU_RULES (rules));
  g_return_if_fail (filename != NULL);

  /* Append filename to the list */
  std_rules->filenames = g_list_append (std_rules->filenames, g_strdup (filename));
}



static void
xfce_menu_standard_rules_add_category (XfceMenuRules *rules,
                                       const gchar   *category)
{
  XfceMenuStandardRules *std_rules = XFCE_MENU_STANDARD_RULES (rules);

  g_return_if_fail (XFCE_IS_MENU_RULES (rules));
  g_return_if_fail (category != NULL);

  /* Append category to the list (if not yet included) */
  std_rules->categories = g_list_append (std_rules->categories, g_strdup (category));
}


static gboolean
xfce_menu_standard_rules_match (XfceMenuRules *rules,
                                XfceMenuItem  *item)
{
  XfceMenuStandardRules *std_rules = XFCE_MENU_STANDARD_RULES (rules);

  g_return_val_if_fail (XFCE_IS_MENU_STANDARD_RULES (std_rules), FALSE);
  g_return_val_if_fail (XFCE_IS_MENU_ITEM (item), FALSE);

  return (*XFCE_MENU_STANDARD_RULES_GET_CLASS (std_rules)->match_item) (std_rules, item);
}



static gboolean
xfce_menu_standard_rules_match_item (XfceMenuStandardRules *rules,
                                     XfceMenuItem          *item)
{
  g_return_val_if_fail (XFCE_IS_MENU_STANDARD_RULES (rules), FALSE);
  g_return_val_if_fail (XFCE_IS_MENU_ITEM (item), FALSE);

  return FALSE;
}



gboolean 
xfce_menu_standard_rules_get_include (XfceMenuStandardRules *rules)
{
  g_return_val_if_fail (XFCE_IS_MENU_STANDARD_RULES (rules), TRUE);
  return rules->include;
}



void xfce_menu_standard_rules_set_include (XfceMenuStandardRules *rules,
                                           gboolean               include)
{
  g_return_if_fail (XFCE_IS_MENU_STANDARD_RULES (rules));

  /* Do nothing if values are equal */
  if (rules->include == include)
    return;

  /* Assign new value */
  rules->include = include;

  /* Notify listeners */
  g_object_notify (G_OBJECT (rules), "include");
}

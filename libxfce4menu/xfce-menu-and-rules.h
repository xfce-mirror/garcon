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

#if !defined(LIBXFCE4MENU_INSIDE_LIBXFCE4MENU_H) && !defined(LIBXFCE4MENU_COMPILATION)
#error "Only <libxfce4menu/libxfce4menu.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __XFCE_MENU_AND_RULES_H__
#define __XFCE_MENU_AND_RULES_H__

#include <glib-object.h>

typedef struct _XfceMenuAndRules        XfceMenuAndRules;
typedef struct _XfceMenuAndRulesClass   XfceMenuAndRulesClass;

#define XFCE_TYPE_MENU_AND_RULES             (xfce_menu_and_rules_get_type ())
#define XFCE_MENU_AND_RULES(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_MENU_AND_RULES, XfceMenuAndRules))
#define XFCE_MENU_AND_RULES_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_MENU_AND_RULES, XfceMenuAndRulesClass))
#define XFCE_IS_MENU_AND_RULES(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_MENU_AND_RULES))
#define XFCE_IS_MENU_AND_RULES_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), XFCE_TYPE_MENU_AND_RULES))
#define XFCE_MENU_AND_RULES_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_MENU_AND_RULES, XfceMenuAndRulesClass))

GType             xfce_menu_and_rules_get_type (void) G_GNUC_CONST;

XfceMenuAndRules *xfce_menu_and_rules_new      (void);

G_END_DECLS;

#endif /* !__XFCE_MENU_AND_RULES_H__ */

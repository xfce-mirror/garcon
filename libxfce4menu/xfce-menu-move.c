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

#include <libxfce4menu/xfce-menu-move.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_OLD,
  PROP_NEW,
};



static void xfce_menu_move_class_init   (XfceMenuMoveClass *klass);
static void xfce_menu_move_init         (XfceMenuMove      *move);
static void xfce_menu_move_finalize     (GObject           *object);
static void xfce_menu_move_get_property (GObject           *object,
                                         guint              prop_id,
                                         GValue            *value,
                                         GParamSpec        *pspec);
static void xfce_menu_move_set_property (GObject           *object,
                                         guint              prop_id,
                                         const GValue      *value,
                                         GParamSpec        *pspec);



struct _XfceMenuMoveClass
{
  GObjectClass __parent__;
};

struct _XfceMenuMove
{
  GObject  __parent__;

  /* Name of the submenu to move/rename */
  gchar   *old;

  /* Name of the target path/name */
  gchar   *new;
};



static GObjectClass *xfce_menu_move_parent_class = NULL;



GType
xfce_menu_move_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
      {
        sizeof (XfceMenuMoveClass),
        NULL,
        NULL,
        (GClassInitFunc) xfce_menu_move_class_init,
        NULL,
        NULL,
        sizeof (XfceMenuMove),
        0,
        (GInstanceInitFunc) xfce_menu_move_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, "XfceMenuMove", &info, 0);
    }

  return type;
}



static void
xfce_menu_move_class_init (XfceMenuMoveClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_menu_move_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_menu_move_finalize;
  gobject_class->get_property = xfce_menu_move_get_property;
  gobject_class->set_property = xfce_menu_move_set_property;

  /**
   * XfceMenuMove:old:
   *
   * Name of the submenu to be moved/renamed.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_OLD,
                                   g_param_spec_string ("old",
                                                        _("Old name"),
                                                        _("Name of the submenu to be moved"),
                                                        NULL,
                                                        G_PARAM_READWRITE));

  /**
   * XfceMenuMove:new:
   *
   * Target path/name of the move/rename operation.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_NEW,
                                   g_param_spec_string ("new",
                                                        _("New name"),
                                                        _("Target path/name of the move/rename operation"),
                                                        NULL,
                                                        G_PARAM_READWRITE));
}



static void
xfce_menu_move_init (XfceMenuMove *move)
{
  move->old = NULL;
  move->new = NULL;
}



static void
xfce_menu_move_finalize (GObject *object)
{
  XfceMenuMove *move = XFCE_MENU_MOVE (object);

  /* Free instance variables */
  xfce_menu_move_set_old (move, NULL);
  xfce_menu_move_set_new (move, NULL);

  (*G_OBJECT_CLASS (xfce_menu_move_parent_class)->finalize) (object);
}



static void
xfce_menu_move_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  XfceMenuMove *move = XFCE_MENU_MOVE (object);

  switch (prop_id)
    {
    case PROP_OLD:
      g_value_set_string (value, xfce_menu_move_get_old (move));
      break;

    case PROP_NEW:
      g_value_set_string (value, xfce_menu_move_get_new (move));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_menu_move_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  XfceMenuMove *move = XFCE_MENU_MOVE (object);

  switch (prop_id)
    {
    case PROP_OLD:
      xfce_menu_move_set_old (move, g_value_get_string (value));
      break;

    case PROP_NEW:
      xfce_menu_move_set_new (move, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



XfceMenuMove*
xfce_menu_move_new (void)
{
  return g_object_new (XFCE_TYPE_MENU_MOVE, NULL);
}



const gchar*
xfce_menu_move_get_old (XfceMenuMove *move)
{
  g_return_val_if_fail (XFCE_IS_MENU_MOVE (move), NULL);
  return move->old;
}



void
xfce_menu_move_set_old (XfceMenuMove *move, 
                        const gchar  *old)
{
  g_return_if_fail (XFCE_IS_MENU_MOVE (move));

  /* Check if old value is set */
  if (G_UNLIKELY (move->old != NULL))
    {
      /* Abort if old and new value is the same */
      if (G_UNLIKELY (old != NULL && g_utf8_collate (move->old, old) == 0))
        return;

      /* Otherwise, free the old value */
      g_free (move->old);
    }

  /* Assign the new value */
  move->old = g_strdup (old);
}



const gchar*
xfce_menu_move_get_new (XfceMenuMove *move)
{
  g_return_val_if_fail (XFCE_IS_MENU_MOVE (move), NULL);
  return move->new;
}



void
xfce_menu_move_set_new (XfceMenuMove *move, 
                        const gchar  *new)
{
  g_return_if_fail (XFCE_IS_MENU_MOVE (move));

  /* Check if old value is set */
  if (G_UNLIKELY (move->new != NULL))
    {
      /* Abort if old and new value is the same */
      if (G_UNLIKELY (new != NULL && g_utf8_collate (move->new, new) == 0))
        return;

      /* Otherwise, free the old value */
      g_free (move->new);
    }

  /* Assign the new value */
  move->new = g_strdup (new);
}

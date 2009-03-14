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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib.h>

#include <libxfce4menu/libxfce4menu.h>



#define FILENAME "/etc/xdg/menus/applications.menu"



static gboolean print_node (GNode *node, 
                            gint   depth);



const gchar *
node_name (GNode *node)
{
  switch (xfce_menu_node_tree_get_node_type (node))
    {
      case XFCE_MENU_NODE_TYPE_MENU: return "Menu"; break;
      case XFCE_MENU_NODE_TYPE_INCLUDE: return "Include"; break;
      case XFCE_MENU_NODE_TYPE_EXCLUDE: return "Exclude"; break;
      case XFCE_MENU_NODE_TYPE_OR: return "Or"; break;
      case XFCE_MENU_NODE_TYPE_AND: return "And"; break;
      case XFCE_MENU_NODE_TYPE_NOT: return "Not"; break;
      case XFCE_MENU_NODE_TYPE_MOVE: return "Move"; break;
      case XFCE_MENU_NODE_TYPE_LAYOUT: return "Layout"; break;
      default: return NULL; break;
    }
}



static void
print_child_nodes (GNode *node,
                   gint   depth)
{
  GNode *child;

  for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
    {
      g_node_traverse (child, G_PRE_ORDER, G_TRAVERSE_ALL, 1, 
                       (GNodeTraverseFunc) print_node, GINT_TO_POINTER (depth+2));
    }
}



static gboolean
print_node (GNode *node,
            gint   depth)
{
  gint i;

#define INDENT {for (i = 0; i < depth; ++i) g_print (" ");}

  if (G_UNLIKELY (xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_MENU ||
                  xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_INCLUDE ||
                  xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_EXCLUDE ||
                  xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_OR ||
                  xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_AND ||
                  xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_NOT ||
                  xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_MOVE ||
                  xfce_menu_node_tree_get_node_type (node) == XFCE_MENU_NODE_TYPE_LAYOUT))
    {
      INDENT; g_print ("<%s>\n", node_name (node));
      print_child_nodes (node, depth);
      INDENT; g_print ("</%s>\n", node_name (node));
    }
  else
    {
      switch (xfce_menu_node_tree_get_node_type (node))
        {
        case XFCE_MENU_NODE_TYPE_NAME:
          INDENT; g_print ("<Name>%s</Name>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_DIRECTORY:
          INDENT; g_print ("<Directory>%s</Directory>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_DIRECTORY_DIR:
          INDENT; g_print ("<DirectoryDir>%s</DirectoryDir>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS:
          INDENT; g_print ("<DefaultDirectoryDirs/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_APP_DIR:
          INDENT; g_print ("<AppDir>%s</AppDir>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_DEFAULT_APP_DIRS:
          INDENT; g_print ("<DefaultAppDirs/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_ONLY_UNALLOCATED:
          INDENT; g_print ("<OnlyUnallocated/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED:
          INDENT; g_print ("<NotOnlyUnallocated/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_DELETED:
          INDENT; g_print ("<Deleted/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_NOT_DELETED:
          INDENT; g_print ("<NotDeleted/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_FILENAME:
          INDENT; g_print ("<Filename>%s</Filename>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_CATEGORY:
          INDENT; g_print ("<Category>%s</Category>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_OLD:
          INDENT; g_print ("<Old>%s</Old>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_NEW:
          INDENT; g_print ("<New>%s</New>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_MENUNAME:
          INDENT; g_print ("<Menuname>%s</Menuname>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_SEPARATOR:
          INDENT; g_print ("<Separator/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_MERGE:
          INDENT; 
          switch (xfce_menu_node_tree_get_layout_merge_type (node))
            {
            case XFCE_MENU_LAYOUT_MERGE_ALL:
              g_print ("<Merge type=\"all\"/>\n");
              break;
            case XFCE_MENU_LAYOUT_MERGE_MENUS:
              g_print ("<Merge type=\"menus\"/>\n");
              break;
            case XFCE_MENU_LAYOUT_MERGE_FILES:
              g_print ("<Merge type=\"files\"/>\n");
              break;
            }
          break;
        case XFCE_MENU_NODE_TYPE_MERGE_FILE:
          INDENT;
          switch (xfce_menu_node_tree_get_merge_file_type (node))
            {
            case XFCE_MENU_MERGE_FILE_PATH:
              g_print ("<MergeFile type=\"path\">%s</MergeFile>\n", 
                       xfce_menu_node_tree_get_merge_file_filename (node));
              break;
            case XFCE_MENU_MERGE_FILE_PARENT:
              g_print ("<MergeFile type=\"parent\"/>\n");
              break;
            }
          break;
        case XFCE_MENU_NODE_TYPE_MERGE_DIR:
          INDENT; g_print ("<MergeDir>%s</MergeDir>\n", xfce_menu_node_tree_get_string (node));
          break;
        case XFCE_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS:
          INDENT; g_print ("<DefaultMergeDirs/>\n");
          break;
        default:
          break;
        }
    }

  return FALSE;
}



void
print_tree (XfceMenuTreeProvider *provider)
{
  GNode *tree;

  g_return_if_fail (XFCE_IS_MENU_TREE_PROVIDER (provider));

  tree = xfce_menu_tree_provider_get_tree (provider);
  print_node (tree, 0);
  xfce_menu_node_tree_free (tree);
}



static const gchar ROOT_SPECS[][30] = 
{
  "menus/applications.menu",
  "menus/xfce-applications.menu",
  "menus/gnome-applications.menu",
  "menus/kde-applications.menu",
};



int
main (int    argc,
      char **argv)
{
  XfceMenuParser *parser;
  XfceMenuMerger *merger;
  GError         *error = NULL;
  GFile          *file = NULL;
  gchar          *filename;
  gint            result = EXIT_SUCCESS;
  gint            n;

  if (!g_thread_supported ())
    g_thread_init (NULL);

  libxfce4menu_init ("XFCE");

  if (argc > 1)
    file = g_file_new_for_path (argv[1]);
  else
    {
      /* Search for a usable root menu file */
      for (n = 0; n < G_N_ELEMENTS (ROOT_SPECS) && file == NULL; ++n)
        {
          /* Search for the root menu file */
          filename = xfce_menu_config_lookup (ROOT_SPECS[n]);
          if (G_UNLIKELY (filename == NULL))
            continue;

          /* Try to load the root menu from this file */
          file = g_file_new_for_path (filename);
          g_free (filename);

          if (!g_file_query_exists (file, NULL))
            {
              g_object_unref (file);
              file = NULL;
            }
        }
    }

  parser = xfce_menu_parser_new (file);
  g_object_unref (file);
  
  if (G_LIKELY (xfce_menu_parser_run (parser, NULL, &error)))
    {
      print_tree (XFCE_MENU_TREE_PROVIDER (parser));

      merger = xfce_menu_merger_new (XFCE_MENU_TREE_PROVIDER (parser));

      if (G_LIKELY (xfce_menu_merger_run (merger, NULL, &error)))
        {
          g_print ("\n\n");
          print_tree (XFCE_MENU_TREE_PROVIDER (merger));
        }
      else
        {
          if (error != NULL)
            {
              g_error ("Could not merge menus in %s: %s", 
                       argc > 1 ? argv[1] : FILENAME, 
                       error->message);

              g_error_free (error);
            }

          result = EXIT_FAILURE;
        }

      g_object_unref (merger);
    }
  else
    {
      if (error != NULL)
        {
          g_error ("Could not parse %s: %s", argc > 1 ? argv[1] : FILENAME, error->message);
          g_error_free (error);
        }

      result = EXIT_FAILURE;
    }

  g_object_unref (parser);

  libxfce4menu_shutdown ();

  return result;
}

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
  XfceMenuNode *node_ = node->data;

  if (node_ == NULL) return "Menu";

  switch (node_->node_type)
    {
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
  gint   i;

  for (i = 0, child = g_node_nth_child (node, i); 
       child != NULL; 
       ++i, child = g_node_nth_child (node, i))
    {
      g_node_traverse (child, G_PRE_ORDER, G_TRAVERSE_ALL, 1, 
                       (GNodeTraverseFunc) print_node, GINT_TO_POINTER (depth+2));
    }
}



static gboolean
print_node (GNode *node,
            gint   depth)
{
  XfceMenuNode *node_ = node->data;
  gint          i;

#define INDENT {for (i = 0; i < depth; ++i) g_print (" ");}

  if (G_UNLIKELY (node_ == NULL || 
                  node_->node_type == XFCE_MENU_NODE_TYPE_INCLUDE ||
                  node_->node_type == XFCE_MENU_NODE_TYPE_EXCLUDE ||
                  node_->node_type == XFCE_MENU_NODE_TYPE_OR ||
                  node_->node_type == XFCE_MENU_NODE_TYPE_AND ||
                  node_->node_type == XFCE_MENU_NODE_TYPE_NOT ||
                  node_->node_type == XFCE_MENU_NODE_TYPE_MOVE ||
                  node_->node_type == XFCE_MENU_NODE_TYPE_LAYOUT))
    {
      INDENT; g_print ("<%s>\n", node_name (node));
      print_child_nodes (node, depth);
      INDENT; g_print ("</%s>\n", node_name (node));
    }
  else
    {
      switch (node_->node_type)
        {
        case XFCE_MENU_NODE_TYPE_NAME:
          INDENT; g_print ("<Name>%s</Name>\n", node_->data.string);
          break;
        case XFCE_MENU_NODE_TYPE_DIRECTORY:
          INDENT; g_print ("<Directory>%s</Directory>\n", node_->data.string);
          break;
        case XFCE_MENU_NODE_TYPE_DIRECTORY_DIR:
          INDENT; g_print ("<DirectoryDir>%s</DirectoryDir>\n", node_->data.string);
          break;
        case XFCE_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS:
          INDENT; g_print ("<DefaultDirectoryDirs/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_APP_DIR:
          INDENT; g_print ("<AppDir>%s</AppDir>\n", node_->data.string);
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
          INDENT; g_print ("<Filename>%s</Filename>\n", node_->data.string);
          break;
        case XFCE_MENU_NODE_TYPE_CATEGORY:
          INDENT; g_print ("<Category>%s</Category>\n", node_->data.string);
          break;
        case XFCE_MENU_NODE_TYPE_OLD:
          INDENT; g_print ("<Old>%s</Old>\n", node_->data.string);
          break;
        case XFCE_MENU_NODE_TYPE_NEW:
          INDENT; g_print ("<New>%s</New>\n", node_->data.string);
          break;
        case XFCE_MENU_NODE_TYPE_MENUNAME:
          INDENT; g_print ("<Menuname>%s</Menuname>\n", node_->data.string);
          break;
        case XFCE_MENU_NODE_TYPE_SEPARATOR:
          INDENT; g_print ("<Separator/>\n");
          break;
        case XFCE_MENU_NODE_TYPE_MERGE:
          INDENT; 
          switch (node_->data.layout_merge_type)
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
          switch (node_->data.merge_file.type)
            {
            case XFCE_MENU_MERGE_FILE_PATH:
              g_print ("<MergeFile type=\"path\">%s</MergeFile>\n", 
                       node_->data.merge_file.filename);
              break;
            case XFCE_MENU_MERGE_FILE_PARENT:
              g_print ("<MergeFile type=\"parent\"/>\n");
              break;
            }
          break;
        case XFCE_MENU_NODE_TYPE_MERGE_DIR:
          INDENT; g_print ("<MergeDir>%s</MergeDir>\n", node_->data.string);
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



int
main (int    argc,
      char **argv)
{
  XfceMenuParser *parser;
  XfceMenuMerger *merger;
  GError         *error = NULL;
  GFile          *file;
  gint            result = EXIT_SUCCESS;

  if (!g_thread_supported ())
    g_thread_init (NULL);

  xfce_menu_init ("XFCE");

  file = g_file_new_for_path (argc > 1 ? argv[1] : FILENAME);
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
          g_error ("Could not merge menus in %s: %s", 
                   argc > 1 ? argv[1] : FILENAME, 
                   error->message);

          g_error_free (error);

          result = EXIT_FAILURE;
        }

      g_object_unref (merger);
    }
  else
    {
      g_error ("Could not parse %s: %s", argc > 1 ? argv[1] : FILENAME, error->message);
      g_error_free (error);

      result = EXIT_FAILURE;
    }

  g_object_unref (parser);

  xfce_menu_shutdown ();

  return result;
}

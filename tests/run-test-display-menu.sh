#!/bin/sh
#
# Copyright (c) 2007 Jannis Pohlmann <jannis@xfce.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301 USA.
#

GLIB_OPTIONS=`pkg-config --cflags --libs glib-2.0`
GTK_OPTIONS=`pkg-config --cflags --libs gtk+-2.0`
GARCON_OPTIONS=`pkg-config --cflags --libs garcon-1`

gcc -o test-display-menu test-display-menu.c $GLIB_OPTIONS $GTK_OPTIONS $GARCON_OPTIONS &&
G_DEBUG=fatal-criticals gdb ./test-display-menu -x gdb-test-display-menu

#!/bin/sh

GLIB_OPTIONS=`pkg-config --cflags --libs glib-2.0`
GTK_OPTIONS=`pkg-config --cflags --libs gtk+-2.0`
GARCON_OPTIONS=`pkg-config --cflags --libs garcon-1`

gcc -o test-display-menu test-display-menu.c $GLIB_OPTIONS $GTK_OPTIONS $GARCON_OPTIONS &&
G_DEBUG=fatal-criticals gdb ./test-display-menu -x gdb-test-display-menu

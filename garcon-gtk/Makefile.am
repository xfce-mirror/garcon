# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
# 
# Copyright (c) 2013 Nick Schermer <nick@xfce.org>
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU Library General Public License for more details.
# 
# You should have received a copy of the GNU Library General 
# Public License along with this library; if not, write to the 
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DGARCON_COMPILATION \
	-DGARCON_VERSION_API=\"$(GARCON_VERSION_API)\" \
	-DG_LOG_DOMAIN=\"garcon\" \
	-DSYSCONFIGDIR=\"$(sysconfdir)/xdg\" \
	$(PLATFORM_CFLAGS)

lib_LTLIBRARIES = \
	libgarcon-gtk3-1.la

libgarcon_gtk_headers = \
	garcon-gtk.h \
	garcon-gtk-menu.h

libgarcon_gtk_sources = \
	garcon-gtk-menu.c


libgarcon_gtk3includedir = \
	$(includedir)/garcon-gtk3-1/garcon-gtk

libgarcon_gtk3include_HEADERS = \
	$(libgarcon_gtk_headers)

libgarcon_gtk3_1_la_SOURCES = \
	$(libgarcon_gtk_sources) \
	$(libgarcon_gtk_headers)

libgarcon_gtk3_1_la_CFLAGS = \
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(LIBXFCE4UI2_CFLAGS) \
	$(PLATFORM_CFLAGS)

libgarcon_gtk3_1_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info $(GARCON_VERINFO) \
	-export-symbols-regex "^[^_].*" \
	$(PLATFORM_LDFLAGS)

libgarcon_gtk3_1_la_LIBADD = \
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la \
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(GTK3_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(GOBJECT_LIBS) \
	$(GTHREAD_LIBS) \
	$(LIBXFCE4UI2_LIBS)

libgarcon_gtk3_1_la_DEPENDENCIES = \
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = garcon-gtk3-1.pc

##
## GTK+ 2 support library
##
if ENABLE_GTK2_LIBRARY

lib_LTLIBRARIES += libgarcon-gtk2-1.la

libgarcon_gtk2includedir = \
	$(includedir)/garcon-gtk2-1/garcon-gtk

libgarcon_gtk2include_HEADERS = \
	$(libgarcon_gtk_headers)

libgarcon_gtk2_1_la_SOURCES = \
	$(libgarcon_gtk_sources) \
	$(libgarcon_gtk_headers)

libgarcon_gtk2_1_la_CFLAGS = \
	$(GIO_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK2_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(PLATFORM_CFLAGS)

libgarcon_gtk2_1_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info $(GARCON_VERINFO) \
	-export-symbols-regex "^[^_].*" \
	$(PLATFORM_LDFLAGS)

libgarcon_gtk2_1_la_LIBADD = \
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la \
	$(GIO_LIBS) \
	$(GLIB_LIBS) \
	$(GTK2_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(GOBJECT_LIBS) \
	$(GTHREAD_LIBS) \
	$(LIBXFCE4UI_LIBS)

libgarcon_gtk2_1_la_DEPENDENCIES = \
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la

pkgconfig_DATA += garcon-gtk2-1.pc

endif


# Required for gtk-doc and make distcheck
dist-hook: all

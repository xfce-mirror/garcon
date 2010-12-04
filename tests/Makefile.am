# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
# 
# Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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

INCLUDES =								\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"garcon-tests\"

noinst_PROGRAMS =							\
	test-menu-parser						\
	test-menu-spec

test_menu_parser_SOURCES =						\
	test-menu-parser.c

test_menu_parser_CFLAGS =						\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(GIO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GOBJECT_CFLAGS)

test_menu_parser_DEPENDENCIES =						\
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la

test_menu_parser_LDADD =						\
	$(LIBXFCE4UTIL_LIBS)						\
	$(GIO_LIBS)							\
	$(GLIB_LIBS)							\
	$(GOBJECT_LIBS)							\
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la

test_menu_spec_SOURCES =						\
	test-menu-spec.c

test_menu_spec_CFLAGS =							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(GIO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GOBJECT_CFLAGS)

test_menu_spec_DEPENDENCIES =						\
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la

test_menu_spec_LDADD =							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(GIO_LIBS)							\
	$(GLIB_LIBS)							\
	$(GOBJECT_LIBS)							\
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la

EXTRA_DIST =								\
	test-display-menu.c

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
# 
# Copyright (c) 2007-2009 Jannis Pohlmann <jannis@xfce.org>
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

INCLUDES = 								\
	-I$(top_srcdir)							\
	-DGARCON_COMPILATION						\
	-DGARCON_VERSION_API=\"$(GARCON_VERSION_API)\"			\
	-DG_LOG_DOMAIN=\"garcon\"					\
	$(PLATFORM_CFLAGS)

lib_LTLIBRARIES = 							\
	libgarcon-1.la

libgarcon_headers =							\
	garcon.h							\
	garcon-config.h							\
	garcon-menu-element.h						\
	garcon-menu-separator.h						\
	garcon-menu-directory.h						\
	garcon-menu-item-pool.h						\
	garcon-menu-item-cache.h					\
	garcon-environment.h						\
	garcon-menu.h							\
	garcon-menu-item.h						\
	garcon-menu-node.h						\
	garcon-menu-tree-provider.h					\
	garcon-menu-merger.h						\
	garcon-menu-parser.h

libgarcon_sources =							\
	garcon-config.c							\
	garcon-menu-element.c						\
	garcon-menu-separator.c						\
	garcon-menu-directory.c						\
	garcon-menu-item-pool.c						\
	garcon-menu-item-cache.c					\
	garcon-environment.c						\
	garcon-menu.c							\
	garcon-menu-item.c						\
	garcon-menu-node.c						\
	garcon-menu-tree-provider.c					\
	garcon-menu-merger.c						\
	garcon-menu-parser.c						\
	garcon-private.c						\
	garcom-private.h

libgarconincludedir = $(includedir)/garcon-1/garcon

libgarconinclude_HEADERS =						\
	$(libgarcon_headers)

libgarcon_1_la_SOURCES =						\
	$(libgarcon_sources)						\
	$(libgarcon_headers)

libgarcon_1_la_CFLAGS = 						\
	$(GLIB_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(GIO_CFLAGS)							\
	$(PLATFORM_CFLAGS)

libgarcon_1_la_LDFLAGS =						\
	-no-undefined							\
	-export-dynamic							\
	-version-info $(GARCON_VERINFO)					\
	-export-symbols-regex "^[^_].*"					\
	$(PLATFORM_LDFLAGS)
	
libgarcon_1_la_LIBADD = 						\
	$(GLIB_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GIO_LIBS)

EXTRA_DIST =								\
	garcon-config.h.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = garcon-1.pc

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
# 
# Copyright (c) 2007-2010 Jannis Pohlmann <jannis@xfce.org>
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

AM_CPPFLAGS = 								\
	-I$(top_srcdir)							\
	-DGARCON_COMPILATION						\
	-DGARCON_VERSION_API=\"$(GARCON_VERSION_API)\"			\
	-DG_LOG_DOMAIN=\"garcon\"					\
	-DSYSCONFIGDIR=\"$(sysconfdir)/xdg\"				\
	$(PLATFORM_CFLAGS)

lib_LTLIBRARIES = 							\
	libgarcon-1.la							\
	libgarcon-2.la

libgarcon_built_public_sources =					\
	garcon-marshal.h

libgarcon_built_sources =						\
	$(libgarcon_built_public_sources)				\
	garcon-marshal.c

libgarcon_headers =							\
	garcon.h							\
	garcon-config.h							\
	garcon-marshal.h						\
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
	garcon-marshal.c						\
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
	garcon-private.h

libgarconincludedir = $(includedir)/garcon-1/garcon

libgarconinclude_HEADERS =						\
	$(libgarcon_headers)

libgarcon_1_la_SOURCES =						\
	$(libgarcon_sources)						\
	$(libgarcon_headers)

libgarcon_1_la_CFLAGS = 						\
	$(GIO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(GOBJECT_CFLAGS)						\
	$(GTHREAD_CFLAGS)						\
	$(PLATFORM_CFLAGS)

libgarcon_1_la_LDFLAGS =						\
	-no-undefined							\
	-export-dynamic							\
	-version-info $(GARCON_VERINFO)					\
	-export-symbols-regex "^[^_].*"					\
	$(PLATFORM_LDFLAGS)

libgarcon_1_la_LIBADD = 						\
	$(GIO_LIBS)							\
	$(GLIB_LIBS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(GOBJECT_LIBS)							\
	$(GTHREAD_LIBS)

libgarcon2_includedir = $(includedir)/garcon-2/garcon

libgarcon2_include_HEADERS = $(libgarcon_headers)

libgarcon_2_la_SOURCES =						\
	$(libgarcon_sources)						\
	$(libgarcon_headers)

libgarcon_2_la_CFLAGS = 						\
	$(GIO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(GOBJECT_CFLAGS)						\
	$(GTHREAD_CFLAGS)						\
	$(PLATFORM_CFLAGS)

libgarcon_2_la_LDFLAGS =						\
	-no-undefined							\
	-export-dynamic							\
	-version-info $(GARCON_VERINFO)					\
	-export-symbols-regex "^[^_].*"					\
	$(PLATFORM_LDFLAGS)

libgarcon_2_la_LIBADD = 						\
	$(GIO_LIBS)							\
	$(GLIB_LIBS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(GOBJECT_LIBS)							\
	$(GTHREAD_LIBS)

EXTRA_DIST =								\
	garcon-config.h.in						\
	garcon-marshal.list

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = garcon-1.pc garcon-2.pc

DISTCLEANFILES =							\
	stamp-garcon-marshal.h						\
	garcon-config.h							\
	$(libgarcon_built_sources)

BUILT_SOURCES =								\
	$(libgarcon_built_sources)

garcon-marshal.h: stamp-garcon-marshal.h
	@true

stamp-garcon-marshal.h: garcon-marshal.list Makefile
	$(AM_V_GEN) ( \
	  cd $(top_builddir)/garcon \
	  && echo "#ifndef __GARCON_MARSHAL_H__" > xgen-emh \
	  && echo "#define __GARCON_MARSHAL_H__" >> xgen-emh \
	  && ( glib-genmarshal \
	  	--prefix=garcon_marshal \
	  	--header $(srcdir)/garcon-marshal.list ) >> xgen-emh \
	  && echo "#endif /* !__GARCON_MARSHAL_H__ */" >> xgen-emh \
	  && ( cmp -s xgen-emh garcon-marshal.h \
	  	|| cp xgen-emh garcon-marshal.h ) \
	  && rm -f xgen-emh \
	  && echo timestamp > $(@F) \
	)

garcon-marshal.c: garcon-marshal.list Makefile
	$(AM_V_GEN) ( \
	  cd $(top_builddir)/garcon \
	  && echo "#include <garcon/garcon-marshal.h>" > xgen-emc \
	  && ( glib-genmarshal \
	  	--prefix=garcon_marshal \
	  	--body $(srcdir)/garcon-marshal.list ) >> xgen-emc \
	  && cp xgen-emc garcon-marshal.c \
	  && rm -f xgen-emc \
	)

# Required for gtk-doc and make distcheck
dist-hook: all

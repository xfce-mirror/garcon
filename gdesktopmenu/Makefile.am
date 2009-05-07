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
	-DGDESKTOPMENU_COMPILATION					\
	-DGDESKTOPMENU_VERSION_API=\"$(GDESKTOPMENU_VERSION_API)\"	\
	-DG_LOG_DOMAIN=\"gdesktopmenu\"					\
	$(PLATFORM_CFLAGS)

lib_LTLIBRARIES = 							\
	libgdesktopmenu-1.la

libgdesktopmenu_headers =						\
	gdesktopmenu.h							\
	gdesktopmenugio.h						\
	gdesktopmenuelement.h						\
	gdesktopmenuseparator.h						\
	gdesktopmenudirectory.h						\
	gdesktopmenuitempool.h						\
	gdesktopmenuitemcache.h						\
	gdesktopmenuenvironment.h					\
	gdesktopmenumain.h						\
	gdesktopmenumonitor.h						\
	gdesktopmenuitem.h						\
	gdesktopmenunode.h						\
	gdesktopmenutreeprovider.h					\
	gdesktopmenumerger.h						\
	gdesktopmenuparser.h

libgdesktopmenu_sources =						\
	gdesktopmenu.c							\
	gdesktopmenugio.c						\
	gdesktopmenuelement.c						\
	gdesktopmenuseparator.c						\
	gdesktopmenudirectory.c						\
	gdesktopmenuitempool.c						\
	gdesktopmenuitemcache.c						\
	gdesktopmenuenvironment.c					\
	gdesktopmenumain.c						\
	gdesktopmenumonitor.c						\
	gdesktopmenuitem.c						\
	gdesktopmenunode.c						\
	gdesktopmenutreeprovider.c					\
	gdesktopmenumerger.c						\
	gdesktopmenuparser.c

libgdesktopmenuincludedir = $(includedir)/gdesktopmenu-1/gdesktopmenu

libgdesktopmenuinclude_HEADERS =					\
	$(libgdesktopmenu_headers)					\
	gdesktopmenu.h							\
	gdesktopmenu-config.h

libgdesktopmenu_1_la_SOURCES =						\
	$(libgdesktopmenu_sources)					\
	$(libgdesktopmenu_headers)

libgdesktopmenu_1_la_CFLAGS = 						\
	$(GLIB_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(GIO_CFLAGS)							\
	$(PLATFORM_CFLAGS)

libgdesktopmenu_1_la_LDFLAGS =						\
	-no-undefined							\
	-export-dynamic							\
	-version-info $(GDESKTOPMENU_VERINFO)				\
	$(PLATFORM_LDFLAGS)
	
libgdesktopmenu_1_la_LIBADD = 						\
	$(GLIB_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GIO_LIBS)

EXTRA_DIST =								\
	gdesktopmenu-config.h.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gdesktopmenu-1.pc

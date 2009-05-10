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
	garcongio.h							\
	garconmenuelement.h						\
	garconmenuseparator.h						\
	garconmenudirectory.h						\
	garconmenuitempool.h						\
	garconmenuitemcache.h						\
	garconenvironment.h						\
	garconmenu.h							\
	garconmenumonitor.h						\
	garconmenuitem.h						\
	garconmenunode.h						\
	garconmenutreeprovider.h					\
	garconmenumerger.h						\
	garconmenuparser.h

libgarcon_sources =							\
	garcon.c							\
	garcongio.c							\
	garconmenuelement.c						\
	garconmenuseparator.c						\
	garconmenudirectory.c						\
	garconmenuitempool.c						\
	garconmenuitemcache.c						\
	garconenvironment.c						\
	garconmenu.c							\
	garconmenumonitor.c						\
	garconmenuitem.c						\
	garconmenunode.c						\
	garconmenutreeprovider.c					\
	garconmenumerger.c						\
	garconmenuparser.c

libgarconincludedir = $(includedir)/garcon-1/garcon

libgarconinclude_HEADERS =						\
	$(libgarcon_headers)						\
	garcon.h							\
	garcon-config.h

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
	$(PLATFORM_LDFLAGS)
	
libgarcon_1_la_LIBADD = 						\
	$(GLIB_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GIO_LIBS)

EXTRA_DIST =								\
	garcon-config.h.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = garcon-1.pc

# $Id$ 
#
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

INCLUDES = 								\
	-I$(top_srcdir)							\
	-DLIBXFCE4MENU_COMPILATION					\
	-DLIBXFCE4MENU_VERSION_API=\"$(LIBXFCE4MENU_VERSION_API)\"	\
	-DG_LOG_DOMAIN=\"libxfce4menu\"					\
	$(PLATFORM_CFLAGS)

lib_LTLIBRARIES = 							\
	libxfce4menu-0.1.la

libxfce4menu_headers =							\
	xfce-menu.h							\
	xfce-menu-separator.h						\
	xfce-menu-layout.h						\
	xfce-menu-directory.h						\
	xfce-menu-rules.h						\
	xfce-menu-standard-rules.h					\
	xfce-menu-or-rules.h						\
	xfce-menu-and-rules.h						\
	xfce-menu-not-rules.h						\
	xfce-menu-item-pool.h						\
	xfce-menu-item-cache.h						\
	xfce-menu-move.h						\
	xfce-menu-environment.h						\
	xfce-menu-item.h						

libxfce4menu_sources =							\
	xfce-menu.c							\
	xfce-menu-separator.c						\
	xfce-menu-layout.c						\
	xfce-menu-directory.c						\
	xfce-menu-rules.c						\
	xfce-menu-standard-rules.c					\
	xfce-menu-or-rules.c						\
	xfce-menu-and-rules.c						\
	xfce-menu-not-rules.c						\
	xfce-menu-item-pool.c						\
	xfce-menu-item-cache.c						\
	xfce-menu-move.c						\
	xfce-menu-environment.c						\
	xfce-menu-item.c						

libxfce4menuincludedir = $(includedir)/xfce4/libxfce4menu-0.1/libxfce4menu

libxfce4menuinclude_HEADERS =						\
	$(libxfce4menu_headers)						\
	libxfce4menu.h							\
	libxfce4menu-config.h

libxfce4menu_0_1_la_DEPENDENCIES = 					\
	$(top_builddir)/tdb/libtdb.la

libxfce4menu_0_1_la_SOURCES =						\
	$(libxfce4menu_sources)						\
	$(libxfce4menu_headers)

libxfce4menu_0_1_la_CFLAGS = 						\
	$(GLIB_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(PLATFORM_CFLAGS)						

libxfce4menu_0_1_la_LDFLAGS =						\
	-no-undefined							\
	-export-dynamic							\
	-version-info $(LIBXFCE4MENU_VERINFO)				\
	$(PLATFORM_LDFLAGS)
	
libxfce4menu_0_1_la_LIBADD = 						\
	$(GLIB_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(top_builddir)/tdb/libtdb.la

EXTRADIST =								\
	libxfce4menu-config.h.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libxfce4menu-0.1.pc

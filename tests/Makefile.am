# $Id$
#
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

INCLUDES =								\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"libxfce4menu-tests\"

TESTS =									\
	test-menu-spec							\
	test-display-menu

check_PROGRAMS =							\
	test-menu-spec							\
	test-display-menu

test_menu_spec_SOURCES =						\
	test-menu-spec.c

test_menu_spec_CFLAGS =							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(GLIB_CFLAGS)

test_menu_spec_DEPENDENCIES =						\
	$(top_builddir)/libxfce4menu/libxfce4menu-$(LIBXFCE4MENU_VERSION_MAJOR).$(LIBXFCE4MENU_VERSION_MINOR).la

test_menu_spec_LDADD =							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(GLIB_LIBS)							\
	$(top_builddir)/libxfce4menu/libxfce4menu-$(LIBXFCE4MENU_VERSION_MAJOR).$(LIBXFCE4MENU_VERSION_MINOR).la

test_display_menu_SOURCES =						\
	test-display-menu.c

test_display_menu_CFLAGS =						\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(GLIB_CFLAGS)

test_display_menu_DEPENDENCIES =					\
	$(top_builddir)/libxfce4menu/libxfce4menu-$(LIBXFCE4MENU_VERSION_MAJOR).$(LIBXFCE4MENU_VERSION_MINOR).la

test_display_menu_LDADD =						\
	$(LIBXFCE4UTIL_LIBS)						\
	$(GTK_LIBS)							\
	$(GLIB_LIBS)							\
	$(top_builddir)/libxfce4menu/libxfce4menu-$(LIBXFCE4MENU_VERSION_MAJOR).$(LIBXFCE4MENU_VERSION_MINOR).la


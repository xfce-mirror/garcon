# $Id$
#
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

AUTOMAKE_OPTIONS = 1.8

# Name of the module
DOC_MODULE=libxfce4menu

# Top-level SGML file
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.xml

SCAN_OPTIONS=--rebuild-sections

# Extra options to pass to gtkdoc-scangobj
SCANGOBJ_OPTIONS=

# Directory containing the source code
DOC_SOURCE_DIR=../../libxfce4menu

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS=--sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS=

# Used for dependencies
HFILE_GLOB=$(top_srcdir)/libxfce4menu/*.h
CFILE_GLOB=$(top_srcdir)/libxfce4menu/*.c

# Extra files to add when scanning (relative to $srcdir)
EXTRA_HFILES=

# Images to copy into HTML directory
HTML_IMAGES =

# Extra SGML files that are included by DOC_MAIN_SGML_FILE
content_files =								\
	version.xml

# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
INCLUDES =	 							\
	-I$(top_srcdir) 						\
	-I$(top_builddir) 						\
	$(GLIB_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)

GTKDOC_LIBS =								\
	$(GLIB_CFLAGS)							\
	$(GTHREAD_LIBS)							\
	$(GTK_CFLAGS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(top_builddir)/libxfce4menu/libxfce4menu-$(LIBXFCE4MENU_VERSION_API).la

include $(top_srcdir)/gtk-doc.make

# Other files to distribute
EXTRA_DIST +=								\
	version.xml.in

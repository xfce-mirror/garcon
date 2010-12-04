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

AUTOMAKE_OPTIONS = 1.8

# Name of the module
DOC_MODULE=garcon

# Top-level SGML file
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.xml

SCAN_OPTIONS=--rebuild-sections

# Extra options to pass to gtkdoc-scangobj
SCANGOBJ_OPTIONS=

# Directory containing the source code
DOC_SOURCE_DIR=../../garcon

# Extra options to supply to gtkdoc-mkdb
MKDB_OPTIONS=--sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-fixref
FIXXREF_OPTIONS=

# Used for dependencies
HFILE_GLOB=$(top_srcdir)/garcon/*.h
CFILE_GLOB=$(top_srcdir)/garcon/*.c

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
	$(GIO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GOBJECT_CFLAGS)						\
	$(GTHREAD_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)

GTKDOC_LIBS =								\
	$(GIO_LIBS)							\
	$(GLIB_LIBS)							\
	$(GOBJECT_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GTK_CFLAGS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(top_builddir)/garcon/libgarcon-$(GARCON_VERSION_API).la

include $(top_srcdir)/gtk-doc.make

# Other files to distribute
EXTRA_DIST +=								\
	version.xml.in

# Required for make distcheck
dist-hook: all

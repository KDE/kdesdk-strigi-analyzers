## Makefile.am for diff file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_diff.h

kde_module_LTLIBRARIES = kfile_diff.la

kfile_diff_la_SOURCES = kfile_diff.cpp
kfile_diff_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_diff_la_LIBADD = $(LIB_KSYCOCA)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kfile_diff.pot

services_DATA = kfile_diff.desktop
servicesdir = $(kde_servicesdir)

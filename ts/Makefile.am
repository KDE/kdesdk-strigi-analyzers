## Makefile.am for text file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

noinst_HEADERS   = kfile_ts.h

kde_module_LTLIBRARIES = kfile_ts.la

kfile_ts_la_SOURCES = kfile_ts.cpp
kfile_ts_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_ts_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kfile_ts.pot

services_DATA = kfile_ts.desktop
servicesdir = $(kde_servicesdir)

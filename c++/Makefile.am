## Makefile.am for c++ file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_cpp.h

kde_module_LTLIBRARIES = kfile_cpp.la

kfile_cpp_la_SOURCES = kfile_cpp.cpp
kfile_cpp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_cpp_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kfile_cpp.pot

services_DATA = kfile_cpp.desktop kfile_h.desktop
servicesdir = $(kde_servicesdir)

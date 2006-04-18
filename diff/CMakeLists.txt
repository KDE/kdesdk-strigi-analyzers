kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_diff_PART_SRCS kfile_diff.cpp )

kde4_automoc(${kfile_diff_PART_SRCS})

kde4_add_plugin(kfile_diff ${kfile_diff_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_diff )

target_link_libraries(kfile_diff  ${KDE4_KIO_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kfile_diff )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_diff.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for diff file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS   = kfile_diff.h
#
#kde_module_LTLIBRARIES = kfile_diff.la
#
#kfile_diff_la_SOURCES = kfile_diff.cpp
#kfile_diff_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_diff_la_LIBADD = $(LIB_KSYCOCA)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kfile_diff.pot
#
#services_DATA = kfile_diff.desktop
#servicesdir = $(kde_servicesdir)

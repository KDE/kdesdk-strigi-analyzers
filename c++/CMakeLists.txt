kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_cpp_PART_SRCS kfile_cpp.cpp )

kde4_automoc(${kfile_cpp_PART_SRCS})

kde4_add_plugin(kfile_cpp ${kfile_cpp_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_cpp )

target_link_libraries(kfile_cpp ${LIB_KIO} )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_cpp )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_ts.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for text file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
#
#noinst_HEADERS   = kfile_ts.h
#
#kde_module_LTLIBRARIES = kfile_ts.la
#
#kfile_ts_la_SOURCES = kfile_ts.cpp
#kfile_ts_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_ts_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kfile_ts.pot
#
#services_DATA = kfile_ts.desktop
#servicesdir = $(kde_servicesdir)
